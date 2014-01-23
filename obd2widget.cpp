#include "obd2widget.h"
#include "ui_obd2widget.h"

OBD2Widget::OBD2Widget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::OBD2Widget)
{
    ui->setupUi(this);
}

OBD2Widget::~OBD2Widget()
{
    delete ui;
}
