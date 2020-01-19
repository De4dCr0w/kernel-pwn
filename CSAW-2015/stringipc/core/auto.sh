#!/bin/bash
#gcc ./myexp/pwn_task_struct.c -o ./myexp/pwn_task_struct -static
#gcc ./myexp/pwn_vdso.c -o ./myexp/pwn_vdso -static
gcc ./myexp/pwn_prctl.c -o ./myexp/pwn_prctl -static
#gcc pwn_task_struct.c -o pwn_task_struct -static
./fs.sh
