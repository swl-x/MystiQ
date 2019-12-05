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

#include "ffmpeginterface.h"
#include "mediaprobe.h"
#include "exepath.h"
#include <QRegExp>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QAtomicInt>
#include <cmath>

// timeout before force-terminating ffmpeg
#ifdef OPERATION_TIMEOUT
#   define TIMEOUT OPERATION_TIMEOUT
#else
#   define TIMEOUT 3000
#endif

namespace {
namespace patterns {
    const char progress[]
        = "size=\\s*([0-9]+)kB\\s+time=\\s*([0-9]+\\.[0-9]+)\\s+bitrate=\\s*([0-9]+\\.[0-9]+)kbits/s";
    enum Progress_1_Fields {
        PROG_1_TIME = 2
    };

    const char progress2[] // another possible format where time is represented as hh:mm:ss
        = "size=\\s*([0-9]+)kB\\s+time=\\s*([0-9][0-9]):([0-9][0-9]):([0-9][0-9](\\.[0-9][0-9]?)?)\\s+"
          "bitrate=\\s*([0-9]+\\.[0-9]+)kbits/s";
    enum Progress_2_Fields {
        PROG_2_HR = 2,
        PROG_2_MIN,
        PROG_2_SEC
    };

    const char duration[]
        = "Duration:\\s+([0-9][0-9]):([0-9][0-9]):([0-9][0-9](\\.[0-9][0-9]?)?)";
} // namespace patterns

namespace info {
    QAtomicInt is_encoders_read(false);
    volatile bool ffmpeg_exist = false;
    QString ffmpeg_version;
    QString ffmpeg_codec_info;
    QString ffmpeg_format_info;
    QList<QString> audio_encoders;
    QList<QString> video_encoders;
    QList<QString> subtitle_encoders;
    QList<QString> muxing_formats;
    QList<QString> demuxing_formats;

namespace inner {

    /**
     * @brief Extract encoder information from codec description.
     * @param target Extracted encoder names will be pushed into @a target
     * @param s the codec description string
     * @return number of encoders found
     */
    int find_encoders_in_desc(QStringList& target, const QString& s) {
        const char *keyword_begin = "(encoders:";
        const char *keyword_end = ")";
        int begin = s.indexOf(keyword_begin);
        if (begin < 0)
            return 0; // encoder name not found in description
        begin += strlen(keyword_begin);
        int end = s.indexOf(keyword_end, begin);
        if (end < 0)
            return 0; // error, mission ')'
        int length = end - begin;

        // encoder_str contains encoder names separated by spaces, and
        // may contain leading and trailing spaces.
        QString encoders_str = s.mid(begin, length);

        // split encoder_str into encoder names and skip whitespaces
        QStringList encoders = encoders_str.split(' ', QString::SkipEmptyParts);
        foreach (QString s, encoders) {
            target.push_back(s); // fill codec names into the list
        }

        return encoders.size();
    }

    bool read_ffmpeg_codecs(const char *flag)
    {
        QProcess ffmpeg_process;
        QStringList parameters;
        parameters.push_back(QString(flag));

        ffmpeg_process.setReadChannel(QProcess::StandardOutput);

        //qDebug() << ExePath::getPath("ffmpeg") << parameters.join(" ");
        ffmpeg_process.start(ExePath::getPath("ffmpeg"), parameters);

        // Wait until ffmpeg has started.
        if (!ffmpeg_process.waitForStarted(TIMEOUT)) {
            return false;
        }

        // Wait until ffmpeg has finished.
        if (!ffmpeg_process.waitForFinished(TIMEOUT)) {
            return false;
        }

        if (ffmpeg_process.exitCode() != 0) {
            return false; // error
        }

        // Find all available encoders
        QRegExp pattern("[ D]E([ VAS])...\\s+([^ ]+)\\s*(.*)$");
        QStringList encoder_list; // temporary storage of encoder names
        const int AV_INDEX = 1;
        const int CODEC_NAME_INDEX = 2;
        const int CODEC_DESC = 3;

        ffmpeg_codec_info.clear();
        while (ffmpeg_process.canReadLine()) {
            QString line(ffmpeg_process.readLine());
            ffmpeg_codec_info.append(line);

            if (pattern.indexIn(line) != -1) {
                QString av = pattern.cap(AV_INDEX);
                QString codec = pattern.cap(CODEC_NAME_INDEX);
                QString desc = pattern.cap(CODEC_DESC);

                // extract codec names
                encoder_list.clear();
                if (!find_encoders_in_desc(encoder_list, desc))
                    encoder_list.push_back(codec);

                foreach (QString codec_name, encoder_list) {
                    if (av == "A") { // audio encoder
                        //qDebug() << "Audio Codec: " + codec_name;
                        audio_encoders.push_back(codec_name);
                    } else if (av == "V") { // video encoder
                        //qDebug() << "Video Codec: " + codec_name;
                        video_encoders.push_back(codec_name);
                    } else if (av == "S") { // subtitle encoder
                        //qDebug() << "Subtitle Codec: " + codec_name;
                        subtitle_encoders.push_back(codec_name);
                    }
                }
            }
        }

        return true;
    }

