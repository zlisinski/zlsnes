#include <QtWidgets>
#include <stdint.h>
#include <thread>

#include "debugger/DebuggerWindow.h"
#include "InfoWindow.h"
#include "LogWindow.h"
#include "MainWindow.h"
#include "SettingsConstants.h"
#include "SettingsDialog.h"
#include "UiUtils.h"
#include "../core/Emulator.h"
//#include "../Input.h"
#include "../core/Logger.h"


MainWindow::MainWindow(const QString &romFilename, bool startInDebug, uint32_t runToAddress, bool saveToRecent, QWidget *parent) :
    QMainWindow(parent),
    graphicsView(NULL),
    labelFps(NULL),
    labelPause(NULL),
    paused(false),
    emulator(NULL),
    fpsTimer(),
    frameCount(0),
    frameCapTimer(),
    frameCapSetting(60),
#ifdef QT_GAMEPAD_LIB
    gamepad(NULL),
#endif
    infoWindow(NULL),
    displayInfoWindowAction(NULL),
    logWindow(NULL),
    displayLogWindowAction(NULL),
    debuggerWindow(NULL),
    displayDebuggerWindowAction(NULL),
    emuSaveStateAction(NULL),
    emuLoadStateAction(NULL)
    /*audioEnabled(true),
    audioOutput(NULL),
    audioBuffer(NULL),
    audioSampleRate(48000),
    enabledAudioChannels({true, true, true, true}),
    audioVolume(128)*/
{
    qRegisterMetaType<QVector<int>>("QVector<int>");

    QSettings settings;

    // Setup logger before anything else.
    Logger::SetLogLevel(static_cast<LogLevel>(settings.value(SETTINGS_LOGGER_LEVEL, 0).toInt()));
    Logger::SetInstructionLogLevel(settings.value(SETTINGS_LOGGER_INSTRUCTION_LEVEL, 0).toUInt());
    logWindow = new LogWindow(this);
    Logger::SetOutput(logWindow);
    if (settings.value(SETTINGS_LOGWINDOW_DISPLAY, false).toBool())
        logWindow->show();

    setWindowTitle("ZLSNES");

    SetupMenuBar();
    SetupStatusBar();
    SetupGamepad();
    //SetupAudio();

    LoadKeyBindings();

    graphicsView = new QGraphicsView(this);
    graphicsView->setFrameStyle(QFrame::NoFrame);
    setCentralWidget(graphicsView);

    QGraphicsScene *scene = new QGraphicsScene(this);
    graphicsView->setScene(scene);

    infoWindow = new InfoWindow(this);
    if (settings.value(SETTINGS_INFOWINDOW_DISPLAY, false).toBool())
        infoWindow->show();

    fpsTimer.start();
    frameCapTimer.start();

    restoreGeometry(settings.value(SETTINGS_MAINWINDOW_GEOMETRY).toByteArray());
    displayScale = settings.value(SETTINGS_VIDEO_SCALE, 1).toInt();
    SetDisplayScale(displayScale);

    connect(this, SIGNAL(SignalFrameReady()), this, SLOT(SlotDrawFrame()));
    connect(this, SIGNAL(SignalShowMessageBox(const QString&)), this, SLOT(SlotShowMessageBox(const QString &)));

    // If debugging or a run-to address was set on the command line, pass it on to the debugger.
    if (startInDebug)
    {
        debuggerWindow = new DebuggerWindow(this, true, runToAddress);
        debuggerWindow->show();
    }
    else
    {
        debuggerWindow = new DebuggerWindow(this);
        if (settings.value(SETTINGS_DEBUGGERWINDOW_DISPLAY, false).toBool())
            debuggerWindow->show();
    }

    emulator = new Emulator(this, /*this,*/ infoWindow, debuggerWindow/*, this*/);

    // Open the file if one was passed on the command line.
    if (romFilename != "")
    {
        OpenRom(romFilename, saveToRecent);
    }
}


MainWindow::~MainWindow()
{
    delete emulator;

    Logger::SetOutput(NULL);
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    Buttons::Button button = keyboardBindings.value(static_cast<Qt::Key>(event->key()), Buttons::Button::eButtonNone);
    if (button != Buttons::Button::eButtonNone)
    {
        emulator->ButtonPressed(button);
    }
    else
    {
        QMainWindow::keyPressEvent(event);
    }
}


