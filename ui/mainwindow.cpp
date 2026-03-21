#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ustawieniaarx.h"
#include "WarstwaU.h"
#include <iostream>
#include <cmath>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <qdebug>

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

    double y = warstwaUslug->calculateARX(u);

    // ZABEZPIECZENIE 3: Ochrona wyjścia obiektu
    if (std::isnan(y) || std::isinf(y)) y = 0.0;

    y_prev = y;

    // 3. Dodawanie danych
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

    if (ui->chartWykres4)
        skalujWykres(ui->chartWykres4);

    // 6. Aktualizacja czasu
    aktualnyCzas += dt;
    ui->lblCzas->setText(QString("Czas: %1 s").arg(QString::number(aktualnyCzas, 'f', 2)));
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
        wykres->replot();
        return;
    }

    // Logika marginesów
    if(zakres.size() < 0.1){

        wykres->yAxis->setRange(zakres.center(), minSpan, Qt::AlignCenter);
    }
    else{

        wykres->yAxis->scaleRange(1.3);
    }

    wykres->replot();
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
}
