#! /bin/sh
cd `dirname $0`
#stty intr ^]
qemu-system-x86_64 \
    -m 256M \
    -nographic \
    -kernel bzImage \
    -append 'console=ttyS0 loglevel=3 oops=panic panic=1 nokaslr' \
    -monitor /dev/null \
    -initrd ./core.cpio \
    -smp cores=4,threads=2 \
    -cpu qemu64,smep,smap 2>/dev/null \
    #-gdb tcp::1234 -S \
    #-initrd initramfs.cpio \

