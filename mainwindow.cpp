#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileInfo>
#include <QDirIterator>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings(new QSettings)
{
    ui->setupUi(this);

    //player and playlist setup
    player = new QMediaPlayer(this);
    playlist = new QMediaPlaylist();
    player->setPlaylist(playlist);
    playlist->setPlaybackMode(QMediaPlaylist::Random);
    loadPlaylist();
    ui->playerInfoWidget->setPlayerAndPlaylist(player, playlist);

    QList<int> obdPages = getObdLayout();
    if (obdPages.size() == 0) {
        //for testing only. should add setting based config
        obdPages << OBD2Widget::PID_COLLANT_TEMP << OBD2Widget::PID_TIMING_ADVANCE << OBD2Widget::PID_LONG_TERM_FUEL_TRIM << OBD2Widget::PID_SHORT_TERM_FUEL_TRIM << OBD2Widget::PID_MAF_AIRFLOW_RATE << OBD2Widget::PID_ENGINE_RPM;
        obdPages << OBD2Widget::PID_ENGINE_LOAD << OBD2Widget::PID_FUEL_PRESURE << OBD2Widget::PID_VEHICLE_SPEED << OBD2Widget::PID_INTAKE_AIR_TEMP << OBD2Widget::PID_THROTTLE_POSITION << OBD2Widget::PID_RELATIVE_THROTTLE_POSITION;
        obdPages << OBD2Widget::PID_AMBIENT_AIR_TEMP << OBD2Widget::PID_ENGINE_FUEL_RATE;
        setObdLayout(obdPages);
    }
    ui->obd2Widget->setPages(obdPages);

    QString mainStyleSheet = readStylesheetFile("main");
    ui->centralWidget->setStyleSheet(mainStyleSheet);
    qApp->setStyleSheet(mainStyleSheet);

    QString serialPortName = settings->value("obd2/serial_port/name", "N/A").toString();
    int serialPortBuadrate = settings->value("obd2/serial_port/baudrate", -1).toInt();
    if (serialPortName.compare("N/A") == 0) {
        serialPortName = "";
        settings->setValue("obd2/serial_port/name", "");
    }
    if (serialPortBuadrate == -1) {
        serialPortBuadrate = 0;
        settings->setValue("obd2/serial_port/baudrate", 0);
    }
    ui->obd2Widget->connectSerialPort(serialPortName, serialPortBuadrate);

//    connect(this->, SIGNAL())
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent ( QResizeEvent * event ) {
    qDebug() << "resizeEvent()";
}

QString MainWindow::readStylesheetFile(QString filename) {
    QFile file(":/qss/" + filename + ".qss");
    file.open(QFile::ReadOnly);
    return QString::fromLatin1(file.readAll());
}


void MainWindow::loadPlaylist() {
    playlist->clear();
    QStringList list = getMediaLocations();
    if (list.size() == 0) {
    QString tmpstr = QDir::homePath();
#if defined(Q_OS_MACX)
    tmpstr.append("/Music/iTunes/iTunes Media/Music/Daft Punk");
#else
    tmpstr.append("/Music");
#endif
    list << tmpstr;
    setMediaLocations(list);
    }
    for (int i=0; i<list.size(); i++) {
        addToPlaylist2(list.at(i));
    }
    player->play();
}

void MainWindow::addToPlaylist(QString filename) {
    qDebug()<<filename;
    QFileInfo fileInfo(filename);
    if (fileInfo.exists()) {
        if (fileInfo.isFile()) {
            QUrl url = QUrl::fromLocalFile(fileInfo.absoluteFilePath());
            if (fileInfo.suffix().toLower() == QLatin1String("m3u")) {
                playlist->load(url);
            } else
                playlist->addMedia(url);
        } else {
            QDirIterator it(filename, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QString str = it.next();
                if (!str.contains("/.", Qt::CaseInsensitive)) {
                    addToPlaylist(str);
                }
            }
        }
    } else {
        QUrl url(filename);
        if (url.isValid()) {
            playlist->addMedia(url);
        }
    }
}

void MainWindow::addToPlaylist2(QString filename) {
    qDebug()<< "addToPlaylist2(" << filename << ")";
    QFileInfo fileInfo(filename);
    if (fileInfo.isFile()) {
        QUrl url = QUrl::fromLocalFile(fileInfo.absoluteFilePath());
        if (fileInfo.suffix().toLower() == QLatin1String("m3u")) {
            playlist->load(url);
        } else {
//            qDebug() << "added: " << filename;
            playlist->addMedia(url);
        }
    } else if (fileInfo.isDir()) {
        QStringList list = parseMediaFolder(filename);
        qDebug() << "found : " << list.size();

        for (int i=0; i<list.size(); i++) {
            QString str = list.at(i);
            QFileInfo fi(str);
            QUrl url = QUrl::fromLocalFile(fi.absoluteFilePath());
            if (fi.suffix().toLower() == QLatin1String("m3u")) {
                playlist->load(url);
            } else {
//                qDebug() << "added: " << str;
                playlist->addMedia(url);
            }
        }
        qDebug() << "added " << list.size() << " files.";
    }
}

QStringList MainWindow::parseMediaFolder(QString filename) {
    QStringList sl;
    QFileInfo fileInfo(filename);
    if (fileInfo.isFile()) {
        sl.append(filename);
    } else {
        QDirIterator it(filename, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString str = it.next();
            QFileInfo fi(str);
            if (!str.contains("/.", Qt::CaseInsensitive)) {
                if (fi.isFile()) {
                    if(!sl.contains(str)) {
                        sl.append(str);
                    }
                } else if (fi.isDir()) {
                    QStringList sltmp = parseMediaFolder(str);
                    for (int i=0; i<sltmp.size(); i++) {
                        QString t = sltmp.at(i);
                        if (!sl.contains(t)) {
                            sl.append(t);
                        }
                    }
                }
            }
        }
    }
    return sl;
}


QStringList MainWindow::getMediaLocations() {
    qDebug() << "getMediaLocations()";
    QStringList list;
    settings->sync();
    int size = settings->beginReadArray("media/locations");
    for(int i=0; i<size; i++) {
        settings->setArrayIndex(i);
        list.append(settings->value("location").toString());
    }
    settings->endArray();
    return list;
}

void MainWindow::setMediaLocations(QStringList list) {
    qDebug() << "setMediaLocations()";
    settings->beginWriteArray("media/locations");
    int size = list.size();
    for(int i=0; i<size; i++) {
        settings->setArrayIndex(i);
        settings->setValue("location", list.at(i));
    }
    settings->endArray();
    settings->sync();
}

QList<int> MainWindow::getObdLayout() {
    qDebug() << "getObdLayout()";
    QList<int> list;
    settings->sync();
    int size = settings->beginReadArray("obd2/layout");
    for(int i=0; i<size; i++) {
        settings->setArrayIndex(i);
        list.append(settings->value("pid").toInt());
    }
    settings->endArray();
    return list;
}

void MainWindow::setObdLayout(QList<int> list) {
    qDebug() << "setObdLayout()";
    settings->beginWriteArray("obd2/layout");
    int size = list.size();
    for(int i=0; i<size; i++) {
        settings->setArrayIndex(i);
        settings->setValue("pid", list.at(i));
    }
    settings->endArray();
    settings->sync();
}
