#pragma once

#include <array>
#include <QElapsedTimer>
#ifdef QT_GAMEPAD_LIB
#include <QtGamepad>
#endif
//#include <QAudioOutput>
#include <QGraphicsView>
#include <QMainWindow>

//#include "../core/AudioInterface.h"
#include "../core/Buttons.h"
#include "../core/DisplayInterface.h"
//#include "../core/GameSpeedObserver.h"

class DebuggerWindow;
class InfoWindow;
class Emulator;
class LogWindow;
class QLabel;

const int MAX_RECENT_FILES = 20;


class MainWindow : public QMainWindow, public DisplayInterface//, public AudioInterface, public GameSpeedSubject
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &romFilename, bool startInDebug, uint32_t runToAddress, bool saveToRecent, QWidget *parent = 0);
    ~MainWindow();

    // DisplayInterface functions.
    // Callback for Emulator to signal a frame is ready to be drawn.
    void FrameReady(const std::array<uint32_t, SCREEN_X * SCREEN_Y> &displayFrameBuffer) override;
    // Callback for Emulator to show message box.
    void RequestMessageBox(const std::string &message) override;

    // AudioInterface functions.
    /*virtual void AudioDataReady(const std::array<int16_t, AudioInterface::BUFFER_LEN> &data);
    virtual int GetAudioSampleRate() {return audioSampleRate;}
    virtual bool GetAudioEnabled() {return audioEnabled;}
    virtual AudioInterface::Channels GetEnabledAudioChannels() {return enabledAudioChannels;}
    virtual uint8_t GetAudioVolume() {return audioVolume;}
    virtual int GetGameSpeed() {return frameCapSetting;}*/

    // Don't allow copy and assignment.
    MainWindow(const MainWindow&) = delete;
    void operator=(const MainWindow&) = delete;

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    void SetupMenuBar();
    void SetupStatusBar();
    void SetupGamepad();
    //void SetupAudio();
    //void LoadAudioSettings();
    void LoadKeyBindings();
    void UpdateRecentFile(const QString &filename);
    void UpdateRecentFilesActions();
    void SetDisplayScale(int scale);
    void OpenRom(const QString &filename, bool saveToRecent = true);

    QGraphicsView *graphicsView;
    QLabel *labelFps;
    QLabel *labelPause;

    bool paused;

    Emulator *emulator;

    // FPS variables.
    QElapsedTimer fpsTimer;
    int frameCount;

    // Frame cap variables.
    QElapsedTimer frameCapTimer;
    int frameCapSetting;

    QHash<Qt::Key, Buttons::Button> keyboardBindings;
#ifdef QT_GAMEPAD_LIB
    QHash<QGamepadManager::GamepadButton, Buttons::Button> gamepadBindings;
    QGamepad *gamepad;
#endif

    std::array<uint32_t, SCREEN_X * SCREEN_Y> frameBuffer;
    int displayScale;

    InfoWindow *infoWindow;
    QAction *displayInfoWindowAction;
    LogWindow *logWindow;
    QAction *displayLogWindowAction;
    DebuggerWindow *debuggerWindow;
    QAction *displayDebuggerWindowAction;

    QAction *emuSaveStateAction;
    QAction *emuLoadStateAction;

    QAction *recentFilesActions[MAX_RECENT_FILES];

    /*bool audioEnabled;
    QAudioOutput *audioOutput;
    QIODevice *audioBuffer;
    int audioSampleRate;
    AudioInterface::Channels enabledAudioChannels;
    uint8_t audioVolume;*/

private slots:
    void SlotOpenRom();
    void SlotOpenRecentRom();
    void SlotReset();
    void SlotTogglePause(bool checked);
    void SlotEndEmulation();
    void SlotSetFpsCap();
    void SlotQuit();
    void SlotDrawFrame();
    void SlotShowMessageBox(const QString &message);
    void SlotSetDisplayScale();
    void SlotSetDisplayInfoWindow(bool checked);
    void SlotInfoWindowClosed();
    void SlotSetDisplayLogWindow(bool checked);
    void SlotLogWindowClosed();
    void SlotSetDisplayDebuggerWindow(bool checked);
    void SlotDebuggerWindowClosed();
    void SlotSaveState();
    void SlotLoadState();
    void SlotOpenSettings();
    //void SlotAudioStateChanged(QAudio::State state);
#ifdef QT_GAMEPAD_LIB
    void SlotGamepadPressed(int deviceId, QGamepadManager::GamepadButton gamepadButton, double value);
    void SlotGamepadReleased(int deviceId, QGamepadManager::GamepadButton gamepadButton);
#endif

signals:
    void SignalFrameReady();
    void SignalShowMessageBox(const QString &message);
};
