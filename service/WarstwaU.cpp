#include "WarstwaU.h"

// ------------------- KONSTRUKTOR I DESTRUKTOR -------------------

WarstwaU::WarstwaU(QObject *parent) : QObject(parent)
{
    // Inicjalizacja obiektów logicznych
    std::vector<double> A = {-0.4, 0.1, -0.05};
    std::vector<double> B = {0.6, 0.2, 0.1};
    int k = 1;

    ARX model(A, B, k);
    PID regulator(1.0, 50.0, 0.1);
    GeneratorWartosciZadanej gen(1.0, 10.0, 0.0, 0.5, 200, TypSygnalu::SygnalProstokatny);

    symulator = new UAR(model, regulator, gen);

    // Ustawienia domyślne
    symulator->getARX().ustawLimitWejscia(-10.0, 10.0);
    symulator->getARX().ustawLimitWyjscia(-10.0, 10.0);
    symulator->getARX().przelaczLimity(true);

    // --- KONFIGURACJA TIMERA ---
    zegarSymulacji = new QTimer(this);


    connect(zegarSymulacji, &QTimer::timeout, this, &WarstwaU::zadanieOdswiezenia);
}

WarstwaU::~WarstwaU()
{
    if (symulator) delete symulator;

}

// ------------------- STEROWANIE TIMEREM -------------------

void WarstwaU::startSymulacji(int interwalMs)
{
    if (interwalMs < 10) interwalMs = 10;
    zegarSymulacji->start(interwalMs);
}

void WarstwaU::stopSymulacji()
{
    zegarSymulacji->stop();
}

void WarstwaU::setInterwalSymulacji(int ms)
{
    if (ms >= 10 && zegarSymulacji->interval() != ms)
    {
        zegarSymulacji->setInterval(ms);
    }
}

bool WarstwaU::czySymulacjaDziala() const
{
    return zegarSymulacji->isActive();
}

double WarstwaU::getInterwalSekundy() const
{
    return zegarSymulacji->interval() / 1000.0;
}


void WarstwaU::setArxA(const std::vector<double> &a) { symulator->getARX().ustawA(a); }
void WarstwaU::setArxB(const std::vector<double> &b) { symulator->getARX().ustawB(b); }
void WarstwaU::setArxK(int k) { symulator->getARX().ustawOpoznienie(k); }
void WarstwaU::setArxDelay(int k) { symulator->getARX().ustawOpoznienie(k); }

// --- Implementacja brakujących getterów ARX do zapisu ---
double WarstwaU::getArxUMin() { return symulator->getARX().getUMin(); }
double WarstwaU::getArxUMax() { return symulator->getARX().getUMax(); }
double WarstwaU::getArxYMin() { return symulator->getARX().getYMin(); }
double WarstwaU::getArxYMax() { return symulator->getARX().getYMax(); }
bool WarstwaU::getArxLimitsActive() { return symulator->getARX().getLimityAktywne(); }
bool WarstwaU::getArxNoiseActive() { return symulator->getARX().getSzumAktywny(); }
double WarstwaU::getArxNoise() { return symulator->getARX().getSzum(); }

void WarstwaU::setArxLimits(double uMin, double uMax, double yMin, double yMax) {
    symulator->getARX().ustawLimitWejscia(uMin, uMax);
    symulator->getARX().ustawLimitWyjscia(yMin, yMax);
}
void WarstwaU::setArxInputLimit(double Min, double Max) { symulator->getARX().ustawLimitWejscia(Min, Max); }
void WarstwaU::setArxOutputLimit(double Min, double Max) { symulator->getARX().ustawLimitWyjscia(Min, Max); }
void WarstwaU::toggleArxLimits(bool stan) { symulator->getARX().przelaczLimity(stan); }
void WarstwaU::toggleArxNoise(bool stan) { symulator->getARX().przelaczSzum(stan); }
void WarstwaU::setArxNoiseAmplitude(double amp) { symulator->getARX().ustawAmplitudeSzumu(amp);}
double WarstwaU::calculateARX(double u) { return symulator->getARX().symuluj(u); }
std::vector<double> WarstwaU::getArxA() { return symulator->getARX().getA(); }
std::vector<double> WarstwaU::getArxB() { return symulator->getARX().getB(); }
int WarstwaU::getArxK() { return symulator->getARX().getK(); }

// GWZ
void WarstwaU::setGwzAmplitude(double A) { symulator->getGWZ().setAmplituda(A); }
void WarstwaU::setGwzPeriod(double T) { symulator->getGWZ().setTRZ(T); }
void WarstwaU::setGwzStala(double S) { symulator->getGWZ().setStala(S); }
void WarstwaU::setGwzWypelnienie(double p) { symulator->getGWZ().setWypelnienie(p); }
void WarstwaU::setGwzType(TypSygnalu typ) { symulator->getGWZ().setTyp(typ); }
void WarstwaU::setGwzTT(double tt) { symulator->getGWZ().setTT(tt); }
double WarstwaU::generateGwz() { return symulator->getGWZ().obliczSygnal(); }
void WarstwaU::resetGwz() { symulator->getGWZ().reset(); }

// PID
void WarstwaU::setPidK(double k) { symulator->getPID().setK(k); }
void WarstwaU::setPidTI(double TI) { symulator->getPID().setTI(TI); }
void WarstwaU::setPidTD(double TD) { symulator->getPID().setTD(TD); }
double WarstwaU::PIDgetP() { return symulator->getPID().getP(); }
double WarstwaU::PIDgetI() { return symulator->getPID().getI(); }
double WarstwaU::PIDgetD() { return symulator->getPID().getD(); }
void WarstwaU::setPidMode(PID::trybCalki tryb) { symulator->getPID().setTryb(tryb); }
PID::trybCalki WarstwaU::getPidMode() { return symulator->getPID().getTryb(); }
void WarstwaU::resetPid() { symulator->getPID().resetujSumeUchybu(); }
double WarstwaU::calculatePID(double uchyb, double dt) { return symulator->getPID().oblicz(uchyb, dt); }

// UAR
double WarstwaU::simulateUAR(double zadanie) { return symulator->symuluj(zadanie); }
