#ifndef KEYBOARD_DRIVER_H
#define KEYBOARD_DRIVER_H

#define KEYBOARD_IRQ        0x01
#define KEYBOARD_PORT       0x60
#define SCAN_CODE_START     0x00
#define SCAN_CODE_END       0x39
#define KEYBOARD_BUF_SIZE   128

// some scancodes
#define Q_PRESS        0x10
#define P_PRESS        0x19
#define A_PRESS        0x1E
#define L_PRESS        0x26
#define Z_PRESS        0x2C
#define M_PRESS        0x32
#define TAB_PRESS      0x0F
#define L_SHIFT_PRESS   0x2A
#define R_SHIFT_PRESS   0x36
#define L_SHIFT_RELEASE  0xAA
#define R_SHIFT_RELEASE  0xB6
#define CAPS_PRESS       0x3A
#define CAPS_RELEASE     0xBA
#define L_CTRL_PRESS     0x1D
#define L_CTRL_RELEASE   0x9D
#define L_ALT_PRESS      0x38
#define L_ALT_RELEASE    0xB8
#define F1_PRESS        0x3B
#define F2_PRESS        0x3C
#define F3_PRESS        0x3D
#define F4_PRESS        0x3E
#define BACKSPACE       0x0E
#define MULT_KEY_CODES  0xE0

typedef struct __attribute__ ((packed)){
    unsigned char keyboard_buf[KEYBOARD_BUF_SIZE];
    unsigned char* buf_position;
    unsigned char* buf_end_addr; 
    unsigned char* buf_line_two_addr;
    unsigned char enter_count;
    unsigned char read_flag;
} keyboard_buf_t;

// handle keyboard interrupt
void keyboard_irq_handler();

// initialize keyboard
void keyboard_init();

// fills whole keyboard buffer with \0
extern void purge_keyboard_buffer();

// purges part of the buffer and shifts left anything remaining in the keyboard buffer
extern void purge_and_align_keyboard_buffer(int n);

// shifts elements remaining in the keyboard buffer to the beginning
extern void align_keyboard_buffer(int new_start);

// returns a pointer to the keyboard buffer so the terminal driver can access its contents
unsigned char * get_keyboard_buffer();

// attempt to write a new character to the keyboard buffer
unsigned char add_to_keyboard_buffer(unsigned char input); 

// attempt to remove a character from the keyboard buffer
unsigned char remove_from_keyboard_buffer(); 

// Returns value of enter_flag
extern unsigned char get_enter_count();

// Decrements value of enter count
extern void decrement_enter_count();

// Sets enter_flag to 0
extern void clear_enter_flag();

// sets read_flag 
extern void set_read_flag(unsigned char flag);

// sets the active keyboard and the currently active (displayed) terminal
extern unsigned char set_active_terminal_and_keyboard (void * new_terminal);

#endif /*KEYBOARD_DRIVER_H*/
