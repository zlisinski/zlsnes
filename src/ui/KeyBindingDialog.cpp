#include <QtCore/QHash>
#include <QtGui/QKeyEvent>

#include "KeyBindingDialog.h"
#include "ui_KeyBindingDialog.h"


const QHash<Buttons::Button, QString> ButtonNames = {
    {Buttons::eButtonUp, "'Up'"},
    {Buttons::eButtonDown, "'Down'"},
    {Buttons::eButtonLeft, "'Left'"},
    {Buttons::eButtonRight, "'Right'"},
    {Buttons::eButtonB, "'B'"},
    {Buttons::eButtonA, "'A'"},
    {Buttons::eButtonY, "'Y'"},
    {Buttons::eButtonX, "'X'"},
    {Buttons::eButtonL, "'L'"},
    {Buttons::eButtonR, "'R'"},
    {Buttons::eButtonStart, "'Start'"},
    {Buttons::eButtonSelect, "'Select'"},
};


KeyBindingDialog::KeyBindingDialog(bool isKeyboard, Buttons::Button buttonId, QWidget *parent) :
    QDialog(parent),
    key(0),
    ui(new Ui::KeyBindingDialog),
    isKeyboard(isKeyboard)
{
    ui->setupUi(this);

    const QString buttonName = ButtonNames[buttonId];

    if (isKeyboard)
    {
        ui->label->setText("Press a keyboard key for " + buttonName);

        key = Qt::Key_unknown;
    }
    else
    {
#ifdef QT_GAMEPAD_LIB
        ui->label->setText("Press a gamepad button for " + buttonName);

        key = QGamepadManager::ButtonInvalid;

        connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonPressEvent, this, &KeyBindingDialog::SlotGamepadPressed);
#else
        reject();
#endif
    }

    // Stop the buttons from getting arrows, tab, enter.
    ui->btnCancel->setFocusPolicy(Qt::ClickFocus);
    ui->btnUnbind->setFocusPolicy(Qt::ClickFocus);

    connect(ui->btnCancel, &QPushButton::clicked, this, &KeyBindingDialog::reject);
    connect(ui->btnUnbind, &QPushButton::clicked, this, &KeyBindingDialog::accept);
}


KeyBindingDialog::~KeyBindingDialog()
{
    delete ui;
}


void KeyBindingDialog::keyPressEvent(QKeyEvent *event)
{
    if (isKeyboard)
    {
        key = event->key();
        accept();
    }
}


#ifdef QT_GAMEPAD_LIB
void KeyBindingDialog::SlotGamepadPressed(int deviceId, QGamepadManager::GamepadButton button, double value)
{
    Q_UNUSED(deviceId);
    Q_UNUSED(value);

    if (!isKeyboard)
    {
        key = button;
        accept();
    }
}
#endif