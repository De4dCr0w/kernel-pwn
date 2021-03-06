gdb -n \
    -ex "add-auto-load-safe-path $(pwd)" \
    -ex "file bzImage" \
    -ex 'set arch i386:x86-64' \
    -ex 'target remote localhost:1234' \
    -ex 'continue' \
    -ex 'add-symbol-file ./hackme.ko 0xffffffffc0000000' \
    -ex 'b hackme_ioctl' \
    -ex 'layout split' \
    -ex 'layout regs' \
    -ex 'focus cmd' \
    -ex 'continue'
