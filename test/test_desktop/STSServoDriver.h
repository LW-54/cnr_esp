// This is a self-contained, powerful, header-only mock for the STS Servo Driver library.
// It defines a single STSServoDriver class that contains all the logic
// for simulating a bus of virtual servos for unit testing.

#ifndef MOCK_STSSERVO_DRIVER_H
#define MOCK_STSSERVO_DRIVER_H

#include <Arduino.h> // We need this for 'byte' and 'HardwareSerial' from our other mock
#include <map>
#include "servo_config.h" // Include the separated, shared configuration

enum STSMode { POSITION = 0, VELOCITY = 1, STEP = 3 };

// A struct to hold the complete state of a single simulated servo.
struct VirtualServo {
    byte id = 0;
    byte mode = STSMode::POSITION;
    bool torque_enabled = true;
    int16_t torque_limit = 1000;
    
    int current_position = 0;
    int current_speed = 0;
    int current_temperature = 0;
    int16_t current_voltage_raw = 0;
    float current_current_amps = 0.0f;
    bool is_moving_status = false;

    int target_position = 0;
    int target_velocity = 0;
};

// This is the one and only mock class. It simulates the real library's behavior.
class STSServoDriver {
public:
    std::map<byte, VirtualServo> virtual_servos;

    // Helper to get or create a virtual servo
    VirtualServo& getServo(byte const& servoId) {
        if (virtual_servos.find(servoId) == virtual_servos.end()) {
            virtual_servos[servoId].id = servoId;
        }
        return virtual_servos.at(servoId);
    }

    // --- Mocked implementations of all library functions ---
    virtual ~STSServoDriver() {}
    
    // We make these virtual so our simple test mock can inherit and override them if needed,
    // but we also provide a default implementation.
    virtual bool init(byte const &dirPin, HardwareSerial *serialPort = nullptr, long const &baudRate = 1000000) { return true; }
    virtual bool ping(byte const &servoId) { return true; }
    
    virtual bool setMode(unsigned char const& servoId, STSMode const& mode) { 
        getServo(servoId).mode = mode; 
        return true; 
    }
    
    virtual bool writeRegister(byte const &servoId, byte const &registerId, byte const &value, bool const &asynchronous = false);
    
    virtual bool writeTwoBytesRegister(byte const &servoId, byte const &registerId, int16_t const &value, bool const &asynchronous = false);

    virtual int16_t readTwoBytesRegister(byte const &servoId, byte const &registerId);

    virtual int getCurrentPosition(byte const &servoId) { return getServo(servoId).current_position; }
    
    virtual bool setTargetPosition(byte const &servoId, int const &position, int const &speed = 4095, bool const &asynchronous = false) {
        VirtualServo& servo = getServo(servoId);
        servo.target_position = position;
        servo.target_velocity = speed;
        return true;
    }
    
    virtual int getCurrentSpeed(byte const &servoId) { return getServo(servoId).current_speed; }
    
    virtual bool setTargetVelocity(byte const &servoId, int const &velocity, bool const &asynchronous = false) {
        getServo(servoId).target_velocity = velocity;
        return true;
    }

    virtual int getCurrentTemperature(byte const &servoId) { return getServo(servoId).current_temperature; }
    virtual float getCurrentCurrent(byte const &servoId) { return getServo(servoId).current_current_amps; }
    virtual bool isMoving(byte const &servoId) { return getServo(servoId).is_moving_status; }
};

// Now we can define the methods that depend on ST3215_CONFIG
inline bool STSServoDriver::writeRegister(byte const &servoId, byte const &registerId, byte const &value, bool const &asynchronous) {
    if (registerId == ST3215_CONFIG.reg_torque_switch) {
        getServo(servoId).torque_enabled = (value == 1);
    }
    return true;
}

inline bool STSServoDriver::writeTwoBytesRegister(byte const &servoId, byte const &registerId, int16_t const &value, bool const &asynchronous) {
    if (registerId == ST3215_CONFIG.reg_torque_limit) {
        getServo(servoId).torque_limit = value;
    }
    return true;
}

inline int16_t STSServoDriver::readTwoBytesRegister(byte const &servoId, byte const &registerId) {
    if (registerId == ST3215_CONFIG.reg_voltage) {
        return getServo(servoId).current_voltage_raw;
    }
    return -1;
}


#endif // MOCK_STSSERVO_DRIVER_H
