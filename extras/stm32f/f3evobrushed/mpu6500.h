/*
   mpu6500.h : Ad-hoc implementation of MPU6500 IMU

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

#pragma once

extern "C" {

#include <stdint.h>
#include <stdbool.h>

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


    class MPU6500 {

        private:

            busDevice_t * _bus;

        public:

            MPU6500(busDevice_t * bus);

            bool ready(void);

            void readAccel(int16_t & ax, int16_t & ay, int16_t & az);

            void readGyro(int16_t & gx, int16_t & gy, int16_t & gz);
    };

}

