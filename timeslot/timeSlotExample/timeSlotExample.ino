#include <limits.h>
#include "TimeSlot.h"

const int cpupin = 13;
const int buttonpin = 2;
const int ledpin = 10;
const int blinkpin = 11;

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

TimeSlotChildIF *tasklist[] = {
    new myButton(buttonpin, ledpin),
    new blinker(blinkpin,50),
    NULL
};

void setup()
{
    TimeSlot::m_cpupin = cpupin;
    TimeSlot::setup(tasklist);
}

int ledstate = 0;
int oddeven=0;
void loop()
{
    TimeSlot::loop(tasklist);
}

