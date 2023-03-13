#define ASM
#include "idt_asm.h"
/*#include "i8259.h"*/

/* will need to add macro above pushal for pushing vector num and error code from pic */
#define HANDLER_LNK(name, function, vector)   \
	.GLOBL name               		 ;\
	name:						 	 ;\
		PUSHAL						 ;\
		PUSHFL 						 ;\
		PUSHL $vector				 ;\
		CALL function   			 ;\
		ADDL $4, %ESP				 ;\
		POPFL	          			 ;\
		POPAL	         			 ;\
		IRET						 ;\

# the exception library wrappers 

# vectors 0-19, in this section these aren't magic numbers, they're just the vector numbers....
HANDLER_LNK(divide_error_handler_lnk, generic_handler, 0)
HANDLER_LNK(reserved1_handler_lnk, generic_handler, 1)
HANDLER_LNK(nmi_interrupt_handler_lnk, generic_handler, 2)
HANDLER_LNK(breakpoint_handler_lnk, generic_handler, 3)
HANDLER_LNK(overflow_handler_lnk, generic_handler, 4)
HANDLER_LNK(bound_range_exceeded_handler_lnk, generic_handler, 5)
HANDLER_LNK(invalid_opcode_handler_lnk, generic_handler, 6)
HANDLER_LNK(device_na_handler_lnk, generic_handler, 7)
HANDLER_LNK(double_fault_handler_lnk, generic_handler, 8)
HANDLER_LNK(reserved9_handler_lnk, generic_handler, 9)
HANDLER_LNK(invalid_tss_handler_lnk, generic_handler, 10)
HANDLER_LNK(seg_not_present_handler_lnk, generic_handler, 11)
HANDLER_LNK(stack_set_fault_handler_lnk, generic_handler, 12)
HANDLER_LNK(general_protection_handler_lnk, generic_handler,  13)
HANDLER_LNK(page_fault_handler_lnk, generic_handler, 14)
HANDLER_LNK(reserved15_handler_lnk, generic_handler, 15)
HANDLER_LNK(fpu_float_error_handler_lnk, generic_handler, 16)
HANDLER_LNK(alignment_check_handler_lnk, generic_handler,  17)
HANDLER_LNK(machine_check_handler_lnk, generic_handler, 18)
HANDLER_LNK(smid_float_exception_handler_lnk, generic_handler, 19)

# pic irq vectors - keyboard and rtc
# these might need to push own unique error code, to be figured out later....
HANDLER_LNK(keyboard_handler_lnk, generic_handler, KEYBOARD_VECTOR) 
HANDLER_LNK(rtc_handler_lnk, generic_handler, RTC_VECTOR)

# system call 
# also might need to push own unique error code, to be figured out later...
HANDLER_LNK(generic_system_call_handler_lnk, generic_handler, SYSTEM_CALL_VECTOR)

/*
HANDLER_LNK(divide_error_handler_lnk, divide_error_handler)
HANDLER_LNK(reserved1_handler_lnk, reserved1_handler)
HANDLER_LNK(nmi_interrupt_handler_lnk, nmi_interrupt_handler)
HANDLER_LNK(breakpoint_handler_lnk, breakpoint_handler)
HANDLER_LNK(overflow_handler_lnk, overflow_handler)
HANDLER_LNK(bound_range_exceeded_handler_lnk, bound_range_exceeded_handler)
HANDLER_LNK(invalid_opcode_handler_lnk, invalid_opcode_handler)
HANDLER_LNK(device_na_handler_lnk, device_na_handler)
HANDLER_LNK(double_fault_handler_lnk, double_fault_handler)
HANDLER_LNK(reserved9_handler_lnk, reserved9_handler)
HANDLER_LNK(invalid_tss_handler_lnk, invalid_tss_handler)
HANDLER_LNK(seg_not_present_handler_lnk, seg_not_present_handler)
HANDLER_LNK(stack_set_fault_handler_lnk, stack_set_fault_handler)
HANDLER_LNK(general_protection_handler_lnk, general_protection_handler)
HANDLER_LNK(page_fault_handler_lnk, page_fault_handler)
HANDLER_LNK(reserved15_handler_lnk, reserved15_handler)
HANDLER_LNK(fpu_float_error_handler_lnk, fpu_float_error_handler)
HANDLER_LNK(alignment_check_handler_lnk, alignment_check_handler)
HANDLER_LNK(machine_check_handler_lnk, machine_check_handler)
HANDLER_LNK(smid_float_exception_handler_lnk, smid_float_exception_handler)
*/

/* Call the main() function, then halt with its return value. */

