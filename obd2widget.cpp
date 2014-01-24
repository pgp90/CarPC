#include "obd2widget.h"
#include "ui_obd2widget.h"
#include <cmath>

#include <QDebug>
#include <QTimer>

OBD2Widget::OBD2Widget(QWidget *parent, QString portName, int baudrate) :
    QFrame(parent),
    ui(new Ui::OBD2Widget)
{
    ui->setupUi(this);

    connect(ui->prevPageButton, SIGNAL(clicked()), this, SLOT(prevPageClicked()));
    connect(ui->nextPageButton, SIGNAL(clicked()), this, SLOT(nextPageClicked()));


    obdSerial = new QSerialPort(this);

    connectSerialPort(portName, baudrate);

    m_obdPages << OBD_PAGE_t();

    pidInfo[PID_ENGINE_LOAD] =                  PID_INFO_t(1, "0104", "Engine Load",                    "%1 \%");
    pidInfo[PID_COLLANT_TEMP] =                 PID_INFO_t(1, "0105", "Coolant Temp",                   "%1 ºF");
    pidInfo[PID_SHORT_TERM_FUEL_TRIM] =         PID_INFO_t(1, "0106", "Short-Term Fuel Trim",           "%1 \%");
    pidInfo[PID_LONG_TERM_FUEL_TRIM] =          PID_INFO_t(1, "0107", "Long-Term Fuel Trim",            "%1 \%");
    pidInfo[PID_FUEL_PRESURE] =                 PID_INFO_t(1, "010A", "Fuel Pressure",                  "%1 kPa");
    pidInfo[PID_ENGINE_RPM] =                   PID_INFO_t(1, "010C", "Engine RPM",                     "%1 RPM");
    pidInfo[PID_VEHICLE_SPEED] =                PID_INFO_t(1, "010D", "Vehicle Speed",                  "%1 km/h");
    pidInfo[PID_TIMING_ADVANCE] =               PID_INFO_t(1, "010E", "Timing Adv.",                    "%1 º");
    pidInfo[PID_INTAKE_AIR_TEMP] =              PID_INFO_t(1, "010F", "Intake Air Temp",                "%1 ºC");
    pidInfo[PID_MAF_AIRFLOW_RATE] =             PID_INFO_t(1, "0110", "MAF",                            "%1 g/s");
    pidInfo[PID_THROTTLE_POSITION] =            PID_INFO_t(1, "0111", "Throttle Position",              "%1 \%");
    pidInfo[PID_RELATIVE_THROTTLE_POSITION] =   PID_INFO_t(1, "0145", "Relative Throttle Position",     "%1 \%");
    pidInfo[PID_AMBIENT_AIR_TEMP] =             PID_INFO_t(1, "0146", "Ambient Ait Temp",               "%1 ºC");
    pidInfo[PID_ENGINE_FUEL_RATE] =             PID_INFO_t(1, "015E", "Engine Fuel Rate",               "%1 L/h");

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
    }
    if (!obdSerial->setBaudRate(baudrate)) {
        qDebug() << QObject::tr("Failed to set %1 baud for port %2, error: %3").arg(baudrate).arg(portName).arg(obdSerial->errorString()) << endl;
        isSerialGood = false;
    }
    if (!obdSerial->setDataBits(QSerialPort::Data8)) {
        qDebug() << QObject::tr("Failed to set 8 data bits for port %1, error: %2").arg(portName).arg(obdSerial->errorString()) << endl;
        isSerialGood = false;
    }
    if (!obdSerial->setParity(QSerialPort::NoParity)) {
        qDebug() << QObject::tr("Failed to set no parity for port %1, error: %2").arg(portName).arg(obdSerial->errorString()) << endl;
        isSerialGood = false;
    }
    if (!obdSerial->setStopBits(QSerialPort::OneStop)) {
        qDebug() << QObject::tr("Failed to set 1 stop bit for port %1, error: %2").arg(portName).arg(obdSerial->errorString()) << endl;
        isSerialGood = false;
    }
    if (!obdSerial->setFlowControl(QSerialPort::NoFlowControl)) {
        qDebug() << QObject::tr("Failed to set no flow control for port %1, error: %2").arg(portName).arg(obdSerial->errorString()) << endl;
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
    setDataLabel(ui->itemLabel0, pageInfo.block1);
    setDataLabel(ui->itemLabel1, pageInfo.block2);
    setDataLabel(ui->itemLabel2, pageInfo.block3);
    setDataLabel(ui->itemLabel3, pageInfo.block4);
    setDataLabel(ui->itemLabel4, pageInfo.block5);
    setDataLabel(ui->itemLabel5, pageInfo.block6);
}

void OBD2Widget::setValues() {
    OBD_PAGE_t pageInfo = m_obdPages[pageNum];
    setValueLabel(ui->itemValue0, pageInfo.block1);
    setValueLabel(ui->itemValue1, pageInfo.block2);
    setValueLabel(ui->itemValue2, pageInfo.block3);
    setValueLabel(ui->itemValue3, pageInfo.block4);
    setValueLabel(ui->itemValue4, pageInfo.block5);
    setValueLabel(ui->itemValue5, pageInfo.block6);
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
    QString format = ((PID_INFO_t)pidInfo[pid]).valueStr;

    if (str.compare("") == 0) {
        format = format.arg("-");
    } else {
        QString s;
        int A, B, C, D;

        parseResultStr(str, &A, &B, &C, &D);

        switch (pid) {
        case PID_AMBIENT_AIR_TEMP:
        case PID_INTAKE_AIR_TEMP:
        case PID_COLLANT_TEMP:
            s = s.number(A-40);
            break;
        case PID_ENGINE_RPM:
            s = s.number((float)((A*256)+B)/4.0);
            break;
        case PID_LONG_TERM_FUEL_TRIM:
        case PID_SHORT_TERM_FUEL_TRIM:
            s = s.number((float)((A-128)*100)/128.0);
            break;
        case PID_MAF_AIRFLOW_RATE:
            s = s.number((float)((A*256)+B)/100.0);
            break;
        case PID_TIMING_ADVANCE:
            s = s.number((float)A/2.0-64.0);
            break;
        case PID_ENGINE_LOAD:
            s = s.number((float)(A*100)/255.0);
            break;
        case PID_FUEL_PRESURE:
            s = s.number(A*3);
            break;
        case PID_VEHICLE_SPEED:
            s = s.number(A);
            break;
        case PID_THROTTLE_POSITION:
        case PID_RELATIVE_THROTTLE_POSITION:
            s = s.number((float)(A*100)/255.0);
            break;
        case PID_ENGINE_FUEL_RATE:
            s = s.number((float)(A*100)*0.05);
            break;
        default:
            break;
        }
        format = format.arg(s);
    }
    return format;
}

void OBD2Widget::parseResultStr(QString str, int* a, int* b, int* c, int* d) {
    bool ok;
    str.replace(" ", "");
//    qDebug() << "str: " << str;
    QString str2 = str.mid(4);
//    qDebug() << "str2: " << str2;
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
