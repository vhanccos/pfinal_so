#ifndef PROCESS_H
#define PROCESS_H

#include <pthread.h>
#include <stdbool.h>

// Niveles de prioridad mejorados
typedef enum {
  REAL_TIME_PRIORITY = 0,   // Prioridad de tiempo real (máxima)
  SYSTEM_PRIORITY = 1,      // Prioridad de procesos del sistema
  INTERACTIVE_PRIORITY = 2, // Prioridad de procesos interactivos
  BACKGROUND_PRIORITY = 3   // Procesos en segundo plano
} ProcessPriority;

// Estructura de página
typedef struct {
  int id;             // Identificador de página
  bool active;        // Indica si la página está activa
  int processId;      // Identificador del proceso padre
  int lastAccessTime; // Último timestamp de acceso
  bool inMemory;      // Indica si la página está en memoria física
} Page;

// Estructura de proceso
typedef struct {
  int id;                   // Identificador del proceso
  int startTime;            // Tiempo de inicio del proceso
  ProcessPriority priority; // Prioridad del proceso
  int memorySize;           // Tamaño total de memoria
  int totalPages;           // Número total de páginas
  Page *pages;              // Arreglo de páginas del proceso

  // Gestión de ciclos de CPU
  int *cpuBurstTimes; // Tiempos de ráfagas de CPU
  int *ioBurstTimes;  // Tiempos de espera de E/S
  int burstIndex;     // Índice de la ráfaga actual
  int totalBursts;    // Número total de ráfagas

  // Estados del proceso
  enum { READY, RUNNING, WAITING, TERMINATED } state;
  pthread_t threadId;           // Identificador del hilo
  pthread_mutex_t processMutex; // Mutex para sincronización

  // Métricas de rendimiento
  int waitTime;       // Tiempo de espera
  int turnaroundTime; // Tiempo total de ejecución
  int responseTime;   // Tiempo de respuesta
} Process;

#endif // PROCESS_H
