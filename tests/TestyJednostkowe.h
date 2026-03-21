#pragma once
#include <string>

class TestyJednostkowe {
public:
    // Metoda pomocnicza do raportowania wyników
    static void sprawdz(bool warunek, const std::string& nazwaTestu);
};

class TestyPID {
public:
    static void uruchom();
private:
    static void testProporcjonalny(); // Test 1
    static void testResetu();         // Test 2
    static void testZmianyTrybu();    // Test 3
};

class TestyARX {
public:
    static void uruchom();
private:
    static void testStabilnosci();   // Test 4
    static void testLimitow();       // Test 5
    static void testKonfiguracji();  // Test 6
};

class TestyGWZ {
public:
    static void uruchom();
private:
    static void testSkladowejStalej(); // Test 7
    static void testOkresu();           // Test 8
};

class TestyWarstwaU {
public:
    static void uruchom();
private:
    static void testZegara();                   // Test 9
    static void testPrzekazywaniaParametrow();  // Test 10
};
