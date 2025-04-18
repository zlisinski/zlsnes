#pragma once

#include <memory>
#include <QDialog>
#include <QMutex>
#include <vector>

#include "../core/Logger.h"

namespace Ui {
class LogWindow;
}

class LogWindow : public QDialog, public LoggerOutput
{
    Q_OBJECT

public:
    explicit LogWindow(QWidget *parent = 0);
    ~LogWindow();

    // Inherited from LoggerOutput.
    void Output(std::unique_ptr<LogEntry> entry) override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void SetInstructionCheckboxEnabled(bool enable);
    void SetInstructionLevel(InstructionLogLevel subsystem, bool checked);

private:
    Ui::LogWindow *ui;
    std::vector<std::unique_ptr<LogEntry>> entries;
    QMutex entriesMutex;

private slots:
    void SlotOutputMessage();

    // Auto connect signal/slot based on function name.
    void on_rbError_clicked();
    void on_rbWarning_clicked();
    void on_rbInfo_clicked();
    void on_rbDebug_clicked();
    void on_rbInstruction_clicked();
    void on_chkCpu_clicked(bool checked);
    void on_chkPpu_clicked(bool checked);
    void on_chkMemory_clicked(bool checked);
    void on_chkInput_clicked(bool checked);
    void on_chkTimer_clicked(bool checked);
    void on_chkInterrupt_clicked(bool checked);
    void on_chkApu_clicked(bool checked);
    void on_chkDma_clicked(bool checked);
    void on_chkSpc700_clicked(bool checked);
    void on_chkSpcMem_clicked(bool checked);
    void on_chkSpcTimer_clicked(bool checked);
    void on_btnClear_clicked();
    void on_btnSave_clicked();

signals:
    void SignalLogWindowClosed();
    void SignalMessageReady();
};