    bool read_ffmpeg_version()
    {
        QProcess ffmpeg_process;
        QStringList parameters;
        parameters.push_back(QString("-version"));

        //qDebug() << ExePath::getPath("ffmpeg") << parameters.join(" ");
        ffmpeg_process.start(ExePath::getPath("ffmpeg"), parameters);

        ffmpeg_process.waitForStarted(TIMEOUT);
        ffmpeg_process.waitForFinished(TIMEOUT);
        ffmpeg_version = QString(ffmpeg_process.readAll());

        return true;
    }

    bool read_ffmpeg_formats()
    {
        QProcess ffmpeg_process;
        QStringList parameters;
        parameters << "-formats";

        ffmpeg_process.start(ExePath::getPath("ffmpeg"), parameters);

        ffmpeg_process.waitForStarted(TIMEOUT);
        ffmpeg_process.waitForFinished(TIMEOUT);
        if (ffmpeg_process.exitCode() != 0)
            return false;

        muxing_formats.clear();
        demuxing_formats.clear();
        ffmpeg_format_info.clear();

        QRegExp pattern("^ ([ D])([ E]) ([^ ]+)\\s+(.*)$");
        const int INDEX_DEMUX = 1;
        const int INDEX_MUX = 2;
        const int INDEX_NAME = 3;
        //const int INDEX_DETAIL = 4;

        while (ffmpeg_process.canReadLine()) {
            QString line(ffmpeg_process.readLine());
            ffmpeg_format_info.append(line);
            if (pattern.indexIn(line) != -1) {
                QString name = pattern.cap(INDEX_NAME);
                if (pattern.cap(INDEX_DEMUX) == "D")
                    demuxing_formats.append(name);
                if (pattern.cap(INDEX_MUX) == "E")
                    muxing_formats.append(name);
            }
        }

        return true;
    }

} // namespace inner

    /* Read FFmpeg information.
     *  (1) Check available encoder from "ffmpeg -codecs" and "ffmpeg -formats".
     *  (2) Read ffmpeg version information by "ffmpeg -version" command.
     * Once the information is correctly read, it never
     * execute ffmpeg to acquire the information again.
    */
    void read_ffmpeg_info()
    {
        if (!is_encoders_read.testAndSetAcquire(false, true))
            return; // Skip the operation if the information was already read.

        qDebug() << "Read FFmpeg Information";

        /* Older versions of ffmpeg has no "-codecs" flag and report all
         * supported codecs by "-formats". Recent versions report codecs
         * by "-codecs" flag, so we check "-codecs" first. If ffmpeg
         * returns an error, retry with flag "-formats".
         */
        if (!inner::read_ffmpeg_codecs("-codecs") && !inner::read_ffmpeg_codecs("-formats")) {
            is_encoders_read = false; // allow retry when failed
            return;
        }

        if (!inner::read_ffmpeg_version())
            return;

        if (!inner::read_ffmpeg_formats())
            return;

        ffmpeg_exist = true;
    }

} // namespace info

    // extract error message from the line
    QString extract_errmsg(const QString& line)
    {
        QRegExp pattern("^[^:]*:(.*)$");
        const int INDEX_MESSAGE = 1;
        if (pattern.indexIn(line) != -1)
            return pattern.cap(INDEX_MESSAGE).trimmed();
        else
            return "";
    }

    // scale sec with speed_factor if scale == true
    double scale_time(unsigned int sec, bool scale, double speed_factor)
    {
        if (!sec)
            return 0;
        else if (scale)
            return sec / speed_factor; // (speed *= 2) means (time /= 2)
        else
            return sec;
    }

} // anonymous namespace

struct FFmpegInterface::Private
{
    double duration;
    double progress;
    QString stringBuffer;
    QRegExp progress_pattern;
    QRegExp progress_pattern_2;
    QRegExp duration_pattern;

