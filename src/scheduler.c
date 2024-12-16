#include "scheduler.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Process *processQueue[MAX_PROCESSES];
static int processCount = 0;

int addProcessToQueue(Process *process) {
  if (processCount < MAX_PROCESSES) {
    processQueue[processCount++] = process;
    return processCount - 1;
  }
  return -1;
}

void advancedRoundRobinScheduler() {
  int currentTime = 0;
  int activeProcesses = processCount;
  logMessage("Iniciando Round Robin Scheduler con Quantum");

  while (activeProcesses > 0) {
    for (int i = 0; i < processCount; i++) {
      Process *currentProcess = processQueue[i];

      // Saltar procesos ya terminados
      if (currentProcess->state == TERMINATED)
        continue;

      // Verificar si el proceso ya puede comenzar
      if (currentTime < currentProcess->startTime) {
        currentTime = currentProcess->startTime;
      }

      // Calcular tiempo de ejecución en este quantum
      int executionTime =
          (currentProcess->cpuBurstTimes[currentProcess->burstIndex] < QUANTUM)
              ? currentProcess->cpuBurstTimes[currentProcess->burstIndex]
              : QUANTUM;

      // Información de cambio de contexto
      char buffer[256];
      snprintf(buffer, sizeof(buffer),
               "Tiempo %d: Cambiando a Proceso %d (Quantum %d)", currentTime,
               currentProcess->id, executionTime);
      logMessage(buffer);

      // Simular ejecución del proceso
      currentProcess->state = RUNNING;

      // Simular acceso de páginas durante la ejecución
      for (int p = 0; p < currentProcess->totalPages; p++) {
        if (!currentProcess->pages[p].inMemory) {
          // Mostrar estado de los marcos al final de cada ciclo
          displayMemoryFrames(currentTime);
          handlePageFault(&currentProcess->pages[p], currentTime);
        }
      }

      currentProcess->cpuBurstTimes[currentProcess->burstIndex] -=
          executionTime;
      currentTime += executionTime;

      // Verificar si la ráfaga de CPU ha terminado
      if (currentProcess->cpuBurstTimes[currentProcess->burstIndex] <= 0) {
        // Pasar a la siguiente ráfaga o terminar
        currentProcess->burstIndex++;
        if (currentProcess->burstIndex >= currentProcess->totalBursts) {
          currentProcess->state = TERMINATED;
          activeProcesses--;
          snprintf(buffer, sizeof(buffer), "Proceso %d completado en tiempo %d",
                   currentProcess->id, currentTime);
          logMessage(buffer);
        } else {
          // Simular tiempo de E/S
          int ioTime =
              currentProcess->ioBurstTimes[currentProcess->burstIndex - 1];
          currentProcess->state = WAITING;
          snprintf(buffer, sizeof(buffer),
                   "Proceso %d esperando E/S por %d tiempo", currentProcess->id,
                   ioTime);
          logMessage(buffer);
          currentTime += ioTime;
        }
      }
    }
  }
  logMessage("Round Robin Scheduler finalizado");
}

Process **getProcessQueue() { return processQueue; }

int *getProcessCount() { return &processCount; }
