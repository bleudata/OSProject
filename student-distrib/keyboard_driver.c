/* Terminal driver / keyboard related functions*/

#include "idt.h"
#include "idt_asm.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"
#include "tests.h"
#include "keyboard_driver.h"
#include "terminal_driver.h"

/*Variables that keep track of CapsLock, Shift, Ctrl*/
unsigned char capslock_on = 0x0;
unsigned char capslock_released = 0x1;
unsigned char shift_pressed = 0x0;
unsigned char ctrl_pressed = 0x0;
unsigned char alt_pressed = 0x0;

/*Keyboard buffer variables*/
static unsigned char keyboard_buf[KEYBOARD_BUF_SIZE];
static unsigned char* buf_position = keyboard_buf; //points to next empty index in the buffer
//static unsigned char* buf_end = keyboard_buf+128;
unsigned char enter_count = 0;
unsigned char read_flag = 0; // 1 if inside a read, 0 else

#define BUF_END_ADDR        keyboard_buf+127 // need minus one because the last index is the newline
#define BUF_LINE_TWO_ADDR   keyboard_buf+80
#define NEWLINE_INDEX       80

// Array that holds Shifted Values
// [nonshifted value, shifted value], \0 are function keys!! except the first two
static unsigned char scancodes[58][2] = { // values 0x00 - 0x39
    {'\0', '\0'}, {'\0', '\0'}, {'1', '!'},    {'2', '@'}, 
    {'3', '#'},   {'4', '$'},   {'5', '%'},    {'6', '^'}, 
    {'7', '&'},   {'8', '*'},   {'9', '('},    {'0', ')'},    
    {'-', '_'},   {'=', '+'},   {'\0', '\0'},  {'\t', '\t'}, 
    {'q', 'Q'},   {'w', 'W'},   {'e', 'E'},    {'r', 'R'}, 
    {'t', 'T'},   {'y', 'Y'},   {'u', 'U'},    {'i', 'I'}, 
    {'o', 'O'},   {'p', 'P'},   {'[', '}'},    {']', '}'}, 
    {'\n', '\n'}, {'\0', '\0'}, {'a', 'A'},    {'s', 'S'}, 
    {'d', 'D'},   {'f', 'F'},   {'g', 'G'},    {'h', 'H'}, 
    {'j', 'J'},   {'k', 'K'},   {'l', 'L'},    {';', ':'}, 
    {'\'', '\"'}, {'`', '~'},   {'\0', '\0'},  {'\\', '|'}, 
    {'z', 'Z'},   {'x', 'X'},   {'c', 'C'},    {'v', 'V'}, 
    {'b', 'B'},   {'n', 'N'},   {'m', 'M'},    {',', '>'}, 
    {'.', '>'},   {'/', '?'},   {'\0', '\0'},  {'\0', '\0'},
    {'\0', '\0'}, {' ', ' '}
};


/*
 * keyboard_irq_handler
 *   DESCRIPTION: echos to the screen the character of a key press on the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints a character to the screen
 */
