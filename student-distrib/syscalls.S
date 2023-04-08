#define ASM 1

.align 4096


.text
.globl context_switch

/*
context_switch_1:
    movl 4(%esp), %eax
    pushl USER_DS # push user SS
    pushl PROGRAM_END
    pushfl # push flags
    pushl USER_CS # push user cs
    pushl %eax  # push EIP

    iret
*/

/*
context_switch:
    pushl $0x002B # push user DS
    pushl esp_start # push new process esp
    pushfl # push flags
    pushl $0x0023 # push user cs
    pushl entry_point # push EIP
    iret
*/
# asm for halt
system_halt:
    
    iret

# system call
# vector number is 0x80
.global system_call_handler_lnk 
system_call_handler_lnk:
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

	cmpl $3, %eax
	jne INVALID_SYS_NUM # for now chnage when we need other sys calls

    pushl %edx
    pushl %ecx
	pushl %ebx # where the parameter is
    call *syscall_jumptable(,%eax, 4) 
    addl $12, %esp

    jmp VALID_SYS_NUM

INVALID_SYS_NUM:
    movl $-1, %eax
VALID_SYS_NUM:
	popfl

	popl %ebp
    popl %edi
	popl %esi
    popl %edx
    popl %ecx
    popl %ebx


	iret


syscall_jumptable:
    .long   halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn


