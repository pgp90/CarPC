#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QString>
#include <QStringList>
#include <QSettings>

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

    QSettings *settings;

    QString readStylesheetFile(QString filename);

    void loadPlaylist();
//    void updateMediaLocationsList();
    void addToPlaylist(QString filename);
    void addToPlaylist2(QString filename);
    QStringList parseMediaFolder(QString filename);

    void resizeEvent ( QResizeEvent * event );

    QStringList getMediaLocations();
    void setMediaLocations(QStringList list);
    QList<int> getObdLayout();
    void setObdLayout(QList<int> list);

};

#endif // MAINWINDOW_H
