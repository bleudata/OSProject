/* This file will hold all functions relating to IDT */

#include "idt.h"
#include "idt_asm.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"
#include "tests.h"
#include "rtc.h"
#include "keyboard_driver.h"
#include "syscalls.h"

// array of strings to print for each vector 0-19 exception
static unsigned char * intel_handler_strings[] = {
    [DIVIDE_ERROR] = (unsigned char *) "Divide error",
    [RESERVED1] = (unsigned char *) "Reserved1",
    [NMI_INTERRUPT] = (unsigned char *) "NMI interrupt",
    [BREAKPOINT] = (unsigned char *) "Breakpoint",
    [OVERFLOW] = (unsigned char *) "Overflow",
    [BOUND_RANGE_EXCEEDED] = (unsigned char *) "Bound range exceeded",
    [INVALID_OPCODE] = (unsigned char *) "Invalid opcode",
    [DEVICE_NA] = (unsigned char *) "Device not assigned",
    [DOUBLE_FAULT] = (unsigned char *) "Double fault", // error code is 0
    [RESERVED9] = (unsigned char *) "Reserved9",
    [INVALID_TSS] = (unsigned char *) "Invalid TSS", //yes
    [SEG_NOT_PRESENT] = (unsigned char *) "Segment not present", //yes
    [STACK_SEG_FAULT] = (unsigned char *) "Stack seg fault", //yes
    [GENERAL_PROTECTION] = (unsigned char *) "General protection", //yes
    [PAGE_FAULT] = (unsigned char *) "Page fault", //yes
    [RESERVED15] = (unsigned char *) "Reserved 15",
    [FPU_FLOAT_ERROR] = (unsigned char *) "FPU Float error",
    [ALIGNMENT_CHECK] = (unsigned char *) "Alignment check", // error code is 0
    [MACHINE_CHECK] = (unsigned char *) "Machine check",
    [SIMD_FLOAT_EXCEPTION] = (unsigned char *) "SIMD float exception"
};


/*
 * idt_init
 *   DESCRIPTION: initializes the idt (interrupt descriptor table) by setting entries for vectors 0-19,
 *                keyboard interrupts, rtc interrupts, and system calls
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes values for entries in the idt such as gates, sets handlers in the idt, and loads the idt address
 */
void idt_init() {
    int i;
    
    // intel exceptions
    for(i = DIVIDE_ERROR; i <= SIMD_FLOAT_EXCEPTION; i++) {
        idt[i].size = 1; // want size to be 32 bit 1110 for interrupt gate
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 0;
        idt[i].reserved4 = 0;
        idt[i].reserved0 = 0;
        idt[i].dpl = 0;
        idt[i].present = 1;
        idt[i].seg_selector = KERNEL_CS;
    }
    // pic irqs
    idt[KEYBOARD_VECTOR].size = 1; // 1110 for interrupt gate
    idt[KEYBOARD_VECTOR].reserved1 = 1; 
    idt[KEYBOARD_VECTOR].reserved2 = 1;
    idt[KEYBOARD_VECTOR].reserved3 = 0;
    idt[KEYBOARD_VECTOR].reserved4 = 0;
    idt[KEYBOARD_VECTOR].reserved0 = 0;
    idt[KEYBOARD_VECTOR].dpl = 0;
    idt[KEYBOARD_VECTOR].present = 1;
    idt[KEYBOARD_VECTOR].seg_selector = KERNEL_CS;

    idt[RTC_VECTOR].size = 1;
    idt[RTC_VECTOR].reserved1 = 1; 
    idt[RTC_VECTOR].reserved2 = 1;
    idt[RTC_VECTOR].reserved3 = 0;
    idt[RTC_VECTOR].reserved4 = 0;
    idt[RTC_VECTOR].reserved0 = 0;
    idt[RTC_VECTOR].dpl = 0;
    idt[RTC_VECTOR].present = 1;
    idt[RTC_VECTOR].seg_selector = KERNEL_CS;

    // system call
    idt[SYSTEM_CALL_VECTOR].size = 1;
    idt[SYSTEM_CALL_VECTOR].reserved1 = 1; // 1111 for trap gate
    idt[SYSTEM_CALL_VECTOR].reserved2 = 1;
    idt[SYSTEM_CALL_VECTOR].reserved3 = 1;
    idt[SYSTEM_CALL_VECTOR].reserved4 = 0;
    idt[SYSTEM_CALL_VECTOR].reserved0 = 0;
    idt[SYSTEM_CALL_VECTOR].dpl = 3;
    idt[SYSTEM_CALL_VECTOR].present = 1;
    idt[SYSTEM_CALL_VECTOR].seg_selector = KERNEL_CS;

    setup_idt();
    lidt(idt_desc_ptr); // load the idt address
}

