#ifndef OBD2WIDGET_H
#define OBD2WIDGET_H

#include <QFrame>
#include <QLabel>
#include <QBitArray>
#include <QByteArray>
#include <QtSerialPort/QSerialPort>

namespace Ui {
class OBD2Widget;
}

struct OBD_PAGE_t {
    int block1;
    int block2;
    int block3;
    int block4;
    int block5;
    int block6;

    OBD_PAGE_t(int a=-1, int b=-1, int c=-1, int d=-1, int e=-1, int f=-1) :
        block1(a),
        block2(b),
        block3(c),
        block4(d),
        block5(e),
        block6(f){}
};

struct PID_INFO_t {
    char numValues;
    QString requestStr;
    QString labelStr;
    QString valueStr;
    bool codeSupport;

    PID_INFO_t(char a = 0, QString b = "", QString c = "", QString d = "") :
        numValues(a), requestStr(b), labelStr(c), valueStr(d) {
        codeSupport = (requestStr.compare("") != 0);
    }
};

class OBD2Widget : public QFrame
{
    Q_OBJECT

public:
    enum OBD2_PID {PID_SHORT_TERM_FUEL_TRIM = 0x06,
                   PID_LONG_TERM_FUEL_TRIM = 0x07,
                   PID_ENGINE_RPM = 0x0C,
                   PID_MAF_AIRFLOW_RATE = 0x10,
                   PID_COLLANT_TEMP = 0x05,
                   PID_TIMING_ADVANCE = 0x0E,
                   PID_ENGINE_LOAD = 0x04,
                   PID_FUEL_PRESURE = 0x0A,
                   PID_VEHICLE_SPEED = 0x0D,
                   PID_INTAKE_AIR_TEMP = 0x0F,
                   PID_THROTTLE_POSITION = 0x11,
                   PID_RELATIVE_THROTTLE_POSITION = 0x45,
                   PID_AMBIENT_AIR_TEMP = 0x46,
                   PID_ENGINE_FUEL_RATE = 0x5E};

    explicit OBD2Widget(QWidget *parent = 0, QString portName = "", int baudrate = QSerialPort::Baud9600);
    ~OBD2Widget();

    void connectSerialPort(QString portName = "", int baudrate = QSerialPort::Baud9600);

    void setPages(QList<int> pageitems);


//    void setUpdateTime()

//    bool setDataLabel(QLabel* label, int pid);
//    bool setValueLabel(QLabel* label, int pid);
//    bool setValueLabelTest(QLabel* label, int pid, QString response);

private slots:
    void updateObdValues();
    void nextPageClicked();
    void prevPageClicked();


private:
    Ui::OBD2Widget *ui;

//    std::vector<OBD_PAGE_t> m_obdPages;
    QList<OBD_PAGE_t> m_obdPages;

    QSerialPort *obdSerial;
    bool isSerialGood;
    int pageNum;

    std::map<int, PID_INFO_t> pidInfo;
    std::map<int, bool> pidSupported;

    QString obdRequest(QString req);

    QString parseResult(QString str, int pid);
    void parseResultStr(QString str, int* a, int* b, int* c, int* d);

    QBitArray decodePIDSupport(QString str);
    void loadSupportedPids();


    void setLabels();
    void setValues();

    void setDataLabel(QLabel* label, int pid);
    void setValueLabel(QLabel* label, int pid);

};

#endif // OBD2WIDGET_H
