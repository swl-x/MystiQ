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
#include "mediaplayerwidget.h"
#include "ui_mediaplayerwidget.h"
#include "myqmpwidget.h"
#include "services/constants.h"

#define DEFAULT_VOLUME Constants::getInteger("MediaPlayer/DefaultVolume")
#define SLIDER_STYLESHEET Constants::getString("MediaPlayer/SliderStyle")
#define MINIMUM_HEIGHT 210

#define MAX_VOLUME 100
#define VOLUME_SETTING_KEY "mediaplayer/volume"

namespace {
QString sec2hms(int seconds)
{
    int h = seconds / 3600;
    int m = (seconds % 3600) / 60;
    int s = (seconds % 60);
    QString result;
    result.sprintf("%02d:%02d:%02d", h, m, s);
    return result;
}
}

MediaPlayerWidget::MediaPlayerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaPlayerWidget),
    m_playUntil(-1)
{
    ui->setupUi(this);
    ui->slideVolume->setRange(0, MAX_VOLUME);
    mplayer = new MyQMPwidget(this);
    mplayer->start();
    mplayer->setAcceptDrops(false);
    mplayer->setSeekSlider(ui->slideSeek);
    mplayer->setVolumeSlider(ui->slideVolume);
    ui->layoutPlayer->addWidget(mplayer);
    ui->slideSeek->setStyleSheet(SLIDER_STYLESHEET);

    connect(mplayer, SIGNAL(stateChanged(int)), SLOT(playerStateChanged()));
    connect(ui->slideSeek, SIGNAL(valueChanged(int)), SLOT(seekSliderChanged()));
    connect(ui->btnPlayPause, SIGNAL(clicked()), SLOT(togglePlayPause()));
    connect(ui->btnBack, SIGNAL(clicked()), SLOT(seekBack()));
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
    MyQMPwidget::MediaInfo info = mplayer->mediaInfo();
    return info.ok;
}

double MediaPlayerWidget::duration() const
{
    MyQMPwidget::MediaInfo info = mplayer->mediaInfo();
    if (info.ok)
        return info.length;
    else
        return 0;
}

double MediaPlayerWidget::position() const
{
    return mplayer->tell();
}

// public slots

void MediaPlayerWidget::load(const QString &url)
{
    m_file = url;
    mplayer->load(url);
    ui->slideVolume->setValue(m_volume);
    mplayer->pause();
}

void MediaPlayerWidget::reload()
{
    load(m_file);
}

void MediaPlayerWidget::play()
{
    mplayer->play();
    ui->btnPlayPause->setFocus();
    refreshButtonState();
}

void MediaPlayerWidget::playRange(int begin_sec, int end_sec)
{
    if (mplayer->state() == MyQMPwidget::IdleState)
        reload();
    if (begin_sec >= 0)
        seek(begin_sec);
    else
        seek(0);
    if (end_sec >= 0)
        m_playUntil = end_sec;
    else
        m_playUntil = -1;
    play();
}

void MediaPlayerWidget::pause()
{
    mplayer->pause();
    ui->btnPlayPause->setFocus();
    refreshButtonState();
}

void MediaPlayerWidget::seek(int sec)
{
    mplayer->seek(sec);
}

void MediaPlayerWidget::seek_and_pause(int sec)
{
    // seek() is asynchrous. If call mplayer->seek(); mplayer->pause(); in a row,
    // the latter command will be ignored because seek() is not done yet.
    mplayer->seek(sec);
    m_playUntil = sec; // seek() is asynchrouse
}

void MediaPlayerWidget::togglePlayPause()
{
    switch (mplayer->state()) {
    case MyQMPwidget::IdleState:
        reload();
        if (ui->slideSeek->value() < ui->slideSeek->maximum()) { // user seeks
            mplayer->seek(ui->slideSeek->value());
        } else { // otherwise rewind to begin
            ui->slideSeek->setValue(0);
        }
        pause();
        break;
    case MyQMPwidget::PlayingState:
        pause();
        break;
    case MyQMPwidget::PausedState:
        play();
        break;
    default:
        break;
    }
}

// events

void MediaPlayerWidget::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8; // delta is in eighths of a degree
    if (event->orientation() == Qt::Vertical) {
        if (numDegrees >= 0) {
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
    MyQMPwidget::MediaInfo info = mplayer->mediaInfo();
    int duration, position, remaining;
    if (info.ok) {
        duration = info.length;
        position = mplayer->tell();
        if (position < 0) position = 0;
        remaining = duration - position;
    } else {
        duration = position = remaining = 0;
    }
    ui->lblPosition->setText(QString("%1 / %2")
                             .arg(sec2hms(position))
                             .arg(sec2hms(duration)));
    ui->lblRemaining->setText(QString("-%1").arg(sec2hms(remaining)));
}

void MediaPlayerWidget::refreshButtonState()
{
    QString button_icon = ":/actions/icons/play.svg";
    switch (mplayer->state()) {
    case MyQMPwidget::PlayingState:
        button_icon = ":/actions/icons/pause.svg"; break;
    default:
        break;
    }
    ui->btnPlayPause->setIcon(QIcon(button_icon));
}

void MediaPlayerWidget::playerStateChanged()
{
    refreshTimeDisplay();
    refreshButtonState();
    emit stateChanged();
}

void MediaPlayerWidget::seekSliderChanged()
{
    refreshTimeDisplay();
    if (m_playUntil >= 0 && position() >= m_playUntil) {
        // reaches temporary pause position
        m_playUntil = -1;
        pause();
    }
}

void MediaPlayerWidget::seekBackward()
{
    mplayer->seek(-3, MyQMPwidget::RelativeSeek);
}

void MediaPlayerWidget::seekForward()
{
    mplayer->seek(3, MyQMPwidget::RelativeSeek);
}

void MediaPlayerWidget::resetPosition()
{
    mplayer->seek(0);
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
