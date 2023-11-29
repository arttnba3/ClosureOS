# Author: arttnba3
# 2023.11

boot_dir = boot
X86_INCLUDE = $(shell pwd)/include/

src_asm = $(wildcard *.S)
src_asm += $(wildcard $(boot_dir)/*.S)

src_c = $(wildcard *.c)
src_c += $(wildcard $(boot_dir)/*.c)

src_files = $(src_asm)
src_files += $(src_c)

obj_files = $(notdir $(patsubst %S, %o, $(src_asm)))
obj_files += $(notdir $(patsubst %c, %o, $(src_c)))
obj_files += boot_font.o

all: $(src_files)
	$(CC) $(CFLAGS) -c $(src_files) -I $(INCLUDE) -I $(X86_INCLUDE)
	objcopy -O elf64-x86-64 -B i386 -I binary $(boot_dir)/font.psf boot_font.o
	mv $(obj_files) -t $(ARCH_OUT)
