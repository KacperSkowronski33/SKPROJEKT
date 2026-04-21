#include "ramka.h"

QDataStream &operator<<(QDataStream &out, const Ramka &ramka) //operator do serializacji
{
    out << static_cast<qint32>(ramka.typ);

    switch(ramka.typ) {
    case TypRamki::DaneSymulacji:
        out << static_cast<qint32>(ramka.aktStan)
            << ramka.numerProbki
            << ramka.interwal
            << ramka.u
            << ramka.y;
               break;

    case TypRamki::ParametryARX:
        out << static_cast<quint32>(ramka.wekA.size());
        for (double A : ramka.wekA) out << A;
        out << static_cast<quint32>(ramka.wekB.size());
        for (double B : ramka.wekB) out << B;
        out << ramka.opoznienie
            << ramka.ampSzum
            << ramka.umax
            << ramka.umin
            << ramka.ymax
            << ramka.ymin
            << ramka.czyLim
            << ramka.czySzum;
        break;

    case TypRamki::ParametryPID:
        out << static_cast<qint32>(ramka.typCalki)
            << ramka.kp
            << ramka.ti
            << ramka.td;
        break;

    case TypRamki::ParametryGWZ:
        out << static_cast<qint32>(ramka.typSyg)
            << ramka.amplituda
            << ramka.okres
            << ramka.wypelnienie;
               break;
    }
    return out;
};


QDataStream &operator>>(QDataStream &in, Ramka &ramka) //operator do deserializacji
{
    qint32 typRamki;
    in >> typRamki;
    ramka.typ = static_cast<TypRamki>(typRamki);

    switch(ramka.typ)
    {
    case TypRamki::DaneSymulacji:
        qint32 stan;
        in >> stan
            >> ramka.numerProbki
            >> ramka.interwal
            >> ramka.u
            >> ramka.y;
        ramka.aktStan = static_cast<StanSymulacji>(stan);
        break;
    case TypRamki::ParametryARX:
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
            >> ramka.ymin
            >> ramka.czyLim
            >> ramka.czySzum;
        break;
    case TypRamki::ParametryPID:
        qint32 tryb;
        in >> tryb
            >> ramka.kp
            >> ramka.ti
            >> ramka.td;
        ramka.typCalki = static_cast<PID::trybCalki>(tryb);
        break;
    case TypRamki::ParametryGWZ:
        qint32 typSygnalu;
        in >> typSygnalu
            >> ramka.amplituda
            >> ramka.okres
            >> ramka.wypelnienie;
        ramka.typSyg = static_cast<TypSygnalu>(typSygnalu);
        break;
    }
    return in;
};
