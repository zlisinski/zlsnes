#include <QtCore/QSettings>
#include <thread>

#include "core/Cpu.h"
#include "core/Memory.h"
#include "core/Ppu.h"

#include "../SettingsConstants.h"
#include "../UiUtils.h"
#include "AddressDialog.h"
#include "DebuggerWindow.h"
#include "DisassemblyModel.h"
#include "IoRegisterModel.h"
#include "MemoryModel.h"
#include "ui_DebuggerWindow.h"


// 0xFFFFFFFF is outside the 24-bit addressable range, so it's safe to use for an invalid address value.
static const uint32_t INVALID_ADDR = 0xFFFFFFFF;


DebuggerWindow::DebuggerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DebuggerWindow),
    cpu(nullptr),
    memory(nullptr),
    ppu(nullptr),
    currentSp(0),
    debuggingEnabled(false),
    singleStep(false),
    runToAddress(INVALID_ADDR),
    disassemblyModel(new DisassemblyModel(palette(), this)),
    ioRegisterModel(new IoRegisterModel(this)),
    memoryModel(new MemoryModel(this))
{
    ui->setupUi(this);
    ui->actionToggleDebugging->setChecked(debuggingEnabled);
    SlotToggleDebugging(debuggingEnabled);
    ui->menuView->addAction(ui->dockCallStack->toggleViewAction());
    ui->menuView->addAction(ui->dockMemory->toggleViewAction());
    ui->menuView->addAction(ui->dockRegisters->toggleViewAction());
    ui->menuView->addAction(ui->dockStack->toggleViewAction());

    qRegisterMetaType<QItemSelection>();
    qRegisterMetaType<Address>("Address");
    qRegisterMetaType<uint16_t>("uint16_t");

    QSettings settings;
    restoreGeometry(settings.value(SETTINGS_DEBUGGERWINDOW_GEOMETRY).toByteArray());
    restoreState(settings.value(SETTINGS_DEBUGGERWINDOW_STATE).toByteArray());

    ui->disassemblyView->setModel(disassemblyModel);
    ui->ioRegistersView->setModel(ioRegisterModel);
    ui->memoryView->setModel(memoryModel);

    ui->memoryView->resizeColumnsToContents();

    ui->cmbMemoryType->addItems({"WRAM", "VRAM", "OAM", "CGRAM"});

    connect(ui->actionToggleDebugging, SIGNAL(triggered(bool)), this, SLOT(SlotToggleDebugging(bool)));
    connect(ui->actionStep, SIGNAL(triggered()), this, SLOT(SlotStep()));
    connect(ui->actionRunToLine, SIGNAL(triggered()), this, SLOT(SlotRunToLine()));
    connect(ui->actionDisassemble, SIGNAL(triggered()), this, SLOT(SlotDisassembleAddress()));
    connect(this, SIGNAL(SignalDebuggerWindowClosed()), parent, SLOT(SlotDebuggerWindowClosed()));
    connect(this, SIGNAL(SignalUpdateReady(Address)), this, SLOT(SlotProcessUpdate(Address)));
    connect(this, SIGNAL(SignalReenableActions()), this, SLOT(SlotReenableActions()));
    connect(this, SIGNAL(SignalObjectsChanged()), this, SLOT(SlotObjectsChanged()));
    connect(this, SIGNAL(SignalMemoryChanged(Address, uint16_t)), this, SLOT(SlotMemoryChanged(Address, uint16_t)));
}


DebuggerWindow::~DebuggerWindow()
{
    delete ui;
}


void DebuggerWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue(SETTINGS_DEBUGGERWINDOW_GEOMETRY, saveGeometry());
    settings.setValue(SETTINGS_DEBUGGERWINDOW_STATE, saveState());

    emit SignalDebuggerWindowClosed();

    QWidget::closeEvent(event);
}


void DebuggerWindow::SetEmulatorObjects(Memory *newMemory, Cpu *newCpu, Ppu *newPpu)
{
    //This function runs in the thread context of the Emulator worker thread.

    // All values should be NULL or not NULL. If only some are NULL, treat them all as NULL.
    if (newMemory == nullptr || newCpu == nullptr || newPpu == nullptr)
    {
        memory = nullptr;
        cpu = nullptr;
        ppu = nullptr;
    }
    else
    {
        memory = newMemory;
        cpu = newCpu;
        ppu = newPpu;
    }

    // Run the rest in current thread context.
    emit SignalObjectsChanged();
}


