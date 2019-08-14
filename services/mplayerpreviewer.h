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

#ifndef MPLAYERPREVIEWER_H
#define MPLAYERPREVIEWER_H

#include <QProcess>
#include "abstractpreviewer.h"

class MPlayerPreviewer : public AbstractPreviewer
{
    Q_OBJECT
public:
    explicit MPlayerPreviewer(QObject *parent = nullptr);

    bool available() const;

    /** Play the media file with ffplay.
     *  If a media file is being played, it will be stopped before
     *  playing the new one.
     */
    void play(const QString& filename);
    void play(const QString &filename, int t_begin, int t_end);

    void playFrom(const QString &filename, int t_begin);
    void playUntil(const QString &filename, int t_end);

    void stop();

private:
    QProcess *m_proc;

};

#endif // MPLAYERPREVIEWER_H
