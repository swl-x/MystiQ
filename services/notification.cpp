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

#include "notification.h"
#include "notificationservice-qt.h"
#include "notificationservice-notifysend.h"
#ifdef USE_LIBNOTIFY
 #include "notificationservice-libnotify.h"
#endif

namespace {
    NotificationService* notify_service[Notification::END_OF_TYPE];
}

Notification::NotificationType Notification::m_type;

#define SERVICE(type, cl) notify_service[type] = new cl();
void Notification::init()
{
    // define all notification services here
    SERVICE(TYPE_MSGBOX, NotificationService_qt)
    SERVICE(TYPE_NOTIFYSEND, NotificationService_NotifySend)
#ifdef USE_LIBNOTIFY
    SERVICE(TYPE_LIBNOTIFY, NotificationService_libnotify);
#endif

    // default to msgbox
    m_type = TYPE_MSGBOX;
}
#undef SERVICE

void Notification::release()
{
    for (int i=0; i<END_OF_TYPE; i++)
        delete notify_service[i]; // free all services
}

bool Notification::serviceAvailable(NotificationType type)
{
    if (type < 0 || type >= END_OF_TYPE)
        return false;
    NotificationService *service = notify_service[type];
    if (!service)
        return false;
    return service->serviceAvailable();
}

bool Notification::setType(NotificationType type)
{
    if (serviceAvailable(type)) {
        m_type = type;
        return true;
    } else {
        m_type = TYPE_MSGBOX;
        return false;
    }
}

void Notification::send(QWidget *parent, QString title, QString message)
{
    if (m_type < 0 || m_type >= END_OF_TYPE)
        return;
    notify_service[m_type]->send(parent, title, message);
}

void Notification::send(QWidget *parent, QString title, QString message, int level)
{
    if (m_type < 0 || m_type >= END_OF_TYPE)
        return;
    notify_service[m_type]->send(parent, title, message, level);
}
