#pragma once

#include <mutex>
#include <thread>
#include <vector>
//#include "Buttons.h"

//class Audio;
//class AudioInterface;
class Cpu;
//class DebuggerInterface;
//class Display;
class DisplayInterface;
//class GameSpeedSubject;
class InfoInterface;
//class Input;
//class Interrupt;
class Memory;
//class Serial;
//class Timer;

class Emulator
{
public:
    Emulator(DisplayInterface *displayInterface, /*AudioInterface *audioInterface,*/ InfoInterface *infoInterface/*,
             DebuggerInterface *debuggerInterface, GameSpeedSubject *gameSpeedSubject*/);
    ~Emulator();

    bool LoadRom(const std::string &filename);
    void ResetEmulation();
    void PauseEmulation(bool pause);
    void EndEmulation();
    //void ButtonPressed(Buttons::Button button);
    //void ButtonReleased(Buttons::Button button);

    void SaveState(int slot);
    void LoadState(int slot);

private:
    void ThreadFunc();

    void SetBootState(Memory *memory, Cpu *cpu);

    bool paused;
    bool quit;

    std::vector<uint8_t> gameRomMemory;

    std::string romFilename;
    std::string ramFilename;

    std::thread workThread;
    std::mutex saveStateMutex;

    DisplayInterface *displayInterface;
    //AudioInterface *audioInterface;
    InfoInterface *infoInterface;
    /*DebuggerInterface *debuggerInterface;
    GameSpeedSubject *gameSpeedSubject;*/

    //Audio *audio;
    //Buttons buttons;
    Cpu *cpu;
    //Display *display;
    //Input *input;
    //Interrupt *interrupts;
    Memory *memory;
    //Serial *serial;
    //Timer *timer;
};