#include <QtCore/QString>

#include "core/Memory.h"

#include "../UiUtils.h"
#include "MemoryModel.h"


static const int ROW_COUNT = 0x1000;
static const int COL_COUNT = 17;
static const QString HEADERS[COL_COUNT] = {"Section", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"};


MemoryModel::MemoryModel(QObject *parent /*= NULL*/) :
    QAbstractTableModel(parent),
    memory(NULL)
{

}


MemoryModel::~MemoryModel()
{

}


void MemoryModel::SetMemory(const Memory *newMemory)
{
    beginResetModel();
    memory = newMemory;
    endResetModel();
}


void MemoryModel::InvalidateMemory(uint16_t address, uint16_t len)
{
    int startRow = address / 16;
    int endRow = (address + len - 1) / 16;
    QModelIndex startIndex = this->index(startRow, 0);
    QModelIndex endIndex = this->index(endRow, COL_COUNT);

    emit dataChanged(startIndex, endIndex);
}


int MemoryModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    Q_UNUSED(parent);

    return ROW_COUNT;
}


int MemoryModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    Q_UNUSED(parent);

    return COL_COUNT;
}


QVariant MemoryModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return QVariant();

    if (memory == NULL)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        if (index.column() == 0)
        {
            int address = index.row() * 16;

            if (address < 0x4000)
                return "ROM0";
            if (address < 0x8000)
                return "ROM" + UiUtils::FormatHexByte(memory->GetCurRomBank());
            if (address < 0xA000)
                return "VRAM";
            if (address < 0xC000)
                return "SRAM";
            if (address < 0xE000)
                return "WRAM";
            if (address < 0xFE00)
                return "ECHO";
            if (address < 0xFEA0)
                return "OAM";
            if (address < 0xFF00)
                return "NONE";
            if (address < 0xFF80)
                return "IOREG";
            return "HRAM";
        }
        else
        {
            uint8_t byte = memory->ReadRawByte((index.row() * 16) + (index.column() - 1));
            return UiUtils::FormatHexByte(byte);
        }
    }

    return QVariant();
}


QVariant MemoryModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        return HEADERS[section];
    }
    else
    {
        return UiUtils::FormatHexWord(section * 16);
    }
}