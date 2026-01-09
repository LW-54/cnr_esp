#ifndef SERVO_CONFIG_H
#define SERVO_CONFIG_H

#include <Arduino.h> // For 'byte'

// A configuration struct to hold all model-specific constants for a servo.
// This allows the ServoHandler class to be adapted for different servo models.
struct ServoModelConfig {
    // Physical characteristics
    float rpm_per_unit;
    int max_position;
    int max_torque_val;
    float max_angle_deg;

    // Unit conversion factors
    float voltage_divisor;          // Divisor to convert raw register value to Volts (e.g., 10.0 for raw values in 0.1V)
    float current_multiplier_ma;    // Multiplier to convert library's Amps value to milliamps (e.g., 1000.0)

    // Register Addresses
    byte reg_torque_switch;
    byte reg_torque_limit;
    byte reg_voltage;
};

// Pre-defined configuration for the Feetech ST3215 servo.
const ServoModelConfig ST3215_CONFIG = {
    .rpm_per_unit = 0.24f,
    .max_position = 4095,
    .max_torque_val = 1000,
    .max_angle_deg = 360.0f,
    .voltage_divisor = 10.0f,
    .current_multiplier_ma = 1000.0f,
    .reg_torque_switch = 0x28,
    .reg_torque_limit = 0x30,
    .reg_voltage = 0x3E
};

#endif // SERVO_CONFIG_H
