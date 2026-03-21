#include "PID.h"
#include <cmath>

PID::PID(double _k, double _TI, double _TD)
    : k(_k), TI(_TI), TD(_TD), typCalkowania(trybCalki::wew),
      sumaUchybu(0.0), poprzedniUchyb(0.0),
      lastP(0.0), lastI(0.0), lastD(0.0)
{
}

void PID::setK(double newK) { k = newK; }
void PID::setTI(double newTI) { TI = newTI; }
void PID::setTD(double newTD) { TD = newTD; }

void PID::setTryb(trybCalki nowyTryb)
{
    if (typCalkowania == nowyTryb)
        return;

    if (TI != 0.0)
    {
        if (typCalkowania == trybCalki::zew && nowyTryb == trybCalki::wew)
            sumaUchybu /= TI;
        else if (typCalkowania == trybCalki::wew && nowyTryb == trybCalki::zew)
            sumaUchybu *= TI;
    }
    typCalkowania = nowyTryb;
}

void PID::resetujSumeUchybu()
{
    sumaUchybu = 0.0;
    poprzedniUchyb = 0.0;
    lastI = 0.0;
}

double PID::oblicz(double uchyb, double dt)
{
    // Obliczenie P
    double uP = k * uchyb;

    // Obliczenie I
    double uI = 0.0;
    double Itemp = 0.0;
    if (TI != 0.0)
    {
        if (typCalkowania == trybCalki::zew)
        {
            sumaUchybu += uchyb;
            Itemp = sumaUchybu / TI;
        }
        else
        {
            sumaUchybu += (uchyb / TI);
            Itemp = sumaUchybu;
        }
    }

    uI = k * Itemp;

    // Obliczenie D
    double uD = k * TD * ((uchyb - poprzedniUchyb));
    poprzedniUchyb = uchyb;


    lastP = uP;
    lastI = uI;
    lastD = uD;

    return uP + uI + uD;
}
