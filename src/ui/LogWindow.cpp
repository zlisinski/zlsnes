#include <iomanip>
#include <sstream>
#include <QSettings>
#include <QtWidgets>

#include "LogWindow.h"
#include "SettingsConstants.h"
#include "ui_LogWindow.h"
#include "UiUtils.h"


LogWindow::LogWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogWindow)
{
    ui->setupUi(this);

    QSettings settings;
    restoreGeometry(settings.value(SETTINGS_LOGWINDOW_GEOMETRY).toByteArray());

    SetInstructionCheckboxEnabled(false);

    switch (Logger::GetLogLevel())
    {
        case LogLevel::eError:
            ui->rbError->setChecked(true);
            break;
        case LogLevel::eWarning:
            ui->rbWarning->setChecked(true);
            break;
        case LogLevel::eInfo:
            ui->rbInfo->setChecked(true);
            break;
        case LogLevel::eDebug:
            ui->rbDebug->setChecked(true);
            break;
        case LogLevel::eInstruction:
            ui->rbInstruction->setChecked(true);
            SetInstructionCheckboxEnabled(true);
            break;
    }

    uint32_t instLevel = Logger::GetInstructionLogLevel();
    ui->chkCpu->setChecked(instLevel & static_cast<uint32_t>(InstructionLogLevel::eCpu));
    ui->chkPpu->setChecked(instLevel & static_cast<uint32_t>(InstructionLogLevel::ePpu));
    ui->chkMemory->setChecked(instLevel & static_cast<uint32_t>(InstructionLogLevel::eMemory));
    ui->chkInput->setChecked(instLevel & static_cast<uint32_t>(InstructionLogLevel::eInput));
    ui->chkTimer->setChecked(instLevel & static_cast<uint32_t>(InstructionLogLevel::eTimer));
    ui->chkInterrupt->setChecked(instLevel & static_cast<uint32_t>(InstructionLogLevel::eInterrupt));
    ui->chkApu->setChecked(instLevel & static_cast<uint32_t>(InstructionLogLevel::eApu));
    ui->chkDma->setChecked(instLevel & static_cast<uint32_t>(InstructionLogLevel::eDma));
    ui->chkSpc700->setChecked(instLevel & static_cast<uint32_t>(InstructionLogLevel::eSpc700));
    ui->chkSpcMem->setChecked(instLevel & static_cast<uint32_t>(InstructionLogLevel::eSpcMem));
    ui->chkSpcTimer->setChecked(instLevel & static_cast<uint32_t>(InstructionLogLevel::eSpcTimer));

    connect(this, SIGNAL(SignalLogWindowClosed()), parent, SLOT(SlotLogWindowClosed()));
    connect(this, SIGNAL(SignalMessageReady()), this, SLOT(SlotOutputMessage()));
}


LogWindow::~LogWindow()
{
    delete ui;
}


void LogWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue(SETTINGS_LOGWINDOW_GEOMETRY, saveGeometry());

    emit SignalLogWindowClosed();

    QWidget::closeEvent(event);
}


void LogWindow::Output(std::unique_ptr<LogEntry> entry)
{
    // // This function runs in the thread context of the Emulator worker thread, or whoever called Log().
    QMutexLocker lock(&entriesMutex);

    entries.push_back(std::move(entry));

    lock.unlock();

    // Signal the main thread to add the text.
    emit SignalMessageReady();
}


void LogWindow::SlotOutputMessage()
{
    QMutexLocker lock(&entriesMutex);

    for (const auto &entry : entries)
    {
        std::stringstream ss;
        char timeBuf[9];
        tm *now = localtime(&entry->tv.tv_sec);
        strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", now);
        ss << timeBuf << "." << std::setfill('0') << std::setw(6) << entry->tv.tv_usec << ":  " << entry->message;
        //if (entry->level == LogLevel::eInstruction)
            printf("%s\n", ss.str().c_str());
        //else
        //    ui->txtOutput->append(ss.str().c_str());
    }

    entries.clear();
}


void LogWindow::SetInstructionCheckboxEnabled(bool enable)
{
    ui->chkCpu->setEnabled(enable);
    ui->chkPpu->setEnabled(enable);
    ui->chkMemory->setEnabled(enable);
    ui->chkInput->setEnabled(enable);
    ui->chkTimer->setEnabled(enable);
    ui->chkInterrupt->setEnabled(enable);
    ui->chkApu->setEnabled(enable);
    ui->chkDma->setEnabled(enable);
    ui->chkSpc700->setEnabled(enable);
    ui->chkSpcMem->setEnabled(enable);
    ui->chkSpcTimer->setEnabled(enable);
}


