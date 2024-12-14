#pragma once

#include <QtWidgets/QDialog>

#include "core/InfoInterface.h"

namespace Ui {
class InfoWindow;
}

class QTimer;

class InfoWindow : public QDialog, public InfoInterface
{
    Q_OBJECT

public:
    explicit InfoWindow(QWidget *parent = 0);
    ~InfoWindow();

    void SetIoPorts21(const uint8_t *ioPorts21) override {this->ioPorts21 = ioPorts21;}
    void SetVram(const uint8_t *vram) override {this->vram = vram;}
    void SetOam(const uint8_t *oam) override {this->oam = oam;}
    void SetCgram(const uint8_t *cgram) override {this->cgram = cgram;}
    void UpdateCartridgeInfo(const Cartridge &cartridge) override;

    void ClearCartridgeInfo();
    void DrawFrame();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    QString GetChipsetString(uint8_t chipset);
    void GeneratePalette();
    void UpdateTileView();
    void UpdatePaletteView();
    void UpdateMemoryView();

    Ui::InfoWindow *ui;

    const uint8_t *ioPorts21;
    const uint8_t *vram;
    const uint8_t *oam;
    const uint8_t *cgram;

    uint32_t paletteData[256];

    QTimer *timer;

private slots:
    void SlotDrawFrame();

signals:
    void SignalInfoWindowClosed();
    void SignalDrawFrame();
};