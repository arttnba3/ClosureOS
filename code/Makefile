# Author: arttnba3
# 2023.11

CC	:= $(GCCPREFIX)gcc
AS	:= $(GCCPREFIX)as
AR	:= $(GCCPREFIX)ar
LD	:= $(GCCPREFIX)ld
CFLAGS := -pipe -ffreestanding -nostdlib -fno-pie -fno-stack-protector -mcmodel=large
LDFLAGS := -nostdlib -z max-page-size=0x1000

export CC
export AS
export AR
export LD
export CFLAGS

INCLUDE := $(shell pwd)/include/
ARCH := $(shell uname -m)
OUT := $(shell pwd)/out/
ARCH_OUT := $(shell pwd)/out/arch/

export INCLUDE
export ARCH
export OUT
export ARCH_OUT

all: prepare kernel.bin

prepare:
	mkdir -p $(OUT)
	mkdir -p $(ARCH_OUT)

kernel.bin: kernel_file arch_file
	$(LD) -T linker.ld -o $(OUT)/$@ $(shell find $(OUT) -name *.o)

kernel_file:
	make -C kernel/

arch_file:
	make -C arch/

clean:
	rm -rf ./out

.PHONY: all
