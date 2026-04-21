#ifndef INTERFEJSSIECIOWY_H
#define INTERFEJSSIECIOWY_H

#include <QObject>
#include <QString>
#include <QByteArray>

class InterfejsSieciowy : public QObject
{
    Q_OBJECT
public:
    explicit InterfejsSieciowy(QObject *parent = nullptr) : QObject(parent){}
    virtual ~InterfejsSieciowy() {}

    virtual void polacz(const QString &adres, int port) = 0;
    virtual void rozlacz() = 0;
    virtual void wyslijDane(const QByteArray &dane) = 0;

protected:
    bool czyPortOk(int port);
    bool czyIpOk(const QString &adres);

signals:
    void polaczono();
    void rozlaczono();
    void daneOdebrane(const QByteArray &dane);
    void blad(const QString &blad);
};

#endif // INTERFEJSSIECIOWY_H
