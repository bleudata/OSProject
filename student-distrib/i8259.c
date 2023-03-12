/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    outb(MASTER_8259_PORT, ICW1);  // starts the initialization sequence (in cascade mode)
	///io_wait();
	outb(SLAVE_8259_PORT, ICW1);
	//io_wait();
	outb(PIC1_DATA, ICW2_MASTER);                 // ICW2: Master PIC vector offset
	//io_wait();
	outb(PIC2_DATA, ICW2_SLAVE);                 // ICW2: Slave PIC vector offset
	//io_wait();
	outb(PIC1_DATA, ICW3_MASTER);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	//io_wait();
	outb(PIC2_DATA, ICW3_SLAVE);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	//io_wait();
	outb(PIC1_DATA, ICW4);
	//io_wait();
	outb(PIC2_DATA, ICW4);
	//io_wait();
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
    outb(port, value); 
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
    outb(port, value);
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if(irq_num >= 8) // if irq is on the secondary pic, send eoi to both pics
		outb(SLAVE_8259_PORT,EOI);
 
	outb(MASTER_8259_PORT,EOI);
}


