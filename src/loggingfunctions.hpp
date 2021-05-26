#pragma once

#include "debugger.hpp"
#include <String>
#include "board.hpp"

namespace hf {
    char buf[30000];
    int index = 0;
    va_list ap;

    void print(const char * fmt, ...){
        va_start(ap, fmt);
        
        int size = vsnprintf(&buf[index], 200, fmt, ap);

        index += size;
        
        va_end(ap);

        if(index >= 29800){
            Serial.write(buf, index+1);
            index = 0;
        }
    }

    void printTaskTime(String task_name, bool task_start)
    {
        print("Task ");
        print("%s", task_name.c_str());

        if (task_start)
            print(" has started at time,");
        else
            print(" has terminated at time,");

        print("%ld\n", micros());
    }
}

