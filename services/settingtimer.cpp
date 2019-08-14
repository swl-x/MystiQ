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

#include <QSettings>
#include <QDateTime>
#include "settingtimer.h"

namespace
{
qint64 msecsTo(const QDateTime& t1, const QDateTime& t2)
{
    qint64 days = t1.daysTo(t2);
    qint64 msecs = t1.time().msecsTo(t2.time());
    return days * (24*60*60*1000) + msecs;
}
}

SettingTimer::SettingTimer(const QString& key)
    : m_key(key)
{
}

void SettingTimer::start()
{
    restart();
}

void SettingTimer::restart()
{
    QSettings().setValue(m_key, QDateTime::currentDateTime());
}

void SettingTimer::invalidate()
{
    QSettings().remove(m_key);
}

bool SettingTimer::isValid() const
{
    return QSettings().contains(m_key);
}

qint64 SettingTimer::elapsedMilliseconds() const
{
    QDateTime prev_time = QSettings().value(m_key).toDateTime();
    QDateTime current_time = QDateTime::currentDateTime();
    return msecsTo(prev_time, current_time);
}

qint64 SettingTimer::elapsedSeconds() const
{
    const int milliseconds_per_second = 1000;
    return elapsedMilliseconds() / milliseconds_per_second;
}
