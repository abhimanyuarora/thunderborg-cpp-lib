#ifndef THUNDERBORG_H
#define THUNDERBORG_H

#include <stdint.h>
#include <cstdio>
#include <stdarg.h>

#define PWM_MAX                      255
#define I2C_MAX_LEN                  6
#define VOLTAGE_PIN_MAX              36.3  // Maximum voltage from the analog voltage monitoring pin
#define VOLTAGE_PIN_CORRECTION       0.0   // Correction value for the analog voltage monitoring pin
#define BATTERY_MIN_DEFAULT          7.0   // Default minimum battery monitoring voltage
#define BATTERY_MAX_DEFAULT          35.0  // Default maximum battery monitoring voltage

#define I2C_ID_THUNDERBORG           0x15

#define COMMAND_SET_LED1             1     // Set the colour of the ThunderBorg LED
#define COMMAND_GET_LED1             2     // Get the colour of the ThunderBorg LED
#define COMMAND_SET_LED2             3     // Set the colour of the ThunderBorg Lid LED
#define COMMAND_GET_LED2             4     // Get the colour of the ThunderBorg Lid LED
#define COMMAND_SET_LEDS             5     // Set the colour of both the LEDs
#define COMMAND_SET_LED_BATT_MON     6     // Set the colour of both LEDs to show the current battery level
#define COMMAND_GET_LED_BATT_MON     7     // Get the state of showing the current battery level via the LEDs
#define COMMAND_SET_A_FWD            8     // Set motor A PWM rate in a forwards direction
#define COMMAND_SET_A_REV            9     // Set motor A PWM rate in a reverse direction
#define COMMAND_GET_A                10    // Get motor A direction and PWM rate
#define COMMAND_SET_B_FWD            11    // Set motor B PWM rate in a forwards direction
#define COMMAND_SET_B_REV            12    // Set motor B PWM rate in a reverse direction
#define COMMAND_GET_B                13    // Get motor B direction and PWM rate
#define COMMAND_ALL_OFF              14    // Switch everything off
#define COMMAND_GET_DRIVE_A_FAULT    15    // Get the drive fault flag for motor A, indicates faults such as short-circuits and under voltage
#define COMMAND_GET_DRIVE_B_FAULT    16    // Get the drive fault flag for motor B, indicates faults such as short-circuits and under voltage
#define COMMAND_SET_ALL_FWD          17    // Set all motors PWM rate in a forwards direction
#define COMMAND_SET_ALL_REV          18    // Set all motors PWM rate in a reverse direction
#define COMMAND_SET_FAILSAFE         19    // Set the failsafe flag, turns the motors off if communication is interrupted
#define COMMAND_GET_FAILSAFE         20    // Get the failsafe flag
#define COMMAND_GET_BATT_VOLT        21    // Get the battery voltage reading
#define COMMAND_SET_BATT_LIMITS      22    // Set the battery monitoring limits
#define COMMAND_GET_BATT_LIMITS      23    // Get the battery monitoring limits
#define COMMAND_WRITE_EXTERNAL_LED   24    // Write a 32bit pattern out to SK9822 / APA102C
#define COMMAND_GET_ID               0x99  // Get the board identifier
#define COMMAND_SET_I2C_ADD          0xAA  // Set a new I2C address

#define COMMAND_VALUE_FWD            1     // I2C value representing forward
#define COMMAND_VALUE_REV            2     // I2C value representing reverse

#define COMMAND_VALUE_ON             1     // I2C value representing on
#define COMMAND_VALUE_OFF            0     // I2C value representing off

#define COMMAND_ANALOG_MAX           0x3FF // Maximum value for analog readings

class Thunderborg {
    private:
        bool foundChip             = false;
        bool debug                 = false;
        int  i2cFile;

        void debug_printf(const char *fmt, ...) {
            va_list args;
            va_start(args, fmt);
            va_end(args);
            if (debug) vfprintf(stdout,fmt, args);
        }

    public:
        int  busNumber             = 1;                     // Check here for Rev 1 vs Rev 2 and select the correct bus
        int  i2cAddress            = I2C_ID_THUNDERBORG;    // I2C address, override for a different address

        int RawWrite(char command, char* data, int len = 0);
        int RawRead(char command, char* buf, int length, int retryCount = 3);
        void InitBusOnly(int busNumber, int address);
        void Init(bool tryOtherBus = false);
        bool SetMotor1(float power);
        float GetMotor1();
        bool SetMotor2(float power);
        float GetMotor2();
        bool SetMotors(float power);
        bool MotorsOff();
        bool SetLed1(float* rgb);
        bool GetLed1(float* rgb);
        bool SetLed2(float* rgb);
        bool GetLed2(float* rgb);
        bool SetLeds(float* rgb);
        bool SetLedShowBattery(int state);
        int GetLedShowBattery();
        bool SetCommsFailsafe(int state);
        int GetCommsFailsafe();
        int GetDriveFault1();
        int GetDriveFault2();
        float GetBatteryReading();
        bool SetBatteryMonitoringLimits(float minimum, float maximum);
        bool GetBatteryMonitoringLimits(float* minimum, float* maximum);
        bool WriteExternalLedWord(char b0, char b1, char b2, char b3);
        bool SetExternalLedColours(float colors[][3], int numColors);

        void SetDebug(bool state) {debug = state;}

        Thunderborg() {}
        Thunderborg(int busNumber_, int i2cAddress) {
            busNumber = busNumber_;
            i2cAddress = i2cAddress;
        }
};

#endif