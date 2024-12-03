#pragma once

#include <vector>
#include "Zlsnes.h"


class Cartridge
{
public:
    enum ERomType
    {
        eLoROM = 0x00,
        eHiROM = 0x01,
        eLoROMSDD1 = 0x02,
        eLoROMSA1 = 0x03,
        eExHiROM = 0x05,
        //eHiRomSPC7110 = 0x0A
    };

    Cartridge();

    bool Validate(std::vector<uint8_t> &data);

    // Standard Header Info.
    char title[22] = {0};
    ERomType romType = ERomType::eLoROM;
    bool isLoRom = true;
    bool fastSpeed = false;
    uint8_t chipset = 0;
    uint8_t romSize = 0;
    uint8_t ramSize = 0;
    uint8_t country = 0;
    uint8_t devId = 0;
    uint8_t romVersion = 0;
    uint16_t checksumComplement = 0;
    uint16_t checksum = 0;

    // Extended Header Info.
    char makerCode[3] = {0};
    char gameCode[5] = {0};
    uint8_t expansionFlashSize = 0;
    uint8_t expansionRamSize = 0;
    uint8_t specialVersion = 0;
    uint8_t chipsetSubtype = 0;
};