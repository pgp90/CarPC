#include "obd2widget.h"
#include "ui_obd2widget.h"
#include <cmath>

#include <QDebug>
#include <QTimer>

OBD2Widget::OBD2Widget(QWidget *parent, QString portName, int baudrate) :
    QWidget(parent),
    ui(new Ui::OBD2Widget)
{
    ui->setupUi(this);

    ui->nextPageButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    ui->prevPageButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));

    connect(ui->prevPageButton, SIGNAL(clicked()), this, SLOT(prevPageClicked()));
    connect(ui->nextPageButton, SIGNAL(clicked()), this, SLOT(nextPageClicked()));


    obdSerial = new QSerialPort(this);

    connectSerialPort(portName, baudrate);

    m_obdPages << OBD_PAGE_t();

    pidInfo[PID_ENGINE_LOAD] =                  PID_INFO_t("0104", "Engine Load",                       "\%"    );
    pidInfo[PID_COLLANT_TEMP] =                 PID_INFO_t("0105", "Coolant Temp",                      "ºC",   9.0*5.0+32.0  , "ºF");
    pidInfo[PID_SHORT_TERM_FUEL_TRIM] =         PID_INFO_t("0106", "Short-Term Fuel Trim bank 1",       "\%"    );
    pidInfo[PID_LONG_TERM_FUEL_TRIM] =          PID_INFO_t("0107", "Long-Term Fuel Trim bank 1",        "\%"    );
    pidInfo[PID_FUEL_PRESURE] =                 PID_INFO_t("010A", "Fuel Pressure",                     "kPa"   );
    pidInfo[PID_ENGINE_RPM] =                   PID_INFO_t("010C", "Engine RPM",                        "RPM"   );
    pidInfo[PID_VEHICLE_SPEED] =                PID_INFO_t("010D", "Vehicle Speed",                     "km/h", 0.621371      , "mph");
    pidInfo[PID_TIMING_ADVANCE] =               PID_INFO_t("010E", "Timing Adv.",                       "º"     );
    pidInfo[PID_INTAKE_AIR_TEMP] =              PID_INFO_t("010F", "Intake Air Temp",                   "ºC",   9.0*5.0+32.0  , "ºF");
    pidInfo[PID_MAF_AIRFLOW_RATE] =             PID_INFO_t("0110", "MAF",                               "g/s"   );
    pidInfo[PID_THROTTLE_POSITION] =            PID_INFO_t("0111", "Throttle Position",                 "\%"    );
    pidInfo[PID_RELATIVE_THROTTLE_POSITION] =   PID_INFO_t("0145", "Relative Throttle Position",        "\%"    );
    pidInfo[PID_AMBIENT_AIR_TEMP] =             PID_INFO_t("0146", "Ambient Air Temp",                  "ºC",   9.0*5.0+32.0  , "ºF");
    pidInfo[PID_ENGINE_FUEL_RATE] =             PID_INFO_t("015E", "Engine Fuel Rate",                  "L/h"   );
    pidInfo[PID_EXHAUST_GAS_TEMP_11] =          PID_INFO_t("0178", "Exhaust Gas temp bank 1, sen 1",    "ºC",   9.0*5.0+32.0  , "ºF");

    for ( int i=0; i < 0xE0; i++) {
        pidSupported[i] = false;
    }
    pidSupported[0] = true;

    pageNum = 0;

    loadSupportedPids();

    setLabels();
    setValues();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateObdValues()));
    timer->start(1000);

}

OBD2Widget::~OBD2Widget()
{
    obdSerial->close();
    delete ui;
}

void OBD2Widget::connectSerialPort(QString portName, int baudrate) {
    obdSerial->close();

    obdSerial->setPortName(portName);

    isSerialGood = true;

    if (!obdSerial->open(QIODevice::ReadWrite)) {
        qDebug() << QObject::tr("Failed to open port %1, error: %2").arg(portName).arg(obdSerial->errorString()) << endl;
        isSerialGood = false;
    } else if (!obdSerial->setBaudRate(baudrate)) {
        qDebug() << QObject::tr("Failed to set %1 baud for port %2, error: %3").arg(baudrate).arg(portName).arg(obdSerial->errorString()) << endl;
        obdSerial->close();
        isSerialGood = false;
    } else if (!obdSerial->setDataBits(QSerialPort::Data8)) {
        qDebug() << QObject::tr("Failed to set 8 data bits for port %1, error: %2").arg(portName).arg(obdSerial->errorString()) << endl;
        obdSerial->close();
        isSerialGood = false;
    } else if (!obdSerial->setParity(QSerialPort::NoParity)) {
        qDebug() << QObject::tr("Failed to set no parity for port %1, error: %2").arg(portName).arg(obdSerial->errorString()) << endl;
        obdSerial->close();
        isSerialGood = false;
    } else if (!obdSerial->setStopBits(QSerialPort::OneStop)) {
        qDebug() << QObject::tr("Failed to set 1 stop bit for port %1, error: %2").arg(portName).arg(obdSerial->errorString()) << endl;
        obdSerial->close();
        isSerialGood = false;
    } else if (!obdSerial->setFlowControl(QSerialPort::NoFlowControl)) {
        qDebug() << QObject::tr("Failed to set no flow control for port %1, error: %2").arg(portName).arg(obdSerial->errorString()) << endl;
        obdSerial->close();
        isSerialGood = false;
    }
}

