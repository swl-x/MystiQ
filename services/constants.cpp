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

#define REGEXP_HEXDIGIT "[0-9a-fA-F]"

namespace
{
    bool constants_initialized = false;
    XmlLookupTable constants;
    QString color_pattern(QString("#(%1%1)(%1%1)(%1%1)(%1%1)?").arg(REGEXP_HEXDIGIT));

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
        QString qualified_str = QString("0x%1").arg(hex_str);
        bool ok;
        int value = qualified_str.toInt(&ok, 16);
        if (ok)
            return value;
        else
            return 0;
    }

    QColor str2color(const QString& color_str)
    {
        QRegExp color(color_pattern);
        if (color.indexIn(color_str) >= 0) {
            int r_value = hex2int(color.cap(1));
            int g_value = hex2int(color.cap(2));
            int b_value = hex2int(color.cap(3));
            int a_value = 0xff;
            if (!color.cap(4).isEmpty()) // with alpha value
                a_value = hex2int(color.cap(4));
            return QColor(r_value, g_value, b_value, a_value);
        } else {
            return QColor(0, 0, 0); // default black
        }
    }
}

bool Constants::readFile(QFile &file)
{
    constants.clear();
    constants_initialized = false;
    if (constants.readFile(file)) {
        constants_initialized = true;
        constants.setPrefix("MystiQConstants");
    }
    return constants_initialized;
}

bool Constants::getBool(const char *key)
{
    Q_ASSERT(constants_initialized);
    QString value = lookup_constant(key).trimmed().toLower();
    if (value.isEmpty() || value == "0" || value == "false")
        return false;
    else
        return true;
}

int Constants::getInteger(const char *key)
{
    Q_ASSERT(constants_initialized);
    return lookup_constant(key).toInt();
}

float Constants::getFloat(const char *key)
{
    Q_ASSERT(constants_initialized);
    return lookup_constant(key).toFloat();
}

QString Constants::getString(const char *key)
{
    Q_ASSERT(constants_initialized);
    return lookup_constant(key).trimmed();
}

QStringList Constants::getSpaceSeparatedList(const char *key)
{
    Q_ASSERT(constants_initialized);
    QString collapsed_string = lookup_constant(key).replace(QRegExp("[\n\t ]"), " ");
    return collapsed_string.split(" ", QString::SkipEmptyParts);
}

QColor Constants::getColor(const char *key)
{
    Q_ASSERT(constants_initialized);
    return str2color(lookup_constant(key));
}
