#include "playerinfowidget.h"
#include "ui_playerinfowidget.h"

#include <QTime>
#include <QMediaMetaData>

PlayerInfoWidget::PlayerInfoWidget(QWidget *parent, QMediaPlayer *player, QMediaPlaylist *playlist) :
    QWidget(parent),
    ui(new Ui::PlayerInfoWidget),
    m_player(player),
    m_playlist(playlist)
{
    ui->setupUi(this);

    ui->nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui->prevButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));

    if (m_player == NULL || m_playlist == NULL) {
        m_playerSet = false;
    } else {
        m_playerSet = true;
    }

    ui->mediaButton->setEnabled(m_playerSet);
    ui->playButton->setEnabled(m_playerSet);
    ui->nextButton->setEnabled(m_playerSet);
    ui->prevButton->setEnabled(m_playerSet);
    ui->playbackSlider->setEnabled(m_playerSet);
    if (m_playerSet) {
        makeInternalConnections();
    }
}

PlayerInfoWidget::~PlayerInfoWidget()
{
    delete ui;
}

void PlayerInfoWidget::setPlayerAndPlaylist(QMediaPlayer *player, QMediaPlaylist *playlist) {
    if (m_playerSet) {
        diconnectInternalConnections();
    }
    m_player = player;
    m_playlist = playlist;
    m_playerSet = true;
    ui->mediaButton->setEnabled(m_playerSet);
    ui->playButton->setEnabled(m_playerSet);
    ui->nextButton->setEnabled(m_playerSet);
    ui->prevButton->setEnabled(m_playerSet);
    ui->playbackSlider->setEnabled(m_playerSet);
    makeInternalConnections();
}

void PlayerInfoWidget::makeInternalConnections() {
    // player signal connections
    connect(m_player, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(m_player, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));
    connect(m_player, SIGNAL(metaDataChanged()), SLOT(metaDataChanged()));
    connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));

    // control button signals
    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(playClicked()));
    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(nextClicked()));
    connect(ui->prevButton, SIGNAL(clicked()), this, SLOT(prevClicked()));
    connect(ui->playbackSlider, SIGNAL(sliderMoved(int)), this, SLOT(seek(int)));
    connect(ui->playbackSlider, SIGNAL(valueChanged(int)), this, SLOT(seek(int)));
}

void PlayerInfoWidget::diconnectInternalConnections() {
    // player signal connections
//    disconnect(m_player, &QMediaPlayer::durationChanged, )
    disconnect(m_player, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));
    disconnect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    disconnect(m_player, SIGNAL(metaDataChanged()), this, SLOT(metaDataChanged()));
    disconnect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));

    // control button signals
    disconnect(ui->playButton, SIGNAL(clicked()), this, SLOT(playClicked()));
    disconnect(ui->nextButton, SIGNAL(clicked()), this, SLOT(nextClicked()));
    disconnect(ui->prevButton, SIGNAL(clicked()), this, SLOT(prevClicked()));
    disconnect(ui->playbackSlider, SIGNAL(sliderMoved(int)), this, SLOT(seek(int)));
    disconnect(ui->playbackSlider, SIGNAL(valueChanged(int)), this, SLOT(seek(int)));
}

void PlayerInfoWidget::durationChanged(qint64 duration) {
    ui->durationLabel->setText(timeToString(duration));
    ui->playbackSlider->setMaximum(duration);
}

void PlayerInfoWidget::positionChanged(qint64 progress) {
    ui->positionLabel->setText(timeToString(progress));
    m_pos = progress / 1000;
    if (!ui->playbackSlider->isSliderDown()) {
        ui->playbackSlider->setValue(progress);
    }
}

void PlayerInfoWidget::metaDataChanged() {
    qDebug()<<"metaDataChanged()";
    if (m_player->isMetaDataAvailable()) {
        ui->titleLabel->setText(m_player->metaData(QMediaMetaData::Title).toString());
        ui->albumLabel->setText(m_player->metaData(QMediaMetaData::AlbumArtist).toString());
    } else {
        ui->titleLabel->setText("-");
        ui->albumLabel->setText("-");
    }
}

void PlayerInfoWidget::statusChanged(QMediaPlayer::MediaStatus status) {
    qDebug()<<"statusChanged()";
    switch (status) {
    case QMediaPlayer::StoppedState:
    case QMediaPlayer::PausedState:
//        play();
        m_player->play();
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    case QMediaPlayer::PlayingState:
//        pause();
        m_player->pause();
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    default:
        break;
    }
}

void PlayerInfoWidget::setState(QMediaPlayer::State state) {
    qDebug()<<"setState() needs to be finished";
}


void PlayerInfoWidget::playClicked() {
    qDebug()<<"playClicked()";
    switch (m_player->state()) {
    case QMediaPlayer::StoppedState:
    case QMediaPlayer::PausedState:
        m_player->play();
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    case QMediaPlayer::PlayingState:
        m_player->pause();
        ui->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}

void PlayerInfoWidget::nextClicked() {
    qDebug()<<"nextClicked()";
    m_playlist->shuffle();
    m_playlist->next();
}

void PlayerInfoWidget::prevClicked() {
    qDebug()<<"prevClicked()";
    if(m_player->position() <= 5000)
        m_playlist->previous();
    else
        m_player->setPosition(0);

}

void PlayerInfoWidget::seek(int seconds) {
    if (seconds/1000 != m_pos) {
        qDebug()<<"seek()";
        m_player->setPosition(seconds);
    }
}

QString PlayerInfoWidget::timeToString(qint64 t) {
    QTime currentTime((t/3600000)%60, (t/60000)%60, (t/1000)%60, t%1000);
    QString format = "mm:ss";
    QString out = currentTime.toString(format);
    return out;
}


