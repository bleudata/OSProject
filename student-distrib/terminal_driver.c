/*terminal driver works between keyboard driver and user space????*/

#include "idt.h"
#include "idt_asm.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"
#include "tests.h"
#include "keyboard_driver.h"
#include "terminal_driver.h"

terminal_t terminal_array[3];
unsigned char user_terminal_num = 0; //terminal that user is on

/*
 * terminal_init
 *   DESCRIPTION: Sets the values in each of the terminal structs in the terminal array, and clears the keyboard buffers for each terminal
 *   INPUTS: filename -- name of file to open
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:  none
 */
void terminal_init(){
    int i;
    for(i = 0; i < 3; i++) {
        terminal_array[i].keyboard.buf_position = terminal_array[i].keyboard.keyboard_buf;
        terminal_array[i].keyboard.buf_end_addr = (terminal_array[i].keyboard.keyboard_buf) + KEYBOARD_BUF_SIZE - 1; 
        terminal_array[i].keyboard.buf_line_two_addr = (terminal_array[i].keyboard.keyboard_buf) + NEWLINE_INDEX;
        terminal_array[i].screen_x = 0;
        terminal_array[i].screen_y = 0;

    }
    // terminal_array[0].storage_addr = (unsigned char* ) T0_VIRTUAL_ADDR;
    // terminal_array[0].storage_offset = (uint8_t) VMEM_OFFSET_T0;
    // terminal_array[1].storage_addr = (unsigned char* ) T1_VIRTUAL_ADDR;
    // terminal_array[1].storage_offset = (uint8_t) VMEM_OFFSET_T1;
    // terminal_array[2].storage_addr = (unsigned char* )T2_VIRTUAL_ADDR;
    // terminal_array[2].storage_offset = (uint8_t) VMEM_OFFSET_T2;


    user_terminal_num = 2; // default  to display terminal 0?
    set_active_terminal_and_keyboard(&terminal_array[2]);
    purge_keyboard_buffer();
    user_terminal_num = 1;
    set_active_terminal_and_keyboard(&terminal_array[1]);
    purge_keyboard_buffer();
    user_terminal_num = 0;
    set_active_terminal_and_keyboard(&terminal_array[0]);
    set_screen_x(&(terminal_array[0].screen_x));
    set_screen_y(&(terminal_array[0].screen_y));
    purge_keyboard_buffer();
    // need to set each terminal to have the active keyboard buffer and purge the buffer to help the init
}

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
 *            n - the number of bytes we want to read from the buffer
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes read or -1 for FAIL
 *   SIDE EFFECTS: empties part of the keyboard buffer
 */
int32_t terminal_read(int32_t fd, void * buf, int32_t n) {
    // Return data from one line that ended in \n or a full buffer
    int32_t i, ret; // loop counter and index, also counts the number of characters read
    unsigned char * new_buf = (unsigned char *)buf;
    terminal_t * terminal = get_terminal(get_cur_sched_terminal()); //update terminal and keyboard structs to the one that is currently displayed
  // now use newBuf instead of buf

    set_read_flag(1); // tell keyboard we're inside a terminal read

    // validate input, null pointer provided by user
    if(new_buf == 0) {
        return -1; 
    }
    // validate fd 
    if (fd != 0) {
        return -1;
    }
    // validate n
    if (n < 0) {
        return -1;
    }

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
    while((i < n) && (terminal->keyboard.keyboard_buf[i] != '\n')) {
        new_buf[i] = terminal->keyboard.keyboard_buf[i]; 
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

    // : How do we know the buffer size? How to check if the buffer size is equal to n ?
    for(i = 0; i < n; i ++) {
        if(new_buf[i] != '\0') {
            putc(new_buf[i]);
        }
    }

    // TODO: find a way to check if the current process is shown on the active terminal
    // only want to update the cursor if on the active terminal
    if(get_cur_sched_terminal() == get_cur_user_terminal()){
        update_cursor(get_x_position(), get_y_position());
    }
    
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
    if(fd<0 || fd > FD_MAX_SIZE){
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
    outb((inb(VGA_DATA_REG) & CURSOR_ENABLE) | cursor_start, VGA_DATA_REG);// bit 5 = 0 to enable cursor, bits 0-4 cursor scanline start, probably 15
    outb(0x0B, VGA_ADDR_REG);
    outb((inb(VGA_DATA_REG) & CURSOR_SKEW) | cursor_end, VGA_DATA_REG); // bits 6-7 cursor skew, bits 0-4 cursor end line, probably 15
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
    outb((uint8_t) (pos & LOWER_16),VGA_DATA_REG);
    outb(0x0E, VGA_ADDR_REG); // cursor location high
    outb((uint8_t) ((pos >> BYTE_SHIFT) & LOWER_16), VGA_DATA_REG);
}

/*
 * set_user_terminal_num
 *   DESCRIPTION: sets the active terminal number
 *   INPUTS: num -- 0-2 corresponding to terminal number 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 success, -1 fail
 *   SIDE EFFECTS: terminal number changes
 */
unsigned char set_user_terminal_num(unsigned char num) {
    if((num > 2)) {
        return -1;
    }
    user_terminal_num = num;
    return 0;
}

// /*
//  * get_active_keyboard
//  *   DESCRIPTION: returns the address of the keyboard struct corresponding to the active terminal
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   RETURN VALUE: address of the keyboard struct corresponding to the active terminal
//  *   SIDE EFFECTS: none
//  */
// keyboard_buf_t* get_active_keyboard() {
//     return &(terminal_array[user_terminal_num].keyboard);
// }


/*
 * get_user_terminal
 *   DESCRIPTION: returns the address of the terminal struct corresponding to the active terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: address of the terminal struct corresponding to the active terminal
 *   SIDE EFFECTS: none
 */
terminal_t* get_user_terminal() {
    return &(terminal_array[user_terminal_num]);
}

terminal_t* first_get_terminal() {
    return &(terminal_array[1]);
}

terminal_t* second_get_terminal() {
    return &(terminal_array[2]);
}



terminal_t * get_terminal(unsigned char num) {
    return &(terminal_array[num]);
}
