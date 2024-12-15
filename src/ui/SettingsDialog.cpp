#include <QtCore/QHash>
#include <QtCore/QSettings>
#include <QtWidgets/QtWidgets>

#include "KeyBindingDialog.h"
#include "SettingsConstants.h"
#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"


#ifdef QT_GAMEPAD_LIB
const QHash<QGamepadManager::GamepadButton, QString> GamepadButtonNames = {
    {QGamepadManager::ButtonInvalid, "Unbound"},
    {QGamepadManager::ButtonA, "A"},
    {QGamepadManager::ButtonB, "B"},
    {QGamepadManager::ButtonX, "X"},
    {QGamepadManager::ButtonY, "Y"},
    {QGamepadManager::ButtonL1, "L1"},
    {QGamepadManager::ButtonR1, "R1"},
    {QGamepadManager::ButtonL2, "L2"},
    {QGamepadManager::ButtonR2, "R2"},
    {QGamepadManager::ButtonSelect, "Select"},
    {QGamepadManager::ButtonStart, "Start"},
    {QGamepadManager::ButtonL3, "L3"},
    {QGamepadManager::ButtonR3, "R3"},
    {QGamepadManager::ButtonUp, "Up"},
    {QGamepadManager::ButtonDown, "Down"},
    {QGamepadManager::ButtonRight, "Right"},
    {QGamepadManager::ButtonLeft, "Left"},
    {QGamepadManager::ButtonCenter, "Center"},
    {QGamepadManager::ButtonGuide, "Guide"}
};
#endif


SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    dirty(false)
{
    ui->setupUi(this);

    QSettings settings;

    ui->chkAudioEnabled->setChecked(settings.value(SETTINGS_AUDIO_ENABLED, true).toBool());
    ui->sldAudioVolume->setValue(settings.value(SETTINGS_AUDIO_VOLUME, 128).toInt());
    ui->chkAudioChannel1->setChecked(settings.value(SETTINGS_AUDIO_CHANNEL1, true).toBool());
    ui->chkAudioChannel2->setChecked(settings.value(SETTINGS_AUDIO_CHANNEL2, true).toBool());
    ui->chkAudioChannel3->setChecked(settings.value(SETTINGS_AUDIO_CHANNEL3, true).toBool());
    ui->chkAudioChannel4->setChecked(settings.value(SETTINGS_AUDIO_CHANNEL4, true).toBool());

    // Use existing functions to enable/disable controls, and set dirty to false, since it is set in functions.
    SlotToggleEnableAudio(ui->chkAudioEnabled->isChecked());
    SlotAudioVolumeChanged(ui->sldAudioVolume->value());
    dirty = false;

    keyBindings = {
        {Buttons::eButtonUp, static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_UP, Qt::Key_W).toInt())},
        {Buttons::eButtonDown, static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_DOWN, Qt::Key_S).toInt())},
        {Buttons::eButtonLeft, static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_LEFT, Qt::Key_A).toInt())},
        {Buttons::eButtonRight, static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_RIGHT, Qt::Key_D).toInt())},
        {Buttons::eButtonB, static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_B, Qt::Key_K).toInt())},
        {Buttons::eButtonA, static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_A, Qt::Key_L).toInt())},
        {Buttons::eButtonY, static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_Y, Qt::Key_I).toInt())},
        {Buttons::eButtonX, static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_X, Qt::Key_O).toInt())},
        {Buttons::eButtonL, static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_L, Qt::Key_U).toInt())},
        {Buttons::eButtonR, static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_R, Qt::Key_P).toInt())},
        {Buttons::eButtonStart, static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_START, Qt::Key_J).toInt())},
        {Buttons::eButtonSelect, static_cast<Qt::Key>(settings.value(SETTINGS_INPUT_KEY_SELECT, Qt::Key_H).toInt())}
    };
#ifdef QT_GAMEPAD_LIB
    padBindings = {
        {Buttons::eButtonUp, static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_UP, QGamepadManager::ButtonInvalid).toInt())},
        {Buttons::eButtonDown, static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_DOWN, QGamepadManager::ButtonInvalid).toInt())},
        {Buttons::eButtonLeft, static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_LEFT, QGamepadManager::ButtonInvalid).toInt())},
        {Buttons::eButtonRight, static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_RIGHT, QGamepadManager::ButtonInvalid).toInt())},
        {Buttons::eButtonB, static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_B, QGamepadManager::ButtonInvalid).toInt())},
        {Buttons::eButtonA, static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_A, QGamepadManager::ButtonInvalid).toInt())},
        {Buttons::eButtonY, static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_Y, QGamepadManager::ButtonInvalid).toInt())},
        {Buttons::eButtonX, static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_X, QGamepadManager::ButtonInvalid).toInt())},
        {Buttons::eButtonL, static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_L, QGamepadManager::ButtonInvalid).toInt())},
        {Buttons::eButtonR, static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_R, QGamepadManager::ButtonInvalid).toInt())},
        {Buttons::eButtonStart, static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_START, QGamepadManager::ButtonInvalid).toInt())},
        {Buttons::eButtonSelect, static_cast<QGamepadManager::GamepadButton>(settings.value(SETTINGS_INPUT_PAD_SELECT, QGamepadManager::ButtonInvalid).toInt())},
    };