    bool encoders_read;
    bool __dummy_padding[7]; // Avoid internally padding of struct on RAM
    QList<QString> audio_encoders;
    QList<QString> video_encoders;
    QList<QString> subtitle_encoders;

    QString errmsg;

    Private() : duration(0), progress(0)
      , progress_pattern(patterns::progress)
      , progress_pattern_2(patterns::progress2)
      , duration_pattern(patterns::duration)
      , encoders_read(false) { }

    bool check_progress(const QString&);
    QStringList getOptionList(const ConversionParameters&, bool*, bool*);
};

/*! Check whether the output line is a progress line.
    If it is, update p->progress and return true.
    Otherwise, keep the value of p->progress and return false.
*/
bool FFmpegInterface::Private::check_progress(const QString& line)
{
    QRegExp& pattern = progress_pattern;
    int index = pattern.indexIn(line);
    if (index != -1) {
        const double t = pattern.cap(patterns::PROG_1_TIME).toDouble();

        // calculate progress
        progress = (t / duration) * 100;

        return true;
    } else { // try another pattern
        QRegExp& alternate_pattern = progress_pattern_2;
        if (alternate_pattern.indexIn(line) != -1) {
            const int hour = alternate_pattern.cap(patterns::PROG_2_HR).toInt();
            const int min = alternate_pattern.cap(patterns::PROG_2_MIN).toInt();
            const double sec = alternate_pattern.cap(patterns::PROG_2_SEC).toDouble();
            const double t = hour*3600 + min*60 + sec;

            progress = (t / duration) * 100;

            return true;
        }
    }
    errmsg = line; // save the last output line
    return false;
}

/**
 * Convert a ConversionParameters object into ffmpeg command-line option list.
 * @param o the parameter object
 * @return a QStringList containing command-line options
 * @note This function is time-consuming because it calls ffprobe to obtain
 *       media information.
 */
