#include "interfejssieciowy.h"
#include <QHostAddress>

bool InterfejsSieciowy::czyPortOk(int port)
{
    return (port > 0 && port < 65535);
}

bool InterfejsSieciowy::czyIpOk(const QString &adres)
{
    QHostAddress test;
    return test.setAddress(adres);
}
