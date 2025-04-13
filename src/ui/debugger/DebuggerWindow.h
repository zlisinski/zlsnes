#pragma once

#include <QtWidgets/QMainWindow>

#include "../../core/Zlsnes.h"
#include "../../core/DebuggerInterface.h"

namespace Ui {
class DebuggerWindow;
}

class Cpu;
class DisassemblyModel;
class IoRegisterModel;
class Memory;
class MemoryModel;


// 0xFFFFFFFF is outside the 24-bit addressable range, so it's safe to use for an invalid address value.
const uint32_t INVALID_ADDR = 0xFFFFFFFF;


class DebuggerWindow : public QMainWindow, public DebuggerInterface
{
    Q_OBJECT

public:
    explicit DebuggerWindow(QWidget *parent, bool debuggingEnabled = false, uint32_t runToAddress = INVALID_ADDR);
    ~DebuggerWindow();

    void SetEmulatorObjects(Memory *newMemory, Cpu *newCpu, Ppu *newPpu) override;

    bool ShouldRun(Address pc) override;
    void SetCurrentOp(Address pc) override;

    void MemoryChanged(Address address, uint16_t len) override;

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    void UpdateStack();
    void UpdateWidgets(Address pc);
    void UpdateMemoryView();

    Ui::DebuggerWindow *ui;

    Cpu *cpu;
    Memory *memory;
    Ppu *ppu;
    uint16_t currentSp;

    std::atomic<bool> singleStep;
    std::atomic<uint32_t> runToAddress;

    DisassemblyModel *disassemblyModel;
    IoRegisterModel *ioRegisterModel;
    MemoryModel *memoryModel;

private slots:
    void SlotProcessUpdate(Address pc);
    void SlotToggleDebugging(bool checked);
    void SlotStep();
    void SlotRunToLine();
    void SlotDisassembleAddress();
    void SlotReenableActions();
    void SlotObjectsChanged();
    void SlotMemoryChanged(Address address, uint16_t len);
    void on_cmbMemoryType_currentTextChanged(const QString &text);
    void on_btnExportMemory_clicked();

signals:
    void SignalDebuggerWindowClosed();
    void SignalUpdateReady(Address pc);
    void SignalReenableActions();
    void SignalObjectsChanged();
    void SignalMemoryChanged(Address address, uint16_t len);
};
