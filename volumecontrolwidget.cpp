#include "volumecontrolwidget.h"
#include "ui_volumecontrolwidget.h"
#include <QDebug>

#if defined(Q_OS_MACX)
#include <string>
#include <iostream>
#include <stdio.h>
#elif defined(Q_OS_UNIX)
#include <unistd.h>
#include <fcntl.h>
#ifdef OSSCONTROL
#define MIXER_DEV "/dev/dsp"
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <stdio.h>
#else
#include <alsa/asoundlib.h>
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



#if defined(Q_OS_MACX)
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
#elif defined(Q_OS_UNIX)

int VolumeControlWidget::getMasterVolume() {
    long *outvol;
#ifdef OSSCONTROL
    int ret = 0;
    int fd, devs;

    if ((fd = open(MIXER_DEV, O_WRONLY)) > 0)
    {
        ioctl(fd, SOUND_MIXER_READ_VOLUME, outvol);
        close(fd);
        *outvol = *outvol & 0xff;
        return outvol;
    }
    return -1;
#else
    snd_mixer_t* handle;
    snd_mixer_elem_t* elem;
    snd_mixer_selem_id_t* sid;

    static const char* mix_name = "Master";
    static const char* card = "default";
    static int mix_index = 0;

    long pmin, pmax;
    long get_vol, set_vol;
    float f_multi;

    snd_mixer_selem_id_alloca(&sid);

    //sets simple-mixer index and name
    snd_mixer_selem_id_set_index(sid, mix_index);
    snd_mixer_selem_id_set_name(sid, mix_name);

        if ((snd_mixer_open(&handle, 0)) < 0)
        return -1;
    if ((snd_mixer_attach(handle, card)) < 0) {
        snd_mixer_close(handle);
        return -2;
    }
    if ((snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
        snd_mixer_close(handle);
        return -3;
    }
    ret = snd_mixer_load(handle);
    if (ret < 0) {
        snd_mixer_close(handle);
        return -4;
    }
    elem = snd_mixer_find_selem(handle, sid);
    if (!elem) {
        snd_mixer_close(handle);
        return -5;
    }

    long minv, maxv;

    snd_mixer_selem_get_playback_volume_range (elem, &minv, &maxv);
    fprintf(stderr, "Volume range <%i,%i>\n", minv, maxv);

    if(snd_mixer_selem_get_playback_volume(elem, 0, outvol) < 0) {
        snd_mixer_close(handle);
        return -6;
    }

    fprintf(stderr, "Get volume %i with status %i\n", *outvol, ret);
    /* make the value bound to 100 */
    *outvol -= minv;
    maxv -= minv;
    minv = 0;
    *outvol = 100 * (*outvol) / maxv; // make the value bound from 0 to 100
    snd_mixer_close(handle);
    return 0;
#endif

}

void VolumeControlWidget::setMasterVolume(int vol) {//setMasterVolume(long* outvol) {
    long* outvol = &vol;
#ifdef OSSCONTROL
    int ret = 0;
    int fd, devs;

    if ((fd = open(MIXER_DEV, O_WRONLY)) > 0)
    {
        if(*outvol < 0 || *outvol > 100)
            return -2;
        *outvol = (*outvol << 8) | *outvol;
        ioctl(fd, SOUND_MIXER_WRITE_VOLUME, outvol);
        close(fd);
        return;// 0;
    }
    return;// -1;;
#else
    snd_mixer_t* handle;
    snd_mixer_elem_t* elem;
    snd_mixer_selem_id_t* sid;

    static const char* mix_name = "Master";
    static const char* card = "default";
    static int mix_index = 0;

    long pmin, pmax;
    long get_vol, set_vol;
    float f_multi;

    snd_mixer_selem_id_alloca(&sid);

    //sets simple-mixer index and name
    snd_mixer_selem_id_set_index(sid, mix_index);
    snd_mixer_selem_id_set_name(sid, mix_name);

        if ((snd_mixer_open(&handle, 0)) < 0)
        return;// -1;
    if ((snd_mixer_attach(handle, card)) < 0) {
        snd_mixer_close(handle);
        return;// -2;
    }
    if ((snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
        snd_mixer_close(handle);
        return;// -3;
    }
    ret = snd_mixer_load(handle);
    if (ret < 0) {
        snd_mixer_close(handle);
        return;// -4;
    }
    elem = snd_mixer_find_selem(handle, sid);
    if (!elem) {
        snd_mixer_close(handle);
        return;// -5;
    }

    long minv, maxv;

    snd_mixer_selem_get_playback_volume_range (elem, &minv, &maxv);
    fprintf(stderr, "Volume range <%i,%i>\n", minv, maxv);

    if(*outvol < 0 || *outvol > VOLUME_BOUND) // out of bounds
        return;// -7;
    *outvol = (*outvol * (maxv - minv) / (100-1)) + minv;

    if(snd_mixer_selem_set_playback_volume(elem, 0, *outvol) < 0) {
        snd_mixer_close(handle);
        return;// -8;
    }
    if(snd_mixer_selem_set_playback_volume(elem, 1, *outvol) < 0) {
        snd_mixer_close(handle);
        return;// -9;
    }
    fprintf(stderr, "Set volume %i with status %i\n", *outvol, ret);

    snd_mixer_close(handle);
    return;// 0;
#endif

}

#endif