#endif

    UpdateKeyBindingButtonText();

    // Add custom property to each button, so we can use one slot for all {key,pad} buttons.
    ui->btnUpKey->setProperty("buttonId", Buttons::eButtonUp);
    ui->btnDownKey->setProperty("buttonId", Buttons::eButtonDown);
    ui->btnLeftKey->setProperty("buttonId", Buttons::eButtonLeft);
    ui->btnRightKey->setProperty("buttonId", Buttons::eButtonRight);
    ui->btnBKey->setProperty("buttonId", Buttons::eButtonB);
    ui->btnAKey->setProperty("buttonId", Buttons::eButtonA);
    ui->btnYKey->setProperty("buttonId", Buttons::eButtonY);
    ui->btnXKey->setProperty("buttonId", Buttons::eButtonX);
    ui->btnLKey->setProperty("buttonId", Buttons::eButtonL);
    ui->btnRKey->setProperty("buttonId", Buttons::eButtonR);
    ui->btnStartKey->setProperty("buttonId", Buttons::eButtonStart);
    ui->btnSelectKey->setProperty("buttonId", Buttons::eButtonSelect);
    ui->btnUpPad->setProperty("buttonId", Buttons::eButtonUp);
    ui->btnDownPad->setProperty("buttonId", Buttons::eButtonDown);
    ui->btnLeftPad->setProperty("buttonId", Buttons::eButtonLeft);
    ui->btnRightPad->setProperty("buttonId", Buttons::eButtonRight);
    ui->btnBPad->setProperty("buttonId", Buttons::eButtonB);
    ui->btnAPad->setProperty("buttonId", Buttons::eButtonA);
    ui->btnYPad->setProperty("buttonId", Buttons::eButtonY);
    ui->btnXPad->setProperty("buttonId", Buttons::eButtonX);
    ui->btnLPad->setProperty("buttonId", Buttons::eButtonL);
    ui->btnRPad->setProperty("buttonId", Buttons::eButtonR);
    ui->btnStartPad->setProperty("buttonId", Buttons::eButtonStart);
    ui->btnSelectPad->setProperty("buttonId", Buttons::eButtonSelect);

    keyButtons = {ui->btnUpKey, ui->btnDownKey, ui->btnLeftKey, ui->btnRightKey,
                  ui->btnBKey, ui->btnAKey, ui->btnYKey, ui->btnXKey, ui->btnLKey, ui->btnRKey,
                  ui->btnStartKey, ui->btnSelectKey};
    padButtons = {ui->btnUpPad, ui->btnDownPad, ui->btnLeftPad, ui->btnRightPad,
                  ui->btnBPad, ui->btnAPad, ui->btnYPad, ui->btnXPad, ui->btnLPad, ui->btnRPad,
                  ui->btnStartPad, ui->btnSelectPad};

    connect(ui->chkAudioEnabled, &QCheckBox::toggled, this, &SettingsDialog::SlotToggleEnableAudio);
    connect(ui->sldAudioVolume, &QSlider::valueChanged, this, &SettingsDialog::SlotAudioVolumeChanged);
    connect(ui->chkAudioChannel1, &QCheckBox::toggled, [&](){dirty = true;});
    connect(ui->chkAudioChannel2, &QCheckBox::toggled, [&](){dirty = true;});
    connect(ui->chkAudioChannel3, &QCheckBox::toggled, [&](){dirty = true;});
    connect(ui->chkAudioChannel4, &QCheckBox::toggled, [&](){dirty = true;});
    for (QPushButton *button : keyButtons)
    {
        connect(button, &QPushButton::clicked, this, &SettingsDialog::SlotInputBindKey);
    }
    for (QPushButton *button : padButtons)
    {
#ifdef QT_GAMEPAD_LIB
        connect(button, &QPushButton::clicked, this, &SettingsDialog::SlotInputBindPad);
#else
        button->setEnabled(false);
#endif
    }
}


