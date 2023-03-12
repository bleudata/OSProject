/* This file will hold all functions relating to IDT */

#include "idt.h"
#include "idt_asm.h"
#include "lib.h"
#include "x86_desc.h"

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
void idt_init() {
    int i;
    lidt(idt_desc_ptr); // load the idt address

    // intel exceptions
    for(i = DIVIDE_ERROR; i <= SIMD_FLOAT_EXCEPTION; i++) {
        idt[i].size = 1; // want size to be 32 bit 1110 for interrupt gate
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 0;
    }
    // pic irqs
    for(i = IRQ1_VECTOR; i <= IRQ15_VECTOR; i++) {
        idt[i].size = 1; // want size to be 32 bit 1110 for interrupt gate
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 0;
    }

    // system call
    idt[SYSTEM_CALL_VECTOR].size = 1;
    idt[SYSTEM_CALL_VECTOR].reserved1 = 1; // 1111 for trap gate
    idt[SYSTEM_CALL_VECTOR].reserved2 = 1;
    idt[SYSTEM_CALL_VECTOR].reserved3 = 1;
}

// add the handlers into the idt
void setup_idt() {
    // int i;  HEARD PATRICK TELLING SOMEONE TO JUST DO IT 20 TIMES INSTEAD OF LOOP CAUSE ITS EASIER
    // for (i = 0; i < len(exceptions); i++) {
    //     SET_IDT_ENTRY(idt[0], exceptions[0]);
    // }

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

    //pic irqs
    //SET_IDT_ENTRY(idt[KEYBOARD_VECTOR], )

    //system calls
    SET_IDT_ENTRY(idt[SYSTEM_CALL_VECTOR], generic_system_call_handler_lnk);
}

 void generic_handler(int vector) {
    if(vector < 0 || vector > 19) {
        generic_intel_handler(vector);
    }
    else if(vector < IRQ1_VECTOR || vector > IRQ15_VECTOR) {
        generic_irq_handler(vector);
    }
    else if(vector == SYSTEM_CALL_VECTOR) {
        generic_system_call_handler();
    }
 }
// test to make one handler for all intel exceptions 0-19 to just print and sit in a while loop
void generic_intel_handler(int vector) {
    if(vector < 0 || vector > 19) {
        return; // invalid vector number for this function
    }
    printf("%s", intel_handler_strings[vector]);
    while(1); // infinite loop here for now, supposed to have this according to slides???
}

void generic_irq_handler(int vector) {
    if(vector < IRQ1_VECTOR || vector > IRQ15_VECTOR) {
        return; //invalid vector number for this function
    }
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

void generic_system_call_handler() {
    printf("System call");
}

