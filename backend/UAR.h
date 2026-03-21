#pragma once
//
#include "PID.h"
#include "ARX.h"
#include "GWZ.h"

class UAR
{
private:
    ARX model;
    PID pid;
    GeneratorWartosciZadanej GWZ;
    double y_prev;

public:
    UAR(const ARX &m, const PID &r)
        : model(m),
          pid(r),
          y_prev(0.0)
    {
        model.przelaczLimity(false);
        model.przelaczSzum(false);
    }

    UAR(const ARX &m, const PID &r, const GeneratorWartosciZadanej &g)
        : model(m),
          pid(r),
          GWZ(g),
          y_prev(0.0)
    {
        model.przelaczLimity(false);
        model.przelaczSzum(false);
    }

    ARX& getARX() { return model; }
    PID& getPID() { return pid; }
    GeneratorWartosciZadanej& getGWZ() { return GWZ; }

    double symuluj(double zadanie);
    double symuluj();
};