void keyboard_irq_handler() {
    int code = inb(KEYBOARD_PORT);
    unsigned char echo;

    if(code >= SCAN_CODE_START && code <= SCAN_CODE_END) { // check if key is invalid for print
        unsigned char val = 0;
        if ((code >= Q_PRESS && code <= P_PRESS) || (code >= A_PRESS && code <= L_PRESS) || (code >= Z_PRESS && code <= M_PRESS)) {
            val = shift_pressed ^ capslock_on; // if scancode is letter we care about caps lock and shift
        }
        else {
            val = shift_pressed; // if scancode is anything else only care about shift :)
        }
        if (ctrl_pressed && code == L_PRESS) { // 0x26 is the scan code for L/l
            clear_reset_cursor();
            update_cursor(0,0); // move cursor back to top left of the screen
            if(read_flag == 0) { //only purge the keyboard buffer if not in a terminal read
                purge_keyboard_buffer(); 
            }
        }
        else {
            echo = scancodes[code][val]; // print char if key was valid
            if (echo == '\n') {
                enter_count++;
            }
            if((echo != '\0')) { // if not function key
                if(add_to_keyboard_buffer(echo)){ // if successfully wrote to the buffer
                    if(echo == '\t') { // special case for tab
                        putc_new(' '); // need to print multiple spaces
                        putc_new(' ');
                        putc_new(' ');
                        putc_new(' ');
                    }
                    else {
                        putc_new(echo);
                    }
                    update_cursor(get_x_position(), get_y_position()); 
                }
            }
        }
    }

    // // Function Buttons (Not done Alt and F2,F3,F4) TODO: CKPT 5 
    // SHIFT
    if ((code == L_SHIFT_PRESS) || (code == R_SHIFT_PRESS)) {
        shift_pressed = 1;
    }
    else if ((code == L_SHIFT_RELEASE) || (code == R_SHIFT_RELEASE)) {
        shift_pressed = 0;
    }
    // CAPSLOCK
    else if (code == CAPS_PRESS) {
        if (capslock_released) {
            capslock_on = capslock_on ? 0 : 1;
        } 
        capslock_released = 0;
    }
    else if (code == CAPS_RELEASE) {
        capslock_released = 1;
    }
    // CTRL Left
    else if (code == L_CTRL_PRESS) {
        ctrl_pressed = 1;
    }
    else if (code == L_CTRL_RELEASE) {
        ctrl_pressed = 0;
    }
    // ALT Left
    else if (code == L_ALT_PRESS) {
        alt_pressed = 1;
    }
    else if (code == L_ALT_RELEASE) {
        alt_pressed = 0;
    }
    // F2
    else if (code == F2_PRESS) {

    }
    // F3
    else if (code == F3_PRESS) {

    }
    // F4
    else if (code == F4_PRESS) {

    }
    // BACKSPACE
    else if (code == BACKSPACE) {
        unsigned char temp;
        temp = remove_from_keyboard_buffer();
        if(temp > 0){
            if(temp == 2) { // this was a tab, need to delete a total of 4 spaces
                unput_c(' '); // delete space 2
                unput_c(' '); // delete space 2
                unput_c(' '); // delete space 3
                unput_c(' '); // delete space 4
            }
            else {
                unput_c(*(buf_position+1));
            }
            update_cursor(get_x_position(), get_y_position());
        }
    }

    // Handles all key presses that expect mutliple codes
    else if (code == MULT_KEY_CODES) {
        int next_code = inb(KEYBOARD_PORT);
        // CTRL Right
        if (next_code == L_CTRL_PRESS) {
            ctrl_pressed = 1;
        }
        else if (next_code == L_CTRL_RELEASE) {
            ctrl_pressed = 0;
        }
        // ALT Right
        else if (next_code == L_ALT_PRESS) {
            alt_pressed = 1;
        }
        else if (next_code == L_ALT_RELEASE) {
            alt_pressed = 0;
        }
    }

    send_eoi(KEYBOARD_IRQ); // send the irq
}

/*
 * keyboard_init
 *   DESCRIPTION: initializes the keyboard by enabling its irq on the PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enables keyboard irq on PIC
 */
void keyboard_init() {
    enable_irq(KEYBOARD_IRQ);
    purge_keyboard_buffer(); // make sure the buffer is clean when we first start
}

/*
 * purge_keyboard_buffer
 *   DESCRIPTION: fills the buffer with null
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: chnages keyboard buffer
 */
void purge_keyboard_buffer() {
    memset(keyboard_buf, '\0', KEYBOARD_BUF_SIZE);
    buf_position = keyboard_buf; // move position back to the start of the buffer
}

