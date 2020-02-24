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

#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QString>
#include <QXmlStreamReader>

/**
 * @brief Get update information from google code.
 */
class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(QObject *parent = nullptr);
    ~UpdateChecker();

    enum CheckResult {
        None,
        ConnectionError,
        DataError,
        UpdateNotFound,
        UpdateDevChanel,
        UpdateFound
    };

    CheckResult result() const;
    bool hasUpdate() const;
    QString versionName() const;
    unsigned int versionId() const;
    QString releaseDate() const;
    QString releaseNotes() const;
    QString downloadUrl() const;
    QString downloadPage() const;
    
signals:
    void receivedResult(int result);
    
public slots:
    void checkUpdate();

private slots:
    void downloadFinished(bool, QString, QString);
    
private:
    Q_DISABLE_COPY(UpdateChecker)
    class Private;
    Private *p;
};

#endif // UPDATECHECKER_H
