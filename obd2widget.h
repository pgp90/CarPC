#ifndef OBD2WIDGET_H
#define OBD2WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QBitArray>
#include <QByteArray>
#include <QtSerialPort/QSerialPort>

namespace Ui {
class OBD2Widget;
}

struct OBD_PAGE_t {
    int block0;
    int block1;
    int block2;
    int block3;
    int block4;
    int block5;

    OBD_PAGE_t(int a=-1, int b=-1, int c=-1, int d=-1, int e=-1, int f=-1) :
        block0(a),
        block1(b),
        block2(c),
        block3(d),
        block4(e),
        block5(f){}
};

struct PID_INFO_t {
    QString requestStr;
    QString labelStr;
    QString unit;
    bool hasConversion;
    float conversionFactor;
    QString usUnit;
    bool codeSupport;

    PID_INFO_t(QString req, QString label, QString un) :
        requestStr(req), labelStr(label), unit(un), hasConversion(false), conversionFactor(1), usUnit(un) {
        codeSupport = (requestStr.compare("") != 0);
    }

    PID_INFO_t(QString req, QString label, QString un, float con, QString us) :
        requestStr(req), labelStr(label), unit(un), hasConversion(true), conversionFactor(con), usUnit(us) {
        codeSupport = (requestStr.compare("") != 0);
    }

    PID_INFO_t(QString req, QString label, QString un, bool hasCon, float con, QString us) :
        requestStr(req), labelStr(label), unit(un), hasConversion(hasCon), conversionFactor(con), usUnit(us) {
        codeSupport = (requestStr.compare("") != 0);
    }

    PID_INFO_t() : requestStr(""), labelStr(""), unit(""), hasConversion(false), conversionFactor(0), usUnit(""), codeSupport(false) {}

};

class OBD2Widget : public QWidget
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
                   PID_ENGINE_FUEL_RATE = 0x5E,
                   PID_EXHAUST_GAS_TEMP_11 = 0x78
                  };

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
    QString preParseResultString(QString str);
    void parseResultStr(QString str, int* a, int* b, int* c, int* d);

    //for uncommon pids...
    void parseResultStr(QString str, int* a, int* b, int* c, int* d, int* e);
    void parseResultStr(QString str, int* a, int* b, int* c, int* d, int* e, int* f, int* g);
    void parseResultStr(QString str, int* a, int* b, int* c, int* d, int* e, int* f, int* g, int* h, int* i);
    void parseResultStr(QString str, int* a, int* b, int* c, int* d, int* e, int* f, int* g, int* h, int* i, int* j, int* k, int* l, int* m);

    QBitArray decodePIDSupport(QString str);
    void loadSupportedPids();


    void setLabels();
    void setValues();

    void setDataLabel(QLabel* label, int pid);
    void setValueLabel(QLabel* label, int pid);

    struct EGT_DATA {
        bool sensor1Supported;
        bool sensor2Supported;
        bool sensor3Supported;
        bool sensor4Supported;
        float sensor1Temp;
        float sensor2Temp;
        float sensor3Temp;
        float sensor4Temp;

        EGT_DATA(bool a=false, bool b=false, bool c=false, bool d=false, float e=0.0, float f=0.0, float g=0.0, float h=0.0) :
            sensor1Supported(a), sensor2Supported(b), sensor3Supported(c), sensor4Supported(d),
            sensor1Temp(e), sensor2Temp(f), sensor3Temp(g), sensor4Temp(h) {}

        EGT_DATA(int a, int b, int c, int d, int e, int f, int g, int h, int i) {
            unsigned char tmpa = a;
            sensor1Supported = (tmpa & 0x01) > 0;
            sensor2Supported = (tmpa & 0x02) > 0;
            sensor3Supported = (tmpa & 0x04) > 0;
            sensor4Supported = (tmpa & 0x08) > 0;
            sensor1Temp = (b*256+c)/10.0 - 10.0;
            sensor2Temp = (d*256+e)/10.0 - 10.0;
            sensor3Temp = (f*256+g)/10.0 - 10.0;
            sensor4Temp = (h*256+i)/10.0 - 10.0;
        }
    };

};

#endif // OBD2WIDGET_H
