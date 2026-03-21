#include "ARX.h"
#include <iostream>
#include <numeric>
#include <cassert>

ARX::ARX(vector<double> _a, vector<double> _b, int _k) : a(_a), b(_b), k(_k), generator(std::random_device{}()), stanLimitow(true),
                                                         stanSzumu(false), distribution{0.0, 0.01}, uMin(-10.0), uMax(10.0), yMin(-10.0), yMax(10.0)
{
    assert(a.size() >= 3 && "Wektor a musi miec co najmniej 3 elementy");
    assert(b.size() >= 3 && "Wektor b musi miec co najmniej 3 elementy");
    yHist.assign(a.size(), 0.0);

    uHist.assign(b.size(), 0.0);

    kBuffer.assign(k, 0.0);
}

double ARX::symuluj(double u)
{
    if (stanLimitow)
        u = zastosujLimity(uMin, uMax, u);

    kBuffer.push_front(u);
    double uDelayed = kBuffer.back();
    kBuffer.pop_back();

    uHist.push_front(uDelayed);
    uHist.pop_back();

    // obliczenie czesci przy B
    double y = std::inner_product(b.begin(), b.end(), uHist.begin(), 0.0);

    // obliczenie czesci przy A
    y -= std::inner_product(a.begin(), a.end(), yHist.begin(), 0.0);

    if (stanSzumu == true)
    {
        y += generujSzum();
    }

    if (stanLimitow)
        y = zastosujLimity(yMin, yMax, y);

    yHist.push_front(y);
    yHist.pop_back();
    return y;
}

void ARX::ustawAmplitudeSzumu(double amp)
{

    if(amp < 0) amp = 0; // Zabezpieczenie
    distribution = std::normal_distribution<double>(0.0, amp);
}

double ARX::generujSzum()
{
    return distribution(generator);
}

double ARX::zastosujLimity(double min, double max, double wartosc)
{
    if (wartosc < min)
        wartosc = min;
    if (wartosc > max)
        wartosc = max;
    return wartosc;
}

void ARX::ustawParametry(vector<double> newA, vector<double> newB, int newK)
{
    a = newA;
    yHist.resize(a.size(), 0.0);

    b = newB;
    uHist.resize(b.size(), 0.0);

    k = newK;
    kBuffer.resize(k, 0.0);
}

void ARX::ustawA(vector<double> newA)
{
    a = newA;
    yHist.resize(a.size(), 0.0);
}

void ARX::ustawB(vector<double> newB)
{
    b = newB;
    uHist.resize(b.size(), 0.0);
}

void ARX::ustawOpoznienie(int newK)
{
    k = newK;
    kBuffer.resize(k,0.0);
}

void ARX::przelaczLimity(bool stan)
{
    stanLimitow = stan;
}

void ARX::przelaczSzum(bool stan)
{
    stanSzumu = stan;
}

void ARX::ustawLimitWejscia(double newUMIN, double newUMAX)
{
    uMin = newUMIN;
    uMax = newUMAX;
}

void ARX::ustawLimitWyjscia(double newYMIN, double newYMAX)
{
    yMin = newYMIN;
    yMax = newYMAX;
}
