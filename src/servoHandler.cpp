#include "servoHandler.h"

// --- Constructors ---

// Primary constructor: Binds the 'sts' reference to the internal driver instance.
ServoHandler::ServoHandler() : internal_driver(), sts(internal_driver) {
    busMutex = NULL;
}

// Testing constructor: Binds the 'sts' reference to the provided mock driver.
ServoHandler::ServoHandler(STSServoDriver& driver_for_testing) : internal_driver(), sts(driver_for_testing) {
    busMutex = NULL;
}

// --- Initialization ---
bool ServoHandler::begin(HardwareSerial& serial, int dirPin, const ServoModelConfig& config) {
    // Store the provided configuration
    this->config = config;

    // Initialize the actual driver that the sts reference is bound to
    if (!sts.init(dirPin, &serial)) {
        return false;
    }

    busMutex = xSemaphoreCreateMutex();
    return (busMutex != NULL);
}

// --- High-Level Motion Control ---

bool ServoHandler::setAngle(byte servoId, float angle_deg) {
    if (xSemaphoreTake(busMutex, portMAX_DELAY) == pdTRUE) {
        sts.setMode(servoId, STSMode::POSITION);
        int position = angleToPosition(angle_deg);
        bool result = sts.setTargetPosition(servoId, position);
        xSemaphoreGive(busMutex);
        return result;
    }
    return false;
}

bool ServoHandler::setAngle(byte servoId, float angle_deg, int speed_rpm) {
    if (xSemaphoreTake(busMutex, portMAX_DELAY) == pdTRUE) {
        sts.setMode(servoId, STSMode::POSITION);
        int position = angleToPosition(angle_deg);
        int speed = rpmToSpeed(speed_rpm);
        bool result = sts.setTargetPosition(servoId, position, speed);
        xSemaphoreGive(busMutex);
        return result;
    }
    return false;
}

bool ServoHandler::setVelocity(byte servoId, int speed_rpm) {
    if (xSemaphoreTake(busMutex, portMAX_DELAY) == pdTRUE) {
        sts.setMode(servoId, STSMode::VELOCITY);
        int speed = rpmToSpeed(speed_rpm);
        bool result = sts.setTargetVelocity(servoId, speed);
        xSemaphoreGive(busMutex);
        return result;
    }
    return false;
}

bool ServoHandler::stop(byte servoId) {
    if (xSemaphoreTake(busMutex, portMAX_DELAY) == pdTRUE) {
        sts.setMode(servoId, STSMode::VELOCITY);
        bool result = sts.setTargetVelocity(servoId, 0);
        xSemaphoreGive(busMutex);
        return result;
    }
    return false;
}

// --- Torque Control ---

bool ServoHandler::setTorque(byte servoId, bool enable) {
    if (xSemaphoreTake(busMutex, portMAX_DELAY) == pdTRUE) {
        bool result = sts.writeRegister(servoId, config.reg_torque_switch, enable ? 1 : 0);
        xSemaphoreGive(busMutex);
        return result;
    }
    return false;
}

bool ServoHandler::setTorqueLimit(byte servoId, float percentage) {
    if (xSemaphoreTake(busMutex, portMAX_DELAY) == pdTRUE) {
        percentage = constrain(percentage, 0.0f, 100.0f);
        int16_t torqueValue = static_cast<int16_t>((percentage / 100.0f) * config.max_torque_val);
        bool result = sts.writeTwoBytesRegister(servoId, config.reg_torque_limit, torqueValue);
        xSemaphoreGive(busMutex);
        return result;
    }
    return false;
}

// --- Feedback ---

float ServoHandler::getAngle(byte servoId) {
    float angle = -1.0f;
    if (xSemaphoreTake(busMutex, portMAX_DELAY) == pdTRUE) {
        int current_pos = sts.getCurrentPosition(servoId);
        if (current_pos != -1) {
            angle = positionToAngle(current_pos);
        }
        xSemaphoreGive(busMutex);
    }
    return angle;
}

bool ServoHandler::getFeedback(byte servoId, ServoFeedback& feedback) {
    bool result = false;
    if (xSemaphoreTake(busMutex, portMAX_DELAY) == pdTRUE) {
        int position = sts.getCurrentPosition(servoId);
        if (position != -1) {
            feedback.position_deg = positionToAngle(position);
            feedback.speed_rpm = speedToRpm(sts.getCurrentSpeed(servoId));
            feedback.temperature_c = sts.getCurrentTemperature(servoId);
            feedback.voltage = sts.readTwoBytesRegister(servoId, config.reg_voltage) / config.voltage_divisor;
            feedback.current_ma = sts.getCurrentCurrent(servoId) * config.current_multiplier_ma;
            feedback.is_moving = sts.isMoving(servoId);
            result = true;
        }
        xSemaphoreGive(busMutex);
    }
    return result;
}

bool ServoHandler::ping(byte servoId) {
    if (xSemaphoreTake(busMutex, portMAX_DELAY) == pdTRUE) {
        bool result = sts.ping(servoId);
        xSemaphoreGive(busMutex);
        return result;
    }
    return false;
}

// --- Private Helper Functions for Unit Conversions ---

int ServoHandler::angleToPosition(float angle_deg) {
    // Manually map using float math for precision, then convert to int.
    float pos = (angle_deg / config.max_angle_deg) * config.max_position;
    return constrain(static_cast<int>(round(pos)), 0, config.max_position);
}

float ServoHandler::positionToAngle(int position) {
    // Manually map using float math for precision.
    return (static_cast<float>(position) / config.max_position) * config.max_angle_deg;
}

int ServoHandler::rpmToSpeed(int rpm) {
    if (rpm == 0) return 0;
    float speed_val = static_cast<float>(rpm) / config.rpm_per_unit;
    // The max speed is limited by the servo's position range (0-4095 for ST3215)
    return constrain(static_cast<int>(round(speed_val)), -config.max_position, config.max_position);
}

int ServoHandler::speedToRpm(int speed_val) {
    return static_cast<int>(round(static_cast<float>(speed_val) * config.rpm_per_unit));
}
