/*
   Hackflight sketch for Ladybug Flight Controller with Spektrum DSMX receiver

   Additional libraries needed:

       https://github.com/simondlevy/EM7180
       https://github.com/simondlevy/CrossPlatformDataBus
       https://github.com/simondlevy/SpektrumDSM 

   Hardware support for Ladybug flight controller:

       https://github.com/simondlevy/grumpyoldpizza

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

#include <Arduino.h>

#include "hackflight.hpp"
#include "boards/realboards/arduino/ladybugfc.hpp"
#include "receivers/arduino/cppm.hpp"
#include "mixers/quadxcf.hpp"
#include "pidcontrollers/rate.hpp"
#include "pidcontrollers/level.hpp"
#include "pidcontrollers/althold.hpp"
#include "debugger.hpp"

// throttle, roll, pitch, yaw, aux, arm 
static constexpr uint8_t CHANNEL_MAP[6] = {2, 3, 1, 0, 4, 5};
static constexpr float DEMAND_SCALE = 1.0f;

hf::Hackflight h;

hf::CPPM_Receiver rc = hf::CPPM_Receiver(0, CHANNEL_MAP, DEMAND_SCALE);

hf::MixerQuadXCF mixer;

hf::RatePid ratePid = hf::RatePid(0.225, 0.001875, 0.375, 1.0625, 0.005625f);

hf::LevelPid levelPid = hf::LevelPid(0.20f);

hf::AltitudeHoldPid altholdPid = hf::AltitudeHoldPid(
    1.00f,   // Altitude Hold P
    0.15f,   // Altitude Hold Velocity P
    0.01f,   // Altitude Hold Velocity I
    0.05f);  // Altitude Hold Velocity D

void setup(void)
{
    delay(3000);
    // Initialize Hackflight firmware
    h.init(new hf::LadybugFC(), &hf::ladybugIMU, &rc, &mixer, &hf::ladybugFcNewMotors);

    hf::Debugger::printf("Started\n");

    // Add PID controllers
    h.addPidController(&levelPid);
    h.addPidController(&ratePid);
    h.addPidController(&altholdPid, 1);
}

void loop(void)
{
    h.update();
}
