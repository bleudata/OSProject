/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */
int pic_error_code;

/* Initialize the 8259 PIC */
void i8259_init(void) {

}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {

}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {

}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {

}

void keyboard_irq_handler() {
    puts("test in keyboard handler");
}

void rtc_irq_handler() {
    test_interrupts(); // doc says to use this to test rtc
}

int get_pic_error_code() {
    return pic_error_code;
}

