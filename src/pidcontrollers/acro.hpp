/*
   Acro-mode PID controller 

   Copyright (c) 2018 Juan Gallostra and Simon D. Levy

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
#include "filters.hpp"
#include "datatypes.hpp"
#include "rate.hpp"
#include "pidcontroller.hpp"

namespace hf {

    class AcroPid : public PidController {

        friend class Hackflight;

        private: 

        // Acro mode uses a rate controller for roll, pitch
        RatePid _rollPid;
        RatePid _pitchPid;

        float maxval(float a, float b)
        {
            return a > b ? a : b;
        }

        protected:

        // proportion of cyclic demand compared to its maximum
        float _proportionalCyclicDemand;

        public:

        AcroPid(const float P, const float I, const float D,float demandsScale=1.0f) 
            : _demandsScale(demandsScale)
        {
            _rollPid.init(P, I, D, demandScale);
            _pitchPid.init(P, I, D, demandScale);
        }

        bool modifyDemands(state_t & state, demands_t & demands, float currentTime)
        {
            (void)currentTime;

            _PTerm[0] = demands.roll;
            _PTerm[1] = demands.pitch;

            // Pitch, roll use Euler angles
            demands.roll  = computeCyclicPid(demands.roll,  state.angularVel, AXIS_ROLL);
            demands.pitch = computeCyclicPid(demands.pitch, state.angularVel, AXIS_PITCH);

            // We've always gotta do this!
            return true;
        }

        virtual void updateReceiver(demands_t & demands, bool throttleIsDown) override
        {
            // Compute proportion of cyclic demand compared to its maximum
            _proportionalCyclicDemand = maxval(fabs(demands.roll), fabs(demands.pitch)) / 0.5f;

            // When landed, reset integral component of PID
            if (throttleIsDown) {
                resetIntegral();
            }
        }

    };  // class AcroPid

} // namespace hf
