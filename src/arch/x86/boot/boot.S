#define ASM_FILE 1
#include <boot/multiboot2.h>
#include <asm/cpu_types.h>
#include <asm/page_types.h>

#define GRUB_MULTIBOOT_ARCHITECTURE_I386 (0)
#define MULTIBOOT2_HEADER_LEN (multiboot_header_end - multiboot_header)
#define MULTIBOOT2_HEADER_CHECKSUM \
    -(MULTIBOOT2_HEADER_MAGIC \
    + GRUB_MULTIBOOT_ARCHITECTURE_I386 \
    + MULTIBOOT2_HEADER_LEN)

.section .boot.loader.header
    .align 8

    multiboot_header:
        .long   MULTIBOOT2_HEADER_MAGIC
        .long   GRUB_MULTIBOOT_ARCHITECTURE_I386
        .long   MULTIBOOT2_HEADER_LEN
        .long   MULTIBOOT2_HEADER_CHECKSUM

    tag_entry:
        .align 8
        .short MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS
        .short 0
        .long 12
        .long _start
    
    tag_frame_buffer:
        .align 8
        .short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
        .short 0
        .long 20
        .long 1024
        .long 768
        .long 32

    tags_end:
        .align 8
        .short  MULTIBOOT_HEADER_TAG_END
        .short  0
        .long   8
    multiboot_header_end:

.section .boot.loader.text32:
    .code32
    .align 0x1000
    .extern boot_main

    .globl _start

    _start:
        # turn off the interrupt temporarily,
        # and we should turn it on after our own IDT has been built.
        cli

        # check for multiboot2 header
        cmp $MULTIBOOT2_BOOTLOADER_MAGIC, %eax
        jne .loop

        # temporary stack
        mov     $boot_stack_top, %esp

        # set the boot information as parameters for boot_main()
        mov     %eax, %edi
        mov     %ebx, %esi

        # clear eflags
        pushl   $0
        popf

        # disable paging (UEFI may turn it on)
        mov     %cr0, %eax
        mov     $CR0_PG, %ebx
        not     %ebx
        and     %ebx, %eax
        mov     %eax, %cr0

        # set up page table for booting stage
        # it's okay to write only 32bit here :)
        mov     $boot_pud, %eax
        or      $(PAGE_ATTR_P | PAGE_ATTR_RW), %eax
        mov     %eax, boot_pgd

        xor     %eax, %eax
        or      $(PAGE_ATTR_P | PAGE_ATTR_RW | PAGE_ATTR_PS), %eax
        movl    %eax, boot_pud

        xor     %eax, %eax
        mov     %eax, (boot_pgd + 4)
        mov     %eax, (boot_pud + 4)

        # load page table
        mov     $boot_pgd, %eax
        mov     %eax, %cr3

        # enable PAE and PGE
        mov     %cr4, %eax
        or      $(CR4_PAE | CR4_PGE), %eax
        mov     %eax, %cr4

        # enter long mode by enabling EFER.LME
        mov     $0xC0000080, %ecx
        rdmsr
        or      $(1 << 8), %eax
        wrmsr

        # enable paging
        mov     %cr0, %eax
        or      $CR0_PG, %eax
        mov     %eax, %cr0

        # set up GDT
        mov     $gdt64_ptr, %eax
        lgdt    0(%eax)

        # reload all the segment registers
        mov $(2 << SELECTOR_INDEX), %ax
        mov     %ax, %ds
        mov     %ax, %ss
        mov     %ax, %es
        mov     %ax, %fs
        mov     %ax, %gs

        # enter the 64-bit world within a long jmp
        jmp $(1 << SELECTOR_INDEX), $boot_main

        # we shouldn't get here...
    .loop:
        hlt
        jmp .loop

.section .boot.loader.data
    .align 0x1000

    .globl boot_stack, boot_stack_top, boot_pgd, boot_pud

    #
    # When the system is booted under legacy BIOS, there's no stack
    # So we reserve a page there as a temporary stack for booting
    #
    boot_stack:
        .space 0x1000
    boot_stack_top:

    boot_pgd:
        .space 0x1000
    boot_pud:
        .space 0x1000

    # global segment descriptor table
    .align 0x1000   # it should be aligned to page
    .globl gdt64, gdt64_ptr
    gdt64:
        .quad 0 # first one must be zero
    gdt64_code_segment:
        .quad 0x00209A0000000000 # exec/read
    gdt64_data_segment:
        .quad 0x0000920000000000 # read/write
    gdt64_ptr:
        .short gdt64_ptr - gdt64 - 1    # GDT limit
        .long gdt64                     # GDT Addr
