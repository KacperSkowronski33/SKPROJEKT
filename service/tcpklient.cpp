#include "tcpklient.h"

TCPKlient::TCPKlient(QObject *parent) :InterfejsSieciowy(parent)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &InterfejsSieciowy::polaczono);
    connect(m_socket, &QTcpSocket::disconnected, this, &InterfejsSieciowy::rozlaczono);
    connect(m_socket, &QTcpSocket::readyRead, this, [=]() {
        QByteArray dane = m_socket->readAll();
        emit daneOdebrane(dane);
    });
}

TCPKlient::~TCPKlient()
{
    if(m_socket->isOpen()) m_socket->close();
}

void TCPKlient::polacz(const QString &adres, int port)
{
    if(czyIpOk(adres) && czyPortOk(port)) {
        m_socket->connectToHost(adres, port);
    } else {
        emit blad("bledny port lub adres");
    }
}

void TCPKlient::rozlacz()
{
    m_socket->disconnectFromHost();
}

void TCPKlient::wyslijDane(const QByteArray &dane)
{
    if(m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(dane);
        m_socket->flush();
    } else emit blad("brak polaczenia");
}