bool DebuggerWindow::ShouldRun(Address pc)
{
    //This function runs in the thread context of the Emulator worker thread.

    // Run if we're in single step mode, or we have a run-to address and we're not there.
    return debuggingEnabled == false || singleStep == true || (runToAddress != INVALID_ADDR && pc.ToUint() != runToAddress);
}


void DebuggerWindow::SetCurrentOp(Address pc)
{
    //This function runs in the thread context of the Emulator worker thread.

    // Stop single step mode.
    if (singleStep == true)
    {
        // Re-Enable controls. Emit a signal, since we are running in a non-main thread.
        emit SignalReenableActions();
    }
    singleStep = false;

    // If we've hit the run-to address, reset the variable.
    if (runToAddress == pc.ToUint())
    {
        runToAddress = INVALID_ADDR;

        // Re-Enable controls. Emit a signal, since we are running in a non-main thread.
        emit SignalReenableActions();
    }

    emit SignalUpdateReady(pc);
}


void DebuggerWindow::MemoryChanged(Address address, uint16_t len)
{
    //This function runs in the thread context of the Emulator worker thread.

    // Run the rest in current thread context.
    emit SignalMemoryChanged(address, len);
}


void DebuggerWindow::UpdateStack()
{
    if (cpu == NULL || memory == NULL)
        return;

    currentSp = cpu->reg.sp + 1;

    // Let the view determine how many rows to be displayed.
    const int rowCount = ui->stackView->rowCount();

    ui->stackView->clearContents();
    ui->stackView->setVerticalHeaderLabels({});

    for (int i = 0; i < rowCount; i++)
    {
        // Don't go outside the wram mirror block, the pointer is no longer valid.
        if ((currentSp + i) > 0x1FFF)
            break;

        uint16_t address = currentSp + i;
        QTableWidgetItem *item = new QTableWidgetItem(UiUtils::FormatHexWord(address));
        ui->stackView->setVerticalHeaderItem(i, item);

        uint8_t value = memory->ReadRaw8Bit(address);
        item = new QTableWidgetItem(UiUtils::FormatHexByte(value));
        ui->stackView->setItem(i, 0, item);
    }
}


void DebuggerWindow::UpdateWidgets(Address pc)
{
    // This assumes we are executing code from a ROM bank. This could cause problems if the pc points to a wram mirror.
    disassemblyModel->AddRow(pc, memory->GetBytePtr(pc.ToUint()), &cpu->reg);

    int rowIndex = disassemblyModel->GetRowIndex(pc);
    if (rowIndex >= 0)
    {
        disassemblyModel->SetCurrentRow(rowIndex);
        ui->disassemblyView->scrollTo(ui->disassemblyView->model()->index(rowIndex, 0));
        ui->disassemblyView->viewport()->update();
    }

    if (cpu != NULL)
    {
        ui->txtRegA->setText(UiUtils::FormatHexWord(cpu->reg.a));
        ui->txtRegX->setText(UiUtils::FormatHexWord(cpu->reg.x));
        ui->txtRegY->setText(UiUtils::FormatHexWord(cpu->reg.y));
        ui->txtRegD->setText(UiUtils::FormatHexWord(cpu->reg.d));
        ui->txtRegP->setText(UiUtils::FormatHexByte(cpu->reg.p));
        ui->txtRegPB->setText(UiUtils::FormatHexByte(cpu->reg.pb));
        ui->txtRegDB->setText(UiUtils::FormatHexByte(cpu->reg.db));
        ui->txtRegPC->setText(UiUtils::FormatHexByte(cpu->reg.pb) + ":" + UiUtils::FormatHexWord(cpu->reg.pc));
        ui->txtRegSP->setText("00:" + UiUtils::FormatHexWord(cpu->reg.sp));

        ui->chkFlagC->setChecked(cpu->reg.flags.c);
        ui->chkFlagZ->setChecked(cpu->reg.flags.z);
        ui->chkFlagI->setChecked(cpu->reg.flags.i);
        ui->chkFlagD->setChecked(cpu->reg.flags.d);
        ui->chkFlagX->setChecked(cpu->reg.flags.x);
        ui->chkFlagM->setChecked(cpu->reg.flags.m);
        ui->chkFlagV->setChecked(cpu->reg.flags.v);
        ui->chkFlagN->setChecked(cpu->reg.flags.n);
        ui->chkFlagE->setChecked(cpu->reg.emulationMode);

        if (currentSp != cpu->reg.sp)
            UpdateStack();
    }
}


