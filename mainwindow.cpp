#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileInfo>
#include <QDirIterator>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //player and playlist setup
    player = new QMediaPlayer(this);
    playlist = new QMediaPlaylist();
    player->setPlaylist(playlist);
    playlist->setPlaybackMode(QMediaPlaylist::Random);
    loadPlaylist();
    ui->playerInfoWidget->setPlayerAndPlaylist(player, playlist);

    QList<int> obdPages;
    //for testing only. should add setting based config
//    obdPages.push_back(OBD_PAGE_t(OBD::PID_COLLANT_TEMP, OBD::PID_TIMING_ADVANCE, OBD::PID_LONG_TERM_FUEL_TRIM, OBD::PID_SHORT_TERM_FUEL_TRIM, OBD::PID_MAF_AIRFLOW_RATE, OBD::PID_ENGINE_RPM));
//    obdPages.push_back(OBD_PAGE_t(OBD::PID_ENGINE_LOAD, OBD::PID_FUEL_PRESURE, OBD::PID_VEHICLE_SPEED, OBD::PID_INTAKE_AIR_TEMP, OBD::PID_THROTTLE_POSITION, OBD::PID_RELATIVE_THROTTLE_POSITION));
//    obdPages.push_back(OBD_PAGE_t(OBD::PID_AMBIENT_AIR_TEMP, OBD::PID_ENGINE_FUEL_RATE));
    obdPages << OBD2Widget::PID_COLLANT_TEMP << OBD2Widget::PID_TIMING_ADVANCE << OBD2Widget::PID_LONG_TERM_FUEL_TRIM << OBD2Widget::PID_SHORT_TERM_FUEL_TRIM << OBD2Widget::PID_MAF_AIRFLOW_RATE << OBD2Widget::PID_ENGINE_RPM;
    obdPages << OBD2Widget::PID_ENGINE_LOAD << OBD2Widget::PID_FUEL_PRESURE << OBD2Widget::PID_VEHICLE_SPEED << OBD2Widget::PID_INTAKE_AIR_TEMP << OBD2Widget::PID_THROTTLE_POSITION << OBD2Widget::PID_RELATIVE_THROTTLE_POSITION;
    obdPages << OBD2Widget::PID_AMBIENT_AIR_TEMP << OBD2Widget::PID_ENGINE_FUEL_RATE;
    ui->obd2Widget->setPages(obdPages);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadPlaylist() {
    playlist->clear();
    QStringList list;// = getMediaLocations();
    list << "/Users/jpalnick/Music/iTunes/iTunes Media/Music/Daft Punk";
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
    qDebug()<<filename;
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
