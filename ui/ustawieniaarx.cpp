#include "ustawieniaarx.h"
#include "ui_ustawieniaarx.h"
#include <QMessageBox>

UstawieniaARX::UstawieniaARX(QWidget *parent) : QDialog(parent),
                                                ui(new Ui::UstawieniaARX)
{
    ui->setupUi(this);
}

UstawieniaARX::~UstawieniaARX()
{
    delete ui;
}

std::vector<double> UstawieniaARX::parsujWektor(QString tekst) const
{
    std::vector<double> wynik;
    QStringList lista = tekst.split(',');

    for (const QString &str : lista)
    {
        bool ok;
        double val = str.trimmed().toDouble(&ok);
        if (ok)
        {
            wynik.push_back(val);
        }
    }
    return wynik;
}

QString UstawieniaARX::wektorNaTekst(const std::vector<double> &vec)
{
    QStringList lista;
    for (double v : vec)
        lista << QString::number(v);
    return lista.join(", ");
}

//SETTERY
void UstawieniaARX::setWielomianA(const std::vector<double> &a) { ui->leA->setText(wektorNaTekst(a)); }
void UstawieniaARX::setWielomianB(const std::vector<double> &b) { ui->leB->setText(wektorNaTekst(b)); }
void UstawieniaARX::setK(int k) { ui->spinK->setValue(k); }
void UstawieniaARX::setSzumAmp(double s) { ui->spinSzum->setValue(s); }

void UstawieniaARX::setSzumAktywny(bool akt) { ui->checkSzum->setChecked(akt); }

void UstawieniaARX::setLimityAktywne(bool aktywne)
{
    ui->checkLimity->setChecked(aktywne);
}

void UstawieniaARX::setLimityWejscia(double min, double max)
{
    ui->spinUmin->setValue(min);
    ui->spinUmax->setValue(max);
}

void UstawieniaARX::setLimityWyjscia(double min, double max)
{
    ui->spinYmin->setValue(min);
    ui->spinYmax->setValue(max);
}

//GETTERY
std::vector<double> UstawieniaARX::getA() const { return parsujWektor(ui->leA->text()); }
std::vector<double> UstawieniaARX::getB() const { return parsujWektor(ui->leB->text()); }
int UstawieniaARX::getK() const { return ui->spinK->value(); }
double UstawieniaARX::getSzumAmp() const { return ui->spinSzum->value(); }
bool UstawieniaARX::getSzumAktywny() const { return ui->checkSzum->isChecked(); }
bool UstawieniaARX::getLimityAktywne() const { return ui->checkLimity->isChecked(); }
double UstawieniaARX::getUmin() const { return ui->spinUmin->value(); }
double UstawieniaARX::getUmax() const { return ui->spinUmax->value(); }
double UstawieniaARX::getYmin() const { return ui->spinYmin->value(); }
double UstawieniaARX::getYmax() const { return ui->spinYmax->value(); }

//AKCJE PRZYCISKÓW
void UstawieniaARX::on_btnZatwierdz_clicked()
{
    
    std::vector<double> a = getA();
    std::vector<double> b = getB();

    if (a.size() < 3 || b.size() < 3)
    {
        QMessageBox::warning(this, "Błąd", "Wielomiany muszą mieć min. 3 współczynniki!");
        return;
    }

    // Jeśli wszystko OK, zamykamy okno z wynikiem Accepted
    accept();
}

void UstawieniaARX::on_btnAnuluj_clicked()
{
    reject(); // Zamykamy z wynikiem Rejected
}
