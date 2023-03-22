/* Terminal driver / keyboard related functions*/

#include "idt.h"
#include "idt_asm.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"
#include "tests.h"
#include "keyboard_driver.h"


static unsigned char keyboard_buf[KEYBOARD_BUF_SIZE];

// scan code set 1, maps scan codes from keyboard to characters
static unsigned char scancodes[] = { // values 0x00 - 0x53, length 83
   '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=','\0',  
   '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0',
   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 
   'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '*', '\0', ' ', '\0',
   '\0', '\0', '\0', '\0', '\0', '\0','\0', '\0','\0', '\0','\0', '\0', '7', '8', '9', 
   '-', '4', '5', '6', '+', '1', '2', '3', '0', '.'
};

// output the character to the screen
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
    //printf("keyboard handler \n");
    if(code >= SCAN_CODE_START && code <= SCAN_CODE_END) { // check if key is invalid for print
        echo = scancodes[code]; // print char if key was valid
        if(echo != '\0') {
            putc(echo);
        }
    }
    send_eoi(KEYBOARD_IRQ); // send the irq
    //printf("eoi sent");
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
}