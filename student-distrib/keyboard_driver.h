#ifndef KEYBOARD_DRIVER_H
#define KEYBOARD_DRIVER_H

#define KEYBOARD_IRQ        0x01
#define KEYBOARD_PORT       0x60
#define SCAN_CODE_START     0x00
#define SCAN_CODE_END       0x53
#define KEYBOARD_BUF_SIZE   128

// handle keyboard interrupt
void keyboard_irq_handler();

// initialize keyboard
void keyboard_init();

#endif /*KEYBOARD_DRIVER_H*/
