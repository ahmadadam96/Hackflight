/*
   Hackflight core algorithm

   Copyright (c) 2020 Simon D. Levy

   This file is part of Hackflight.

   Hackflight is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Hackflight is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MEReceiverHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with Hackflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "debugger.hpp"
#include "mspparser.hpp"
#include "imu.hpp"
#include "board.hpp"
#include "receiver.hpp"
#include "datatypes.hpp"
#include "pidcontroller.hpp"
#include "motor.hpp"
#include "mixer.hpp"
#include "sensors/surfacemount.hpp"
#include "timertasks/pidtask.hpp"
#include "timertasks/serialtask.hpp"
#include "sensors/surfacemount/gyrometer.hpp"
#include "sensors/surfacemount/quaternion.hpp"
#include "loggingfunctions.hpp"
#include "update_scheduler.hpp"

namespace hf {

    class Hackflight {

        private:

            static constexpr float MAX_ARMING_ANGLE_DEGREES = 25.0f;

            // Supports periodic ad-hoc debugging
            Debugger _debugger;

            // Sensors 
            Sensor * _sensors[256] = {NULL};
            uint8_t _sensor_count = 0;

            // Safety
            bool _safeToArm = false;

            // Support for headless mode
            float _yawInitial = 0;

            // Timer task for PID controllers
            PidTask _pidTask;

            // Passed to Hackflight::init() for a particular build
            IMU        * _imu      = NULL;
            Mixer      * _mixer    = NULL;

            // Serial timer task for GCS
            SerialTask _serialTask;

             // Mandatory sensors on the board
            Gyrometer _gyrometer;
            Quaternion _quaternion; // not really a sensor, but we treat it like one!

            bool safeAngle(uint8_t axis)
            {
                return fabs(_state.rotation[axis]) < Filter::deg2rad(MAX_ARMING_ANGLE_DEGREES);
            }

            Board    * _board    = NULL;
            Receiver * _receiver = NULL;

            // Update scheduler with the sensor count
            UpdateScheduler _update_scheduler;

            // Vehicle state
            state_t _state;

            void checkSensors(void)
            {
                for (uint8_t k=0; k<_sensor_count; ++k) {
                    Sensor * sensor = _sensors[k];
                    float time = _board->getTime();
                    if (sensor->ready(time)) {
                        printTaskTime(k+2, true);
                        sensor->modifyState(_state, time);
                        printTaskTime(k+2, false);
                        _update_scheduler.task_completed(k+2);
                    }
                }
            }

            void add_sensor(Sensor * sensor)
            {
                _sensors[_sensor_count++] = sensor;
            }

            void add_sensor(SurfaceMountSensor * sensor, IMU * imu) 
            {
                add_sensor(sensor);

                sensor->imu = imu;
            }

            void add_sensor(SurfaceMountSensor* sensor, IMU* imu, unsigned int sensor_frequency)
            {
                add_sensor(sensor, imu);

                _update_scheduler.set_task_period(1 + _sensor_count, 1000000 / sensor_frequency);
            }

            void general_init(Board * board, Receiver * receiver, Mixer * mixer)
            {  

                // Store the essentials
                _board    = board;
                _receiver = receiver;
                _mixer = mixer;

                // Ad-hoc debugging support
                _debugger.init(board);

                // Support adding new sensors and PID controllers
                _sensor_count = 0;

                // Initialize state
                memset(&_state, 0, sizeof(state_t));

                // Initialize the receiver
                _receiver->begin();

                // Setup failsafe
                _state.failsafe = false;

                _update_scheduler.init(2, 1520, _receiver);

                // Initialize timer task for PID controllers
                _pidTask.init(_board, _receiver, _mixer, &_state, &_update_scheduler);
            }

            void checkReceiver(void)
            {
                // Sync failsafe to receiver
                if (_receiver->lostSignal() && _state.armed) {
                    _mixer->cut();
                    _state.armed = false;
                    Debugger::printf("Disarmed\n");
                    _state.failsafe = true;
                    _board->showArmedStatus(false);
                    return;
                }

                // Check whether receiver data is available
                if (!_receiver->getDemands(_state.rotation[AXIS_YAW] - _yawInitial)) return;

                // Disarm
                if (_state.armed && !_receiver->getAux1State()) {
                    _state.armed = false;
                    Debugger::printf("Disarmed\n");
                } 

                // Avoid arming if aux1 switch down on startup
                if (!_safeToArm) {
                    _safeToArm = !_receiver->getAux1State();
                }

                // Arm (after lots of safety checks!)
                if (_safeToArm && !_state.armed && _receiver->throttleIsDown() && _receiver->getAux1State() && 
                        !_state.failsafe && safeAngle(AXIS_ROLL) && safeAngle(AXIS_PITCH)) {
                    _state.armed = true;
                    Debugger::printf("Armed\n");
                    _yawInitial = _state.rotation[AXIS_YAW]; // grab yaw for headless mode
                }

                // Cut motors on throttle-down
                if (_state.armed && _receiver->throttleIsDown()) {
                    _mixer->cut();
                }

                // Set LED based on arming status
                _board->showArmedStatus(_state.armed);

                printTaskTime(1000, false);
            } // checkReceiver

        public:

            void init(Board * board, IMU * imu, Receiver * receiver, Mixer * mixer, Motor * motors, bool armed=false)
            {  
                // Do general initialization
                general_init(board, receiver, mixer);

                // Store pointers to IMU, mixer
                _imu   = imu;
                _mixer = mixer;

                // Initialize serial timer task
                _serialTask.init(board, &_state, receiver, mixer, &_update_scheduler);

                // Support safety override by simulator
                _state.armed = armed;

                // Support for mandatory sensors
                // frequencies from usfs.hpp
                add_sensor(&_quaternion, imu, 66);
                add_sensor(&_gyrometer, imu, 330);

                // Start the IMU
                imu->begin();

                // Tell the mixer which motors to use, and initialize them
                mixer->useMotors(motors);

            } // init

            void addSensor(Sensor * sensor) 
            {
                add_sensor(sensor);
            }

            void addPidController(PidController * pidController, uint8_t auxState=0) 
            {
                _pidTask.addPidController(pidController, auxState);
            }

            void update(void)
            {
                static unsigned int count = 0;
                if(millis() > 20000 && count == 0){
                    _update_scheduler.initialize_scheduling(2000);
                    count++;
                }

                // Grab control signal if available
                checkReceiver();

                // Update PID controllers task
                _pidTask.update();

                // Check sensors
                checkSensors();

                // Update serial comms task
                _serialTask.update();

            }

    }; // class Hackflight

} // namespace
