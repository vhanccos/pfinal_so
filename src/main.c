#include <stdio.h>
#include "scheduler.h"
#include "process.h"
#include "resource.h"
#include "interrupt.h"
#include "memory.h"

int main() {
    // Inicialización de módulos
    init_resources();
    init_memory();
    
    // Crear y ejecutar algunos procesos
    Process *p1 = create_process("Proceso 1");
    Process *p2 = create_process("Proceso 2");

    add_process_to_scheduler(p1);
    add_process_to_scheduler(p2);
    
    // Simulamos la ejecución
    run_scheduler();
    
    // Liberar recursos
    free_process(p1);
    free_process(p2);

    return 0;
}

