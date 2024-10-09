OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS
{
    /* boot loader will load the kernel there */
    . = 1M;
	__boot_start = .;

	/* ASM boot-state kernel */

    .boot.loader :
	{
		KEEP(*(.boot.header))
		*(.boot.*)
		arch/x86/boot/libBoot.a
	}

	. = ALIGN(4096);

	/* C boot-state kernel */

	.boot.text ALIGN(4096) :
	{
		arch/x86/boot/libBoot.a(.text)
	}

	. = ALIGN(4096);

	.boot.rodata ALIGN(4096) :
	{
		arch/x86/boot/libBoot.a(.rodata)
	}

	. = ALIGN(4096);

	.boot.data ALIGN(4096) :
	{
		arch/x86/boot/libBoot.a(.data)
		arch/x86/boot/libBoot.a(.*)
	}

	. = ALIGN(4096);

	__boot_end = .;

	/* now we come to the REAL kernel */

	KERN_ELF_BASE = 0xFFFFFF8000000000;
	. = KERN_ELF_BASE;
	__kernel_start = .;

	/* we use AT() there to make it loaded on phys correctly */

	KERN_TEXT_BASE = KERN_ELF_BASE;
	. = KERN_TEXT_BASE;
	__kernel_text_base = .;

	.text ALIGN(4096) : AT (__kernel_text_base - __kernel_start + __boot_end)
	{
		*(.text)
		*(.text.*)
		*(.ltext)
		*(.ltext.*)
	}

	. = ALIGN(4096);

	__kernel_text_end = .;
	__kernel_text_sz = __kernel_text_end - __kernel_text_base;

	KERN_DATA_BASE = 0xFFFFFF8010000000;
	. = KERN_DATA_BASE;
	__kernel_data_base = .;

	.data ALIGN(4096) : AT (__kernel_text_sz + __boot_end)
	{
		*(.data)
		*(.ldata)

		/* we have to set these arraies aligned to 8 bytes */
		. = ALIGN(8);

		__init_array = .;
		*(.init_array)
		__init_array_end = .;
		__fini_array = .;
		*(.fini_array)
	}

	. = ALIGN(4096);

	__kernel_data_end = .;
	__kernel_data_sz = __kernel_data_end - __kernel_data_base;

	KERN_RODATA_BASE = 0xFFFFFF8020000000;
	. = KERN_RODATA_BASE;
	__kernel_rodata_base = .;

	.rodata ALIGN(4096) : AT (__kernel_data_sz + __kernel_text_sz + __boot_end)
	{
		*(.rodata)
		*(.rodata.*)
	}

	. = ALIGN(4096);

	__kernel_rodata_end = .;
	__kernel_rodata_sz = __kernel_rodata_end - __kernel_rodata_base;

	KERN_BSS_BASE = 0xFFFFFF8030000000;
	. = KERN_BSS_BASE;
	__kernel_bss_base = .;

	.bss ALIGN(4096) : AT (__kernel_rodata_sz + __kernel_data_sz + __kernel_text_sz + __boot_end)
	{
		*(COMMON)
		*(.bss)
		*(.lbss)
	}

	. = ALIGN(4096);

	__kernel_bss_end = .;
	__kernel_bss_sz = __kernel_bss_end - __kernel_bss_base;

	. = ALIGN(4096);

	__kernel_end = .;
}