void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    Buttons::Button button = keyboardBindings.value(static_cast<Qt::Key>(event->key()), Buttons::Button::eButtonNone);
    if (button != Buttons::Button::eButtonNone)
    {
        emulator->ButtonReleased(button);
    }
    else
    {
        QMainWindow::keyReleaseEvent(event);
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    infoWindow->close();
    logWindow->close();
    debuggerWindow->close();

    QSettings settings;
    settings.setValue(SETTINGS_MAINWINDOW_GEOMETRY, saveGeometry());
    QWidget::closeEvent(event);
}


void MainWindow::SetupMenuBar()
{
    QSettings settings;

    menuBar()->setNativeMenuBar(false);

    // File Menu
    QMenu *fileMenu = menuBar()->addMenu("&File");

    // File | Open...
    QAction *fileOpenAction = new QAction("&Open ROM...", this);
    fileMenu->addAction(fileOpenAction);
    connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(SlotOpenRom()));

    // File | Open Recent
    QMenu *fileOpenRecentMenu = fileMenu->addMenu("Open &Recent");
    for (int i = 0; i < MAX_RECENT_FILES; i++)
    {
        recentFilesActions[i] = new QAction(this);
        recentFilesActions[i]->setVisible(false);
        fileOpenRecentMenu->addAction(recentFilesActions[i]);
        connect(recentFilesActions[i], SIGNAL(triggered()), this, SLOT(SlotOpenRecentRom()));
    }
    UpdateRecentFilesActions();

    fileMenu->addSeparator();

    // File | Settings
    QAction *fileSettingsAction = new QAction("&Settings...", this);
    fileMenu->addAction(fileSettingsAction);
    connect(fileSettingsAction, SIGNAL(triggered()), this, SLOT(SlotOpenSettings()));

    // File | Quit
    fileMenu->addSeparator();
    QAction *fileQuitAction = new QAction("&Quit", this);
    fileMenu->addAction(fileQuitAction);
    connect(fileQuitAction, SIGNAL(triggered()), this, SLOT(SlotQuit()));

    ///////////////////////////////////////////////////////////////////////////

    // Emulator Menu
    QMenu *emuMenu = menuBar()->addMenu("&Emulator");

    // Emulator | Reset
    QAction *emuResetAction = new QAction("&Reset", this);
    emuMenu->addAction(emuResetAction);
    connect(emuResetAction, SIGNAL(triggered()), this, SLOT(SlotReset()));

    // Emulator | Pause
    QAction *emuPauseAction = new QAction("&Pause", this);
    emuPauseAction->setShortcut(Qt::Key_Escape);
    emuPauseAction->setCheckable(true);
    emuMenu->addAction(emuPauseAction);
    connect(emuPauseAction, SIGNAL(triggered(bool)), this, SLOT(SlotTogglePause(bool)));

    // Emulator | End
    QAction *emuEndAction = new QAction("&End Emulation", this);
    emuMenu->addAction(emuEndAction);
    connect(emuEndAction, SIGNAL(triggered()), this, SLOT(SlotEndEmulation()));

    // Emulator | Speed
    QMenu *emuSpeedMenu = emuMenu->addMenu("&Speed");
    QActionGroup *emuSpeedGroup = new QActionGroup(this);
    std::pair<std::string, int> speedVals[4] = {{"&Half", 30}, {"&Normal", 60}, {"&Double", 120}, {"&Uncapped", 0}};
    QAction *emuSpeedActions[4];
    for (int i = 0; i < 4; i++)
    {
        emuSpeedActions[i] = new QAction(speedVals[i].first.c_str(), this);
        emuSpeedActions[i]->setCheckable(true);
        emuSpeedActions[i]->setData(speedVals[i].second);
        if (speedVals[i].second == frameCapSetting)
            emuSpeedActions[i]->setChecked(true);
        emuSpeedMenu->addAction(emuSpeedActions[i]);
        emuSpeedGroup->addAction(emuSpeedActions[i]);
        connect(emuSpeedActions[i], SIGNAL(triggered()), this, SLOT(SlotSetFpsCap()));
    }

    // Emulator | Save State
    emuSaveStateAction = new QAction("&Save State", this);
    emuSaveStateAction->setShortcut(Qt::Key_F1);
    emuSaveStateAction->setEnabled(false);
    emuMenu->addAction(emuSaveStateAction);
    connect(emuSaveStateAction, SIGNAL(triggered()), this, SLOT(SlotSaveState()));

    // Emulator | Load State
    emuLoadStateAction = new QAction("&Load State", this);
    emuLoadStateAction->setShortcut(Qt::Key_F3);
    emuLoadStateAction->setEnabled(false);
    emuMenu->addAction(emuLoadStateAction);
    connect(emuLoadStateAction, SIGNAL(triggered()), this, SLOT(SlotLoadState()));

    ///////////////////////////////////////////////////////////////////////////

    // Display Menu
    QMenu *displayMenu = menuBar()->addMenu("&Display");

    // Display | Size
    QMenu *displaySizeMenu = displayMenu->addMenu("&Size");
    QActionGroup *displaySizeGroup = new QActionGroup(this);
    QAction *displaySizeActions[6];
    for (int i = 0; i < 6; i++)
    {
        displaySizeActions[i] = new QAction(QString("&%1x").arg(i+1), this);
        displaySizeActions[i]->setCheckable(true);
        displaySizeActions[i]->setData(i+1);
        if (i+1 == displayScale)
            displaySizeActions[i]->setChecked(true);
        displaySizeMenu->addAction(displaySizeActions[i]);
        displaySizeGroup->addAction(displaySizeActions[i]);
        connect(displaySizeActions[i], SIGNAL(triggered()), this, SLOT(SlotSetDisplayScale()));
    }

    // Display | Info Window
    displayInfoWindowAction = displayMenu->addAction("&Info Window");
    displayInfoWindowAction->setCheckable(true);
    displayInfoWindowAction->setChecked(settings.value(SETTINGS_INFOWINDOW_DISPLAY, false).toBool());
    connect(displayInfoWindowAction, SIGNAL(triggered(bool)), this, SLOT(SlotSetDisplayInfoWindow(bool)));

    // Display | Log Window
    displayLogWindowAction = displayMenu->addAction("&Log Window");
    displayLogWindowAction->setCheckable(true);
    displayLogWindowAction->setChecked(settings.value(SETTINGS_LOGWINDOW_DISPLAY, false).toBool());
    connect(displayLogWindowAction, SIGNAL(triggered(bool)), this, SLOT(SlotSetDisplayLogWindow(bool)));

    // Display | Debugger Window
    displayDebuggerWindowAction = displayMenu->addAction("&Debugger Window");
    displayDebuggerWindowAction->setCheckable(true);
    displayDebuggerWindowAction->setChecked(settings.value(SETTINGS_DEBUGGERWINDOW_DISPLAY, false).toBool());
    connect(displayDebuggerWindowAction, SIGNAL(triggered(bool)), this, SLOT(SlotSetDisplayDebuggerWindow(bool)));
}


