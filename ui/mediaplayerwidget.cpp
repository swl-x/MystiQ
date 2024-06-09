/*  MystiQ - a C++/Qt5 gui frontend for ffmpeg
 *  Copyright (C) 2011-2019 Maikel Llamaret Heredia <llamaret@webmisolutions.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QSettings>
#include <QWheelEvent>
#include <QMediaPlayer>
#include <QVideoWidget>

#include "mediaplayerwidget.h"
#include "ui_mediaplayerwidget.h"
#include "services/constants.h"

#define DEFAULT_VOLUME Constants::getInteger("MediaPlayer/DefaultVolume")
#define SLIDER_STYLESHEET Constants::getString("MediaPlayer/SliderStyle")
#define MINIMUM_HEIGHT 210

#define MAX_VOLUME 100
#define VOLUME_SETTING_KEY "mediaplayer/volume"

namespace {
QString sec2hms(qint64 miliseconds)
{
    qint64 seconds = miliseconds / 1000;

    int h = static_cast<int> (seconds / 3600);
    int m = (seconds % 3600) / 60;
    int s = (seconds % 60);

    QString result;
    result.asprintf("%02d:%02d:%02d", h, m, s);
    return result;
}
}

MediaPlayerWidget::MediaPlayerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaPlayerWidget),
    m_beginSec(0),
    m_playUntil(-1)
{
    ui->setupUi(this);
    ui->slideVolume->setRange(0, MAX_VOLUME);

    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput();
    m_mediaPlayer->setAudioOutput(m_audioOutput);

    m_videoView = new QVideoWidget(this);

    ui->layoutPlayer->addWidget(m_videoView);

    m_mediaPlayer->setVideoOutput(m_videoView);

    ui->slideSeek->setStyleSheet(SLIDER_STYLESHEET);

    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, [this] {
        refreshTimeDisplay();
        refreshButtonState();
        emit stateChanged();
    });

    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, [this] {
        if (m_playUntil > 0
            && m_mediaPlayer->position() >= m_playUntil * 1000
            && m_mediaPlayer->playbackState() != QMediaPlayer::PausedState)
        {
            pause();

            if (m_mediaPlayer->position() > m_playUntil * 1000)
            {
                m_mediaPlayer->setPosition(m_playUntil * 1000);
                return;
            }
        }

        refreshTimeDisplay();
        refreshButtonState();
        emit stateChanged();

        double end = m_mediaPlayer->duration();
        double ratio = 1000000;

        if (m_playUntil > 0 && m_mediaPlayer->duration() > 0)
        {
            ratio = m_playUntil * 1000 * 1000000 / m_mediaPlayer->duration();
            end = m_playUntil * 1000.0;
        }

        double endValue =  m_mediaPlayer->position() * 1.0 / end;

        ui->slideSeek->blockSignals(true);
        ui->slideSeek->setValue(qRound(endValue * ratio));
        ui->slideSeek->blockSignals(false);
    });

    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, [this] (QMediaPlayer::MediaStatus status) {
        if (status != QMediaPlayer::InvalidMedia && status != QMediaPlayer::LoadingMedia && status != QMediaPlayer::StalledMedia)
        {
            m_mediaPlayer->setPosition(m_beginSec * 1000);

            update_limits();
        }
    });

    connect(ui->slideSeek, SIGNAL(valueChanged(int)), SLOT(seekSliderChanged()));
    connect(ui->btnPlayPause, SIGNAL(clicked()), SLOT(togglePlayPause()));
    connect(ui->btnBack, SIGNAL(clicked()), SLOT(seekBackward()));
    connect(ui->btnForward, SIGNAL(clicked()), SLOT(seekForward()));
    connect(ui->btnReset, SIGNAL(clicked()), SLOT(resetPosition()));

    setMinimumHeight(MINIMUM_HEIGHT);

    load_volume();
}

MediaPlayerWidget::~MediaPlayerWidget()
{
    save_volume();
    delete ui;
}

bool MediaPlayerWidget::ok() const
{

    return true;
}

double MediaPlayerWidget::duration() const
{
    return m_mediaPlayer->duration() / 1000;
}

double MediaPlayerWidget::position() const
{
    return m_mediaPlayer->position() / 1000;
}

// public slots

void MediaPlayerWidget::load(const QString &url, qint64 begin, qint64 end)
{
    m_file = url;

    m_beginSec = begin;
    m_playUntil = end;

    update_limits();

    m_mediaPlayer->setSource(QUrl::fromLocalFile(url));

    ui->slideVolume->setValue(m_volume);
    m_audioOutput->setVolume(m_volume);
}

void MediaPlayerWidget::reload()
{
    load(m_file, m_beginSec, m_playUntil);
}

void MediaPlayerWidget::play()
{
    m_mediaPlayer->play();
    ui->btnPlayPause->setFocus();
    refreshButtonState();
}

void MediaPlayerWidget::playRange(int begin_sec, int end_sec)
{
    if (m_mediaPlayer->playbackState() != QMediaPlayer::PlayingState)
    {
        reload();
    }

    m_beginSec = begin_sec;

    if (end_sec > 0)
    {
        m_playUntil = end_sec;
    }
    else
    {
        m_playUntil = -1;
    }

    m_mediaPlayer->setPosition(m_beginSec * 1000);

    //update_limits();
    play();
}

void MediaPlayerWidget::pause()
{
    m_mediaPlayer->pause();
    ui->btnPlayPause->setFocus();
    refreshButtonState();
}

void MediaPlayerWidget::seek(qint64 sec)
{
    m_mediaPlayer->setPosition(sec * 1000);
}

void MediaPlayerWidget::seek_and_pause(qint64 sec)
{
    qDebug() << "Using seek and pause";

    m_mediaPlayer->setPosition(sec * 1000);
    m_playUntil = sec;
}

void MediaPlayerWidget::togglePlayPause()
{
    switch (m_mediaPlayer->playbackState()) {
    case QMediaPlayer::StoppedState:
        reload();

        if (ui->slideSeek->value() < ui->slideSeek->maximum()) { // user seeks
            m_mediaPlayer->setPosition(ui->slideSeek->value());
        } else { // otherwise rewind to begin
            ui->slideSeek->setValue(0);
        }

        m_mediaPlayer->pause();
        break;
    case QMediaPlayer::PlayingState:
        pause();
        break;
    case QMediaPlayer::PausedState:
        play();
        break;
    }
}

// events

void MediaPlayerWidget::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() != 0) { // is vertical
        if (event->angleDelta().y() / 8 >= 0) {
            seekForward();
        } else {
            seekBackward();
        }
    }
}

void MediaPlayerWidget::mousePressEvent(QMouseEvent */*event*/)
{
    togglePlayPause();
}

