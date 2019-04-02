#!/bin/sh
find . | cpio -o --format=newc > ../core_new.cpio

