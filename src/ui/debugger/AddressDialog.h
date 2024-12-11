#pragma once

#include <QDialog>

namespace Ui {
class AddressDialog;
}

class AddressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddressDialog(QWidget *parent = 0);
    ~AddressDialog();

    virtual void accept();

    uint32_t address;

private:
    Ui::AddressDialog *ui;
};
