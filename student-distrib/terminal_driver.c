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
 *   DESCRIPTION: TO DO
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int terminal_open() {return 0;}
/*
 * terminal_read
 *   DESCRIPTION: TO DO
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int terminal_read(int fd, unsigned char * buf, int n) {return 0;}
/*
 * terminal_write
 *   DESCRIPTION: TO DO
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int terminal_write(int fd, unsigned char * buf, int n) {return 0;}
/*
 * terminal_close
 *   DESCRIPTION: TO DO
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
int terminal_close(int fd) {return 0;}

