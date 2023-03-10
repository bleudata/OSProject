/* This file will hold all functions relating to IDT */

#include "idt.h"

#include "lib.h"
#include "x86_desc.h"
enum intel_Exceptions {
    DIVIDE_ERROR,
    RESERVED1,
    NMI_INTERRUPT,
    BREAKPOINT,
    OVERFLOW,
    BOUND_RANGE_EXCEEDED,
    INVALID_OPCODE,
    DEVICE_NA,
    DOUBLE_FAULT, // error code is 0
    RESERVED9,
    INVALID_TSS, //yes
    SEG_NOT_PRESENT, //yes
    STACK_SEG_FAULT, //yes
    GENERAL_PROTECTION, //yes
    PAGE_FAULT, //yes
    RESERVED15,
    FPU_FLOAT_ERROR,
    ALIGNMENT_CHECK, // error code is 0
    MACHINE_CHECK,
    SIMD_FLOAT_EXCEPTION
};

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

// go through each entry and set to either trap or interrupt
void init_idt() {

    return; 

}

// add the handlers into the idt
void setup_idt() {
    // int i;  HEARD PATRICK TELLING SOMEONE TO JUST DO IT 20 TIMES INSTEAD OF LOOP CAUSE ITS EASIER
    // for (i = 0; i < len(exceptions); i++) {
    //     SET_IDT_ENTRY(idt[0], exceptions[0]);
    // }

   // SET_IDT_ENTRY(idt[0], )
}

// test to make one handler for all intel exceptions 0-19 to just print and sit in a while loop
void intel_handler_0_19(int vector) {
    if(vector < 0 || vector > 19) {
        return; // invalid vector number for this function
    }
    printf("%s", intel_handler_strings[vector]);
    while(1); // infinite loop here for now, supposed to have this according to slides???
}

void divide_error_handler() {
    printf("Divide Error."); // maybe we want to puts instead?
    while(1); // want it to break for now
}

void reserved1_handler() {

return; 
}

void nmi_interrupt_handler() {

}

void breakpoint_handler() {

}

void overflow_handler() {

}

void bound_range_exceeded_handler() {

}

void invalid_opcode_handler() {

}

void device_na_handler() {

}

void double_fault_handler() {

}

void reserved9_handler() {

}

void invalid_tss_handler() {

}

void seg_not_present_handler() {

}

void stack_set_fault_handler() {

}

void general_protection_handler() {

}

void page_fault_handler() {

}

void reserved15_handler() {

}

void fpu_float_error_handler() {

}

void alignment_check_handler() {

}

void machine_check_handler() {

}

void smid_float_exception_handler() {
    return ;
}
