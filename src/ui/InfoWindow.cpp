#include <memory>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtWidgets/QGraphicsPixmapItem>
#include <QCryptographicHash>

#include "InfoWindow.h"
#include "SettingsConstants.h"
#include "ui_InfoWindow.h"
#include "UiUtils.h"

#include "core/Cartridge.h"
#include "core/Memory.h"
#include "core/Ppu.h"
#include "core/PpuConstants.h"


InfoWindow::InfoWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoWindow),
    ioPorts21(nullptr),
    ppu(nullptr),
    paletteData{0},
    spriteLiveUpdate(true),
    spritePaletteId(0)
{
    ui->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->gvTiles->setScene(scene);
    scene = new QGraphicsScene(this);
    ui->gvPalette->setScene(scene);

    scene = new QGraphicsScene(this);
    ui->gvObjTable1->setScene(scene);
    scene = new QGraphicsScene(this);
    ui->gvObjTable2->setScene(scene);

    scene = new QGraphicsScene(this);
    ui->gvTilemap->setScene(scene);

    QSettings settings;
    restoreGeometry(settings.value(SETTINGS_INFOWINDOW_GEOMETRY).toByteArray());

    connect(this, SIGNAL(SignalInfoWindowClosed()), parent, SLOT(SlotInfoWindowClosed()));
    connect(this, SIGNAL(SignalDrawFrame()), this, SLOT(SlotDrawFrame()));

    on_chkSpriteLive_clicked(spriteLiveUpdate);

    ClearWidgets();
    DrawFrame();
}


InfoWindow::~InfoWindow()
{
    delete ui;
}


void InfoWindow::ClearWidgets()
{
    ClearMemoryTab();
    ClearTilesTab();
    ClearSpriteTab();
}


void InfoWindow::DrawFrame()
{
    // This should be called from the same thread, but it crashes sometimes without this emit. ¯\_(ツ)_/¯
    emit SignalDrawFrame();
}


void InfoWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}


void InfoWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue(SETTINGS_INFOWINDOW_GEOMETRY, saveGeometry());

    emit SignalInfoWindowClosed();

    QWidget::closeEvent(event);
}


void InfoWindow::GeneratePalette()
{
    if (ioPorts21 == nullptr || ppu == nullptr)
        return;

    // If the palette data hasn't changed, do nothing.
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(reinterpret_cast<const char *>(ppu->cgram.data()), ppu->cgram.size());
    if (paletteHash == hash.result())
        return;

    paletteHash = hash.result();

    // Use the ppu palette but set full brightness.
    for (int i = 0; i < 256; i++)
        paletteData[i] = 0xFF000000 | ppu->palette[i];

    // Make the palette icons for the sprite tab.
    GeneratePaletteIcons();
}


void InfoWindow::SlotDrawFrame()
{
    if (!this->isVisible())
        return;

    GeneratePalette();
    UpdateMemoryTab();
    UpdateTilesTab();
    UpdateSpriteTab();
}


// Memory Tab /////////////////////////////////////////////////////////////////


void InfoWindow::ClearMemoryTab()
{
    ui->labelTitle->setText("");
    ui->labelRomType->setText("");
    ui->labelRomSpeed->setText("");
    ui->labelChipset->setText("");
    ui->labelChipsetSubtype->setText("");
    ui->labelRomSize->setText("");
    ui->labelRamSize->setText("");
    ui->labelExpansionRamSize->setText("");
    ui->labelExpansionFlashSize->setText("");
    ui->labelInterleaved->setText("");
    ui->labelCountry->setText("");
    ui->labelDevId->setText("");
    ui->labelRomVersion->setText("");
    ui->labelSpecialVersion->setText("");
    ui->labelChecksum1->setText("");
    ui->labelChecksum2->setText("");
    ui->labelMakerCode->setText("");
    ui->labelGameCode->setText("");
}


void InfoWindow::UpdateMemoryTab()
{

}


