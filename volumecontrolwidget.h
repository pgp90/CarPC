#ifndef VOLUMECONTROLWIDGET_H
#define VOLUMECONTROLWIDGET_H

#include <QWidget>

namespace Ui {
class VolumeControlWidget;
}

class VolumeControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VolumeControlWidget(QWidget *parent = 0);
    ~VolumeControlWidget();

private slots:
    void muteClicked();
    void volUpClicked();
    void volDownClicked();
    void setVolume(int volume);
    void setMuted(bool muted);

private:
    Ui::VolumeControlWidget *ui;

    int getVolume();

    int getMasterVolume();
    void setMasterVolume(int vol);

};

#endif // VOLUMECONTROLWIDGET_H
