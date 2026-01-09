#ifndef SERVOHANDLER_H
#define SERVOHANDLER_H

#include "Arduino.h"
#include <STSServoDriver.h>
#include "freertos/semphr.h"

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

// A struct to hold comprehensive feedback from a servo
struct ServoFeedback {
    float position_deg;
    int speed_rpm;
    int temperature_c;
    float voltage;
    float current_ma;
    bool is_moving;
};

class ServoHandler {
public:
    ServoHandler();

    // Initialize the serial communication and mutex with a specific servo configuration.
    // Defaults to the ST3215 configuration if none is provided.
    bool begin(HardwareSerial& serial, int dirPin, const ServoModelConfig& config = ST3215_CONFIG);

    // --- High-Level Motion Control ---
    bool setAngle(byte servoId, float angle_deg);
    bool setAngle(byte servoId, float angle_deg, int speed_rpm);
    bool setVelocity(byte servoId, int speed_rpm);
    bool stop(byte servoId);

    // --- Torque Control ---
    bool setTorque(byte servoId, bool enable);
    bool setTorqueLimit(byte servoId, float percentage);

    // --- Feedback ---
    float getAngle(byte servoId);
    bool getFeedback(byte servoId, ServoFeedback& feedback);
    bool ping(byte servoId);

private:
    STSServoDriver sts;
    SemaphoreHandle_t busMutex;
    ServoModelConfig config; // Holds the configuration for the servo model

    // Private helper functions for unit conversions
    int angleToPosition(float angle_deg);
    float positionToAngle(int position);
    int rpmToSpeed(int rpm);
    int speedToRpm(int speed_val);
};

#endif // SERVOHANDLER_H
