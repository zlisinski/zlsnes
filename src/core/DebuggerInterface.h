#pragma once

#include <atomic>

#include "Zlsnes.h"
#include "Address.h"

class Cpu;
class Memory;
class Ppu;

class DebuggerInterface
{
public:
    explicit DebuggerInterface(bool debuggingEnabled = false) : debuggingEnabled(debuggingEnabled) {}

    virtual void SetEmulatorObjects(Memory *newMemory, Cpu *newCpu, Ppu *newPpu) = 0;

    inline bool GetDebuggingEnabled() {return debuggingEnabled;}
    virtual bool ShouldRun(Address pc) = 0;
    virtual void SetCurrentOp(Address pc) = 0;

    virtual void MemoryChanged(Address address, uint16_t len) = 0;

protected:
    std::atomic<bool> debuggingEnabled;
};