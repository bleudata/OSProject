
#include "paging.h"
#include "terminal_driver.h"
#include "keyboard_driver.h"
#include "lib.h"

//create page directory for kernel
page_directory_entry page_directory[DIR_SIZE] __attribute__((aligned(FOUR_KB)));


// Creates the page table that hold 0 - 4MB  section of Virtual Memory
page_table_entry video_memory[TABLE_SIZE] __attribute__((aligned(FOUR_KB))); 


// DELETE SOON cause we want to use vidmap to write to one of the 3 terminal video memories
// This is the video
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

    //Initialize the different video memory
    //initialize first page table to all not present
    for(i = 0; i< TABLE_SIZE; i++){
        
        video_memory[i].entry = BLANK_ENTRY;

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
    page_directory[0].entry = (uint32_t)(video_memory) | VMEM_ENTRY_SET;
    video_memory[VMEM_OFFSET].pt_fields.present = 1;
    video_memory[VMEM_OFFSET].pt_fields.read_write = 1;
    video_memory[VMEM_OFFSET].pt_fields.page_address = VMEM_OFFSET;

    // Sets up the virtual memory for terminal one
    video_memory[VMEM_OFFSET_T0].pt_fields.present = 1;
    video_memory[VMEM_OFFSET_T0].pt_fields.read_write = 1;
    video_memory[VMEM_OFFSET_T0].pt_fields.page_address = VMEM_OFFSET_T0;

    // Sets up the virtual memory for terminal two
    video_memory[VMEM_OFFSET_T1].pt_fields.present = 1;
    video_memory[VMEM_OFFSET_T1].pt_fields.read_write = 1;
    video_memory[VMEM_OFFSET_T1].pt_fields.page_address = VMEM_OFFSET_T1;

    // Sets up the virtual memory for terminal three
    video_memory[VMEM_OFFSET_T2].pt_fields.present = 1;
    video_memory[VMEM_OFFSET_T2].pt_fields.read_write = 1;
    video_memory[VMEM_OFFSET_T2].pt_fields.page_address = VMEM_OFFSET_T2;

    
    //load page dir to %cr3, enable mixed size pages and turn on paging
    load_page_dir((uint32_t *)page_directory);
    enable_mixed_size();
    enable_paging();
}

/*
 * map_helper()
 *   DESCRIPTION: maps virtual memory 128 MB to the correct 4MB page in physical memory depending on the pid
 *   INPUTS: pid - is a process ID BRO
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
    // uint32_t base_addr = ((page_directory[pd_offset]).entry)[pt_offset].pt_fields.page_address;
    base_addr = base_addr << 12;
    terminal_t* terminal = get_active_terminal();
    unsigned char terminal_num = terminal->number;
    
    // currently on active terminal, make sure lib.c video_mem points to VIDEO
    if((terminal->storage_offset) == pt_offset) {
        //set lib.c video_mem to VIDEO
        return;
    }
    // on a background terminal, set lib.c video_mem to storage addr
    if(pt_offset == VMEM_OFFSET_T0) {
        // video_mem = (unsigned char * ) T0_VIRTUAL_ADDR
    }
    else if (pt_offset == VMEM_OFFSET_T1) {
        // video_mem = (unsigned char * ) T0_VIRTUAL_ADDR
    }
    else if (pt_offset == VMEM_OFFSET_T2) {
        // video_mem = (unsigned char * ) T0_VIRTUAL_ADDR    
    }
    flush_tlb();
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
void copy_video_memory(unsigned char * source, unsigned char * destination) {
    int i;
    for (i = 0; i < SCREEN_BYTES; i++) {
        destination[i] = source[i];
    }
}


