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
static unsigned char* buf_position = keyboard_buf;
//static unsigned char* buf_end = keyboard_buf+128;
static unsigned char screen_buf[SCREEN_BYTES];
// unsigned char enter_flag = 0;

#define BUF_END_ADDR   keyboard_buf+127 // need minus one because the last index is the newline
#define BUF_LINE_TWO_ADDR keyboard_buf+80
#define NEWLINE_INDEX   80

// Holds all possible key press combinations
// [nonshifted value, shifted value]
static unsigned char scancodes[58][2] = { // values 0x00 - 0x39
    {'\0', '\0'}, {'\0', '\0'}, {'1', '!'},    {'2', '@'}, 
    {'3', '#'},   {'4', '$'},   {'5', '%'},    {'6', '^'}, 
    {'7', '&'},   {'8', '*'},   {'9', '('},    {'0', ')'},    
    {'-', '_'},   {'=', '+'},   {'\0', '\0'},  {'\0', '\0'}, 
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
 * purge_buffer
 *   DESCRIPTION: fills the buffer with null
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: chnages keyboard buffer
 */
void purge_buffer() {
    int i;
    for (i = 0; i < KEYBOARD_BUF_SIZE; i++) {
        keyboard_buf[i] = '\0';
    }
    buf_position = keyboard_buf; // move position back to the start of the buffer
}

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
        if ((code >= 0x10 && code <= 0x19) || (code >= 0x1e && code <= 0x26) || (code >= 0x2c && code <= 0x32)) {
            val = shift_pressed ^ capslock_on; 
        }
        else {
            val = shift_pressed;
        }
        if (ctrl_pressed && code == 0x26) { // 0x26 is the scan code for L/l
            clear_reset_cursor();
            purge_buffer();
            update_cursor(0,0); // move cursor back to top left of the screen
        }
        else {
            echo = scancodes[code][val]; // print char if key was valid
            if(echo != '\0') {
                //putc(echo);
                if(add_to_keyboard_buffer(echo)){ // if successfully wrote to the buffer
                    putc_new(echo, screen_buf);
                    update_cursor(get_x_position(), get_y_position()); 
                }
            }
        }
    }

    // Function Buttons (Not done Alt and F2,F3,F4) TODO: CKPT 5 
    //  TAB
    if (code == 0x0f){
        int i;
        for(i = 0; i < 4; i++) {
            if((add_to_keyboard_buffer(' ')) && (buf_position <= BUF_LINE_TWO_ADDR)) {
                putc_new(' ', screen_buf);
                update_cursor(get_x_position(), get_y_position());
            }
            else {
                break;
            }
        }
    
    }
    // SHIFT
    else if ((code == 0x2a) || (code == 0x36)) {
        shift_pressed = 1;
    }
    else if ((code == 0xaa) || (code == 0xb6)) {
        shift_pressed = 0;
    }
    // CAPSLOCK
    else if (code == 0x3a) {
        if (capslock_released) {
            capslock_on = capslock_on ? 0 : 1;
        } 
        capslock_released = 0;
    }
    else if (code == 0xba) {
        capslock_released = 1;
    }
    // CTRL Left
    else if (code == 0x1d) {
        ctrl_pressed = 1;
    }
    else if (code == 0x9d) {
        ctrl_pressed = 0;
    }
    // ALT Right
    else if (code == 0x38) {
        alt_pressed = 1;
    }
    else if (code == 0xb8) {
        alt_pressed = 0;
    }
    // F2
    else if (code == 0x3c) {

    }
    // F3
    else if (code == 0x3d) {

    }
    // F4
    else if (code == 0x3e) {

    }
    // BACKSPACE
    else if (code == 0x0e) {
        if(remove_from_keyboard_buffer()){
            unput_c();
            update_cursor(get_x_position(), get_y_position());
        }
    }

    // Handles all key presses that expect mutliple codes
    else if (code == 0xe0) {
        int next_code = inb(KEYBOARD_PORT);
        // CTRL Right
        if (next_code == 0x1d) {
            ctrl_pressed = 1;
        }
        else if (next_code == 0x9d) {
            ctrl_pressed = 0;
        }
        // ALT Right
        else if (next_code == 0x38) {
            alt_pressed = 1;
        }
        else if (next_code == 0xb8) {
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
    purge_buffer(); // make sure the buffer is clean when we first start
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
    if(input == '\n' ) { // found a new line character
        // enter_flag = 1;
        purge_buffer();
        return 1;   
    }
    else {
        if(buf_position < BUF_END_ADDR){
            *buf_position = input;
            buf_position++;
            return 1;
        }
        if(buf_position == BUF_END_ADDR) {
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
 *   RETURN VALUE: 1 if removed character, 0 if buffer was full
 *   SIDE EFFECTS: none
 */
unsigned char remove_from_keyboard_buffer() {
    if(buf_position > keyboard_buf) { // if buffer is not empty 
        buf_position--; 
        *buf_position = ' ';
        return 1;
    }
    return 0;
}


// unsigned char get_enter_flag() {
//     return enter_flag;
// }

// void clear_enter_flag(){
//     enter_flag = 0;
// }
