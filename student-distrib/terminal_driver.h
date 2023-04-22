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
    unsigned char * virtual_mem_addr;
    keyboard_buf_t keyboard;
    
} terminal_t;

// initializes the structs for the three terminals
void terminal_init(); 
// for now int assuming want to return 0 or -1 for success fail according to discussion slides
extern int terminal_open(const uint8_t * filename);
extern int terminal_read(int32_t fd, void * buf, int32_t n);
extern int terminal_write(int32_t fd, const void * buf, int32_t n);
extern int terminal_close(int32_t fd); // assuming don't need other params?

// to properly move the cursor
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void update_cursor(int x, int y);
extern unsigned char get_active_terminal_num();
extern unsigned char set_active_terminal_num(unsigned char num);
extern keyboard_buf_t* get_active_keyboard();
extern terminal_t* get_terminal();

#endif /*TERMINAL_DRIVER_H*/
