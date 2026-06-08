#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ustawieniaarx.h"
#include "WarstwaU.h"
#include <cmath>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include "ustawieniasieci.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. Inicjalizacja logiki (Warstwa Usług)
    warstwaUslug = new WarstwaU(this);


    connect(warstwaUslug, &WarstwaU::zadanieOdswiezenia, this, &MainWindow::aktualizujSymulacje);

    // Inicjalizacja zmiennych pomocniczych
    aktualnyCzas = 0.0;
    y_prev = 0.0;

    // 2. Konfiguracja wykresów (QCustomPlot)
    setupPlot(ui->chartWykres1, "Regulacja", "Wartość");
    ui->chartWykres1->addGraph();
    ui->chartWykres1->graph(0)->setPen(QPen(Qt::red));
    ui->chartWykres1->graph(0)->setName("Zadana (w)");
    ui->chartWykres1->addGraph();
    ui->chartWykres1->graph(1)->setPen(QPen(Qt::blue));
    ui->chartWykres1->graph(1)->setName("Wyjście (y)");
    ui->chartWykres1->legend->setVisible(true);

    setupPlot(ui->chartWykres2, "Uchyb", "e");
    ui->chartWykres2->addGraph();
    ui->chartWykres2->graph(0)->setPen(QPen(Qt::black));

    setupPlot(ui->chartwykres3, "Sterowanie", "u");
    ui->chartwykres3->addGraph();
    ui->chartwykres3->graph(0)->setPen(QPen(Qt::darkGreen));

    setupPlot(ui->chartWykres4, "Składowe PID", "Wartość");
    ui->chartWykres4->addGraph();
    ui->chartWykres4->graph(0)->setPen(QPen(Qt::red));    ui->chartWykres4->graph(0)->setName("P");
    ui->chartWykres4->addGraph();
    ui->chartWykres4->graph(1)->setPen(QPen(Qt::green));  ui->chartWykres4->graph(1)->setName("I");
    ui->chartWykres4->addGraph();
    ui->chartWykres4->graph(2)->setPen(QPen(Qt::blue));   ui->chartWykres4->graph(2)->setName("D");
    ui->chartWykres4->legend->setVisible(true);



    connect(ui->spinKp, &QDoubleSpinBox::editingFinished, this, &MainWindow::on_parametryChanged);
    connect(ui->spinTi, &QDoubleSpinBox::editingFinished, this, &MainWindow::on_parametryChanged);
    connect(ui->spinTd, &QDoubleSpinBox::editingFinished, this, &MainWindow::on_parametryChanged);

    connect(ui->spinAmp, &QDoubleSpinBox::editingFinished, this, &MainWindow::on_parametryChanged);
    connect(ui->spinOkres, &QDoubleSpinBox::editingFinished, this, &MainWindow::on_parametryChanged);
    connect(ui->spinStala, &QDoubleSpinBox::editingFinished, this, &MainWindow::on_parametryChanged);
    connect(ui->spinWypelnienie, &QDoubleSpinBox::editingFinished, this, &MainWindow::on_parametryChanged);
    connect(ui->spinOkno, &QDoubleSpinBox::editingFinished, this, &MainWindow::on_parametryChanged);

    connect(ui->spinInterwal, &QDoubleSpinBox::editingFinished, this, &MainWindow::on_parametryChanged);

    connect(ui->comboTyp, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_parametryChanged);
    connect(ui->boxRozniczka, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_parametryChanged);

    // 5. Pierwsza synchronizacja parametrów z UI do logiki
    on_parametryChanged();

    //6. Siec
    m_ostatniPort = 00000;
    m_czyOstatniTrybLokalny = true;
    m_czyOstatniSerwer = false;
    m_czyOstatniObiekt = false;
    connect(warstwaUslug, &WarstwaU::ramkaOdebrana, this, &MainWindow::on_odebranaRamka);
    connect(warstwaUslug, &WarstwaU::infoRozlaczono, this, &MainWindow::on_rozlaczono);
    ui->lblSiec->clear();
    m_opoznienieSieci = 0;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupPlot(QCustomPlot *plot, QString tytul, QString yLabel)
{
    plot->plotLayout()->insertRow(0);
    plot->plotLayout()->addElement(0, 0, new QCPTextElement(plot, tytul, QFont("sans", 10, QFont::Bold)));
    plot->xAxis->setLabel("Czas [s]");
    plot->yAxis->setLabel(yLabel);
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void MainWindow::on_btnStop_clicked()
{
    warstwaUslug->stopSymulacji();
}

void MainWindow::aktualizujSymulacje()
{
    if (!m_czyOstatniTrybLokalny && m_czyOstatniObiekt) { //siec - obiekt
        return;
    }
    double dt = warstwaUslug->getInterwalSekundy();
    double w = warstwaUslug->generateGwz();

    // ZABEZPIECZENIE 1: Ochrona przed NaN na wejściu pętli
    if (std::isnan(w) || std::isinf(w)) w = 0.0;
    if (std::isnan(y_prev) || std::isinf(y_prev)) y_prev = 0.0;

    double e = w - y_prev;
    double u = warstwaUslug->calculatePID(e, dt);

    // ZABEZPIECZENIE 2: Ochrona sterowania
    if (std::isnan(u) || std::isinf(u)) u = 0.0;

    double valP = warstwaUslug->PIDgetP();
    double valI = warstwaUslug->PIDgetI();
    double valD = warstwaUslug->PIDgetD();

    if(m_czyOstatniTrybLokalny) { //lokalny

        double y = warstwaUslug->calculateARX(u);
        if (std::isnan(y) || std::isinf(y)) y = 0.0;
        y_prev = y;
        rysujWykresy(w, u, e, y, valP, valI, valD, dt);

    } else { //siec - regulator
        if(!m_czyOdebranoOdpowiedz) {
            ui->lblSiecLampka->setStyleSheet("background-color: red; border-radius: 10px");
            ui->lblSiecInfo->setText("Opóźnienie: " + QString::number(m_opoznienieSieci) + " ms");
        } else {
            ui->lblSiecLampka->setStyleSheet("background-color: green; border-radius: 10px");
        }
        m_czyOdebranoOdpowiedz = false;
        m_stoperSiec.start();

        Ramka rDaneSym;
        rDaneSym.typ = TypRamki::DaneSymulacji;
        rDaneSym.u = u;
        rDaneSym.w = w;
        m_numerProbki++;
        rDaneSym.numerProbki = m_numerProbki;
        warstwaUslug->wyslijRamke(rDaneSym);

        m_tempStan.w = w;
        m_tempStan.u = u;
        m_tempStan.e = e;
        m_tempStan.p = valP;
        m_tempStan.i = valI;
        m_tempStan.d = valD;
        //rysujWykresy(w, u, e, y_prev, valP, valI, valD, dt); //byc moze to powoduje opoznienie rysowania wykresu!!!!
    }
}

void MainWindow::rysujWykresy(double w, double u, double e, double y, double valP, double valI, double valD, double dt)
{
    ui->chartWykres1->graph(0)->addData(aktualnyCzas, w);
    ui->chartWykres1->graph(1)->addData(aktualnyCzas, y);
    ui->chartWykres2->graph(0)->addData(aktualnyCzas, e);
    ui->chartwykres3->graph(0)->addData(aktualnyCzas, u);
    ui->chartWykres4->graph(0)->addData(aktualnyCzas, valP);
    ui->chartWykres4->graph(1)->addData(aktualnyCzas, valI);
    ui->chartWykres4->graph(2)->addData(aktualnyCzas, valD);

    // 4. Przesuwanie osi X
    double okno = ui->spinOkno->value();
    double minX = (aktualnyCzas > okno) ? aktualnyCzas - okno : 0.0;
    double maxX = (aktualnyCzas > okno) ? aktualnyCzas : okno;

    ui->chartWykres1->xAxis->setRange(minX, maxX);
    ui->chartWykres2->xAxis->setRange(minX, maxX);
    ui->chartwykres3->xAxis->setRange(minX, maxX);
    ui->chartWykres4->xAxis->setRange(minX, maxX);

    ui->chartWykres1->graph(0)->data()->removeBefore(minX);
    ui->chartWykres1->graph(1)->data()->removeBefore(minX);
    ui->chartWykres2->graph(0)->data()->removeBefore(minX);
    ui->chartwykres3->graph(0)->data()->removeBefore(minX);
    ui->chartWykres4->graph(0)->data()->removeBefore(minX);
    ui->chartWykres4->graph(1)->data()->removeBefore(minX);
    ui->chartWykres4->graph(2)->data()->removeBefore(minX);

    // 5. SKALOWANIE OSI Y
    skalujWykres(ui->chartWykres1);
    skalujWykres(ui->chartWykres2);
    skalujWykres(ui->chartwykres3);

    if (ui->chartWykres4) {
        if(m_czyOstatniObiekt) {
            ui->chartWykres4->yAxis->setRange(-2.0,2.0);
        } else skalujWykres(ui->chartWykres4);
    }

    // 6. Aktualizacja czasu
    aktualnyCzas += dt;
    ui->lblCzas->setText(QString("Czas: %1 s").arg(QString::number(aktualnyCzas, 'f', 2)));

    ui->chartWykres1->replot(QCustomPlot::rpQueuedReplot);
    ui->chartWykres2->replot(QCustomPlot::rpQueuedReplot);
    ui->chartwykres3->replot(QCustomPlot::rpQueuedReplot);
    ui->chartWykres4->replot(QCustomPlot::rpQueuedReplot);
}

void MainWindow::on_btnStart_clicked()
{
    int interwal = ui->spinInterwal->value();
    warstwaUslug->startSymulacji(interwal);
}

void MainWindow::on_btnResetCalki_clicked()
{
    warstwaUslug->resetPid();
}

void MainWindow::on_btnReset_clicked()
{
    warstwaUslug->stopSymulacji();

    warstwaUslug->resetGwz();
    warstwaUslug->resetPid();
    aktualnyCzas = 0.0;
    y_prev = 0.0;
    ui->lblCzas->setText("Czas: 0.00 s");

    // Czyszczenie danych na wykresach
    ui->chartWykres1->graph(0)->data()->clear();
    ui->chartWykres1->graph(1)->data()->clear();
    ui->chartWykres2->graph(0)->data()->clear();
    ui->chartwykres3->graph(0)->data()->clear();
    ui->chartWykres4->graph(0)->data()->clear();
    ui->chartWykres4->graph(1)->data()->clear();
    ui->chartWykres4->graph(2)->data()->clear();

    ui->chartWykres1->yAxis->setRange(-1, 1);
    ui->chartWykres2->yAxis->setRange(-1, 1);
    ui->chartwykres3->yAxis->setRange(-1, 1);
    ui->chartWykres4->yAxis->setRange(-1, 1);

    ui->chartWykres1->replot();
    ui->chartWykres2->replot();
    ui->chartwykres3->replot();
    ui->chartWykres4->replot();
}

void MainWindow::on_btnUstawieniaARX_clicked()
{
    UstawieniaARX okno(this);

    okno.setWielomianA(warstwaUslug->getArxA());
    okno.setWielomianB(warstwaUslug->getArxB());
    okno.setK(warstwaUslug->getArxK());
    okno.setSzumAmp(warstwaUslug->getArxNoise());
    okno.setSzumAktywny(warstwaUslug->getArxNoiseActive());
    okno.setLimityAktywne(warstwaUslug->getArxLimitsActive());
    okno.setLimityWejscia(warstwaUslug->getArxUMin(), warstwaUslug->getArxUMax());
    okno.setLimityWyjscia(warstwaUslug->getArxYMin(), warstwaUslug->getArxYMax());

    if(!m_czyOstatniObiekt && !m_czyOstatniTrybLokalny)
    {
        okno.ustawBlokadyARX();
    }


    if (okno.exec() == QDialog::Accepted)
    {
        warstwaUslug->setArxA(okno.getA());
        warstwaUslug->setArxB(okno.getB());
        warstwaUslug->setArxDelay(okno.getK());
        warstwaUslug->setArxNoiseAmplitude(okno.getSzumAmp());
        warstwaUslug->toggleArxNoise(okno.getSzumAktywny());

        double uMin = okno.getUmin();
        double uMax = okno.getUmax();
        if (!okno.getLimityAktywne())
        {
            uMin = -999999.0;
            uMax = 999999.0;
        }

        warstwaUslug->setArxInputLimit(okno.getUmin(), okno.getUmax());
        warstwaUslug->setArxOutputLimit(okno.getYmin(), okno.getYmax());
        warstwaUslug->toggleArxLimits(okno.getLimityAktywne());

        Ramka rARX;
        rARX.typ = TypRamki::ParametryARX;
        rARX.wekA = okno.getA();
        rARX.wekB = okno.getB();
        rARX.opoznienie = okno.getK();
        rARX.ampSzum = okno.getSzumAmp();
        rARX.czySzum = okno.getSzumAktywny();
        rARX.umin = uMin;
        rARX.umax = uMax;
        rARX.ymin = okno.getYmin();
        rARX.ymax = okno.getYmax();
        rARX.czyLim = okno.getLimityAktywne();

        warstwaUslug->wyslijRamke(rARX);
    }
}

void MainWindow::on_btnZapisz_clicked()
{
    QString nazwaPliku = QFileDialog::getSaveFileName(this, "Zapisz konfigurację", "", "Pliki JSON (*.json)");
    if (nazwaPliku.isEmpty()) return;

    QJsonObject root;

    // --- 1. ZAPIS PID ---
    QJsonObject jsonPID;
    jsonPID["Kp"] = ui->spinKp->value();
    jsonPID["Ti"] = ui->spinTi->value();
    jsonPID["Td"] = ui->spinTd->value();
    jsonPID["Tryb"] = (warstwaUslug->getPidMode() == PID::trybCalki::zew) ? 1 : 0;
    root["PID"] = jsonPID;

    // --- 2. ZAPIS GENERATORA ---
    QJsonObject jsonGEN;
    jsonGEN["Amplituda"] = ui->spinAmp->value();
    jsonGEN["Okres"] = ui->spinOkres->value();
    jsonGEN["Wypelnienie"] = ui->spinWypelnienie->value();
    jsonGEN["Stala"] = ui->spinStala->value();
    jsonGEN["Typ"] = ui->comboTyp->currentIndex();
    root["Generator"] = jsonGEN;

    // --- 3. ZAPIS ARX ---
    QJsonObject jsonARX;

    QJsonArray arrayA;
    for (double val : warstwaUslug->getArxA()) arrayA.append(val);
    jsonARX["A"] = arrayA;

    QJsonArray arrayB;
    for (double val : warstwaUslug->getArxB()) arrayB.append(val);
    jsonARX["B"] = arrayB;

    jsonARX["k"] = warstwaUslug->getArxK();
    jsonARX["SzumAktywny"] = warstwaUslug->getArxNoiseActive();
    jsonARX["SzumAmplituda"] = warstwaUslug->getArxNoise();

    QJsonObject jsonLimity;
    jsonLimity["Aktywne"] = warstwaUslug->getArxLimitsActive();
    jsonLimity["U_min"] = warstwaUslug->getArxUMin();
    jsonLimity["U_max"] = warstwaUslug->getArxUMax();
    jsonLimity["Y_min"] = warstwaUslug->getArxYMin();
    jsonLimity["Y_max"] = warstwaUslug->getArxYMax();
    jsonARX["Limity"] = jsonLimity;

    root["ARX"] = jsonARX;

    // --- 4. POZOSTAŁE ---
    root["Interwal"] = ui->spinInterwal->value();
    root["Skalowanie okna"] = ui->spinOkno->value(); //dodane zapisywanie

    QFile file(nazwaPliku);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Błąd", "Nie można zapisać pliku!");
        return;
    }
    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();

    ui->statusbar->showMessage("Zapisano pełną konfigurację!", 3000);
}

