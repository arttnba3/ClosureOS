#!/bin/bash
#
# Copyright (C) 2024 arttnba3
#
# This script is licensed under the GPL v2 License.
# You may obtain a copy of the License at https://opensource.org/license/gpl-2-0
#
# Description: This script helps user compile the code and pack the kernel into an iso containing grub
# Author: arttnba3
# Version: 1.0.0
# Date: 2024-06-04
#

export PROJECT_ROOT_DIR="$PWD"
export SOURCE_CODE_DIR="$PROJECT_ROOT_DIR/src"
export BUILD_DIR="$PROJECT_ROOT_DIR/build"
export TARGET_DIR="$PROJECT_ROOT_DIR/targets/x86_64/iso"
export IMAGE_OUTPUT_DIR="$PROJECT_ROOT_DIR"

function show_help() {

	cat << EOF
Usage: repack_iso.sh [options]

Available options:

	-h|--help		show this message
	-r|--root		root directory of the project
					(default: "\$PWD")
	-s|--source		source code of the project
					(default: "\$PWD/src")
	-b|--build		directory used for building
					(default: "\$PWD/build")
	-t|--target		directory for grub-rescue to pack
					(default: "\$PWD/targets/x86_64/iso")
	-i|--image_dir		directory for filesystem image output
					(default: "\$PWD")

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
			-r|--root)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				PROJECT_ROOT_DIR="$1"
				shift
				;;
			-s|--source)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				SOURCE_CODE_DIR="$1"
				shift
				;;
			-b|--build)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				BUILD_DIR="$1"
				shift
				;;
			-i|--image_dir)
				shift
				if [[ -z "$1" ]]; then
					echo "[x] Error: Missing argument for $option"
					exit 1
				fi
				IMAGE_OUTPUT_DIR="$1"
				shift
				;;
			*)
				echo "[*] Warning: Unknown argument $1"
				shift
				;;
		esac
	done

}

declare -a needed_programs=("git" "cmake" "ninja" "clang" "clang++" "ld" "ar" "ranlib" "xorriso" "mformat")
declare -a grub_rescue_programs=("grub-mkrescue" "grub2-mkrescue")
declare -a grub_efi_dirs=("/usr/share/grub2/x86_64-efi" "/usr/lib/grub/x86_64-efi")
declare -a grub_bios_dirs=("/usr/share/grub2/i386-pc" "/usr/lib/grub/i386-pc")

export DEFAULT_GRUB_RESCUE=""
export DEFAULT_GRUB_DATA_DIR=""

function env_check {

	for program in "${needed_programs[@]}"
	do
		if ! which $program &>/dev/null ; then
			echo "[x] program \"$program\" needed for building the project not found."
			exit 1
		fi
	done

	for program in "${grub_rescue_programs[@]}"
	do
		if which $program &>/dev/null ; then
			DEFAULT_GRUB_RESCUE=$program
			break
		fi
	done

	if [ -z "$DEFAULT_GRUB_RESCUE" ] ; then
		echo "[x] No grub tools found on the computer!"
		exit 1
	fi

	for dir in "${grub_efi_dirs[@]}"
	do
		if [ -x "$dir" ] ; then
			DEFAULT_GRUB_DATA_DIR=$dir
			break
		elif [ -d "$dir" ] ; then 
			echo "[!] Warn: Grub EFI directory \"$dir\" found but could not be accessed"
		fi
	done

	if [ -z "$DEFAULT_GRUB_DATA_DIR" ] ; then
		echo "[!] No grub efi data available found. Try to use legacy bios..."

		for dir in "${grub_bios_dirs[@]}"
		do
			if [ -x "$dir" ] ; then
				DEFAULT_GRUB_DATA_DIR=$dir
				break
			elif [ -d "$dir" ] ; then 
				echo "[!] Warn: Grub BIOS directory \"$dir\" found but could not be accessed"
			fi
		done
	fi

	if [ -z "$DEFAULT_GRUB_DATA_DIR" ] ; then
		echo "[x] No grub data available found!"
		exit 1
	fi

}

function build_project() {

	rm -rf $BUILD_DIR
	mkdir $BUILD_DIR
	cd $BUILD_DIR
	cmake $SOURCE_CODE_DIR \
		-DCMAKE_C_COMPILER=clang \
		-DCMAKE_CXX_COMPILER=clang++ \
		-G Ninja
	ninja

}

function pack_iso() {

	mv $BUILD_DIR/kernel.bin $TARGET_DIR/boot/
	$DEFAULT_GRUB_RESCUE $DEFAULT_GRUB_DATA_DIR -o $IMAGE_OUTPUT_DIR/kernel.iso $TARGET_DIR

}

function main() {
	trap 'echo "[x] Failed to build the project, failed at command \"${BASH_COMMAND}\""; exit 1' ERR

	env_check

	parse_cmd "$@"

	build_project

	pack_iso

}

main "$@"
