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

namespace hf
{
    class UpdateScheduler
    {
        friend class Hackflight;

        // task order receiver, PID, sensor, serial
        #define number_of_tasks

        struct task_info {
            int task_id;
            int time_task_ended;
            int period;
            int time_next_invocation;
        };

        task_info task_infos[number_of_tasks];

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

            if (min_value - current_time < update_time_required)
                return micros();
            else
                return 0;
        }

    };  // SerialTask

}  // namespace hf
