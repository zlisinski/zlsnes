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


InfoWindow::InfoWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoWindow),
    ioPorts21(nullptr),
    vram(nullptr),
    oam(nullptr),
    cgram(nullptr),
    paletteData{0}
{
    ui->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->gvTiles->setScene(scene);
    scene = new QGraphicsScene(this);
    ui->gvPalette->setScene(scene);

    QSettings settings;
    restoreGeometry(settings.value(SETTINGS_INFOWINDOW_GEOMETRY).toByteArray());

    connect(this, SIGNAL(SignalInfoWindowClosed()), parent, SLOT(SlotInfoWindowClosed()));
    connect(this, SIGNAL(SignalDrawFrame()), this, SLOT(SlotDrawFrame()));

    ClearCartridgeInfo();
    DrawFrame();

    // Use a timer for updating the tilemap images until I get actual frames rendering.
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(SlotDrawFrame()));
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
    timer->start(100);

    QWidget::showEvent(event);
}


void InfoWindow::closeEvent(QCloseEvent *event)
{
    // No need to keep updating if the window isn't open.
    timer->stop();

    QSettings settings;
    settings.setValue(SETTINGS_INFOWINDOW_GEOMETRY, saveGeometry());

    emit SignalInfoWindowClosed();

    QWidget::closeEvent(event);
}


void InfoWindow::ClearCartridgeInfo()
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
    if (ioPorts21 == nullptr || vram == nullptr || oam == nullptr || cgram == nullptr)
        return;

    // Convert BGR555 to ARGB8888
    for (int i = 0; i < 256; i++)
    {
        uint16_t color = Bytes::Make16Bit(cgram[(i * 2) + 1], cgram[i * 2]);

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
    if (ioPorts21 == nullptr || vram == nullptr || oam == nullptr || cgram == nullptr)
        return;

    const int SCALE = 3;
    const uint16_t bg1TileOffset = (ioPorts21[eRegBG12NBA & 0xFF] & 0x0F) << 13;
    const uint8_t *tilesetData = &vram[bg1TileOffset];

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
    if (ioPorts21 == nullptr || vram == nullptr || oam == nullptr || cgram == nullptr)
        return;

    ui->labelVideoMode->setText(QString::number(ioPorts21[eRegBGMODE & 0xFF] & 0x07));
}


void InfoWindow::SlotDrawFrame()
{
    GeneratePalette();
    UpdateTileView();
    UpdatePaletteView();
    UpdateMemoryView();
}