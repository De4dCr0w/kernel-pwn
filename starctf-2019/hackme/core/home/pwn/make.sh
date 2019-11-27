#!/bin/bash

gcc -w pwn.c -o pwn -static
#gcc -w pwn.c -o pwn -static
./fs.sh 
./../startvm.sh
