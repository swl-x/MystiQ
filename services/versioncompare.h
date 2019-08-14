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

#ifndef VERSIONCOMPARE_H
#define VERSIONCOMPARE_H

#include <QStringList>

class Version
{
public:
    Version(int major, int minor, int patch);
    explicit Version(const QString& s);
    static Version fromString(const QString& s);
    QString toString() const;
    bool operator <(const Version& other) const;
    bool operator >(const Version& other) const;
    bool operator ==(const Version& other) const;
    bool operator !=(const Version& other) const;
    bool operator <=(const Version& other) const;
    bool operator >=(const Version& other) const;
private:
    int m_major, m_minor, m_patch;
};

/**
 * @brief This class represets a set of versions.
 *
 * The format of version range string can contain multiple
 * intervals separated by commas. Each interval can be in one of the
 * following formats:
 * "x.x.x~x.x.x" (inclusive range),
 * "lt x.x.x", (less than),
 * "gt x.x.x", (greater than),
 * "le x.x.x", (less than or equal to),
 * "ge x.x.x", (greater than or equal to)
 * and "x.x.x" (exactly the version).
 */
class VersionRange
{
public:
    explicit VersionRange(const QString& s);
    static VersionRange fromString(const QString& s);
    bool containsVersion(const Version& version) const;
private:
    QStringList m_range;
    bool match_range(const QString& range, const Version& version) const;
};

#endif // VERSIONCOMPARE_H
