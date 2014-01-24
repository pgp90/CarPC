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

private:
    Ui::VolumeControlWidget *ui;
};

#endif // VOLUMECONTROLWIDGET_H