void InfoWindow::UpdateCartridgeInfo(const Cartridge &cartridge)
{
    Cartridge::StandardHeader standardHeader = cartridge.GetStandardHeader();
    Cartridge::ExtendedHeader extendedHeader = cartridge.GetExtendedHeader();

    ui->labelTitle->setText(QString::fromLatin1(standardHeader.title, sizeof(standardHeader.title)));
    switch (cartridge.GetRomType())
    {
        case Cartridge::ERomType::eLoROM:
            ui->labelRomType->setText("LoROM");
            break;
        case Cartridge::ERomType::eHiROM:
            ui->labelRomType->setText("HiROM");
            break;
        case Cartridge::ERomType::eLoROMSDD1:
            ui->labelRomType->setText("LoROM SDD1");
            break;
        case Cartridge::ERomType::eLoROMSA1:
            ui->labelRomType->setText("LoROM SA1");
            break;
        case Cartridge::ERomType::eExHiROM:
            ui->labelRomType->setText("ExHiROM");
            break;
        default:
            ui->labelRomType->setText("Unknown");
            break;
    }
    ui->labelRomSpeed->setText(cartridge.IsFastSpeed() ? "Fast" : "Slow");
    ui->labelChipset->setText(GetChipsetString(standardHeader.chipset));
    ui->labelChipsetSubtype->setText(UiUtils::FormatHex(extendedHeader.chipsetSubtype));
    ui->labelRomSize->setText(QStringLiteral("%1 KB").arg(1 << standardHeader.romSize));
    ui->labelRamSize->setText(QStringLiteral("%1 KB").arg(1 << standardHeader.ramSize));
    ui->labelExpansionRamSize->setText(QStringLiteral("%1 KB").arg(1 << extendedHeader.expansionRamSize));
    ui->labelExpansionFlashSize->setText(QStringLiteral("%1 KB").arg(1 << extendedHeader.expansionFlashSize));
    ui->labelInterleaved->setText(cartridge.IsInterleaved() ? "True" : "False");
    ui->labelCountry->setText(UiUtils::FormatHex(standardHeader.country));
    ui->labelDevId->setText(UiUtils::FormatHex(standardHeader.devId));
    ui->labelRomVersion->setText(UiUtils::FormatHex(standardHeader.romVersion));
    ui->labelSpecialVersion->setText(UiUtils::FormatHex(extendedHeader.specialVersion));
    ui->labelChecksum1->setText(UiUtils::FormatHex(standardHeader.checksum));
    ui->labelChecksum2->setText(UiUtils::FormatHex(standardHeader.checksumComplement));
    ui->labelMakerCode->setText(QString::fromLatin1(extendedHeader.makerCode, sizeof(extendedHeader.makerCode)));
    ui->labelGameCode->setText(QString::fromLatin1(extendedHeader.gameCode, sizeof(extendedHeader.gameCode)));
}


QString InfoWindow::GetChipsetString(uint8_t chipset)
{
    QString str = QStringLiteral("%1 - ROM").arg(UiUtils::FormatHex(chipset));

    if (chipset == 0)
        return str;
    if (chipset == 1)
        return str + ", RAM";
    if (chipset == 2)
        return str + ", RAM, Bat";
    
    // Everything else has a coprocessor 

    if ((chipset & 0x0F) == 0x03)
        ; // Rom only
    else if ((chipset & 0x0F) == 0x04)
        str + ", RAM";
    else if ((chipset & 0x0F) == 0x05)
        str += ", RAM, Bat";
    else if ((chipset & 0x0F) == 0x06)
        str += ", Bat";
    else
        return QStringLiteral("%1 - Unknown").arg(UiUtils::FormatHex(chipset));

    if ((chipset & 0xF0) == 0x00)
        str += ", DSP";
    else if ((chipset & 0xF0) == 0x10)
        str += ", GSU";
    else if ((chipset & 0xF0) == 0x20)
        str += ", OBC1";
    else if ((chipset & 0xF0) == 0x30)
        str += ", SA-1";
    else if ((chipset & 0xF0) == 0x40)
        str += ", S-DD1";
    else if ((chipset & 0xF0) == 0x50)
        str += ", S-RTC";
    else if ((chipset & 0xF0) == 0xE0)
        str += ", Other";
    else if ((chipset & 0xF0) == 0xF0)
        str += ", Custom";
    else
        return QStringLiteral("%1 - Unknown").arg(UiUtils::FormatHex(chipset));
    
    return str;
}


// Tiles Tab //////////////////////////////////////////////////////////////////


