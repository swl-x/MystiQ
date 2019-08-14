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

#ifndef HTTPDOWNLOADER_H
#define HTTPDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>

class HttpDownloader : public QObject
{
    Q_OBJECT
public:
    explicit HttpDownloader(QObject *parent = nullptr);

    virtual ~HttpDownloader();

    /**
     * @brief Set the maximum allowable download size.
     * @param limit the size limit in bytes. 0 means unlimited.
     */
    void setSizeLimit(unsigned int limit);

    /**
     * @brief Get the maximum allowable download size.
     * @return the size limit in bytes
     */
    unsigned int sizeLimit() const;

public slots:
    void startDownload(QString url);
    void cancelAllDownloads();

signals:
    void downloadFinished(bool success, QString url, QString content);

private slots:
    void slotDownloadFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager m_webCtrl;
    unsigned int m_sizeLimit;
    QMap<QNetworkReply*, QString> m_downloads;
    void readData(QString& dest, QNetworkReply *reply);
};

#endif // HTTPDOWNLOADER_H
