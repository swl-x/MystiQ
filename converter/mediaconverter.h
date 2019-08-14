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

#ifndef MEDIACONVERTER_H
#define MEDIACONVERTER_H

#include <QObject>
#include <QProcess>
#include "conversionparameters.h"

class ConversionParameters;
class ConverterInterface;
class AudioFilter;

class MediaConverter : public QObject
{
    Q_OBJECT
public:
    explicit MediaConverter(QObject *parent = nullptr);
    ~MediaConverter();

    /*!
     * Start the conversion process.
     * @return If the process is successfully started, the function returns true
     *  Otherwise, the function returns false.
     */
    bool start(ConversionParameters param);

    void stop();
    double progress();

    /*!
     * Check whether required external programs (e.g. ffmpeg)
     * is available.
     * @param msg [out] Output message if an error occurs.
     * @return true if all dependencies are met, false otherwise.
     */
    static bool checkExternalPrograms(QString &msg);

    /*!
     * Get the error message if the conversion fails.
     * @return the last output line ffmpeg prints, likely to be
     *         an error message
     */
    QString errorMessage() const;

signals:
    void finished(int exitcode);
    void progressRefreshed(int percentage);

public slots:

private slots:
    void readProcessOutput();
    void convertProgressFinished(int, QProcess::ExitStatus);
    void convertProgressRefreshed(double);

private:
    Q_DISABLE_COPY(MediaConverter)
    QProcess m_proc;
    ConversionParameters m_convParam;
    ConverterInterface *m_pConv;
    AudioFilter *m_pAudioFilter;
    QString m_outputFileName;
    QString m_tmpFileName;
    bool m_stopped;
};

#endif // MEDIACONVERTER_H
