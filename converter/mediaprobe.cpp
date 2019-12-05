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

#include "mediaprobe.h"
#include "exepath.h"
#include <QProcess>
#include <QStringList>
#include <QRegularExpression>

#ifdef OPERATION_TIMEOUT
#   define TIMEOUT OPERATION_TIMEOUT
#else
#   define TIMEOUT 3000
#endif
#define SECONDS_PER_HOUR 3600
#define SECONDS_PER_MINUTE 60

namespace {

namespace patterns {

// META
const char meta[]
    = "Duration: ([0-9]+):([0-9]+):([0-9]+\\.[0-9]*)(, start: ([0-9]+\\.[0-9]*))?, bitrate: ([0-9]+) kb/s";
const char META_HOUR_INDEX = 1;        // matched type: integer
const char META_MINUTE_INDEX = 2;      // matched type: integer
const char META_SECOND_INDEX = 3;      // matched type: double
const char META_START_INDEX = 5;       // matched type: double
const char META_BITRATE_INDEX = 6;     // matched type: integer

// AUDIO
const char audio[]
    = "Stream #([0-9]+).([0-9]+).*: Audio:\\s*([^,]*),\\s*([0-9]+)\\s*Hz,\\s*([^,]*),\\s*([^,]*),\\s*([0-9]+)\\s*kb/s";
//const int AUDIO_STREAM_INDEX = 2;      // matched type: integer
const int AUDIO_CODEC_INDEX = 3;       // matched type: string
const int AUDIO_SAMPLERATE_INDEX = 4;  // matched type: integer
const int AUDIO_CHANNELS_INDEX = 5;    // matched type: string
const int AUDIO_BITRATE_INDEX = 7;     // matched type: integer

const char audio_check[] = "^\\s*Stream .* Audio";

// VIDEO
const char video[]
= "Stream #([0-9]+).([0-9]+).*: Video:\\s*([^,]*),\\s*([^,]*),\\s*([0-9]+)x([0-9]+)[^,]*"
        ",\\s*([0-9]+)[^,]*,\\s*([0-9]+(\\.[0-9]*)?)";
const int VIDEO_STREAM_INDEX = 2;      // matched type: integer
const int VIDEO_CODEC_INDEX = 3;       // matched type: string
const int VIDEO_WIDTH_INDEX = 5;       // matched type: integer
const int VIDEO_HEIGHT_INDEX = 6;      // matched type: integer
const int VIDEO_BITRATE_INDEX = 7;     // matched type: integer
const int VIDEO_FRAMERATE_INDEX = 8;   // matched type: double

const char video_check[] = "^\\s*Stream .* Video";

// SUBTITLE
const char subtitle[]
    = "Stream [^(]*\\(([^:]*)\\): Subtitle:";

} // namespace patterns

} // namespace

/* Media Information Structures
   The parse() method in the structures should return true if it accepts the input,
   or return false otherwise.
*/

// stores information of the whole file
struct MetaInformation
{
    double duration; ///< duration in seconds
    double start; ///< start time in seconds
    int bitrate; ///< bitrate in kb/s
    bool __dummy_padding[4];
    QRegExp pattern;

    MetaInformation() : pattern(patterns::meta) { clear(); }

    void clear() { duration = 0; start = 0; bitrate = 0; }

    bool parse(const QString& line)
    {
        int index = pattern.indexIn(line);
        if (index != -1) {
            int hour = pattern.cap(patterns::META_HOUR_INDEX).toInt();
            int minute = pattern.cap(patterns::META_MINUTE_INDEX).toInt();
            double second = pattern.cap(patterns::META_SECOND_INDEX).toDouble();
            duration = hour * SECONDS_PER_HOUR + minute * SECONDS_PER_MINUTE + second;
            start = pattern.cap(patterns::META_START_INDEX).toDouble();
            bitrate = pattern.cap(patterns::META_BITRATE_INDEX).toInt();
            return true;
        }
        return false;
    }
};

// stores information of the audio stream
struct AudioInformation
{
    bool has_audio;
    bool __dummy_padding[3];
    int sample_rate; ///< sample rate in kb/s
    int bitrate; ///< bitrate in kb/s
    int channels; ///< number of channels
    QString codec;
    QRegExp pattern;
    QRegExp pattern_check;

