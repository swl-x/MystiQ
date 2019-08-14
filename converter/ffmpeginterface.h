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

#ifndef FFMPEGINTERFACE_H
#define FFMPEGINTERFACE_H
#include <QObject>
#include <QList>
#include <QSet>
#include "conversionparameters.h"
#include "converterinterface.h"

class FFmpegInterface : public ConverterInterface
{
    Q_OBJECT
public:
    explicit FFmpegInterface(QObject *parent = nullptr);
    virtual ~FFmpegInterface();
    QString executableName() const;
    void reset();
    QProcess::ProcessChannel processReadChannel() const;
    bool needsAudioFiltering(const ConversionParameters& param) const;
    void fillParameterList(const ConversionParameters& param, QStringList& list
                           , bool *needs_audio_filter);
    void parseProcessOutput(const QString& data);
    double progress() const;
    QString errorMessage() const;

    static bool getAudioEncoders(QList<QString>& target);
    static bool getAudioEncoders(QSet<QString>& target);
    static bool getVideoEncoders(QList<QString>& target);
    static bool getVideoEncoders(QSet<QString>& target);
    static bool getSubtitleEncoders(QList<QString>& target);
    static bool getSubtitleEncoders(QSet<QString>& target);
    static QString getFFmpegVersionInfo();
    static QString getFFmpegCodecInfo();
    static QString getFFmpegFormatInfo();
    static bool getSupportedMuxingFormats(QSet<QString>& target);
    static bool getSupportedDemuxingFormats(QSet<QString>& target);
    static bool hasFFmpeg();

    static void refreshFFmpegInformation();

signals:
    void progressRefreshed(double percentage);

public slots:

private:
    Q_DISABLE_COPY(FFmpegInterface)
    struct Private;
    Private *p;
};

#endif // FFMPEGINTERFACE_H