void MainWindow::SetupStatusBar()
{
    labelFps = new QLabel("0 FPS", this);
    labelPause = new QLabel("", this);
    statusBar()->addPermanentWidget(labelFps);
    statusBar()->addPermanentWidget(labelPause);
}


void MainWindow::SetupGamepad()
{
#ifdef QT_GAMEPAD_LIB
    QList<int> gamepads = QGamepadManager::instance()->connectedGamepads();
    if (gamepads.isEmpty())
    {
        return;
    }

    gamepad = new QGamepad(*gamepads.begin(), this);

    connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonPressEvent, this, &MainWindow::SlotGamepadPressed);
    connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonReleaseEvent, this, &MainWindow::SlotGamepadReleased);
#endif
}


/*void MainWindow::SetupAudio()
{
    LoadAudioSettings();

    QAudioFormat format;
    format.setSampleRate(audioSampleRate);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format))
    {
        UiUtils::MessageBox("Audio format not supported");
        return;
    }

    audioOutput = new QAudioOutput(format, this);
    audioOutput->setBufferSize(0x4000);
    //audioOutput->setVolume(0.4); // This is broken with pulseaudio
    connect(audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(SlotAudioStateChanged(QAudio::State)));

    audioBuffer = audioOutput->start();
}


void MainWindow::LoadAudioSettings()
{
    QSettings settings;
    audioEnabled = settings.value(SETTINGS_AUDIO_ENABLED, true).toBool();
    audioVolume = settings.value(SETTINGS_AUDIO_VOLUME, 128).toInt();
    enabledAudioChannels.channel1 = settings.value(SETTINGS_AUDIO_CHANNEL1, true).toBool();
    enabledAudioChannels.channel2 = settings.value(SETTINGS_AUDIO_CHANNEL2, true).toBool();
    enabledAudioChannels.channel3 = settings.value(SETTINGS_AUDIO_CHANNEL3, true).toBool();
    enabledAudioChannels.channel4 = settings.value(SETTINGS_AUDIO_CHANNEL4, true).toBool();
}*/


