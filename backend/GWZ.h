#pragma once
#include <cmath>

using namespace std;

constexpr double PI = 3.14159265358979323846;

enum class TypSygnalu
{
    Sinusoida,
    SygnalProstokatny
};

class GeneratorWartosciZadanej
{
private:
    TypSygnalu typ;

    double A; // Amplituda
    double T; // Okres w probkach
    double S; // Skladowa stala
    double p; // Wypelnienie (0..1) dla prostokata
    double TRZ;
    int TT;

    int i = 0; // Licznik probek

public:
    GeneratorWartosciZadanej() = default;
    GeneratorWartosciZadanej(double, double, double, double, int, TypSygnalu);

    // SETTERY
    void setAmplituda(double);
    void setOkres(double);
    void setStala(double);
    void setWypelnienie(double);
    void setTyp(TypSygnalu);
    void setTRZ(double);
    void setTT(int);

    // GETTERY
    double getAmplituda() const;
    double getOkres() const;
    double getStala() const;
    double getWypelnienie() const;
    TypSygnalu getTyp() const;

    // G��WNA METODA � ZGODNIE ZE WZOREM
    double obliczSygnal();
    void przeliczOkresDyskretny();
    // RESET LICZNIKA
    void reset();
};
