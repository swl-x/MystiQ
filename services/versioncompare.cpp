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

#include "versioncompare.h"
#include <QRegularExpression>

// Version

/* patterns to extract the version number
 * first matched pattern will be used
 */
static const char *version_patterns[] = {
    "([0-9]+)\\.([0-9]+)\\.([0-9]+)",
    "([0-9]+)\\.([0-9]+)",
    "([0-9]+)",
    nullptr // end of array
};

Version::Version(int major, int minor, int patch)
    : m_major(major), m_minor(minor), m_patch(patch)
{
}

Version::Version(const QString &s)
{
    m_major = m_minor = m_patch = -1; // invalid value
    for (int i=0; version_patterns[i]; i++) { // try each pattern
        QRegularExpression pattern(version_patterns[i]);
        QRegularExpressionMatch match = pattern.match(s);
        if (match.hasMatch()) {
            const QStringList cap = match.capturedTexts();
            const int capture_count = cap.size() - 1;
            m_major = m_minor = m_patch = 0;
            if (capture_count >= 1)
                m_major = cap[1].toInt();
            if (capture_count >= 2)
                m_minor = cap[2].toInt();
            if (capture_count >= 3)
                m_patch = cap[3].toInt();
            break;
        }
    }
}

Version Version::fromString(const QString &s)
{
    return Version(s);
}

QString Version::toString() const
{
    return QString("%1.%2.%3").arg(m_major).arg(m_minor).arg(m_patch);
}

bool Version::operator <(const Version& other) const
{
    if (m_major < 0 || other.m_major < 0) // invalid value
        return false;
    if (m_major == other.m_major) {
        if (m_minor == other.m_minor) {
            return m_patch < other.m_patch;
        } else
            return m_minor < other.m_minor;
    } else
        return m_major < other.m_major;
}

bool Version::operator >(const Version& other) const
{
    return other < *this;
}

bool Version::operator ==(const Version& other) const
{
    return !(other < *this || *this < other);
}

bool Version::operator !=(const Version& other) const
{
    return other < *this || *this < other;
}

bool Version::operator <=(const Version& other) const
{
    return !(other < *this);
}

bool Version::operator >=(const Version& other) const
{
    return !(*this < other);
}

// VersionRange

VersionRange::VersionRange(const QString &s)
{
    if (!s.isEmpty()) {
        QStringList lst = s.split(",", Qt::SkipEmptyParts);
        foreach (QString range, lst) {
            m_range.push_back(range.trimmed());
        }
    }
}

bool VersionRange::containsVersion(const Version &version) const
{
    foreach (QString range, m_range) {
        if (match_range(range, version))
            return true;
    }
    return false;
}

bool VersionRange::match_range(const QString& range, const Version &version) const
{
    if (range.indexOf("~") != -1) { // v1~v2
        QStringList ranges = range.split("~");
        Version version_lower(ranges[0]), version_upper(ranges[1]);
        return version_lower <= version && version <= version_upper;
    } else if (range.startsWith("le")) { // v<=v0
        return version <= Version(range);
    } else if (range.startsWith("ge")) { // v>=v0
        return version >= Version(range);
    } else if (range.startsWith("lt")) { // v<v0
        return version < Version(range);
    } else if (range.startsWith("gt")) { // v>v0
        return version > Version(range);
    } else { // v==v0
        return version == Version(range);
    }
}
