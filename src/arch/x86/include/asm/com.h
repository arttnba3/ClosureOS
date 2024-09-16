/* SPDX-License-Identifier: GPL-2.0 */

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

/* Line Control Register bits */

/* Divisor Latch Access Bit */
#define COM_LCR_DLAB    7
    #define COM_LCR_DLAB_ON     (1 << COM_LCR_DLAB)
    #define COM_LCR_DLAB_OFF    (0 << COM_LCR_DLAB)
/* Break Enable Bit */
#define COM_LCR_BEB     6
/* Parity Bits (3 bits) */
#define COM_LCR_PB      3
    #define COM_LCR_PB_NO       (0b000 << COM_LCR_PB)
    #define COM_LCR_PB_ODD      (0b001 << COM_LCR_PB)
    #define COM_LCR_PB_EVEN     (0b011 << COM_LCR_PB)
    #define COM_LCR_PB_HIGH     (0b101 << COM_LCR_PB)
    #define COM_LCR_PB_LOW      (0b111 << COM_LCR_PB)
/* Stop Bits */
#define COM_LCR_SB      2
    #define COM_LCR_SB_ONE  (0 << COM_LCR_SB)
    #define COM_LCR_SB_TWO  (1 << COM_LCR_SB)
/* Data Bits (2 bits)*/
#define COM_LCR_DB      0
    #define COM_LCR_DB_WLEN_5   (0b00 << COM_LCR_DB)
    #define COM_LCR_DB_WLEN_6   (0b01 << COM_LCR_DB)
    #define COM_LCR_DB_WLEN_7   (0b10 << COM_LCR_DB)
    #define COM_LCR_DB_WLEN_8   (0b11 << COM_LCR_DB)

/* First In First Out Control Register bits */

/* Interrupt Trigger Level bit (2 bits)*/
#define COM_FCR_ITLB     6
    #define COM_FCR_ITLB_TL_1    (0b00 << COM_FCR_ITLB)
    #define COM_FCR_ITLB_TL_4    (0b01 << COM_FCR_ITLB)
    #define COM_FCR_ITLB_TL_8    (0b10 << COM_FCR_ITLB)
    #define COM_FCR_ITLB_TL_14   (0b11 << COM_FCR_ITLB)
/* Bit 5~4 are Reserved */
/* DMA Mode Select bit */
#define COM_FCR_DMSB     3
/* Clear Transmit FIFO bit */
#define COM_FCR_CTFB     2
    #define COM_FCR_CTFB_ON (1 << COM_FCR_CTFB)
/* Clear Receive FIFO bit */
#define COM_FCR_CRFB     1
    #define COM_FCR_CRFB_ON  (1 << COM_FCR_CRFB)
/* Enable FIFO bit */
#define COM_FCR_EFB      0
    #define COM_FCR_EFB_ON  (1 << COM_FCR_EFB)

/* Modem Control Register bits */

/* Bit 7~5 are Reserved */
/* Loop bit */
#define COM_MCR_LB      4
    #define COM_MCR_LB_ON   (1 << COM_MCR_LB)
/* Out 2 bit */
#define COM_MCR_OUT2    3
    #define COM_MCR_OUT2_ON (1 << COM_MCR_OUT2) /* for enabling IRQ */
/* Out 1 bit */
#define COM_MCR_OUT1    2
    #define COM_MCR_OUT1_ON (1 << COM_MCR_OUT1) /* useless? */
/* Request To Send bit */
#define COM_MCR_RTSB    1
    #define COM_MCR_RTSB_ON (1 << COM_MCR_RTSB)
/* Data Terminal Ready bit */
#define COM_MCR_DTRB    0
    #define COM_MCR_DTRB_ON (1 << COM_MCR_DTRB)

#endif // X86_ASM_COM_H