void InfoWindow::ClearTilesTab()
{
    ui->labelVideoMode->setText("");

    ui->labelBG1Enabled->setText("");
    ui->labelBG4Enabled->setText("");
    ui->labelBG2Enabled->setText("");
    ui->labelBG3Enabled->setText("");

    ui->labelBG1ChrSize->setText("");
    ui->labelBG2ChrSize->setText("");
    ui->labelBG3ChrSize->setText("");
    ui->labelBG4ChrSize->setText("");

    ui->labelBG1Tileset->setText("");
    ui->labelBG2Tileset->setText("");
    ui->labelBG3Tileset->setText("");
    ui->labelBG4Tileset->setText("");

    ui->labelBG1Tilemap->setText("");
    ui->labelBG2Tilemap->setText("");
    ui->labelBG3Tilemap->setText("");
    ui->labelBG4Tilemap->setText("");

    ui->labelBG1HOFS->setText("");
    ui->labelBG2HOFS->setText("");
    ui->labelBG3HOFS->setText("");
    ui->labelBG4HOFS->setText("");

    ui->labelBG1VOFS->setText("");
    ui->labelBG2VOFS->setText("");
    ui->labelBG3VOFS->setText("");
    ui->labelBG4VOFS->setText("");
}


void InfoWindow::UpdateTilesTab()
{
    if (ppu == nullptr)
        return;

    ui->labelVideoMode->setText(QString::number(ppu->bgMode));
    ui->labelBG1ChrSize->setText(QStringLiteral("%1x%2").arg(ppu->bgChrSize[eBG1]).arg(ppu->bgChrSize[eBG1]));
    ui->labelBG2ChrSize->setText(QStringLiteral("%1x%2").arg(ppu->bgChrSize[eBG2]).arg(ppu->bgChrSize[eBG2]));
    ui->labelBG3ChrSize->setText(QStringLiteral("%1x%2").arg(ppu->bgChrSize[eBG3]).arg(ppu->bgChrSize[eBG3]));
    ui->labelBG4ChrSize->setText(QStringLiteral("%1x%2").arg(ppu->bgChrSize[eBG4]).arg(ppu->bgChrSize[eBG4]));

    ui->labelBG1Enabled->setText((ppu->mainScreenLayerEnabled[eBG1] ? "Main" : "") + QString(" ") + (ppu->subScreenLayerEnabled[eBG1] ? "Sub" : ""));
    ui->labelBG2Enabled->setText((ppu->mainScreenLayerEnabled[eBG2] ? "Main" : "") + QString(" ") + (ppu->subScreenLayerEnabled[eBG2] ? "Sub" : ""));
    ui->labelBG3Enabled->setText((ppu->mainScreenLayerEnabled[eBG3] ? "Main" : "") + QString(" ") + (ppu->subScreenLayerEnabled[eBG3] ? "Sub" : ""));
    ui->labelBG4Enabled->setText((ppu->mainScreenLayerEnabled[eBG4] ? "Main" : "") + QString(" ") + (ppu->subScreenLayerEnabled[eBG4] ? "Sub" : ""));
    
    ui->labelBG1Tileset->setText(UiUtils::FormatHexWord(ppu->bgChrAddr[eBG1]));
    ui->labelBG2Tileset->setText(UiUtils::FormatHexWord(ppu->bgChrAddr[eBG2]));
    ui->labelBG3Tileset->setText(UiUtils::FormatHexWord(ppu->bgChrAddr[eBG4]));
    ui->labelBG4Tileset->setText(UiUtils::FormatHexWord(ppu->bgChrAddr[eBG4]));
    
    ui->labelBG1Tilemap->setText(UiUtils::FormatHexWord(ppu->bgTilemapAddr[eBG1]) +
                                 " " + QString::number(ppu->bgTilemapWidth[eBG1]) +
                                 "x" + QString::number(ppu->bgTilemapHeight[eBG1]));

    ui->labelBG2Tilemap->setText(UiUtils::FormatHexWord(ppu->bgTilemapAddr[eBG2]) +
                                 " " + QString::number(ppu->bgTilemapWidth[eBG2]) +
                                 "x" + QString::number(ppu->bgTilemapHeight[eBG2]));

    ui->labelBG3Tilemap->setText(UiUtils::FormatHexWord(ppu->bgTilemapAddr[eBG3]) +
                                 " " + QString::number(ppu->bgTilemapWidth[eBG3]) +
                                 "x" + QString::number(ppu->bgTilemapHeight[eBG3]));

    ui->labelBG4Tilemap->setText(UiUtils::FormatHexWord(ppu->bgTilemapAddr[eBG4]) +
                                 " " + QString::number(ppu->bgTilemapWidth[eBG4]) +
                                 "x" + QString::number(ppu->bgTilemapHeight[eBG4]));

    ui->labelBG1HOFS->setText(QString::number(ppu->bgHOffset[0]));
    ui->labelBG2HOFS->setText(QString::number(ppu->bgHOffset[1]));
    ui->labelBG3HOFS->setText(QString::number(ppu->bgHOffset[2]));
    ui->labelBG4HOFS->setText(QString::number(ppu->bgHOffset[3]));

    ui->labelBG1VOFS->setText(QString::number(ppu->bgVOffset[0]));
    ui->labelBG2VOFS->setText(QString::number(ppu->bgVOffset[1]));
    ui->labelBG3VOFS->setText(QString::number(ppu->bgVOffset[2]));
    ui->labelBG4VOFS->setText(QString::number(ppu->bgVOffset[3]));

    UpdatePaletteView();
    UpdateTileView();
    UpdateTilemapView();
}


