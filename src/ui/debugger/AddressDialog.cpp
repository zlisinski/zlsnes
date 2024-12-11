#include "../UiUtils.h"
#include "AddressDialog.h"
#include "ui_AddressDialog.h"

AddressDialog::AddressDialog(QWidget *parent) :
    QDialog(parent),
    address(0),
    ui(new Ui::AddressDialog)
{
    ui->setupUi(this);
}


AddressDialog::~AddressDialog()
{
    delete ui;
}


void AddressDialog::accept()
{
    int val = ui->spinAddress->value();

    // The following sections are not code areas, and don't make sense to allow selection for disassembly.

    if (val >= 0x8000 && val < 0xA000)
    {
        UiUtils::MessageBox("Can't select VRAM");
        return;
    }

    if (val >= 0xE000 && val < 0xFE00)
    {
        UiUtils::MessageBox("Can't select ECHO");
        return;
    }

    if (val >= 0xFE00 && val < 0xFEA0)
    {
        UiUtils::MessageBox("Can't select OAM");
        return;
    }

    if (val >= 0xFEA0 && val < 0xFF00)
    {
        UiUtils::MessageBox("Can't select unused RAM");
        return;
    }

    if (val >= 0xFF00 && val < 0xFF80)
    {
        UiUtils::MessageBox("Can't select IO Registers");
        return;
    }

    address = val;

    QDialog::accept();
}
