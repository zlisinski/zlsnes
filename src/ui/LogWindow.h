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

    virtual void Output(std::unique_ptr<LogEntry> entry);

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    Ui::LogWindow *ui;
    std::vector<std::unique_ptr<LogEntry>> entries;
    QMutex entriesMutex;

private slots:
    void SlotOutputMessage();
    void SlotErrorClicked();
    void SlotWarningClicked();
    void SlotInfoClicked();
    void SlotDebugClicked();
    void SlotInstructionClicked();
    void SlotClearOutputClicked();
    void SlotSaveOutputClicked();

signals:
    void SignalLogWindowClosed();
    void SignalMessageReady();
};
