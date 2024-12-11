#pragma once

#include <QtCore/QAbstractTableModel>
#include <QtCore/QSet>
#include <vector>

#include "core/Zlsnes.h"
#include "core/Address.h"
#include "Opcode.h"

struct Registers;

class DisassemblyModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    DisassemblyModel(const QPalette &palette, QObject *parent = NULL);
    virtual ~DisassemblyModel();

    void AddRow(Address pc, const uint8_t *memoryAtPc, const Registers *reg);
    void RemoveRows(Address address, uint32_t len);
    int GetRowIndex(Address pc) const;
    Address GetAddressOfRow(int row) const {return opcodes[row].GetAddress();}
    void SetCurrentRow(int row) {currentRow = row;}

    // Overrides for QAbstractTableModel.
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    static const int colCount = 4;
    static const QSet<uint8_t> stopOpcodes;

    std::vector<Opcode> opcodes;
    QSet<uint32_t> addresses;

    int currentRow;
    const QPalette &palette;
    const QColor &currentRowColor;
};