#ifndef TERMINAL_DRIVER_H
#define TERMINAL_DRIVER_H


// for now int assuming want to return 0 or -1 for success fail according to discussion slides
extern int terminal_open();
extern int terminal_read(int fd, unsigned char * buf, int n);
extern int terminal_write(int fd, unsigned char * buf, int n);
extern int terminal_close(int fd); // assuming don't need other params?

#endif /*TERMINAL_DRIVER_H*/