SettingsDialog::~SettingsDialog()
{
    delete ui;
}


void SettingsDialog::accept()
{
    SaveSettings();

    QDialog::accept();
}


void SettingsDialog::closeEvent(QCloseEvent *event)
{
    if (dirty)
    {
        QMessageBox msg(QMessageBox::Warning, "Unsaved changes",
                        "There are unsaved changes. Do you want to save them?",
                        QMessageBox::Save | QMessageBox::Discard, this);

        if (msg.exec() == QMessageBox::Save)
            SaveSettings();
    }

    QWidget::closeEvent(event);
}


void SettingsDialog::SaveSettings()
{
    QSettings settings;

    settings.setValue(SETTINGS_AUDIO_ENABLED, ui->chkAudioEnabled->isChecked());
    settings.setValue(SETTINGS_AUDIO_VOLUME, ui->sldAudioVolume->value());
    settings.setValue(SETTINGS_AUDIO_CHANNEL1, ui->chkAudioChannel1->isChecked());
    settings.setValue(SETTINGS_AUDIO_CHANNEL2, ui->chkAudioChannel2->isChecked());
    settings.setValue(SETTINGS_AUDIO_CHANNEL3, ui->chkAudioChannel3->isChecked());
    settings.setValue(SETTINGS_AUDIO_CHANNEL4, ui->chkAudioChannel4->isChecked());

    settings.setValue(SETTINGS_INPUT_KEY_UP, keyBindings[Buttons::eButtonUp]);
    settings.setValue(SETTINGS_INPUT_KEY_DOWN, keyBindings[Buttons::eButtonDown]);
    settings.setValue(SETTINGS_INPUT_KEY_LEFT, keyBindings[Buttons::eButtonLeft]);
    settings.setValue(SETTINGS_INPUT_KEY_RIGHT, keyBindings[Buttons::eButtonRight]);
    settings.setValue(SETTINGS_INPUT_KEY_B, keyBindings[Buttons::eButtonB]);
    settings.setValue(SETTINGS_INPUT_KEY_A, keyBindings[Buttons::eButtonA]);
    settings.setValue(SETTINGS_INPUT_KEY_Y, keyBindings[Buttons::eButtonY]);
    settings.setValue(SETTINGS_INPUT_KEY_X, keyBindings[Buttons::eButtonX]);
    settings.setValue(SETTINGS_INPUT_KEY_L, keyBindings[Buttons::eButtonL]);
    settings.setValue(SETTINGS_INPUT_KEY_R, keyBindings[Buttons::eButtonR]);
    settings.setValue(SETTINGS_INPUT_KEY_START, keyBindings[Buttons::eButtonStart]);
    settings.setValue(SETTINGS_INPUT_KEY_SELECT, keyBindings[Buttons::eButtonSelect]);
#ifdef QT_GAMEPAD_LIB
    settings.setValue(SETTINGS_INPUT_PAD_UP, padBindings[Buttons::eButtonUp]);
    settings.setValue(SETTINGS_INPUT_PAD_DOWN, padBindings[Buttons::eButtonDown]);
    settings.setValue(SETTINGS_INPUT_PAD_LEFT, padBindings[Buttons::eButtonLeft]);
    settings.setValue(SETTINGS_INPUT_PAD_RIGHT, padBindings[Buttons::eButtonRight]);
    settings.setValue(SETTINGS_INPUT_PAD_B, padBindings[Buttons::eButtonB]);
    settings.setValue(SETTINGS_INPUT_PAD_A, padBindings[Buttons::eButtonA]);
    settings.setValue(SETTINGS_INPUT_PAD_Y, padBindings[Buttons::eButtonY]);
    settings.setValue(SETTINGS_INPUT_PAD_X, padBindings[Buttons::eButtonX]);
    settings.setValue(SETTINGS_INPUT_PAD_L, padBindings[Buttons::eButtonL]);
    settings.setValue(SETTINGS_INPUT_PAD_R, padBindings[Buttons::eButtonR]);
    settings.setValue(SETTINGS_INPUT_PAD_START, padBindings[Buttons::eButtonStart]);
    settings.setValue(SETTINGS_INPUT_PAD_SELECT, padBindings[Buttons::eButtonSelect]);
#endif
}


QString SettingsDialog::GetKeyString(Qt::Key keycode)
{
    switch (keycode)
    {
        case Qt::Key_unknown:
            return "Unbound";
        case Qt::Key_Shift:
            return "Shift";
        case Qt::Key_Control:
            return "Control";
        case Qt::Key_Alt:
            return "Alt";
        case Qt::Key_Meta:
            return "Meta";
        default:
            return QKeySequence(keycode).toString();
    }
}


