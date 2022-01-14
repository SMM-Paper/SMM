# Simulation Infrastructure.
For evaluation we use [ChampSim](https://github.com/ChampSim/ChampSim) , a detailed simulator that models an  out-of-order processor used in many TLB or Cache related researches. We modified ChampSim with the following configurations:

* **TLB-4KB & TLB-2MB**: Original ChampSim model 4KB page (TLB-4KB) and 2MB page (TLB-2MB). 
* **SMM-4KB & SMM-2MB**: SMM model with 4KB page (SMM-4KB) and 2MB page (SMM-2MB).
* **IdealRMM**: Last-level RangeTLB model proposed by RMM. The RangeTLB hit-rate set to $100\%$ when simulated.
* **IdealDS**: Ideal zero translation overhead model like Direct-Segment with access validation omitted.


| Component    | Description                                                                  |
|--------------|------------------------------------------------------------------------------|
| Processor    | 2.5Ghz, OoO, 6-fetch, 6-decode, 6-dispatch, 4-execute, 5-retire              |
| L1 I-Cache   | 32KB, 8-way, 1-cycle, 8-entry MSHR, 2-read, 2-write                          |
| L1 D-cache   | 48KB, 12-way, 1-cycle, 8-entry MSHR, 2-read, 2-write                         |
| L2 D-cache   | 256KB, 4-way, 12-cycle, 16-entry MSHR, 1-read, 1-write                       |
| LLC          | 8MB, 16-way, 38-cycle, 32-entry MSHR, 1-read, 1-write                        |
| DRAM         | 3200Mhz, 128G, tRP=tRCD=tCAS=11                                              |
| I-TLB        | 64-entry, fully associative, 1-cycle, 4-entry MSHR, 2-read, 2-write, 4KB/2MB |
| D-TLB        | 64-entry, fully associative, 1-cycle, 4-entry MSHR, 2-read, 2-write, 4KB/2MB |
| S-TLB        | 2048-entry, 12-way, 8-cycle, 8-entry MSHR, 1-read, 1-write, 4KB/2MB          |
| L/S Pipe     | 2-translate-for-load, 2-load, 2-translate-for-store, 2-store                 |
| SMM L/S Pipe | 2-access-validation, 2-load, 2-translate-for-store, 2-store                  |
| IdealRMM     | RangeTLB, 1-read, 4-cycle, always hit                                        |
| IdealDS      | direct access D-cache without AG stage, 2-load, 2-store                      |

# SMM operating system prototype.

We implement SMM in Linux kernel v5.7.  The total newly added or modified source lines of code(SLOC) is about 2938. We use several environment variables listed in the Table below to indicate the size of each virtual segment for a program. The user program does not need to make any changes, including link mode, malloc library, etc. It is only necessary to set up the above environment variables when the test program is started, so that SMM can be used transparently.

| ENV       | Meaning            | Example |
|-----------|--------------------|---------|
| SMM_CODE  | Code Segment Size  | 8m      |
| SMM_HEAP  | Heap Segment Size  | 256m    |
| SMM_MMAP  | Mmap Segment Size  | 60g     |
| SMM_STACK | Stack Segment Size | 8m      |


# Workloads

We use micro-benchmark and comprehensive benchmarks to evaluate. The micro-benchmark is [test-tlb](https://github.com/torvalds/test-tlb) for quick memory latency and TLB test. For comprehensive benchmarks, we consider all the benchmarks from [SPEC CPU 2017](https://www.spec.org/cpu2017/) and big data workloads included in [GAP suite](https://github.com/sbeamer/gapbs) and the [XSBench](https://github.com/ANL-CESAR/XSBench). We choose six graph benchmarks from the GAP suite. The input data is generated with a Kronecker graph generator with the same parameters as Graph 500 (A=0.57, B=C=0.19, D=0.05). All traces were obtained using the [Pin](https://www.intel.com/content/www/us/en/developer/articles/tool/pin-a-dynamic-binary-instrumentation-tool.html) methodology. Each SPEC workload runs 1 million warmup instructions and one billion instructions are executed to measure the experimental results. For the test-tlb, GAP and XSBench workloads,  we use 1 million warmup instructions and 100 million instructions for measuring the results.


# SMM ChampSim Usage Example


The traces are obtained manually following ChampSim instructions.

For convenience, we currently disable the randmaps (ASLR) with the following command  to record traces.

```
echo 0 > /proc/sys/kernel/randomize_va_space 
```

The pintool used can be found in tools/pintool.


The ChampSim with 4KB or 2MB page size can be compiled with the following method:

```
./config.sh ./champsim_config.json
rm bin/champsim.*
sed -i "s/PAGE_SIZE 4096u/PAGE_SIZE 2097152u/g" inc/champsim_constants.h
make -j8
cp bin/champsim bin/champsim.2m
sed -i "s/PAGE_SIZE 2097152u/PAGE_SIZE 4096u/g" inc/champsim_constants.h
make -j8
cp bin/champsim bin/champsim.4k

```

* TLB-4KB:

```
champsim.4k  -c 8M -h 4G -m 64G -s 1G -i 1000000000 -traces test.trace
```

* TLB-2MB

```
champsim.2m  -c 8M -h 4G -m 64G -s 1G -i 1000000000 -traces test.trace

```

* IdealRMM

```
champsim.4k  -c 8M -h 4G -m 64G -s 1G -rmm -i 1000000000 -traces test.trace
```

* IdealDS

```
champsim.4k  -c 8M -h 4G -m 64G -s 1G -direct_segment -i 1000000000 -traces test.trace

```

* SMM-4K

```
champsim.4k  -c 8M -h 4G -m 64G -s 1G -smm -i 1000000000 -traces test.trace

```

* SMM-2M

```
champsim.2m  -c 8M -h 4G -m 64G -s 1G -smm -i 1000000000 -traces test.trace

```



# SMM Kernel Setup

The current implementation only support X86_64 for now. Make sure to select kernel config as follows:


```
CONFIG_TRANSPARENT_SEGMENTPAGE=y
```

The current SMM kernel should booted with the following command line.

```
norandmaps smm_reserve=40g vdso=0
```

The above command line will reserve 40GB  memory for SMM segmentation usage.

For convenience, we currently disable the randmaps (ASLR) and vdso functions.

For test SMM application and kernel  on common X86_64 platform, follow the instructions in tools/micro_pgfault and tools/smm_elf_hack.

