
#define ASM 1
#include "idt_asm.h"


/*
 * HANDLER_LINK
 *   DESCRIPTION: Macro to create exception / interrupt assmebly linkage using handler functions
 *   INPUTS: name -- name of the new c function for the handler with assembly linkage
 * 			 function -- handler function to call 
 * 			 vector -- the vector number of the exception / interrupt 
 */
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
.text
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
HANDLER_LNK(page_fault_handler_lnk, generic_handler, 14) # will likely need to change this to later, see PAGE FAULT SAMPLE 
HANDLER_LNK(reserved15_handler_lnk, generic_handler, 15)
HANDLER_LNK(fpu_float_error_handler_lnk, generic_handler, 16)
HANDLER_LNK(alignment_check_handler_lnk, generic_handler,  17)
HANDLER_LNK(machine_check_handler_lnk, generic_handler, 18)
HANDLER_LNK(smid_float_exception_handler_lnk, generic_handler, 19)

# pic irq vectors - keyboard and rtc
# these might need to push own unique error code, to be figured out later.
HANDLER_LNK(keyboard_handler_lnk, generic_handler, KEYBOARD_VECTOR) 
HANDLER_LNK(rtc_handler_lnk, generic_handler, RTC_VECTOR)

# system call 
# also might need to push own unique error code, to be figured out later.
# HANDLER_LNK(generic_system_call_handler_lnk, generic_handler, SYSTEM_CALL_VECTOR)


# system call
# vector number is 0x80
.global system_call_handler_lnk 
system_call_handler_lnk:
	pushal
	pushfl
	pushl $SYSTEM_CALL_VECTOR
	call generic_handler
	addl $4, %esp
	popfl
	popal
	iret



/* PAGE FAULT SAMPLE
 .globl page_fault_handler_lnk
  page_fault_lnk:
	pushal
	pushfl
	pushl $14
	call generic_handler
	addl $4,%esp
	popfl
	popal
	iret
*/

