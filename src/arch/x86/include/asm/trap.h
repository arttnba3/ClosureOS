#ifndef X86_ASM_TRAP_H
#define X86_ASM_TRAP_H

/* traps & exceptions */
#define TRAP_DE     0   /* divided by zero */
#define TRAP_DB     1   /* debug */
#define TRAP_NMI    2   /* non-maskable interrupt */
#define TRAP_BP     3   /* breakpoint */
#define TRAP_OF     4   /* overflow */
#define TRAP_BR     5   /* bound range exceeded */
#define TRAP_UD     6   /* invalid opcode */
#define TRAP_NM     7   /* device not available */
#define TRAP_DF     8   /* double fault */
#define TRAP_OLD_MF 9   /* co-processor segment overrun */
#define TRAP_TS     10  /* invalid TSS */
#define TRAP_NP     11  /* segment not present */
#define TRAP_SS     12  /* stack segment fault */
#define TRAP_GP     13  /* general protection */
#define TRAP_PF     14  /* page fault */
#define TRAP_NON_1  15  /* reserve */
#define TRAP_MF     16  /* floating-point error */
#define TRAP_AC     17  /* alignment check */
#define TRAP_MC     18  /* machine check */
#define TRAP_XM     19  /* SIMD floating-point exception */
#define TRAP_VE     20  /* vivrtualization exception */
#define TRAP_CP     21  /* control protection exception */
/* 22-31 reserved */
/* 32-255 maskable interrupts */

#endif // X86_ASM_TRAP_H
