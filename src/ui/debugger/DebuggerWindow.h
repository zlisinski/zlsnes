#pragma once

#include <QtWidgets/QMainWindow>

#include "core/Zlsnes.h"
#include "core/DebuggerInterface.h"

namespace Ui {
class DebuggerWindow;
}

class Cpu;
class DisassemblyModel;
class IoRegisterModel;
class Memory;
//class MemoryModel;


class DebuggerWindow : public QMainWindow, public DebuggerInterface
{
    Q_OBJECT

public:
    explicit DebuggerWindow(QWidget *parent = 0);
    ~DebuggerWindow();

    void SetEmulatorObjects(Memory *newMemory, Cpu *newCpu) override;

    bool GetDebuggingEnabled() override {return debuggingEnabled;}
    bool ShouldRun(Address pc) override;
    void SetCurrentOp(Address pc) override;

    void MemoryChanged(Address address, uint16_t len) override;

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    void UpdateStack();
    void UpdateWidgets(Address pc);

    Ui::DebuggerWindow *ui;

    Cpu *cpu;
    Memory *memory;
    uint16_t currentSp;

    std::atomic<bool> debuggingEnabled;
    std::atomic<bool> singleStep;
    std::atomic<uint32_t> runToAddress;

    DisassemblyModel *disassemblyModel;
    IoRegisterModel *ioRegisterModel;
    //MemoryModel *memoryModel;

private slots:
    void SlotProcessUpdate(Address pc);
    void SlotToggleDebugging(bool checked);
    void SlotStep();
    void SlotRunToLine();
    void SlotDisassembleAddress();
    void SlotReenableActions();
    void SlotObjectsChanged();
    void SlotMemoryChanged(Address address, uint16_t len);

signals:
    void SignalDebuggerWindowClosed();
    void SignalUpdateReady(Address pc);
    void SignalReenableActions();
    void SignalObjectsChanged();
    void SignalMemoryChanged(Address address, uint16_t len);
};
