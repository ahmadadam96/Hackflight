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
            Serial.write(buf, index);
            index = 0;
        }
        va_end(ap);
    }

    void printTaskTime(String task_name, bool task_start)
    {
        print_string("Task ");
        print_string("%s", task_name.c_str());

        if (task_start)
            print_string(" has started at time,");
        else
            print_string(" has terminated at time,");

        print_string("%ld\n", micros());
    }
}

