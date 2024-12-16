#ifndef MEMORY_H
#define MEMORY_H

#include "process.h"

#define NUM_FRAMES 8     // Número de marcos de memoria física
#define FRAME_SIZE 4096  // Tamaño de cada marco de memoria
#define PAGE_SIZE 4096   // Tamaño de cada página
#define MAX_PROCESSES 10 // Número máximo de procesos soportados

// Estructura de marcos de memoria
typedef struct {
  Page *occupyingPage; // Página que ocupa el marco
  int lastUsedTime;    // Último tiempo de uso del marco
} MemoryFrame;

// Función de inicialización del sistema de memoria
void initializeMemorySystem();

// Función para manejar fallos de página
bool handlePageFault(Page *page, int currentTime);

// Función para mostrar marcos de memoria
void displayMemoryFrames(int currentTime);

// Función para seleccionar página para reemplazo
int selectPageForReplacement();

// Obtener acceso a los marcos de memoria físicos
MemoryFrame *getPhysicalMemory();

void logMessage(const char *message);

#endif // MEMORY_H
