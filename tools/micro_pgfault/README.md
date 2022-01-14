micro pagefault is an micro benchmark for x86_64 Linux platform.


All test will map a 10GB memory, then touch first 8B of every page in the 10GB address space to triger page faults, and unmap the space. 
the test will run for 20 times with 200GB memory allocation.

# Standard 4K

## MMAP 4K

```
$ ./a.out 
=========== Test Mode    =================
Brk Mode:               0
Fixedaddr Mode:         0
Populate  Mode   :      0
Hugetlbfs 1G Mode:      0
Hugetlbfs 2M Mode:      0
Madvise Hugep Mode:     0
=========== Test Summary =================
Mmap Time:  0.000150 seconds
Total Time: 59.439977 seconds
PageFault:  52428800 times
Bandwidth:  3445.492572 MB/s
=========== Test End     =================
```


## Madvise THP 2M

```
$ ./a.out -t
=========== Test Mode    =================
Brk Mode:               0
Fixedaddr Mode:         0
Populate  Mode   :      0
Hugetlbfs 1G Mode:      0
Hugetlbfs 2M Mode:      0
Madvise Hugep Mode:     1
=========== Test Summary =================
Mmap Time:  0.000150 seconds
Total Time: 24.554223 seconds
PageFault:  112620 times
Bandwidth:  8340.724252 MB/s
=========== Test End     =================

```

# SMM

Use smm_elf_hack to modify ./a.out first.

## SMM-4K

```
$ SMM_CODE=2m SMM_HEAP=1g SMM_MMAP=11g SMM_STACK=8m ./a.out 
=========== Test Mode    =================
Brk Mode:               0
Fixedaddr Mode:         0
Populate  Mode   :      0
Hugetlbfs 1G Mode:      0
Hugetlbfs 2M Mode:      0
Madvise Hugep Mode:     0
=========== Test Summary =================
Mmap Time:  0.000105 seconds
Total Time: 51.968969 seconds
PageFault:  52428800 times
Bandwidth:  3940.813233 MB/s
=========== Test End     =================

```

## SMM-2M

```
$ SMM_CODE=2m SMM_HEAP=1g SMM_MMAP=11g SMM_STACK=8m SMM_PMD=1 ./a.out 
=========== Test Mode    =================
Brk Mode:               0
Fixedaddr Mode:         0
Populate  Mode   :      0
Hugetlbfs 1G Mode:      0
Hugetlbfs 2M Mode:      0
Madvise Hugep Mode:     0
=========== Test Summary =================
Mmap Time:  0.000050 seconds
Total Time: 16.980498 seconds
PageFault:  112620 times
Bandwidth:  12060.894881 MB/s
=========== Test End     =================

```

### SMM-1G

```
$ SMM_CODE=2m SMM_HEAP=1g SMM_MMAP=11g SMM_STACK=8m SMM_PMD=1 SMM_PUD=1 ./a.out 
=========== Test Mode    =================
Brk Mode:               0
Fixedaddr Mode:         0
Populate  Mode   :      0
Hugetlbfs 1G Mode:      0
Hugetlbfs 2M Mode:      0
Madvise Hugep Mode:     0
=========== Test Summary =================
Mmap Time:  0.000050 seconds
Total Time: 12.799303 seconds
PageFault:  20640 times
Bandwidth:  16000.870945 MB/s
=========== Test End     =================

```
