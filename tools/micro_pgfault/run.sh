#!/bin/bash

# 4K 
t=$(./a.out | grep 'Total Time' | awk '{print $3}')
echo "4K $t"

# THP-2M 
t=$(./a.out -t | grep 'Total Time' | awk '{print $3}')
echo "THP-2M $t"
