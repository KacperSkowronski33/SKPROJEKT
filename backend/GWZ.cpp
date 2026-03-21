#include "GWZ.h"
#include <cmath>
#include <algorithm>

GeneratorWartosciZadanej::GeneratorWartosciZadanej(double amplituda, double okresRzeczywisty, double stala,
                                                   double wypelnienie, int TTms,
                                                   TypSygnalu typsygnalu)
    : A(amplituda), S(stala), p(wypelnienie), TRZ(okresRzeczywisty), TT(TTms),
      typ(typsygnalu), i(0)
{
    przeliczOkresDyskretny();
}
void GeneratorWartosciZadanej::setAmplituda(double ampliduda) { A = ampliduda; }
void GeneratorWartosciZadanej::setOkres(double okres) { T = okres; }
void GeneratorWartosciZadanej::setStala(double stala) { S = stala; }
void GeneratorWartosciZadanej::setWypelnienie(double wyp) { p = wyp; }
void GeneratorWartosciZadanej::setTyp(TypSygnalu typsygnalu) { GeneratorWartosciZadanej::typ = typsygnalu; }

void GeneratorWartosciZadanej::setTRZ(double okresRzeczywisty)
{
    TRZ = okresRzeczywisty;
    przeliczOkresDyskretny();
}

void GeneratorWartosciZadanej::setTT(int TTms)
{
    TT = TTms;
    przeliczOkresDyskretny();
}

double GeneratorWartosciZadanej::getAmplituda() const { return A; }
double GeneratorWartosciZadanej::getOkres() const { return T; }
double GeneratorWartosciZadanej::getStala() const { return S; }
double GeneratorWartosciZadanej::getWypelnienie() const { return p; }
TypSygnalu GeneratorWartosciZadanej::getTyp() const { return GeneratorWartosciZadanej::typ; }

double GeneratorWartosciZadanej::obliczSygnal()
{
    if (T <= 0)
        return S; // Zabezpieczenie

    int modT = i % (int)T;
    double Wi = 0.0;

    if (typ == TypSygnalu::Sinusoida)
    {
        // w_i = A * sin( (i mod T)/T * 2π ) + S
        double argument = (double)modT / T * 2.0 * PI;
        Wi = A * sin(argument) + S;
    }
    else if (typ == TypSygnalu::SygnalProstokatny)
    {

        if (modT < p * T)
            Wi = A + S;
        else
            Wi =  - A + S;
    }

    i++; // przechodzimy do nastepnej probki
    return Wi;
}

void GeneratorWartosciZadanej::przeliczOkresDyskretny()
{
    if (TT <= 0 || TRZ <= 0)
    {
        T = 1; // minimalny okres
        return;
    }

    double Td = TRZ * 1000.0 / TT;
    T = max(1, (int)std::round(Td));
}

void GeneratorWartosciZadanej::reset() { i = 0; }
