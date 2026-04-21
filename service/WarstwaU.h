#pragma once
#include <QObject>
#include <QTimer>
#include <vector>
#include "ARX.h"
#include "GWZ.h"
#include "PID.h"
#include "UAR.h"
#include "interfejssieciowy.h"
#include "ramka.h"


class WarstwaU : public QObject
{
    Q_OBJECT

private:
    UAR *symulator;
    QTimer *zegarSymulacji;
    InterfejsSieciowy *m_siec;
    //QByteArray m_buforSieciowy;



public:
    explicit WarstwaU(QObject *parent = nullptr);
    ~WarstwaU();

    // --- STEROWANIE SYMULACJĄ ---
    void startSymulacji(int interwalMs);
    void stopSymulacji();
    void setInterwalSymulacji(int ms);
    bool czySymulacjaDziala() const;
    double getInterwalSekundy() const;

    //siec
    void wlaczTrybSieciowy(bool serwer, int port ,const QString &adres = "127.0.0.1");
    void wyslijRamke(const Ramka &ramka);


signals:
    void zadanieOdswiezenia();
    void infoPolaczono();
    void ramkaOdebrana(const Ramka &ramka);

private slots:
    void on_daneOdebrane(const QByteArray &dane);

public:

    // --- ARX ---
    void setArxA(const std::vector<double> &a);
    void setArxB(const std::vector<double> &b);
    void setArxK(int k);
    void setArxDelay(int k);
    void setArxLimits(double uMin, double uMax, double yMin, double yMax);
    void setArxInputLimit(double Min, double Max);
    void setArxOutputLimit(double Min, double Max);
    void toggleArxLimits(bool stan);
    void toggleArxNoise(bool stan);
    void setArxNoiseAmplitude(double amp);
    double calculateARX(double u);
    std::vector<double> getArxA();
    std::vector<double> getArxB();
    int getArxK();
    double getArxNoise();
    bool getArxLimitsActive();
    bool getArxNoiseActive();
    double getArxUMin();
    double getArxUMax();
    double getArxYMin();
    double getArxYMax();

    // --- GWZ ---
    void setGwzAmplitude(double A);
    void setGwzPeriod(double T);
    void setGwzStala(double S);
    void setGwzWypelnienie(double p);
    void setGwzType(TypSygnalu typ);
    void setGwzTT(double tt);
    double generateGwz();
    void resetGwz();

    // --- PID ---
    void setPidK(double k);
    void setPidTI(double TI);
    void setPidTD(double TD);
    double PIDgetP();
    double PIDgetI();
    double PIDgetD();
    void setPidMode(PID::trybCalki tryb);
    PID::trybCalki getPidMode();
    void resetPid();
    double calculatePID(double uchyb, double dt);

    // --- UAR ---
    double simulateUAR(double zadanie);
};
