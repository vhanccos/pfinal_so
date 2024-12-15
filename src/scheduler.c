#include <stdio.h>
#include "scheduler.h"
#include "process.h"

void run_scheduler() {
    printf("Ejecutando planificador...\n");
}

void add_process_to_scheduler(Process *p) {
    printf("Añadiendo proceso %s al planificador...\n", p->name);
}

