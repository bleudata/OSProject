/*terminal driver works between keyboard driver and user space????*/

#include "idt.h"
#include "idt_asm.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"
#include "tests.h"
#include "keyboard_driver.h"
#include "terminal_driver.h"

// static unsigned char* screen_buf[SCREEN_BYTES];

/*
 * terminal_open
 *   DESCRIPTION: TO DO
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:  none
 */
int terminal_open() {
 
    return 0;
}
/*
 * terminal_read
 *   DESCRIPTION: Reads from the keyboard buffer and copies specified number of bytes into an array given by the user
 *   INPUTS: fd -- file descriptor
*            buf -- array provided by user, data from keyboard buffer is copied into here
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read
 *   SIDE EFFECTS: 
 */
int terminal_read(int fd, unsigned char * buf, int n) {
    // Return data from one line that ended in \n or a full buffer
    int i;
    unsigned char * keyboard_buf;
    keyboard_buf = get_keyboard_buffer();

    // validate input, null pointer provided by user
    if(buf == 0) {
        return 0; 
    }
    // validate input, at most can read all of the keyboard buffer
    if(n > KEYBOARD_BUF_SIZE) { 
        n = KEYBOARD_BUF_SIZE; 
    }

    for(i = 0; i < n; i++) {
        buf[i] = keyboard_buf[i]; 
    }

    return n;
}
/*
 * terminal_write
 *   DESCRIPTION: write data from input buffer to video memory to display on the screen
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: number or bytes written if successful, else -1; 
 *   SIDE EFFECTS: 
 */
int terminal_write(int fd, unsigned char * buf, int n) {
    int i;
    if(n > SCREEN_SIZE) {
        return -1;
    }
    // if(n > SCREEN_SIZE) {
    //     n = SCREEN_SIZE;
    // }

    for(i = 0; i < n; i ++) {
        putc_new(buf[i], 0);
    }


    return n;
}
/*
 * terminal_close
 *   DESCRIPTION: TO DO
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: 
 */
int terminal_close(int fd) {

    return 0;
}

