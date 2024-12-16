#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

#define QUANTUM 20 // Quantum para Round Robin

// Función para agregar un proceso a la cola de procesos
int addProcessToQueue(Process *process);

// Función para ejecutar el planificador Round Robin avanzado
void advancedRoundRobinScheduler();

// Obtener acceso a la cola de procesos
Process **getProcessQueue();

// Obtener el número de procesos en la cola
int *getProcessCount();

#endif // SCHEDULER_H
