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

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QString>
#include "notificationservice.h"

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

class Notification
{
public:
    enum NotificationType {
        TYPE_MSGBOX = 0, ///< use non-blocking message box
        TYPE_LIBNOTIFY, ///< use libnotify which doesn't require the command-line utility
        TYPE_NOTIFYSEND, ///< use the notify-send command line utility
        END_OF_TYPE
    };

    /** Initialize the notification subsystem.
     *  @warning This function is not thread-safe. Only call it in the main thread.
     */
    static void init();

    /** Release the notification subsystem.
     */
    static void release();

    /** Determine whether the notification type is available in the system.
     */
    static bool serviceAvailable(NotificationType type);

    /** Decide how the notification will be sent.
     *  @param type the type of the notification
     *  @return If the function succeeds, the function returns true.
     *          Otherwise, the function returns false and fallback to TYPE_QT.
     */
    static bool setType(NotificationType type);

    static void send(QWidget *parent, QString title, QString message);

    static void send(QWidget *parent, QString title, QString message, int level);
private:
    static NotificationType m_type;
};

#endif // NOTIFICATION_H
