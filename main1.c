#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define NUM_FRAMES 8     // Número de marcos de memoria física
#define FRAME_SIZE 4096  // Tamaño de cada marco de memoria
#define PAGE_SIZE 4096   // Tamaño de cada página
#define MAX_PROCESSES 10 // Número máximo de procesos soportados
#define QUANTUM 20       // Quantum para Round Robin

// Niveles de prioridad mejorados
typedef enum {
  REAL_TIME_PRIORITY = 0,   // Prioridad de tiempo real (máxima)
  SYSTEM_PRIORITY = 1,      // Prioridad de procesos del sistema
  INTERACTIVE_PRIORITY = 2, // Prioridad de procesos interactivos
  BACKGROUND_PRIORITY = 3   // Prioridad de procesos en segundo plano
} ProcessPriority;

// Estructura mejorada de página
typedef struct {
  int id;             // Identificador de página
  bool active;        // Indica si la página está activa
  int processId;      // Identificador del proceso padre
  int lastAccessTime; // Último timestamp de acceso
  bool inMemory;      // Indica si la página está en memoria física
} Page;

// Estructura mejorada de proceso
typedef struct {
  int id;                   // Identificador del proceso
  int startTime;            // Tiempo de inicio del proceso
  ProcessPriority priority; // Prioridad del proceso
  int memorySize;           // Tamaño total de memoria
  int totalPages;           // Número total de páginas
  Page *pages;              // Arreglo de páginas del proceso

  // Gestión mejorada de ciclos de CPU
  int *cpuBurstTimes; // Tiempos de ráfagas de CPU
  int *ioBurstTimes;  // Tiempos de espera de E/S
  int burstIndex;     // Índice de la ráfaga actual
  int totalBursts;    // Número total de ráfagas

  // Seguimiento del estado del proceso
  enum {
    READY,     // Proceso listo para ejecutarse
    RUNNING,   // Proceso en ejecución
    WAITING,   // Proceso esperando (E/S)
    TERMINATED // Proceso finalizado
  } state;
  pthread_t threadId;           // Identificador del hilo
  pthread_mutex_t processMutex; // Mutex para sincronización

  // Métricas de rendimiento
  int waitTime;       // Tiempo de espera
  int turnaroundTime; // Tiempo total de ejecución
  int responseTime;   // Tiempo de respuesta
} Process;

// Estructura de marcos de memoria
typedef struct {
  Page *occupyingPage; // Página que ocupa el marco
  int lastUsedTime;    // Último tiempo de uso del marco
} MemoryFrame;

// Variables globales para gestión de procesos y memoria
MemoryFrame physicalMemory[NUM_FRAMES]; // Memoria física
Process *processQueue[MAX_PROCESSES];   // Cola de procesos
int processCount = 0;                   // Número de procesos
int fifoIndex = 0;                      // Índice para FIFO

// Mutex para sincronización
pthread_mutex_t memoryMutex =
    PTHREAD_MUTEX_INITIALIZER; // Mutex para acceso a memoria
pthread_mutex_t logMutex =
    PTHREAD_MUTEX_INITIALIZER; // Mutex para registro de logs

// Función para registrar mensajes con sincronización
void logMessage(const char *message) {
  pthread_mutex_lock(&logMutex);
  printf("%s\n", message);
  pthread_mutex_unlock(&logMutex);
}

// Función para inicializar las métricas de cada proceso
void initializeProcessMetrics(Process *p) {
  p->waitTime = 0;
  p->turnaroundTime = 0;
  p->responseTime = -1; // -1 indica que aún no ha comenzado
}

// Función para registrar y mostrar métricas de rendimiento
void logPerformanceMetrics() {
  int totalWaitTime = 0;
  int totalTurnaroundTime = 0;
  int totalResponseTime = 0;
  int completedProcesses = 0;

  for (int i = 0; i < processCount; i++) {
    Process *p = processQueue[i];
    if (p->state == TERMINATED) {
      totalWaitTime += p->waitTime;
      totalTurnaroundTime += p->turnaroundTime;
      totalResponseTime += p->responseTime;
      completedProcesses++;
    }
  }

  if (completedProcesses > 0) {
    float avgWaitTime = (float)totalWaitTime / completedProcesses;
    float avgTurnaroundTime = (float)totalTurnaroundTime / completedProcesses;
    float avgResponseTime = (float)totalResponseTime / completedProcesses;
    float throughput = (float)completedProcesses / (float)MAX_PROCESSES;

    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "Métricas de rendimiento:\nPromedio de Tiempo de Espera: %.2f\n"
             "Promedio de Tiempo de Retorno: %.2f\nPromedio de Tiempo de "
             "Respuesta: %.2f\n",
             /* "Throughput: %.2f\n", */
             avgWaitTime, avgTurnaroundTime, avgResponseTime, throughput);
    logMessage(buffer);
  }
}

