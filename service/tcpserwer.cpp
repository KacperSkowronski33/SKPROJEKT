#include "tcpserwer.h"

TCPSerwer::TCPSerwer(QObject *parent) :InterfejsSieciowy(parent) {
    m_serwer = new QTcpServer(this);
    m_klient = nullptr;

    connect(m_serwer, &QTcpServer::newConnection, this, [=](){
        if(m_klient) {
            QTcpSocket *nowePolaczenie = m_serwer->nextPendingConnection();
            nowePolaczenie->disconnectFromHost();
            nowePolaczenie->deleteLater();
            return;
        }
        m_klient = m_serwer->nextPendingConnection();
        connect(m_klient, &QTcpSocket::disconnected, this, [=](){
            emit rozlaczono();
            m_klient->deleteLater();
            m_klient = nullptr;
        });

        connect(m_klient, &QTcpSocket::readyRead, this, [=](){
            QByteArray dane = m_klient->readAll();
            emit daneOdebrane(dane);
        });
        emit polaczono();
    });

}

void TCPSerwer::polacz(const QString &adres, int port) //metoda do startu nasluchiwania, nazwa mylaca ale przynajmniej jest polimorfizm
{
    if(!czyPortOk(port)) {
        emit blad("bledny port");
        return;
    }
    if(!m_serwer->listen(QHostAddress::Any, port)) emit blad("nie udalo sie uruchomic serwera");
}


void TCPSerwer::rozlacz()
{
    if(m_klient) m_klient->disconnectFromHost();
    if(m_serwer->isListening()) m_serwer->close();
}

TCPSerwer::~TCPSerwer()
{
    TCPSerwer::rozlacz();
}

void TCPSerwer::wyslijDane(const QByteArray &dane)
{
    if(m_klient && m_klient->state() == QAbstractSocket::ConnectedState) {
        m_klient->write(dane);
    } else emit blad("brak klienta");
}
