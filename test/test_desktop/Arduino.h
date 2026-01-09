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

#endif
