#include "rtc.h"

static int rtc_irq_flag;
static int rtc_uhz_per_syshz = 1;

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
    outb(prev | 0x40, RTC_RW_PORT); // set bit 6 to 1 using 0x40 to enable periodic interrupts
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
    test_interrupts();
    outb(RTC_REG_C, RTC_REG_PORT); // select register c
    result = inb(RTC_RW_PORT); // need to read from c register or the interrupt won't happen again
    send_eoi(RTC_IRQ);
    rtc_irq_flag = 1;
}

int rtc_open(){
    uint16_t rate = RTC_RATE;
    outb(RTC_REG_A_DISABLE, RTC_REG_PORT); // set index to register A, disable NMI
    char prev = inb(RTC_RW_PORT);	// get initial value of register A, should be 32kHz
    outb(RTC_REG_A_DISABLE, RTC_REG_PORT);		// reset index to A
    outb(RTC_RW_PORT, (prev & 0xf0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits.

    rtc_uhz_per_syshz = 1; //since its init, the system frequency of interrupts will always be 1 of itself

    return 0;
}

int rtc_close(){
    rtc_uhz_per_syshz = 1; //reset the # of interrupts per system freq to be just 1, so its itself
    return 0;
}

int rtc_read(){
    while(rtc_irq_flag != 1);
    rtc_irq_flag = 0;
    return 0;
}

int rtc_write(uint8_t *buffer){
    /*checking to make sure buffer is exactly 4 bytes, otherwise return -1*/
    if(sizeof(buffer) != RTC_BUFF_SIZE){
        return -1;
    }

    /*if buffer is a valid size, calculate the requested frequency from the buffer*/ 
    uint32_t req_freq = 0;
    int i;
    for(i = RTC_BUFF_SIZE - 1; i > 0; i--){
        req_freq = req_freq + buffer[i];
        req_freq = req_freq << RTC_BUFF_SIZE;
    }

    /*checking that the requested user frequency is between 2 Hz and 1024 Hz inclusive and is a power of 2, otherwise return -1*/
    if(req_freq < RTC_HZ_MIN || req_freq > RTC_HZ || (req_freq & (req_freq - 1)) != 0){
        return -1;
    }

    /*Should load the ratio of 1 system cycle per however many requested user cycles*/
    rtc_uhz_per_syshz = RTC_HZ / req_freq; 
    return 0;
}

int rtc_get_uHz(){
    return rtc_uhz_per_syshz;
}