// Marcus Engene myfirstname@mylastname.se; BSD license.

#include "TimeSlot.h"
#include <limits.h>
#include "Arduino.h"

unsigned int TimeSlot::m_tickstart = 0;

int TimeSlot::m_cpupin = -1;

int TimeSlot::m_pid_sequence = 1;

// There's one user provided hard task list, and then this dynamic for adding
// and removing jobs
TimeSlotChildIF * TimeSlot::m_dynamicTasklist[10];

int TimeSlot::m_nbr_dynamic_tasks = 0;

unsigned int TimeSlot::m_us_delay = 10000;

TimeSlot::TimeSlot()
{
}

void TimeSlot::timerstart_tasklist()
{
    m_tickstart = micros();
}

void TimeSlot::timerstop_tasklist()
{
    unsigned int us_stop, us_total, us_start;

    us_start = m_tickstart;
    us_stop = micros();

    if (us_stop > us_start) {
        us_total = us_stop - us_start;
    } else {
        us_total = UINT_MAX - us_start;
        us_total += us_stop;
    }

    if (us_total < m_us_delay) {
        delayMicroseconds(m_us_delay - us_total);
    }
}

/**
 * Call from Arduino's setup() function
 */
void TimeSlot::setup(TimeSlotChildIF *tasklist[])
{
    if (0 <= m_cpupin) {
        pinMode(m_cpupin, OUTPUT);
    }

    if (NULL != tasklist)
    {
        for (int i=0; NULL != tasklist[i]; i++)
        {
            tasklist[i]->setup();
        }
    }
}

/**
 * Internal function called from TimeSlot::loop()
 */
void TimeSlot::tick_tasklist(TimeSlotChildIF *tasklist[])
{
    if (NULL != tasklist)
    {
        for (int i=0; NULL != tasklist[i]; i++)
        {
            TimeSlotChildIF *ts = tasklist[i];

            if (0 < ts->m_skip_ticks)
            {
                ts->m_skip_ticks--;
            }
            else if (0 == ts->m_skip_ticks)
            {
                ts->m_skip_ticks = ts->tick();
            }
            else
            {
                // if negative, then not called!
            }
        }
    }

    for (int i=0; i < m_nbr_dynamic_tasks; i++)
    {
        TimeSlotChildIF *ts = m_dynamicTasklist[i];

        if (0 < ts->m_skip_ticks)
        {
            ts->m_skip_ticks--;
        }
        else if (0 == ts->m_skip_ticks)
        {
            ts->m_skip_ticks = ts->tick();
        }
    }
}

/**
 * This is the loop function you should call from arduiono loop()
 */
void TimeSlot::loop(TimeSlotChildIF *tasklist[])
{
    // Keep track of execution time so we sleep accordingly to make calls to
    // tick evenly spread out.
    TimeSlot::timerstart_tasklist();

    if (0 <= m_cpupin)
    {
        digitalWrite(m_cpupin, HIGH);
    }

    // Executes both static and dynamic task list
    TimeSlot::tick_tasklist(tasklist);

    if (0 <= m_cpupin)
    {
        digitalWrite(m_cpupin, LOW);
    }

    TimeSlot::timerstop_tasklist();
}

int TimeSlot::add_dynamic_task(TimeSlotChildIF *task)
{
    task->setup();

    if (10 == m_nbr_dynamic_tasks)
    {
        delete task;
        return -1;
    }

    m_dynamicTasklist[m_nbr_dynamic_tasks++] = task;

    return task->m_pid;
}

void TimeSlot::kill_dynamic_task(int pid)
{
    for (int i=0; i < m_nbr_dynamic_tasks; i++)
    {
        TimeSlotChildIF *ts = m_dynamicTasklist[i];
        if (pid == ts->m_pid)
        {
            delete ts;
            for (int j=(i+1); j < m_nbr_dynamic_tasks; j++)
            {
                m_dynamicTasklist[j-1] = m_dynamicTasklist[j];
            }
            m_nbr_dynamic_tasks--;
            return;
        }
    }
}

int TimeSlot::get_pid_seq()
{
    return m_pid_sequence++;
}

TimeSlotChildIF::TimeSlotChildIF()
{
    m_skip_ticks = 0;
    m_pid = TimeSlot::get_pid_seq();
}

TimeSlotChildIF::~TimeSlotChildIF()
{
}

void TimeSlotChildIF::setup()
{
}

int TimeSlotChildIF::tick()
{
}

TimeSlotChildIF *TimeSlotChildIF::set_skip_ticks(int skip_ticks)
{
    this->m_skip_ticks = skip_ticks;
    return this;
}

TimeSlotButton::TimeSlotButton(int pin, unsigned char init_state, unsigned char init_toggle_state, unsigned char state_delay)
{
    m_pin = pin;
    m_state = init_state;
    m_toggle_state = init_toggle_state;
    m_state_delay = state_delay;
}

void TimeSlotButton::setup()
{
    pinMode(m_pin, INPUT);
}

int TimeSlotButton::tick()
{
    int present_state = digitalRead(m_pin);

    if (present_state != m_state)
    {
        if (HIGH == present_state)
        {
            m_state = 1;
            m_toggle_state ^= 1;
            on();
        }
        else
        {
            m_state = 0;
            off();
        }

        onoff();

        return m_state_delay;
    }
    return 0;
}

TimeSlotSequence::TimeSlotSequence(int capacity)
{
    m_sleep_child = new TimeSlotSequenceChild *[capacity];
    m_nbr_sleepers = 0;
    m_at_sleeper = 0;
}

void TimeSlotSequence::setup()
{
}

int TimeSlotSequence::tick()
{
    if (0 == m_nbr_sleepers)
    {
        return 1000;
    }

    m_sleep_child[m_at_sleeper]->exec();
    int retTicks = m_sleep_child[m_at_sleeper]->m_state_delay;

    m_at_sleeper++;
    if (m_at_sleeper >= m_nbr_sleepers)
    {
        m_at_sleeper = 0;
    }

    return retTicks;
}

TimeSlotSequence *TimeSlotSequence::add_child(TimeSlotSequenceChild *child)
{
    m_sleep_child[m_nbr_sleepers++] = child;
    return this;
}

TimeSlotSequenceChild::TimeSlotSequenceChild(int state_delay)
{
    m_state_delay = state_delay;
}