    AudioInformation() : pattern(patterns::audio)
      , pattern_check(patterns::audio_check) { clear(); }

    void clear()
    {
        has_audio = false;
        sample_rate = 0;
        bitrate = 0;
        channels = 2; // default 2 channels
        codec.clear();
    }

    bool parse(const QString& line)
    {
        int index = pattern.indexIn(line);
        if (index != -1) {
            has_audio = true;
            codec = pattern.cap(patterns::AUDIO_CODEC_INDEX);
            sample_rate = pattern.cap(patterns::AUDIO_SAMPLERATE_INDEX).toInt();
            bitrate = pattern.cap(patterns::AUDIO_BITRATE_INDEX).toInt();

            // extract number of channels
            QString channels_field = pattern.cap(patterns::AUDIO_CHANNELS_INDEX);
            QRegExp channels_pattern("([0-9]+)\\s+channel");
            if (channels_pattern.indexIn(channels_field) != -1) {
                channels = channels_pattern.cap(1).toInt();
            } else if (channels_field.indexOf("stereo") != -1) {
                channels = 2;
            } else if (channels_field.indexOf("mono") != -1) {
                channels = 1;
            }

            return true;
        }
        // audio existence must be correct
        if (pattern_check.indexIn(line) != -1) {
            has_audio = true;
            codec = "unknown";
        }
        return false;
    }
};

// stores information of video streams
struct VideoInformation
{
    bool has_video;
    bool __dummy_padding[3];
    int stream_index;
    int width;
    int height;
    int bitrate; ///< bitrate in kb/s
    bool __dummy_padding2[4];
    double frame_rate; ///< frame rate in fps
    QString codec;
    QString format;
    QRegExp pattern;
    QRegExp pattern_check;

    VideoInformation() : pattern(patterns::video)
      , pattern_check(patterns::video_check) { clear(); }

    void clear()
    {
        has_video = false;
        stream_index = 0;
        width = height = 0;
        bitrate = 0;
        frame_rate = 0;
        codec.clear();
        format.clear();
    }

    bool parse(const QString& line)
    {
        int index = pattern.indexIn(line);
        if (index != -1) {
            has_video = true;
            stream_index = pattern.cap(patterns::VIDEO_STREAM_INDEX).toInt();
            width = pattern.cap(patterns::VIDEO_WIDTH_INDEX).toInt();
            height = pattern.cap(patterns::VIDEO_HEIGHT_INDEX).toInt();
            bitrate = pattern.cap(patterns::VIDEO_BITRATE_INDEX).toInt();
            frame_rate = pattern.cap(patterns::VIDEO_FRAMERATE_INDEX).toDouble();
            codec = pattern.cap(patterns::VIDEO_CODEC_INDEX);
            return true;
        }
        // video existence must be correct
        if (pattern_check.indexIn(line) != -1) {
            has_video = true;
            codec = "unknown";
        }
        return false;
    }
};

// stores information of subtitle streams
struct SubtitleInformation
{
    bool has_subtitle;
    bool __dummy_padding[7];
    QRegExp pattern;

    SubtitleInformation() : pattern(patterns::subtitle) { }

    void clear() { has_subtitle = false; }
    bool parse(const QString& line)
    {
        int index = pattern.indexIn(line);
        if (index != -1) {
            has_subtitle = true;
        }
        return false; // reject all inputs
    }
};

/* MediaProbe::Private Structure */

struct MediaProbe::Private
{
    MetaInformation metainfo;
    AudioInformation audioinfo;
    VideoInformation videoinfo;
    SubtitleInformation subtitleinfo;
    QProcess ffprobe_proc;
    int exitcode;

    bool __dummy_padding[4];

    void clear();
    void parse_line(const QString& line);
};

// clear stored data
void MediaProbe::Private::clear()
{
    metainfo.clear();
    audioinfo.clear();
    videoinfo.clear();
    subtitleinfo.clear();
    exitcode = 0;
}

// parse process output
void MediaProbe::Private::parse_line(const QString &line)
{
    if (metainfo.parse(line))
        return;
    if (audioinfo.parse(line))
        return;
    if (videoinfo.parse(line))
        return;
    if (subtitleinfo.parse(line))
        return;
}

