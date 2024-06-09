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

#include "audiofilter.h"
#include "conversionparameters.h"
#include "exepath.h"
#include "ffmpeginterface.h"
#include <QProcess>
#include <QSet>
#include <QDebug>

#ifdef OPERATION_TIMEOUT
#   define TIMEOUT OPERATION_TIMEOUT
#else
#   define TIMEOUT 3000
#endif

/*
  SoX provides much more audio-processing options than
  ffmpeg, yet it doesn't accept as many formats as
  ffmpeg does. In this pipeline, format conversion
  is done by ffmpeg and audio processing is done by SoX.

  The audio-filtering pipeline consists of two stages:

  1. Audio Extraction
     command: ffmpeg -i <infile> -vn -f <fmt> -
     input: infile
     output: stdout
     Extract the audio stream from the input file, convert it
     into sox format, and pipe it to stdout.

  2. Audio Filtering
     command: sox -t <fmt> - -t <fmt> - <options>
     input: stdin
     output: stdout
     Process the audio stream using SoX. The output is piped to
     stdout.

*/

AudioFilter::AudioFilter(QObject *parent) :
    QObject(parent),
    m_extractAudioProc(new QProcess(this)),
    m_soxProc(new QProcess(this))
{
    QSet<QString> muxing, demuxing;
    FFmpegInterface::getSupportedMuxingFormats(muxing);
    FFmpegInterface::getSupportedDemuxingFormats(demuxing);
    m_useSoxFormat = muxing.contains(QString::fromLatin1("sox"))
                     && demuxing.contains(QString::fromLatin1("sox"));
}

bool AudioFilter::start(ConversionParameters& params, QProcess *dest)
{
    QStringList ffmpeg_param;
    QStringList sox_param;
    const char *fmt = m_useSoxFormat ? "sox" : "flac";

    if (m_soxProc->state() != QProcess::NotRunning) {
        m_soxProc->kill();
        m_soxProc->waitForFinished(TIMEOUT);
    }

    if (m_extractAudioProc->state() != QProcess::NotRunning) {
        m_extractAudioProc->kill();
        m_extractAudioProc->waitForFinished(TIMEOUT);
    }

    // ffmpeg process settings
    ffmpeg_param << QString::fromLatin1("-i") << params.source << QString::fromLatin1("-vn")
                 << QString::fromLatin1("-f") << QString::fromLatin1(fmt)
                 << QString::fromLatin1("-");
    m_extractAudioProc->setStandardOutputProcess(m_soxProc);

    // sox process settings
    sox_param << QString::fromLatin1("-t") << QString::fromLatin1(fmt) << QString::fromLatin1("-")
              << QString::fromLatin1("-t") << QString::fromLatin1(fmt) << QString::fromLatin1("-");
    if (params.speed_scaling) {
        sox_param << QString::fromLatin1("tempo") << QString::number(params.speed_scaling_factor);
    }
    m_soxProc->setStandardOutputProcess(dest);

    qDebug() << "ffmpeg" << ffmpeg_param;
    qDebug() << "sox" << sox_param;
    // start the two processes
    m_extractAudioProc->start(ExePath::getPath(QString::fromLatin1("ffmpeg")), ffmpeg_param);
    m_soxProc->start(ExePath::getPath(QString::fromLatin1("sox")), sox_param);

    return m_extractAudioProc->waitForStarted(TIMEOUT)
            && m_soxProc->waitForStarted(TIMEOUT);
}

bool AudioFilter::available()
{
    QProcess sox_process;
    sox_process.start(ExePath::getPath(QString::fromLatin1("sox")), QStringList());
    return sox_process.waitForStarted(TIMEOUT);
}
