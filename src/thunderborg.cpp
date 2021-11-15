#include <thunderborg.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <math.h>
#include <algorithm>

int Thunderborg::RawWrite(char command, char* data, int len) {
    char cmd[len+1] = {0};
    cmd[0] = command;
    if (data) {
        memcpy(cmd+1,data,len);
    }
    return write(i2cFile, cmd, len+1);
}

int Thunderborg::RawRead(char command, char* buf, int length, int retryCount) {
    char resp[length];
    int n = -1;
    while (retryCount > 0) {
        RawWrite(command,NULL);
        n = read(i2cFile, resp, length);
        memcpy(buf,resp,length);

        if (command == resp[0]) break;
        else retryCount--;
    }
    return n;
}

void Thunderborg::InitBusOnly(int busNumber_, int address) {
    char filename[20];
    snprintf(filename, 19, "/dev/i2c-%d", busNumber_);
    i2cFile = open(filename, O_RDWR);
    if (i2cFile < 0) {
        throw std::runtime_error("Couldn't open I2C port");
    }
    if (ioctl(i2cFile, I2C_SLAVE, address) < 0) {
        throw std::runtime_error("Couldn't set slave address");
    }
}

void Thunderborg::Init(bool tryOtherBus) {
    debug_printf("Loading ThunderBorg on bus %d, address\n", busNumber);

    // Open the bus
    InitBusOnly(busNumber, i2cAddress);

    char resp[I2C_MAX_LEN];
    if (RawRead(COMMAND_GET_ID, resp, I2C_MAX_LEN) == I2C_MAX_LEN) {
        if (resp[1] == I2C_ID_THUNDERBORG) {
            foundChip = true;
            debug_printf("Found ThunderBorg at %02X\n", i2cAddress);
        } else {
            foundChip = true;
            debug_printf("Found a device at %02X, but it is not a ThunderBorg (ID %02X instead of %02X)\n", i2cAddress, resp[1], I2C_ID_THUNDERBORG);
        }
    } else {
        foundChip = false;
        debug_printf("Missing ThunderBorg at %02X\n", i2cAddress);
    }

    if (!foundChip) {
        debug_printf("ThunderBorg was not found\n");
        if (tryOtherBus) {
            if (busNumber == 1) {
                busNumber = 0;
            } else {
                busNumber = 1;
            }
            debug_printf("Trying bus %d instead\n", busNumber);
            Init(false);
        } else {
            debug_printf("Are you sure your ThunderBorg is properly attached, the correct address is used, and the I2C drivers are running?\n");
            throw std::runtime_error("Couldn't connect to Thunderborg");
        }
    } else {
        debug_printf("ThunderBorg loaded on bus %d\n", busNumber);
    }
}

bool Thunderborg::SetMotor1(float power) {
    char command = 0;
    int pwm_int = 0;
    if (power < 0) {
        // Reverse
        command = COMMAND_SET_A_REV;
        pwm_int = -(PWM_MAX * power);
        if (pwm_int > PWM_MAX) pwm_int = PWM_MAX;
    } else {
        // Forward / stopped
        command = COMMAND_SET_A_FWD;
        pwm_int = (PWM_MAX * power);
        if (pwm_int > PWM_MAX) pwm_int = PWM_MAX;
    }
    char pwm = (char) pwm_int;
    if (RawWrite(command, &pwm, 1) != 2) {
        debug_printf("Failed sending motor 1 drive level!\n");
        return false;
    }
    return true;
}

float Thunderborg::GetMotor1() {
    float power = NAN;

    char resp[I2C_MAX_LEN];
    if (RawRead(COMMAND_GET_A, resp, I2C_MAX_LEN) == I2C_MAX_LEN) {
        if (resp[1] == COMMAND_VALUE_FWD) {
            power = (float) resp[2] / (float) PWM_MAX;
        } else if (resp[1] == COMMAND_VALUE_REV) {
            power = -(float) resp[2] / (float) PWM_MAX;
        }
    } else {
        debug_printf("Failed reading motor 1 drive level!\n");
    }

    return power;
}

