#include "Zlsnes.h"
#include "Input.h"
#include "Memory.h"


enum ButtonMask
{
    eMaskARight    = ~0x01, // 0xFE
    eMaskBLeft     = ~0x02, // 0xFD
    eMaskSelectUp  = ~0x04, // 0xFB
    eMaskStartDown = ~0x08, // 0xF7
    eMaskDirection = 0x10, // 0xEF
    eMaskButtons   = 0x20  // 0xDF
};


Input::Input(IoRegisterSubject *ioRegisterSubject/*, Interrupt *interrupts*/) :
    //regP1(ioRegisterSubject->AttachIoRegister(eRegP1, this)),
    //interrupts(interrupts),
    buttonData()
{
    // Start out with all buttons released.
    //*regP1 = 0xFF;
}


Input::~Input()
{

}


bool Input::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
   LogInstruction("Input::WriteByte %04X, %02X", ioReg, byte);

    switch (ioReg)
    {
        /*case eRegP1:
            UpdateRegP1(byte);
            return true;*/
        default:
            return false;
    }
}


uint8_t Input::ReadRegister(EIORegisters ioReg) const
{
    LogInstruction("Input::ReadByte %04X", ioReg);

    switch (ioReg)
    {
        /*case eRegP1:
            return *regP1;*/
        default:
            throw std::range_error(fmt("Input doesnt handle reads to 0x%04X", ioReg));
    }
}


void Input::SetButtons(const Buttons &buttons)
{
    buttonData = buttons;
    //UpdateRegP1(*regP1);
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