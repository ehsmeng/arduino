# arduino


## TimeSlot

This is a quick cooperative "sheduler" for tasks. Imagine you want to blink one leds in 3Hz, 4Hz, and 5Hz. How do you do that in a pretty way?

Put TimeSlot in ~/Documents/Arduino/libraries (or corresponding place in your OS). Then you can just use it by including TimeSlot.h

```c++
#include <limits.h>
#include "TimeSlot.h"

const int cpupin = 13;
const int buttonpin = 2;
const int ledpin = 10;
const int blinkpin = 11;

// This takes care of mechanical bounce too, as it's not checked in a tight loop.
class myButton : public TimeSlotButton
{
public:
    int m_ledpin;
    myButton(int pin, int ledpin) : TimeSlotButton(pin), m_ledpin(ledpin) {}
    void setup() {
        pinMode(ledpin, OUTPUT);
    }
    void onoff() {
        digitalWrite(ledpin, m_state);
    }
};

class blinker : public TimeSlotChildIF
{
public:
    int m_ledpin;
    int m_sleepslots;
    int m_blinkstate;
    blinker(int ledpin, int sleepslots) : m_ledpin(ledpin),  m_sleepslots(sleepslots), m_blinkstate(0)  { }

    void setup() {
        pinMode(m_ledpin, OUTPUT);
    }

    int tick() {
        m_blinkstate ^=1;
        digitalWrite(m_ledpin, m_blinkstate);
        return m_sleepslots;
    }
};

// You can add tasks dynamically too. This is the list of static tasks.
TimeSlotChildIF *tasklist[] = {
    new myButton(buttonpin, ledpin),
    new blinker(blinkpin,50),
    NULL
};

void setup()
{
    // if set, the timeslot loop raises this pin when working. Then you can see in your dso how much cpu is used.
    TimeSlot::m_cpupin = cpupin;
    TimeSlot::setup(tasklist);
}

int ledstate = 0;
int oddeven=0;
void loop()
{
    TimeSlot::loop(tasklist);
}
```
