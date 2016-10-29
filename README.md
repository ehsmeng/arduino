# arduino


## TimeSlot

This is a quick cooperative "sheduler" for tasks. Imagine you want to blink one leds in 3Hz, 4Hz, and 5Hz. How do you do that in a pretty way?

Put TimeSlot in ~/Documents/Arduino/libraries (or corresponding place in your OS). Then you can just use it by including TimeSlot.h

```c++
#include <limits.h>
#include "TimeSlot.h"
#include <Servo.h>

const int cpupin = 13;
const int buttonpin = 2;
const int ledpin = 10;
const int blinkpin = 11;
const int servopin = 5;

class ClickEventListenerIF {
public:
    virtual void click(char state) = 0; // HIGH LOW
};

class myButton : public TimeSlotButton
{
public:
    int m_ledpin;
    ClickEventListenerIF *m_click_event_listener;
    myButton(int pin, int ledpin, ClickEventListenerIF *listener=0)
      : TimeSlotButton(pin), m_ledpin(ledpin), m_click_event_listener(listener) {}
    void setup() {
        pinMode(ledpin, OUTPUT);
    }
    void on() {
        if (0 != m_click_event_listener) {
            m_click_event_listener->click(HIGH);
        }
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

class ServoSweep : public TimeSlotChildIF, public ClickEventListenerIF
{
public:
    char m_state;
    char m_outpin;
    int m_degree;
    Servo *m_servo;
    ServoSweep(char outpin) : m_state(0), m_outpin(outpin) {
        m_servo = new Servo();
        m_degree = 10;
    }
    void setup() {
        pinMode(m_outpin, OUTPUT);
        m_servo->attach(m_outpin);
        m_servo->write(m_degree);
    }
    void click(char state) {
        if (m_state != 0 || state != HIGH) {
        //    return;
        }
        m_state = 1;
    }
    int tick() {
        if (1 == m_state) {
            m_degree++;
            m_degree++;
            m_servo->write(m_degree);
            if (170 <= m_degree) {
                m_state = -1;
            }
        } else if(-1 == m_state) {
            m_degree--;
            m_degree--;
            m_servo->write(m_degree);
            if (10 >= m_degree) {
                m_state = 0;
            }
        }
        return 1;
    }

};


ServoSweep *servoSweep = new ServoSweep(servopin);
TimeSlotChildIF *tasklist[] = {
    servoSweep,
    new myButton(buttonpin, ledpin, (ClickEventListenerIF *)servoSweep),
    new blinker(blinkpin,50),
    NULL
};

void setup()
{
    TimeSlot::m_cpupin = cpupin;
    TimeSlot::setup(tasklist);
    Serial.begin(9600);
}

int ledstate = 0;
int oddeven=0;
void loop()
{
    TimeSlot::loop(tasklist);
}

```
