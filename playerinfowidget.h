#ifndef PLAYERINFOWIDGET_H
#define PLAYERINFOWIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QString>

namespace Ui {
class PlayerInfoWidget;
}

class PlayerInfoWidget : public QWidget
{
    Q_OBJECT

public:
//    explicit PlayerInfoWidget(QWidget *parent = 0);
    explicit PlayerInfoWidget(QWidget *parent = 0, QMediaPlayer *player = 0, QMediaPlaylist *playlist = 0);
    ~PlayerInfoWidget();

    void setPlayerAndPlaylist(QMediaPlayer *player, QMediaPlaylist *playlist);
    void diconnectInternalConnections();

private slots:
    void durationChanged(qint64 duration);
    void positionChanged(qint64 progress);
    void metaDataChanged();
    void statusChanged(QMediaPlayer::MediaStatus status);
    void setState(QMediaPlayer::State state);

    void prevClicked();
    void nextClicked();
    void playClicked();
    void seek(int seconds);

//public slots:
//    void setValue(int value);

//signals:

private:
    Ui::PlayerInfoWidget *ui;

    qint64 m_pos;

    QMediaPlayer *m_player;
    QMediaPlaylist *m_playlist;
    bool m_playerSet;

    void makeInternalConnections();

    QString timeToString(qint64 t);

};

#endif // PLAYERINFOWIDGET_H
