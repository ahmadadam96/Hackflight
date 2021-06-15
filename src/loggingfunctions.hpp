#pragma once

#include "debugger.hpp"
#include <String>
#include "board.hpp"

namespace hf {
    char buf[30000];
    int index = 0;
    va_list ap;

    static void print_string(const char * fmt, ...){
        va_start(ap, fmt);
        
        int size = vsnprintf(&buf[index], 200, fmt, ap);

        index += size;

        if(index >= 29800){
            printf("Writing to serial started at,%d\n", micros());
            Serial.write(buf, index);
            printf("Writing to serial completed at,%d\n", micros());
            index = 0;
        }
        va_end(ap);
    }

    void printTaskTime(int task_id, bool task_start)
    {
        if (task_start)
            print_string("Task,%d,started at time,%ld\n", task_id, micros());
        else
            print_string("Task,%d,terminated at time,%ld\n", task_id, micros());
    }
}

