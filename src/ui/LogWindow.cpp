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
            break;
    }

    connect(this, SIGNAL(SignalLogWindowClosed()), parent, SLOT(SlotLogWindowClosed()));
    connect(this, SIGNAL(SignalMessageReady()), this, SLOT(SlotOutputMessage()));
    connect(ui->rbError, SIGNAL(clicked()), this, SLOT(SlotErrorClicked()));
    connect(ui->rbWarning, SIGNAL(clicked()), this, SLOT(SlotWarningClicked()));
    connect(ui->rbInfo, SIGNAL(clicked()), this, SLOT(SlotInfoClicked()));
    connect(ui->rbDebug, SIGNAL(clicked()), this, SLOT(SlotDebugClicked()));
    connect(ui->rbInstruction, SIGNAL(clicked()), this, SLOT(SlotInstructionClicked()));
    connect(ui->btnClear, SIGNAL(clicked()), this, SLOT(SlotClearOutputClicked()));
    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(SlotSaveOutputClicked()));
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
        if (entry->level == LogLevel::eInstruction)
            printf("%s\n", ss.str().c_str());
        else
            ui->txtOutput->append(ss.str().c_str());
    }

    entries.clear();
}


void LogWindow::SlotErrorClicked()
{
    QSettings settings;
    settings.setValue(SETTINGS_LOGGER_LEVEL, static_cast<int>(LogLevel::eError));
    Logger::SetLogLevel(LogLevel::eError);
}


void LogWindow::SlotWarningClicked()
{
    QSettings settings;
    settings.setValue(SETTINGS_LOGGER_LEVEL, static_cast<int>(LogLevel::eWarning));
    Logger::SetLogLevel(LogLevel::eWarning);
}


void LogWindow::SlotInfoClicked()
{
    QSettings settings;
    settings.setValue(SETTINGS_LOGGER_LEVEL, static_cast<int>(LogLevel::eInfo));
    Logger::SetLogLevel(LogLevel::eInfo);
}


void LogWindow::SlotDebugClicked()
{
    QSettings settings;
    settings.setValue(SETTINGS_LOGGER_LEVEL, static_cast<int>(LogLevel::eDebug));
    Logger::SetLogLevel(LogLevel::eDebug);
}


void LogWindow::SlotInstructionClicked()
{
    QSettings settings;
    settings.setValue(SETTINGS_LOGGER_LEVEL, static_cast<int>(LogLevel::eInstruction));
    Logger::SetLogLevel(LogLevel::eInstruction);
}


void LogWindow::SlotClearOutputClicked()
{
    ui->txtOutput->clear();
}


void LogWindow::SlotSaveOutputClicked()
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