void MainWindow::LoadKeyBindings()
{
    QSettings settings;
    keyboardBindings = {
        {static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_UP, Qt::Key_W).toInt()), Buttons::eButtonUp},
        {static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_DOWN, Qt::Key_S).toInt()), Buttons::eButtonDown},
        {static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_LEFT, Qt::Key_A).toInt()), Buttons::eButtonLeft},
        {static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_RIGHT, Qt::Key_D).toInt()), Buttons::eButtonRight},
        {static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_B, Qt::Key_K).toInt()), Buttons::eButtonB},
        {static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_A, Qt::Key_L).toInt()), Buttons::eButtonA},
        {static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_Y, Qt::Key_I).toInt()), Buttons::eButtonY},
        {static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_X, Qt::Key_O).toInt()), Buttons::eButtonX},
        {static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_L, Qt::Key_U).toInt()), Buttons::eButtonL},
        {static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_R, Qt::Key_P).toInt()), Buttons::eButtonR},
        {static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_START, Qt::Key_J).toInt()), Buttons::eButtonStart},
        {static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_SELECT, Qt::Key_H).toInt()), Buttons::eButtonSelect}
    };
#ifdef QT_GAMEPAD_LIB
    gamepadBindings = {
        {static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_UP, QGamepadManager::ButtonInvalid).toInt()), Buttons::eButtonUp},
        {static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_DOWN, QGamepadManager::ButtonInvalid).toInt()), Buttons::eButtonDown},
        {static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_LEFT, QGamepadManager::ButtonInvalid).toInt()), Buttons::eButtonLeft},
        {static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_RIGHT, QGamepadManager::ButtonInvalid).toInt()), Buttons::eButtonRight},
        {static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_B, QGamepadManager::ButtonInvalid).toInt()), Buttons::eButtonB},
        {static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_A, QGamepadManager::ButtonInvalid).toInt()), Buttons::eButtonA},
        {static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_Y, QGamepadManager::ButtonInvalid).toInt()), Buttons::eButtonY},
        {static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_X, QGamepadManager::ButtonInvalid).toInt()), Buttons::eButtonX},
        {static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_L, QGamepadManager::ButtonInvalid).toInt()), Buttons::eButtonL},
        {static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_R, QGamepadManager::ButtonInvalid).toInt()), Buttons::eButtonR},
        {static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_START, QGamepadManager::ButtonInvalid).toInt()), Buttons::eButtonStart},
        {static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_SELECT, QGamepadManager::ButtonInvalid).toInt()), Buttons::eButtonSelect}
    };
#endif
}


void MainWindow::UpdateRecentFile(const QString &filename)
{
    QSettings settings;
    QStringList files = settings.value(SETTINGS_FILES_RECENTFILELIST).toStringList();

    files.removeAll(filename);
    files.prepend(filename);
    while (files.size() > MAX_RECENT_FILES)
        files.removeLast();

    settings.setValue(SETTINGS_FILES_RECENTFILELIST, files);

    UpdateRecentFilesActions();
}


void MainWindow::UpdateRecentFilesActions()
{
    QSettings settings;
    QStringList files = settings.value(SETTINGS_FILES_RECENTFILELIST).toStringList();

    int numRecentFiles = qMin(files.size(), MAX_RECENT_FILES);

    for (int i = 0; i < numRecentFiles; i++) {
        QString text = tr("&%1 %2").arg(i).arg(files[i]);
        recentFilesActions[i]->setText(text);
        recentFilesActions[i]->setData(files[i]);
        recentFilesActions[i]->setVisible(true);
    }
    for (int i = numRecentFiles; i < MAX_RECENT_FILES; i++)
        recentFilesActions[i]->setVisible(false);
}


