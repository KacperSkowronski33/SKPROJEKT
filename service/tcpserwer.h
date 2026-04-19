#ifndef TCPSERWER_H
#define TCPSERWER_H

#include "interfejssieciowy.h"
#include <QTcpServer>
#include <QTcpSocket>

class TCPSerwer : public InterfejsSieciowy
{
    Q_OBJECT
public:
    explicit TCPSerwer(QObject *parent = nullptr);
    ~TCPSerwer() override;

    void polacz(const QString &adres, int port) override;
    void rozlacz() override;
    void wyslijDane(const QByteArray &dane) override;

private:
    QTcpServer *m_serwer;
    QTcpSocket *m_klient;
};

#endif // TCPSERWER_H