void MainWindow::on_btnWczytaj_clicked()
{
    QString nazwaPliku = QFileDialog::getOpenFileName(this, "Wczytaj konfigurację", "", "Pliki JSON (*.json)");
    if (nazwaPliku.isEmpty()) return;

    QFile file(nazwaPliku);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć pliku!");
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) return;
    QJsonObject root = doc.object();

    // --- 1. WCZYTANIE PID ---
    if (root.contains("PID")) {
        QJsonObject jsonPID = root["PID"].toObject();
        ui->spinKp->setValue(jsonPID["Kp"].toDouble());
        ui->spinTi->setValue(jsonPID["Ti"].toDouble());
        ui->spinTd->setValue(jsonPID["Td"].toDouble());

        if (jsonPID.contains("Tryb")) {
            int trybIndex = jsonPID["Tryb"].toInt();
            ui->boxRozniczka->setCurrentIndex(trybIndex);
            warstwaUslug->setPidMode(trybIndex == 1 ? PID::trybCalki::zew : PID::trybCalki::wew);
        }
    }

    // --- 2. WCZYTANIE GENERATORA ---
    if (root.contains("Generator")) {
        QJsonObject jsonGEN = root["Generator"].toObject();
        ui->spinAmp->setValue(jsonGEN["Amplituda"].toDouble());
        ui->spinOkres->setValue(jsonGEN["Okres"].toDouble());
        ui->spinWypelnienie->setValue(jsonGEN["Wypelnienie"].toDouble());
        if (jsonGEN.contains("Stala")) ui->spinStala->setValue(jsonGEN["Stala"].toDouble());
        ui->comboTyp->setCurrentIndex(jsonGEN["Typ"].toInt());
    }

    // --- 3. WCZYTANIE ARX ---
    if (root.contains("ARX")) {
        QJsonObject jsonARX = root["ARX"].toObject();

        std::vector<double> newA, newB;
        QJsonArray arrA = jsonARX["A"].toArray();
        for (const auto &val : arrA) newA.push_back(val.toDouble());
        QJsonArray arrB = jsonARX["B"].toArray();
        for (const auto &val : arrB) newB.push_back(val.toDouble());

        warstwaUslug->setArxA(newA);
        warstwaUslug->setArxB(newB);
        warstwaUslug->setArxK(jsonARX["k"].toInt());

        if (jsonARX.contains("SzumAktywny")) {
            warstwaUslug->toggleArxNoise(jsonARX["SzumAktywny"].toBool());
        }
        if (jsonARX.contains("SzumAmplituda")) {
            warstwaUslug->setArxNoiseAmplitude(jsonARX["SzumAmplituda"].toDouble());
        }

        if (jsonARX.contains("Limity")) {
            QJsonObject jsonLimity = jsonARX["Limity"].toObject();
            double uMin = jsonLimity["U_min"].toDouble();
            double uMax = jsonLimity["U_max"].toDouble();
            double yMin = jsonLimity["Y_min"].toDouble();
            double yMax = jsonLimity["Y_max"].toDouble();
            bool aktywne = jsonLimity["Aktywne"].toBool();

            warstwaUslug->setArxInputLimit(uMin, uMax);
            warstwaUslug->setArxOutputLimit(yMin, yMax);
            warstwaUslug->toggleArxLimits(aktywne);
        }
    }

    // --- 4. POZOSTAŁE ---
    if (root.contains("Interwal")) {
        ui->spinInterwal->setValue(root["Interwal"].toInt());
        warstwaUslug->setGwzTT(ui->spinInterwal->value());
    }

    if (root.contains("Skalowanie okna")) {
        ui->spinOkno->setValue(root["Skalowanie okna"].toInt()); //dodane wczytywanie skalowania okna
    }

    on_parametryChanged();          //żeby odczyt działał prawidłowo
    on_btnReset_clicked();
    aktualizujSymulacje();
    QMessageBox::information(this, "Sukces", "Wczytano pełną konfigurację.");
}