void OBD2Widget::setPages(QList<int> pageitems) {
    int numOfPages = floor((pageitems.size() + 5) / 6.0);
    m_obdPages.clear();
    for(int page =0; page<numOfPages-1; page++) {
        m_obdPages << OBD_PAGE_t(pageitems[page*6+0], pageitems[page*6+1], pageitems[page*6+2], pageitems[page*6+3], pageitems[page*6+4], pageitems[page*6+5]);
    }
    int p = numOfPages - 1;
    int rem = pageitems.size() - p*6;
    switch (rem) {
    case 1:
        m_obdPages << OBD_PAGE_t(pageitems[p*6+0]);
        break;
    case 2:
        m_obdPages << OBD_PAGE_t(pageitems[p*6+0], pageitems[p*6+1]);
        break;
    case 3:
        m_obdPages << OBD_PAGE_t(pageitems[p*6+0], pageitems[p*6+1], pageitems[p*6+2]);
        break;
    case 4:
        m_obdPages << OBD_PAGE_t(pageitems[p*6+0], pageitems[p*6+1], pageitems[p*6+2], pageitems[p*6+3]);
        break;
    case 5:
        m_obdPages << OBD_PAGE_t(pageitems[p*6+0], pageitems[p*6+1], pageitems[p*6+2], pageitems[p*6+3], pageitems[p*6+4]);
        break;
    case 6:
        m_obdPages << OBD_PAGE_t(pageitems[p*6+0], pageitems[p*6+1], pageitems[p*6+2], pageitems[p*6+3], pageitems[p*6+4], pageitems[p*6+5]);
        break;
    default:
        break;
    }
    if (pageNum >= m_obdPages.size()) {
        pageNum = 0;
    }
    setLabels();
    setValues();

//    m_obdPages = obdPages;
}


void OBD2Widget::nextPageClicked() {
    pageNum++;
    if (pageNum >= m_obdPages.size()) {
        pageNum = 0;
    }
    setLabels();
    setValues();
}

void OBD2Widget::prevPageClicked() {
    pageNum--;
    if (pageNum < 0) {
        pageNum = m_obdPages.size()-1;
    }
    setLabels();
    setValues();
}


void OBD2Widget::updateObdValues() {
    qDebug()<<"updateObdValues()";
    setValues();
}

void OBD2Widget::setLabels() {
    OBD_PAGE_t pageInfo = m_obdPages[pageNum];
    setDataLabel(ui->itemLabel0, pageInfo.block0);
    setDataLabel(ui->itemLabel1, pageInfo.block1);
    setDataLabel(ui->itemLabel2, pageInfo.block2);
    setDataLabel(ui->itemLabel3, pageInfo.block3);
    setDataLabel(ui->itemLabel4, pageInfo.block4);
    setDataLabel(ui->itemLabel5, pageInfo.block5);
}

void OBD2Widget::setValues() {
    OBD_PAGE_t pageInfo = m_obdPages[pageNum];
    setValueLabel(ui->itemValue0, pageInfo.block0);
    setValueLabel(ui->itemValue1, pageInfo.block1);
    setValueLabel(ui->itemValue2, pageInfo.block2);
    setValueLabel(ui->itemValue3, pageInfo.block3);
    setValueLabel(ui->itemValue4, pageInfo.block4);
    setValueLabel(ui->itemValue5, pageInfo.block5);
}


void OBD2Widget::setDataLabel(QLabel* label, int pid) {
    if (pid == -1 || !((PID_INFO_t)pidInfo[pid]).codeSupport) {
        label->setText("");
    } else {
        QString labelStr = ((PID_INFO_t)pidInfo[pid]).labelStr;
        label->setText(labelStr);
    }
}

void OBD2Widget::setValueLabel(QLabel* label, int pid) {
    if (pid == -1 || !((PID_INFO_t)pidInfo[pid]).codeSupport) {
        label->setText("");
    } else {
        QString response = obdRequest(((PID_INFO_t)pidInfo[pid]).requestStr);
        QString str = parseResult(response, pid);
        label->setText(str);
    }
}


