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

#include <QDebug>
#include <QRegularExpression>
#include "constants.h"
#include "xmllookuptable.h"

#define REGEXP_HEXDIGIT QString::fromLatin1("[0-9a-fA-F]"

namespace
{
    bool constants_initialized = false;
    XmlLookupTable constants;
    QString color_pattern(QString::fromLatin1("#(%1%1)(%1%1)(%1%1)(%1%1)?").arg(REGEXP_HEXDIGIT)));

    QString lookup_constant(const QString& key)
    {
        bool ok;
        QString result = constants.lookup(key, &ok);
        if (!ok)
            qWarning() << "Constants: lookup undefined key " << key;
        return result;
    }

    int hex2int(const QString& hex_str)
    {
        QString qualified_str = QString::fromLatin1("0x%1").arg(hex_str);
        bool ok;
        int value = qualified_str.toInt(&ok, 16);
        if (ok)
            return value;
        else
            return 0;
    }

    QColor str2color(const QString& color_str)
    {
        QColor c = QColor(0, 0, 0); // default black
        QRegularExpression color(color_pattern);
        QRegularExpressionMatch match = color.match(color_str);
        if (match.hasMatch()) {
            c.setRed(hex2int(match.captured(1)));
            c.setGreen(hex2int(match.captured(2)));
            c.setBlue(hex2int(match.captured(3)));
            if(match.hasCaptured(4)) { // has alpha value
                c.setAlpha(hex2int(match.captured(4)));
            }
        }
        return c;
    }
}

bool Constants::readFile(QFile &file)
{
    constants.clear();
    constants_initialized = false;
    if (constants.readFile(file)) {
        constants_initialized = true;
        constants.setPrefix(QString::fromLatin1("MystiQConstants"));
    }
    return constants_initialized;
}

bool Constants::getBool(const char *key)
{
    Q_ASSERT(constants_initialized);
    QString value = lookup_constant(QString::fromLatin1(key)).trimmed().toLower();
    if (value.isEmpty() || value == QString::fromLatin1("0")
        || value == QString::fromLatin1("false"))
        return false;
    else
        return true;
}

int Constants::getInteger(const char *key)
{
    Q_ASSERT(constants_initialized);
    return lookup_constant(QString::fromLatin1(key)).toInt();
}

float Constants::getFloat(const char *key)
{
    Q_ASSERT(constants_initialized);
    return lookup_constant(QString::fromLatin1(key)).toFloat();
}

QString Constants::getString(const char *key)
{
    Q_ASSERT(constants_initialized);
    return lookup_constant(QString::fromLatin1(key)).trimmed();
}

QStringList Constants::getSpaceSeparatedList(const char *key)
{
    Q_ASSERT(constants_initialized);
    QString collapsed_string = lookup_constant(QString::fromLatin1(key))
                                   .replace(QRegularExpression(QString::fromLatin1("[\n\t ]")),
                                            QString::fromLatin1(" "));
    return collapsed_string.split(QString::fromLatin1(" "), Qt::SkipEmptyParts);
}

QColor Constants::getColor(const char *key)
{
    Q_ASSERT(constants_initialized);
    return str2color(lookup_constant(QString::fromLatin1(key)));
}
