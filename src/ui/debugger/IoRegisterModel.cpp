#include <QtCore/QString>

#include "core/Memory.h"

#include "../UiUtils.h"
#include "IoRegisterModel.h"


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
    {"421F JOY4H", 0x421F},
    {"4300 DMAP0", 0x4300},
    {"4301 BBAD0", 0x4301},
    {"4302 A1T0L", 0x4302},
    {"4303 A1T0H", 0x4303},
    {"4304 A1B0", 0x4304},
    {"4305 DAS0L", 0x4305},
    {"4306 DAS0H", 0x4306},
    {"4307 DASB0", 0x4307},
    {"4308 A2A0L", 0x4308},
    {"4309 A2A0H", 0x4309},
    {"430A NTRL0", 0x430A},
    {"4310 DMAP1", 0x4310},
    {"4311 BBAD1", 0x4311},
    {"4312 A1T1L", 0x4312},
    {"4313 A1T1H", 0x4313},
    {"4314 A1B1", 0x4314},
    {"4315 DAS1L", 0x4315},
    {"4316 DAS1H", 0x4316},
    {"4317 DASB1", 0x4317},
    {"4318 A2A1L", 0x4318},
    {"4319 A2A1H", 0x4319},
    {"431A NTRL1", 0x431A},
    {"4320 DMAP2", 0x4320},
    {"4321 BBAD2", 0x4321},
    {"4322 A1T2L", 0x4322},
    {"4323 A1T2H", 0x4323},
    {"4324 A1B2", 0x4324},
    {"4325 DAS2L", 0x4325},
    {"4326 DAS2H", 0x4326},
    {"4327 DASB2", 0x4327},
    {"4328 A2A2L", 0x4328},
    {"4329 A2A2H", 0x4329},
    {"432A NTRL2", 0x432A},
    {"4330 DMAP3", 0x4330},
    {"4331 BBAD3", 0x4331},
    {"4332 A1T3L", 0x4332},
    {"4333 A1T3H", 0x4333},
    {"4334 A1B3", 0x4334},
    {"4335 DAS3L", 0x4335},
    {"4336 DAS3H", 0x4336},
    {"4337 DASB3", 0x4337},
    {"4338 A2A3L", 0x4338},
    {"4339 A2A3H", 0x4339},
    {"433A NTRL3", 0x433A},
    {"4340 DMAP4", 0x4340},
    {"4341 BBAD4", 0x4341},
    {"4342 A1T4L", 0x4342},
    {"4343 A1T4H", 0x4343},
    {"4344 A1B4", 0x4344},
    {"4345 DAS4L", 0x4345},
    {"4346 DAS4H", 0x4346},
    {"4347 DASB4", 0x4347},
    {"4348 A2A4L", 0x4348},
    {"4349 A2A4H", 0x4349},
    {"434A NTRL4", 0x434A},
    {"4350 DMAP5", 0x4350},
    {"4351 BBAD5", 0x4351},
    {"4352 A1T5L", 0x4352},
    {"4353 A1T5H", 0x4353},
    {"4354 A1B5", 0x4354},
    {"4355 DAS5L", 0x4355},
    {"4356 DAS5H", 0x4356},
    {"4357 DASB5", 0x4357},
    {"4358 A2A5L", 0x4358},
    {"4359 A2A5H", 0x4359},
    {"435A NTRL5", 0x435A},
    {"4360 DMAP6", 0x4360},
    {"4361 BBAD6", 0x4361},
    {"4362 A1T6L", 0x4362},
    {"4363 A1T6H", 0x4363},
    {"4364 A1B6", 0x4364},
    {"4365 DAS6L", 0x4365},
    {"4366 DAS6H", 0x4366},
    {"4367 DASB6", 0x4367},
    {"4368 A2A6L", 0x4368},
    {"4369 A2A6H", 0x4369},
    {"436A NTRL6", 0x436A},
    {"4370 DMAP7", 0x4370},
    {"4371 BBAD7", 0x4371},
    {"4372 A1T7L", 0x4372},
    {"4373 A1T7H", 0x4373},
    {"4374 A1B7", 0x4374},
    {"4375 DAS7L", 0x4375},
    {"4376 DAS7H", 0x4376},
    {"4377 DASB7", 0x4377},
    {"4378 A2A7L", 0x4378},
    {"4379 A2A7H", 0x4379},
    {"437A NTRL7", 0x437A},
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
        QModelIndex endIndex = this->index(this->rowCount() - 1, 0);
        emit dataChanged(startIndex, endIndex);
    }
}


int IoRegisterModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    Q_UNUSED(parent);

    return sizeof(ioRegData) / sizeof(IoRegData);
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