QStringList FFmpegInterface::Private::getOptionList(const ConversionParameters &o
                                                    , bool *needs_audio_filter
                                                    , bool *success)
{
    MediaProbe probe;
    bool bNeedsAudioFilter;

    if (!probe.run(o.source, TIMEOUT)) {
        if (success)
            *success = false;
        return QStringList();
    }

    bNeedsAudioFilter = o.speed_scaling && !o.disable_audio && probe.hasAudio();

    QStringList list;
    QString source = o.source;
    int last_point_source = source.lastIndexOf(".");
    //Begins Subtitle files declaration
    QString subtitulosrt, subtitulossa;
    if (last_point_source==-1){
        subtitulosrt = source+".srt";
        subtitulossa = source+".ssa";
    }
    else{
        subtitulosrt = source.replace(last_point_source, 5, ".srt");
        subtitulossa = source.replace(last_point_source, 5, ".ssa");
    }
    QFile subtitulo_srt, subtitulo_ssa;
    subtitulo_srt.setFileName(subtitulosrt);
    subtitulo_ssa.setFileName(subtitulossa);
    //Finishing Subtitle files declaration

    // overwrite if file exists
    list.append("-y");

    if (!bNeedsAudioFilter) {
        /* in this configuration, input is read from file
           arguments: -i <infile>
        */
        list << "-i" << o.source;
    } else {
        /* In this configuration, video (if any) is read from file
           and audio is read from stdin
           if source file contains video:
              arguments: -i <infile> -i - -map 0:<vstreamindex> -map 1
           if source file has no video stream:
              arguments: -i -
        */
        if (probe.hasVideo() && !o.disable_video)
            list << "-i" << o.source << "-i" << "-"
                 << "-map" << QString("0:%1").arg(probe.videoStreamIndex())
                 << "-map" << "1";
        else{
            list << "-i" << "-";
        }
    }
    // begining insert subtitle
       if (subtitulo_srt.exists() && o.insert_subtitle) {
           //TODO make insert subtitle happends
           list << "-vf subtitles='"+subtitulosrt+"':force_style='Fontsize=24':charenc=cp1256";
       }
       else if (!subtitulo_srt.exists() && subtitulo_ssa.exists() && o.insert_subtitle) {
           //TODO make insert subtitle happends
           list << "-vf subtitles='"+subtitulossa+"':force_style='Fontsize=24':charenc=cp1256";
       }
     // finishing insert subtitle

    // enable experimental codecs by default
   // list << "-strict" << "experimental";

    /* ==== Additional Options ==== */
    if (!o.ffmpeg_options.isEmpty()) {
        QList<QString> additional_options =
                o.ffmpeg_options.split(" ", QString::SkipEmptyParts);
        foreach (QString opt, additional_options)
            list.append(opt);
    }

    if (o.threads >= 2) {
        list.append("-threads");
        list.append(QString::number(o.threads));
    }

    /* ==== Audio/Video Options ==== */

    // Audio Options
    if (o.disable_audio) {
        list.append("-an"); // no audio
    } else if (o.copy_audio) { // copy audio data (no re-encode)
        list.append("-acodec");
        list.append("copy");
    } else { // audio enabled

        // audio bitrate in kb/s
        if (o.audio_bitrate > 0) {
            list.append("-ab");
            list.append(QString("%1k").arg(o.audio_bitrate));
        }

        // audio sample rate in hz
        if (o.audio_sample_rate > 0) {
            list.append("-ar");

            int sample_rate = o.audio_sample_rate;
            if (o.audio_keep_sample_rate
                    && !probe.error()
                    && probe.audioSampleRate() != 0) {
                sample_rate = probe.audioSampleRate();
                qDebug() << "Apply probed sample rate: " + QString::number(sample_rate);
            }

            list.append(QString("%1").arg(sample_rate));
        }

        // audio channels
        if (o.audio_channels > 0) {
            list.append("-ac");
            list.append(QString("%1").arg(o.audio_channels));
        }

        // volume
        // 256 is normal volume
        if (o.audio_volume > 0 && o.audio_volume != 256) {
            list.append("-vol");
            list.append(QString("%1").arg(o.audio_volume));
        }

    }

    // Video Options
    if (o.disable_video || !probe.hasVideo()) {
        list.append("-vn"); // no video
    } else if (o.copy_video) { // copy video data (no re-encode)
        list.append("-vcodec");
        list.append("copy");
    } else { // video enabled

        // same video quality as source
        if (o.video_same_quality) {
            list.append("-sameq");
        }

        // deinterlace
        if (o.video_deinterlace) {
            list.append("-deinterlace");
        }

        // video bitrate
        if (o.video_bitrate > 0) {
            list.append("-b");
            list.append(QString("%1k").arg(o.video_bitrate));
        }

        // video dimensions
        if (o.video_width > 0 && o.video_height > 0) {
            list.append("-s");
            list.append(QString("%1x%2").arg(o.video_width).arg(o.video_height));
        }

        // crop video
        list.append("-filter:v");

        QString crop = QString("crop=%1:%2:%3:%4")
                .arg(o.video_crop_right - o.video_crop_left)
                .arg(o.video_crop_bottom - o.video_crop_top)
                .arg(o.video_crop_left)
                .arg(o.video_crop_top);
        list.append(crop);

        /* -vf "setpts=<1/rate>*PTS": video filter to change video speed
            <1/rate> is the reciprocal of the scaling factor (1.0 is original speed) */
        if (o.speed_scaling)
            list << "-vf" << QString("setpts=%1*PTS").arg(1/o.speed_scaling_factor);
    }

    // Time Options

    /* Scale begin time and end time if speed_scaling is on.
     * scaled_time_begin and scaled_time_end are doubles, so never compare time
     * (ex. time == 0) by them. Compare using o.time_begin and o.time_end instead.
     */
    double scaled_time_begin = scale_time(o.time_begin, o.speed_scaling, o.speed_scaling_factor);
    double scaled_time_end = scale_time(o.time_end, o.speed_scaling, o.speed_scaling_factor);

    /* -ss time_begin
        When used as an output option, ffmpeg decodes but discards input
        until timestamp reaches time_begin */
    if (o.time_begin > 0) {
        list.append("-ss");
        list.append(QString("%1").arg(scaled_time_begin));
    }
    /* -t time_duration
        Stop writing the output after its duration reaches time_duration */
    if (o.time_end > 0) {
        Q_ASSERT(o.time_end >= o.time_begin);
        double scaled_duration = scaled_time_end - scaled_time_begin;
        list.append("-t");
        list.append(QString("%1").arg(scaled_duration));
    }

    // destination file
    list.append(o.destination);

    // record duration
    duration = probe.mediaDuration();
    if (o.speed_scaling)
        duration /= o.speed_scaling_factor;

    if (needs_audio_filter)
        *needs_audio_filter = bNeedsAudioFilter;
    if (success)
        *success = true;
    return list;
}

FFmpegInterface::FFmpegInterface(QObject *parent) :
    ConverterInterface(parent), p(new Private)
{
}

FFmpegInterface::~FFmpegInterface()
{
    delete p;
}

