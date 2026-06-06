#!/usr/bin/env bash
set -e

make
qemu-system-i386 -drive file=build/main_floppy.img,if=floppy,format=raw -boot a -display gtk,gl=off
