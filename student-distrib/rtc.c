#include "rtc.h"

static int rtc_irq_flag;

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
    outb(RTC_REG_PORT, RTC_REG_A_DISABLE);		// reset index to A
    outb(RTC_RW_PORT, (prev & 0xf0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits.
    return 0;
}

int rtc_close(){
    return 0;
}

int rtc_read(){
    while(rtc_irq_flag != 1);
    rtc_irq_flag = 0;
    return 0;
}

int rtc_write(uint32_t buffer){
    return 0;
}

