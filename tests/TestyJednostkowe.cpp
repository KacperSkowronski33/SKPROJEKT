#include "TestyJednostkowe.h"
#include "PID.h"
#include "ARX.h"
#include "GWZ.h"
#include "WarstwaU.h"
#include <iostream>
#include <vector>

void TestyJednostkowe::sprawdz(bool warunek, const std::string& nazwaTestu) {
    if (warunek) {
        std::cout << "[OK] " << nazwaTestu << std::endl;
    } else {
        std::cerr << "[BLAD] " << nazwaTestu << std::endl;
    }
}

// --- TESTY PID ---
void TestyPID::uruchom() {
    testProporcjonalny();
    testResetu();
    testZmianyTrybu();
}

void TestyPID::testProporcjonalny() {
    PID pid(2.5, 0.0, 0.0); // K=2.5
    double wynik = pid.oblicz(4.0, 0.1); // P = 2.5 * 4.0 = 10.0
    TestyJednostkowe::sprawdz(wynik == 10.0, "PID: Obliczenie czlonu P");
}

void TestyPID::testResetu() {
    PID pid(1.0, 5.0, 0.0);
    pid.oblicz(10.0, 0.2);
    pid.resetujSumeUchybu();
    TestyJednostkowe::sprawdz(pid.getI() == 0.0, "PID: Resetowanie sumy uchybu");
}

void TestyPID::testZmianyTrybu() {
    PID pid(1.0, 10.0, 0.0);
    pid.setTryb(PID::trybCalki::zew);
    pid.setTryb(PID::trybCalki::wew);
    TestyJednostkowe::sprawdz(pid.getTryb() == PID::trybCalki::wew, "PID: Zmiana trybu calkowania");
}

// --- TESTY ARX ---
void TestyARX::uruchom() {
    testStabilnosci();
    testLimitow();
    testKonfiguracji();
}

void TestyARX::testStabilnosci() {
    ARX arx({0.1, 0.1, 0.1}, {0.5, 0.0, 0.0}, 1);
    double wynik = arx.symuluj(0.0);
    TestyJednostkowe::sprawdz(wynik == 0.0, "ARX: Zero na wejsciu daje zero na wyjsciu");
}

void TestyARX::testLimitow() {
    ARX arx({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, 1);
    arx.ustawLimitWyjscia(-2.0, 2.0);
    arx.przelaczLimity(true);
    double wynik = arx.symuluj(10.0);
    TestyJednostkowe::sprawdz(wynik == 2.0, "ARX: Dzialanie limitu wyjsciowego");
}

void TestyARX::testKonfiguracji() {
    ARX arx({0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 1);
    arx.ustawOpoznienie(5);
    TestyJednostkowe::sprawdz(arx.getK() == 5, "ARX: Ustawienie opoznienia");
}

// --- TESTY GWZ ---
void TestyGWZ::uruchom() {
    testSkladowejStalej();
    testOkresu();
}

void TestyGWZ::testSkladowejStalej() {
    GeneratorWartosciZadanej gwz(0.0, 1.0, 7.5, 0.5, 100, TypSygnalu::Sinusoida);
    double wynik = gwz.obliczSygnal();
    TestyJednostkowe::sprawdz(wynik == 7.5, "GWZ: Skladowa stala przy zerowej amplitudzie");
}

void TestyGWZ::testOkresu() {
    // TRZ=1.0s, TT=100ms -> T=10 próbek
    GeneratorWartosciZadanej gwz(1.0, 1.0, 0.0, 0.5, 100, TypSygnalu::SygnalProstokatny);
    TestyJednostkowe::sprawdz(gwz.getOkres() == 10, "GWZ: Przeliczanie okresu na probki");
}

// --- TESTY WARSTWA U ---
void TestyWarstwaU::uruchom() {
    testZegara();
    testPrzekazywaniaParametrow();
}

void TestyWarstwaU::testZegara() {
    WarstwaU wu;
    wu.startSymulacji(100);
    bool dziala = wu.czySymulacjaDziala();
    wu.stopSymulacji();
    TestyJednostkowe::sprawdz(dziala && !wu.czySymulacjaDziala(), "WarstwaU: Start i stop zegara");
}

void TestyWarstwaU::testPrzekazywaniaParametrow() {
    WarstwaU wu;
    wu.setPidK(12.3);
    double wynik = wu.calculatePID(1.0, 0.1);
    TestyJednostkowe::sprawdz(wynik >= 12.3, "WarstwaU: Przekazywanie wzmocnienia do PID");
}