/* MediaProbe Class */

// constructor
MediaProbe::MediaProbe(QObject *parent) :
    QObject(parent), p(new Private)
{
    connect(&p->ffprobe_proc, SIGNAL(finished(int))
            , this, SLOT(m_proc_finished(int)));
}

MediaProbe::~MediaProbe()
{
    stop();
    delete p;
}

bool MediaProbe::available()
{
    MediaProbe probe;

    // Try to start the probe to see if the probing program is available.
    if (!probe.start(""))
        return false;

    return true;
}

// Start ffprobe process.
bool MediaProbe::start(const QString& filename)
{
    if (p->ffprobe_proc.state() == QProcess::NotRunning) {
        QStringList list;

        //list.push_back("-show_format");
        //list.push_back("-convert_tags");
        list.push_back(filename);

        p->ffprobe_proc.setReadChannel(QProcess::StandardError);
        p->ffprobe_proc.start(ExePath::getPath("ffprobe"), list);
        return p->ffprobe_proc.waitForStarted(TIMEOUT);
    }
    return false;
}

bool MediaProbe::start(const char *filename)
{
    return start(QString(filename));
}

bool MediaProbe::run(const QString &filename, int timeout)
{
    return start(filename) && wait(timeout) && !error();
}

// Wait for the process to finish.
bool MediaProbe::wait(int msecs)
{
    if (p->ffprobe_proc.state() == QProcess::Running) {
        return p->ffprobe_proc.waitForFinished(msecs);
    }
    return false;
}

// Kill the process and block until it terminates.
void MediaProbe::stop()
{
    if (p->ffprobe_proc.state() == QProcess::Running) {
        p->ffprobe_proc.kill();
        p->ffprobe_proc.waitForFinished(-1); // wait indefinitely
    }
}

bool MediaProbe::error() const
{
    return p->exitcode != 0;
}

// Media Information

int MediaProbe::hours() const
{
    return int(p->metainfo.duration) / 3600;
}

int MediaProbe::minutes() const
{
    return (int(p->metainfo.duration) % 3600) / 60;
}

double MediaProbe::seconds() const
{
    return p->metainfo.duration - hours()*3600 - minutes()*60;
}

double MediaProbe::mediaDuration() const
{
    return p->metainfo.duration;
}

int MediaProbe::mediaBitRate() const
{
    return p->metainfo.bitrate;
}

// Audio Information

bool MediaProbe::hasAudio() const
{
    return p->audioinfo.has_audio;
}

int MediaProbe::audioSampleRate() const
{
    return p->audioinfo.sample_rate;
}

int MediaProbe::audioBitRate() const
{
    return p->audioinfo.bitrate;
}

int MediaProbe::audioChannels() const
{
    return p->audioinfo.channels;
}

const QString& MediaProbe::audioCodec() const
{
    return p->audioinfo.codec;
}

// Video Information

bool MediaProbe::hasVideo() const
{
    return p->videoinfo.has_video;
}

int MediaProbe::videoStreamIndex() const
{
    return p->videoinfo.stream_index;
}

int MediaProbe::videoWidth() const
{
    return p->videoinfo.width;
}

int MediaProbe::videoHeight() const
{
    return p->videoinfo.height;
}

int MediaProbe::videoBitRate() const
{
    return p->videoinfo.bitrate;
}

double MediaProbe::videoFrameRate() const
{
    return p->videoinfo.frame_rate;
}

const QString& MediaProbe::videoCodec() const
{
    return p->videoinfo.codec;
}

// Subtitle Information

bool MediaProbe::hasSubtitle() const
{
    return p->subtitleinfo.has_subtitle;
}

// Private Slots

void MediaProbe::m_proc_finished(int exitcode)
{
    p->exitcode = exitcode;

    if (exitcode == 0) { // If the process finished normally, parse its output.
        p->clear();
        while (p->ffprobe_proc.canReadLine()) {
            QString line = QString(p->ffprobe_proc.readLine()).trimmed();
            p->parse_line(line);
        }
    }

    emit process_finished();
}
