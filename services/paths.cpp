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

#include "paths.h"
#include <QDir>
#include <QDebug>
#include <QLibraryInfo>

namespace {
    QString app_path;
}

void Paths::setAppPath(const QString &path)
{
    app_path = path;
    qDebug() << "Set application path: " + app_path;
    qDebug() << "Application Path: " + app_path;
    qDebug() << "Data Path: " + dataPath();
    qDebug() << "translationPath: " + translationPath();
}

QString Paths::appPath()
{
    return app_path;
}

QString Paths::dataPath()
{
#ifdef DATA_PATH
    return QString(DATA_PATH);
#else
    return app_path;
#endif
}

QString Paths::dataFileName(const QString &filename)
{
    return QDir(dataPath()).absoluteFilePath(filename);
}

QString Paths::translationPath()
{
    return QDir(dataPath()).absoluteFilePath("translations");
}

QString Paths::qtTranslationPath()
{
#ifndef Q_OS_WIN32 // unix: load qt translation file from the default path
    return QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#else // windows: load qt translation file from translation directory
    return translationPath();
#endif
}
