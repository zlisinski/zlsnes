#include "Zlsnes.h"
#include "Input.h"
#include "Memory.h"
#include "Timer.h"


Input::Input(Memory *memory, Timer *timer) :
    memory(memory),
    timer(timer),
    buttonData(),
    regJOYA(memory->RequestOwnership(eRegJOYA, this)),
    regJOYB(memory->RequestOwnership(eRegJOYB, this)),
    regJOY1L(memory->RequestOwnership(eRegJOY1L, this)),
    regJOY1H(memory->RequestOwnership(eRegJOY1H, this)),
    regJOY2L(memory->RequestOwnership(eRegJOY2L, this)),
    regJOY2H(memory->RequestOwnership(eRegJOY2H, this)),
    regJOY3L(memory->RequestOwnership(eRegJOY3L, this)),
    regJOY3H(memory->RequestOwnership(eRegJOY3H, this)),
    regJOY4L(memory->RequestOwnership(eRegJOY4L, this)),
    regJOY4H(memory->RequestOwnership(eRegJOY4H, this))
{
    // Start out with all buttons released.
    regJOYA = 0;
    regJOYB = 0;
    regJOY1L = 0;
    regJOY1H = 0;
    regJOY2L = 0;
    regJOY2H = 0;
    regJOY3L = 0;
    regJOY3H = 0;
    regJOY4L = 0;
    regJOY4H = 0;

    timer->AttachVBlankObserver(this);
}


Input::~Input()
{

}


void Input::SetButtons(const Buttons &buttons)
{
    LogInput("SetButtons: %04X", buttons.data);
    buttonData = buttons;
}


uint8_t Input::ReadRegister(EIORegisters ioReg)
{
    LogInput("Input::ReadByte %04X", ioReg);

    switch (ioReg)
    {
        case eRegJOYA:
            // eClockIoReg will be added in the Memory class, so only add the difference for this slow register.
            timer->AddCycle(EClockSpeed::eClockOther - EClockSpeed::eClockIoReg);
            return (regJOYA & 0x03) | (memory->GetOpenBusValue() & 0xFC);
        case eRegJOYB:
            // eClockIoReg will be added in the Memory class, so only add the difference for this slow register.
            timer->AddCycle(EClockSpeed::eClockOther - EClockSpeed::eClockIoReg);
            return (regJOYB & 0x03) | 0x1C | (memory->GetOpenBusValue() & 0xE0);
        case eRegJOY1L:
            return regJOY1L;
        case eRegJOY1H:
            return regJOY1H;
        case eRegJOY2L:
            return regJOY2L;
        case eRegJOY2H:
            return regJOY2H;
        case eRegJOY3L:
            return regJOY3L;
        case eRegJOY3H:
            return regJOY3H;
        case eRegJOY4L:
            return regJOY4L;
        case eRegJOY4H:
            return regJOY4H;
        default:
            throw std::range_error(fmt("Input doesnt handle reads to 0x%04X", ioReg));
    }
}


bool Input::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
   LogInput("Input::WriteByte %04X, %02X", ioReg, byte);

    switch (ioReg)
    {
        case eRegJOYWR:
            // eClockIoReg will be added in the Memory class, so only add the difference for this slow register.
            timer->AddCycle(EClockSpeed::eClockOther - EClockSpeed::eClockIoReg);
            return true;
        // The rest are all read-only.
        default:
            return false;
    }
}


void Input::ProcessVBlankStart()
{
    bool autoReadFlag = Bytes::GetBit<0>(memory->ReadRaw8Bit(eRegHVBJOY));

    if (autoReadFlag)
    {
        regJOY1L = Bytes::GetByte<0>(buttonData.data);
        regJOY1H = Bytes::GetByte<1>(buttonData.data);
    }
}


/*bool Input::SaveState(FILE *file)
{
    if (!fwrite(&buttonData.data, sizeof(buttonData.data), 1, file))
        return false;

    return true;
}


bool Input::LoadState(uint16_t version, FILE *file)
{
    (void)version;

    if (!fread(&buttonData.data, sizeof(buttonData.data), 1, file))
        return false;

    return true;
}*/