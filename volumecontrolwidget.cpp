#include "volumecontrolwidget.h"
#include "ui_volumecontrolwidget.h"

VolumeControlWidget::VolumeControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VolumeControlWidget)
{
    ui->setupUi(this);
}

VolumeControlWidget::~VolumeControlWidget()
{
    delete ui;
}
