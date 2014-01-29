#include "volumecontrolwidget.h"
#include "ui_volumecontrolwidget.h"
#include <QDebug>
#include <QString>
#include <QStringList>

#if defined(Q_OS_MACX)
#include <string>
#include <iostream>
#include <stdio.h>
#else//if defined(Q_OS_LINUX)
#include <unistd.h>
#include <fcntl.h>
#ifdef OSSCONTROL
#define MIXER_DEV "/dev/dsp"
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <stdio.h>
#else
//#include <alsa/asoundlib.h>
#endif
#endif


VolumeControlWidget::VolumeControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VolumeControlWidget)
{
    ui->setupUi(this);

    ui->muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    ui->volUpButton->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    ui->volDownButton->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));

    int vol = getMasterVolume();
    ui->volSlider->setValue(vol);


    connect(ui->muteButton, SIGNAL(clicked()), this, SLOT(muteClicked()));
    connect(ui->volUpButton, SIGNAL(clicked()), this, SLOT(volUpClicked()));
    connect(ui->volDownButton, SIGNAL(clicked()), this, SLOT(volDownClicked()));
    connect(ui->volSlider, SIGNAL(sliderMoved(int)), this, SLOT(setVolume(int)));
    connect(ui->volSlider, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));
}

VolumeControlWidget::~VolumeControlWidget()
{
    delete ui;
}

void VolumeControlWidget::muteClicked() {
    qDebug()<<"muteClicked()";
}

void VolumeControlWidget::volUpClicked() {
    qDebug()<<"volUpClicked()";
    int vol = getMasterVolume();
    vol += 10;
    if (vol > 100)
        vol = 100;
    setMasterVolume(vol);
    ui->volSlider->setValue(vol);
}

void VolumeControlWidget::volDownClicked() {
    qDebug()<<"volDownClicked()";
    int vol = getMasterVolume();
    vol -= 10;
    if (vol < 0)
        vol = 0;
    setMasterVolume(vol);
    ui->volSlider->setValue(vol);
}

void VolumeControlWidget::setVolume(int volume) {
    qDebug()<<"setVolume()";
    int vol = volume;
    if (vol < 0)
        vol = 0;
    else if (vol > 100)
        vol = 100;
    setMasterVolume(vol);
    if (!ui->volSlider->isSliderDown()) {
        ui->volSlider->setValue(vol);
    }
}

void VolumeControlWidget::setMuted(bool muted) {
    qDebug()<<"setMuted()";
}



std::string exec(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}

//#if defined(Q_OS_LINUX)
#if defined(Q_OS_MACX)
int VolumeControlWidget::getMasterVolume() {
    QString str = QString::fromStdString(exec("osascript -e \"output volume of (get volume settings)\""));
    str.replace("\"", "");
    str.replace("\n", "");
    bool ok;
    return str.toInt(&ok, 10);
}

void VolumeControlWidget::setMasterVolume(int vol) {
    QString str = QString("osascript -e \"set volume output volume %1\"").arg(vol);
    const char *c_str2 = str.toLocal8Bit().data();
    std::string s = exec(c_str2);
}
#else //if defined(Q_OS_LINUX)

int VolumeControlWidget::getMasterVolume() {
    bool ok;
    QString str = QString::fromStdString(exec("amixer get Master | grep \": Playback \""));
    QStringList mainList = str.split("\n");
    int limit = 0;
    int value = 0;
    for (int i=0; i<mainList.size()-1; i++) {
        QString lineStr = mainList.at(i);
        QStringList lineList = lineStr.split(": Playback ");
        QString nameStr = lineList.at(0);
        nameStr = nameStr.replace(" ", "").toLower();
        QString valueStr = lineList.at(1);
        if (nameStr.compare("limits") == 0) {
            QStringList tmp = valueStr.split(" - ");
            limit = tmp.at(1).toInt(&ok, 10);
        } else {
            QStringList tmp = valueStr.split(" ");
            value = tmp.at(0).toInt(&ok, 10);
        }
    }

    return value * 100 / limit;
}

void VolumeControlWidget::setMasterVolume(int vol) {
    QString str = QString("amixer set Master %1\%").arg(vol);
    const char *c_str2 = str.toLocal8Bit().data();
    std::string s = exec(c_str2);
}

#endif


