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

#ifndef SETTINGTIMER_H
#define SETTINGTIMER_H

#include <QString>

/**
 * @brief timer using QSettings to maintain the time across sessions
 */
class SettingTimer
{
public:
    SettingTimer(const QString& key);
    void start();
    void restart();
    void invalidate();
    bool isValid() const;
    qint64 elapsedMilliseconds() const;
    qint64 elapsedSeconds() const;
private:
    QString m_key;
};

#endif // SETTINGTIMER_H
