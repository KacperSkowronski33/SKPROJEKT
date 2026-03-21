#pragma once

class PID
{
public:
    enum class trybCalki
    {
        zew,
        wew
    };

private:
    double k;
    double TI;
    double TD;
    trybCalki typCalkowania;
    double sumaUchybu;
    double poprzedniUchyb;
    double lastFilteredD = 0.0;
    double lastP;
    double lastI;
    double lastD;

public:
    PID(double, double, double);

    void setK(double);
    void setTI(double);
    void setTD(double);
    void setTryb(trybCalki);

    void resetujSumeUchybu();

    double oblicz(double uchyb, double dt = 0.2);

    // --- NOWE GETTERY ---
    double getP() const { return lastP; }
    double getI() const { return lastI; }
    double getD() const { return lastD; }
    // --- NOWE GETTERY DO ZAPISU ---
    trybCalki getTryb() const { return typCalkowania; }
};
