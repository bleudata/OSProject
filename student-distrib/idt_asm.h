
#ifndef IDT_ASM_H
#define IDT_ASM_H

#define SYSTEM_CALL_VECTOR 0x80
#define DIVIDE_ERROR    0
#define RESERVED1    1
#define NMI_INTERRUPT 2
#define BREAKPOINT    3
#define OVERFLOW   4
#define BOUND_RANGE_EXCEEDED    5
#define INVALID_OPCODE   6
#define DEVICE_NA 7
#define DOUBLE_FAULT  8
#define RESERVED9  9
#define INVALID_TSS  10
#define SEG_NOT_PRESENT   11
#define STACK_SEG_FAULT 12
#define GENERAL_PROTECTION  13
#define PAGE_FAULT  14
#define RESERVED15  15
#define FPU_FLOAT_ERROR 16
#define ALIGNMENT_CHECK  17
#define MACHINE_CHECK  18
#define SIMD_FLOAT_EXCEPTION  19

#ifndef ASM


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


extern void divide_error_handler_lnk();
extern void reserved1_handler_lnk();
extern void nmi_interrupt_handler_lnk();
extern void breakpoint_handler_lnk();
extern void overflow_handler_lnk();
extern void bound_range_exceeded_handler_lnk();
extern void invalid_opcode_handler_lnk();
extern void device_na_handler_lnk();
extern void double_fault_handler_lnk();
extern void reserved9_handler_lnk();
extern void invalid_tss_handler_lnk();
extern void seg_not_present_handler_lnk();
extern void stack_set_fault_handler_lnk();
extern void general_protection_handler_lnk();
extern void page_fault_handler_lnk();
extern void reserved15_handler_lnk();
extern void fpu_float_error_handler_lnk();
extern void alignment_check_handler_lnk();
extern void machine_check_handler_lnk();
extern void smid_float_exception_handler_lnk();

#endif /* ASM */
#endif /* IDT_ASM_H */
