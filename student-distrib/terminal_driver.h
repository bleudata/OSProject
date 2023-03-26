#ifndef TERMINAL_DRIVER_H
#define TERMINAL_DRIVER_H


// for now int assuming want to return 0 or -1 for success fail according to discussion slides
extern int terminal_open(const uint8_t * filename);
extern int terminal_read(int32_t fd, unsigned char * buf, int32_t n);
extern int terminal_write(int32_t fd, unsigned char * buf, int32_t n);
extern int terminal_close(int32_t fd); // assuming don't need other params?

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void update_cursor(int x, int y);


#endif /*TERMINAL_DRIVER_H*/
