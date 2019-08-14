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

#include "notificationservice-libnotify.h"
#include <libnotify/notify.h>

NotificationService_libnotify::NotificationService_libnotify() : m_success(true)
{
    if (!notify_is_initted())
        m_success = notify_init("MystiQ");
}

NotificationService_libnotify::~NotificationService_libnotify()
{
    /* Do not call notify_uninit() here because other instances
       of this class may be still using the notification system. */
}

void NotificationService_libnotify::send(QWidget *parent, QString title, QString message)
{
    send(parent, title, message, NotifyLevel::INFO);
}

void NotificationService_libnotify::send(QWidget */*parent*/, QString title, QString message, int level)
{
    const char *icon;

    // icon
    switch (level) {
    case NotifyLevel::INFO:
        icon = "dialog-information";
        break;
    case NotifyLevel::WARNING:
        icon = "dialog-warning";
        break;
    case NotifyLevel::CRITICAL:
        icon = "dialog-error";
        break;
    default: // no icon
        icon = "";
    }

/* In older libnotify, notify_notification_new() takes 4 arguments. */
#ifdef NOTIFY_CHECK_VERSION
#if NOTIFY_CHECK_VERSION(0, 7, 0)
    NotifyNotification *msg
            = notify_notification_new(title.toLocal8Bit().data()
                                      , message.toLocal8Bit().data(), icon);
#else
    NotifyNotification *msg
            = notify_notification_new(title.toLocal8Bit().data()
                                      , message.toLocal8Bit().data(), icon, 0);
#endif
#else
    NotifyNotification *msg
            = notify_notification_new(title.toLocal8Bit().data()
                                      , message.toLocal8Bit().data(), icon, 0);
#endif

    notify_notification_show(msg, 0);
}

bool NotificationService_libnotify::serviceAvailable() const
{
    return m_success;
}

QString NotificationService_libnotify::getVersion()
{
#if defined(NOTIFY_VERSION_MAJOR) && defined(NOTIFY_VERSION_MINOR) && defined(NOTIFY_VERSION_MICRO)
    return QString("%1.%2.%3").arg(NOTIFY_VERSION_MAJOR)
            .arg(NOTIFY_VERSION_MINOR).arg(NOTIFY_VERSION_MICRO);
#else
    return QString("");
#endif
}
