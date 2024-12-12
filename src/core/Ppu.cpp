#include "IoRegisters.h"
#include "DebuggerInterface.h"
#include "Memory.h"
#include "Ppu.h"


Ppu::Ppu(Memory *memory, DebuggerInterface *debuggerInterface) :
    memory(memory),
    debuggerInterface(debuggerInterface),
    regINIDISP(memory->AttachIoRegister(eRegINIDISP, this)),
    regOBSEL(memory->AttachIoRegister(eRegOBSEL, this)),
    regOAMADDL(memory->AttachIoRegister(eRegOAMADDL, this)),
    regOAMADDH(memory->AttachIoRegister(eRegOAMADDH, this)),
    regOAMDATA(memory->AttachIoRegister(eRegOAMDATA, this)),
    regBGMODE(memory->AttachIoRegister(eRegBGMODE, this)),
    regMOSAIC(memory->AttachIoRegister(eRegMOSAIC, this)),
    regBG1SC(memory->AttachIoRegister(eRegBG1SC, this)),
    regBG2SC(memory->AttachIoRegister(eRegBG2SC, this)),
    regBG3SC(memory->AttachIoRegister(eRegBG3SC, this)),
    regBG4SC(memory->AttachIoRegister(eRegBG4SC, this)),
    regBG12NBA(memory->AttachIoRegister(eRegBG12NBA, this)),
    regBG34NBA(memory->AttachIoRegister(eRegBG34NBA, this)),
    regBG1HOFS(memory->AttachIoRegister(eRegBG1HOFS, this)),
    regBG1VOFS(memory->AttachIoRegister(eRegBG1VOFS, this)),
    regBG2HOFS(memory->AttachIoRegister(eRegBG2HOFS, this)),
    regBG2VOFS(memory->AttachIoRegister(eRegBG2VOFS, this)),
    regBG3HOFS(memory->AttachIoRegister(eRegBG3HOFS, this)),
    regBG3VOFS(memory->AttachIoRegister(eRegBG3VOFS, this)),
    regBG4HOFS(memory->AttachIoRegister(eRegBG4HOFS, this)),
    regBG4VOFS(memory->AttachIoRegister(eRegBG4VOFS, this)),
    regVMAIN(memory->AttachIoRegister(eRegVMAIN, this)),
    regVMADDL(memory->AttachIoRegister(eRegVMADDL, this)),
    regVMADDH(memory->AttachIoRegister(eRegVMADDH, this)),
    regVMDATAL(memory->AttachIoRegister(eRegVMDATAL, this)),
    regVMDATAH(memory->AttachIoRegister(eRegVMDATAH, this)),
    regM7SEL(memory->AttachIoRegister(eRegM7SEL, this)),
    regM7A(memory->AttachIoRegister(eRegM7A, this)),
    regM7B(memory->AttachIoRegister(eRegM7B, this)),
    regM7C(memory->AttachIoRegister(eRegM7C, this)),
    regM7D(memory->AttachIoRegister(eRegM7D, this)),
    regM7X(memory->AttachIoRegister(eRegM7X, this)),
    regM7Y(memory->AttachIoRegister(eRegM7Y, this)),
    regCGADD(memory->AttachIoRegister(eRegCGADD, this)),
    regCGDATA(memory->AttachIoRegister(eRegCGDATA, this)),
    regW12SEL(memory->AttachIoRegister(eRegW12SEL, this)),
    regW34SEL(memory->AttachIoRegister(eRegW34SEL, this)),
    regWOBJSEL(memory->AttachIoRegister(eRegWOBJSEL, this)),
    regWH0(memory->AttachIoRegister(eRegWH0, this)),
    regWH1(memory->AttachIoRegister(eRegWH1, this)),
    regWH2(memory->AttachIoRegister(eRegWH2, this)),
    regWH3(memory->AttachIoRegister(eRegWH3, this)),
    regWBGLOG(memory->AttachIoRegister(eRegWBGLOG, this)),
    regWOBJLOG(memory->AttachIoRegister(eRegWOBJLOG, this)),
    regTM(memory->AttachIoRegister(eRegTM, this)),
    regTS(memory->AttachIoRegister(eRegTS, this)),
    regTMW(memory->AttachIoRegister(eRegTMW, this)),
    regTSW(memory->AttachIoRegister(eRegTSW, this)),
    regCGWSEL(memory->AttachIoRegister(eRegCGWSEL, this)),
    regCGADSUB(memory->AttachIoRegister(eRegCGADSUB, this)),
    regCOLDATA(memory->AttachIoRegister(eRegCOLDATA, this)),
    regSETINI(memory->AttachIoRegister(eRegSETINI, this)),
    regMPYL(memory->AttachIoRegister(eRegMPYL, this)),
    regMPYM(memory->AttachIoRegister(eRegMPYM, this)),
    regMPYH(memory->AttachIoRegister(eRegMPYH, this)),
    regSLHV(memory->AttachIoRegister(eRegSLHV, this)),
    regRDOAM(memory->AttachIoRegister(eRegRDOAM, this)),
    regRDVRAML(memory->AttachIoRegister(eRegRDVRAML, this)),
    regRDVRAMH(memory->AttachIoRegister(eRegRDVRAMH, this)),
    regRDCGRAM(memory->AttachIoRegister(eRegRDCGRAM, this)),
    regOPHCT(memory->AttachIoRegister(eRegOPHCT, this)),
    regOPVCT(memory->AttachIoRegister(eRegOPVCT, this)),
    regSTAT77(memory->AttachIoRegister(eRegSTAT77, this)),
    regSTAT78(memory->AttachIoRegister(eRegSTAT78, this))
{

}


