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

#ifndef NOTIFICATIONSERVICE_LIBNOTIFY_H
#define NOTIFICATIONSERVICE_LIBNOTIFY_H

#include "notificationservice.h"

/** An implementation of notification functions by using the libnotify library
    directly. This is the most preferred notification mechanism on Linux as
    it is desktop-independent and doesn't require the libnotify command line tools. */
class NotificationService_libnotify : public NotificationService
{
public:
    NotificationService_libnotify();
    ~NotificationService_libnotify();

    virtual void send(QWidget *parent, QString titie, QString message);

    virtual void send(QWidget *parent, QString title, QString message, int level);

    virtual bool serviceAvailable() const;

    static QString getVersion();

private:
    bool m_success;
};

#endif // NOTIFICATIONSERVICE_LIBNOTIFY_H
