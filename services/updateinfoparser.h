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

#ifndef UPDATEINFOPARSER_H
#define UPDATEINFOPARSER_H

#include <QString>

class UpdateInfoParser
{
public:
    UpdateInfoParser() { }
    virtual ~UpdateInfoParser() { }
    virtual bool parse(QString s) = 0;
    virtual QString version() const = 0;
    virtual QString releaseDate() const = 0;
    virtual QString releaseNotes() const = 0;
    virtual QString downloadUrl() const = 0;
    virtual QString downloadPage() const = 0;
};

QT_BEGIN_NAMESPACE
class QXmlStreamReader;
class QXmlStreamAttributes;
QT_END_NAMESPACE

class XmlUpdateInfoParser : public UpdateInfoParser
{
public:
    XmlUpdateInfoParser();
    ~XmlUpdateInfoParser();
    bool parse(QString s);
    QString version() const;
    unsigned int versionId() const;
    QString releaseDate() const;
    QString releaseNotes() const;
    QString downloadUrl() const;
    QString downloadPage() const;
private:
    QString m_version;
    QString m_vid;
    QString m_releaseDate;
    QString m_releaseNotes;
    QString m_downloadUrlWindows;
    QString m_downloadUrlLinux;
    QString m_downloadUrlMacOS;
    QString m_downloadPage;
};

#endif // UPDATEINFOPARSER_H
