#include <fstream>
#include <unordered_set>

#include "Cartridge.h"
#include "Bytes.h"

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

    if (!Validate())
        return false;

    if (standardHeader.ramSize != 0)
    {
        uint32_t ramSize = (1 << standardHeader.ramSize) * 1024;
        sramSizeMask = ramSize - 1;
        sramFilename = filename + ".srm";
        std::ifstream file(sramFilename, std::ios::binary);
        if (file)
        {
            std::istreambuf_iterator<char> start(file), end;
            sram = std::vector<uint8_t>(start, end);
            if (sram.size() != ramSize)
            {
                LogWarning("Ram file %s not the same size as SRAM chip. Starting with fresh SRAM.");
                sram = std::vector<uint8_t>(ramSize, 0);
            }
        }
        else
        {
            LogWarning("Couldn't open %s. Starting with fresh SRAM", sramFilename.c_str());
            sram = std::vector<uint8_t>(ramSize, 0);
        }
    }

    return true;
}


bool Cartridge::SaveSram()
{
    std::ofstream file(sramFilename, std::ios::binary);
    if (!file.is_open())
    {
        LogError("Error opening sram file %s", sramFilename.c_str());
        return false;
    }

    file.write(reinterpret_cast<char *>(sram.data()), sram.size());
    file.close();

    LogInfo("Saved SRAM");

    return true;
}


void Cartridge::Reset()
{
    rom.clear();
    sram.clear();
    sramFilename = "";
    isInterleaved = false;
    romType = ERomType::eLoROM;
    isLoRom = true;
    isFastSpeed = false;
    standardHeader = StandardHeader();
    extendedHeader = ExtendedHeader();
}


uint32_t Cartridge::MapAddress(uint32_t addr, std::vector<uint8_t> **mem)
{
    // This assumes that accesses to special addresses like wram and io ports have already been filtered out before getting here.

    if (isLoRom)
    {
        if (standardHeader.ramSize != 0 && (addr & 0x708000) == 0x700000 && (addr & 0xFE0000) != 0x7E0000)
        {
            // This is a read from SRAM. 70-7D:0000-7FFF, F0-FF:0000-7FFF
            *mem = &sram;

            // Remove the high bit of the offset and shift the bank right one so that LSBit of bank is MSBit of offset.
            // Ignore the high nybble of bank.
            uint32_t mappedAddr = (((addr & 0x0F0000) >> 1) | (addr & 0x7FFF)) & sramSizeMask;
            return mappedAddr;
        }

        // This is a read from LoROM area. 00-7D:8000-FFFF, 80-FF:8000-FFFF
        // or
        // This is a read from HiROM area. 40-7D:0000-7FFF, C0-FF:0000-7FFF
        *mem = &rom;

        // Remove the high bit of the offset and shift the bank right one so that LSBit of bank is MSBit of offset.
        // Ignore the high bit of bank, which selects WS1/WS2.
        uint32_t mappedAddr = ((addr & 0x7F0000) >> 1) | (addr & 0x7FFF);
        return mappedAddr;
    }
    else
    {
        if (standardHeader.ramSize != 0 && (addr & 0x40E000) == 0x006000)
        {
            // This is a read from SRAM. 30-3F:6000-7FFF, B0-BF:6000-7FFF
            *mem = &sram;

            // Shift the bank down to fill the unused top 3 bits of the offset.
            uint32_t mappedAddr = (((addr & 0xFF0000) >> 3) | (addr & 0x1FFF)) & sramSizeMask;
            return mappedAddr;
        }

        // This is a read from HiROM area. 40-7D:0000-FFFF, C0-FF:0000-FFFF
        // or
        // This is a read from LoROM area. 00-3F:8000-FFFF, 80-BF:8000-FFFF
        *mem = &rom;

        // Clear bits 22 and 23.
        uint32_t mappedAddr = addr & 0x3FFFFF;
        return mappedAddr;
    }
}


uint8_t Cartridge::ReadByte(uint32_t addr)
{
    std::vector<uint8_t> *mem;
    uint32_t mappedAddr = MapAddress(addr, &mem);

    return (*mem)[mappedAddr];
}


void Cartridge::WriteByte(uint32_t addr, uint8_t byte)
{
    std::vector<uint8_t> *mem;
    uint32_t mappedAddr = MapAddress(addr, &mem);

    if (mem == &rom)
    {
        LogError("Write to ROM address %06X(%06X) = %02X", addr, mappedAddr, byte);
        //throw std::range_error(fmt("Write to ROM address %06X(%06X)", addr, mappedAddr));
        return;
    }

    (*mem)[mappedAddr] = byte;
}


uint8_t *Cartridge::GetBytePtr(uint32_t addr)
{
    std::vector<uint8_t> *mem;
    uint32_t mappedAddr = MapAddress(addr, &mem);

    return &(*mem)[mappedAddr];
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

    size_t offset = 0;

    if ((rom.size() >= LOROM_HEADER_OFFSET + 32) && FindHeader(LOROM_HEADER_OFFSET))
    {
        offset = LOROM_HEADER_OFFSET;
    }
    else if ((rom.size() >= HIROM_HEADER_OFFSET + 32) && FindHeader(HIROM_HEADER_OFFSET))
    {
        offset = HIROM_HEADER_OFFSET;
    }
    else
    {
        //Skip checking ExHiROM, since it's only 2 Japanese games.
        LogError("Unable to determine cartridge type");
        //return false;
        // Default to LoROM for now.
        offset = LOROM_HEADER_OFFSET;
        memcpy(&standardHeader, &rom[offset], sizeof(StandardHeader));
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
    if ((header.checksum ^ header.checksumComplement) != 0xFFFF &&
        !(header.checksum == 0x5343 && header.checksumComplement == 0x4343) && // Some test roms use these hardcoded values.
        !(header.checksum == 0x0000 && header.checksumComplement == 0x0000)) // Some test roms have all zeros.
    {
        LogError("Bad checksum at %04X", headerOffset);
        return false;
    }

    LogInfo("Found valid standard header at %04X", headerOffset);
    standardHeader = header;

    return true;
}