void MainWindow::skalujWykres(QCustomPlot *wykres, double minSpan)
{
    if(!wykres || wykres->graphCount()==0 )return;

    // Próba dopasowania do danych
    wykres->yAxis->rescale(true);

    QCPRange zakres = wykres->yAxis->range();

    if (!QCPRange::validRange(zakres)) {
        wykres->yAxis->setRange(-minSpan/2.0, minSpan/2.0);
        //wykres->replot();
        return;
    }

    // Logika marginesów
    if(zakres.size() < 0.1){

        wykres->yAxis->setRange(zakres.center(), minSpan, Qt::AlignCenter);
    }
    else{

        wykres->yAxis->scaleRange(1.3);
    }

    //wykres->replot();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    int nowyRozmiar = this->width() / 120;
    if (nowyRozmiar < 10) nowyRozmiar = 10;
    if (nowyRozmiar > 25) nowyRozmiar = 25;

    this->setStyleSheet(QString("QMainWindow,QWidget#centralWidget{background-color:#2b2b2b;color:#f0f0f0;}QGroupBox{background-color:#333333;border:1px solid #555555;border-radius:6px;margin-top:22px;font-weight:bold;}QGroupBox::title{subcontrol-origin:margin;subcontrol-position:top center;padding:0 5px;color:#4da6ff;background-color:transparent;}QLabel{color:#dddddd;background-color:transparent;border:none;}QDoubleSpinBox,QSpinBox,QComboBox{background-color:#454545;border:1px solid #666666;border-radius:3px;padding:4px;color:#ffffff;selection-background-color:#4da6ff;}QDoubleSpinBox:hover,QSpinBox:hover,QComboBox:hover{border:1px solid #4da6ff;}QPushButton{background-color:#404040;border:1px solid #555555;border-radius:4px;color:white;padding:6px 12px;min-height:25px;}QPushButton:hover{background-color:#505050;border:1px solid #4da6ff;}QPushButton:pressed{background-color:#2a2a2a;}QPushButton#btnStart{background-color:#2e7d32;border-color:#4caf50;}QPushButton#btnStart:hover{background-color:#388e3c;}QPushButton#btnStop{background-color:#c62828;border-color:#ef5350;}QPushButton#btnStop:hover{background-color:#d32f2f;}QStatusBar{background-color:#202020;color:#aaaaaa;border-top:1px solid #444;}QCustomPlot{border:1px solid #555555;border-radius:4px;background-color:#1e1e1e;}").arg(nowyRozmiar));

    ui->lblCzas->setStyleSheet(QString("QLabel {color: #dddddd;background-color: transparent; border: none; font-size: %1pt;}").arg(nowyRozmiar));
    ui->label->setStyleSheet(QString("QLabel {color: #dddddd;background-color: transparent; border: none; font-size: %1pt;}").arg(nowyRozmiar));
    ui->labelRozniczka->setStyleSheet(QString("QLabel {color: #dddddd;background-color: transparent; border: none; font-size: %1pt;}").arg(nowyRozmiar));
    ui->label_2->setStyleSheet(QString("QLabel {color: #dddddd;background-color: transparent; border: none; font-size: %1pt;}").arg(nowyRozmiar));
    ui->label_3->setStyleSheet(QString("QLabel {color: #dddddd;background-color: transparent; border: none; font-size: %1pt;}").arg(nowyRozmiar));
    ui->label_4->setStyleSheet(QString("QLabel {color: #dddddd;background-color: transparent; border: none; font-size: %1pt;}").arg(nowyRozmiar));
    ui->label_5->setStyleSheet(QString("QLabel {color: #dddddd;background-color: transparent; border: none; font-size: %1pt;}").arg(nowyRozmiar));
    ui->label_6->setStyleSheet(QString("QLabel {color: #dddddd;background-color: transparent; border: none; font-size: %1pt;}").arg(nowyRozmiar));
    ui->label_7->setStyleSheet(QString("QLabel {color: #dddddd;background-color: transparent; border: none; font-size: %1pt;}").arg(nowyRozmiar));
    ui->label_8->setStyleSheet(QString("QLabel {color: #dddddd;background-color: transparent; border: none; font-size: %1pt;}").arg(nowyRozmiar));
    ui->label_9->setStyleSheet(QString("QLabel {color: #dddddd;background-color: transparent; border: none; font-size: %1pt;}").arg(nowyRozmiar));
}

