/*  MystiQ - a qt4 gui frontend for ffmpeg
 *  Copyright (C) 2011-2013 Timothy Lin <lzh9102@gmail.com>
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

#include "testupdateinfoparser.h"

QTEST_MAIN(TestUpdateInfoParser)

void TestUpdateInfoParser::testParseXml()
{
	const char *xml =
		"<?xml version=\"1.0\"?>"
		"<MystiQVersionInfo>"
		"<Name>0.1.8</Name>"
		"<ReleaseDate>20120213</ReleaseDate>"
		"<ReleaseNotes>release notes</ReleaseNotes>"
		"</MystiQVersionInfo>";
	XmlUpdateInfoParser parser;
    bool success = parser.parse(QString(xml));
    QVERIFY(success);
	QCOMPARE(QString("0.1.8"), parser.version());
	QCOMPARE(QString("20120213"), parser.releaseDate());
	QCOMPARE(QString("release notes"), parser.releaseNotes());
	/* download link not tested because it varies across platforms */
}
