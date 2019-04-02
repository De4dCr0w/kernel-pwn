qemu-system-x86_64 \
-m 256M \
-kernel ./bzImage \
-initrd  ./core_new.cpio \
-append "root=/dev/ram rw console=ttyS0 oops=panic panic=1 quiet kaslr useradd p4nda" \
-nographic \
-netdev user,id=t0, -device e1000,netdev=t0,id=nic0 \
-gdb tcp::1234 -S \
#-enable-kvm -monitor /dev/null -m 128M --nographic  -smp cores=1,threads=1 -cpu kvm64,+smep -s \
