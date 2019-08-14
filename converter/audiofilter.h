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

#ifndef AUDIOFILTER_H
#define AUDIOFILTER_H

#include <QObject>

class QProcess;
class ConversionParameters;

class AudioFilter : public QObject
{
    Q_OBJECT
public:
    explicit AudioFilter(QObject *parent = nullptr);

    /**
     * Start the audio-filtering process pipeline.
     * @param param the conversion parameter
     * @param dest the process to receive the output from stdin
     */
    bool start(ConversionParameters& param, QProcess *dest);

    /**
     * Check if execution conditions are met.
     * @return true if AudioFilter works, false otherwise.
     */
    static bool available();

signals:

public slots:

private slots:

private:
    QProcess *m_extractAudioProc;
    QProcess *m_soxProc;
    bool m_useSoxFormat;
};

#endif // AUDIOFILTER_H
