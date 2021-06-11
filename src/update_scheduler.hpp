/*
   Timer task for serial comms

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

#include "loggingfunctions.hpp"
#include <vector>
#include <limits.h>
namespace hf
{
    class UpdateScheduler
    {
        friend class Hackflight;

        struct task_info {
            int time_task_ended = 0;
            int period = 0;
            int time_next_invocation = 0;
        };
        unsigned int update_time_required;
        unsigned int number_of_tasks;

        bool update_scheduled = false;

       public:
        std::vector<task_info> task_infos;

        // task id 0 receiver task
        // task id 1 PID task
        // task id 2 serial communication task
        // task ids 3+ sensor tasks
        UpdateScheduler(unsigned int sensor_count = 2, unsigned int update_time_required = 1520)
        {
            number_of_tasks = sensor_count + 3;
            hf::UpdateScheduler::update_time_required = update_time_required;
            for(int i = 0; i< number_of_tasks; i++){
                task_infos.push_back(task_info());
            }
        }
        
        void set_task_period(int task_id, unsigned int period){
            task_infos[task_id].period = period;
        }

        void task_completed(int task_id)
        {
            task_infos[task_id].time_task_ended = micros();
            task_infos[task_id].time_next_invocation = task_infos[task_id].time_task_ended\
                + task_infos[task_id].period;   
            if(!update_scheduled) when_schedule_update(update_time_required);
        }

        unsigned int when_schedule_update(unsigned int update_time_required)
        {
            unsigned int min_value = UINT_MAX;
            unsigned int min_index;
            for (unsigned int i = 0; i < number_of_tasks; i++) {
                if (task_infos[i].time_next_invocation < min_value) {
                    min_value = task_infos[i].time_next_invocation;
                    min_index = i;
                }
            }
            unsigned int current_time = micros();
            print_string("Minimum value ,%d, current time ,%d\n", min_value, current_time);

            if (min_value > current_time && min_value - current_time > update_time_required)
            {
                print_string("Update of size ,%d scheduled at time ,%d\n", update_time_required, current_time);
                update_scheduled = true;
                return current_time;
            }

            else{
                print_string("Update scheduling failed\n");
                return 0;
            }
        }

        void take_actions()
        {
            /*
            What sort of actions to take?
            1. Disable the receiver
            2. Switch serial task to dynamic frequency
            3. Turn on reactive control
            4. Use real-time calculus frequencies for sensors
            */
        }
    }; 


}  // namespace hf