// private slots

void MediaPlayerWidget::refreshTimeDisplay()
{
    qint64 duration, position, remaining;

    duration = m_playUntil == -1 ?
                m_mediaPlayer->duration() - (m_beginSec * 1000)
              : (m_playUntil - m_beginSec) * 1000;
    position = m_mediaPlayer->position() - (m_beginSec * 1000);

    if (position < 0)
    {
        position = 0;
    }

    remaining = duration - position;

    ui->lblPosition->setText(
        QString::fromLatin1("%1 / %2").arg(sec2hms(position)).arg(sec2hms(duration)));

    ui->lblRemaining->setText(QString::fromLatin1("-%1").arg(sec2hms(remaining)));
}

void MediaPlayerWidget::refreshButtonState()
{
    QString button_icon = QString::fromLatin1(m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState
                                                  ? ":/actions/icons/pause.svg"
                                                  : ":/actions/icons/play.svg"
                                              );

    ui->btnPlayPause->setIcon(QIcon(button_icon));
}

void MediaPlayerWidget::seekSliderChanged()
{
    refreshTimeDisplay();
    refreshButtonState();
    emit stateChanged();

    double ratio = 1000000;

    if (m_playUntil > 0 && m_mediaPlayer->duration() > 0)
    {
        ratio = m_playUntil * 1000 * 1000000 / m_mediaPlayer->duration();
    }

    double end = m_mediaPlayer->duration();
    double endValue = ui->slideSeek->value() / ratio;

    if (m_playUntil > 0)
    {
        end = m_playUntil * 1000.0;
    }

    m_mediaPlayer->blockSignals(true);
    m_mediaPlayer->setPosition(qRound(endValue * end));
    m_mediaPlayer->blockSignals(false);
}

void MediaPlayerWidget::seekBackward()
{
    m_mediaPlayer->setPosition(m_mediaPlayer->position() - 3000);
}

void MediaPlayerWidget::seekForward()
{
    m_mediaPlayer->setPosition(m_mediaPlayer->position() + 3000);
}

void MediaPlayerWidget::resetPosition()
{
    m_mediaPlayer->setPosition(m_beginSec);
}

void MediaPlayerWidget::load_volume()
{
    QSettings settings;
    m_volume = DEFAULT_VOLUME;
    if (settings.contains(VOLUME_SETTING_KEY))
        m_volume = settings.value(VOLUME_SETTING_KEY).toInt();
    if (m_volume < 0)
        m_volume = 0;
    if (m_volume > MAX_VOLUME)
        m_volume = MAX_VOLUME;
}

void MediaPlayerWidget::save_volume()
{
    m_volume = ui->slideVolume->value();
    QSettings().setValue(VOLUME_SETTING_KEY, m_volume);
}

void MediaPlayerWidget::update_limits()
{
    if (m_mediaPlayer->duration() > 0)
    {
        ui->slideSeek->setMinimum(static_cast<int> (m_beginSec * 1000 * 1.0 / m_mediaPlayer->duration() * 1000000));

        if (m_playUntil > 0)
        {
            ui->slideSeek->setMaximum(static_cast<int> (m_playUntil * 1000 * 1.0 / m_mediaPlayer->duration() * 1000000));
        }
        else
        {
            ui->slideSeek->setMaximum(1000000);
        }

        refreshTimeDisplay();
        refreshButtonState();
        emit stateChanged();
    }
}

void MediaPlayerWidget::on_slideVolume_valueChanged(int value)
{
    m_volume = value;

    qreal linearVolume = QAudio::convertVolume(value / qreal(100.0),
                                               QAudio::LogarithmicVolumeScale,
                                               QAudio::LinearVolumeScale);

    m_audioOutput->setVolume(qRound(linearVolume * 100));
}
