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

    QString getIP() const;
    int getPort() const;
    bool getCzyLokalny() const;
    bool getCzySerwer() const;
    bool getCzyObiekt() const;

    void setIP(const QString &adres);
    void setPort(int port);
    void setCzyLokalny(bool czyLok);
    void setCzySerwer(bool czySer);
    void setCzyObiekt(bool czyObi);

    void statusPolaczono(const QString &adresip);

private slots:
    void on_btnZatwierdz_clicked();
    void on_btnAnuluj_clicked();


    void on_btnRozlacz_clicked();

signals:
    void sygnalPolacz(bool czySerwer, int port, QString adres);

private:
    Ui::UstawieniaSieci *ui;
    void zablokujKontrolki(bool zablokuj);

};

#endif // USTAWIENIASIECI_H
