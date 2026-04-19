#ifndef INTERFEJSSIECIOWY_H
#define INTERFEJSSIECIOWY_H

#include <QObject>
#include <QString>
#include <QByteArray>

class InterfejsSieciowy : public QObject
{
    Q_OBJECT
public:
    InterfejsSieciowy(QObject *parent = nullptr) : QObject(parent){}
    virtual ~InterfejsSieciowy() {}

    virtual void polacz(const QString &adres, int port);
    virtual void rozlacz();
    virtual void wyslijDane(const QByteArray &dane);

signals:
    void polaczono();
    void rozlaczono();
    void daneOdebrane(const QByteArray &dane);
};

#endif // INTERFEJSSIECIOWY_H