/*
 * purge_and_aligned_keyboard_buffer
 *   DESCRIPTION: purges n characters in the buffer starting from index 0, then aligns the buffer such that the first nonempty index is index 0
 *   INPUTS: n -- number of characters/bytes to purge from the buffer and the new starting index once the buffer is aligned
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: chnages keyboard buffer
 */
void purge_and_align_keyboard_buffer(int n) {
    if(n <= KEYBOARD_BUF_SIZE) {
        memset(keyboard_buf, '\0', n);
    }
    if(n < KEYBOARD_BUF_SIZE) {
        align_keyboard_buffer(n); // only need to align if didn't completely purge the buffer
    }
    else {
        buf_position = keyboard_buf;
    }
}
/*
 * align_buffer
 *   DESCRIPTION: moves the characters in the keyboard buffer to the left, starting from index new_start which will be moved to index 0, etc. Then adds null characters
 *                to the end of the keyboard buffer as necessary
 *   INPUTS: new_start -- the index of the value to have as the new index 0 in the keyboard buffer
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes keyboard buffer
 */
void align_keyboard_buffer(int new_start) {
    int i = 0;
    if((new_start > KEYBOARD_BUF_SIZE-1) || (new_start < 1)) { // can't start past the bounds of the keyboard buffer
        return; 
    }
    while((new_start < KEYBOARD_BUF_SIZE)) { 
        keyboard_buf[i] = keyboard_buf[new_start];
        i++;
        new_start++;
    }
    while(i < KEYBOARD_BUF_SIZE) {
        keyboard_buf[i] = '\0';
        i++;
    }
    buf_position = keyboard_buf + KEYBOARD_BUF_SIZE - new_start; // point to next empty
}


/*
 * get_keyboard_buffer
 *   DESCRIPTION: returns pointer to keyboard buffer so it can be accessed by the terminal driver
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to keyboard_buf
 *   SIDE EFFECTS: none
 */
unsigned char * get_keyboard_buffer() {
    return keyboard_buf;
}

/*
 * add_to_keyboard_buffer
 *   DESCRIPTION: adds a character to the keyboard buffer if it isn't full, and clears if its a newline
 *   INPUTS: input -- character to try to print to the screen
 *   OUTPUTS: none
 *   RETURN VALUE: 1 if wrote to buffer or cleared the buffer, 0 if buffer was full
 *   SIDE EFFECTS: none
 */
unsigned char add_to_keyboard_buffer(unsigned char input) {
    if(buf_position < BUF_END_ADDR){
        *buf_position = input;
        buf_position++;
        return 1;
    }
    if(buf_position == BUF_END_ADDR) { // keyboard_buf[127] can only be a newline
        if(input == '\n') {
           *buf_position = '\n'; 
        }
        
    } 
    return 0;// couldn't add to buffer, buffer full
}

/*
 * remove_from_keyboard_buffer
 *   DESCRIPTION: removes one character from the keyboard  buffer, helper function for backspace
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 2 if character removed was tab, 1 if removed another character, 0 if buffer was empty
 *   SIDE EFFECTS: changes the keyboard buffer
 */
unsigned char remove_from_keyboard_buffer() {
    if(buf_position > keyboard_buf) { // if buffer is not empty 
        if(*(buf_position-1) == '\n') {
            return 0;
        }
        buf_position--; 
        if(*buf_position == '\t') {
            *buf_position = '\0';
            return 2;
        }
        else {
            *buf_position = '\0';
            return 1;
        }
    }
    return 0;
}

/*
 * get_enter_count
 *   DESCRIPTION: returns the number of enters in the keyboard buf
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: number of enters
 *   SIDE EFFECTS: none
 */
unsigned char get_enter_count() {
    return enter_count;
}

/*
 * decrement_enter_count
 *   DESCRIPTION: subtracts one from the keyboard buffer
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: number of enters
 *   SIDE EFFECTS: none
 */
void decrement_enter_count() {
    enter_count--;
}

void set_read_flag(unsigned char flag) {
    read_flag = flag;
}
