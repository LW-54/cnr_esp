#include <unity.h>
#include "servoHandler.h" // The class we are testing
#include <map>

// --- Dummy Headers for Desktop Compilation ---
// In mock Arduino.h

// --- Advanced Mocking Framework ---

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

class MockSTSServoDriver : public STSServoDriver {
public:
    std::map<byte, VirtualServo> virtual_servos;

    VirtualServo& getServo(byte const& servoId) {
        if (virtual_servos.find(servoId) == virtual_servos.end()) {
            virtual_servos[servoId].id = servoId;
        }
        return virtual_servos.at(servoId);
    }

    // --- Overridden methods with CORRECT signatures and NO 'override' keyword ---
    bool init(byte const &dirPin, HardwareSerial *serialPort = nullptr, long const &baudRate = 1000000) { return true; }
    bool ping(byte const &servoId) { return true; }
    
    bool setMode(unsigned char const& servoId, STSMode const& mode) { 
        getServo(servoId).mode = mode; 
        return true; 
    }
    
    bool writeRegister(byte const &servoId, byte const &registerId, byte const &value, bool const &asynchronous = false) {
        if (registerId == ST3215_CONFIG.reg_torque_switch) {
            getServo(servoId).torque_enabled = (value == 1);
        }
        return true;
    }
    
    bool writeTwoBytesRegister(byte const &servoId, byte const &registerId, int16_t const &value, bool const &asynchronous = false) {
        if (registerId == ST3215_CONFIG.reg_torque_limit) {
            getServo(servoId).torque_limit = value;
        }
        return true;
    }

    int16_t readTwoBytesRegister(byte const &servoId, byte const &registerId) {
        if (registerId == ST3215_CONFIG.reg_voltage) {
            return getServo(servoId).current_voltage_raw;
        }
        return -1;
    }

    int getCurrentPosition(byte const &servoId) { return getServo(servoId).current_position; }
    
    bool setTargetPosition(byte const &servoId, int const &position, int const &speed = 4095, bool const &asynchronous = false) {
        VirtualServo& servo = getServo(servoId);
        servo.target_position = position;
        servo.target_velocity = speed;
        return true;
    }
    
    int getCurrentSpeed(byte const &servoId) { return getServo(servoId).current_speed; }
    
    bool setTargetVelocity(byte const &servoId, int const &velocity, bool const &asynchronous = false) {
        getServo(servoId).target_velocity = velocity;
        return true;
    }

    int getCurrentTemperature(byte const &servoId) { return getServo(servoId).current_temperature; }
    float getCurrentCurrent(byte const &servoId) { return getServo(servoId).current_current_amps; }
    bool isMoving(byte const &servoId) { return getServo(servoId).is_moving_status; }
};


// --- The Tests ---
MockSTSServoDriver mock_driver;
ServoHandler handler_for_testing(mock_driver);

void setUp(void) {
    mock_driver.virtual_servos.clear();
    handler_for_testing.begin(*((HardwareSerial*)nullptr), 0); 
}

void tearDown(void) {}

// ... (Rest of the test functions are unchanged) ...
void test_setAngle_converts_and_sends_correctly(void) {
    handler_for_testing.setAngle(12, 90.0f);
    handler_for_testing.setAngle(25, 360.0f, 100);

    TEST_ASSERT_EQUAL(STSMode::POSITION, mock_driver.getServo(12).mode);
    TEST_ASSERT_EQUAL(1023, mock_driver.getServo(12).target_position);
    TEST_ASSERT_EQUAL(4095, mock_driver.getServo(12).target_velocity);

    TEST_ASSERT_EQUAL(STSMode::POSITION, mock_driver.getServo(25).mode);
    TEST_ASSERT_EQUAL(4095, mock_driver.getServo(25).target_position);
    TEST_ASSERT_EQUAL(416, mock_driver.getServo(25).target_velocity);
}

void test_getAngle_converts_from_raw_correctly(void) {
    mock_driver.getServo(5).current_position = 2047;
    float angle = handler_for_testing.getAngle(5);
    TEST_ASSERT_FLOAT_WITHIN(0.1, 179.95, angle);
}

void test_setVelocity_and_stop(void) {
    handler_for_testing.setVelocity(7, 200);
    TEST_ASSERT_EQUAL(STSMode::VELOCITY, mock_driver.getServo(7).mode);
    TEST_ASSERT_EQUAL(833, mock_driver.getServo(7).target_velocity); // 200 / 0.24

    handler_for_testing.stop(7);
    TEST_ASSERT_EQUAL(STSMode::VELOCITY, mock_driver.getServo(7).mode);
    TEST_ASSERT_EQUAL(0, mock_driver.getServo(7).target_velocity);
}

void test_setTorque_writes_correct_value(void) {
    handler_for_testing.setTorque(8, false);
    TEST_ASSERT_EQUAL(false, mock_driver.getServo(8).torque_enabled);

    handler_for_testing.setTorque(8, true);
    TEST_ASSERT_EQUAL(true, mock_driver.getServo(8).torque_enabled);
}

void test_setTorqueLimit_converts_percentage(void) {
    handler_for_testing.setTorqueLimit(9, 50.0f);
    TEST_ASSERT_EQUAL(500, mock_driver.getServo(9).torque_limit);

    handler_for_testing.setTorqueLimit(9, 0.0f);
    TEST_ASSERT_EQUAL(0, mock_driver.getServo(9).torque_limit);

    handler_for_testing.setTorqueLimit(9, 100.0f);
    TEST_ASSERT_EQUAL(1000, mock_driver.getServo(9).torque_limit);
}

void test_getFeedback_populates_all_fields(void) {
    VirtualServo& servo = mock_driver.getServo(15);
    servo.current_position = 1023;
    servo.current_speed = 416;
    servo.current_temperature = 55;
    servo.current_voltage_raw = 123;
    servo.current_current_amps = 0.5f;
    servo.is_moving_status = true;

    ServoFeedback feedback;
    bool success = handler_for_testing.getFeedback(15, feedback);

    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_FLOAT_WITHIN(0.1, 89.96, feedback.position_deg);
    TEST_ASSERT_EQUAL(99, feedback.speed_rpm);
    TEST_ASSERT_EQUAL(55, feedback.temperature_c);
    TEST_ASSERT_FLOAT_WITHIN(0.1, 12.3, feedback.voltage);
    TEST_ASSERT_FLOAT_WITHIN(1.0, 500.0, feedback.current_ma);
    TEST_ASSERT_TRUE(feedback.is_moving);
}

void test_error_conditions(void) {
    mock_driver.getServo(20).current_position = -1;
    float angle = handler_for_testing.getAngle(20);
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, angle);
    
    ServoFeedback feedback;
    bool success = handler_for_testing.getFeedback(20, feedback);
    TEST_ASSERT_FALSE(success);
}


void runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_setAngle_converts_and_sends_correctly);
    RUN_TEST(test_getAngle_converts_from_raw_correctly);
    RUN_TEST(test_setVelocity_and_stop);
    RUN_TEST(test_setTorque_writes_correct_value);
    RUN_TEST(test_setTorqueLimit_converts_percentage);
    RUN_TEST(test_getFeedback_populates_all_fields);
    RUN_TEST(test_error_conditions);
    UNITY_END();
}

#ifdef PIO_UNIT_TESTING_DESKTOP
int main(int argc, char **argv) {
    runUnityTests();
    return 0;
}
#endif
