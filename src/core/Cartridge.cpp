#include <unordered_set>

#include "Bytes.h"
#include "InfoInterface.h"
#include "Cartridge.h"

static const size_t LOROM_HEADER_OFFSET = 0x7FC0;
static const size_t HIROM_HEADER_OFFSET = 0xFFC0;
static const size_t EXHIROM_HEADER_OFFSET = 0x40FFC0;

static const size_t MODE_OFFSET = 0x15;
static const size_t CHIPSET_OFFSET = 0x16;
static const size_t ROM_SIZE_OFFSET = 0x17;
static const size_t RAM_SIZE_OFFSET = 0x18;
static const size_t COUNTRY_OFFSET = 0x19;
static const size_t DEVID_OFFSET = 0x1A;
static const size_t ROM_VERSION_OFFSET = 0x1B;
static const size_t CHECKSUM_OFFSET = 0x1C;
static const ssize_t MAKERID_OFFSET = -0x10;
static const ssize_t GAMECODEID_OFFSET = -0x0E;
static const ssize_t EXP_FLASH_SIZE_OFFSET = -0x04;
static const ssize_t EXP_RAM_SIZE_OFFSET = -0x03;
static const ssize_t SPECIAL_VERSION_OFFSET = -0x02;
static const ssize_t CHIPSET_SUBTYPE_OFFSET = -0x01;


static const std::unordered_set<uint8_t> romModeLookup = {
    Cartridge::ERomType::eLoROM,
    Cartridge::ERomType::eLoROMSDD1,
    Cartridge::ERomType::eLoROMSA1,
    Cartridge::ERomType::eHiROM,
    Cartridge::ERomType::eExHiROM
};


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

    // TODO: Replace with actual minimal ROM size. This value will allow checking for both LoROM and HiROM headers.
    if (data.size() < 0xFFFF)
    {
        LogError("File is too small");
        return false;
    }

    size_t offset = 0;

    if (FindHeader(data, LOROM_HEADER_OFFSET))
    {
        offset = LOROM_HEADER_OFFSET;
    }
    else if (FindHeader(data, HIROM_HEADER_OFFSET))
    {
        offset = HIROM_HEADER_OFFSET;
    }
    else
    {
        //Skip checking ExHiROM, since it's only 2 Japanese games.
        LogError("Unable to determine cartridge type");
        return false;
    }

    std::copy(data.begin() + offset, data.begin() + offset + 21, title);
    title[21] = 0;
    LogInfo("Title = %s", title);

    romType = static_cast<ERomType>(data[offset + MODE_OFFSET] & 0x0F);
    isLoRom = (romType == ERomType::eLoROM || romType == ERomType::eLoROMSA1 || romType == ERomType::eLoROMSDD1);
    isInterleaved = (offset == LOROM_HEADER_OFFSET) && !isLoRom; // Interleaved ROMs can be hiROM, but have the header at the LoROM file offset.
    if (isInterleaved)
    {
        throw NotYetImplementedException("Iterleaved ROM NYI");
    }
    fastSpeed = (data[offset + MODE_OFFSET] >> 4) & 1;
    chipset = data[offset + CHIPSET_OFFSET];
    romSize = data[offset + ROM_SIZE_OFFSET];
    ramSize = data[offset + RAM_SIZE_OFFSET];
    country = data[offset + COUNTRY_OFFSET];
    devId = data[offset + DEVID_OFFSET];
    romVersion = data[offset + ROM_VERSION_OFFSET];

    if (devId == 0x33)
    {
        std::copy(data.begin() + offset - 0x10, data.begin() + offset - 0x0E, makerCode);
        makerCode[2] = 0;

        std::copy(data.begin() + offset - 0x0E, data.begin() + offset - 0x0A, gameCode);
        gameCode[4] = 0;

        expansionFlashSize = data[offset + EXP_FLASH_SIZE_OFFSET];
        expansionRamSize = data[offset + EXP_RAM_SIZE_OFFSET];
        specialVersion = data[offset + SPECIAL_VERSION_OFFSET];
        chipsetSubtype = data[offset + CHIPSET_SUBTYPE_OFFSET];
    }

    return true;
}


bool Cartridge::FindHeader(const std::vector<uint8_t> &data, size_t headerOffset)
{
    // Check that title field is all ASCII. The last byte can be null, so skip checking it.
    for (int i = 0; i < 20; i++)
    {
        if (data[headerOffset + i] < 0x20 || data[headerOffset + i] > 0x7E)
        {
            LogError("Non-ASCII Title at %04X %d %02X", headerOffset, i, data[headerOffset + i]);
            return false;
        }
    }

    // Look for a mode-like byte. This succeeds at the wrong offset with FF5 which is why the rest of the checks are necesasry.
    if (romModeLookup.find(data[headerOffset + MODE_OFFSET] & 0x0F) == romModeLookup.end())
    {
        LogError("Bad mode at %02X %04X", data[headerOffset + MODE_OFFSET], headerOffset + MODE_OFFSET);
        return false;
    }

    // Check checksums. This doesn't actually verify all ROM data in case this is a hacked ROM.
    checksumComplement = Bytes::Make16Bit(data[headerOffset + CHECKSUM_OFFSET + 1], data[headerOffset + CHECKSUM_OFFSET]);
    checksum = Bytes::Make16Bit(data[headerOffset + CHECKSUM_OFFSET + 3], data[headerOffset + CHECKSUM_OFFSET + 2]);
    if ((checksum ^ checksumComplement) != 0xFFFF)
    {
        LogError("Bad checksum at %04X", headerOffset);
        return false;
    }

    return true;
}