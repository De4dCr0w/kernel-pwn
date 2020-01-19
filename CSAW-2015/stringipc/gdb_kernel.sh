gdb -n \
    -ex "add-auto-load-safe-path $(pwd)" \
    -ex "file vmlinux" \
    -ex 'set arch i386:x86-64:intel' \
    -ex 'target remote localhost:1234' \
    -ex 'continue' \
    -ex 'disconnect' \
    -ex 'set arch i386:x86-64' \
    -ex 'target remote localhost:1234'\
    -ex 'add-symbol-file ./core/stringipc.ko 0xffffffffc0000000' \
    -ex 'b write_ipc_channel' \
    -ex 'continue' \

    #-ex 'b realloc_ipc_channel' \
    #-ex 'b seek_ipc_channel' \
    #-ex 'b csaw_ioctl' \