void MainWindow::SetDisplayScale(int scale)
{
    graphicsView->scene()->clear();
    graphicsView->setSceneRect(0, 0, SCREEN_X*scale, SCREEN_Y*scale);
    graphicsView->setFixedSize(SCREEN_X*scale, SCREEN_Y*scale);
    adjustSize();
    displayScale = scale;
}


void MainWindow::OpenRom(const QString &filename, bool saveToRecent)
{
    if (filename != "")
    {
        statusBar()->showMessage("Filename = " + filename, 5000);

        if (saveToRecent)
            UpdateRecentFile(filename);

        if (!emulator->LoadRom(filename.toLatin1().data()))
        {
            LogError("Error loading ROM");
            infoWindow->ClearWidgets();
            return;
        }

        setWindowTitle("ZLSNES - " + filename);

        emuSaveStateAction->setEnabled(true);
        emuLoadStateAction->setEnabled(true);

        infoWindow->DrawFrame();
    }
}


void MainWindow::FrameReady(const std::array<uint32_t, SCREEN_X * SCREEN_Y> &displayFrameBuffer)
{
    // This function runs in the thread context of the Emulator worker thread.

    // Copy data so Emulator thread doesn't change data while we're drawing the screen.
    //memcpy(frameBuffer, displayFrameBuffer, sizeof(frameBuffer));
    frameBuffer = displayFrameBuffer;

    // Signal the main thread to draw the screen.
    emit SignalFrameReady();

    int64_t elapsedTime = frameCapTimer.elapsed();

    if (frameCapSetting > 0)
    {
        const float frameMillis = 1.0 / frameCapSetting * 1000;

        if (elapsedTime < frameMillis)
        {
            // Block the Emulator from running to limit frame rate.
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(frameMillis - elapsedTime)));
        }
    }

    //NotifyGameSpeedObservers(frameCapTimer.elapsed());

    frameCapTimer.restart();
}


void MainWindow::RequestMessageBox(const std::string &message)
{
    // This function runs in the thread context of the Emulator worker thread.
    // Signal the main thread to show a message box.
    emit SignalShowMessageBox(QString::fromStdString(message));
}


/*void MainWindow::AudioDataReady(const std::array<int16_t, AudioInterface::BUFFER_LEN> &data)
{
    // This function runs in the thread context of the Emulator worker thread.

    if (audioBuffer != NULL)
    {
        qint64 written = 0;
        //do {
            size_t dataSize = data.size() * sizeof(data[0]);
            written += audioBuffer->write(reinterpret_cast<const char *>(&data[written]), dataSize - written);
            if (written != AudioInterface::BUFFER_LEN)
                LogAudio("Only wrote %lld bytes of audio data", written);
        //} while (written < AudioInterface::BUFFER_LEN);
    }
}*/


void MainWindow::SlotOpenRom()
{
    QSettings settings;
    QString dir = settings.value(SETTINGS_FILES_OPENROMDIR).toString();

    QString filename = QFileDialog::getOpenFileName(this, "Open ROM File", dir);

    if (filename != "")
    {
        QFileInfo info(filename);
        settings.setValue(SETTINGS_FILES_OPENROMDIR, info.absoluteDir().path());
    }

    OpenRom(filename);
}


void MainWindow::SlotOpenRecentRom()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        QString filename = action->data().toString();
        OpenRom(filename);
    }
}


void MainWindow::SlotReset()
{
    emulator->ResetEmulation();
}


void MainWindow::SlotTogglePause(bool checked)
{
    paused = checked;
    labelPause->setText(checked ? "Paused" : "");
    emulator->PauseEmulation(paused);
}


void MainWindow::SlotEndEmulation()
{
    emulator->EndEmulation();
    graphicsView->scene()->clear();

    emuSaveStateAction->setEnabled(false);
    emuLoadStateAction->setEnabled(false);
}


void MainWindow::SlotSetFpsCap()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        frameCapSetting = action->data().toInt();
    }
}


void MainWindow::SlotQuit()
{
    emulator->EndEmulation();
    qApp->quit();
}