void SettingsDialog::UpdateKeyBindingButtonText()
{
    ui->btnUpKey->setText(GetKeyString(keyBindings[Buttons::eButtonUp]));
    ui->btnDownKey->setText(GetKeyString(keyBindings[Buttons::eButtonDown]));
    ui->btnLeftKey->setText(GetKeyString(keyBindings[Buttons::eButtonLeft]));
    ui->btnRightKey->setText(GetKeyString(keyBindings[Buttons::eButtonRight]));
    ui->btnBKey->setText(GetKeyString(keyBindings[Buttons::eButtonB]));
    ui->btnAKey->setText(GetKeyString(keyBindings[Buttons::eButtonA]));
    ui->btnYKey->setText(GetKeyString(keyBindings[Buttons::eButtonY]));
    ui->btnXKey->setText(GetKeyString(keyBindings[Buttons::eButtonX]));
    ui->btnLKey->setText(GetKeyString(keyBindings[Buttons::eButtonL]));
    ui->btnRKey->setText(GetKeyString(keyBindings[Buttons::eButtonR]));
    ui->btnStartKey->setText(GetKeyString(keyBindings[Buttons::eButtonStart]));
    ui->btnSelectKey->setText(GetKeyString(keyBindings[Buttons::eButtonSelect]));
#ifdef QT_GAMEPAD_LIB
    ui->btnUpPad->setText(GamepadButtonNames[padBindings[Buttons::eButtonUp]]);
    ui->btnDownPad->setText(GamepadButtonNames[padBindings[Buttons::eButtonDown]]);
    ui->btnLeftPad->setText(GamepadButtonNames[padBindings[Buttons::eButtonLeft]]);
    ui->btnRightPad->setText(GamepadButtonNames[padBindings[Buttons::eButtonRight]]);
    ui->btnBPad->setText(GamepadButtonNames[padBindings[Buttons::eButtonB]]);
    ui->btnAPad->setText(GamepadButtonNames[padBindings[Buttons::eButtonA]]);
    ui->btnYPad->setText(GamepadButtonNames[padBindings[Buttons::eButtonY]]);
    ui->btnXPad->setText(GamepadButtonNames[padBindings[Buttons::eButtonX]]);
    ui->btnLPad->setText(GamepadButtonNames[padBindings[Buttons::eButtonL]]);
    ui->btnRPad->setText(GamepadButtonNames[padBindings[Buttons::eButtonR]]);
    ui->btnStartPad->setText(GamepadButtonNames[padBindings[Buttons::eButtonStart]]);
    ui->btnSelectPad->setText(GamepadButtonNames[padBindings[Buttons::eButtonSelect]]);
#endif
}


void SettingsDialog::SlotToggleEnableAudio(bool checked)
{
    ui->sldAudioVolume->setEnabled(checked);
    ui->chkAudioChannel1->setEnabled(checked);
    ui->chkAudioChannel2->setEnabled(checked);
    ui->chkAudioChannel3->setEnabled(checked);
    ui->chkAudioChannel4->setEnabled(checked);
    ui->lblAudioVolume->setEnabled(checked);

    dirty = true;
}


void SettingsDialog::SlotAudioVolumeChanged(int value)
{
    ui->lblAudioVolume->setText("Volume     " + QString::number(value));
    dirty = true;
}


void SettingsDialog::SlotInputBindKey()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button == NULL)
        return;

    Buttons::Button buttonId = static_cast<Buttons::Button>(button->property("buttonId").toInt());
    KeyBindingDialog dialog(true, buttonId, this);
    dialog.setModal(true);
    if (dialog.exec())
    {
        Qt::Key key = static_cast<Qt::Key>(dialog.key);
        keyBindings[buttonId] = key;

        // TODO: Handle duplicate keys.

        UpdateKeyBindingButtonText();

        dirty = true;
    }
}


void SettingsDialog::SlotInputBindPad()
{
#ifdef QT_GAMEPAD_LIB
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button == NULL)
        return;

    Buttons::Button buttonId = static_cast<Buttons::Button>(button->property("buttonId").toInt());
    KeyBindingDialog dialog(false, buttonId, this);
    dialog.setModal(true);
    if (dialog.exec())
    {
        QGamepadManager::GamepadButton key = static_cast<QGamepadManager::GamepadButton>(dialog.key);
        padBindings[buttonId] = key;

        // TODO: Handle duplicate keys.

        UpdateKeyBindingButtonText();

        dirty = true;
    }
#endif
}