#pragma once

#include <mutex>
#include <thread>
#include <vector>
#include "Buttons.h"
#include "Cartridge.h"

class Apu;
//class AudioInterface;
class Cpu;
class DebuggerInterface;
class DisplayInterface;
//class GameSpeedSubject;
class InfoInterface;
class Input;
class Interrupt;
class Memory;
class Ppu;
class Timer;

class Emulator
{
public:
    Emulator(DisplayInterface *displayInterface, /*AudioInterface *audioInterface,*/ InfoInterface *infoInterface,
             DebuggerInterface *debuggerInterface/*, GameSpeedSubject *gameSpeedSubject*/);
    ~Emulator();

    bool LoadRom(const std::string &filename);
    void ResetEmulation();
    void PauseEmulation(bool pause);
    void EndEmulation();
    void ButtonPressed(Buttons::Button button);
    void ButtonReleased(Buttons::Button button);
    void ToggleLayer(int layer, bool enabled);

    void SaveState(int slot);
    void LoadState(int slot);

private:
    void ThreadFunc();

    bool paused;
    bool quit;

    std::string romFilename;

    std::thread workThread;
    std::mutex saveStateMutex;

    DisplayInterface *displayInterface;
    //AudioInterface *audioInterface;
    InfoInterface *infoInterface;
    DebuggerInterface *debuggerInterface;
    //GameSpeedSubject *gameSpeedSubject;

    Apu *apu;
    Buttons buttons;
    Cartridge cartridge;
    Cpu *cpu;
    Input *input;
    Interrupt *interrupts;
    Memory *memory;
    Ppu *ppu;
    Timer *timer;

    bool enabledLayers[5];
};