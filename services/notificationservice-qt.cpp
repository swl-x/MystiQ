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

#include "notificationservice-qt.h"
#include <QProcess>
#include <QMessageBox>

NotificationService_qt::NotificationService_qt()
{

}

NotificationService_qt::~NotificationService_qt()
{

}

void NotificationService_qt::send(QWidget *parent, QString title, QString message)
{
    send(parent, title, message, NotifyLevel::INFO);
}

void NotificationService_qt::send(QWidget *parent, QString title, QString message, int level)
{
    QMessageBox *msgbox = new QMessageBox(parent);
    msgbox->setAttribute(Qt::WA_DeleteOnClose); // delete itself on close
    msgbox->setWindowFlags(msgbox->windowFlags()
                           | Qt::WindowStaysOnTopHint); // always on top
    msgbox->setStandardButtons(QMessageBox::Ok);
    msgbox->setWindowTitle(title);
    msgbox->setText(message);
    msgbox->setModal(false); // non-modal message box

    switch (level) {
    case NotifyLevel::INFO:
        msgbox->setIcon(QMessageBox::Information);
        break;
    case NotifyLevel::WARNING:
        msgbox->setIcon(QMessageBox::Warning);
        break;
    case NotifyLevel::CRITICAL:
        msgbox->setIcon(QMessageBox::Critical);
        break;
    default:
        msgbox->setIcon(QMessageBox::NoIcon);
    }

    msgbox->show();
}

bool NotificationService_qt::serviceAvailable() const
{
    return true;
}
