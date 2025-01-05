#pragma once

#include <QtCore/QAbstractTableModel>

#include "core/Zlsnes.h"
#include "core/Address.h"


class MemoryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    MemoryModel(QObject *parent = NULL);
    virtual ~MemoryModel();

    void SetMemory(const uint8_t *newMemory, size_t size);
    void InvalidateMemory(Address address, uint16_t len);

    // Overrides for QAbstractTableModel.
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
    const uint8_t *memory;
    size_t memSize;
};