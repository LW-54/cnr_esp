#ifndef Arduino_h
#define Arduino_h

#include <stdint.h>
#include <math.h>

// Define the basic types our library needs
typedef uint8_t byte;

// A dummy class for HardwareSerial so that the compiler is happy
class HardwareSerial {};

// A dummy implementation of the constrain macro/function
template<class T>
T constrain(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Dummy FreeRTOS types and functions for desktop compilation
typedef void* SemaphoreHandle_t;
static SemaphoreHandle_t xSemaphoreCreateMutex() { return (void*)1; } // Return a non-null dummy value
static bool xSemaphoreTake(SemaphoreHandle_t, int) { return true; }
static void xSemaphoreGive(SemaphoreHandle_t) {}

// Dummy FreeRTOS macros
#define pdTRUE 1
#define portMAX_DELAY 0xFFFFFFFF

#endif
