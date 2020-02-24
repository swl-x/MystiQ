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

#include <QXmlStreamReader>
#include "updateinfoparser.h"
#include "constants.h"
#include "xmllookuptable.h"

XmlUpdateInfoParser::XmlUpdateInfoParser()
{
}

XmlUpdateInfoParser::~XmlUpdateInfoParser()
{
}

bool XmlUpdateInfoParser::parse(QString s)
{
    XmlLookupTable table;
    if (!table.readString(s))
        return false;
    table.setPrefix("MystiQVersionInfo");

    m_version = table.lookup("Name");
    m_vid = table.lookup("VersionId");
    m_releaseDate = table.lookup("ReleaseDate");
    m_releaseNotes = table.lookup("ReleaseNotes");
    m_downloadUrlWindows = table.lookup("DownloadLinks/WindowsInstaller");
    m_downloadUrlLinux = table.lookup("DownloadLinks/LinuxInstaller");
    m_downloadUrlMacOS = table.lookup("DownloadLinks/MacOSInstaller");
    m_downloadPage = table.lookup("DownloadPage");

    if (!m_version.isEmpty())
        return true;
    else
        return false;
}

QString XmlUpdateInfoParser::version() const
{
    return m_version;
}

unsigned int XmlUpdateInfoParser::versionId() const
{
    return m_vid.toUInt();
}

QString XmlUpdateInfoParser::releaseDate() const
{
    return m_releaseDate;
}

QString XmlUpdateInfoParser::releaseNotes() const
{
    return m_releaseNotes;
}

QString XmlUpdateInfoParser::downloadUrl() const
{
    #ifdef Q_OS_WIN32
    return m_downloadUrlWindows;
    #endif
    #ifdef Q_OS_LINUX
    return m_downloadUrlLinux;
    #endif
    #ifdef Q_OS_MACOS
    return m_downloadUrlMacOS;
    #endif
}

QString XmlUpdateInfoParser::downloadPage() const
{
    return m_downloadPage;
}
