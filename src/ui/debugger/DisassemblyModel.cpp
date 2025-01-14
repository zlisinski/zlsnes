#include <algorithm>
#include <QtGui/QPalette>

#include "DisassemblyModel.h"
#include "Opcode.h"
#include "core/Cpu.h"


const QSet<uint8_t> DisassemblyModel::stopOpcodes = {
    0x80, // BRA
    0x82, // BRL
    0x4C, // JMP abs
    0x5C, // JMP long
    0x6C, // JMP (abs)
    0x7C, // JMP (abs,X)
    0xDC, // JMP [abs]
    //0x22, // JSL
    //0x20, // JSR abs
    //0xFC, // JSR (abs,X)
    0x6B, // RTL
    0x60, // RTS
    0x00, // BRK
    0x02, // COP
    0x40, // RTI
    0xDB, // STP
    0xCB, // WAI
    // These change the size of the A, X, and Y registers which affects decompilation.
    0xC2, // REP 
    0xE2, // SEP
};


DisassemblyModel::DisassemblyModel(const QPalette &palette, QObject *parent /*= NULL*/) :
    QAbstractTableModel(parent),
    currentRow(-1),
    palette(palette),
    currentRowColor(palette.color(QPalette::AlternateBase))
{

}


DisassemblyModel::~DisassemblyModel()
{

}


void DisassemblyModel::AddRow(Address pc, const uint8_t *memoryAtPc, const Registers *reg)
{
    uint32_t origUintPc = pc.ToUint();

    if (addresses.contains(origUintPc))
        return;

    int curRowCount = opcodes.size();

    // Disassemble opcodes until we reach a previously reached address, an unconditional jump/call/return, or cross a bank/half-bank boundary.
    std::vector<Opcode> newRows;
    do
    {
        Opcode opcode = Opcode::GetOpcode(pc, memoryAtPc, reg);
        newRows.push_back(opcode);
        addresses.insert(pc.ToUint());

        // Stop if we reach an opcode that would jump/return.
        if (stopOpcodes.contains(*memoryAtPc))
            break;

        // Advance the program counter to the next instruction.
        // Check that we are not moving into a different section of memory.
        pc = pc.AddOffset(opcode.GetByteCount());
        memoryAtPc += opcode.GetByteCount();
        // If we rolled over into the next bank, our memory pointer is no longer valid.
        if (pc.GetBank() != Bytes::GetByte<2>(origUintPc))
            break;
    } while (!addresses.contains(pc.ToUint()));
    
    // Find where to insert the new rows.
    int insertIndex = 0;
    auto it = std::find_if(opcodes.cbegin(), opcodes.cend(), [&origUintPc](const Opcode &item){return item.GetAddress() > origUintPc;});
    if (it == opcodes.cend())
        insertIndex = curRowCount;
    else
        insertIndex = std::distance(opcodes.cbegin(), it);

    beginInsertRows(QModelIndex(), insertIndex, insertIndex + newRows.size() - 1);
    opcodes.insert(it, newRows.cbegin(), newRows.cend());
    endInsertRows();
}


void DisassemblyModel::RemoveRows(Address address, uint32_t len)
{
    // Find the first opcode that contains address.
    auto startIt = std::find_if(opcodes.cbegin(), opcodes.cend(),
        [&address](const Opcode &item)
        {
            return address >= item.GetAddress() && address < (item.GetAddress().AddOffset(item.GetByteCount()));
        });

    if (startIt == opcodes.cend())
        return;

    // Find the first element past the end of address+len.
    auto endIt = startIt + 1;
    if (len > 1)
    {
        Address endAddr = address.AddOffset(len);
        endIt = std::find_if(opcodes.cbegin(), opcodes.cend(),
            [&endAddr](const Opcode &item)
            {
                return item.GetAddress().AddOffset(item.GetByteCount()) > endAddr;
            });
    }

    int startIndex = std::distance(opcodes.cbegin(), startIt);
    int endIndex = std::distance(opcodes.cbegin(), endIt) - 1;

    for (auto it = startIt; it < endIt; ++it)
        addresses.remove(it->GetAddress().ToUint());

    beginRemoveRows(QModelIndex(), startIndex, endIndex);
    opcodes.erase(startIt, endIt);
    endRemoveRows();
}


int DisassemblyModel::GetRowIndex(Address pc) const
{
    auto it = std::find_if(opcodes.cbegin(), opcodes.cend(), [&pc](const Opcode &item){return item.GetAddress() == pc;});
    
    if (it == opcodes.cend())
        return -1;
    
    return std::distance(opcodes.cbegin(), it);
}


int DisassemblyModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    Q_UNUSED(parent);

    return opcodes.size();
}


int DisassemblyModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    Q_UNUSED(parent);

    return colCount;
}


QVariant DisassemblyModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= (int)opcodes.size())
        return QVariant();

    if (index.column() >= colCount)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        const Opcode &item = opcodes[index.row()];
        switch (index.column())
        {
            case 0:
                return item.GetBytesStr();
            case 1:
                return item.GetOpcodeStr();
            case 2:
                return item.GetOperandStr();
            case 3:
                return item.GetAddrModeStr();
        }
    }
    else if (role == Qt::BackgroundRole)
    {
        if (index.row() == currentRow)
            return currentRowColor;
    }

    return QVariant();
}


QVariant DisassemblyModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    
    if (section >= (int)opcodes.size())
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        return QVariant();
    }
    else
    {
        Address pc = opcodes[section].GetAddress();
        return QStringLiteral("%1").arg(pc.ToUint(), 6, 16, QChar('0')).toUpper();
    }
}