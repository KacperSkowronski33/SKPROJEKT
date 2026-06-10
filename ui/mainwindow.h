#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"
#include "WarstwaU.h"
#include "ustawieniasieci.h"
#include <QResizeEvent>
#include <QElapsedTimer>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnStart_clicked();
    void on_btnStop_clicked();
    void on_btnReset_clicked();
    void on_btnUstawieniaARX_clicked();
    void on_btnZapisz_clicked();
    void on_btnWczytaj_clicked();
    void on_btnResetCalki_clicked();
    void aktualizujSymulacje();
    void on_parametryChanged();

    void on_actionUstawienia_triggered();
    void on_odebranaRamka(const Ramka &ramka);
    void on_rozlaczono();

private:
    Ui::MainWindow *ui;


    WarstwaU *warstwaUslug;

    void rysujWykresy(double w, double u, double e, double y, double yEst, double valP, double valI, double valD, double dt);
    double aktualnyCzas;
    double y_prev;

    void setupPlot(QCustomPlot *plot, QString tytul, QString yLabel);
    void skalujWykres(QCustomPlot *plot, double minSpan = 10.0);

    QString m_ostatnieIP;
    int m_ostatniPort;
    bool m_czyOstatniSerwer;
    bool m_czyOstatniObiekt;
    bool m_czyOstatniTrybLokalny;
    unsigned int m_numerProbki = 0;
    bool m_czyOdebranoOdpowiedz = true;
    QElapsedTimer m_stoperSiec;
    qint64 m_opoznienieSieci;
    QMap<unsigned int, qint64> m_siecHistoriaProbek;
    UstawieniaSieci *oknoSiec;

    struct tempWyslaneDane {
        double w = 0.0;
        double u = 0.0;
        double e = 0.0;
        double p = 0.0;
        double i = 0.0;
        double d = 0.0;
    } m_tempStan;

    void ustawBlokadySymulacji(bool czyObiekt);
protected:
    void resizeEvent(QResizeEvent *event) override;
};
#endif // MAINWINDOW_H
