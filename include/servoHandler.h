#ifndef SERVOHANDLER_H
#define SERVOHANDLER_H

#include "Arduino.h"
#include <STSServoDriver.h>
#include "freertos/semphr.h"
#include "servo_config.h" // Include the separated configuration definitions

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
    // --- Constructors ---
    // The primary constructor for normal use.
    ServoHandler();
    // A constructor specifically for injecting a mock driver during testing.
    ServoHandler(STSServoDriver& driver_for_testing);

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
    STSServoDriver internal_driver; // The real driver instance for normal use
    STSServoDriver& sts;            // A reference that will alias either the internal_driver or a mock

    SemaphoreHandle_t busMutex;
    ServoModelConfig config; // Holds the configuration for the servo model

    // Private helper functions for unit conversions
    int angleToPosition(float angle_deg);
    float positionToAngle(int position);
    int rpmToSpeed(int rpm);
    int speedToRpm(int speed_val);
};

#endif // SERVOHANDLER_H
