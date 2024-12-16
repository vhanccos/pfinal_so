#include "memory.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

// Variables globales para gestión de memoria
static MemoryFrame physicalMemory[NUM_FRAMES];
static pthread_mutex_t memoryMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;

// Registrar mensajes con sincronización
void logMessage(const char *message) {
  pthread_mutex_lock(&logMutex);
  printf("%s\n", message);
  pthread_mutex_unlock(&logMutex);
}

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

int selectPageForReplacement() {
  int lruFrame = 0;
  int oldestTime = physicalMemory[0].lastUsedTime;

  // Encontrar el marco menos recientemente usado
  for (int i = 1; i < NUM_FRAMES; i++) {
    if (physicalMemory[i].lastUsedTime < oldestTime) {
      lruFrame = i;
      oldestTime = physicalMemory[i].lastUsedTime;
    }
  }
  return lruFrame;
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
      snprintf(buffer, sizeof(buffer), "Marco %d: [VACÍO]", i);
    }
    logMessage(buffer);
  }
  snprintf(buffer, sizeof(buffer), "Tiempo actual: %d\n", currentTime);
  logMessage(buffer);
}

bool handlePageFault(Page *page, int currentTime) {
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

  // Si no hay marcos libres, usar reemplazo LRU
  int replacementFrame = selectPageForReplacement();

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
  pthread_mutex_unlock(&memoryMutex);
  return true;
}

MemoryFrame *getPhysicalMemory() { return physicalMemory; }
