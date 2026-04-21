#ifndef TCPKLIENT_H
#define TCPKLIENT_H
#include "interfejssieciowy.h"
#include <QTcpSocket>

class TCPKlient : public InterfejsSieciowy
{
    Q_OBJECT
public:
    explicit TCPKlient(QObject *parent);
    ~TCPKlient() override;

    void polacz(const QString &adres, int port) override;
    void rozlacz() override;
    void wyslijDane(const QByteArray &dane) override;


private:
    QTcpSocket *m_socket;

};

#endif // TCPKLIENT_H
