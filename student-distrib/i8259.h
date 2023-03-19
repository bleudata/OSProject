/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */


// referenced osdev.org PIC 
#ifndef _I8259_H
#define _I8259_H

#include "types.h"
#include "lib.h"


/* Ports that each PIC sits on */
#define MASTER_8259_PORT    0x20 //command
#define SLAVE_8259_PORT     0xA0 //command
#define PIC1_DATA   MASTER_8259_PORT+1
#define PIC2_DATA   SLAVE_8259_PORT+1

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11
#define ICW2_MASTER         0x20
#define ICW2_SLAVE          0x28
#define ICW3_MASTER         0x04
#define ICW3_SLAVE          0x02
#define ICW4                0x01

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60
// noelle says some fix was changing this to 0x20??? bit aditya says no don't change it

#define KEYBOARD_IRQ        0x01
#define PIC2_IRQ            0x02
#define RTC_IRQ             0x08
#define KEYBOARD_PORT       0x60
#define SCAN_CODE_START     0x00
#define SCAN_CODE_END       0x53

#define RTC_REG_PORT      0x70
#define RTC_RW_PORT    0x71
#define RTC_DIVIDER         0x03
#define RTC_REG_A           0x0A
#define RTC_REG_B_DISABLE   0x8B   
#define RTC_REG_C           0x0C

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);
// handle keyboard interrupt
void keyboard_irq_handler();
// handler rtc interrupt
void rtc_irq_handler();
// initialize keyboard
void keyboard_init();
// intialize rtc periodic interrupts
void rtc_init();
// enable nmi
void nmi_enable();
// disable nmi
void nmi_disable();

#endif /* _I8259_H */
