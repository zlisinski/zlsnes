#pragma once

#ifdef QT_GAMEPAD_LIB
#include <QtGamepad/QGamepadManager>
#endif
#include <QtWidgets/QDialog>

#include "core/Buttons.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    virtual void accept();

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    void SaveSettings();
    QString GetKeyString(Qt::Key keycode);
    void UpdateKeyBindingButtonText();

    Ui::SettingsDialog *ui;

    QVector<QPushButton*> keyButtons;
    QVector<QPushButton*> padButtons;
    QHash<Buttons::Button, Qt::Key> keyBindings;
#ifdef QT_GAMEPAD_LIB
    QHash<Buttons::Button, QGamepadManager::GamepadButton> padBindings;
#endif

    bool dirty;

private slots:
    void SlotToggleEnableAudio(bool checked);
    void SlotAudioVolumeChanged(int value);
    void SlotInputBindKey();
    void SlotInputBindPad();
};