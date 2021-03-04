#pragma once

#include <string>

namespace hf {
    void printTaskTime(const std::string& task_name, bool task_start)
    {
        Serial.print("Task ");
        Serial.print(task_name);

        if (task_start)
            Serial.print(" has started at time ");
        else
            Serial.print(" has terminated at time ");

        Serial.print(micros());

        Serial.print("\n");
        }
}

