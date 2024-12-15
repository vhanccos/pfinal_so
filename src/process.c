#include "process.h"
#include <stdlib.h> // Añadir esta línea para incluir malloc y free
#include <string.h> // Necesario para strdup

Process *create_process(const char *name) {
  Process *p = (Process *)malloc(sizeof(Process));
  p->name = strdup(name); // Si decides hacer una copia del nombre
  p->state = RUNNING;     // Usar el valor del enum RUNNING
  return p;
}

void free_process(Process *p) {
  free(p->name); // Liberar la memoria de name
  free(p);       // Liberar la memoria del proceso
}
