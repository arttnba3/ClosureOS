#!/bin/sh
qemu-system-x86_64 \
	-cpu kvm64,+smep,+smap \
	-smp sockets=1,dies=1,cores=4,threads=2 \
	-m 1G \
	--machine q35 \
	-drive file=./kernel.iso,format=raw,index=0,media=disk \
	-s \
	-bios /usr/share/ovmf/OVMF.fd

