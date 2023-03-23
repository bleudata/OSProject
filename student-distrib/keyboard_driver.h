#ifndef KEYBOARD_DRIVER_H
#define KEYBOARD_DRIVER_H

#define KEYBOARD_IRQ        0x01
#define KEYBOARD_PORT       0x60
#define SCAN_CODE_START     0x00
#define SCAN_CODE_END       0x39
#define KEYBOARD_BUF_SIZE   128

// handle keyboard interrupt
void keyboard_irq_handler();

// initialize keyboard
void keyboard_init();

// returns a pointer to the keyboard buffer so the terminal driver can access its contents
unsigned char * get_keyboard_buffer();

// attempt to write a new character to the keyboard buffer
unsigned char add_to_keyboard_buffer(unsigned char input); 

// attempt to remove a character from the keyboard buffer
unsigned char remove_from_keyboard_buffer(); 

#endif /*KEYBOARD_DRIVER_H*/
