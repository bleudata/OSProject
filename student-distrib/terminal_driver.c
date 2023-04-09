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
    d_entry dentry;
    int32_t dentry_success = read_dentry_by_name(filename, &dentry);

    if(dentry_success == -1){
        return -1;
    }

    register uint32_t cur_esp asm("esp");
    pcb_t * pcb_address = (pcb_t*)(cur_esp & 0xFFFFE000);

    int32_t inode_num = dentry.inode_num;
    int32_t fd = 0;
    
    while(fd < 2){
        if(inode_num == ((pcb_t*)pcb_address)->fd_array[fd].inode_num){
            return fd;
        }
        fd++;
    }
    //if it reaches here it failed
    return -1;
}
/*
 * terminal_read
 *   DESCRIPTION: Reads from the keyboard buffer and copies specified number of bytes into an array given by the user
 *   INPUTS: fd -- file descriptor
 *            buf -- array provided by user, data from keyboard buffer is copied into here
 *            n - the number of bytes we want to read from the buffer
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read or -1 for FAIL
 *   SIDE EFFECTS: empties part of the keyboard buffer
 */
int32_t terminal_read(int32_t fd, void * buf, int32_t n) {
    // Return data from one line that ended in \n or a full buffer
    int32_t i, ret; // loop counter and index, also counts the number of characters read
    unsigned char * new_buf = (unsigned char *)buf;
  // now use newBuf instead of buf
    unsigned char * keyboard_buf;
    keyboard_buf = get_keyboard_buffer();
    set_read_flag(1); // tell keyboard we're inside a terminal read

    // validate input, null pointer provided by user
    if(new_buf == 0) {
        return -1; 
    }
    // validate fd 
    if (fd != 0) { // for testing just use 1 for fd
        return -1;
    }
    // validate n
    if (n < 0) { // for testing just use 1 for fd
        return -1;
    }

    // Need to clear out the user buffer before we adjust for the keyboard_buffer size to make sure we clear out 
    // any random stuff even if comes after we've filled as much as possible using the contents of the keyboard buffer
    // NOTE: will cause a page fault if n > length of new_buf, so user needs to ensure n <= length(new_buf)
    memset(new_buf, '\0', n); 

    // validate input
    if(n > KEYBOARD_BUF_SIZE) { 
        n = KEYBOARD_BUF_SIZE; 
    }
    
    i = 0;
    ret = 0;
    
    // Loop while we wait for an enter
    while(get_enter_count() < 1);
    // Read the keyboard buffer and delete it 
    while((i < n) && (keyboard_buf[i] != '\n')) {
        new_buf[i] = keyboard_buf[i]; 
        ret++;
        i++;
    }
    // new_buf[i] = '?';
    new_buf[i] = '\n'; // [\n\n]
    ret++;
    purge_and_align_keyboard_buffer(ret);
    decrement_enter_count();
    set_read_flag(0); // tell keyboard we're done with terminal read

    return ret;
}

/*
 * terminal_write
 *   DESCRIPTION: write data from input buffer to video memory to display on the screen
 *   INPUTS: fd - file descriptor
 *          buf - the buffer to read from to print to the screen
 *          n  - number of bytes to print 
 *   OUTPUTS: none
 *   RETURN VALUE: number or bytes written if successful, else -1; 
 *   SIDE EFFECTS: write to the screen 
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
        if(new_buf[i] != '\0') {
            // if(new_buf[i] == '\n') {
            //     printf("this is a newline lalalalallallalalalalalalalalalalalalal");
            // }
            putc_new(new_buf[i]);
        }
    }

    // printf("buffer: %s", new_buf);
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
    if(fd<0 || fd >7){
        return -1;
    }
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

