#ifndef RAMKA_H
#define RAMKA_H

#include "PID.h"
#include "GWZ.h"
#include <vector>
#include <QDataStream>

enum class TypRamki
{
    DaneSymulacji,
    //SygnalRegulator,
    //SygnalObiekt,
    ParametryARX,
    ParametryPID,
    ParametryGWZ
};

enum class StanSymulacji
{
    Stop,
    Start,
    Reset
};

struct Ramka {
    TypRamki typ;

    //stan
    StanSymulacji aktStan = StanSymulacji::Stop;
    unsigned int numerProbki = 0;
    int interwal = 0;
    double u = 0.0;
    double y = 0.0;

    //arx
    std::vector<double> wekA;
    std::vector<double> wekB;
    int opoznienie = 0;
    double ampSzum = 0.0;
    double umax = 0.0;
    double umin = 0.0;
    double ymax = 0.0;
    double ymin = 0.0;
    bool czyLim = false;
    bool czySzum = false;

    //pid
    PID::trybCalki typCalki = PID::trybCalki::wew;
    double kp = 0.0;
    double ti = 0.0;
    double td = 0.0;

    //gwz
    TypSygnalu typSyg = TypSygnalu::SygnalProstokatny;
    double amplituda = 0.0;
    double okres = 0.0;
    double wypelnienie = 0.0;

};

QDataStream &operator<<(QDataStream &out, const Ramka &ramka);
QDataStream &operator>>(QDataStream &in, Ramka &ramka);

#endif // RAMKA_H
