/*
   Abstract class for timer tasks

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

#include "board.hpp"
#include "debugger.hpp"

namespace hf {

    class TimerTask {

        private:

            float _time = 0;
            bool _enabled = true;

        protected:

            Board * _board = NULL;

            TimerTask(float freq)
            {
                period = 1 / freq;
                _time = 0;
            }

            void init(Board * board)
            {
                _board = board;
            }

            virtual void doTask(void) = 0;

            virtual bool specific_conditions(void) = 0;

        public:

            float period = 0;

            void update(void)
            {
                if (specific_conditions() && basic_conditions())
                {
                    doTask();
                }
            }

            bool basic_conditions(void){
                float time = _board->getTime();
                return time - _time > period && _enabled;
            }

    };  // TimerTask

} // namespace hf
