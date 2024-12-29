#include <memory>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtWidgets/QGraphicsPixmapItem>

#include "InfoWindow.h"
#include "SettingsConstants.h"
#include "ui_InfoWindow.h"
#include "UiUtils.h"

#include "core/Cartridge.h"
#include "core/Memory.h"
#include "core/Ppu.h"


static const uint8_t OBJ_H_SIZE_LOOKUP[8][2] = {
    {8, 16}, // 0
    {8, 32}, // 1
    {8, 64}, // 2
    {16, 32}, // 3
    {16, 64}, // 4
    {32, 64}, // 5
    {16, 32}, // 6
    {16, 32}, // 7
};

static const uint8_t OBJ_V_SIZE_LOOKUP[8][2] = {
    {8, 16}, // 0
    {8, 32}, // 1
    {8, 64}, // 2
    {16, 32}, // 3
    {16, 64}, // 4
    {32, 64}, // 5
    {32, 64}, // 6
    {32, 32}, // 7
};


InfoWindow::InfoWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoWindow),
    ioPorts21(nullptr),
    ppu(nullptr),
    paletteData{0}
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

    QSettings settings;
    restoreGeometry(settings.value(SETTINGS_INFOWINDOW_GEOMETRY).toByteArray());

    connect(this, SIGNAL(SignalInfoWindowClosed()), parent, SLOT(SlotInfoWindowClosed()));
    connect(this, SIGNAL(SignalDrawFrame()), this, SLOT(SlotDrawFrame()));

    ClearWidgets();
    DrawFrame();
}