void DebuggerWindow::UpdateMemoryView()
{
    if (memory == nullptr || ppu == nullptr)
        return;

    QString selected = ui->cmbMemoryType->currentText();

    if (selected == "WRAM")
    {
        memoryModel->SetMemory(memory->GetBytePtr(WRAM_OFFSET), WRAM_SIZE);
    }
    else if (selected == "VRAM")
    {
        memoryModel->SetMemory(ppu->GetVramPtr(), VRAM_SIZE);
    }
    else if (selected == "OAM")
    {
        memoryModel->SetMemory(ppu->GetOamPtr(), OAM_SIZE);
    }
    else if (selected == "CGRAM")
    {
        memoryModel->SetMemory(ppu->GetCgramPtr(), CGRAM_SIZE);
    }
}


void DebuggerWindow::SlotProcessUpdate(Address pc)
{
    UpdateWidgets(pc);

    // Add current instruction to call stack.
    Opcode opcode = Opcode::GetOpcode(pc, memory->GetBytePtr(pc.ToUint()), &cpu->reg);
    new QListWidgetItem(opcode.ToString(), ui->callStackView);
}


void DebuggerWindow::SlotToggleDebugging(bool checked)
{
    debuggingEnabled = checked;

    if (debuggingEnabled)
    {
        ui->actionToggleDebugging->setText("Stop Debugging");
        ui->actionStep->setEnabled(true);
        ui->actionRunToLine->setEnabled(true);
        ui->actionDisassemble->setEnabled(true);

        // Update memory table with new values.
        UpdateMemoryView();

        // Update widgets with new values.
        if (cpu != NULL)
        {
            // Sleep to make sure the emulator worker thread has stopped running.
            // I don't feel like adding proper thread synchronization right now.
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            UpdateWidgets(cpu->GetFullPC());
        }

        // Clear the call stack. It would be too slow to log the call stack when not debugging.
        ui->callStackView->clear();
    }
    else
    {
        ui->actionToggleDebugging->setText("Start Debugging");
        ui->actionStep->setEnabled(false);
        ui->actionRunToLine->setEnabled(false);
        ui->actionDisassemble->setEnabled(false);
    }
}


void DebuggerWindow::SlotStep()
{
    singleStep = true;

    // Disable controls while running.
    ui->actionRunToLine->setEnabled(false);
    ui->actionStep->setEnabled(false);
    ui->actionDisassemble->setEnabled(false);
}


void DebuggerWindow::SlotRunToLine()
{
    QItemSelectionModel *selection = ui->disassemblyView->selectionModel();

    if (!selection->hasSelection())
    {
        UiUtils::MessageBox("No row selected");
        return;
    }

    runToAddress = disassemblyModel->GetAddressOfRow(selection->selectedRows()[0].row()).ToUint();

    // Disable controls while running.
    ui->actionRunToLine->setEnabled(false);
    ui->actionStep->setEnabled(false);
    ui->actionDisassemble->setEnabled(false);

    // Clear selection so we can see when the line has been reached. TODO: Add "wait" dialog while running.
    ui->disassemblyView->clearSelection();
}


void DebuggerWindow::SlotDisassembleAddress()
{
    if (memory == NULL)
    {
        UiUtils::MessageBox("Game not loaded");
        return;
    }

    AddressDialog dialog(this);
    dialog.setModal(true);
    int ret = dialog.exec();

    if (ret == 1)
    {
        Address addr(dialog.address);
        disassemblyModel->AddRow(addr, memory->GetBytePtr(addr.ToUint()), &cpu->reg);
    }
}


void DebuggerWindow::SlotReenableActions()
{
    ui->actionRunToLine->setEnabled(true);
    ui->actionStep->setEnabled(true);
    ui->actionDisassemble->setEnabled(true);
}


void DebuggerWindow::SlotObjectsChanged()
{
    ioRegisterModel->SetMemory(memory);
    UpdateMemoryView();

    if (cpu != NULL)
        UpdateWidgets(cpu->GetFullPC());
}


void DebuggerWindow::SlotMemoryChanged(Address address, uint16_t len)
{
    // Memory has changed, so the disassembled opcodes are no longer valid.
    disassemblyModel->RemoveRows(address, len);

    // Only update memory table when debugging to avoid slowing things down.
    if (debuggingEnabled == true)
    {
        ioRegisterModel->InvalidateMemory(address, len);

        if (ui->cmbMemoryType->currentText() == "WRAM")
            memoryModel->InvalidateMemory(address, len);
    }
}


void DebuggerWindow::on_cmbMemoryType_currentTextChanged(const QString &text)
{
    (void)text;
    UpdateMemoryView();
}