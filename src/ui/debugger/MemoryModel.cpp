#include <QtCore/QString>

#include "../UiUtils.h"
#include "MemoryModel.h"


static const int COL_COUNT = 16;
static const QString HEADERS[COL_COUNT] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"};


MemoryModel::MemoryModel(QObject *parent /*= NULL*/) :
    QAbstractTableModel(parent),
    memory(nullptr),
    memSize(0)
{

}


MemoryModel::~MemoryModel()
{

}


void MemoryModel::SetMemory(const uint8_t *newMemory, size_t size)
{
    beginResetModel();
    memory = newMemory;
    memSize = size;
    endResetModel();
}


void MemoryModel::InvalidateMemory(Address address, uint16_t len)
{
    /*int startRow = address / 16;
    int endRow = (address + len - 1) / 16;
    QModelIndex startIndex = this->index(startRow, 0);
    QModelIndex endIndex = this->index(endRow, COL_COUNT);

    emit dataChanged(startIndex, endIndex);*/
}


int MemoryModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    Q_UNUSED(parent);

    return memSize / 16;
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
        return UiUtils::FormatHexByte(memory[(index.row() * 16) + index.column()]);
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