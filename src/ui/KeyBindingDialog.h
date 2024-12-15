#pragma once

#include <QtWidgets/QDialog>
#ifdef QT_GAMEPAD_LIB
#include <QtGamepad/QGamepadManager>
#endif

#include "core/Buttons.h"

namespace Ui {
class KeyBindingDialog;
}

class QGamepadManager;

class KeyBindingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeyBindingDialog(bool isKeyboard, Buttons::Button buttonId, QWidget *parent = 0);
    ~KeyBindingDialog();

    int key;

protected:
    virtual void keyPressEvent(QKeyEvent *event);

private:
    Ui::KeyBindingDialog *ui;

    bool isKeyboard;

private slots:
#ifdef QT_GAMEPAD_LIB
    void SlotGamepadPressed(int deviceId, QGamepadManager::GamepadButton button, double value);
#endif
};