void MainWindow::on_parametryChanged()
{
    warstwaUslug->setPidK(ui->spinKp->value());
    warstwaUslug->setPidTI(ui->spinTi->value());
    warstwaUslug->setPidTD(ui->spinTd->value());

    warstwaUslug->setGwzAmplitude(ui->spinAmp->value());
    warstwaUslug->setGwzPeriod(ui->spinOkres->value());
    warstwaUslug->setGwzWypelnienie(ui->spinWypelnienie->value());
    warstwaUslug->setGwzStala(ui->spinStala->value());

    if (ui->comboTyp->currentIndex() == 0)
        warstwaUslug->setGwzType(TypSygnalu::SygnalProstokatny);
    else
        warstwaUslug->setGwzType(TypSygnalu::Sinusoida);

    if (ui->boxRozniczka->currentIndex() == 0)
        warstwaUslug->setPidMode(PID::trybCalki::wew);
    else
        warstwaUslug->setPidMode(PID::trybCalki::zew);

    warstwaUslug->setGwzTT(ui->spinInterwal->value());
    warstwaUslug->setInterwalSymulacji(ui->spinInterwal->value());

    ui->statusbar->showMessage("Parametry zaktualizowane!", 2000);



    Ramka rPID;
    rPID.typ = TypRamki::ParametryPID;
    rPID.kp = ui->spinKp->value();
    rPID.ti = ui->spinTi->value();
    rPID.td = ui->spinTd->value();
    rPID.typCalki = (ui->boxRozniczka->currentIndex() == 0) ? PID::trybCalki::wew : PID::trybCalki::zew;
    warstwaUslug->wyslijRamke(rPID);

    Ramka rGWZ;
    rGWZ.typ = TypRamki::ParametryGWZ;
    rGWZ.amplituda = ui->spinAmp->value();
    rGWZ.okres = ui->spinOkres->value();
    rGWZ.wypelnienie = ui->spinWypelnienie->value();
    rGWZ.typSyg = (ui->comboTyp->currentIndex() == 0) ? TypSygnalu::SygnalProstokatny : TypSygnalu::Sinusoida;
    rGWZ.skladowa_stala = ui->spinStala->value();
    rGWZ.interwal = ui->spinInterwal->value();

    warstwaUslug->wyslijRamke(rGWZ);
}

