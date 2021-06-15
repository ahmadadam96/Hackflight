/*
   Timer task for PID controllers

   Copyright (c) 2020 Simon D. Levy

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

#include "timertask.hpp"
#include "loggingfunctions.hpp"
#include "update_scheduler.hpp"

namespace hf {

    class PidTask : public TimerTask {

        friend class Hackflight;

        private:

            // For now, we keep all PID timer tasks the same.  At some point it might be useful to 
            // investigate, e.g., faster updates for Rate PID than for Level PID.
            float FREQ = 300;

            static constexpr unsigned int task_id = 0;

            // PID controllers
            PidController * _pid_controllers[256] = {NULL};
            uint8_t _pid_controller_count = 0;

            // Other stuff we need
            Receiver * _receiver = NULL;
            Mixer * _mixer = NULL;
            state_t  * _state    = NULL;
            UpdateScheduler *_update_scheduler = NULL;

            demands_t previous_demands = {};
            state_t previous_state = {};


           protected:

            PidTask(void)
                : TimerTask(FREQ)
            {
                _pid_controller_count = 0;
            }

            void init(Board *board, Receiver *receiver, Mixer *mixer, state_t *state, UpdateScheduler *update_scheduler)
            {
                change_frequency(FREQ);
                TimerTask::init(board);

                _receiver = receiver;
                _mixer = mixer;
                _state = state;
                _update_scheduler = update_scheduler;
                _update_scheduler->set_task_period(0, 1000000/FREQ);
            }

            /*

            void update(void)
            {
                float time = _board->getTime();

                if ((time - _time) > _period)
                {
                    doTask();
                    _time = time;
                }
            }

            
            
            float reactive_control(){
                demands_t demands = {};
                demands.throttle = _receiver->demands.throttle;
                demands.roll = _receiver->demands.roll * _receiver->_demandScale;
                demands.pitch = _receiver->demands.pitch * _receiver->_demandScale;
                demands.yaw = _receiver->demands.yaw * _receiver->_demandScale;

                state_t state = *state;

                for(int i = 0; i<3; i++){
                    
                }


                float b1, b2;

                float L;

                for(all states){
                    float x //difference between new and old values of the state;
                    L = 1/(1+exp(-(b1+b2*x)));

                }                



                previous_demands = demands;
                previous_state = *_state;
            }
            */

            void addPidController(PidController * pidController, uint8_t auxState) 
            {
                pidController->auxState = auxState;

                _pid_controllers[_pid_controller_count++] = pidController;
            }

            virtual void doTask(void) override
            {
                printTaskTime(task_id, true);
                // Start with demands from receiver, scaling roll/pitch/yaw by constant
                demands_t demands = {};
                demands.throttle = _receiver->demands.throttle;
                demands.roll     = _receiver->demands.roll  * _receiver->_demandScale;
                demands.pitch    = _receiver->demands.pitch * _receiver->_demandScale;
                demands.yaw      = _receiver->demands.yaw   * _receiver->_demandScale;

                // Each PID controllers is associated with at least one auxiliary switch state
                uint8_t auxState = _receiver->getAux2State();

                //Debugger::printf("Aux state: %d", auxState);

                // Some PID controllers should cause LED to flash when they're active
                bool shouldFlash = false;

                for (uint8_t k=0; k<_pid_controller_count; ++k) {

                    PidController * pidController = _pid_controllers[k];

                    // Some PID controllers need to reset their integral when the throttle is down
                    pidController->updateReceiver(_receiver->throttleIsDown());

                    if (pidController->auxState <= auxState) {

                        pidController->modifyDemands(_state, demands); 

                        if (pidController->shouldFlashLed()) {
                            shouldFlash = true;
                        }
                    }
                }

                // Flash LED for certain PID controllers
                _board->flashLed(shouldFlash);

                // Use updated demands to run motors
                if (_state->armed && !_state->failsafe && !_receiver->throttleIsDown()) {
                    _mixer->run(demands);
                }
                printTaskTime(task_id, false);
                _update_scheduler->task_completed(task_id);
             }

    };  // PidTask

} // namespace hf