QString OBD2Widget::obdRequest(QString req) {
    if (isSerialGood) {
        obdSerial->clear();
        qint64 bytesWritten = obdSerial->write(req.toLocal8Bit());

        if (bytesWritten == -1) {
            qDebug() << QObject::tr("Failed to write the data to port %1, error: %2").arg(obdSerial->portName()).arg(obdSerial->errorString()) << endl;
            return "";
        } else if (bytesWritten != req.length()) {
            qDebug() << QObject::tr("Failed to write all the data to port %1, error: %2").arg(obdSerial->portName()).arg(obdSerial->errorString()) << endl;
            return "";
        } else if (!obdSerial->waitForBytesWritten(5000)) {
            qDebug() << QObject::tr("Operation timed out or an error occurred for port %1, error: %2").arg(obdSerial->portName()).arg(obdSerial->errorString()) << endl;
            return "";
        }
        while (obdSerial->bytesAvailable() < 4) {}
        QString r;
        while (!(r.endsWith("\n"))) {
            QByteArray readData2 = obdSerial->readAll();
            r.append(readData2);
        }
        QStringList sl = r.split("\n");
        QString out = sl[0];
        return out;
    } else {
        return "";
    }
}


QString OBD2Widget::parseResult(QString str, int pid) {
    //    qDebug() << "OBD::parseValue(\"" << str << "\", OBD2_PID)";
    QString value;
    PID_INFO_t info = pidInfo[pid];
    QString unit = info.unit;

    if (str.compare("") == 0) {
        value = "-";
    } else {
        int A, B, C, D;
//        float v;

        parseResultStr(str, &A, &B, &C, &D);

        switch (pid) {
        case PID_AMBIENT_AIR_TEMP:
        case PID_INTAKE_AIR_TEMP:
        case PID_COLLANT_TEMP:
//            v = A-40;
//            if (info.hasConversion && m_useConversion)
            value = value.number(A-40);
            break;
        case PID_ENGINE_RPM:
            value = value.number((float)((A*256)+B)/4.0);
            break;
        case PID_LONG_TERM_FUEL_TRIM:
        case PID_SHORT_TERM_FUEL_TRIM:
            value = value.number((float)((A-128)*100)/128.0);
            break;
        case PID_MAF_AIRFLOW_RATE:
            value = value.number((float)((A*256)+B)/100.0);
            break;
        case PID_TIMING_ADVANCE:
            value = value.number((float)A/2.0-64.0);
            break;
        case PID_ENGINE_LOAD:
            value = value.number((float)(A*100)/255.0);
            break;
        case PID_FUEL_PRESURE:
            value = value.number(A*3);
            break;
        case PID_VEHICLE_SPEED:
            value = value.number(A);
            break;
        case PID_THROTTLE_POSITION:
        case PID_RELATIVE_THROTTLE_POSITION:
            value = value.number((float)(A*100)/255.0);
            break;
        case PID_ENGINE_FUEL_RATE:
            value = value.number((float)(A*100)*0.05);
            break;
        default:
            break;
        case PID_EXHAUST_GAS_TEMP_11:
            int E, F, G, H, I;
            parseResultStr(str, &A, &B, &C, &D, &E, &F, &G, &H, &I);
            EGT_DATA egt_data = EGT_DATA(A, B, C, D, E, F, G, H, I);
            if (egt_data.sensor1Supported) {
                value = value.number(egt_data.sensor1Temp);
            } else {
                value = "-";
            }
            break;
        }
    }

    return value.append(" ").append(unit);
}

QString OBD2Widget::preParseResultString(QString str){
    str.replace(" ", "");
//    qDebug() << "str: " << str;
    QString str2 = str.mid(4);
//    qDebug() << "str2: " << str2;
    return str2;
}

void OBD2Widget::parseResultStr(QString str, int* a, int* b, int* c, int* d) {
    bool ok;
    QString str2 = preParseResultString(str);
    *a = str2.mid(0, 2).toInt(&ok, 16);
    *b = str2.mid(2, 2).toInt(&ok, 16);
    *c = str2.mid(4, 2).toInt(&ok, 16);
    *d = str2.mid(6, 2).toInt(&ok, 16);
}

QBitArray OBD2Widget::decodePIDSupport(QString str) {
    str.replace(" ", "");
    QString str2 = str.right(8);
    QBitArray bits(32);
    bool ok;
    for (int B=0; B<4; B++) {
        int v = str2.mid(0, 2).toInt(&ok, 16);
        for (int b=0; b<8; b++) {
            bits.setBit(B*8+b, v&(1<<(7-b)));
        }
    }
    return bits;
}

