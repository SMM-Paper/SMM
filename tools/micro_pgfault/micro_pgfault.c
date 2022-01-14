#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <stdint.h>
#include <asm/unistd.h>
#include <argp.h>
#include <stdbool.h>

#define MAP_HUGE_2MB    (21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB    (30 << MAP_HUGE_SHIFT)

static long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
		int cpu, int group_fd, unsigned long flags)
{
	int ret;

	ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
			group_fd, flags);
	return ret;
}      

#define SIZE (10UL<<30)
#define PAGE_SIZE (4UL<<10)
#define ROUND 20

const char *argp_program_version = "micro_pagefault 0.1";
const char *argp_program_bug_address = "<wxingyan@gmail.com>";
static char doc[] = "A micro benchmark to test Page fault performance.";
static char args_doc[] = "[FILENAME]...";
static struct argp_option options[] = { 
    { "thp", 't', 0, 0, "madvise huge 2M page with transparent huge page"},
    { "populate", 'p', 0, 0, "map populate"},
    { "fixed_address", 'f', 0, 0, "fixed aligned address"},
    { "brk", 'b', 0, 0, "use brk"},
    { "hugetlbfs_1g", 'g', 0, 0, "use hugetlbfs 1gb"},
    { "hugetlbfs_2m", 'm', 0, 0, "use hugetlbfs 2mb"},
    { 0 } 
};

struct arguments {
    int is_populate;
    int is_madvise;
    int is_aligned_address;
    int is_brk;
    int is_hugetlbfs_1g;
    int is_hugetlbfs_2m;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
    case 't': arguments->is_madvise = 1; break;
    case 'p': arguments->is_populate = 1; break;
    case 'f': arguments->is_aligned_address = 1; break;
    case 'b': arguments->is_brk = 1; break;
    case 'g': arguments->is_hugetlbfs_1g = 1; break;
    case 'm': arguments->is_hugetlbfs_2m = 1; break;
    case ARGP_KEY_ARG: return 0;
    default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char *argv[])
{
	void * addr;

	volatile unsigned long * t;
	int i, r;
	struct timespec start;
	struct timespec end;
	struct timespec mmap_start;
	struct timespec mmap_end;
	double t_mmap_ns = 0.0;
	double t_ns;
	struct perf_event_attr pe_attr_page_faults;
	uint64_t page_faults_count;
	struct arguments arguments = {0};


	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	memset(&pe_attr_page_faults, 0, sizeof(pe_attr_page_faults));
	pe_attr_page_faults.size = sizeof(pe_attr_page_faults);
	pe_attr_page_faults.type =   PERF_TYPE_SOFTWARE;
	pe_attr_page_faults.config = PERF_COUNT_SW_PAGE_FAULTS;
	pe_attr_page_faults.disabled = 1;
	pe_attr_page_faults.exclude_kernel = 1;

	int page_faults_fd = perf_event_open(&pe_attr_page_faults, 0, -1, -1, 0);
	if (page_faults_fd == -1) {
		printf("perf_event_open failed for page faults: %s\n", strerror(errno));
		return -1;
	}
	
	unsigned flag = MAP_ANONYMOUS | MAP_PRIVATE;
	void * a = NULL;
	if (arguments.is_aligned_address) {
		a = (void *)(0x1UL << 30);
		flag  = flag | MAP_FIXED;
	}
	if (arguments.is_populate) {
		flag  = flag | MAP_POPULATE;
	}
	if (arguments.is_hugetlbfs_1g) {
		flag = flag | MAP_HUGETLB | MAP_HUGE_1GB;
	}
	if (arguments.is_hugetlbfs_2m) {
		flag = flag | MAP_HUGETLB | MAP_HUGE_2MB;
	}

	ioctl(page_faults_fd, PERF_EVENT_IOC_RESET, 0);
	ioctl(page_faults_fd, PERF_EVENT_IOC_ENABLE, 0);

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start); // get initial time-stamp


	for (r = 0; r < ROUND; r ++) {
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mmap_start); // get initial time-stamp

		if (arguments.is_brk ) {
			addr = sbrk(SIZE);

		} else {
			addr = mmap(a,  SIZE, PROT_READ|PROT_WRITE, flag, 0, 0 );
		}
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mmap_end); // get initial time-stamp
		t_mmap_ns = t_mmap_ns +  (mmap_end.tv_sec - mmap_start.tv_sec) * 1.0e9 +
			(mmap_end.tv_nsec - mmap_start.tv_nsec);

		if (arguments.is_madvise)
			madvise(addr, SIZE, MADV_HUGEPAGE);
		else
			madvise(addr, SIZE, MADV_NOHUGEPAGE);

		t = (unsigned long *) addr;

		for (i=0; i<SIZE/PAGE_SIZE; i++) {
			* t = 0;
			t = (unsigned long *) ((unsigned long)t + PAGE_SIZE) ;
		}

		if (arguments.is_brk) {
			brk(addr);
		} else {
			munmap(addr, SIZE);
		}
	}



	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);   // get final time-stamp

	// Stop counting and read value
	ioctl(page_faults_fd, PERF_EVENT_IOC_DISABLE, 0);
	read(page_faults_fd, &page_faults_count, sizeof(page_faults_count));

	t_ns = (end.tv_sec - start.tv_sec) * 1.0e9 +
		(end.tv_nsec - start.tv_nsec);


	printf("=========== Test Mode    =================\n");
	printf("Brk Mode:               %d\n", arguments.is_brk);
	printf("Fixedaddr Mode:         %d\n", arguments.is_aligned_address);
	printf("Populate  Mode   :      %d\n", arguments.is_populate);
	printf("Hugetlbfs 1G Mode:      %d\n", arguments.is_hugetlbfs_1g);
	printf("Hugetlbfs 2M Mode:      %d\n", arguments.is_hugetlbfs_2m);
	printf("Madvise Hugep Mode:     %d\n", arguments.is_madvise);
	printf("=========== Test Summary =================\n");
	printf("Mmap Time:  %f seconds\n",t_mmap_ns/1.0e9);
	printf("Total Time: %f seconds\n",t_ns/1.0e9);
	printf("PageFault:  %ld times\n",page_faults_count - 1);
	printf("Bandwidth:  %f MB/s\n", 10.0*ROUND*1024/(t_ns / 1.0e9)); 
	printf("=========== Test End     =================\n");

}
