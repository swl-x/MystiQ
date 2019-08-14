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

#ifndef NOTIFICATIONSERVICEQT_H
#define NOTIFICATIONSERVICEQT_H

#include "notificationservice.h"

/** An implementation of notification functions by an always-on-top, non-modal QMessageBox */
class NotificationService_qt : public NotificationService
{
public:
    NotificationService_qt();
    virtual ~NotificationService_qt();

    virtual void send(QWidget *parent, QString titie, QString message);

    virtual void send(QWidget *parent, QString title, QString message, int level);

    virtual bool serviceAvailable() const;

};

#endif // NOTIFICATIONSERVICEQT_H
