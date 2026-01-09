// This is a dummy header file for FreeRTOS semaphores.
// It provides the minimum necessary definitions to allow code that uses
// FreeRTOS mutexes to compile in a desktop test environment.

#ifndef SEMPHR_H
#define SEMPHR_H

// Dummy FreeRTOS types and functions for desktop compilation
typedef void* SemaphoreHandle_t;
static SemaphoreHandle_t xSemaphoreCreateMutex() { return (void*)1; } // Return a non-null dummy value
static bool xSemaphoreTake(SemaphoreHandle_t, int) { return true; }
static void xSemaphoreGive(SemaphoreHandle_t) {}

// Dummy FreeRTOS macros
#define pdTRUE 1
#define portMAX_DELAY 0xFFFFFFFF

#endif // SEMPHR_H