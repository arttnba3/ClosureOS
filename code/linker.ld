OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS
{
    /* boot loader will load the kernel there */
    . = 1M;
	__boot_start = .;

    .boot :
	{
		KEEP(*(.boot.header))
		*(.boot.*)
		. = ALIGN(4096);

		/* C part boot kernel */
		*arch/boot*.o
	}

	__boot_end = .;

	/* now it comes to the real kernel */
	KERM_VADDR = 0xffffffff81000000;
	. = KERM_VADDR;

	/* we use AT() there to make it loaded on phys correctly */
	.text ALIGN(4096) : AT (ADDR (.text) - KERM_VADDR)
	{
		*(.text)
	}

	. = ALIGN(4096);

	.rodata ALIGN(4096) : AT (ADDR (.rodata) - KERM_VADDR)
	{
		*(.rodata)
	}

	. = ALIGN(4096);

	__roseg_end = .;

	.data ALIGN(4096) : AT (ADDR (.data) - KERM_VADDR)
	{
		*(.data)
	}

	. = ALIGN(4096);

	.bss ALIGN(4096) : AT (ADDR (.bss) - KERM_VADDR)
	{
		*(COMMON)
		*(.bss)
	}

	. = ALIGN(4096);

	__kernel_end = .;
}