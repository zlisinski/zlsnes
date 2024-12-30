#pragma once

#include "Zlsnes.h"


class TimerObserver
{
protected:
    ~TimerObserver() {}
    virtual void ProcessTimerTick(uint32_t value) = 0;
    friend class TimerSubject;
};


class HBlankObserver
{
protected:
    ~HBlankObserver() {}
    virtual void ProcessHBlankStart(uint32_t scanline) = 0;
    virtual void ProcessHBlankEnd(uint32_t scanline) = 0;
    friend class TimerSubject;
};


class VBlankObserver
{
protected:
    ~VBlankObserver() {}
    virtual void ProcessVBlankStart() = 0;
    virtual void ProcessVBlankEnd() = 0;
    friend class TimerSubject;
};


// std::vector was a significant slowdown. Using a static array of pointers speeds this up significantly,
// since NotifyObservers() is called at least once per opcode.
// Don't bother with detaching, since everything is destroyed at the same time.
// There is no case where one observer will be destroyed and the subject won't.
class TimerSubject
{
public:
    TimerSubject() :
        timerObserverCount(0),
        timerObservers{nullptr},
        hBlankObserverCount(0),
        hBlankObservers{nullptr},
        vBlankObserverCount(0),
        vBlankObservers{nullptr}
    {

    }

    void AttachTimerObserver(TimerObserver *observer)
    {
        if (timerObserverCount == timerObserversMax)
            throw std::range_error("Too many timer observers");
        if (observer == nullptr)
            throw std::logic_error("observer == null");
        timerObservers[timerObserverCount++] = observer;
    }

    void AttachHBlankObserver(HBlankObserver *observer)
    {
        if (hBlankObserverCount == timerObserversMax)
            throw std::range_error("Too many timer observers");
        if (observer == nullptr)
            throw std::logic_error("observer == null");
        hBlankObservers[hBlankObserverCount++] = observer;
    }

    void AttachVBlankObserver(VBlankObserver *observer)
    {
        if (vBlankObserverCount == timerObserversMax)
            throw std::range_error("Too many timer observers");
        if (observer == nullptr)
            throw std::logic_error("observer == null");
        vBlankObservers[vBlankObserverCount++] = observer;
    }

protected:
    ~TimerSubject() {}

    void NotifyTimerObservers(uint32_t value)
    {
        for (int i = 0; i < timerObserverCount; i++)
            timerObservers[i]->ProcessTimerTick(value);
    }

    void NotifyHBlankStartObservers(uint32_t scanline)
    {
        for (int i = 0; i < hBlankObserverCount; i++)
            hBlankObservers[i]->ProcessHBlankStart(scanline);
    }

    void NotifyHBlankEndObservers(uint32_t scanline)
    {
        for (int i = 0; i < hBlankObserverCount; i++)
            hBlankObservers[i]->ProcessHBlankEnd(scanline);
    }

    void NotifyVBlankStartObservers()
    {
        for (int i = 0; i < vBlankObserverCount; i++)
            vBlankObservers[i]->ProcessVBlankStart();
    }

    void NotifyVBlankEndObservers()
    {
        for (int i = 0; i < vBlankObserverCount; i++)
            vBlankObservers[i]->ProcessVBlankEnd();
    }

private:
    static const int timerObserversMax = 4;

    int timerObserverCount;
    TimerObserver *timerObservers[timerObserversMax];

    int hBlankObserverCount;
    HBlankObserver *hBlankObservers[timerObserversMax];

    int vBlankObserverCount;
    VBlankObserver *vBlankObservers[timerObserversMax];
};