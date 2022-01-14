#!/bin/bash

# SMM-4K 
t=$(SMM_CODE=2m SMM_STACK=8m SMM_HEAP=1g SMM_MMAP=11g ./a.out | grep 'Total Time' | awk '{print $3}')
echo "4K $t"

# SMM-2M 
t=$(SMM_CODE=2m SMM_STACK=8m SMM_HEAP=1g SMM_MMAP=11g SMM_PMD=1 ./a.out | grep 'Total Time' | awk '{print $3}')
echo "SMM-2M $t"

# SMM-1G 
t=$(SMM_CODE=2m SMM_STACK=8m SMM_HEAP=1g SMM_MMAP=11g SMM_PMD=1 SMM_PUD=1 ./a.out | grep 'Total Time' | awk '{print $3}')
echo "SMM-1G $t"
