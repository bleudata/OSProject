.align 4096


.text
.globl context_switch

# a
context_switch:
    movl 4(%esp), %eax
    pushl USER_DS # push user SS
    pushl PROGRAM_END
    pushfl # push flags
    pushl USER_CS # push user cs
    pushl %eax  # push EIP

    iret


context_switch_1:
    movl 4(%esp), %eax
    pushl %eax # push user SS

    movl 8(%esp), %eax
    pushl %eax # push new process esp

    pushfl # push flags

    movl 12(%esp), %eax
    pushl %eax # push user cs

    movl 16(%esp), %eax
    pushl %eax  # push EIP
    iret

# asm for halt
system_halt:
    
    iret