void MainWindow::SlotDrawFrame()
{
    uint64_t elapsedTime = fpsTimer.elapsed();

    if (elapsedTime > 1000)
    {
        int fps = frameCount / (elapsedTime / 1000.0);
        labelFps->setText(QString::number(fps) + " FPS");
        fpsTimer.restart();
        frameCount = 0;
    }
    else
    {
        frameCount++;
    }

    QImage img((uchar *)(&frameBuffer[0]), SCREEN_X, SCREEN_Y, QImage::Format_ARGB32);
    graphicsView->scene()->clear();
    QGraphicsPixmapItem *pixmap = graphicsView->scene()->addPixmap(QPixmap::fromImage(img));
    pixmap->setScale(displayScale);

    // Only update infoWindow 60 time a second, this stops the program locking up when frame cap is off.
    if ((elapsedTime & 0x0F) == 0)
        infoWindow->DrawFrame();
}


void MainWindow::SlotShowMessageBox(const QString &message)
{
    UiUtils::MessageBox(message);
}


void MainWindow::SlotSetDisplayScale()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        int scale = action->data().toInt();

        SetDisplayScale(scale);

        if (paused)
            SignalFrameReady();

        QSettings settings;
        settings.setValue(SETTINGS_VIDEO_SCALE, scale);
    }
}


void MainWindow::SlotSetDisplayInfoWindow(bool checked)
{
    QSettings settings;
    settings.setValue(SETTINGS_INFOWINDOW_DISPLAY, checked);

    if (checked)
        infoWindow->show();
    else
        infoWindow->hide();
}


void MainWindow::SlotInfoWindowClosed()
{
    displayInfoWindowAction->setChecked(false);
}


void MainWindow::SlotSetDisplayLogWindow(bool checked)
{
    QSettings settings;
    settings.setValue(SETTINGS_LOGWINDOW_DISPLAY, checked);

    if (checked)
        logWindow->show();
    else
        logWindow->hide();
}


void MainWindow::SlotLogWindowClosed()
{
    displayLogWindowAction->setChecked(false);
}


void MainWindow::SlotSetDisplayDebuggerWindow(bool checked)
{
    QSettings settings;
    settings.setValue(SETTINGS_DEBUGGERWINDOW_DISPLAY, checked);

    if (checked)
        debuggerWindow->show();
    else
        debuggerWindow->hide();
}


void MainWindow::SlotDebuggerWindowClosed()
{
    displayDebuggerWindowAction->setChecked(false);
}


void MainWindow::SlotSaveState()
{
    emulator->SaveState(1);
}


void MainWindow::SlotLoadState()
{
    emulator->LoadState(1);
}


void MainWindow::SlotOpenSettings()
{
    SettingsDialog dialog(this);
    dialog.setModal(true);
    dialog.exec();

    //LoadAudioSettings();
    LoadKeyBindings();
}


/*void MainWindow::SlotAudioStateChanged(QAudio::State state)
{
    switch (state)
    {
        case QAudio::ActiveState:
            LogAudio("Audio active");
            break;
        case QAudio::IdleState:
            LogAudio("Audio idle");
            break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        // This doesn't exist in older Qt versions.
        case QAudio::InterruptedState:
            LogError("Audio interrupted");
            break;
#endif
        case QAudio::StoppedState:
            if (audioOutput->error() != QAudio::NoError)
                LogError("Audio error %d", audioOutput->error());
            else
                LogError("Audio Stopped");
            break;
        case QAudio::SuspendedState:
            LogError("Audio suspended");
            break;
    }
}*/

#ifdef QT_GAMEPAD_LIB
void MainWindow::SlotGamepadPressed(int deviceId, QGamepadManager::GamepadButton gamepadButton, double value)
{
    Q_UNUSED(deviceId);
    Q_UNUSED(value);

    Buttons::Button button = gamepadBindings.value(gamepadButton, Buttons::Button::eButtonNone);
    if (button != Buttons::Button::eButtonNone)
    {
        emulator->ButtonPressed(button);
    }
}


void MainWindow::SlotGamepadReleased(int deviceId, QGamepadManager::GamepadButton gamepadButton)
{
    Q_UNUSED(deviceId);

    Buttons::Button button = gamepadBindings.value(gamepadButton, Buttons::Button::eButtonNone);
    if (button != Buttons::Button::eButtonNone)
    {
        emulator->ButtonReleased(button);
    }
}
#endif