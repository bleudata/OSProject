#include "pit.h"

void pit_init(uint16_t desired_frequency){
    // 00 11 010 0
    outb(0x34, 0x43);
    uint16_t counter_value = 1193182 / desired_frequency;
    outb(0x00FF & counter_value, 0x40);
    outb(0xFF00 & counter_value, 0x40);
    enable_irq(0x0);
}
