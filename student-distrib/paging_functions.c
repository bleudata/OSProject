
#include "paging.h"



//create page directory for kernel
page_directory_entry page_directory[DIR_SIZE] __attribute__((aligned(FOUR_KB)));


//create page table for video memory
page_table_entry first_page_table[TABLE_SIZE] __attribute__((aligned(FOUR_KB))); 

page_table_entry user_vid_mem[TABLE_SIZE] __attribute__((aligned(FOUR_KB)));

/*
 * init_paging()
 *   DESCRIPTION: initializes kernel's page dir and page table, calls page initalization functions
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes paging for kernel :)
 */
void init_paging() {
    
    //initialize page directory to all not present
    int i;
    for(i = 0; i < DIR_SIZE; i++){
        page_directory[i].entry = BLANK_ENTRY;

    }

    //initialize first page table to all not present
    for(i = 0; i< TABLE_SIZE; i++){
        
        first_page_table[i].entry = BLANK_ENTRY;

    }

    // 0000 0000 0100 0000  0000 0000 0000 0000 (4mb) 2^22
    // 0x0040 0000 (virtual) -> 0x0040 0000 (physical) (for kernel) (4MB page)
    // 0x000B 8000 (virtual) -> 0x000B 8000 (physical) (for video memory) (4KB page) 2^12

    //initialize 4MB page for kernel
    page_directory[1].fourmb.present = 1;
    page_directory[1].fourmb.read_write = 1;
    page_directory[1].fourmb.page_size = 1;
    page_directory[1].fourmb.page_address = 1;

    //initialize 4KB page for video memory
    page_directory[0].entry = (uint32_t)(first_page_table) | VMEM_ENTRY_SET;
    first_page_table[VMEM_OFFSET].pt_fields.present = 1;
    first_page_table[VMEM_OFFSET].pt_fields.read_write = 1;
    first_page_table[VMEM_OFFSET].pt_fields.page_address = VMEM_OFFSET;

    //initialize 4kb buffer kernel mapping for terminal 0,1 and 2 
    int terminal_num;
    for(terminal_num = 0; terminal_num < 3; terminal_num++){ // check privilege mapping
        page_directory[0].entry = (uint32_t)(first_page_table) | VMEM_ENTRY_SET;
        first_page_table[VMEM_OFFSET+terminal_num+1].pt_fields.present = 1;
        first_page_table[VMEM_OFFSET+terminal_num+1].pt_fields.read_write = 1;
        first_page_table[VMEM_OFFSET+terminal_num+1].pt_fields.page_address = VMEM_OFFSET + terminal_num+1;
    }
    
    
    //load page dir to %cr3, enable mixed size pages and turn on paging
    load_page_dir((uint32_t *)page_directory);
    enable_mixed_size();
    enable_paging();
}

/*
 * map_helper()
 *   DESCRIPTION: maps virtual memory 128 MB (program memory) to the correct 4MB page in physical memory depending on the pid
 *   INPUTS: pid - is a process ID 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none 
 */
void map_helper(uint32_t pid) {
    // get the offset into the page directory
    // uint32_t dir_offset = virtual_address >> 22;
    // uint32_t pde = page_directory[dir_offset];

    // // After getting the PDE we have to zero out non address bits
    int32_t virtual = VIRT_MEM_PAGE;
    virtual = virtual >> VIRT_MEM_SHIFT;
    page_directory[virtual].fourmb.page_address = 2 + 1*pid;  //= PHYS_MEM_BASE + PHYS_MEM_OFF*pid;
    page_directory[virtual].fourmb.present = 1;
    page_directory[virtual].fourmb.read_write = 1;
    page_directory[virtual].fourmb.user_supervisor = 1; 
    page_directory[virtual].fourmb.page_size = 1;
    
    //flush the TLB
    flush_tlb();

}

