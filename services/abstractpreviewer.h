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

#ifndef ABSTRACTPREVIEWER_H
#define ABSTRACTPREVIEWER_H

#include <QObject>

class AbstractPreviewer : public QObject
{
    Q_OBJECT
public:
    explicit AbstractPreviewer(QObject *parent = nullptr);
    virtual ~AbstractPreviewer();

    /** @brief Determine whether this previewer can play files.
     */
    virtual bool available() const = 0;

    /** @brief Play the media file named @a filename.
     *
     *  This function must be asynchronous, i.e. it must return
     *  immediately without waiting for the player to exit.
     */
    virtual void play(const QString& filename) = 0;

    /** @brief Play a portion of the file from @a t_begin to @a t_end (seconds).
     *
     *  @see play(const QString&)
     */
    virtual void play(const QString &filename, int t_begin, int t_end) = 0;

    virtual void playFrom(const QString &filename, int t_begin) = 0;
    virtual void playUntil(const QString &filename, int t_end) = 0;

    virtual void stop() = 0;

private:
    Q_DISABLE_COPY(AbstractPreviewer)
};

#endif // ABSTRACTPREVIEWER_H
