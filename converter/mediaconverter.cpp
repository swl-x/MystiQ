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

#include "mediaconverter.h"
#include "ffmpeginterface.h"
#include "audiofilter.h"
#include "mediaprobe.h"
#include "exepath.h"
#include "services/filepathoperations.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>

#ifdef OPERATION_TIMEOUT
#   define TIMEOUT OPERATION_TIMEOUT
#else
#   define TIMEOUT 3000
#endif

MediaConverter::MediaConverter(QObject *parent) :
    QObject(parent),
    m_pConv(new FFmpegInterface(this)),
    m_pAudioFilter(new AudioFilter(this))
{
    connect(&m_proc, SIGNAL(readyRead())
            , this, SLOT(readProcessOutput()));
    connect(&m_proc, SIGNAL(finished(int,QProcess::ExitStatus))
            , this, SLOT(convertProgressFinished(int,QProcess::ExitStatus)));
    connect(m_pConv, SIGNAL(progressRefreshed(double))
            , this, SLOT(convertProgressRefreshed(double)));
}

MediaConverter::~MediaConverter()
{
    this->stop();
}

// public methods

bool MediaConverter::start(ConversionParameters param)
{
    if (m_proc.state() == QProcess::NotRunning) {
        m_stopped = false;

        emit progressRefreshed(0);

        // Save output filename.
        m_outputFileName = param.destination;

        // Generate temporary output filename.
        m_tmpFileName = FilePathOperations::GenerateTempFileName(m_outputFileName);

        // Output to temporary file.
        param.destination = m_tmpFileName;

        m_pConv->reset();

        QStringList list;
        bool needs_audio_filter;
        m_proc.setReadChannel(m_pConv->processReadChannel());
        m_pConv->fillParameterList(param, list, &needs_audio_filter);

        if (needs_audio_filter) {
            qDebug() << "Audio filter is turned on.";
            m_pAudioFilter->start(param, &m_proc);
        }
        qDebug() << m_pConv->executableName() << list.join(QString::fromLatin1(" "));

        m_proc.start(m_pConv->executableName(), list);
        return m_proc.waitForStarted(TIMEOUT);
    }
    return false;
}

void MediaConverter::stop()
{
    if (m_proc.state() == QProcess::Running) {
        m_stopped = true;
        m_proc.kill();
        m_proc.waitForFinished(-1); // wait indefinitely
    }
}

double MediaConverter::progress()
{
    return m_pConv->progress();
}

/*! Check whether external programs are available
 *   - ffmpeg
 *   - ffprobe
 */
bool MediaConverter::checkExternalPrograms(QString &msg)
{
    //: %1 is a computer program
    QString errmsg = tr(" FFmpeg or FFprobe %1 have not been found in the system. Please consider installing them before running MystiQ");
    // check ffmpeg
    if (!FFmpegInterface::hasFFmpeg()) {
        msg = errmsg.arg(ExePath::getPath(QString::fromLatin1("ffmpeg")));
        return false;
    }

    // check ffprobe
    if (!MediaProbe::available()) { // The probe failed to start.
        msg = errmsg.arg(ExePath::getPath(QString::fromLatin1("ffprobe")));
        return false;
    }

    // sox is optional

    return true;
}

QString MediaConverter::errorMessage() const
{
    return m_pConv->errorMessage();
}

// private slots

void MediaConverter::readProcessOutput()
{
    m_pConv->parseProcessOutput(QString::fromLatin1(m_proc.readAll()));
}

void MediaConverter::convertProgressFinished(int exitcode, QProcess::ExitStatus)
{
    QFile output_file(m_outputFileName);
    QFile tmp_file(m_tmpFileName);

    if (exitcode == 0 && tmp_file.exists() && !m_stopped) { // succeed
        output_file.remove();
        if (!tmp_file.rename(m_outputFileName)) // Rename tmpfile to outputfile.
            exitcode = -1; // If the rename fails, return a negative exitcode.
    } else { // failed
        tmp_file.remove(); // Remove tmpfile if conversion failed.
    }

    if (exitcode == 0 && !m_stopped)
        emit progressRefreshed(100); // 100% finished
    else
        emit progressRefreshed(0);

    emit finished(exitcode);
}

void MediaConverter::convertProgressRefreshed(double percentage)
{
    emit progressRefreshed(static_cast<int>(percentage));
}

// private methods
