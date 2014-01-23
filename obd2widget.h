#ifndef OBD2WIDGET_H
#define OBD2WIDGET_H

#include <QFrame>

namespace Ui {
class OBD2Widget;
}

class OBD2Widget : public QFrame
{
    Q_OBJECT

public:
    explicit OBD2Widget(QWidget *parent = 0);
    ~OBD2Widget();

private:
    Ui::OBD2Widget *ui;
};

#endif // OBD2WIDGET_H
