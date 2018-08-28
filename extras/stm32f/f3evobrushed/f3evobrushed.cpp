/*
   f3evobrushed.h : Board implmentation for Hyperion F3 Evo Brushed board

   Copyright (C) 2018 Simon D. Levy 

   This file is part of Hackflight.

   Hackflight is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Hackflight is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Hackflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "f3evobrushed.h"

static const uint16_t BRUSHED_PWM_RATE     = 32000;
static const uint16_t BRUSHED_IDLE_PULSE   = 0; 

static const float    MOTOR_MIN = 1000;
static const float    MOTOR_MAX = 2000;

// Here we put code that interacts with Cleanflight
extern "C" {

#include "platform.h"

#include "drivers/system.h"
#include "drivers/timer.h"
#include "drivers/time.h"
#include "drivers/pwm_output.h"
#include "drivers/light_led.h"
#include "drivers/serial.h"
#include "drivers/serial_uart.h"
#include "drivers/serial_usb_vcp.h"
#include "drivers/bus_spi.h"
#include "drivers/bus_spi_impl.h"
#include "pg/bus_spi.h"

#include "io/serial.h"

#include "target.h"

#include "stm32f30x.h"

    static serialPort_t * _serial0;

    static busDevice_t _bus;

    F3EvoBrushed::F3EvoBrushed(void)
    {
        initMotors();
        initUsb();
        initImu();

        RealBoard::init();
    }

    void F3EvoBrushed::initImu(void)
    {
        spiPinConfigure(spiPinConfig(0));
        spiPreInit();

        SPIDevice spiDevice = SPIINVALID;

        if (MPU6500_SPI_INSTANCE == SPI1) {
            spiDevice = SPIDEV_1;
        }

        else if (MPU6500_SPI_INSTANCE == SPI2) {
            spiDevice = SPIDEV_2;
        }

        else if (MPU6500_SPI_INSTANCE == SPI3) {
            spiDevice = SPIDEV_3;
        }

        spiInit(spiDevice);

        spiBusSetInstance(&_bus, MPU6500_SPI_INSTANCE);

        _bus.busdev_u.spi.csnPin = IOGetByTag(IO_TAG(MPU6500_CS_PIN));

        delay(100);

        IOInit(_bus.busdev_u.spi.csnPin, OWNER_MPU_CS, 0);
        IOConfigGPIO(_bus.busdev_u.spi.csnPin, SPI_IO_CS_CFG);
        IOHi(_bus.busdev_u.spi.csnPin);
        spiSetDivisor(_bus.busdev_u.spi.instance, SPI_CLOCK_FAST);

        _imu = new MPU6500(&_bus);
    }

    void F3EvoBrushed::initUsb(void)
    {
        _serial0 = usbVcpOpen();
    }

    void F3EvoBrushed::initMotors(void)
    {
        motorDevConfig_t dev;

        dev.motorPwmRate = BRUSHED_PWM_RATE;
        dev.motorPwmProtocol = PWM_TYPE_BRUSHED;
        dev.motorPwmInversion = false;
        dev.useUnsyncedPwm = true;
        dev.useBurstDshot = false;

        dev.ioTags[0] = timerioTagGetByUsage(TIM_USE_MOTOR, 0);
        dev.ioTags[1] = timerioTagGetByUsage(TIM_USE_MOTOR, 1);
        dev.ioTags[2] = timerioTagGetByUsage(TIM_USE_MOTOR, 2);
        dev.ioTags[3] = timerioTagGetByUsage(TIM_USE_MOTOR, 3);

        motorDevInit(&dev, BRUSHED_IDLE_PULSE, 4);

        pwmEnableMotors();
    }

    void F3EvoBrushed::writeMotor(uint8_t index, float value)
    {
        pwmWriteMotor(index, MOTOR_MIN + value*(MOTOR_MAX-MOTOR_MIN));
    }

    void systemBeep(bool ignore) { (void)ignore; } // XXX to satisfy EXTI

    void F3EvoBrushed::delaySeconds(float sec)
    {
        delay((uint16_t)(sec*1000));
    }

    void F3EvoBrushed::setLed(bool is_on)
    {
        ledSet(0, is_on);
    }

    uint32_t F3EvoBrushed::getMicroseconds(void)
    {
        return micros();
    }

    void F3EvoBrushed::reboot(void)
    {
        systemResetToBootloader();
    }

    uint8_t F3EvoBrushed::serialAvailableBytes(void)
    {
        return serialRxBytesWaiting(_serial0);
    }

    uint8_t F3EvoBrushed::serialReadByte(void)
    {
        return serialRead(_serial0);
    }

    void F3EvoBrushed::serialWriteByte(uint8_t c)
    {
        serialWrite(_serial0, c);
    }

    bool F3EvoBrushed::imuRead(void)
    {
        if (_imu->ready()) {

            int16_t ax=0, ay=0, az=0;
            _imu->readAccel(ax, ay, az);

            int16_t gx=0, gy=0, gz=0;
            _imu->readGyro(gx, gy, gz);

            hf::Debug::printf("%d %d %d %d %d %d\n", ax, ay, az, gx, gy, gz);

            return true;
        }

        return false;
    }

    void hf::Board::outbuf(char * buf)
    {
        for (char *p=buf; *p; p++)
            serialWrite(_serial0, *p);
    }

} // extern "C"