bool Thunderborg::SetMotor2(float power) {
    char command = 0;
    int pwm_int = 0;
    if (power < 0) {
        // Reverse
        command = COMMAND_SET_B_REV;
        pwm_int = -(PWM_MAX * power);
        if (pwm_int > PWM_MAX) pwm_int = PWM_MAX;
    } else {
        // Forward / stopped
        command = COMMAND_SET_B_FWD;
        pwm_int = (PWM_MAX * power);
        if (pwm_int > PWM_MAX) pwm_int = PWM_MAX;
    }
    char pwm = (char) pwm_int;
    if (RawWrite(command, &pwm, 1) != 2) {
        debug_printf("Failed sending motor 2 drive level!\n");
        return false;
    }
    return true;
}

float Thunderborg::GetMotor2() {
    float power = NAN;

    char resp[I2C_MAX_LEN];
    if (RawRead(COMMAND_GET_B, resp, I2C_MAX_LEN) == I2C_MAX_LEN) {
        if (resp[1] == COMMAND_VALUE_FWD) {
            power = (float) resp[2] / (float) PWM_MAX;
        } else if (resp[1] == COMMAND_VALUE_REV) {
            power = -(float) resp[2] / (float) PWM_MAX;
        }
    } else {
        debug_printf("Failed reading motor 2 drive level!\n");
    }

    return power;
}

bool Thunderborg::SetMotors(float power) {
    char command = 0;
    int pwm_int = 0;
    if (power < 0) {
        // Reverse
        command = COMMAND_SET_ALL_REV;
        pwm_int = -(PWM_MAX * power);
        if (pwm_int > PWM_MAX) pwm_int = PWM_MAX;
    } else {
        // Forward / stopped
        command = COMMAND_SET_ALL_FWD;
        pwm_int = (PWM_MAX * power);
        if (pwm_int > PWM_MAX) pwm_int = PWM_MAX;
    }
    char pwm = (char) pwm_int;
    if (RawWrite(command, &pwm, 1) != 2) {
        debug_printf("Failed sending motor 1 drive level!\n");
        return false;
    }
    return true;
}

bool Thunderborg::MotorsOff() {
    char data[1] = {0};
    if (RawWrite(COMMAND_ALL_OFF, data, 1) != 2) {
        debug_printf("Failed sending motors off command!\n");
        return false;
    }
    return true;
}

bool Thunderborg::SetLed1(float* rgb) {
    char levelRGB[] = {
        std::max<char>(0, std::min<char>(PWM_MAX, (int) (rgb[0]*PWM_MAX))),
        std::max<char>(0, std::min<char>(PWM_MAX, (int) (rgb[1]*PWM_MAX))),
        std::max<char>(0, std::min<char>(PWM_MAX, (int) (rgb[2]*PWM_MAX)))
    };

    if (RawWrite(COMMAND_SET_LED1, levelRGB, 3) != 4) {
        debug_printf("Failed sending colour for the ThunderBorg LED!\n");
        return false;
    }
    return true;
}

bool Thunderborg::GetLed1(float* rgb) {
    char resp[I2C_MAX_LEN];
    if (RawRead(COMMAND_GET_LED1, resp, I2C_MAX_LEN) == I2C_MAX_LEN) {
        rgb[0] = (float) resp[1] / (float) PWM_MAX;
        rgb[1] = (float) resp[2] / (float) PWM_MAX;
        rgb[2] = (float) resp[3] / (float) PWM_MAX;
    } else {
        debug_printf("Failed reading ThunderBorg LED colour!\n");
        return false;
    }
    return true;
}

