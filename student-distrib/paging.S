.align 4096


.text
.globl load_page_dir


# loads page directory address into cr3, takes one argument (address of page directory) 
load_page_dir:
    push %ebp
    movl %esp, %ebp
    movl 8(%ebp), %eax
    movl %eax, %cr3
    leave
    ret



.globl enable_paging

# set the paging bit (32nd bit in the CR0 register)
enable_paging:
    push %ebp
    movl %esp, %ebp
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0
    leave
    ret



.globl enable_mixed_size

# enable mixed size pages (4kb and 4MB) setting bit 4 of cr4
enable_mixed_size:
    push %ebp
    movl %esp, %ebp
    movl %cr4, %eax
    orl $0x00000010, %eax
    movl %eax, %cr4
    leave
    ret