/*
 * setup_idt
 *   DESCRIPTION: sets handlers for vectors 0-19, keyboard and rtc interrupts, and system calls in the idt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: placed handlers into the idt
 */
void setup_idt() {
    // vectors 0-19
    SET_IDT_ENTRY(idt[DIVIDE_ERROR], divide_error_handler_lnk);
    SET_IDT_ENTRY(idt[RESERVED1], reserved1_handler_lnk);
    SET_IDT_ENTRY(idt[NMI_INTERRUPT], nmi_interrupt_handler_lnk);
    SET_IDT_ENTRY(idt[BREAKPOINT], breakpoint_handler_lnk);
    SET_IDT_ENTRY(idt[OVERFLOW], overflow_handler_lnk);
    SET_IDT_ENTRY(idt[BOUND_RANGE_EXCEEDED], bound_range_exceeded_handler_lnk);
    SET_IDT_ENTRY(idt[INVALID_OPCODE], invalid_opcode_handler_lnk);
    SET_IDT_ENTRY(idt[DEVICE_NA], device_na_handler_lnk);
    SET_IDT_ENTRY(idt[DOUBLE_FAULT], double_fault_handler_lnk);
    SET_IDT_ENTRY(idt[RESERVED9], reserved9_handler_lnk);
    SET_IDT_ENTRY(idt[INVALID_TSS], invalid_tss_handler_lnk);
    SET_IDT_ENTRY(idt[SEG_NOT_PRESENT], seg_not_present_handler_lnk);
    SET_IDT_ENTRY(idt[STACK_SEG_FAULT], stack_set_fault_handler_lnk);
    SET_IDT_ENTRY(idt[GENERAL_PROTECTION], general_protection_handler_lnk);
    SET_IDT_ENTRY(idt[PAGE_FAULT], page_fault_handler_lnk);
    SET_IDT_ENTRY(idt[RESERVED15], reserved15_handler_lnk);
    SET_IDT_ENTRY(idt[FPU_FLOAT_ERROR], fpu_float_error_handler_lnk);
    SET_IDT_ENTRY(idt[ALIGNMENT_CHECK], alignment_check_handler_lnk);
    SET_IDT_ENTRY(idt[MACHINE_CHECK], machine_check_handler_lnk);
    SET_IDT_ENTRY(idt[SIMD_FLOAT_EXCEPTION], smid_float_exception_handler_lnk);

    // pic and system call
    SET_IDT_ENTRY(idt[KEYBOARD_VECTOR], keyboard_handler_lnk);
    SET_IDT_ENTRY(idt[RTC_VECTOR], rtc_handler_lnk);
    SET_IDT_ENTRY(idt[SYSTEM_CALL_VECTOR], system_call_handler_lnk);

}

/*
 * generic_handler
 *   DESCRIPTION: umbrella function to determine which handler to call based on the interrupt vector
 *   INPUTS: vector -- interrupt vector
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls the interrupt handler corresponding to the input vector
 */
 void generic_handler(int vector) {
    //clear();
    //printf("printing vector %d \n", vector);
    if(vector >= 0 && vector <= 19) {
        set_exception_flag();
        halt(0);
        generic_intel_handler(vector);
    }
    else if(vector == KEYBOARD_VECTOR) {
        keyboard_irq_handler(vector); // this is from pic file
    }
    else if(vector == RTC_VECTOR) {
        //printf("entered rtc if statement");
        rtc_irq_handler(); // this is from rtc file
    }
    // else if(vector == PIT_VECTOR){
        
    // }
    // else if(vector == SYSTEM_CALL_VECTOR) {
    //     generic_system_call_handler();
    // }
 }

/*
 * generic_intel_handler
 *   DESCRIPTION: handler for intel determined exceptions: vectors 0-19, and 
 *                prints which exception occurred to the screen and sits in a while loop
 *   INPUTS: vector -- interrupt vector
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints message to the screen saying which exception occured
 */
void generic_intel_handler(int vector) {
    //printf("before line 139 if statement hello \n");
    if(vector < 0 || vector > 19) {
       return; // invalid vector number for this function
    }

    terminal_write(1, intel_handler_strings[vector], 30);
    color_screen(BSOD);
    while(1); // infinite loop here for now, supposed to have this according to slides???
}







