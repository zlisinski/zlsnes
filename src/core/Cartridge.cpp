#include <fstream>
#include <unordered_set>

#include "Cartridge.h"

static const size_t LOROM_HEADER_OFFSET = 0x7FC0;
static const size_t HIROM_HEADER_OFFSET = 0xFFC0;
static const size_t EXHIROM_HEADER_OFFSET = 0x40FFC0;

static const ssize_t EXTENDED_HEADER_OFSET = -0x10;


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


bool Cartridge::LoadRom(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        LogError("Unable to open file %s", filename.c_str());
        return false;
    }
    std::istreambuf_iterator<char> start(file), end;
    rom = std::vector<uint8_t>(start, end);

    return Validate();
}


void Cartridge::Reset()
{
    rom.clear();
    isInterleaved = false;
    romType = ERomType::eLoROM;
    isLoRom = true;
    isFastSpeed = false;
    standardHeader = StandardHeader();
    extendedHeader = ExtendedHeader();
}


bool Cartridge::Validate()
{
    // Check for and strip useless header added by cartridge copying devices.
    size_t copierHeaderLen = rom.size() % 1024;
    if (copierHeaderLen != 0)
    {
        LogInfo("Detected copier header of length %d", copierHeaderLen);
        rom.erase(rom.begin(), rom.begin() + copierHeaderLen);
    }

    // TODO: Replace with actual minimal ROM size. This value will allow checking for both LoROM and HiROM headers.
    if (rom.size() < 0xFFFF)
    {
        LogError("File is too small");
        return false;
    }

    size_t offset = 0;

    if (FindHeader(LOROM_HEADER_OFFSET))
    {
        offset = LOROM_HEADER_OFFSET;
    }
    else if (FindHeader(HIROM_HEADER_OFFSET))
    {
        offset = HIROM_HEADER_OFFSET;
    }
    else
    {
        //Skip checking ExHiROM, since it's only 2 Japanese games.
        LogError("Unable to determine cartridge type");
        return false;
    }

    LogInfo("Title = %.21s", standardHeader.title);

    romType = static_cast<ERomType>(standardHeader.mode & 0x0F);
    isLoRom = (romType == ERomType::eLoROM || romType == ERomType::eLoROMSA1 || romType == ERomType::eLoROMSDD1);
    isFastSpeed = (standardHeader.mode >> 4) & 1;
    
    // Interleaved ROMs can be hiROM, but have the header at the LoROM file offset.
    isInterleaved = (offset == LOROM_HEADER_OFFSET) && !isLoRom;
    if (isInterleaved)
    {
        LogError("Interleaved ROM NYI");
        return false;
    }
    
    if (standardHeader.devId == 0x33 || standardHeader.title[20] == 0)
    {
        // If the last byte of the title is null, this is an early extended header where only the chip subtype byte is valid.
        // If devId is 0x33, this is a later extended header where all fields are valid.
        size_t extOffset = offset + EXTENDED_HEADER_OFSET;
        memcpy(&extendedHeader, &rom[extOffset], sizeof(ExtendedHeader));
    }

    return true;
}


bool Cartridge::FindHeader(size_t headerOffset)
{
    StandardHeader header;
    memcpy(&header, &rom[headerOffset], sizeof(StandardHeader));

    LogInfo("Checking for standard header at %04X", headerOffset);

    // Check that title field is all ASCII. The last byte can be null.
    for (int i = 0; i < 20; i++)
    {
        if (header.title[i] < 0x20 || header.title[i] > 0x7E)
        {
            LogError("Non-ASCII value (%02X) in Title at %04X", header.title[i], headerOffset + i);
            return false;
        }
    }
    if ((header.title[20] > 0 && header.title[20] < 0x20) || header.title[20] > 0x7E)
    {
        LogError("Non-ASCII value (%02X) in Title at %04X", header.title[20], headerOffset + 20);
        return false;
    }

    // Look for a mode-like byte. This succeeds at the wrong offset with FF5 which is why the rest of the checks are necesasry.
    if (romModeLookup.find(header.mode & 0x0F) == romModeLookup.end())
    {
        LogError("Bad mode (%02X) at %04X", header.mode, headerOffset + offsetof(StandardHeader, mode));
        return false;
    }

    // Check checksums. This doesn't actually verify all ROM data in case this is a hacked ROM.
    if ((header.checksum ^ header.checksumComplement) != 0xFFFF)
    {
        LogError("Bad checksum at %04X", headerOffset);
        return false;
    }

    LogInfo("Found valid standard header at %04X", headerOffset);
    standardHeader = header;

    return true;
}