/*
 * destroy_mapping()
 *   DESCRIPTION: destroy the 128MB mapping
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void destroy_mapping(){
    int32_t virtual = VIRT_MEM_PAGE;
    virtual = virtual >> VIRT_MEM_SHIFT;
    page_directory[virtual].entry = 0x0;
    flush_tlb();
}

/*
 * vidmap_helper()
 *   DESCRIPTION: sets up page table and page dir and modifies directory to have this pte mapped to kernel vidmem
 *   INPUTS: pid
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void vidmap_helper(uint32_t virtual_address){
    uint32_t virtual = virtual_address; 
    uint32_t pd_offset = virtual >> VIRT_MEM_SHIFT;
    uint32_t pt_offset = (virtual & PT_INDEX_MAP) >>12; 
    page_directory[pd_offset].entry = (uint32_t)(user_vid_mem) | 7; 

    user_vid_mem[pt_offset].pt_fields.user_supervisor = 1;
    user_vid_mem[pt_offset].pt_fields.present = 1;
    user_vid_mem[pt_offset].pt_fields.read_write = 1;
    //should depend on which terminal process is on and which terminal user is on
    if(get_cur_user_terminal() == get_cur_sched_terminal()){
        user_vid_mem[pt_offset].pt_fields.page_address = VMEM_OFFSET;
    }else{
        user_vid_mem[pt_offset].pt_fields.page_address = VMEM_OFFSET + get_cur_sched_terminal() + 1;
    }
    
    flush_tlb();
}

// set user vid mem to point to terminal buffer
void vidmap_change(uint32_t virtual_address, uint32_t terminal){
    uint32_t pd_offset = virtual_address >> VIRT_MEM_SHIFT;
    uint32_t pt_offset = (virtual_address & PT_INDEX_MAP) >>12; 
    page_directory[pd_offset].entry = (uint32_t)(user_vid_mem) | 7; 

    user_vid_mem[pt_offset].pt_fields.user_supervisor = 1;
    user_vid_mem[pt_offset].pt_fields.present = 1;
    user_vid_mem[pt_offset].pt_fields.read_write = 1;
    user_vid_mem[pt_offset].pt_fields.page_address = VMEM_OFFSET + terminal + 1;
    flush_tlb();
}

// swap terminal buffer and video memory for two terminals
void buffer_swap(uint32_t old_terminal, uint32_t new_terminal){
    //vid mem to old_terminal , maybe do for loop if slow
    //memcpy((void*)(VIDMEM + FOUR_KB + FOUR_KB*old_terminal),  (void*)VIDMEM   , FOUR_KB);
    copy_video_memory((unsigned char *)(VIDMEM + FOUR_KB + FOUR_KB*old_terminal),  (unsigned char *)VIDMEM  );
    
    //new_terminal to vidmem
    //memcpy((void*)VIDMEM   , (void*)(VIDMEM + FOUR_KB + FOUR_KB*new_terminal)  , FOUR_KB);
    copy_video_memory((unsigned char *)VIDMEM  ,   (unsigned char *)(VIDMEM + FOUR_KB + FOUR_KB*new_terminal)  );
}

/*
 * copy_video_memory()
 *   DESCRIPTION: Copys a 4KB block of memory from source to destination
 *   INPUTS: source - 4KB block to copy from
 *           destination - 4KB block to copy to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes physical memory
 */
void copy_video_memory(unsigned char * destination, unsigned char * source) {
    int i;
    for (i = 0; i < 4096; i++) {
        destination[i] = source[i];
    }
}






//todo
//check to destroy memory mapping in halt (for vidmap and stuff)
//check if about popal and eax in interrupt linkage
//need to update setting the top task of the terminal

// terminal write mapping
// extra keyboard buffer and keyboard buffer mapping
// buffer swapping
// PIT interrupt initialization
// make sure the buffer switching works for all cases
// check what int 0x80 does
// difference between handler_lnk and do_call
// clarify what happens during interrupt hit, and difference between syscall 
// make sure that old ebp value doesnt really matter since we are just popping and iret (sched)

// what happens when interrupt occurs when executing system call, does it just push the
// iret context above the kernelstack it is working on

//int0x80 for system calls, when doing iret we are going back to the user space but isnt the asm code stored in the kernel?

// cursor positions
