#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"
#include "WarstwaU.h"
#include <QResizeEvent>


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

private:
    Ui::MainWindow *ui;


    WarstwaU *warstwaUslug;


    double aktualnyCzas;
    double y_prev;

    void setupPlot(QCustomPlot *plot, QString tytul, QString yLabel);
    void skalujWykres(QCustomPlot *plot, double minSpan = 10.0);
protected:
    void resizeEvent(QResizeEvent *event) override;
};
#endif // MAINWINDOW_H
