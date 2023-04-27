#ifndef TERMINAL_DRIVER_H
#define TERMINAL_DRIVER_H

#include "keyboard_driver.h"

#define CURSOR_ENABLE   0xC0
#define CURSOR_SKEW     0xE0

#define LOWER_16        0xFF
#define BYTE_SHIFT      8

typedef struct __attribute__ ((packed)) {
    int screen_x;
    int screen_y; 
    keyboard_buf_t keyboard;    
} terminal_t;

// initializes the structs for the three terminals
void terminal_init(); 

// for now int assuming want to return 0 or -1 for success fail according to discussion slides
extern int terminal_open(const uint8_t * filename);
extern int terminal_read(int32_t fd, void * buf, int32_t n);
extern int terminal_write(int32_t fd, const void * buf, int32_t n);
extern int terminal_close(int32_t fd); 

// to properly move the cursor
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void update_cursor(int x, int y); // update the position of the cursor on the visible terminal
extern unsigned char get_user_terminal_num(); // get the number 0-2 of the terminal that's visible 
extern unsigned char set_user_terminal_num(unsigned char num); // set the number 0-2 of the terminal that's visible
extern terminal_t* get_user_terminal(); // get the address of the terminal struct for the visible terminal
extern terminal_t * get_terminal(unsigned char num); // get address of the terminal struct for an input number 0-2

#endif /*TERMINAL_DRIVER_H*/