InfoWindow::~InfoWindow()
{
    delete ui;
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


void InfoWindow::ClearWidgets()
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

    ui->labelObjTable1Addr->setText("");
    ui->labelObjTable2Addr->setText("");
    ui->labelObjSizes->setText("");
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


void InfoWindow::GeneratePalette()
{
    if (ioPorts21 == nullptr || ppu == nullptr)
        return;

    // Convert BGR555 to ARGB8888
    for (int i = 0; i < 256; i++)
    {
        uint16_t color = Bytes::Make16Bit(ppu->cgram[(i * 2) + 1], ppu->cgram[i * 2]);

        uint8_t r = color & 0x1F;
        r = (r << 3) | ((r >> 2) & 0x07);

        color >>= 5;
        uint8_t g = color & 0x1F;
        g = (g << 3) | ((g >> 2) & 0x07);

        color >>= 5;
        uint8_t b = color & 0x1F;
        b = (b << 3) | ((b >> 2) & 0x07);

        paletteData[i] = 0xFF000000 | Bytes::Make24Bit(r, g, b);
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


void InfoWindow::UpdateMemoryView()
{
    if (ioPorts21 == nullptr || ppu == nullptr)
        return;

    uint8_t value;
    uint8_t value2;

    value = ioPorts21[eRegBGMODE & 0xFF];
    ui->labelVideoMode->setText(QString::number(value & 0x07));
    ui->labelBG1ChrSize->setText(Bytes::GetBit<4>(value) ? "16x16" : "8x8");
    ui->labelBG2ChrSize->setText(Bytes::GetBit<5>(value) ? "16x16" : "8x8");
    ui->labelBG3ChrSize->setText(Bytes::GetBit<6>(value) ? "16x16" : "8x8");
    ui->labelBG4ChrSize->setText(Bytes::GetBit<7>(value) ? "16x16" : "8x8");

    value = ioPorts21[eRegTM & 0xFF];
    value2 = ioPorts21[eRegTS & 0xFF];
    ui->labelBG1Enabled->setText((Bytes::GetBit<0>(value) ? "Main" : "") + QString(" ") + (Bytes::GetBit<0>(value2) ? "Sub" : ""));
    ui->labelBG2Enabled->setText((Bytes::GetBit<1>(value) ? "Main" : "") + QString(" ") + (Bytes::GetBit<1>(value2) ? "Sub" : ""));
    ui->labelBG3Enabled->setText((Bytes::GetBit<2>(value) ? "Main" : "") + QString(" ") + (Bytes::GetBit<2>(value2) ? "Sub" : ""));
    ui->labelBG4Enabled->setText((Bytes::GetBit<3>(value) ? "Main" : "") + QString(" ") + (Bytes::GetBit<3>(value2) ? "Sub" : ""));

    value = ioPorts21[eRegBG12NBA & 0xFF];
    ui->labelBG1Tileset->setText(UiUtils::FormatHexWord((value & 0x0F) << 13));
    ui->labelBG2Tileset->setText(UiUtils::FormatHexWord((value & 0xF0) << 9));

    value = ioPorts21[eRegBG34NBA & 0xFF];
    ui->labelBG3Tileset->setText(UiUtils::FormatHexWord((value & 0x0F) << 13));
    ui->labelBG4Tileset->setText(UiUtils::FormatHexWord((value & 0xF0) << 9));
    
    value = ioPorts21[eRegBG1SC & 0xFF];
    ui->labelBG1Tilemap->setText(UiUtils::FormatHexWord((value & 0xFC) << 9) +
                                 " h=" + QString::number(Bytes::GetBit<0>(value) + 1) +
                                 " v=" + QString::number(Bytes::GetBit<1>(value) + 1));

    value = ioPorts21[eRegBG2SC & 0xFF];
    ui->labelBG2Tilemap->setText(UiUtils::FormatHexWord((value & 0xFC) << 9) +
                                 " h=" + QString::number(Bytes::GetBit<0>(value) + 1) +
                                 " v=" + QString::number(Bytes::GetBit<1>(value) + 1));

    value = ioPorts21[eRegBG3SC & 0xFF];
    ui->labelBG3Tilemap->setText(UiUtils::FormatHexWord((value & 0xFC) << 9) +
                                 " h=" + QString::number(Bytes::GetBit<0>(value) + 1) +
                                 " v=" + QString::number(Bytes::GetBit<1>(value) + 1));

    value = ioPorts21[eRegBG4SC & 0xFF];
    ui->labelBG4Tilemap->setText(UiUtils::FormatHexWord((value & 0xFC) << 9) +
                                 " h=" + QString::number(Bytes::GetBit<0>(value) + 1) +
                                 " v=" + QString::number(Bytes::GetBit<1>(value) + 1));

    ui->labelBG1HOFS->setText(QString::number(ppu->bgHOffset[0]));
    ui->labelBG2HOFS->setText(QString::number(ppu->bgHOffset[1]));
    ui->labelBG3HOFS->setText(QString::number(ppu->bgHOffset[2]));
    ui->labelBG4HOFS->setText(QString::number(ppu->bgHOffset[3]));

    ui->labelBG1VOFS->setText(QString::number(ppu->bgVOffset[0]));
    ui->labelBG2VOFS->setText(QString::number(ppu->bgVOffset[1]));
    ui->labelBG3VOFS->setText(QString::number(ppu->bgVOffset[2]));
    ui->labelBG4VOFS->setText(QString::number(ppu->bgVOffset[3]));
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

    DrawSpriteTable(ppu->objBaseAddr[0], ui->gvObjTable1);
    DrawSpriteTable(ppu->objBaseAddr[1], ui->gvObjTable2);
}


void InfoWindow::DrawSpriteTable(uint16_t baseAddr, QGraphicsView *gv)
{
    if (ppu == nullptr || ioPorts21 == nullptr)
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
                uint16_t tileAddr = baseAddr + (tileId * 8  * 4); // 8px width * 4bpp;
                uint8_t pixelVal = ppu->GetTilePixelData(tileAddr, x, y, 4);
                img.setPixel(x, y, paletteData[pixelVal + 128]);
            }
        }

        QGraphicsPixmapItem *pixmap = gv->scene()->addPixmap(QPixmap::fromImage(img));
        pixmap->setScale(SCALE);
        int xpos = 1 + ((tileId % 16) * 8 * SCALE) + (tileId % 16);
        int ypos = 1 + ((tileId / 16) * 8 * SCALE) + (tileId / 16);
        pixmap->setPos(xpos, ypos);
    }
}


void InfoWindow::SlotDrawFrame()
{
    GeneratePalette();
    UpdateTileView();
    UpdatePaletteView();
    UpdateMemoryView();
    UpdateSpriteTab();
}