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

#include <QProcess>
#include "converter/exepath.h"
#include "mplayerpreviewer.h"

#define TIMEOUT 3000

MPlayerPreviewer::MPlayerPreviewer(QObject *parent) :
    AbstractPreviewer(parent),
    m_proc(new QProcess(this))
{
}

bool MPlayerPreviewer::available() const
{
    QProcess proc;
    QStringList param;
    // test whether mplayer could be invoked
    proc.start(ExePath::getPath("mplayer"), param);
    if (!proc.waitForStarted(TIMEOUT))
        return false;
    proc.kill();
    proc.waitForFinished(TIMEOUT);
    return true;
}

void MPlayerPreviewer::play(const QString& filename)
{
    play(filename, -1, -1);
}

void MPlayerPreviewer::play(const QString &filename, int t_begin, int t_end)
{
    QStringList param;
    stop();
    if (t_begin >= 0) // set begin time: -ss <seconds>
        static_cast<void>(param.append("-ss")), param.append(QString::number(t_begin));
    else
        t_begin = 0;
    if (t_end >= 0) { // set end time: -endpos <seconds>
        param.append("-endpos");
        param.append(QString::number(t_end - t_begin));
    }
    param.append(filename);
    m_proc->start(ExePath::getPath("mplayer"), param);
    m_proc->waitForStarted(TIMEOUT);
}

void MPlayerPreviewer::playFrom(const QString &filename, int t_begin)
{
    play(filename, t_begin, -1);
}

void MPlayerPreviewer::playUntil(const QString &filename, int t_end)
{
    play(filename, t_end, -1);
}

void MPlayerPreviewer::stop()
{
    m_proc->kill();
    m_proc->waitForFinished(TIMEOUT);
}
