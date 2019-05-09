#!/bin/bash

gcc -w exp.c -o exp -static
gcc -w pwn.c -o pwn -static
./fs.sh 
./../startvm.sh
