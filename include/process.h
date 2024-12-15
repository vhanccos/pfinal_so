// include/process.h
#ifndef PROCESS_H
#define PROCESS_H

#include <stdlib.h>

// Definir el enum para los estados del proceso
typedef enum {
  RUNNING = 0,   // Proceso en ejecución
  WAITING = 1,   // Proceso esperando
  TERMINATED = 2 // Proceso terminado
} ProcessState;

typedef struct {
  const char *name;   // Nombre del proceso (const char* si no se modificará)
  ProcessState state; // Usar el tipo ProcessState para el estado
} Process;

Process *create_process(const char *name);
void free_process(Process *p);

#endif // PROCESS_H
