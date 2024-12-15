#include <stdio.h>
#include "interrupt.h"

void handle_interrupt(int interrupt_type) {
    printf("Manejando interrupción tipo %d\n", interrupt_type);
}

