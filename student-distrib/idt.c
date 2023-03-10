/* This file will hold all functions relating to IDT */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"

// Make an array of function pointers
void (*excpetions[1]) = {&divide_error_handler};


void setup_idt() {
    int i;
    for (i = 0; i < len(excpetions); i++) {
        SET_IDT_ENTRY(idt[0], exception[0]);
    }
}

void divide_error_handler() {
    printf("Divide Error.");
}