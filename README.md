# ClosureOS

The `ClosureOS` operating system, compatible with [Multiboot2 Specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html).

[![DIscord](https://img.shields.io/discord/1285336523692183592?style=for-the-badge&color=%237289DA&label=Discord&logo=discord&logoColor=white)](https://discord.gg/kdstJHFsKV) [![QQ Group Invitation](https://img.shields.io/badge/QQ_GROUP-533452711-1DA1F2?style=for-the-badge&logo=linux&logoColor=white)](https://qm.qq.com/q/nnEDYChIUE)

## Getting started

Our scripts under `scripts/` will help you check for missing tool chains in your computer before running, so you don't need to check for it manually.

### Build the ClosureOS

Simply, you can use this script to build an image for local test using QEMU.

```shell
$ ./scripts/repack_iso.sh
```

### Run the ClosureOS

We defaultly recommend you to use QEMU to run it, just run this script after building:

```shell
$ ./scripts/boot_qemu.sh
```

By the way, you can also run it on your physical machine. Currently we don't have an installer yet, so you need to pack it on your own. All you need to do is just to install GRUB on your device, and put the kernel binary file in your expected path on the disk, then specify the kernel path in yout GRUB configuration file. We have an example GRUB configuration file under the `targets/<arch>/iso/boot/grub/grub.cfg` for your reference.

## Community

[![DIscord](https://img.shields.io/discord/1285336523692183592?style=for-the-badge&color=%237289DA&label=Discord&logo=discord&logoColor=white)](https://discord.gg/kdstJHFsKV)

[![QQ Group Invitation](https://img.shields.io/badge/QQ_GROUP-533452711-1DA1F2?style=for-the-badge&logo=linux&logoColor=white)](https://qm.qq.com/q/nnEDYChIUE)

## Devemoplemt Details

You can refer to the following passages to see how it has been developed. Currently we only have Chinese version temporarily, and the English version is under hard writing and will be released soon...

- [现代 64 位 OS 开发手记 I：UEFI 启动与 GRUB 引导](https://arttnba3.github.io/2023/11/29/CODE-0X03-OSDEV64-I_UEFI-GRUB/)
- [现代 64 位 OS 开发手记 II：内核内存分配器与 C++ 的初步引入](https://arttnba3.github.io/2024/06/30/CODE-0X04-OSDEV64-II_MEMORY-MANAGE/)

## Author

- arttnba3 <arttnba3@outlook.com>

## License

This project is licensed under the GPL v2 License.

You may obtain a copy of the License at [https://opensource.org/license/gpl-2-0](https://opensource.org/license/gpl-2-0).
