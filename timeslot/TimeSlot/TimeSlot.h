#ifndef __TIMESLOT_H_INCLUDED__
#define __TIMESLOT_H_INCLUDED__

// Marcus Engene myfirstname@mylastname.se; BSD license.

/**
 * @TODO have some way to change #ticks/s from 1000 to smth else.
 */

class TimeSlotChildIF;

class TimeSlot
{
protected:

    static unsigned int m_tickstart;

    /**
     * This array is for TimeSlotChildIF objects created dynamically.
     *
     * Max 10 entries.
     */
    static TimeSlotChildIF *m_dynamicTasklist[];

    /**
     * Keeps track of how many dynamically created TimeSlotChildIF there are.
     */
    static int m_nbr_dynamic_tasks;

    /**
     * Saves current micsosecond onto m_tickstart.
     */
    static void timerstart_tasklist();

    /**
     * Figures out for how long a timeslot ran and sleeps accordingly.
     */
    static void timerstop_tasklist();

    /**
     * Calls tick() on TimeSlotChildIF where it's time to do so.
     *
     * If tick returns > 0, then this is the number of ticks framework should wait
     * to call tick().
     *
     * This function also calls tick() on the dynamic created TimeSlotChildIF.
     *
     * @param tasklist Statucally created null terminated array of TimeSlotChildIF.
     */
    static void tick_tasklist(TimeSlotChildIF *tasklist[]);

public:

    /// If you want to have a pin high when processing, then set this to the port number.
    static int m_cpupin;

    /// When a TimeSlotChildIF is created, m_pid_sequence++ is used for m_pid.
    static int m_pid_sequence;

    /// By default the event loop is called every 10000us.
    static unsigned int m_us_delay;

    TimeSlot();

    /**
     * Calls setup() on all the TimeSlotChildIF entries.
     *
     * @param tasklist Statucally created null terminated array of TimeSlotChildIF.
     */
    static void setup(TimeSlotChildIF *tasklist[]);

    /**
     * Called from Arduino framework loop() function.
     *
     * @param tasklist Statucally created null terminated array of TimeSlotChildIF.
     */
    static void loop(TimeSlotChildIF *tasklist[]);

    /**
     * Returns a pid.
     */
    static int get_pid_seq();

    /**
     * Add dynamically created TimeSlotChildIF objects.
     *
     * Use the returned pid to kill the task.
     *
     * @param task
     * @return pid
     */
    static int add_dynamic_task(TimeSlotChildIF *task);

    /**
     * Kill a task using its pid.
     *
     * @param pid
     */
    static void kill_dynamic_task(int pid);
};

/**
 * This is the interface for the tasks.
 */
class TimeSlotChildIF
{
public:

    TimeSlotChildIF();

    virtual ~TimeSlotChildIF();

    /**
     * This is the return value of tick(). If >0, tick() is not called again for
     * that number of ticks.
     */
    int m_skip_ticks;

    /**
     * The pid is only used to kill dynamically created TimeSlotChildIF objects.
     */
    int m_pid;

    virtual void setup();

    /**
     * Return value is how many ticks should be skipped until this function is called the next time.
     * If unsure, do a return 0
     */
    virtual int tick();

    /**
     * If you have a bunch of tasks that will process every say 10th tick, they would
     * all hit at the same time default. To help spread it out, you can set the skipTick
     * in your tasklist creation.
     *
     * TimeSlotChildIF *tasklist[] = {
     *     (new sometask())->set_skip_ticks(1),
     *     (new sometask2())->set_skip_ticks(2)
     * };
     */
    TimeSlotChildIF *set_skip_ticks(int skip_ticks);
};

/**
 * Convenience class for button. It's not rocket science to write a button class, but its very
 * convenient to have the bounce logic and all that taken care of.
 */
class TimeSlotButton : public TimeSlotChildIF
{
public:

    int m_pin;

    unsigned char m_state;

    unsigned char m_toggle_state;

    /**
     * Nbr of ticks that should pass before we start the logic again.
     */
    unsigned char m_state_delay;

    TimeSlotButton(int port, unsigned char init_state=0, unsigned char init_toggle_state=0, unsigned char state_delay=3);

    void setup();

    virtual int tick();

    /**
     * Called when state goes on. If you want a toggle button, only use this and
     * check m_toggle_state.
     */
    virtual void on() {};

    /**
     * Called when state goes off
     */
    virtual void off() {};

    /**
     * Called whenever state changes
     */
    virtual void onoff() {};
};

class TimeSlotSequenceChild
{
public:

    int m_state_delay;

    TimeSlotSequenceChild(int state_delay);

    virtual void exec() = 0;
};

/**
 * If you want a sequence of things to happen.
 */
class TimeSlotSequence : public TimeSlotChildIF
{
public:

    TimeSlotSequenceChild **m_sleep_child;

    /**
     * Capacity is now many sleep segments it should be able to have
     */
    TimeSlotSequence(int capacity);

    TimeSlotSequence *add_child(TimeSlotSequenceChild *child);

    void setup();

    virtual int tick();

protected:

    int m_nbr_sleepers;

    int m_at_sleeper;

};

#endif // __TIMESLOT_H_INCLUDED__
