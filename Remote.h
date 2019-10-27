#pragma once

#include <IRremote.h>
#include ".\headers\Event.h"
#include ".\headers\ButtonEventMapping.h"

#define x = 1;

class Remote {
    public:
        // Remote(int pin);
        Remote(int pin): _pin(pin), _irRecv(pin) {};
        void setup();
        Event checkForPress();
    
    private:
        int _pin;
        IRrecv _irRecv;
        decode_results &_decodedSig;
        Event decodeKeyPress(unsigned long keyCode);
};
