#ifndef PIT_H
#define PIT_H

#include "lib.h"
#include "i8259.h"

#define PIT_CH_ZERO     0x40
#define PIT_CMD_REG     0x43

#define PIT_INIT_BITS   0x34
#define PIT_MAX_HZ      1193182

#define LOWER_BYTE      0x00FF
#define UPPER_BYTE      0xFF00
#define SHIFT_BYTE      8

#define ENABLE_IRQ      0

void pit_init(uint16_t desired_frequency);


#endif /*pit*/
