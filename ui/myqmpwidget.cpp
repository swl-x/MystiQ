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

#include "myqmpwidget.h"
#include "converter/exepath.h"

MyQMPwidget::MyQMPwidget(QWidget *parent) :
    QMPwidget(parent)
{
    QMPwidget::setMPlayerPath(ExePath::getPath("mplayer"));
    // disable mouse doubleclick event by not passing it to QMPwidget
}

MyQMPwidget::~MyQMPwidget()
{
}

void MyQMPwidget::mouseDoubleClickEvent(QMouseEvent */*event*/)
{
    // disable mouse doubleclick event by not passing it to QMPwidget
}

void MyQMPwidget::keyPressEvent(QKeyEvent */*event*/)
{
    // disable key events by not passing them to QMPwidget
}

void MyQMPwidget::load(const QString &url)
{
    QMPwidget::load(url);
}

void MyQMPwidget::pause()
{
    QMPwidget::writeCommand("pause");
}