void displayProcessInfo() {
  char buffer[256];
  logMessage("Información de los procesos:");
  for (int i = 0; i < processCount; i++) {
    Process *p = processQueue[i];
    snprintf(buffer, sizeof(buffer),
             "Proceso ID: %d, Prioridad: %d, Tamaño de Memoria: %d", p->id,
             p->priority, p->memorySize);
    logMessage(buffer);
  }
}

void displayMemoryFrames(int currentTime) {
  char buffer[256];
  logMessage("\nEstado de los marcos de memoria:");
  for (int i = 0; i < NUM_FRAMES; i++) {
    if (physicalMemory[i].occupyingPage != NULL) {
      snprintf(buffer, sizeof(buffer),
               "Marco %d: Página %d del Proceso %d (Último uso: %d)", i,
               physicalMemory[i].occupyingPage->id,
               physicalMemory[i].occupyingPage->processId,
               physicalMemory[i].lastUsedTime);
    } else {
      snprintf(buffer, sizeof(buffer), "Marco %d: [VACÍO ]", i);
    }
    logMessage(buffer);
  }
  /* snprintf(buffer, sizeof(buffer), "Tiempo actual: %d\n", currentTime); */
  logMessage(buffer);
}

// Inicialización del sistema de memoria
void initializeMemorySystem() {
  char buffer[256];

  // Limpiar todos los marcos de memoria
  for (int i = 0; i < NUM_FRAMES; i++) {
    physicalMemory[i].occupyingPage = NULL;
    physicalMemory[i].lastUsedTime = -1;
  }

  // Registrar inicialización
  snprintf(buffer, sizeof(buffer), "Memoria inicializada con %d marcos",
           NUM_FRAMES);
  logMessage(buffer);
}

// Selección de página para reemplazo usando FIFO
int selectPageForReplacementFIFO() { return fifoIndex; }

// Manejador de fallos de página usando FIFO
bool handlePageFaultFIFO(Page *page, int currentTime) {
  char buffer[256];
  pthread_mutex_lock(&memoryMutex);

  // Buscar un marco de memoria libre
  for (int i = 0; i < NUM_FRAMES; i++) {
    if (physicalMemory[i].occupyingPage == NULL) {
      physicalMemory[i].occupyingPage = page;
      physicalMemory[i].lastUsedTime = currentTime;
      page->inMemory = true;

      // Registrar carga de página
      snprintf(
          buffer, sizeof(buffer),
          "Page Fault: Proceso %d, Página %d cargada en marco %d (Tiempo: %d)",
          page->processId, page->id, i, currentTime);
      logMessage(buffer);
      pthread_mutex_unlock(&memoryMutex);
      return true;
    }
  }

  // Si no hay marcos libres, usar reemplazo FIFO
  int replacementFrame = selectPageForReplacementFIFO();

  // Desalojar página existente si es necesario
  if (physicalMemory[replacementFrame].occupyingPage) {
    physicalMemory[replacementFrame].occupyingPage->inMemory = false;

    // Registrar reemplazo de página
    snprintf(buffer, sizeof(buffer),
             "Page Fault: Proceso %d, Página %d - Reemplazando página %d en "
             "marco %d (Tiempo: %d)",
             page->processId, page->id,
             physicalMemory[replacementFrame].occupyingPage->id,
             replacementFrame, currentTime);
    logMessage(buffer);
  }

  // Cargar nueva página
  physicalMemory[replacementFrame].occupyingPage = page;
  physicalMemory[replacementFrame].lastUsedTime = currentTime;
  page->inMemory = true;

  // Actualizar el índice FIFO
  fifoIndex = (fifoIndex + 1) % NUM_FRAMES;

  pthread_mutex_unlock(&memoryMutex);
  return true;
}

