#pragma once

#include "debugger.hpp"
#include <String>

namespace hf {
    void printTaskTime(String task_name, bool task_start)
    {
        Debugger::printf("Task ");
        Debugger::printf("%s", task_name.c_str());

        if (task_start)
            Debugger::printf(" has started at time ");
        else
            Debugger::printf(" has terminated at time ");

        Debugger::printf("%ld\n", micros());
        }
}

