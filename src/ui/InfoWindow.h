#pragma once

#include <QtWidgets/QDialog>

#include "core/InfoInterface.h"

namespace Ui {
class InfoWindow;
}

class InfoWindow : public QDialog, public InfoInterface
{
    Q_OBJECT

public:
    explicit InfoWindow(QWidget *parent = 0);
    ~InfoWindow();

    void SetMemory(uint8_t *memory) override {this->memory = memory;}
    void UpdateCartridgeInfo(const Cartridge &cartridge) override;

    void ClearCartridgeInfo();
    void DrawFrame();

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    QString GetChipsetString(uint8_t chipset);
    void UpdateTileView();
    void UpdateMemoryView();

    Ui::InfoWindow *ui;

    uint8_t *memory;

private slots:
    void SlotDrawFrame();

signals:
    void SignalInfoWindowClosed();
    void SignalDrawFrame();
};