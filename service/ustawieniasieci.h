#ifndef USTAWIENIASIECI_H
#define USTAWIENIASIECI_H

#include <QDialog>

namespace Ui {
class UstawieniaSieci;
}

class UstawieniaSieci : public QDialog
{
    Q_OBJECT

public:
    explicit UstawieniaSieci(QWidget *parent = nullptr);
    ~UstawieniaSieci();

private slots:
    void on_btnZatwierdz_clicked();

    void on_btnAnuluj_clicked();

private:
    Ui::UstawieniaSieci *ui;
};

#endif // USTAWIENIASIECI_H
