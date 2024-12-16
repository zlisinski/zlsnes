#include "Zlsnes.h"
#include "Input.h"
#include "Memory.h"


Input::Input(Memory *memory, TimerSubject *timerSubject/*, Interrupt *interrupts*/) :
    memory(memory),
    //interrupts(interrupts),
    buttonData(),
    lastAutoReadFlag(false),
    regJOY1L(memory->AttachIoRegister(eRegJOY1L, this)),
    regJOY1H(memory->AttachIoRegister(eRegJOY1H, this)),
    regJOY2L(memory->AttachIoRegister(eRegJOY2L, this)),
    regJOY2H(memory->AttachIoRegister(eRegJOY2H, this)),
    regJOY3L(memory->AttachIoRegister(eRegJOY3L, this)),
    regJOY3H(memory->AttachIoRegister(eRegJOY3H, this)),
    regJOY4L(memory->AttachIoRegister(eRegJOY4L, this)),
    regJOY4H(memory->AttachIoRegister(eRegJOY4H, this))
{
    // Start out with all buttons released.
    *regJOY1L = 0;
    *regJOY1H = 0;
    *regJOY2L = 0;
    *regJOY2H = 0;
    *regJOY3L = 0;
    *regJOY3H = 0;
    *regJOY4L = 0;
    *regJOY4H = 0;

    timerSubject->AttachObserver(this);
}


Input::~Input()
{

}


void Input::SetButtons(const Buttons &buttons)
{
    LogInput("SetButtons: %04X", buttons.data);
    buttonData = buttons;
}


bool Input::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
   LogInput("Input::WriteByte %04X, %02X", ioReg, byte);

    switch (ioReg)
    {
        // These are all read-only.
        default:
            return false;
    }
}


uint8_t Input::ReadRegister(EIORegisters ioReg) const
{
    LogInput("Input::ReadByte %04X", ioReg);

    switch (ioReg)
    {
        case eRegJOY1L:
            return *regJOY1L;
        case eRegJOY1H:
            return *regJOY1H;
        case eRegJOY2L:
            return *regJOY2L;
        case eRegJOY2H:
            return *regJOY2H;
        case eRegJOY3L:
            return *regJOY3L;
        case eRegJOY3H:
            return *regJOY3H;
        case eRegJOY4L:
            return *regJOY4L;
        case eRegJOY4H:
            return *regJOY4H;
        default:
            throw std::range_error(fmt("Input doesnt handle reads to 0x%04X", ioReg));
    }
}


void Input::UpdateTimer(uint32_t value)
{
    (void)value;

    bool autoReadFlag = Bytes::GetBit<0>(memory->ReadRaw8Bit(eRegHVBJOY));

    // Only update when it changes, we don't need to do this every cycle for multiple scanlines.
    // TODO: Vblank observer?
    if (autoReadFlag && !lastAutoReadFlag)
    {
        *regJOY1L = Bytes::GetByte<0>(buttonData.data);
        *regJOY1H = Bytes::GetByte<1>(buttonData.data);
    }

    lastAutoReadFlag = autoReadFlag;
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