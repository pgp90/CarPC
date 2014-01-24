#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QString>
#include <QStringList>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QMediaPlayer *player;
    QMediaPlaylist *playlist;


    void loadPlaylist();
//    void updateMediaLocationsList();
    void addToPlaylist(QString filename);
    void addToPlaylist2(QString filename);
    QStringList parseMediaFolder(QString filename);

};

#endif // MAINWINDOW_H