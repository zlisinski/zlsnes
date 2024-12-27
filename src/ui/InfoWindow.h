#pragma once

#include <QtWidgets/QDialog>

#include "core/InfoInterface.h"

namespace Ui {
class InfoWindow;
}

class QGraphicsView;
class QTimer;

class InfoWindow : public QDialog, public InfoInterface
{
    Q_OBJECT

public:
    explicit InfoWindow(QWidget *parent = 0);
    ~InfoWindow();

    void SetIoPorts21(const uint8_t *ioPorts21) override {this->ioPorts21 = ioPorts21;}
    void SetPpu(const Ppu *ppu) override {this->ppu = ppu;}
    void UpdateCartridgeInfo(const Cartridge &cartridge) override;

    void ClearWidgets();
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
    void UpdateSpriteTab();
    void DrawSpriteTable(uint16_t baseAddr, QGraphicsView *gv);

    Ui::InfoWindow *ui;

    const uint8_t *ioPorts21;
    const Ppu *ppu;

    uint32_t paletteData[256];

private slots:
    void SlotDrawFrame();

signals:
    void SignalInfoWindowClosed();
    void SignalDrawFrame();
};