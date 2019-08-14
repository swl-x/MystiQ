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

#ifndef XMLLOOKUPTABLE_H
#define XMLLOOKUPTABLE_H

#include <QIODevice>
#include <QMap>
#include <QHash>
#include <QString>

/**
 * @brief Read an xml file and provide path lookup syntax.
 */
class XmlLookupTable
{
public:
    XmlLookupTable();

    /**
     * @brief Read xml from file.
     * @param file a QIODevice opened for reading
     * @note Existing data are cleared no matter the function succeeds or fails.
     * @return true if succeed, false if failed
     */
    bool readFile(QIODevice& file);


    /**
     * @brief Read xml from string
     * @param s a string to read from
     * @note Existing data are cleared no matter the function succeeds or fails.
     * @return true if succeed, false if failed
     */
    bool readString(const QString& s);

    /**
     * @brief Set the lookup prefix.
     *
     * The lookup prefix will be prepended to the path in each lookup.
     * Setting the prefix can prevent repeatly typing common prefixes.
     *
     * @param path the path to be prepended
     */
    void setPrefix(const QString& path);

    /**
     * @brief get the current prefix
     */
    QString prefix() const;

    /**
     * @brief Find the data associated with @a path.
     *
     * @param path the path to lookup. Note that the prefix will be prepended
     * to this path. The path is similar to unix file paths but without leading
     * or trailing '/'. For example: level1/level2/level3
     *
     * @return the data associated witht @a path.
     */
    QString lookup(const QString &path, bool *ok=nullptr) const;

    /**
     * @brief alias for lookup()
     *
     * @see lookup()
     */
    QString operator [](const QString &path) const;

    /**
     * @brief Get the value of the attribute associated with @a path.
     *
     * @param path the path to the node
     *
     * @param attr name of the attribute
     *
     * @see lookup()
     */
    QString attribute(QString path, QString attr) const;

    /**
     * @brief Clear all xml data.
     */
    void clear();

private:

    class Entry
    {
    public:
        QMap<QString, QString> attributes;
        QString data;
    };

    QHash<QString, Entry> m_data;
    QString m_prefix;

    QString full_path(const QString &path) const;
};

#endif // XMLLOOKUPTABLE_H
