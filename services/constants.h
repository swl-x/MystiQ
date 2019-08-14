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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QStringList>
#include <QFile>
#include <QColor>

class Constants
{
public:
    static bool readFile(QFile& file);

    static bool getBool(const char *key);
    static int getInteger(const char *key);
    static float getFloat(const char *key);
    static QString getString(const char *key);
    static QStringList getSpaceSeparatedList(const char *key);
    static QColor getColor(const char *key);

private:
    Constants();
    ~Constants();
    Q_DISABLE_COPY(Constants)
};

#endif // CONSTANTS_H