uint8_t Ppu::ReadRegister(EIORegisters ioReg) const
{
    LogInstruction("Ppu::ReadRegister %04X", ioReg);

    switch (ioReg)
    {
        case eRegINIDISP:
            return *regINIDISP;
        case eRegOBSEL:
            return *regOBSEL;
        case eRegOAMADDL:
            return *regOAMADDL;
        case eRegOAMADDH:
            return *regOAMADDH;
        case eRegOAMDATA:
            return *regOAMDATA;
        case eRegBGMODE:
            return *regBGMODE;
        case eRegMOSAIC:
            return *regMOSAIC;
        case eRegBG1SC:
            return *regBG1SC;
        case eRegBG2SC:
            return *regBG2SC;
        case eRegBG3SC:
            return *regBG3SC;
        case eRegBG4SC:
            return *regBG4SC;
        case eRegBG12NBA:
            return *regBG12NBA;
        case eRegBG34NBA:
            return *regBG34NBA;
        case eRegBG1HOFS:
            return *regBG1HOFS;
        case eRegBG1VOFS:
            return *regBG1VOFS;
        case eRegBG2HOFS:
            return *regBG2HOFS;
        case eRegBG2VOFS:
            return *regBG2VOFS;
        case eRegBG3HOFS:
            return *regBG3HOFS;
        case eRegBG3VOFS:
            return *regBG3VOFS;
        case eRegBG4HOFS:
            return *regBG4HOFS;
        case eRegBG4VOFS:
            return *regBG4VOFS;
        case eRegVMAIN:
            return *regVMAIN;
        case eRegVMADDL:
            return *regVMADDL;
        case eRegVMADDH:
            return *regVMADDH;
        case eRegVMDATAL:
            return *regVMDATAL;
        case eRegVMDATAH:
            return *regVMDATAH;
        case eRegM7SEL:
            return *regM7SEL;
        case eRegM7A:
            return *regM7A;
        case eRegM7B:
            return *regM7B;
        case eRegM7C:
            return *regM7C;
        case eRegM7D:
            return *regM7D;
        case eRegM7X:
            return *regM7X;
        case eRegM7Y:
            return *regM7Y;
        case eRegCGADD:
            return *regCGADD;
        case eRegCGDATA:
            return *regCGDATA;
        case eRegW12SEL:
            return *regW12SEL;
        case eRegW34SEL:
            return *regW34SEL;
        case eRegWOBJSEL:
            return *regWOBJSEL;
        case eRegWH0:
            return *regWH0;
        case eRegWH1:
            return *regWH1;
        case eRegWH2:
            return *regWH2;
        case eRegWH3:
            return *regWH3;
        case eRegWBGLOG:
            return *regWBGLOG;
        case eRegWOBJLOG:
            return *regWOBJLOG;
        case eRegTM:
            return *regTM;
        case eRegTS:
            return *regTS;
        case eRegTMW:
            return *regTMW;
        case eRegTSW:
            return *regTSW;
        case eRegCGWSEL:
            return *regCGWSEL;
        case eRegCGADSUB:
            return *regCGADSUB;
        case eRegCOLDATA:
            return *regCOLDATA;
        case eRegSETINI:
            return *regSETINI;
        case eRegMPYL:
            return *regMPYL;
        case eRegMPYM:
            return *regMPYM;
        case eRegMPYH:
            return *regMPYH;
        case eRegSLHV:
            return *regSLHV;
        case eRegRDOAM:
            return *regRDOAM;
        case eRegRDVRAML:
            return *regRDVRAML;
        case eRegRDVRAMH:
            return *regRDVRAMH;
        case eRegRDCGRAM:
            return *regRDCGRAM;
        case eRegOPHCT:
            return *regOPHCT;
        case eRegOPVCT:
            return *regOPVCT;
        case eRegSTAT77:
            return *regSTAT77;
        case eRegSTAT78:
            return *regSTAT78;
        default:
            throw std::range_error(fmt("Ppu doesnt handle reads to 0x%04X", ioReg));
    }
    return 0;
}


