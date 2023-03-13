#ifndef _IDT_H
#define _IDT_H

extern void idt_init();
extern void setup_idt();
extern void generic_handler(int vector);
extern void generic_intel_handler(int vector);

extern void generic_system_call_handler();


#endif /* _IDT_H */
