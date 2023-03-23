#ifndef RTC_H
#define RTC_H

#include "types.h"
#include "i8259.h"
#include "lib.h"

#define RTC_REG_PORT        0x70
#define RTC_RW_PORT         0x71
#define RTC_RATE            0x03

#define RTC_REG_A           0x0A
#define RTC_REG_A_DISABLE   0x8A
#define RTC_REG_B           0x0B
#define RTC_REG_B_DISABLE   0x8B   
#define RTC_REG_C           0x0C
#define RTC_REG_C_DISABLE   0x8C

static int rtc_irq_flag;

extern void rtc_init();

extern void rtc_irq_handler();

extern int rtc_open();

extern int rtc_close();

extern int rtc_read();

extern int rtc_write(uint32_t buffer);



#endif /*RTC_H*/