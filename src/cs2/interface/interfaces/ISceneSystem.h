#pragma once
#include "core/mem/memory.h"
class c_light_data_queue
{
public:
    char pad_0000[24]; // 0x0000
    void* light_data; // 0x0018
    char pad_0020[4]; // 0x0020
};

class ISceneSystem
{
public:
    char pad_0000[0x2A08]; // 0x0000
    c_light_data_queue* light_data_queue; // 0x2AC8

    void DeleteSceneObject(void* object) {
        if (!object)
            return;

        memory::CallVFunc<void, 92>(this, object);
    }
};