void LogWindow::SetInstructionLevel(InstructionLogLevel subsystem, bool checked)
{
    QSettings settings;
    uint32_t newLevel = Logger::GetInstructionLogLevel();
    if (checked)
        newLevel |= static_cast<uint32_t>(subsystem);
    else
        newLevel &= ~static_cast<uint32_t>(subsystem);

    settings.setValue(SETTINGS_LOGGER_INSTRUCTION_LEVEL, newLevel);
    Logger::SetInstructionLogLevel(newLevel);
}


void LogWindow::on_rbError_clicked()
{
    QSettings settings;
    settings.setValue(SETTINGS_LOGGER_LEVEL, static_cast<int>(LogLevel::eError));
    Logger::SetLogLevel(LogLevel::eError);
    SetInstructionCheckboxEnabled(false);
}


void LogWindow::on_rbWarning_clicked()
{
    QSettings settings;
    settings.setValue(SETTINGS_LOGGER_LEVEL, static_cast<int>(LogLevel::eWarning));
    Logger::SetLogLevel(LogLevel::eWarning);
    SetInstructionCheckboxEnabled(false);
}


void LogWindow::on_rbInfo_clicked()
{
    QSettings settings;
    settings.setValue(SETTINGS_LOGGER_LEVEL, static_cast<int>(LogLevel::eInfo));
    Logger::SetLogLevel(LogLevel::eInfo);
    SetInstructionCheckboxEnabled(false);
}


void LogWindow::on_rbDebug_clicked()
{
    QSettings settings;
    settings.setValue(SETTINGS_LOGGER_LEVEL, static_cast<int>(LogLevel::eDebug));
    Logger::SetLogLevel(LogLevel::eDebug);
    SetInstructionCheckboxEnabled(false);
}


void LogWindow::on_rbInstruction_clicked()
{
    QSettings settings;
    settings.setValue(SETTINGS_LOGGER_LEVEL, static_cast<int>(LogLevel::eInstruction));
    Logger::SetLogLevel(LogLevel::eInstruction);
    SetInstructionCheckboxEnabled(true);
}


void LogWindow::on_chkCpu_clicked(bool checked)
{
    SetInstructionLevel(InstructionLogLevel::eCpu, checked);
}


void LogWindow::on_chkPpu_clicked(bool checked)
{
    SetInstructionLevel(InstructionLogLevel::ePpu, checked);
}


void LogWindow::on_chkMemory_clicked(bool checked)
{
    SetInstructionLevel(InstructionLogLevel::eMemory, checked);
}


void LogWindow::on_chkInput_clicked(bool checked)
{
    SetInstructionLevel(InstructionLogLevel::eInput, checked);
}


void LogWindow::on_chkTimer_clicked(bool checked)
{
    SetInstructionLevel(InstructionLogLevel::eTimer, checked);
}


void LogWindow::on_chkInterrupt_clicked(bool checked)
{
    SetInstructionLevel(InstructionLogLevel::eInterrupt, checked);
}


void LogWindow::on_chkApu_clicked(bool checked)
{
    SetInstructionLevel(InstructionLogLevel::eApu, checked);
}


void LogWindow::on_chkDma_clicked(bool checked)
{
    SetInstructionLevel(InstructionLogLevel::eDma, checked);
}


void LogWindow::on_chkSpc700_clicked(bool checked)
{
    SetInstructionLevel(InstructionLogLevel::eSpc700, checked);
}


void LogWindow::on_chkSpcMem_clicked(bool checked)
{
    SetInstructionLevel(InstructionLogLevel::eSpcMem, checked);
}


void LogWindow::on_chkSpcTimer_clicked(bool checked)
{
    SetInstructionLevel(InstructionLogLevel::eSpcTimer, checked);
}


void LogWindow::on_btnClear_clicked()
{
    ui->txtOutput->clear();
}


void LogWindow::on_btnSave_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save log output");
    if (filename == "")
        return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        UiUtils::MessageBox("Error opening " + filename);
        return;
    }

    file.write(ui->txtOutput->toPlainText().toLatin1().data());
}