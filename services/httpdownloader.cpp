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

#include <QFile>
#include <QNetworkRequest>
#include <vector>
#include "httpdownloader.h"

#define BUFFER_SIZE 1024

HttpDownloader::HttpDownloader(QObject *parent)
    : QObject(parent), m_sizeLimit(0)
{
    connect(&m_webCtrl, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotDownloadFinished(QNetworkReply*)));
}

HttpDownloader::~HttpDownloader()
{
    cancelAllDownloads();
}

void HttpDownloader::setSizeLimit(unsigned int limit)
{
    m_sizeLimit = limit;
}

unsigned int HttpDownloader::sizeLimit() const
{
    return m_sizeLimit;
}

void HttpDownloader::startDownload(QString url)
{
    QNetworkReply *reply = m_webCtrl.get(QNetworkRequest(url));
    m_downloads[reply] = url;
}

void HttpDownloader::cancelAllDownloads()
{
    QList<QNetworkReply*> replies = m_downloads.keys();
    foreach (QNetworkReply *reply, replies) {
        reply->abort();
        reply->deleteLater();
    }
    m_downloads.clear();
}

void HttpDownloader::slotDownloadFinished(QNetworkReply *reply)
{
    QString url = m_downloads[reply];
    bool success = !reply->error();
    QString content;
    readData(content, reply);
    m_downloads.remove(reply);
    reply->deleteLater();
    emit downloadFinished(success, url, content);
}

// read at most m_sizeLimit bytes from reply to dest
// or read all data if m_sizeLimit is 0
void HttpDownloader::readData(QString &dest, QNetworkReply *reply)
{
    if (m_sizeLimit == 0) {
        dest = reply->readAll();
    } else {
        std::vector<char> buffer;
        buffer.resize(m_sizeLimit + 1); // reserve data length and NULL byte
        reply->read(buffer.data(), m_sizeLimit); // buffer.data() is char*
        buffer[m_sizeLimit] = 0; // terminate the data with NULL
        dest = QString(buffer.data()); // convert the data to QString
    }
}
