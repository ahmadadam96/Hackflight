/*
   ESP32 Arduino code for DSHOT600 protocol

   Copyright (c) 2020 Simon D. Levy

   Adapted from https://github.com/JyeSmith/dshot-esc-tester/blob/master/dshot-esc-tester.ino, 
   which contains the following licensing notice:

   "THE PROP-WARE LICENSE" (Revision 42):
   <https://github.com/JyeSmith> wrote this file.  As long as you retain this notice you
   can do whatever you want with this stuff. If we meet some day, and you think
   this stuff is worth it, you can buy me some props in return.   Jye Smith

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

#include <stdint.h>
#include <stdarg.h>

#include "esp32-hal.h"

namespace hf {

    class Esp32DShot600 : public Motor {

        private:

            static constexpr uint16_t LEVEL_MIN = 48;
            static constexpr uint16_t LEVEL_ARM = 75;
            static constexpr uint16_t LEVEL_MAX = 2047;

            static constexpr uint16_t STARTUP_MSEC  = 3500;
            static constexpr uint16_t INITIAL_DELAY = 1000;

            typedef struct {

                rmt_data_t dshotPacket[16];
                rmt_obj_t * rmt_send;
                uint16_t outputValue;
                bool requestTelemetry;
                uint8_t receivedBytes;
                uint8_t pin;

            } motor_t;

            motor_t _motors[MAX_COUNT] = {};

            bool armed = false;

            static void coreTask(void * params)
            {
                Esp32DShot600 * dshot = (Esp32DShot600 *)params;

                while (true) {

                    if (dshot->armed) {

                        for (uint8_t k=0; k<dshot->_count; ++k) {
                            motor_t * motor = &dshot->_motors[k];  
                            dshot->outputOne(motor);
                        }

                    }

                    delay(1);
                } 
            }

            void outputOne(motor_t * motor)
            {
                uint16_t packet = (motor->outputValue << 1) /* | (motor->telemetry ? 1 : 0) */ ;

                int csum = 0;
                int csum_data = packet;
                for (int i = 0; i < 3; i++) {
                    csum ^=  csum_data;
                    csum_data >>= 4;
                }
                csum &= 0xf;
                packet = (packet << 4) | csum;

                // Durations are for dshot600: https://blck.mn/2016/11/dshot-the-new-kid-on-the-block/
                // Bit length (total timing period) is 1.67 microseconds (T0H + T0L or T1H + T1L).
                // For a bit to be 1, pulse width is 1250 nsec (T1H – time the pulse is high for a bit value of ONE).
                // For a bit to be 0, pulse width is 625 nsec (T0H – time the pulse is high for a bit value of ZERO).
                for (int i = 0; i < 16; i++) {
                    motor->dshotPacket[i].level0 = 1;
                    motor->dshotPacket[i].level1 = 0;
                    if (packet & 0x8000) {
                        motor->dshotPacket[i].duration0 = 100;
                        motor->dshotPacket[i].duration1 = 34;
                    } else {
                        motor->dshotPacket[i].duration0 = 50;
                        motor->dshotPacket[i].duration1 = 84;
                    }
                    packet <<= 1;
                }

                rmtWrite(motor->rmt_send, motor->dshotPacket, 16);

            } // outputOne

            void outputAll(uint16_t val) {

                for (uint8_t k=0; k<_count; ++k) {
                    _motors[k].outputValue = val;
                }
            }

        public:

            Esp32DShot600(const uint8_t pins[], const uint8_t count) 
                : Motor(pins, count)
            {
                for (uint8_t k=0; k<count; ++k) {
                    _motors[k].pin = pins[k];
                }
            }

            void init(void)
            {
                for (uint8_t k=0; k<_count; ++k) {

                    motor_t * motor = &_motors[k];

                    if ((motor->rmt_send = rmtInit(motor->pin, true, RMT_MEM_64)) == NULL) {
                        return;
                    }

                    rmtSetTick(motor->rmt_send, 12.5); // 12.5ns sample rate
                }

                for (uint32_t start = millis(); millis()-start < STARTUP_MSEC; ) {

                    for (uint8_t k=0; k<_count; ++k) {

                        motor_t * motor = &_motors[k];

                        // Output disarm signal while esc initialises
                        motor->outputValue = LEVEL_MIN;
                        outputOne(motor);
                    }

                    delay(1);  
                }

                delay(INITIAL_DELAY);

                armed = false;

                TaskHandle_t Task;
                xTaskCreatePinnedToCore(coreTask, "Task", 10000, this, 1, &Task, 0); 
            }

            void write(uint8_t index, float value)
            {
                _motors[index].outputValue = LEVEL_ARM + (uint16_t)(value * (LEVEL_MAX-LEVEL_ARM));
            }

            void arm(void)
            {
                armed = true;

                outputAll(LEVEL_ARM);
            }

            void disarm(void)
            {
                armed = false;

                outputAll(LEVEL_ARM);
            }

    }; // class Esp32DShot600

} // namespace hf
