#pragma once

#include "Zlsnes.h"


class TimerObserver
{
public:
    virtual void UpdateTimer(uint32_t value) = 0;

protected:
    ~TimerObserver() {}
};


// std::vector was a significant slowdown. Using a static array of pointers speeds this up significantly,
// since NotifyObservers() is called at least once per opcode.
class TimerSubject
{
public:
    TimerSubject() : timerObserverCount(0)
    {
        for (int i = 0; i < timerObserversMax; i++)
            timerObservers[i] = NULL;
    }

    void AttachObserver(TimerObserver *observer)
    {
        timerObservers[timerObserverCount++] = observer;
    }

    // Don't bother with detaching, since everything is destroyed at the same time.
    // There is no case where one observer will be destroyed and the subject won't.
    /*void DetachObserver(TimerObserver *observer)
    {
        (void)observer; // Stop warnings about unused variables.
    }*/

    void NotifyObservers(uint32_t value)
    {
        for (int i = 0; i < timerObserverCount; i++)
        {
            if (timerObservers[i])
            {
                timerObservers[i]->UpdateTimer(value);
            }
        }
    }

protected:
    ~TimerSubject() {}

private:
    static const int timerObserversMax = 4;
    int timerObserverCount;
    TimerObserver *timerObservers[timerObserversMax];
};