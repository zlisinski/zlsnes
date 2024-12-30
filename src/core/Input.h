#pragma once

#include "Zlsnes.h"
#include "Buttons.h"
#include "IoRegisterProxy.h"
#include "TimerObserver.h"

class Memory;

class Input : public IoRegisterProxy, public VBlankObserver
{
public:
    Input(Memory *memory, TimerSubject *timerSubject);
    virtual ~Input();

    void SetButtons(const Buttons &buttons);

    bool SaveState(FILE *file);
    bool LoadState(uint16_t version, FILE *file);

    // Inherited from IoRegisterProxy.
    bool WriteRegister(EIORegisters ioReg, uint8_t byte) override;
    uint8_t ReadRegister(EIORegisters ioReg) const override;

protected:
    // Inherited from VBlankObserver.
    void ProcessVBlankStart() override;
    void ProcessVBlankEnd() override {}

private:
    Memory *memory;

    Buttons buttonData;

    //uint8_t &regJOYWR; // 0x4016, //  Joypad Output (W)
    uint8_t &regJOYA;  // 0x4016, //  Joypad Input Register A (R)
    uint8_t &regJOYB;  // 0x4017, //  Joypad Input Register B (R)

    uint8_t &regJOY1L; // 0x4218 Joypad 1 (gameport 1, pin 4) (lower 8bit)
    uint8_t &regJOY1H; // 0x4219 Joypad 1 (gameport 1, pin 4) (upper 8bit)
    uint8_t &regJOY2L; // 0x421A Joypad 2 (gameport 2, pin 4) (lower 8bit)
    uint8_t &regJOY2H; // 0x421B Joypad 2 (gameport 2, pin 4) (upper 8bit)
    uint8_t &regJOY3L; // 0x421C Joypad 3 (gameport 1, pin 5) (lower 8bit)
    uint8_t &regJOY3H; // 0x421D Joypad 3 (gameport 1, pin 5) (upper 8bit)
    uint8_t &regJOY4L; // 0x421E Joypad 4 (gameport 2, pin 5) (lower 8bit)
    uint8_t &regJOY4H; // 0x421F Joypad 4 (gameport 2, pin 5) (upper 8bit)
};