// virtual functions
QString FFmpegInterface::executableName() const
{
    return ExePath::getPath("ffmpeg");
}

void FFmpegInterface::reset()
{
    p->duration = 0;
    p->progress = 0;
    p->stringBuffer.clear();
    p->errmsg.clear();
}

QProcess::ProcessChannel FFmpegInterface::processReadChannel() const
{
    return QProcess::StandardError;
}

bool FFmpegInterface::needsAudioFiltering(const ConversionParameters& param) const
{
    return !param.disable_audio && param.speed_scaling;
}

void FFmpegInterface::fillParameterList(const ConversionParameters &param, QStringList &list
                                        , bool *needs_audio_filter)
{
    bool success; // TODO: return success
    list = p->getOptionList(param, needs_audio_filter, &success);
}

void FFmpegInterface::parseProcessOutput(const QString &data)
{
    //qDebug() << data;

    // split incoming data by [end of line] or [carriage return]
    QStringList lines(data.split(QRegExp("[\r\n]"), QString::KeepEmptyParts));

    if (!p->stringBuffer.isEmpty()) { // prepend buffered data
        lines.front().prepend(p->stringBuffer);
        p->stringBuffer.clear();
    }

    if (!lines.back().isEmpty()) { // buffer incomplete data
        p->stringBuffer = lines.back();
        lines.back().clear();
    }

    QStringList::iterator it = lines.begin();

    for (; it!=lines.end(); ++it) { // parse lines
        QString& line = *it;
        if (line.isEmpty()) continue;
        if (p->check_progress(line)) {
            emit progressRefreshed(p->progress);
            continue;
        }
    }
}

double FFmpegInterface::progress() const
{
    return p->progress;
}

QString FFmpegInterface::errorMessage() const
{
    return extract_errmsg(p->errmsg);
}

bool FFmpegInterface::getAudioEncoders(QList<QString> &target)
{
    info::read_ffmpeg_info();
    if (!info::ffmpeg_exist) return false;

    target = info::audio_encoders;
    return true;
}

bool FFmpegInterface::getAudioEncoders(QSet<QString> &target)
{
    QList<QString> encoder_list;
    if (!getAudioEncoders(encoder_list))
        return false;
    target = QSet<QString>::fromList(encoder_list);
    return true;
}

bool FFmpegInterface::getVideoEncoders(QList<QString> &target)
{
    info::read_ffmpeg_info();
    if (!info::ffmpeg_exist) return false;

    target = info::video_encoders;
    return true;
}

bool FFmpegInterface::getVideoEncoders(QSet<QString> &target)
{
    QList<QString> encoder_list;
    if (!getVideoEncoders(encoder_list))
        return false;
    target = QSet<QString>::fromList(encoder_list);
    return true;
}

bool FFmpegInterface::getSubtitleEncoders(QList<QString> &target)
{
    info::read_ffmpeg_info();
    if (!info::ffmpeg_exist) return false;

    target = info::subtitle_encoders;
    return true;
}

QString FFmpegInterface::getFFmpegVersionInfo()
{
    info::read_ffmpeg_info();
    return info::ffmpeg_version;
}

QString FFmpegInterface::getFFmpegCodecInfo()
{
    info::read_ffmpeg_info();
    return info::ffmpeg_codec_info;
}

QString FFmpegInterface::getFFmpegFormatInfo()
{
    info::read_ffmpeg_info();
    return info::ffmpeg_format_info;
}

bool FFmpegInterface::getSupportedMuxingFormats(QSet<QString> &target)
{
    info::read_ffmpeg_info();
    if (!info::ffmpeg_exist) return false;

    target = QSet<QString>::fromList(info::muxing_formats);
    return true;
}

bool FFmpegInterface::getSupportedDemuxingFormats(QSet<QString> &target)
{
    info::read_ffmpeg_info();
    if (!info::ffmpeg_exist) return false;

    target = QSet<QString>::fromList(info::demuxing_formats);
    return true;
}

bool FFmpegInterface::hasFFmpeg()
{
    info::read_ffmpeg_info();
    return info::ffmpeg_exist;
}

void FFmpegInterface::refreshFFmpegInformation()
{
    info::is_encoders_read = false;
    info::read_ffmpeg_info();
}

bool FFmpegInterface::getSubtitleEncoders(QSet<QString> &target)
{
    QList<QString> encoder_list;
    if (!getSubtitleEncoders(encoder_list))
        return false;
    target = QSet<QString>::fromList(encoder_list);
    return true;
}
