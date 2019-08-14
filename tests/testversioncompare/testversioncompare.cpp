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

#include "testversioncompare.h"

QTEST_MAIN(TestVersionCompare)

void TestVersionCompare::testVersionEquality()
{
	Version v1("0.1.0"), v2("0.1.0"), v3("1.1.0");
	QCOMPARE(v1, v2);
	QVERIFY(v1 != v3);
	QVERIFY(v2 != v3);
	QVERIFY(!(v1 == v3));
	QVERIFY(!(v2 == v3));
	QVERIFY(!(v1 != v2));
}

void TestVersionCompare::testVersionInequality()
{
	const int COUNT = 10;
	Version *v[COUNT*COUNT*COUNT] = {0};
	for (int i=0; i<COUNT; i++)
		for (int k=0; k<COUNT; k++)
			for (int u=0; u<COUNT; u++) {
				v[i*COUNT*COUNT+k*COUNT+u] =
					new Version(QString("%1.%2.%3").arg(i).arg(k).arg(u));
			}

	for (int i=0; i<COUNT*COUNT*COUNT-1; i++) {
		const Version& v1 = *v[i];
		const Version& v2 = *v[i+1];
		QVERIFY(v1 != v2);
		QVERIFY(v1 < v2);
		QVERIFY(v2 > v1);
		QVERIFY(v1 <= v1);
		QVERIFY(v1 >= v1);
	}

	for (int i=0; i<COUNT*COUNT*COUNT; i++)
		delete v[i];
}

void TestVersionCompare::testVersionSingleRange()
{
#define CHECK_CONTAIN(v) QVERIFY(range.containsVersion(Version(v)))
#define CHECK_NOT_CONTAIN(v) QVERIFY(!range.containsVersion(Version(v)))
	VersionRange range("0.1.0~0.2.1");
	CHECK_CONTAIN("0.1.0");
	CHECK_CONTAIN("0.2.1");
	CHECK_CONTAIN("0.1.1");
	CHECK_CONTAIN("0.2.0");
	CHECK_CONTAIN("0.1.10");
	CHECK_NOT_CONTAIN("0.0.9");
	CHECK_NOT_CONTAIN("0.0.10");
	CHECK_NOT_CONTAIN("0.2.2");
	CHECK_NOT_CONTAIN("0.2.10");
}
