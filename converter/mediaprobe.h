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

#ifndef MEDIAPROBE_H
#define MEDIAPROBE_H

#include <QObject>

class MediaProbe : public QObject
{
    Q_OBJECT
public:
    explicit MediaProbe(QObject *parent = nullptr);
    virtual ~MediaProbe();

    /*! Determine whether the probing program (ffprobe) is available. */
    static bool available();

    /*! Start the probe on a file.
     * @param filename the name of the file to probe
     * @return If the process is started successfully, the function returns true.
     *  Otherwise, it returns false.
     * @note This function does not block until the process has finished.
     *       run() is the blocking alternative to this function.
     * @see run()
     */
    bool start(const QString& filename);
    bool start(const char* filename);

    /*! Start the probing process and block until the process has finished.
     * @param filename the file to probe
     * @param timeout timeout value in milliseconds
     * @return If the probing is successful, the function returns true.
     *  Otherwise, it returns false.
     * @note This function returns false when either the probing process
     *       cannot be started (usually because ffprobe not installed correctly)
     *       or the probed file is not recognized by ffprobe; so one cannot
     *       distinguish these two only by looking at the return value of this
     *       function.
     */
    bool run(const QString& filename, int timeout = 3000);

    /*! Block until the process has finished or until msecs milliseconds has passed.
     * @param msecs timeout
     * @return If the function exits because the process has finished, the function returns true.
     *  Otherwise, it returns false.
     */
    bool wait(int msecs = 3000);

    /*! Force the probe to stop.
     * The function will block until the probe is stopped.
     */
    void stop();

    /*! Returns whether the previous probing process has an error.
     *  For example, if the file is not recognized by ffprobe, the
     *  function returns false. If ffprobe has recognized the file
     *  and successfully extracts information from it, the function
     *  returns true.
     */
    bool error() const;

    /*! Returns the **hour** part of the duration */
    int hours() const;

    /*! Returns the **minutes** part of the duration
     *  The value is within 0 and 59(inclusive)
     */
    int minutes() const;

    /*! Returns the **seconds** part of the duration
     *  The value is within 0.0 and 60.0(non-inclusive)
     */
    double seconds() const;

    double mediaDuration() const;
    int mediaBitRate() const;

    bool hasAudio() const;
    int audioSampleRate() const;
    int audioBitRate() const;
    int audioChannels() const;
    const QString& audioCodec() const;

    bool hasVideo() const;
    int videoStreamIndex() const;
    int videoWidth() const;
    int videoHeight() const;
    int videoBitRate() const;
    double videoFrameRate() const;
    const QString& videoCodec() const;

    bool hasSubtitle() const;

signals:
    /*! This signal is fired when the probe finishes.
     */
    void process_finished();

public slots:

private slots:
    void m_proc_finished(int);

private:
    Q_DISABLE_COPY(MediaProbe)
    struct Private;
    Private *p;
};

#endif // MEDIAPROBE_H