void InfoWindow::UpdatePaletteView()
{
    const int SCALE = 16;
    ui->gvPalette->scene()->clear();
    ui->gvPalette->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));
    
    QPen pen;
    QBrush brush;

    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            uint32_t color = paletteData[(y * 16) + x];
            pen.setColor(color);
            brush.setColor(color);
            brush.setStyle(Qt::SolidPattern);
            ui->gvPalette->scene()->addRect((x * SCALE), (y * SCALE), SCALE, SCALE, pen, brush);
        }
    }
}


void InfoWindow::UpdateTileView()
{
    if (ioPorts21 == nullptr || ppu == nullptr)
        return;

    const int SCALE = 3;
    const uint16_t bg1TileOffset = (ioPorts21[eRegBG12NBA & 0xFF] & 0x0F) << 13;
    const uint8_t *tilesetData = &ppu->vram[bg1TileOffset];

    ui->gvTiles->scene()->clear();
    ui->gvTiles->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));
    QPen pen(Qt::green, 1);

    // Draw grid.
    for (int i = 0; i < 25; i++)
    {
        int len = i * ((SCALE * 8) + 1);
        ui->gvTiles->scene()->addLine(0, len, 400, len, pen);
    }
    for (int i = 0; i < 17; i++)
    {
        int len = i * ((SCALE * 8) + 1);
        ui->gvTiles->scene()->addLine(len, 0, len, 600, pen);
    }

    for (int tile = 0; tile < 384; tile++)
    {
        QImage img(8, 8, QImage::Format_RGB32);

        for (int y = 0; y < 8; y++)
        {
            const uint8_t *tileData = &tilesetData[(tile * 16) + (y * 2)];

            for (int x = 0; x < 8; x++)
            {
                uint8_t lowBit = ((tileData[0] >> (7 - x)) & 0x01);
                uint8_t highBit = ((tileData[1] >> (7 - x)) & 0x01);
                uint8_t pixelVal = lowBit | (highBit << 1);
                img.setPixel(x, y, paletteData[pixelVal]);
            }
        }

        QGraphicsPixmapItem *pixmap = ui->gvTiles->scene()->addPixmap(QPixmap::fromImage(img));
        pixmap->setScale(SCALE);
        int xpos = 1 + ((tile % 16) * 8 * SCALE) + (tile % 16);
        int ypos = 1 + ((tile / 16) * 8 * SCALE) + (tile / 16);
        pixmap->setPos(xpos, ypos);
    }
}