bool Ppu::WriteRegister(EIORegisters ioReg, uint8_t byte)
{
    LogInstruction("Ppu::WriteRegister %04X, %02X", ioReg, byte);

    switch (ioReg)
    {
        case eRegINIDISP:
            *regINIDISP = byte;
            return true;
        case eRegOBSEL:
            *regOBSEL = byte;
            return true;
        case eRegOAMADDL:
            *regOAMADDL = byte;
            return true;
        case eRegOAMADDH:
            *regOAMADDH = byte;
            return true;
        case eRegOAMDATA:
            *regOAMDATA = byte;
            return true;
        case eRegBGMODE:
            *regBGMODE = byte;
            return true;
        case eRegMOSAIC:
            *regMOSAIC = byte;
            return true;
        case eRegBG1SC:
            *regBG1SC = byte;
            return true;
        case eRegBG2SC:
            *regBG2SC = byte;
            return true;
        case eRegBG3SC:
            *regBG3SC = byte;
            return true;
        case eRegBG4SC:
            *regBG4SC = byte;
            return true;
        case eRegBG12NBA:
            *regBG12NBA = byte;
            return true;
        case eRegBG34NBA:
            *regBG34NBA = byte;
            return true;
        case eRegBG1HOFS:
            *regBG1HOFS = byte;
            return true;
        case eRegBG1VOFS:
            *regBG1VOFS = byte;
            return true;
        case eRegBG2HOFS:
            *regBG2HOFS = byte;
            return true;
        case eRegBG2VOFS:
            *regBG2VOFS = byte;
            return true;
        case eRegBG3HOFS:
            *regBG3HOFS = byte;
            return true;
        case eRegBG3VOFS:
            *regBG3VOFS = byte;
            return true;
        case eRegBG4HOFS:
            *regBG4HOFS = byte;
            return true;
        case eRegBG4VOFS:
            *regBG4VOFS = byte;
            return true;
        case eRegVMAIN:
            *regVMAIN = byte;
            return true;
        case eRegVMADDL:
            *regVMADDL = byte;
            return true;
        case eRegVMADDH:
            *regVMADDH = byte;
            return true;
        case eRegVMDATAL:
            *regVMDATAL = byte;
            return true;
        case eRegVMDATAH:
            *regVMDATAH = byte;
            return true;
        case eRegM7SEL:
            *regM7SEL = byte;
            return true;
        case eRegM7A:
            *regM7A = byte;
            return true;
        case eRegM7B:
            *regM7B = byte;
            return true;
        case eRegM7C:
            *regM7C = byte;
            return true;
        case eRegM7D:
            *regM7D = byte;
            return true;
        case eRegM7X:
            *regM7X = byte;
            return true;
        case eRegM7Y:
            *regM7Y = byte;
            return true;
        case eRegCGADD:
            *regCGADD = byte;
            return true;
        case eRegCGDATA:
            *regCGDATA = byte;
            return true;
        case eRegW12SEL:
            *regW12SEL = byte;
            return true;
        case eRegW34SEL:
            *regW34SEL = byte;
            return true;
        case eRegWOBJSEL:
            *regWOBJSEL = byte;
            return true;
        case eRegWH0:
            *regWH0 = byte;
            return true;
        case eRegWH1:
            *regWH1 = byte;
            return true;
        case eRegWH2:
            *regWH2 = byte;
            return true;
        case eRegWH3:
            *regWH3 = byte;
            return true;
        case eRegWBGLOG:
            *regWBGLOG = byte;
            return true;
        case eRegWOBJLOG:
            *regWOBJLOG = byte;
            return true;
        case eRegTM:
            *regTM = byte;
            return true;
        case eRegTS:
            *regTS = byte;
            return true;
        case eRegTMW:
            *regTMW = byte;
            return true;
        case eRegTSW:
            *regTSW = byte;
            return true;
        case eRegCGWSEL:
            *regCGWSEL = byte;
            return true;
        case eRegCGADSUB:
            *regCGADSUB = byte;
            return true;
        case eRegCOLDATA:
            *regCOLDATA = byte;
            return true;
        case eRegSETINI:
            *regSETINI = byte;
            return true;
        case eRegMPYL:
            *regMPYL = byte;
            return true;
        case eRegMPYM:
            *regMPYM = byte;
            return true;
        case eRegMPYH:
            *regMPYH = byte;
            return true;
        case eRegSLHV:
            *regSLHV = byte;
            return true;
        case eRegRDOAM:
            *regRDOAM = byte;
            return true;
        case eRegRDVRAML:
            *regRDVRAML = byte;
            return true;
        case eRegRDVRAMH:
            *regRDVRAMH = byte;
            return true;
        case eRegRDCGRAM:
            *regRDCGRAM = byte;
            return true;
        case eRegOPHCT:
            *regOPHCT = byte;
            return true;
        case eRegOPVCT:
            *regOPVCT = byte;
            return true;
        case eRegSTAT77:
            *regSTAT77 = byte;
            return true;
        case eRegSTAT78:
            *regSTAT78 = byte;
            return true;
        default:
            return false;
    }
    return false;
}