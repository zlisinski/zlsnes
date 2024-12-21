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
    QSettings settings;
    uint32_t newLevel = Logger::GetInstructionLogLevel();
    if (checked)
        newLevel |= static_cast<uint32_t>(InstructionLogLevel::eCpu);
    else
        newLevel &= ~static_cast<uint32_t>(InstructionLogLevel::eCpu);

    settings.setValue(SETTINGS_LOGGER_INSTRUCTION_LEVEL, newLevel);
    Logger::SetInstructionLogLevel(newLevel);
}


void LogWindow::on_chkPpu_clicked(bool checked)
{
    QSettings settings;
    uint32_t newLevel = Logger::GetInstructionLogLevel();
    if (checked)
        newLevel |= static_cast<uint32_t>(InstructionLogLevel::ePpu);
    else
        newLevel &= ~static_cast<uint32_t>(InstructionLogLevel::ePpu);

    settings.setValue(SETTINGS_LOGGER_INSTRUCTION_LEVEL, newLevel);
    Logger::SetInstructionLogLevel(newLevel);
}


void LogWindow::on_chkMemory_clicked(bool checked)
{
    QSettings settings;
    uint32_t newLevel = Logger::GetInstructionLogLevel();
    if (checked)
        newLevel |= static_cast<uint32_t>(InstructionLogLevel::eMemory);
    else
        newLevel &= ~static_cast<uint32_t>(InstructionLogLevel::eMemory);

    settings.setValue(SETTINGS_LOGGER_INSTRUCTION_LEVEL, newLevel);
    Logger::SetInstructionLogLevel(newLevel);
}


void LogWindow::on_chkInput_clicked(bool checked)
{
    QSettings settings;
    uint32_t newLevel = Logger::GetInstructionLogLevel();
    if (checked)
        newLevel |= static_cast<uint32_t>(InstructionLogLevel::eInput);
    else
        newLevel &= ~static_cast<uint32_t>(InstructionLogLevel::eInput);

    settings.setValue(SETTINGS_LOGGER_INSTRUCTION_LEVEL, newLevel);
    Logger::SetInstructionLogLevel(newLevel);
}


void LogWindow::on_chkTimer_clicked(bool checked)
{
    QSettings settings;
    uint32_t newLevel = Logger::GetInstructionLogLevel();
    if (checked)
        newLevel |= static_cast<uint32_t>(InstructionLogLevel::eTimer);
    else
        newLevel &= ~static_cast<uint32_t>(InstructionLogLevel::eTimer);

    settings.setValue(SETTINGS_LOGGER_INSTRUCTION_LEVEL, newLevel);
    Logger::SetInstructionLogLevel(newLevel);
}


void LogWindow::on_chkInterrupt_clicked(bool checked)
{
    QSettings settings;
    uint32_t newLevel = Logger::GetInstructionLogLevel();
    if (checked)
        newLevel |= static_cast<uint32_t>(InstructionLogLevel::eInterrupt);
    else
        newLevel &= ~static_cast<uint32_t>(InstructionLogLevel::eInterrupt);

    settings.setValue(SETTINGS_LOGGER_INSTRUCTION_LEVEL, newLevel);
    Logger::SetInstructionLogLevel(newLevel);
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