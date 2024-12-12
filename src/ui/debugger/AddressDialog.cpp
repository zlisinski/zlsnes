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

    if ((val & 0x408000) == 0)
    {
        UiUtils::MessageBox("Can't select System Area");
        return;
    }

    if (val >= 0x7E0000 && val <= 0x7FFFFF)
    {
        UiUtils::MessageBox("Can't select WRAM");
        return;
    }

    address = val;

    QDialog::accept();
}
