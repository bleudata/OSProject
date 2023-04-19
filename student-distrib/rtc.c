#include "rtc.h"

static int rtc_irq_flag;
static uint32_t rtc_syshz_per_uhz;
static uint32_t rtc_ctr;

/*
 * rtc_init
 *   DESCRIPTION: initializes periodic rtc interrupts and enables rtc irq on the PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enables period rtc interrupts, enables rtc irq on PIC, 
 *                 temporarily disables nmi, but re-enabled at end of function
 */
void rtc_init() {
    outb(RTC_REG_B_DISABLE, RTC_REG_PORT); // set to register b and disable nmi
    char prev = inb(RTC_RW_PORT); // get current value from register b
    outb(RTC_REG_B_DISABLE, RTC_REG_PORT); // set to register b and disable nmi
    outb(prev | RTC_PER_IRQ, RTC_RW_PORT); // set bit 6 to 1 using 0x40 to enable periodic interrupts
    enable_irq(RTC_IRQ);
    nmi_enable();
}

/*
 * rtc_irq_handler
 *   DESCRIPTION: calls the test_interrupts function from lib.c when an rtc interrupt occurs
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints various characters to the screen based on test_interrupts function
 */
void rtc_irq_handler() {
    int result;
    //test_interrupts();
    outb(RTC_REG_C, RTC_REG_PORT); // select register c
    result = inb(RTC_RW_PORT); // need to read from c register or the interrupt won't happen again
    send_eoi(RTC_IRQ);
    if(rtc_ctr < rtc_syshz_per_uhz){
        rtc_ctr++;
    }
    else{
        rtc_ctr = 1;
        rtc_irq_flag = RTC_FLAG_SET;
    }
}

/*
 * rtc_open
 *   DESCRIPTION: opens the RTC on the user end and sets it to the default of 1024 Hz
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: fd on success, -1 if fail
 *   SIDE EFFECTS: accesses RTC registers and sets frequency to 1024 Hz
 */

int32_t rtc_open(const uint8_t* filename){

    if(filename == NULL){
        return -1;
    }

    uint16_t rate = RTC_RATE;
    outb(RTC_REG_A_DISABLE, RTC_REG_PORT); // set index to register A, disable NMI
    char prev = inb(RTC_RW_PORT);	// get initial value of register A, should be 32kHz
    outb(RTC_REG_A_DISABLE, RTC_REG_PORT);		// reset index to A
    outb((prev & RTC_DATA_UPPER_BYTE) | rate, RTC_RW_PORT); //write only our rate to A. Note, rate is the bottom 4 bits.
    rtc_ctr = 1;
    rtc_syshz_per_uhz = RTC_USR_DEFAULT; //since its init, the system frequency of interrupts will always be 1 of itself
    
    d_entry dentry;
    int32_t dentry_success = read_dentry_by_name(filename, &dentry);

    if(dentry_success == -1){
        return -1;
    }
   
    return 0;
}

/*
 * rtc_close
 *   DESCRIPTION: closes the RTC on the user end and sets it to the default of 1024 Hz, or setting the syshz per uhz rate back to 1 and counter to 0
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: resets global variables to reflect a clock rate of 1024 Hz from the user perspective
 */

int32_t rtc_close(int32_t fd){

    if(fd < FD_INIT_SIZE || fd > FD_MAX_SIZE){
        return -1;
    }

    terminal_write(1, "in rtc \n", 9);
    rtc_ctr = 1;
    rtc_syshz_per_uhz = RTC_USR_DEFAULT; //reset the # of interrupts per system freq to be just 1, so its itself
    return RTC_PASS;
}

/*
 * rtc_read
 *   DESCRIPTION: a function that returns once the interrupt of the RTC is processed/sent/handled a.k.a "gives" the user the tick rate
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: resets the global rtc irq flag to 0 after an interrupt is processed
 */

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){

    if(fd < FD_INIT_SIZE || fd > FD_MAX_SIZE){
        return -1;
    }
    if(buf == NULL){
        return -1;
    }
    if(nbytes < 0){
        return -1;
    }
    rtc_irq_flag = RTC_GLOB_RES_VAR;
    while(rtc_irq_flag != RTC_FLAG_SET);
    rtc_irq_flag = RTC_GLOB_RES_VAR;
    return RTC_PASS;
}

/*
 * rtc_write
 *   DESCRIPTION: writes a new frequency to the RTC from the user; is limited to be between 2 and 1024 Hz, and must be passed through a buffer.
 *                This is virtualized, therefore it only affects how many system interrupts are processed as one user interrupt
 *   INPUTS: 4-byte buffer, buffer
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 if buffer is of incorrect size or any of the other conditions are unture (not a power of 2, requested frequency too big or too small)
 *   SIDE EFFECTS: writes to the syshz per uhz variable and affects the frequency at which rtc_read() returns, but does not actually change the system RTC frequency
 */

int32_t rtc_write(int32_t fd, const void *buf, int32_t nbytes){
    /*checking to make sure buffer is exactly 4 bytes, otherwise return -1*/
    uint32_t *buffer = (uint32_t*)buf;

    if(nbytes != RTC_BUFF_SIZE){
        return RTC_FAIL;
    }
    if(buf == NULL){
        return -1;
    }
    if(fd < FD_INIT_SIZE || fd > FD_MAX_SIZE){
        return -1;
    }

    /*if buffer is a valid size, calculate the requested frequency from the buffer*/ 
    uint32_t req_freq = *buffer;

    /*checking that the requested user frequency is between 2 Hz and 1024 Hz inclusive and is a power of 2, otherwise return -1*/
    if(req_freq < RTC_HZ_MIN || req_freq > RTC_HZ || (req_freq & (req_freq - RTC_POW_2_DECR)) != 0){
        return RTC_FAIL;
    }

    /*Should load the ratio of 1 system cycle per however many requested user cycles*/
    rtc_syshz_per_uhz = RTC_HZ / req_freq;
    return RTC_PASS;
}
