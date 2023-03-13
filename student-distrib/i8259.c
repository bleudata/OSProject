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
    // disable all irqs
    int i;
    for(i = 0; i < 16; i++) {
        disable_irq(i);
    }

    outb(ICW1, MASTER_8259_PORT);  // starts the initialization sequence (in cascade mode)
	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_MASTER, PIC1_DATA);                 // ICW2: Master PIC vector offset
	outb(ICW2_SLAVE, PIC2_DATA);                 // ICW2: Slave PIC vector offset
	outb(ICW3_MASTER, PIC1_DATA);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(ICW3_SLAVE, PIC2_DATA);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	outb(ICW4, PIC1_DATA);
	outb(ICW4, PIC2_DATA);
	
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
 
    if(irq_num < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_num -= 8;
    }
    value = inb(port) | (1 << irq_num);
    outb(value, port); 
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
 
    if(irq_num < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_num -= 8;
    }
    value = inb(port) & ~(1 << irq_num);
    outb(value, port);
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if(irq_num >= 8) // if irq is on the secondary pic, send eoi to both pics
		outb(EOI, SLAVE_8259_PORT);
 
	outb(EOI, MASTER_8259_PORT); // always send eoi to primary pic
}

// output the character to the screen
void keyboard_irq_handler(int vector) {
    // for now we're just printing that we're inside the handler
    puts("keyboard handler");
    // should actually use putc when echoing the characters
}

// use the test_interrupts from lib.c according to the doc
void rtc_irq_handler() {
    test_interrupts();
}
