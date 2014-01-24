#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileInfo>
#include <QDirIterator>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    player = new QMediaPlayer(this);
    playlist = new QMediaPlaylist();
    player->setPlaylist(playlist);

    ui->playerInfoWidget->setPlayerAndPlaylist(player, playlist);

    playlist->setPlaybackMode(QMediaPlaylist::Random);
    loadPlaylist();

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
