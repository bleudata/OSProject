#ifndef _IDT_H
#define _IDT_H



// enum intel_exceptions {
//     DIVIDE_ERROR = 0,
//     RESERVED1,
//     NMI_INTERRUPT,
//     BREAKPOINT,
//     OVERFLOW,
//     BOUND_RANGE_EXCEEDED,
//     INVALID_OPCODE,
//     DEVICE_NA,
//     DOUBLE_FAULT, // error code is 0
//     RESERVED9,
//     INVALID_TSS, //yes
//     SEG_NOT_PRESENT, //yes
//     STACK_SEG_FAULT, //yes
//     GENERAL_PROTECTION, //yes
//     PAGE_FAULT, //yes
//     RESERVED15,
//     FPU_FLOAT_ERROR,
//     ALIGNMENT_CHECK, // error code is 0
//     MACHINE_CHECK,
//     SIMD_FLOAT_EXCEPTION
// };

// should exceptions also return 0 on success and -1 on fail?
// that's what system calls are doing

extern void idt_init();
extern void setup_idt();
extern void generic_handler(int vector);
extern void generic_intel_handler(int vector);
// extern void divide_error_handler();
// extern void reserved1_handler();
// extern void nmi_interrupt_handler();
// extern void breakpoint_handler();
// extern void overflow_handler();
// extern void bound_range_exceeded_handler();
// extern void invalid_opcode_handler();
// extern void device_na_handler();
// extern void double_fault_handler();
// extern void reserved9_handler();
// extern void invalid_tss_handler();
// extern void seg_not_present_handler();
// extern void stack_set_fault_handler();
// extern void general_protection_handler();
// extern void page_fault_handler();
// extern void reserved15_handler();
// extern void fpu_float_error_handler();
// extern void alignment_check_handler();
// extern void machine_check_handler();
// extern void smid_float_exception_handler();
extern void generic_system_call_handler();


#endif /* _IDT_H */
