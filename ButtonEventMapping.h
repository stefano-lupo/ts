#pragma once

#include ".\headers\Event.h"

class ButtonEventMapping {
    public:
        ButtonEventMapping(unsigned long keyCode, Event event): _keyCode(keyCode), _event(event) {};
        unsigned long _keyCode;
        Event _event;
};