void OBD2Widget::loadSupportedPids() {
    if (isSerialGood) {
        QBitArray bitsa = decodePIDSupport(obdRequest("0100"));
        for (int i=0; i<0x20; i++) {
            pidSupported[0x01+i] = bitsa.testBit(i);
        }
        if (pidSupported[0x20]) {
            QBitArray bitsa = decodePIDSupport(obdRequest("0120"));
            for (int i=0; i<0x20; i++) {
                pidSupported[0201+i] = bitsa.testBit(i);
            }
            if (pidSupported[0x40]) {
                QBitArray bitsa = decodePIDSupport(obdRequest("0140"));
                for (int i=0; i<0x20; i++) {
                    pidSupported[0x41+i] = bitsa.testBit(i);
                }
                if (pidSupported[0x60]) {
                    QBitArray bitsa = decodePIDSupport(obdRequest("0160"));
                    for (int i=0; i<0x20; i++) {
                        pidSupported[0x61+i] = bitsa.testBit(i);
                    }
                    if (pidSupported[0x80]) {
                        QBitArray bitsa = decodePIDSupport(obdRequest("0180"));
                        for (int i=0; i<0x20; i++) {
                            pidSupported[0x81+i] = bitsa.testBit(i);
                        }
                        if (pidSupported[0xA0]) {
                            QBitArray bitsa = decodePIDSupport(obdRequest("01A0"));
                            for (int i=0; i<0x20; i++) {
                                pidSupported[0xA1+i] = bitsa.testBit(i);
                            }
                            if (pidSupported[0xC0]) {
                                QBitArray bitsa = decodePIDSupport(obdRequest("01C0"));
                                for (int i=0; i<0x20; i++) {
                                    pidSupported[0xC1+i] = bitsa.testBit(i);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//for uncommon pids...
void OBD2Widget::parseResultStr(QString str, int* a, int* b, int* c, int* d, int* e) {
    bool ok;
    QString str2 = preParseResultString(str);
    *a = str2.mid(0, 2).toInt(&ok, 16);
    *b = str2.mid(2, 2).toInt(&ok, 16);
    *c = str2.mid(4, 2).toInt(&ok, 16);
    *d = str2.mid(6, 2).toInt(&ok, 16);
    *e = str2.mid(8, 2).toInt(&ok, 16);
}

void OBD2Widget::parseResultStr(QString str, int* a, int* b, int* c, int* d, int* e, int* f, int* g) {
    bool ok;
    QString str2 = preParseResultString(str);
    *a = str2.mid(0, 2).toInt(&ok, 16);
    *b = str2.mid(2, 2).toInt(&ok, 16);
    *c = str2.mid(4, 2).toInt(&ok, 16);
    *d = str2.mid(6, 2).toInt(&ok, 16);
    *e = str2.mid(8, 2).toInt(&ok, 16);
    *f = str2.mid(10, 2).toInt(&ok, 16);
    *g = str2.mid(12, 2).toInt(&ok, 16);
}

void OBD2Widget::parseResultStr(QString str, int* a, int* b, int* c, int* d, int* e, int* f, int* g, int* h, int* i) {
    bool ok;
    QString str2 = preParseResultString(str);
    *a = str2.mid(0, 2).toInt(&ok, 16);
    *b = str2.mid(2, 2).toInt(&ok, 16);
    *c = str2.mid(4, 2).toInt(&ok, 16);
    *d = str2.mid(6, 2).toInt(&ok, 16);
    *e = str2.mid(8, 2).toInt(&ok, 16);
    *f = str2.mid(10, 2).toInt(&ok, 16);
    *g = str2.mid(12, 2).toInt(&ok, 16);
    *h = str2.mid(14, 2).toInt(&ok, 16);
    *i = str2.mid(16, 2).toInt(&ok, 16);
}

void OBD2Widget::parseResultStr(QString str, int* a, int* b, int* c, int* d, int* e, int* f, int* g, int* h, int* i, int* j, int* k, int* l, int* m) {
    bool ok;
    QString str2 = preParseResultString(str);
    *a = str2.mid(0, 2).toInt(&ok, 16);
    *b = str2.mid(2, 2).toInt(&ok, 16);
    *c = str2.mid(4, 2).toInt(&ok, 16);
    *d = str2.mid(6, 2).toInt(&ok, 16);
    *e = str2.mid(8, 2).toInt(&ok, 16);
    *f = str2.mid(10, 2).toInt(&ok, 16);
    *g = str2.mid(12, 2).toInt(&ok, 16);
    *h = str2.mid(14, 2).toInt(&ok, 16);
    *i = str2.mid(16, 2).toInt(&ok, 16);
    *j = str2.mid(18, 2).toInt(&ok, 16);
    *k = str2.mid(20, 2).toInt(&ok, 16);
    *l = str2.mid(22, 2).toInt(&ok, 16);
    *m = str2.mid(24, 2).toInt(&ok, 16);
}
