#include <memory>
#include <QtCore/QSettings>
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
    memory(NULL)
{
    ui->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->gvTiles->setScene(scene);

    QSettings settings;
    restoreGeometry(settings.value(SETTINGS_INFOWINDOW_GEOMETRY).toByteArray());

    connect(this, SIGNAL(SignalInfoWindowClosed()), parent, SLOT(SlotInfoWindowClosed()));
    connect(this, SIGNAL(SignalDrawFrame()), this, SLOT(SlotDrawFrame()));

    ClearCartridgeInfo();
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


void InfoWindow::closeEvent(QCloseEvent *event)
{
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


void InfoWindow::UpdateTileView()
{

}


void InfoWindow::UpdateMemoryView()
{

}


void InfoWindow::SlotDrawFrame()
{
    UpdateTileView();
    UpdateMemoryView();
}