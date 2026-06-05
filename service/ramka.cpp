#include "ramka.h"

QDataStream &operator<<(QDataStream &out, const Ramka &ramka) //operator do serializacji
{
    out << static_cast<qint8>(ramka.typ);

    switch(ramka.typ) {
    case TypRamki::DaneSymulacji:
        out << static_cast<qint8>(ramka.aktStan)
            << ramka.numerProbki
            << ramka.u
            << ramka.y
            << ramka.w;
               break;

    case TypRamki::ParametryARX:
    {
        out << static_cast<quint32>(ramka.wekA.size());
        for (double A : ramka.wekA) out << A;
        out << static_cast<quint32>(ramka.wekB.size());
        for (double B : ramka.wekB) out << B;
        out << ramka.opoznienie
            << ramka.ampSzum
            << ramka.umax
            << ramka.umin
            << ramka.ymax
            << ramka.ymin;
            //<< ramka.czyLim
            //<< ramka.czySzum;
        quint8 maskaFlagi = 0;
        if(ramka.czyLim) maskaFlagi |= (1 << 0);
        if(ramka.czySzum) maskaFlagi |= (1 << 1);
        out << maskaFlagi;

        break;
    }

    case TypRamki::ParametryPID:
        out << static_cast<qint8>(ramka.typCalki)
            << ramka.kp
            << ramka.ti
            << ramka.td;
        break;

    case TypRamki::ParametryGWZ:
        out << static_cast<qint8>(ramka.typSyg)
            << ramka.amplituda
            << ramka.okres
            << ramka.interwal
            << ramka.wypelnienie
            << ramka.skladowa_stala;
               break;
    }
    return out;
};


QDataStream &operator>>(QDataStream &in, Ramka &ramka) //operator do deserializacji
{
    qint8 typRamki;
    in >> typRamki;
    ramka.typ = static_cast<TypRamki>(typRamki);

    switch(ramka.typ)
    {
    case TypRamki::DaneSymulacji:
        qint8 stan;
        in >> stan
            >> ramka.numerProbki
            >> ramka.u
            >> ramka.y
            >> ramka.w;
        ramka.aktStan = static_cast<StanSymulacji>(stan);
        break;
    case TypRamki::ParametryARX:
    {
        quint32 sizeA, sizeB;
        double wart;

        in >> sizeA;
        ramka.wekA.clear();
        for(quint32 i = 0; i < sizeA; i++) {
            in >> wart;
            ramka.wekA.push_back(wart);
        }
        in >> sizeB;
        ramka.wekB.clear();
        for(quint32 i = 0; i < sizeB; i++) {
            in >> wart;
            ramka.wekB.push_back(wart);
        }
        in >> ramka.opoznienie
            >> ramka.ampSzum
            >> ramka.umax
            >> ramka.umin
            >> ramka.ymax
            >> ramka.ymin;
            // >> ramka.czyLim
            // >> ramka.czySzum;
        quint8 maskaFlagi;
        in >> maskaFlagi;
        ramka.czyLim = static_cast<bool>(maskaFlagi & (1 << 0));
        ramka.czySzum = static_cast<bool>(maskaFlagi & (1 << 1));
        break;
    }
    case TypRamki::ParametryPID:
        qint8 tryb;
        in >> tryb
            >> ramka.kp
            >> ramka.ti
            >> ramka.td;
        ramka.typCalki = static_cast<PID::trybCalki>(tryb);
        break;
    case TypRamki::ParametryGWZ:
        qint8 typSygnalu;
        in >> typSygnalu
            >> ramka.amplituda
            >> ramka.okres
            >> ramka.interwal
            >> ramka.wypelnienie
            >> ramka.skladowa_stala;
        ramka.typSyg = static_cast<TypSygnalu>(typSygnalu);
        break;
    }
    return in;
};
