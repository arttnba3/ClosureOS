#!/bin/bash
#
# Copyright (C) 2024 arttnba3
#
# This script is licensed under the GPL v2 License.
# You may obtain a copy of the License at https://opensource.org/license/gpl-2-0
#
# Description: This script helps user run the QEMU to have a test locally
# Author: arttnba3
# Version: 1.0.0
# Date: 2024-06-04
#

export CPU_PARAM="kvm64,+smep,+smap"
export SMP_PARAM="sockets=1,dies=1,cores=4,threads=2"
export MEM_PARAM="256M"
export GDB_PARAM="tcp:localhost:1234"
export MACHINE_PARAM="q35"
export OTHER_PARAMS=""

export DISK_PATH=""
export BIOS_EFI_PATH=""
export QEMU_PATH="qemu-system-x86_64"

function show_help() {

	cat << EOF
Usage: run.sh [options]

Available options:
	General:
		-h|--help		show this message
		-c|--cpu		cpu param for QEMU VM
						(default: "kvm64,+smep,+smap")
		-s|--smp		smp param for QEMU VM
						(default: "sockets=1,dies=1,cores=4,threads=2")
		-m|--mem		memory param for QEMU VM
						(default: "1G")
		-g|--gdb		gdb param for QEMU VM, no debugger if it's set as "none"
						(default: "tcp:localhost:1234")
		-e|--machine		machine architecture param for QEMU VM
						(default: "q35")
		-d|--disk		disk path for QEMU VM
						(default: "\$PWD/kernel.iso")
		-b|--bios		bios/efi path for QEMU VM, default by qemu if it's set as "default"
						(our default: automatical selection, EFI first if available)
		-q|--qemu		qemu path for QEMU VM
						(default: "qemu-system-x86_64")
		-o|--other		other custom params for QEMU VM
						(default: "")

Please note that all redundant arguments will be ignored.
EOF

}

function parse_cmd() {

	while [[ $# -gt 0 ]]; do
		option="$1"
		case "$1" in
			-h|--help)
				show_help
				exit 0
				;;
			-c|--cpu)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				CPU_PARAM="$1"
				shift
				;;
			-s|--smp)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				SMP_PARAM="$1"
				shift
				;;
			-m|--mem)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				MEM_PARAM="$1"
				shift
				;;
			-g|--gdb)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				GDB_PARAM="$1"
				shift
				;;
			-e|--machine)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				MACHINE_PARAM="$1"
				shift
				;;
			-d|--disk)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				DISK_PATH="$1"
				shift
				;;
			-b|--bios)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				BIOS_EFI_PATH="$1"
				shift
				;;
			-q|--qemu)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				QEMU_PATH="$1"
				shift
				;;
			-o|--other)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				OTHER_PARAMS="$1"
				shift
				;;
			*)
				echo "[*] Warning: Unknown argument $1"
				shift
				;;
		esac
	done

}

function env_check() {

	if which $QEMU_PATH &>/dev/null ; then
		echo "[*] Using QEMU: \"$QEMU_PATH\""
	else
		echo "[x] QEMU \"$QEMU_PATH\" not existed!"
		exit 1
	fi

	if [ -z "$DISK_PATH" ] ; then
		DISK_PATH="$PWD/kernel.iso"
	fi

	if [ -e "$DISK_PATH" ]; then
		echo "[*] Using disk file: \"$DISK_PATH\""
	else
		echo "[x] Disk file \"$DISK_PATH\" not existed!"
		exit 1
	fi

}

declare -a bios_efi_dirs=("/usr/share/qemu/ovmf-x86_64.bin" "/usr/share/ovmf/OVMF.fd")

function bios_check() {

	if [[ "$BIOS_EFI_PATH" == "default" ]] ; then
		echo "[*] Using default SEABIOS."
		BIOS_EFI_PATH=""
		return
	elif [ -n "$BIOS_EFI_PATH" ] ; then
		echo "[*] Using custom BIOS/EFI path: \"$BIOS_EFI_PATH\""
		return
	fi

	for path in "${bios_efi_dirs[@]}"
	do
		if [ -e "$path" ] ; then
			BIOS_EFI_PATH=$path
			break
		fi
	done

	if [ -z "$BIOS_EFI_PATH" ] ; then
		echo "[*] Using default SEABIOS."
	else
		echo "[*] Using BIOS/EFI: \"$BIOS_EFI_PATH\""
	fi

}

function gen_param() {

	# cpu
	QEMU_PARAM="${QEMU_PARAM} -cpu ${CPU_PARAM}"

	# smp
	QEMU_PARAM="${QEMU_PARAM} -smp ${SMP_PARAM}"

	# mem
	QEMU_PARAM="${QEMU_PARAM} -m ${MEM_PARAM}"

	# gdb
	if [[ "$GDB_PARAM" != "none" ]]; then
		QEMU_PARAM="${QEMU_PARAM} -gdb ${GDB_PARAM}"
	fi

	# machine
	QEMU_PARAM="${QEMU_PARAM} --machine ${MACHINE_PARAM}"

	# disk
	QEMU_PARAM="${QEMU_PARAM} -drive file=${DISK_PATH},format=raw,index=0,media=disk"

	# bios
	if [[ -n "$BIOS_EFI_PATH" ]]; then
		QEMU_PARAM="${QEMU_PARAM} -bios ${BIOS_EFI_PATH}"
	fi

	# other
	if [[ -n "$OTHER_PARAMS" ]]; then
		echo "[*] Using extra params: \"$OTHER_PARAMS\""
		QEMU_PARAM="${QEMU_PARAM} ${OTHER_PARAMS}"
	fi

}

function run_qemu() {

	$QEMU_PATH $QEMU_PARAM

}

function main() {

	parse_cmd "$@"

	env_check

	bios_check

	gen_param

	run_qemu

}

main "$@"
