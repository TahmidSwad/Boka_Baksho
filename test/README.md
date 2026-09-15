Boka_Baksho2 Test Suite
======================

Unit tests for the Boka_Baksho2 embedded framework components.

Testing Strategy
----------------

The project uses PlatformIO's native test environment for
host-side unit testing. Tests run on the development machine
(not on the ESP32) using the Unity test framework.

Currently, most hardware-dependent code (drivers, services)
is not unit-testable without mocking the Arduino HAL. The
testable components focus on:

- EventBus: subscription, posting, and dispatching
- AppManager: registration, navigation stack, input routing

Running Tests
-------------

Compile and run tests:

    pio test

Run tests for a specific environment:

    pio test -e native

Run tests with verbose output:

    pio test -v

Adding New Tests
----------------

1. Create a new test file in the `test/` directory
   (e.g., `test/test_app_manager.cpp`)

2. Include the Unity framework:

       #include <unity.h>

3. Include the component under test:

       #include "core/app_manager.h"

4. Write test functions using TEST_ASSERT_* macros

5. Create a main() that initializes Unity and runs tests:

       int main() {
           UNITY_BEGIN();
           RUN_TEST(test_something);
           return UNITY_END();
       }

Notes
-----

- All tests use static allocation and do not require
  dynamic memory allocation.
- Tests that interact with hardware (OLED, buttons, etc.)
  require the ESP32 target and are run via serial monitor
  rather than host-side unit tests.