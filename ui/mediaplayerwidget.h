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

#ifndef MEDIAPLAYERWIDGET_H
#define MEDIAPLAYERWIDGET_H

#include <QAudioOutput>
#include <QWidget>

class QVideoWidget;
class QMediaPlayer;

namespace Ui {
class MediaPlayerWidget;
}

class MyQMPwidget;

class MediaPlayerWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit MediaPlayerWidget(QWidget *parent = nullptr);
    ~MediaPlayerWidget();

    bool ok() const;
    double duration() const;
    double position() const;

signals:
    void stateChanged();

public slots:

    /**
     * @brief Load media for playing from @a url.
     *
     * @param url path of the file
     */
    void load(const QString& url, qint64 begin, qint64 end);

    /**
     * @brief Reload previously loaded file.
     */
    void reload();

    /**
     * @brief Start playing the loaded file.
     */
    void play();

    /**
     * @brief Play the media file from @a begin_sec to @a end_sec.
     * The playback will pause as soon as @c position reaches @a end_sec.
     * The range of the slider is not modified, as opposed to @c setRangeLimit().
     *
     * @param begin_sec the beginning point in seconds. -1 means play from beginning
     * @param end_sec the end point in seconds. -1 means play until end
     */
    void playRange(int begin_sec, int end_sec);

    /**
     * @brief Seek to @a sec and start playing.
     *
     * @param sec position in seconds
     */
    void seek(qint64 sec);

    /**
     * @brief Seek to @a sec and pause.
     * @param sec
     */
    void seek_and_pause(qint64 sec);

    /**
     * @brief Pause the media playback.
     */
    void pause();

    /**
     * @brief Toggle between Pause and Play.
     */
    void togglePlayPause();

protected:
    void wheelEvent(QWheelEvent *);
    void mousePressEvent(QMouseEvent *);

private slots:
    void refreshTimeDisplay();
    void refreshButtonState();
    void seekSliderChanged();
    void seekBackward();
    void seekForward();
    void resetPosition();
    
    void on_slideVolume_valueChanged(int value);

private:
    Ui::MediaPlayerWidget *ui;

    QMediaPlayer *m_mediaPlayer;
    QAudioOutput *m_audioOutput;
    QVideoWidget *m_videoView;

    QString m_file;

    // media playback will stop when position() reaches m_playUntil
    qint64 m_beginSec;
    qint64 m_playUntil;
    int m_volume;

    Q_DISABLE_COPY(MediaPlayerWidget)
    void load_volume();
    void save_volume();
    void update_limits();
};

#endif // MEDIAPLAYERWIDGET_H
