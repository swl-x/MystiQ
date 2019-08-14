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

#include "ffplaypreviewer.h"
#include "converter/exepath.h"
#include <QProcess>
#include <QDebug>

#ifdef OPERATION_TIMEOUT
#   define TIMEOUT OPERATION_TIMEOUT
#else
#   define TIMEOUT 3000
#endif

#define DEFAULT_WIDTH 320
#define DEFAULT_HEIGHT 180

FFplayPreviewer::FFplayPreviewer(QObject *parent) :
    AbstractPreviewer(parent),
    m_proc(new QProcess),
    m_w(DEFAULT_WIDTH), m_h(DEFAULT_HEIGHT)
{
}

FFplayPreviewer::~FFplayPreviewer()
{
    delete m_proc;
}

bool FFplayPreviewer::available() const
{
    return FFplayAvailable();
}

void FFplayPreviewer::play(const QString &filename)
{
    ffplay_start(filename, -1, -1);
}

void FFplayPreviewer::play(const QString &filename, int t_begin, int t_end)
{
    ffplay_start(filename, t_begin, t_end);
}

void FFplayPreviewer::playFrom(const QString &filename, int t_begin)
{
    ffplay_start(filename, t_begin, -1);
}

void FFplayPreviewer::playUntil(const QString &filename, int t_end)
{
    ffplay_start(filename, -1, t_end);
}

void FFplayPreviewer::stop()
{
    if (m_proc->state() != QProcess::NotRunning) {
        m_proc->kill();
        m_proc->waitForFinished(TIMEOUT);
    }
}

void FFplayPreviewer::setWindowSize(int w, int h)
{
    m_w = w; m_h = h;
}

void FFplayPreviewer::setWindowTitle(QString str)
{
    m_title = str;
}

bool FFplayPreviewer::FFplayAvailable()
{
    QProcess proc;
    QStringList param;
    /* test whether ffplay could be invoked */
    proc.start(ExePath::getPath("ffplay"), param);
    if (!proc.waitForStarted(TIMEOUT))
        return false;
    proc.kill();
    proc.waitForFinished(TIMEOUT);
    return true;
}

/* If t_begin >= 0, start from t_begin; otherwise, start from time zero.
   If t_end >= 0, stop at t_end; othersize, play until end of file. */
void FFplayPreviewer::ffplay_start(const QString& filename, int t_begin, int t_end)
{
    QStringList param;
    stop();
    param.append("-autoexit");
    param.append("-x"); param.append(QString::number(m_w));
    param.append("-y"); param.append(QString::number(m_h));
    if (!m_title.isEmpty())
        static_cast<void>(param.append("-window_title")), param.append(m_title);
    if (t_begin >= 0)
        static_cast<void>(param.append("-ss")), param.append(QString::number(t_begin));
    if (t_end >= 0) {
        param.append("-t"); // duration
        if (t_begin >= 0)
            param.append(QString::number(t_end - t_begin)); // duration = end - begin
        else
            param.append(QString::number(t_end)); // start from time zero
    }
    param.append(filename);
    qDebug() << "ffplay" << param.join(" ");
    m_proc->start(ExePath::getPath("ffplay"), param);
    m_proc->waitForStarted(TIMEOUT);
}
