#!/bin/sh
gdb \
	-ex "target remote localhost:1234" \
	-ex "continue"\
	-ex "disconnect"\
	-ex "set arch i386:x86-64:intel" \
	-ex "target remote:1234" \
	-ex "add-symbol-file ./core/baby.ko 0xffffffffc0002000"\
