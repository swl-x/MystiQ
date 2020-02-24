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

#include "updatechecker.h"
#include "httpdownloader.h"
#include "updateinfoparser.h"
#include "../version.h"
#include "constants.h"

class UpdateChecker::Private
{
public:
    UpdateChecker::CheckResult result;
    QString version;
    unsigned int versionId;
    QString release_note;
    QString release_date;
    QString download_url;
    QString download_page;
    HttpDownloader downloader;
    Private() : result(UpdateChecker::None) { }
};

UpdateChecker::UpdateChecker(QObject *parent) :
    QObject(parent), p(new Private)
{
    p->downloader.setSizeLimit(Constants::getInteger("UpdateInfoSizeLimit"));
    connect(&p->downloader, SIGNAL(downloadFinished(bool,QString,QString)),
            this, SLOT(downloadFinished(bool,QString,QString)));
}

UpdateChecker::~UpdateChecker()
{
    delete p;
}

UpdateChecker::CheckResult UpdateChecker::result() const
{
    return p->result;
}

bool UpdateChecker::hasUpdate() const
{
    return p->result == UpdateChecker::UpdateFound;
}

QString UpdateChecker::versionName() const
{
    return p->version;
}

unsigned int UpdateChecker::versionId() const
{
    return p->versionId;
}

QString UpdateChecker::releaseDate() const
{
    return p->release_date;
}

QString UpdateChecker::releaseNotes() const
{
    return p->release_note;
}

QString UpdateChecker::downloadUrl() const
{
    return p->download_url;
}

QString UpdateChecker::downloadPage() const
{
    return p->download_page;
}

void UpdateChecker::checkUpdate()
{
    QString update_url = Constants::getString("UpdateInfoUrl");
    p->downloader.startDownload(update_url);
}

void UpdateChecker::downloadFinished(bool success, QString /*url*/, QString content)
{
    if (success) {
        XmlUpdateInfoParser parser;
        if (!parser.parse(content)) {
            // parse error
            p->result = DataError;
        } else if (parser.versionId() > VERSION_INTEGER) {
            // new version > current version
            p->result = UpdateFound;
            p->version = parser.version();
            p->versionId = parser.versionId();
            p->release_note = parser.releaseNotes();
            p->release_date = parser.releaseDate();
            p->download_url = parser.downloadUrl();
            p->download_page = parser.downloadPage();
        } else if (parser.versionId() < VERSION_INTEGER){
            // Using dev channel
            p->result = UpdateDevChanel;
        }
        else if (parser.versionId() == VERSION_INTEGER){
            // no new version found
            p->result = UpdateNotFound;
        }
    } else { // failed to connect to server
        p->result = ConnectionError;
    }
    emit receivedResult(static_cast<int>(p->result));
}
