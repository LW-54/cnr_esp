#include <unity.h>
#include "servoHandler.h"        // The class we are testing
#include "STSServoDriver.h"      // Our new, powerful, all-in-one mock

// --- The Tests ---

// We create an instance of the mock STSServoDriver directly.
// In the test environment, this IS the mock.
STSServoDriver mock_driver; 

// We inject the mock into the handler we're going to test.
ServoHandler handler_for_testing(mock_driver);

void setUp(void) {
    // Clear the map before each test to ensure a clean state
    mock_driver.virtual_servos.clear();
    // We pass a dummy serial object. The mock's init doesn't use it.
    handler_for_testing.begin(*((HardwareSerial*)nullptr), 0); 
}

void tearDown(void) {}

// --- Test Cases ---

void test_setAngle_converts_and_sends_correctly(void) {
    handler_for_testing.setAngle(12, 90.0f); // Test default speed
    handler_for_testing.setAngle(25, 360.0f, 100); // Test specified speed

    TEST_ASSERT_EQUAL(STSMode::POSITION, mock_driver.getServo(12).mode);
    TEST_ASSERT_EQUAL(1024, mock_driver.getServo(12).target_position);
    TEST_ASSERT_EQUAL(4095, mock_driver.getServo(12).target_velocity);

    TEST_ASSERT_EQUAL(STSMode::POSITION, mock_driver.getServo(25).mode);
    TEST_ASSERT_EQUAL(4095, mock_driver.getServo(25).target_position);
    TEST_ASSERT_EQUAL(417, mock_driver.getServo(25).target_velocity);
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
    // 1. Setup the mock with known raw values
    VirtualServo& servo = mock_driver.getServo(15);
    servo.current_position = 1023; // 90 deg
    servo.current_speed = 416;     // 100 rpm
    servo.current_temperature = 55;
    servo.current_voltage_raw = 123; // 12.3V
    servo.current_current_amps = 0.5f; // 500mA
    servo.is_moving_status = true;

    // 2. Run the code
    ServoFeedback feedback;
    bool success = handler_for_testing.getFeedback(15, feedback);

    // 3. Assert
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_FLOAT_WITHIN(0.1, 89.96, feedback.position_deg);
    TEST_ASSERT_EQUAL(100, feedback.speed_rpm); // 416 * 0.24 = 99.84, rounds to 100
    TEST_ASSERT_EQUAL(55, feedback.temperature_c);
    TEST_ASSERT_FLOAT_WITHIN(0.1, 12.3, feedback.voltage);
    TEST_ASSERT_FLOAT_WITHIN(1.0, 500.0, feedback.current_ma);
    TEST_ASSERT_TRUE(feedback.is_moving);
}

void test_error_conditions(void) {
    mock_driver.getServo(20).current_position = -1; // Simulate a read error
    float angle = handler_for_testing.getAngle(20);
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, angle);
    
    ServoFeedback feedback;
    bool success = handler_for_testing.getFeedback(20, feedback);
    TEST_ASSERT_FALSE(success);
}

// --- Test Runner ---
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

#ifdef PIO_UNIT_TESTING
#include <stdio.h>
// On desktop, we need a main function to run the tests
int main(int argc, char **argv) {
    runUnityTests();
    return 0;
}
#endif