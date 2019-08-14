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

/* This file is taken from smplayer */

#include "extensions.h"
#include "constants.h"

ExtensionList::ExtensionList() : QStringList()
{
}

QString ExtensionList::forFilter() {
	QString s;
	for (int n=0; n < count(); n++) {
		s = s + "*." + at(n) + " ";
	}
    if (!s.isEmpty()) s = "(" + s + ")";
	return s;
}

QString ExtensionList::forRegExp() {
	QString s;
	for (int n=0; n < count(); n++) {
		if (!s.isEmpty()) s = s + "|";
		s = s + "^" + at(n) + "$";
	}
	return s;
}

Extensions::Extensions()
{
    QStringList video_exts = Constants::getSpaceSeparatedList("VideoExtensions");
    QStringList audio_exts = Constants::getSpaceSeparatedList("AudioExtensions");

    foreach (QString ext, video_exts)
        _video << ext;

    foreach (QString ext, audio_exts)
        _audio << ext;

    // multimedia = union of video and audio
    _multimedia = _video;
    foreach (QString ext, audio_exts) {
        if (!_multimedia.contains(ext))
            _multimedia << ext;
    }
}

Extensions::~Extensions() {
}

bool Extensions::contains(const QString &ext) const
{
    return _multimedia.contains(ext, Qt::CaseInsensitive);
}
