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

#ifndef FILEPATHOPERATIONS_H
#define FILEPATHOPERATIONS_H
#include <QString>
#include <QDir>
#include <QHash>
#include <QStandardPaths>

class FilePathOperations
{
public:
    static QString GenerateUniqueFileName(const QDir& output_dir, const QString& input_file_basename
                                          , const QString& ext
                                          , const QHash<QString, int>& extra);
    /*! Ensure unique output filename.
       If the destination filename already exists either on disk
       or in %extra, rename it to prevent overwritting
       completed tasks.
       @param filename the expected filename
       @param extra additional filenames to exclude
    */
    static QString GenerateUniqueFileName(const QString& filename
                                          , const QHash<QString, int>& extra);
    static QString GenerateTempFileName(const QString& filename);
private:
    FilePathOperations();
};

#endif // FILEPATHOPERATIONS_H