void InfoWindow::UpdateTilemapView()
{
    if (ppu == nullptr)
        return;

    // This slows down the ui, so only draw when visible.
    if (!ui->gvTilemap->isVisible())
        return;

    //int tileSize = ppu->bgChrSize[eBG1];
    int bpp = BG_BPP_LOOKUP[ppu->bgMode][eBG1];

    for (int tileX = 0; tileX < ppu->bgTilemapWidth[eBG1]; tileX++)
    {
        for (int tileY = 0; tileY < ppu->bgTilemapHeight[eBG1]; tileY++)
        {
            uint16_t tilemapEntry = ppu->GetBgTilemapEntry(eBG1, tileX, tileY);
            uint32_t tileId = tilemapEntry & 0x3FF;
            uint8_t paletteId = (tilemapEntry >> 10) & 0x07;
            bool flipX = Bytes::GetBit<14>(tilemapEntry);
            bool flipY = Bytes::GetBit<15>(tilemapEntry);

            QImage img(8, 8, QImage::Format_RGB32);

            // TODO: Handle 16x16 tiles.
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    uint16_t tileAddr = ppu->bgChrAddr[eBG1] + (tileId * 8 * bpp);
                    uint8_t pixelVal = ppu->GetTilePixelData(tileAddr, x, y, bpp);
                    img.setPixel(x, y, paletteData[pixelVal + (paletteId << bpp)]);
                }
            }

            if (flipX || flipY)
                img = img.mirrored(flipX, flipY);

            QGraphicsPixmapItem *pixmap = ui->gvTilemap->scene()->addPixmap(QPixmap::fromImage(img));
            int xpos = tileX * 8;
            int ypos = tileY * 8;
            pixmap->setPos(xpos, ypos);
        }
    }

    QPen pen(Qt::green);
    int tilemapWidth = ppu->bgTilemapWidth[eBG1] * 8;
    int tilemapHeight = ppu->bgTilemapHeight[eBG1] * 8;
    int xOff = ppu->bgHOffset[eBG1] & (tilemapWidth - 1);
    int yOff = ppu->bgVOffset[eBG1] & (tilemapHeight - 1);
    
    if (xOff + 256 <= tilemapWidth && yOff + 224 <= tilemapHeight)
    {
        ui->gvTilemap->scene()->addRect(xOff, yOff, 256, 224, pen);
    }
    else
    {
        int xOff2 = (xOff + 256) % tilemapWidth;
        int yOff2 = (yOff + 224) % tilemapHeight;

        // Draw top line
        ui->gvTilemap->scene()->addLine(xOff, yOff, std::min(xOff + 256, tilemapWidth - 1), yOff, pen);
        if (xOff + 256 > tilemapWidth)
        {
            int remain = (xOff + 256) - tilemapWidth;
            ui->gvTilemap->scene()->addLine(0, yOff, remain, yOff, pen);
        }

        // Draw bottom line
        ui->gvTilemap->scene()->addLine(xOff, yOff2, std::min(xOff + 256, tilemapWidth - 1), yOff2, pen);
        if (xOff + 256 > tilemapWidth)
        {
            int remain = (xOff + 256) - tilemapWidth;
            ui->gvTilemap->scene()->addLine(0, yOff2, remain, yOff2, pen);
        }

        // Draw left line
        ui->gvTilemap->scene()->addLine(xOff, yOff, xOff, std::min(yOff + 224, tilemapHeight - 1), pen);
        if (yOff + 224 > tilemapHeight)
        {
            int remain = (yOff + 224) - tilemapHeight;
            ui->gvTilemap->scene()->addLine(xOff, 0, xOff, remain, pen);
        }

        // Draw right line
        ui->gvTilemap->scene()->addLine(xOff2, yOff, xOff2, std::min(yOff + 224, tilemapHeight - 1), pen);
        if (yOff + 224 > tilemapHeight)
        {
            int remain = (yOff + 224) - tilemapHeight;
            ui->gvTilemap->scene()->addLine(xOff2, 0, xOff2, remain, pen);
        }
    }
}


// Sprite Tab /////////////////////////////////////////////////////////////////


void InfoWindow::on_chkSpriteLive_clicked(bool checked)
{
    spriteLiveUpdate = checked;
    ui->btnSpriteUpdate->setEnabled(!spriteLiveUpdate);
}


void InfoWindow::on_btnSpriteUpdate_clicked()
{
    if (ppu == nullptr)
        return;

    DrawSpriteTable(ppu->objBaseAddr[0], ui->gvObjTable1);
    DrawSpriteTable(ppu->objBaseAddr[1], ui->gvObjTable2);
}


void InfoWindow::on_cmbSpritePalette_currentIndexChanged(int index)
{
    spritePaletteId = index;

    on_btnSpriteUpdate_clicked();
}


