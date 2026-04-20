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

    connect(ui->btnPolacz, &QPushButton::clicked, this, [=](){
        bool serwer = this->getCzySerwer();
        QString adres = "";
        if(!serwer) adres = this->getIP();
        emit sygnalPolacz(serwer, this->getPort(), adres);
    });
}

UstawieniaSieci::~UstawieniaSieci()
{
    delete ui;
}

QString UstawieniaSieci::getIP() const
{
    return ui->poleIP1->toPlainText().trimmed() + "." +
            ui->poleIP2->toPlainText().trimmed() + "." +
            ui->poleIP3->toPlainText().trimmed() + "." +
           ui->poleIP4->toPlainText().trimmed();
}

int UstawieniaSieci::getPort() const
{
    return ui->polePort->toPlainText().trimmed().toInt();
}

bool UstawieniaSieci::getCzyLokalny() const
{
    return ui->radioLokalny->isChecked();
}

bool UstawieniaSieci::getCzySerwer() const
{
    return ui->radioSerwer->isChecked();
}

bool UstawieniaSieci::getCzyObiekt() const
{
    return ui->radioObiekt->isChecked();
}

void UstawieniaSieci::setIP(const QString &adres)
{
    QStringList adr = adres.split(".");
    if(adr.size() == 4) {
        ui->poleIP1->setPlainText(adr[0]);
        ui->poleIP2->setPlainText(adr[1]);
        ui->poleIP3->setPlainText(adr[2]);
        ui->poleIP4->setPlainText(adr[3]);
    } else {
        ui->poleIP1->setPlainText("127");
        ui->poleIP2->setPlainText("0");
        ui->poleIP3->setPlainText("0");
        ui->poleIP4->setPlainText("1");
    }
}

void UstawieniaSieci::setPort(int port)
{
    ui->polePort->setPlainText(QString::number(port));
}

void UstawieniaSieci::setCzyLokalny(bool czyLok)
{
    if(czyLok) {
        ui->radioLokalny->setChecked(true);
    } else {
        ui->radioSieciowy->setChecked(true);
    }
}

void UstawieniaSieci::setCzySerwer(bool czySer)
{
    if(czySer) {
        ui->radioSerwer->setChecked(true);
    } else
    {
        ui->radioKlient->setChecked(true);
    }
}

void UstawieniaSieci::setCzyObiekt(bool czyObi)
{
    if(czyObi) {
        ui->radioObiekt->setChecked(true);
    } else {
        ui->radioRegulator->setChecked(true);
    }
}

void UstawieniaSieci::statusPolaczono(const QString &adresip)
{
    ui->stanPolaczenia->setText("Połączono z " + adresip);
}

void UstawieniaSieci::on_btnZatwierdz_clicked()
{

    this->accept();
}


void UstawieniaSieci::on_btnAnuluj_clicked()
{
    this->reject();
}


void UstawieniaSieci::on_btnRozlacz_clicked()
{
    ui->stanPolaczenia->setText("Rozłączono");
}

