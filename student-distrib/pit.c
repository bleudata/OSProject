#include "pit.h"

void pit_init(uint16_t desired_frequency){
    //setting PIT CMD REG bits
    outb(PIT_INIT_BITS, PIT_CMD_REG);

    //setting PIT at desired Hz
    uint16_t counter_value = PIT_MAX_HZ / desired_frequency;
    outb(LOWER_BYTE & counter_value, PIT_CH_ZERO);
    outb((UPPER_BYTE & counter_value) >> SHIFT_BYTE, PIT_CH_ZERO);
    
    enable_irq(ENABLE_IRQ);
}
