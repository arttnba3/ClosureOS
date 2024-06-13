#ifndef X86_ASM_COM_H
#define X86_ASM_COM_H

/* register base for serial ports (COMs)  */

#define COM1_BASE 0x3F8
#define COM2_BASE 0x2F8

/* register offsets for serial ports (COMs) */

/* Read: Receivbe buffer */
#define COM_REG_TX      0
/* Write: Transmit Buffer */
#define COM_REG_RX      0
/* Read/Write: Interrupt Enable Register */
#define COM_REG_IER     1
/* (DLAB=1) Read/Write: Divisor Latch LSB Register */
#define COM_REG_DLL     0
/* (DLAB=1) Read/Write: Divisor Latch MSB Register */
#define COM_REG_DLM     1
/* Read： Interrupt Identification */
#define COM_REG_IIR     2
/* Write: FIFO Control Register */
#define COM_REG_FCR     2
/* Write: Line Control Register */
#define COM_REG_LCR     3
/* Read/Write: Model Control Register */
#define COM_REG_MCR     4
/* Read: Line Status Register */
#define COM_REG_LSR     5
/* Read: Modem Status Register */
#define COM_REG_MSR     6
/* Read/Write: Scratch Register */
#define COM_REG_SR      7

#endif // X86_ASM_COM_H
