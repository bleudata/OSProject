#ifndef _IDT_H
#define _IDT_H

// initialize the idt
extern void idt_init();
// place exception / interrupt handlers into the idt
extern void setup_idt();
// umbrella function to determine which interrupt handler to use
extern void generic_handler(int vector);
// handler for vectors 0-19
extern void generic_intel_handler(int vector);



#endif /* _IDT_H */
