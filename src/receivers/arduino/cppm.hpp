/*
   CPPM receiver support for Arduino-based flight controllers

   Copyright (c) 2018 Simon D. Levy

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

#include "receiver.hpp"
#include <CPPMRX.h>

namespace hf {

    class CPPM_Receiver : public Receiver {

        private:

            static const uint16_t PPM_MIN = 981;
            static const uint16_t PPM_MAX = 2074;

            CPPMRX * rx = NULL;

        protected:

            void begin(void)
            {
                rx->begin();
                receiver_running = true;
            }

            bool gotNewFrame(void)
            {
                if(!receiver_running) return false;
                return rx->gotNewFrame();
            }

            void readRawvals(void)
            {
                uint16_t rcData[6];

                rx->computeRC(rcData);

                for (uint8_t k=0; k<6; k++) {

                    rawvals[k] = 2.f * (rcData[k] - PPM_MIN) / (PPM_MAX - PPM_MIN) - 1;
                }
            }

            bool lostSignal(void)
            {
                return false;
            }

        public:

            bool receiver_running = false;

            CPPM_Receiver(uint8_t pin, const uint8_t channelMap[6], const float demandScale) 
                : Receiver(channelMap, demandScale) 
            { 
                rx = new CPPMRX(pin, 6);
            }

            void pause(){
                rx->pause();
                receiver_running = false;
            }

            void resume(){
                rx->resume();
                receiver_running = true;
            }

    }; // class CPPM_Receiver

} // namespace hf
