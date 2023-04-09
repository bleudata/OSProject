#define ASM 1

.align 4096


.text
.globl context_switch


# asm for halt
system_halt:
    
    iret

# system call
# vector number is 0x80
.global system_call_handler_lnk 
system_call_handler_lnk:
    # Save the registers
	pushl %ebx
	pushl %ecx
	pushl %edx
    pushl %esi
    pushl %edi
	pushl %ebp

	pushfl

	cmpl $1, %eax
    jl INVALID_SYS_NUM
    cmpl $10, %eax
    jg INVALID_SYS_NUM
    addl $-1, %eax

    # Always push all three arguments 
    pushl %edx
    pushl %ecx
	pushl %ebx # where the parameter is
    call *syscall_jumptable(,%eax, 4) 
    # Pop arguments off stack
    addl $12, %esp

    jmp VALID_SYS_NUM

INVALID_SYS_NUM:
    movl $-1, %eax
VALID_SYS_NUM:
	popfl

    # Pop registers 
	popl %ebp
    popl %edi
	popl %esi
    popl %edx
    popl %ecx
    popl %ebx


	iret

# jump table for system calls
syscall_jumptable:
    .long   halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn


