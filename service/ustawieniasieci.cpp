#include "ustawieniasieci.h"
#include "ui_ustawieniasieci.h"

UstawieniaSieci::UstawieniaSieci(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UstawieniaSieci)
{
    ui->setupUi(this);

    connect(ui->radioSerwer, &QRadioButton::toggled, this, [=](bool serwer) {
        ui->kontenerIP->setVisible(!serwer);
    });
}

UstawieniaSieci::~UstawieniaSieci()
{
    delete ui;
}

void UstawieniaSieci::on_btnZatwierdz_clicked()
{
    this->close();
}


void UstawieniaSieci::on_btnAnuluj_clicked()
{

}

