# ClosureOS

Another 64-bit OS satisfied Multiboot2 Spec.

Author: arttnba3

## Introduction

You can refer to the following passages to see how it has been developed:

[现代 64 位 OS 开发手记 I：UEFI 启动与 GRUB 引导](https://arttnba3.github.io/2023/11/29/CODE-0X03-OSDEV64-I_UEFI-GRUB/)

[现代 64 位 OS 开发手记 II：内核内存分配器与 C++ 的初步引入](https://arttnba3.github.io/2024/06/30/CODE-0X04-OSDEV64-II_MEMORY-MANAGE/)

## Build

You can use this script to build an image for local test using QEMU.

```shell
$ ./tools/scripts/repack_iso.sh
```

## Run

We defaultly recommend you to use QEMU to run it, just run this script after building:

```shell
$ ./tools/scripts/run.sh
```

By the way, you can also run it on your physical machine. Currently we don't have an installer yet, so you need to pack it on your own. All you need to do is just to install GRUB on your device, and put the kernel binary file in your expected path on the disk, then specify the kernel path in yout GRUB configuration file. We have an example GRUB configuration file under the `targets/<arch>/iso/boot/grub/grub.cfg` for your reference.

## License

GPL v2
