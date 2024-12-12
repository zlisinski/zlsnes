#pragma once

#include "Zlsnes.h"
#include "Address.h"

class Cpu;
class Memory;
class Ppu;

class DebuggerInterface
{
public:
    virtual void SetEmulatorObjects(Memory *newMemory, Cpu *newCpu, Ppu *newPpu) = 0;

    virtual bool GetDebuggingEnabled() = 0;
    virtual bool ShouldRun(Address pc) = 0;
    virtual void SetCurrentOp(Address pc) = 0;

    virtual void MemoryChanged(Address address, uint16_t len) = 0;
};