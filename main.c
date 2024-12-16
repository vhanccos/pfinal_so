#include "memory.h"
#include "process.h"
#include "scheduler.h"
#include <stdlib.h>

extern void logMessage(const char *message);
int *processCount = NULL;
Process **processQueue = NULL;

int main() {
  // Inicializar sistema de memoria
  initializeMemorySystem();

  // Obtener referencias a la cola de procesos y el contador de procesos
  processQueue = getProcessQueue();
  processCount = getProcessCount();

  // Definición de ráfagas de procesos
  int cpuBursts1[] = {50, 30, 40}; // Ráfagas de CPU para el proceso 1
  int ioBursts1[] = {20, 30, 10};  // Ráfagas de E/S para el proceso 1
  int cpuBursts2[] = {60, 40, 35}; // Ráfagas de CPU para el proceso 2
  int ioBursts2[] = {25, 20, 15};  // Ráfagas de E/S para el proceso 2

  // Definición de ráfagas de procesos adicionales
  int cpuBursts3[] = {45, 25, 55}; // Ráfagas de CPU para el proceso 3
  int ioBursts3[] = {15, 35, 25};  // Ráfagas de E/S para el proceso 3
  int cpuBursts4[] = {55, 50, 45}; // Ráfagas de CPU para el proceso 4
  int ioBursts4[] = {20, 40, 30};  // Ráfagas de E/S para el proceso 4

  // Inicialización del Proceso 1
  Process *p1 = malloc(sizeof(Process));
  p1->id = 1;
  p1->startTime = 0; // Inicia en tiempo 0
  p1->priority = SYSTEM_PRIORITY;
  p1->cpuBurstTimes = cpuBursts1;
  p1->ioBurstTimes = ioBursts1;
  p1->totalBursts = 3;
  p1->totalPages = 4;
  p1->burstIndex = 0;
  p1->state = READY;
  p1->pages = malloc(p1->totalPages * sizeof(Page));

  // Inicializar páginas del Proceso 1
  for (int i = 0; i < p1->totalPages; i++) {
    p1->pages[i].id = p1->id * 10 + i;
    p1->pages[i].processId = p1->id;
    p1->pages[i].inMemory = false;
  }

  // Inicialización del Proceso 2
  Process *p2 = malloc(sizeof(Process));
  p2->id = 2;
  p2->startTime = 5; // Inicia 5 unidades después
  p2->priority = INTERACTIVE_PRIORITY;
  p2->cpuBurstTimes = cpuBursts2;
  p2->ioBurstTimes = ioBursts2;
  p2->totalBursts = 3;
  p2->totalPages = 6;
  p2->burstIndex = 0;
  p2->state = READY;
  p2->pages = malloc(p2->totalPages * sizeof(Page));

  // Inicializar páginas del Proceso 2
  for (int i = 0; i < p2->totalPages; i++) {
    p2->pages[i].id = p2->id * 10 + i;
    p2->pages[i].processId = p2->id;
    p2->pages[i].inMemory = false;
  }

  // Inicialización del Proceso 3
  Process *p3 = malloc(sizeof(Process));
  p3->id = 3;
  p3->startTime = 10; // Inicia 10 unidades después
  p3->priority = SYSTEM_PRIORITY;
  p3->cpuBurstTimes = cpuBursts3;
  p3->ioBurstTimes = ioBursts3;
  p3->totalBursts = 3;
  p3->totalPages = 5;
  p3->burstIndex = 0;
  p3->state = READY;
  p3->pages = malloc(p3->totalPages * sizeof(Page));

  // Inicializar páginas del Proceso 3
  for (int i = 0; i < p3->totalPages; i++) {
    p3->pages[i].id = p3->id * 10 + i;
    p3->pages[i].processId = p3->id;
    p3->pages[i].inMemory = false;
  }

  // Inicialización del Proceso 4
  Process *p4 = malloc(sizeof(Process));
  p4->id = 4;
  p4->startTime = 15; // Inicia 15 unidades después
  p4->priority = INTERACTIVE_PRIORITY;
  p4->cpuBurstTimes = cpuBursts4;
  p4->ioBurstTimes = ioBursts4;
  p4->totalBursts = 3;
  p4->totalPages = 7;
  p4->burstIndex = 0;
  p4->state = READY;
  p4->pages = malloc(p4->totalPages * sizeof(Page));

  // Inicializar páginas del Proceso 4
  for (int i = 0; i < p4->totalPages; i++) {
    p4->pages[i].id = p4->id * 10 + i;
    p4->pages[i].processId = p4->id;
    p4->pages[i].inMemory = false;
  }

  // Agregar procesos a la cola
  addProcessToQueue(p1);
  addProcessToQueue(p2);
  /* addProcessToQueue(p3); */
  /* addProcessToQueue(p4); */

  // Ejecutar el planificador Round Robin
  advancedRoundRobinScheduler();

  // Liberación de recursos
  for (int i = 0; i < *processCount; i++) {
    free(processQueue[i]->pages);
    free(processQueue[i]);
  }
  return 0;
}
