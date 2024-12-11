#pragma once

#include <QtCore/QAbstractTableModel>

#include "core/Zlsnes.h"

class Memory;


class IoRegisterModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    IoRegisterModel(QObject *parent = NULL);
    virtual ~IoRegisterModel();

    void SetMemory(Memory *newMemory);
    void InvalidateMemory(Address address, uint16_t len);

    // Overrides for QAbstractTableModel.
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    Memory *memory;
};