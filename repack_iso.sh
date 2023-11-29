#!/bin/sh
cd code
make clean
make || exit
cd ..
mv code/out/kernel.bin targets/x86_64/iso/boot
grub-mkrescue /usr/lib/grub/x86_64-efi -o kernel.iso targets/x86_64/iso
