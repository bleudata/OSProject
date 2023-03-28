/*terminal driver works between keyboard driver and user space????*/

#include "idt.h"
#include "idt_asm.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"
#include "tests.h"
#include "keyboard_driver.h"
#include "terminal_driver.h"

/*
 * terminal_open
 *   DESCRIPTION: Doesn't actually do anything, just need to match system call params
 *   INPUTS: filename -- name of file to open
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:  none
 */
int32_t terminal_open(const uint8_t* filename) {
    return 0;
}
/*
 * terminal_read
 *   DESCRIPTION: Reads from the keyboard buffer and copies specified number of bytes into an array given by the user
 *   INPUTS: fd -- file descriptor
*            buf -- array provided by user, data from keyboard buffer is copied into here
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read or -1 for FAIL
 *   SIDE EFFECTS: 
 */
int32_t terminal_read(int32_t fd, void * buf, int32_t n) {
    // Return data from one line that ended in \n or a full buffer
    int32_t i, ret; // loop counter and index, also counts the number of characters read
    unsigned char * new_buf = (unsigned char *)buf;
  // now use newBuf instead of buf
    unsigned char * keyboard_buf;
    keyboard_buf = get_keyboard_buffer();

    // validate input, null pointer provided by user
    if(new_buf == 0) {
        return -1; 
    }
    // validate fd 
    if (fd != 0) { // for testing just use 1 for fd
        return -1;
    }
    if (n < 0) { // for testing just use 1 for fd
        return -1;
    }

    // TODO: Change if this is actually an error
    // validate input, at most can read all of the keyboard buffer
    if(n > KEYBOARD_BUF_SIZE) { 
        n = KEYBOARD_BUF_SIZE; 
    }
    
    i = 0;
    ret = 0;

    // Loop while we wait for an enter
    while(get_enter_count() < 1);

    // Read the keyboard buffer and delete it 
    while((i < n) && (keyboard_buf[i] != '\n')) {
        new_buf[i] =keyboard_buf[i]; 
        ret++;
        i++;
    }
    new_buf[i] = '\n';
    ret++;
    purge_and_align_keyboard_buffer(ret);
    
    return ret;
}
/*
 * terminal_write
 *   DESCRIPTION: write data from input buffer to video memory to display on the screen
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: number or bytes written if successful, else -1; 
 *   SIDE EFFECTS: 
 */
int32_t terminal_write(int32_t fd, const void * buf, int32_t n) {
    int i;
    const unsigned char * new_buf = (unsigned char*) buf;

    // validate fd 
    if (fd != 1) {
        return -1;
    }
    // check for null pointer
    if(new_buf == 0) {
        return -1;
    }
    if(n < 0) { // cant read negative bytes
        return -1;
    }

    
    // TODO: How do we know the buffer size? How to check if the buffer size is equal to n ?

    for(i = 0; i < n; i ++) {
        putc_new(new_buf[i], 0);
    }
    update_cursor(get_x_position(), get_y_position());

    return n;
}
/*
 * terminal_close
 *   DESCRIPTION: Closes the terminal, 
 *   INPUTS: fd - file descriptor just to match params of system call write function
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: 
 */
int32_t terminal_close(int32_t fd) {
    return 0;
}

/*
 * enable_cursor
 *   DESCRIPTION: enables the cursor 
 *   INPUTS: cursor_start -- scanline for cursor to start on 
 *           cursor_end -- scanline for cursor to end on 
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: cursor will be displayed on the screen
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    outb(0x0A, VGA_ADDR_REG); // select cursor start register 
    outb((inb(VGA_DATA_REG) & 0xC0) | cursor_start, VGA_DATA_REG);// bit 5 = 0 to enable cursor, bits 0-4 cursor scanline start, probably 15
    outb(0x0B, VGA_ADDR_REG);
    outb((inb(VGA_DATA_REG) & 0xE0) | cursor_end, VGA_DATA_REG); // bits 6-7 cursor skew, bits 0-4 cursor end line, probably 15
}


/*
 * update_cursor
 *   DESCRIPTION: moves cursor to new position on the screen
 *   INPUTS: x -- x position on the screen
 *           y -- y position on the screen
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: cursor will be moved to a new location on the screen
 */
void update_cursor(int x, int y)
{
    uint16_t pos = y * NUM_COLS + x;
    outb(0x0F, VGA_ADDR_REG); // cursor location low
    outb((uint8_t) (pos & 0xFF),VGA_DATA_REG);
    outb(0x0E, VGA_ADDR_REG); // cursor location high
    outb((uint8_t) ((pos >> 8) & 0xFF), VGA_DATA_REG);
}

