/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"
#include "tests.h"

// SOURCE CREDIT / CITATION: osdev.org

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

// scan code set 1, maps scan codes from keyboard to characters
static unsigned char scancodes[] = { // values 0x00 - 0x53, length 83
   '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=','\0',  
   '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0',
   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 
   'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '*', '\0', ' ', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0','\0', '\0','\0', '\0', '7', '8', '9', 
   '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'
};

/* Initialize the 8259 PIC */
/*
 * i8259_init
 *   DESCRIPTION: initializes primary and secondary pic and disables all irqs
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes primary and secondary pic and disables all irqs
 */
void i8259_init(void) {
    outb(ICW1, MASTER_8259_PORT);  // starts the initialization sequence (in cascade mode)
    outb(ICW2_MASTER, PIC1_DATA);  // ICW2: Master PIC vector offset
	outb(ICW3_MASTER, PIC1_DATA);  // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(ICW4, PIC1_DATA);

	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, PIC2_DATA);   // ICW2: Slave PIC vector offset
	outb(ICW3_SLAVE, PIC2_DATA);   // ICW3: tell Slave PIC its cascade identity (0000 0010)
	outb(ICW4, PIC2_DATA);
	
    // disable all irqs
    int i;
    for(i = 0; i < 16; i++) {
        disable_irq(i);
    }
}

/* Enable (unmask) the specified IRQ */
/*
 * enable_irq
 *   DESCRIPTION: enables one irq on either the primary or secondary PIC
 *   INPUTS: irq_num -- which irq to enable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: unmasks the specified irq on either the primary or secondary PIC
 */
void enable_irq(uint32_t irq_num) {
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

/* Disable (mask) the specified IRQ */
/*
 * disable_irq
 *   DESCRIPTION: disables one irq on either the primary or secondary PIC
 *   INPUTS: irq_num -- which irq to disable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: masks the specified irq on either the primary or secondary PIC
 */
void disable_irq(uint32_t irq_num) {
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

/* Send end-of-interrupt signal for the specified IRQ */
/*
 * send_eoi
 *   DESCRIPTION: sends the end of interrupt signal to the PIC/PICs
 *   INPUTS: irq_num -- which irq is done being serviced
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sends end of interrupt signal to PIC(s)
 */
void send_eoi(uint32_t irq_num) {
    if(irq_num >= 8) {// if irq is on the secondary pic, send eoi to both pics
		outb(EOI|(irq_num-8), SLAVE_8259_PORT); 
        outb(EOI|PIC2_IRQ, MASTER_8259_PORT); // always send eoi to primary pic
        
    }
    else {
        outb(EOI|irq_num, MASTER_8259_PORT); // always send eoi to primary pic

    }
}

// output the character to the screen
/*
 * keyboard_irq_handler
 *   DESCRIPTION: echos to the screen the character of a key press on the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints a character to the screen
 */
void keyboard_irq_handler() {
    int code = inb(KEYBOARD_PORT);
    unsigned char echo;
    //printf("keyboard handler \n");
    if(code >= SCAN_CODE_START && code <= SCAN_CODE_END) { // check if key is invalid for print
        echo = scancodes[code]; // print char if key was valid
        if(echo != '\0') {
            putc(echo);
        }
    }
    send_eoi(KEYBOARD_IRQ); // send the irq
    //printf("eoi sent");
}

/*
 * rtc_irq_handler
 *   DESCRIPTION: calls the test_interrupts function from lib.c when an rtc interrupt occurs
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints various characters to the screen based on test_interrupts function
 */
void rtc_irq_handler() {
    int result;
    test_interrupts();
    outb(RTC_REG_C, RTC_REG_PORT); // select register c
    result = inb(RTC_RW_PORT); // need to read from c register or the interrupt won't happen again
    send_eoi(RTC_IRQ);
}

/*
 * keyboard_init
 *   DESCRIPTION: initializes the keyboard by enabling its irq on the PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enables keyboard irq on PIC
 */
void keyboard_init() {
    enable_irq(KEYBOARD_IRQ);
}

/*
 * rtc_init
 *   DESCRIPTION: initializes periodic rtc interrupts and enables rtc irq on the PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enables period rtc interrupts, enables rtc irq on PIC, 
 *                 temporarily disables nmi, but re-enabled at end of function
 */
void rtc_init() {
    outb(RTC_REG_B_DISABLE, RTC_REG_PORT); // set to register b and disable nmi
    char prev = inb(RTC_RW_PORT); // get current value from register b
    outb(RTC_REG_B_DISABLE, RTC_REG_PORT); // set to register b and disable nmi
    outb(prev | 0x40, RTC_RW_PORT); // set bit 6 to 1 using 0x40 to enable periodic interrupts
    enable_irq(RTC_IRQ);
    nmi_enable();
}

/*
 * nmi_enable
 *   DESCRIPTION: enables nmi (non maskable interrupts)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enables nmi
 */
void nmi_enable() {
    outb(inb(RTC_REG_PORT) & 0x7F, RTC_REG_PORT); // set the 0x80 bit to 0 to enable 
    inb(RTC_RW_PORT);
}

/*
 * nmi_disable
 *   DESCRIPTION: disables nmi
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: disables nmi
 */
void nmi_disable() {
    outb(inb(RTC_REG_PORT)|0x80 , RTC_REG_PORT); // set the 0x80 bit to 1 to disable
    inb(RTC_RW_PORT);
}
