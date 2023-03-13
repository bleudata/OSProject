#ifndef IDT_ASM_H
#define IDT_ASM_H


#define SYSTEM_CALL_VECTOR 0x80
#define KEYBOARD_VECTOR  0x21
#define RTC_VECTOR   0x28


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
#define PUSH_NEW_EC 2  // push a
#define PUSH_DUMMY_EC 1 // need to push dummy error code, ec = error code
#define DONT_PUSH_EC 0 // already pushed error code, don't need to ec = error code

#ifndef ASM

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
extern void generic_system_call_handler_lnk();
extern void keyboard_handler_lnk();
extern void rtc_handler_lnk();

#endif /* ASM */
#endif /* IDT_ASM_H */