void MainWindow::on_actionUstawienia_triggered()
{
    UstawieniaSieci oknoSiec(this);
    oknoSiec.setIP(m_ostatnieIP);
    oknoSiec.setPort(m_ostatniPort);
    oknoSiec.setCzyLokalny(m_czyOstatniTrybLokalny);
    oknoSiec.setCzyObiekt(m_czyOstatniObiekt);
    oknoSiec.setCzySerwer(m_czyOstatniSerwer);

    connect(&oknoSiec, &UstawieniaSieci::sygnalPolacz, this, [=](bool serwer, int port, QString adres) {
        warstwaUslug->wlaczTrybSieciowy(serwer, port, adres);
    });

    connect(warstwaUslug, &WarstwaU::infoPolaczono, &oknoSiec, [&oknoSiec](){
        oknoSiec.statusPolaczono(oknoSiec.getIP());
    });

    if(oknoSiec.exec() == QDialog::Accepted) {
        m_ostatnieIP = oknoSiec.getIP();
        m_ostatniPort = oknoSiec.getPort();
        m_czyOstatniSerwer = oknoSiec.getCzySerwer();
        m_czyOstatniObiekt = oknoSiec.getCzyObiekt();
        m_czyOstatniTrybLokalny = oknoSiec.getCzyLokalny();

        ustawBlokadySymulacji(m_czyOstatniObiekt);

        if(m_czyOstatniTrybLokalny) {
            ui->lblSiec->clear();
        } else {
            if(m_czyOstatniObiekt) ui->lblSiec->setText("Tryb sieciowy aktywny - obiekt");
            else ui->lblSiec->setText("Tryb sieciowy aktywny - regulator");
        }
    }
}

