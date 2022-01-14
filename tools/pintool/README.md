# Pin to use with ChampSim

We use pin3.17 on ubuntu 20.04 platform to record traces.

Suppose your pin dir is "~/pin-3.17", the following command line shows recording a "test-tlb" benchmark trace of 1002000000 instructions.
The trace is saved as file name tlb.512KM256B.1B.trace.



```
sudo ~/pin-3.17/pin -pid $(pidof test-tlb) -t ~/pin-3.17/tracer/obj-intel64/champsim_tracer.so -t 1002000000 -o tlb.512KM256B.1B.trace
```
