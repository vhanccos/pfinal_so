#include <stdio.h>
#include "resource.h"

void init_resources() {
    printf("Inicializando recursos...\n");
}

void allocate_resource(int resource_id) {
    printf("Recurso %d asignado\n", resource_id);
}