// Planificador por prioridad
void priorityScheduler() {
  int currentTime = 0;
  int activeProcesses = processCount;
  logMessage("Iniciando Priority Scheduler");

  while (activeProcesses > 0) {
    // Buscar el proceso con la mayor prioridad
    Process *highestPriorityProcess = NULL;
    for (int i = 0; i < processCount; i++) {
      Process *currentProcess = processQueue[i];
      if (currentProcess->state == READY &&
          (highestPriorityProcess == NULL ||
           currentProcess->priority < highestPriorityProcess->priority)) {
        highestPriorityProcess = currentProcess;
      }
    }

    if (highestPriorityProcess != NULL) {
      // Calcular el tiempo de espera
      highestPriorityProcess->waitTime =
          currentTime - highestPriorityProcess->startTime;

      // Ejecutar el proceso de mayor prioridad
      char buffer[256];
      snprintf(buffer, sizeof(buffer),
               "Tiempo %d: Ejecutando Proceso %d (Prioridad %d)", currentTime,
               highestPriorityProcess->id, highestPriorityProcess->priority);
      logMessage(buffer);

      // Simular acceso de páginas durante la ejecución
      for (int p = 0; p < highestPriorityProcess->totalPages; p++) {
        if (!highestPriorityProcess->pages[p].inMemory) {
          // Mostrar estado de los marcos al final de cada ciclo
          displayMemoryFrames(currentTime);
          handlePageFaultFIFO(&highestPriorityProcess->pages[p], currentTime);
        }
      }

      // Simular ejecución del proceso
      highestPriorityProcess->state = RUNNING;

      // Actualizar el tiempo de respuesta si es la primera vez que se ejecuta
      if (highestPriorityProcess->responseTime == -1) {
        highestPriorityProcess->responseTime =
            currentTime; // Actualiza el tiempo de respuesta
      }

      currentTime += QUANTUM; // Simular tiempo de ejecución
      highestPriorityProcess->state = TERMINATED;
      activeProcesses--;

      // Calcular el tiempo de retorno
      highestPriorityProcess->turnaroundTime =
          currentTime - highestPriorityProcess->startTime;

      snprintf(buffer, sizeof(buffer), "Proceso %d completado en tiempo %d",
               highestPriorityProcess->id, currentTime);
      logMessage(buffer);
    }
  }
  logMessage("Priority Scheduler finalizado");

  // Calcular y mostrar métricas de rendimiento
  /* logPerformanceMetrics(); */
}

int main() {
  // Inicializar sistema de memoria
  initializeMemorySystem();

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
  initializeProcessMetrics(p1);
  p1->id = 1;
  p1->startTime = 0; // Inicia en tiempo 0
  p1->priority = SYSTEM_PRIORITY;
  p1->memorySize = 4 * PAGE_SIZE; // Tamaño de memoria del proceso 1
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
  initializeProcessMetrics(p2);
  p2->id = 2;
  p2->startTime = 5; // Inicia 5 unidades después
  p2->priority = INTERACTIVE_PRIORITY;
  p2->memorySize = 6 * PAGE_SIZE; // Tamaño de memoria del proceso 2
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
  initializeProcessMetrics(p3);
  p3->id = 3;
  p3->startTime = 10; // Inicia 10 unidades después
  p3->priority = SYSTEM_PRIORITY;
  p3->memorySize = 5 * PAGE_SIZE; // Tamaño de memoria del proceso 3
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
  initializeProcessMetrics(p4);
  p4->id = 4;
  p4->startTime = 15; // Inicia 15 unidades después
  p4->priority = INTERACTIVE_PRIORITY;
  p4->memorySize = 7 * PAGE_SIZE; // Tamaño de memoria del proceso 4
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
  processQueue[processCount++] = p1;
  processQueue[processCount++] = p2;
  /* processQueue[processCount++] = p3; */
  /* processQueue[processCount++] = p4; */

  // Mostrar información de los procesos
  displayProcessInfo();

  // Ejecutar el planificador por prioridad
  priorityScheduler();

  // Liberación de recursos
  for (int i = 0; i < processCount; i++) {
    free(processQueue[i]->pages);
    free(processQueue[i]);
  }
  return 0;
}