void MainWindow::ustawBlokadySymulacji(bool czyObiekt)
{
    if(czyObiekt && !m_czyOstatniTrybLokalny)
    {
        ui->groupBox->setEnabled(false);
        ui->groupBox_2->setEnabled(false);
        ui->groupBox_3->setEnabled(false);
    } else {
        ui->groupBox->setEnabled(true);
        ui->groupBox_2->setEnabled(true);
        ui->groupBox_3->setEnabled(true);
    }
}

void MainWindow::on_odebranaRamka(const Ramka &ramka)
{
    switch(ramka.typ) {
    case TypRamki::DaneSymulacji:
        if(m_czyOstatniTrybLokalny) break;

        if(m_czyOstatniObiekt) {
            double u = ramka.u;
            double w = ramka.w;
            double dt = warstwaUslug->getInterwalSekundy();

            double y = warstwaUslug->calculateARX(u);
            if(std::isnan(y) || std::isinf(y)) y = 0.0;
            Ramka rOdpDoRegulatora;
            rOdpDoRegulatora.typ = TypRamki::DaneSymulacji;
            rOdpDoRegulatora.y = y;
            rOdpDoRegulatora.numerProbki = ramka.numerProbki;
            rOdpDoRegulatora.interwal = ramka.interwal;
            warstwaUslug->wyslijRamke(rOdpDoRegulatora);

            //aktualnyCzas= (ramka.numerProbki - 1) * dt; //probba 1 - synchronizacja czasow
            double e = w - y;
            rysujWykresy(w, u, e, y, 0.0,0.0,0.0,dt);
        } else {
            if(ramka.numerProbki == m_numerProbki) {
                m_opoznienieSieci = m_stoperSiec.elapsed();
                ui->lblSiecInfo->setText("Opóźnienie: " + QString::number(m_opoznienieSieci) + " ms");
                m_czyOdebranoOdpowiedz = true;
                y_prev = ramka.y;


                //tutaj przeniesione rysowanie - aktualizujSymulacje()
                double dt = warstwaUslug->getInterwalSekundy();
                //aktualnyCzas= (ramka.numerProbki - 1) * dt; //synchronizacja czasow


                rysujWykresy(m_tempStan.w, m_tempStan.u, m_tempStan.e, y_prev, m_tempStan.p, m_tempStan.i, m_tempStan.d, dt);
            }
        }
        break;

    case TypRamki::ParametryARX:

        warstwaUslug->setArxA(ramka.wekA);
        warstwaUslug->setArxB(ramka.wekB);
        warstwaUslug->setArxDelay(ramka.opoznienie);
        warstwaUslug->setArxNoiseAmplitude(ramka.ampSzum);
        warstwaUslug->toggleArxNoise(ramka.czySzum);
        warstwaUslug->setArxInputLimit(ramka.umin, ramka.umax);
        warstwaUslug->setArxOutputLimit(ramka.ymin, ramka.ymax);
        warstwaUslug->toggleArxLimits(ramka.czyLim);

        break;

    case TypRamki::ParametryPID:

        warstwaUslug->setPidK(ramka.kp);
        warstwaUslug->setPidTI(ramka.ti);
        warstwaUslug->setPidTD(ramka.td);
        warstwaUslug->setPidMode(ramka.typCalki);

        ui->spinKp->setValue(ramka.kp);
        ui->spinTi->setValue(ramka.ti);
        ui->spinTd->setValue(ramka.td);

        if (ramka.typCalki == PID::trybCalki::wew) {
            ui->boxRozniczka->setCurrentIndex(0);
        } else {
            ui->boxRozniczka->setCurrentIndex(1);
        }

        break;

    case TypRamki::ParametryGWZ:
        warstwaUslug->setGwzAmplitude(ramka.amplituda);
        warstwaUslug->setGwzPeriod(ramka.okres);
        warstwaUslug->setGwzWypelnienie(ramka.wypelnienie);
        warstwaUslug->setGwzType(ramka.typSyg);
        warstwaUslug->setGwzStala(ramka.skladowa_stala);
        warstwaUslug->setInterwalSymulacji(ramka.interwal);
        warstwaUslug->setGwzTT(ramka.interwal);
        ui->spinAmp->setValue(ramka.amplituda);
        ui->spinOkres->setValue(ramka.okres);
        ui->spinWypelnienie->setValue(ramka.wypelnienie);
        ui->comboTyp->setCurrentIndex(ramka.typSyg == TypSygnalu::SygnalProstokatny ? 0 : 1);
        ui->spinStala->setValue(ramka.skladowa_stala);
        ui->spinInterwal->setValue(ramka.interwal);

        break;
    };
}

void MainWindow::on_rozlaczono()
{
    if (!m_czyOstatniTrybLokalny) {
        m_czyOstatniTrybLokalny = true;
        m_czyOstatniObiekt = false;
        ustawBlokadySymulacji(false);
        ui->lblSiecLampka->setStyleSheet("background-color: gray; border-radius: 10px");
        ui->lblSiecInfo->clear();
        ui->lblSiec->clear();
        QMessageBox::critical(this, "ROZŁĄCZONO",
                              "Utracono połączenie sieciowe z drugą instancją.\n\n"
                              "Symulacja została automatycznie przełączona w tryb lokalny.");
    }
}


