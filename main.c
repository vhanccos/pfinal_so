#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_FRAMES 8    // Número de marcos de memoria
#define FRAME_SIZE 4096 // Tamaño de cada marco (4K)
#define PAGE_SIZE 4096  // Tamaño de cada página (4K)

#define SYSTEM_PRIORITY 0      // Prioridad de los procesos del sistema
#define INTERACTIVE_PRIORITY 1 // Prioridad de los procesos interactivos
#define BATCH_PRIORITY 2       // Prioridad de los procesos batch
#define TIME_QUANTUM 2         // Cuántum de tiempo para Round Robin

// Estructura de una página
typedef struct {
  int id;         // ID de la página
  bool active;    // Indica si la página está activa
  int lastAccess; // Último ciclo de acceso
} Page;

// Estructura de un proceso
typedef struct {
  int id;            // ID del proceso
  int priority;      // Prioridad del proceso
  int memorySize;    // Tamaño del proceso en bytes
  int numPages;      // Número de páginas
  Page *pages;       // Páginas del proceso
  int *cpuCycles;    // Ciclos de CPU/I/O
  int cpuCycleIndex; // Índice del ciclo actual
  int currentCycle;  // Contador global de ciclos de ejecución
  pthread_t thread;  // Hilo asociado al proceso
  bool inMemory; // Indica si el proceso está completamente cargado en memoria
} Process;

// Memoria física (marcos)
Page memory[NUM_FRAMES];

// Mutexes para sincronización de recursos compartidos
pthread_mutex_t resourceMutex = PTHREAD_MUTEX_INITIALIZER;

// Función para inicializar la memoria
void initMemory() {
  for (int i = 0; i < NUM_FRAMES; i++) {
    memory[i].id = -1; // No asignada
    memory[i].active = false;
    memory[i].lastAccess = -1;
  }
}

// Función para inicializar un proceso
void initProcess(Process *process, int id, int priority, int memorySize,
                 int *cpuCycles, int numPages) {
  process->id = id;
  process->priority = priority;
  process->memorySize = memorySize;
  process->cpuCycles = cpuCycles;
  process->cpuCycleIndex = 0;
  process->currentCycle = 0; // Inicializa el contador de ciclos
  process->numPages = numPages;
  process->pages = (Page *)malloc(numPages * sizeof(Page));
  process->inMemory = false;

  for (int i = 0; i < numPages; i++) {
    process->pages[i].id = i;
    process->pages[i].active = false;
    process->pages[i].lastAccess = -1;
  }
}

// Función que simula el ciclo de ejecución de un proceso
void *executeProcess(void *arg) {
  Process *process = (Process *)arg;

  while (process->cpuCycleIndex <
         sizeof(process->cpuCycles) / sizeof(process->cpuCycles[0])) {
    // Simulación de la ejecución de un ciclo
    int currentCycle = process->cpuCycles[process->cpuCycleIndex];
    printf("Proceso %d: Ciclo %d de CPU/I/O\n", process->id, currentCycle);

    // Actualizar páginas activas según los ciclos de CPU/I/O
    for (int i = 0; i < process->numPages; i++) {
      if (process->currentCycle >= process->cpuCycles[i]) {
        process->pages[i].active = true;
        process->pages[i].lastAccess = process->currentCycle;
      }
    }

    // Comprobar si las páginas están en memoria (page fault)
    for (int i = 0; i < process->numPages; i++) {
      if (process->pages[i].active && memory[process->pages[i].id].id == -1) {
        // Simular un page fault y cargar la página
        printf("Page fault en el Proceso %d, cargando la página %d\n",
               process->id, process->pages[i].id);
        for (int j = 0; j < NUM_FRAMES; j++) {
          if (memory[j].id == -1) {
            memory[j] = process->pages[i];
            break;
          }
        }
      }
    }

    // Incrementar el contador de ciclos del proceso
    process->cpuCycleIndex++;
    process->currentCycle++;

    // Simula el retraso de ejecución del ciclo
    sleep(1); // Pausa para simular un ciclo de CPU

    // Fin del ciclo
  }
  process->inMemory = true;
  return NULL;
}

// Función para implementar Round Robin
void roundRobinScheduler(Process *processes, int numProcesses) {
  int cycleCount = 0;

  // Realizar Round Robin por ciclos
  while (true) {
    bool allFinished = true;

    for (int i = 0; i < numProcesses; i++) {
      if (!processes[i].inMemory &&
          processes[i].cpuCycleIndex < sizeof(processes[i].cpuCycles) /
                                           sizeof(processes[i].cpuCycles[0])) {
        allFinished = false;
        printf("Ejecutando el Proceso %d\n", processes[i].id);
        executeProcess((void *)&processes[i]);
        // Verificar si el proceso ha terminado su ejecución
        if (processes[i].cpuCycleIndex >=
            sizeof(processes[i].cpuCycles) /
                sizeof(processes[i].cpuCycles[0])) {
          processes[i].inMemory = true;
        }
      }
    }

    if (allFinished) {
      break; // Terminar cuando todos los procesos hayan finalizado
    }

    // Avanzar un ciclo
    cycleCount++;
    sleep(1); // Pausar por un ciclo antes de seguir con el siguiente
  }
}

// Función principal para crear y ejecutar los procesos
int main() {
  // Inicializar la memoria
  initMemory();

  // Crear procesos con diferentes tamaños y prioridades
  Process p1, p2, p3, p4;
  int cycles1[] = {1, 3, 1};
  int cycles2[] = {2, 4, 1};
  int cycles3[] = {5, 3};
  int cycles4[] = {2, 3, 2};

  initProcess(&p1, 1, SYSTEM_PRIORITY, 2 * PAGE_SIZE, cycles1, 1);
  initProcess(&p2, 2, INTERACTIVE_PRIORITY, 10 * PAGE_SIZE, cycles2, 3);
  initProcess(&p3, 3, BATCH_PRIORITY, 8 * PAGE_SIZE, cycles3, 2);
  initProcess(&p4, 4, SYSTEM_PRIORITY, 6 * PAGE_SIZE, cycles4, 2);

  // Crear y ejecutar los procesos con Round Robin
  Process processes[] = {p1, p2, p3, p4};
  roundRobinScheduler(processes, 4);

  return 0;
}