bool Thunderborg::SetLed2(float* rgb) {
    char levelRGB[] = {
        std::max<char>(0, std::min<char>(PWM_MAX, (int) (rgb[0]*PWM_MAX))),
        std::max<char>(0, std::min<char>(PWM_MAX, (int) (rgb[1]*PWM_MAX))),
        std::max<char>(0, std::min<char>(PWM_MAX, (int) (rgb[2]*PWM_MAX)))
    };

    if (RawWrite(COMMAND_SET_LED2, levelRGB, 3) != 4) {
        debug_printf("Failed sending colour for the ThunderBorg Lid LED!\n");
        return false;
    }
    return true;
}

bool Thunderborg::GetLed2(float* rgb) {
    char resp[I2C_MAX_LEN];
    if (RawRead(COMMAND_GET_LED2, resp, I2C_MAX_LEN) == I2C_MAX_LEN) {
        rgb[0] = (float) resp[1] / (float) PWM_MAX;
        rgb[1] = (float) resp[2] / (float) PWM_MAX;
        rgb[2] = (float) resp[3] / (float) PWM_MAX;
    } else {
        debug_printf("Failed reading ThunderBorg Lid LED colour!\n");
        return false;
    }
    return true;
}

bool Thunderborg::SetLeds(float* rgb) {
    char levelRGB[] = {
        std::max<char>(0, std::min<char>(PWM_MAX, (int) (rgb[0]*PWM_MAX))),
        std::max<char>(0, std::min<char>(PWM_MAX, (int) (rgb[1]*PWM_MAX))),
        std::max<char>(0, std::min<char>(PWM_MAX, (int) (rgb[2]*PWM_MAX)))
    };

    if (RawWrite(COMMAND_SET_LEDS, levelRGB, 3) != 4) {
        debug_printf("Failed sending colour for both LEDs!\n");
        return false;
    }
    return true;
}

bool Thunderborg::SetLedShowBattery(int state) {
    char level[1];
    if (state) {
        level[0] = COMMAND_VALUE_ON;
    } else {
        level[0] = COMMAND_VALUE_OFF;
    }

    if (RawWrite(COMMAND_SET_LED_BATT_MON, level, 1) != 2) {
        debug_printf("Failed sending LED battery monitoring state!\n");
        return false;
    }
    return true;
}

int Thunderborg::GetLedShowBattery() {
    char resp[I2C_MAX_LEN];
    if (RawRead(COMMAND_GET_LED_BATT_MON, resp, I2C_MAX_LEN) == I2C_MAX_LEN) {
        if (resp[1] == COMMAND_VALUE_OFF) return false;
        else if (resp[1] == COMMAND_VALUE_ON) return true;
    } else {
        debug_printf("Failed reading LED battery monitoring state!\n");
    }

    return -1;
}

bool Thunderborg::SetCommsFailsafe(int state) {
    char level[1];
    if (state) {
        level[0] = COMMAND_VALUE_ON;
    } else {
        level[0] = COMMAND_VALUE_OFF;
    }

    if (RawWrite(COMMAND_SET_FAILSAFE, level, 1) != 2) {
        debug_printf("Failed sending communications failsafe state!\n");
        return false;
    }
    return true;
}

int Thunderborg::GetCommsFailsafe() {
    char resp[I2C_MAX_LEN];
    if (RawRead(COMMAND_GET_FAILSAFE, resp, I2C_MAX_LEN) == I2C_MAX_LEN) {
        if (resp[1] == COMMAND_VALUE_OFF) return false;
        else if (resp[1] == COMMAND_VALUE_ON) return true;
    } else {
        debug_printf("Failed reading communications failsafe state!\n");
    }

    return -1;
}

int Thunderborg::GetDriveFault1() {
    char resp[I2C_MAX_LEN];
    if (RawRead(COMMAND_GET_DRIVE_A_FAULT, resp, I2C_MAX_LEN) == I2C_MAX_LEN) {
        if (resp[1] == COMMAND_VALUE_OFF) return false;
        else if (resp[1] == COMMAND_VALUE_ON) return true;
    } else {
        debug_printf("Failed reading the drive fault state for motor #1!\n");
    }

    return -1;
}

