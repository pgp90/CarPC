#ifndef MAINWINDOW2_H
#define MAINWINDOW2_H

#include <QMainWindow>
#include "obd2widget.h"

class MainWindow2 : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow2(QWidget *parent = 0);
    ~MainWindow2();

private:
//    Ui::MainWindow *ui;
    OBD2Widget *obd2Widget;
};

#endif // MAINWINDOW2_H
