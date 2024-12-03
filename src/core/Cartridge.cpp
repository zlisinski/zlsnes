#include "Bytes.h"
#include "InfoInterface.h"
#include "Cartridge.h"

static const size_t LOROM_HEADER_OFFSET = 0x7FC0;
static const size_t LOROM_TYPE_OFFSET = 0x7FD5;
static const size_t HIROM_HEADER_OFFSET = 0xFFC0;
static const size_t HIROM_TYPE_OFFSET = 0xFFD5;
static const size_t EXHIROM_HEADER_OFFSET = 0x40FFC0;
static const size_t EXHIROM_TYPE_OFFSET = 0x40FFD5;


Cartridge::Cartridge()
{

}


bool Cartridge::Validate(std::vector<uint8_t> &data)
{
    // Check for and strip useless header added by cartridge copying devices.
    size_t copierHeaderLen = data.size() % 1024;
    if (copierHeaderLen != 0)
    {
        LogInfo("Detected copier header of length %d", copierHeaderLen);
        data.erase(data.begin(), data.begin() + copierHeaderLen);
    }

    size_t offset = 0;

    if (data.size() > LOROM_TYPE_OFFSET &&
        ((data[LOROM_TYPE_OFFSET] & 0x0F) == ERomType::eLoROM ||
         (data[LOROM_TYPE_OFFSET] & 0x0F) == ERomType::eLoROMSDD1 ||
         (data[LOROM_TYPE_OFFSET] & 0x0F) == ERomType::eLoROMSA1))
    {
        LogInfo("ROM type = LoROM");
        romType = static_cast<ERomType>(data[LOROM_TYPE_OFFSET] & 0x0F);
        isLoRom = true;
        fastSpeed = (data[LOROM_TYPE_OFFSET] >> 4) & 1;
        offset = LOROM_HEADER_OFFSET;
    }
    else if (data.size() > HIROM_TYPE_OFFSET &&
             (data[HIROM_TYPE_OFFSET] & 0x0F) == ERomType::eHiROM)
    {
        LogInfo("ROM type = HiROM");
        romType = static_cast<ERomType>(data[HIROM_TYPE_OFFSET] & 0x0F);
        isLoRom = false;
        fastSpeed = (data[HIROM_TYPE_OFFSET] >> 4) & 1;
        offset = HIROM_HEADER_OFFSET;
    }
    else if (data.size() > EXHIROM_TYPE_OFFSET &&
             (data[EXHIROM_TYPE_OFFSET] & 0x0F) == ERomType::eExHiROM)
    {
        LogInfo("ROM type = ExHiROM");
        romType = ERomType::eExHiROM;
        isLoRom = false;
        fastSpeed = (data[EXHIROM_TYPE_OFFSET + copierHeaderLen] >> 4) & 1;
        offset = EXHIROM_HEADER_OFFSET;
        LogError("ExHiROM NYI");
        return false;
    }
    else
    {
        LogError("Unable to determine cartridge type");
        return false;
    }

    checksumComplement = Bytes::Make16Bit(data[offset + 0x1D], data[offset + 0x1C]);
    checksum = Bytes::Make16Bit(data[offset + 0x1F], data[offset + 0x1E]);
    if ((checksum ^ checksumComplement) != 0xFFFF)
    {
        LogError("Checksum and complement don't match %04X %04X", checksumComplement, checksum);
        return false;
    }

    std::copy(data.begin() + offset, data.begin() + offset + 21, title);
    title[21] = 0;
    LogInfo("Title = %s", title);

    chipset = data[offset + 0x16];
    romSize = data[offset + 0x17];
    ramSize = data[offset + 0x18];
    country = data[offset + 0x19];
    devId = data[offset + 0x1A];
    romVersion = data[offset + 0x1B];

    if (devId == 0x33)
    {
        std::copy(data.begin() + offset - 0x10, data.begin() + offset - 0x0E, makerCode);
        makerCode[2] = 0;

        std::copy(data.begin() + offset - 0x0E, data.begin() + offset - 0x0A, gameCode);
        gameCode[4] = 0;

        expansionFlashSize = data[offset - 4];
        expansionRamSize = data[offset - 3];
        specialVersion = data[offset - 2];
        chipsetSubtype = data[offset - 1];
    }

    return true;
}