void InfoWindow::ClearSpriteTab()
{
    ui->labelObjTable1Addr->setText("");
    ui->labelObjTable2Addr->setText("");
    ui->labelObjSizes->setText("");

    ui->gvObjTable1->scene()->clear();
    ui->gvObjTable2->scene()->clear();

    ui->cmbSpritePalette->clear();
    for (int i = 0; i < 8; i++)
    {
        ui->cmbSpritePalette->addItem("Palette " + QString::number(i));
    }
}


void InfoWindow::UpdateSpriteTab()
{
    if (ppu == nullptr || ioPorts21 == nullptr)
        return;

    ui->labelObjTable1Addr->setText(UiUtils::FormatHexWord(ppu->objBaseAddr[0]));
    ui->labelObjTable2Addr->setText(UiUtils::FormatHexWord(ppu->objBaseAddr[1]));

    QString str = QString::number(OBJ_H_SIZE_LOOKUP[ppu->objSize][0]) + "x" + QString::number(OBJ_V_SIZE_LOOKUP[ppu->objSize][0]) + ", " +
                  QString::number(OBJ_H_SIZE_LOOKUP[ppu->objSize][1]) + "x" + QString::number(OBJ_V_SIZE_LOOKUP[ppu->objSize][1]);
    ui->labelObjSizes->setText(str);

    if (spriteLiveUpdate)
    {
        DrawSpriteTable(ppu->objBaseAddr[0], ui->gvObjTable1);
        DrawSpriteTable(ppu->objBaseAddr[1], ui->gvObjTable2);
    }
}


void InfoWindow::GeneratePaletteIcons()
{
    int scale = 8;
    int colorsPerRow = 4;
    int iconSize = scale * colorsPerRow;
    ui->cmbSpritePalette->setIconSize(QSize(iconSize, iconSize));

    for (int i = 0; i < 8; i++)
    {
        QImage img(iconSize, iconSize, QImage::Format_ARGB32);
        for (int colorId = 0; colorId < 16; colorId++)
        {
            int x = (colorId % colorsPerRow) * scale;
            int y = (colorId / colorsPerRow) * scale;
            uint32_t color = paletteData[128 + (i * 16) + colorId];

            for (int x2 = 0; x2 < scale; x2++)
            {
                for (int y2 = 0; y2 < scale; y2++)
                {
                    img.setPixel(x + x2, y + y2, color);
                }
            }
        }

        QIcon icon(QPixmap::fromImage(img));
        ui->cmbSpritePalette->setItemIcon(i, icon);
    }
}


void InfoWindow::DrawSpriteTable(uint16_t baseAddr, QGraphicsView *gv)
{
    if (ppu == nullptr || ioPorts21 == nullptr)
        return;

    if (!gv->isVisible())
        return;

    const int SCALE = 3;

    gv->scene()->clear();
    gv->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));
    QPen pen(Qt::green, 1);

    // Draw grid.
    for (int i = 0; i < 17; i++)
    {
        int len = i * ((SCALE * 8) + 1);
        gv->scene()->addLine(0, len, 400, len, pen);
    }
    for (int i = 0; i < 17; i++)
    {
        int len = i * ((SCALE * 8) + 1);
        gv->scene()->addLine(len, 0, len, 400, pen);
    }

    for (int tileId = 0; tileId < 256; tileId++)
    {
        QImage img(8, 8, QImage::Format_RGB32);

        for (int x = 0; x < 8; x++)
        {
            for (int y = 0; y < 8; y++)
            {
                uint16_t tileAddr = baseAddr + (tileId * 8  * OBJ_BPP); // 8px width * 4bpp;
                uint8_t pixelVal = ppu->GetTilePixelData(tileAddr, x, y, OBJ_BPP);
                img.setPixel(x, y, paletteData[128 + (spritePaletteId * 16) + pixelVal]);
            }
        }

        QGraphicsPixmapItem *pixmap = gv->scene()->addPixmap(QPixmap::fromImage(img));
        pixmap->setScale(SCALE);
        int xpos = 1 + ((tileId % 16) * 8 * SCALE) + (tileId % 16);
        int ypos = 1 + ((tileId / 16) * 8 * SCALE) + (tileId / 16);
        pixmap->setPos(xpos, ypos);
    }
}