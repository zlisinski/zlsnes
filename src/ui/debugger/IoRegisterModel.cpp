#include <QtCore/QString>

#include "core/Memory.h"

#include "../UiUtils.h"
#include "IoRegisterModel.h"


static const int ROW_COUNT = 105;
static const int COL_COUNT = 1;


struct IoRegData
{
    QString name;
    uint16_t addr;

    IoRegData(QString name, uint16_t addr) : name(name), addr(addr) {}
};


static const IoRegData ioRegData[] = {
    {"2100 INIDISP", 0x2100},
    {"2101 OBSEL", 0x2101},
    {"2102 OAMADDL", 0x2102},
    {"2103 OAMADDH", 0x2103},
    {"2104 OAMDATA", 0x2104},
    {"2105 BGMODE", 0x2105},
    {"2106 MOSAIC", 0x2106},
    {"2107 BG1SC", 0x2107},
    {"2108 BG2SC", 0x2108},
    {"2109 BG3SC", 0x2109},
    {"210A BG4SC", 0x210A},
    {"210B BG12NBA", 0x210B},
    {"210C BG34NBA", 0x210C},
    {"210D BG1HOFS", 0x210D},
    {"210E BG1VOFS", 0x210E},
    {"210F BG2HOFS", 0x210F},
    {"2110 BG2VOFS", 0x2110},
    {"2111 BG3HOFS", 0x2111},
    {"2112 BG3VOFS", 0x2112},
    {"2113 BG4HOFS", 0x2113},
    {"2114 BG4VOFS", 0x2114},
    {"2115 VMAIN", 0x2115},
    {"2116 VMADDL", 0x2116},
    {"2117 VMADDH", 0x2117},
    {"2118 VMDATAL", 0x2118},
    {"2119 VMDATAH", 0x2119},
    {"211A M7SEL", 0x211A},
    {"211B M7A", 0x211B},
    {"211C M7B", 0x211C},
    {"211D M7C", 0x211D},
    {"211E M7D", 0x211E},
    {"211F M7X", 0x211F},
    {"2120 M7Y", 0x2120},
    {"2121 CGADD", 0x2121},
    {"2122 CGDATA", 0x2122},
    {"2123 W12SEL", 0x2123},
    {"2124 W34SEL", 0x2124},
    {"2125 WOBJSEL", 0x2125},
    {"2126 WH0", 0x2126},
    {"2127 WH1", 0x2127},
    {"2128 WH2", 0x2128},
    {"2129 WH3", 0x2129},
    {"212A WBGLOG", 0x212A},
    {"212B WOBJLOG", 0x212B},
    {"212C TM", 0x212C},
    {"212D TS", 0x212D},
    {"212E TMW", 0x212E},
    {"212F TSW", 0x212F},
    {"2130 CGWSEL", 0x2130},
    {"2131 CGADSUB", 0x2131},
    {"2132 COLDATA", 0x2132},
    {"2133 SETINI", 0x2133},
    {"2134 MPYL", 0x2134},
    {"2135 MPYM", 0x2135},
    {"2136 MPYH", 0x2136},
    {"2137 SLHV", 0x2137},
    {"2138 RDOAM", 0x2138},
    {"2139 RDVRAML", 0x2139},
    {"213A RDVRAMH", 0x213A},
    {"213B RDCGRAM", 0x213B},
    {"213C OPHCT", 0x213C},
    {"213D OPVCT", 0x213D},
    {"213E STAT77", 0x213E},
    {"213F STAT78", 0x213F},
    {"2140 APUI00", 0x2140},
    {"2141 APUI01", 0x2141},
    {"2142 APUI02", 0x2142},
    {"2143 APUI03", 0x2143},
    {"2180 WMDATA", 0x2180},
    {"2181 WMADDL", 0x2181},
    {"2182 WMADDM", 0x2182},
    {"2183 WMADDH", 0x2183},
    {"4016 JOYWR", 0x4016},
    {"4016 JOYA", 0x4016},
    {"4017 JOYB", 0x4017},
    {"4200 NMITIMEN", 0x4200},
    {"4201 WRIO", 0x4201},
    {"4202 WRMPYA", 0x4202},
    {"4203 WRMPYB", 0x4203},
    {"4204 WRDIVL", 0x4204},
    {"4205 WRDIVH", 0x4205},
    {"4206 WRDIVB", 0x4206},
    {"4207 HTIMEL", 0x4207},
    {"4208 HTIMEH", 0x4208},
    {"4209 VTIMEL", 0x4209},
    {"420A VTIMEH", 0x420A},
    {"420B MDMAEN", 0x420B},
    {"420C HDMAEN", 0x420C},
    {"420D MEMSEL", 0x420D},
    {"4210 RDNMI", 0x4210},
    {"4211 TIMEUP", 0x4211},
    {"4212 HVBJOY", 0x4212},
    {"4213 RDIO", 0x4213},
    {"4214 RDDIVL", 0x4214},
    {"4215 RDDIVH", 0x4215},
    {"4216 RDMPYL", 0x4216},
    {"4217 RDMPYH", 0x4217},
    {"4218 JOY1L", 0x4218},
    {"4219 JOY1H", 0x4219},
    {"421A JOY2L", 0x421A},
    {"421B JOY2H", 0x421B},
    {"421C JOY3L", 0x421C},
    {"421D JOY3H", 0x421D},
    {"421E JOY4L", 0x421E},
    {"421F JOY4H", 0x421F}
};


IoRegisterModel::IoRegisterModel(QObject *parent /*= NULL*/) :
    QAbstractTableModel(parent),
    memory(NULL)
{

}


IoRegisterModel::~IoRegisterModel()
{

}


void IoRegisterModel::SetMemory(Memory *newMemory)
{
    beginResetModel();
    memory = newMemory;
    endResetModel();
}


void IoRegisterModel::InvalidateMemory(Address address, uint16_t len)
{
    (void)len;
    uint32_t page = address.ToUint() & 0x40FF00;
    // For now, don't bother figuring out which row changed, just update them all.
    if (page == 0x2100 || page == 0x4000 || page == 0x4200 || page == 0x4300)
    {
        QModelIndex startIndex = this->index(0, 0);
        QModelIndex endIndex = this->index(ROW_COUNT - 1, 0);
        emit dataChanged(startIndex, endIndex);
    }
}


int IoRegisterModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    Q_UNUSED(parent);

    return ROW_COUNT;
}


int IoRegisterModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    Q_UNUSED(parent);

    return COL_COUNT;
}


QVariant IoRegisterModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return QVariant();

    if (memory == NULL)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        uint8_t byte = memory->ReadRaw8Bit(ioRegData[index.row()].addr);
        return UiUtils::FormatHexByte(byte);
    }

    return QVariant();
}


QVariant IoRegisterModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        return QVariant();
    }
    else
    {
        return ioRegData[section].name;
    }
}