int Thunderborg::GetDriveFault2() {
    char resp[I2C_MAX_LEN];
    if (RawRead(COMMAND_GET_DRIVE_B_FAULT, resp, I2C_MAX_LEN) == I2C_MAX_LEN) {
        if (resp[1] == COMMAND_VALUE_OFF) return false;
        else if (resp[1] == COMMAND_VALUE_ON) return true;
    } else {
        debug_printf("Failed reading the drive fault state for motor #2!\n");
    }

    return -1;
}

float Thunderborg:: GetBatteryReading() {
    float level = NAN;

    char resp[I2C_MAX_LEN];
    if (RawRead(COMMAND_GET_BATT_VOLT, resp, I2C_MAX_LEN) == I2C_MAX_LEN) {
        int raw = (resp[1] << 8) + resp[2];
        level = (float) raw / (float) COMMAND_ANALOG_MAX;
        level *= VOLTAGE_PIN_MAX;
        level += VOLTAGE_PIN_CORRECTION;
    } else {
        debug_printf("Failed reading battery level!\n");
    }

    return level;
}

bool Thunderborg::SetBatteryMonitoringLimits(float minimum, float maximum) {
    minimum = minimum / float(VOLTAGE_PIN_MAX);
    maximum = maximum / float(VOLTAGE_PIN_MAX);
    char levelMinMax[] = {
        std::max<char>(0, std::min<char>(0xFF, (int) (minimum * 0xFF))),
        std::max<char>(0, std::min<char>(0xFF, (int) (maximum * 0xFF)))
    };

    if (RawWrite(COMMAND_SET_BATT_LIMITS, levelMinMax, 2) != 3) {
        debug_printf("Failed sending battery monitoring limits!\n");
        sleep(0.2);
        return false;
    }
    sleep(0.2);
    return true;
}

bool Thunderborg::GetBatteryMonitoringLimits(float* minimum, float* maximum) {
    *minimum = NAN;
    *maximum = NAN;

    char resp[I2C_MAX_LEN];
    if (RawRead(COMMAND_GET_BATT_LIMITS, resp, I2C_MAX_LEN) == I2C_MAX_LEN) {
        int rawMin = resp[1];
        int rawMax = resp[2];

        *minimum = (float) rawMin / (float) 0xFF * VOLTAGE_PIN_MAX;
        *maximum = (float) rawMax / (float) 0xFF * VOLTAGE_PIN_MAX;
    } else {
        debug_printf("Failed reading battery monitoring limits!\n");
        return false;
    }
    return true;
}

bool Thunderborg::WriteExternalLedWord(char b0, char b1, char b2, char b3) {
    char b[] = {
        std::max<char>(0, std::min<char>(PWM_MAX, b0)),
        std::max<char>(0, std::min<char>(PWM_MAX, b1)),
        std::max<char>(0, std::min<char>(PWM_MAX, b2)),
        std::max<char>(0, std::min<char>(PWM_MAX, b3))
    };

    if (RawWrite(COMMAND_SET_BATT_LIMITS, b, 4) != 5) {
        debug_printf("Failed sending word for the external LEDs!\n");
        return false;
    }
    return true;
}

bool Thunderborg::SetExternalLedColours(float colors[][3], int numColors) {

    // Send the start marker
    WriteExternalLedWord(0, 0, 0, 0);

    bool err = false;

    for (int i = 0; i < numColors; i++) {
        err |= WriteExternalLedWord(255, 255*(char)colors[i][2], 255*(char)colors[i][1], 255*(char)colors[i][0]);
        printf("%d %d %d\n", 255*(char)colors[i][2], 255*(char)colors[i][1], 255*(char)colors[i][0]);
    }

    return err;

}
