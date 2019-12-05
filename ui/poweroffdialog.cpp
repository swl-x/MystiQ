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

#include <QMessageBox>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include "poweroffdialog.h"
#include "ui_poweroffdialog.h"
#include "services/powermanagement.h"
#include "services/constants.h"

PoweroffDialog::PoweroffDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PoweroffDialog),
    m_timer(new QTimer(this))
{
    ui->setupUi(this);
    connect(ui->btnExecute, SIGNAL(clicked())
            , this, SLOT(btnExecute_click()));
    connect(ui->btnCancel, SIGNAL(clicked())
            , this, SLOT(btnCancel_click()));
    connect(this, SIGNAL(accepted())
            , this, SLOT(dialog_accepted()));
    connect(this, SIGNAL(rejected())
            , this, SLOT(dialog_rejected()));
    connect(m_timer, SIGNAL(timeout())
            , this, SLOT(timer_event()));

    // setup timer
    m_timer->setInterval(1000); // 1 second
    m_timer->setSingleShot(false); // always execute the timer
    m_timer->stop();

    // topmost frameless window
    setWindowFlags(Qt::Window
                   | Qt::WindowStaysOnTopHint
                   | Qt::FramelessWindowHint
                   | Qt::ToolTip);
}

PoweroffDialog::~PoweroffDialog()
{
    delete ui;
}

int PoweroffDialog::exec(int action)
{
    const char *icon_id = "";
    QString button_text = "";

    switch (action) {
    case PowerManagement::SHUTDOWN:
        //: Shutdown the computer
        button_text = tr("Shutdown immediately");
        icon_id = ":/actions/icons/system_shutdown";
        break;
    case PowerManagement::SUSPEND:
        //: Suspend the computer (sleep to ram, standby)
        button_text = tr("Suspend immediately");
        icon_id = ":/actions/icons/system_suspend";
        break;
    case PowerManagement::HIBERNATE:
        //: Hibernate the computer (sleep to disk, completely poweroff)
        button_text = tr("Hibernate immediately");
        icon_id = ":/actions/icons/system_hibernate";
        break;
    default:
        Q_ASSERT_X(false, __FUNCTION__, "Incorrect id! Be sure to handle every power action in switch().");
    }

    ui->btnExecute->setIcon(QIcon(icon_id));
    ui->btnExecute->setText(button_text);

    m_action = action;
    m_time = Constants::getInteger("PoweroffTimeout");
    m_timer->start();
    refresh_message();
    adjustSize();

    // center window in screen
    QGuiApplication::screens();
    const QRect screen = QGuiApplication::primaryScreen()->virtualGeometry();
    move(screen.center() - this->rect().center());

    return QDialog::exec();
}

int PoweroffDialog::exec()
{
    return QDialog::Rejected;
}

void PoweroffDialog::show()
{

}

void PoweroffDialog::btnExecute_click()
{
    accept();
}

void PoweroffDialog::btnCancel_click()
{
    reject();
}

void PoweroffDialog::dialog_accepted()
{
    m_timer->stop();
    m_success = PowerManagement::sendRequest(m_action);
    if (!m_success) {
        QString action_str;
        switch (m_action) {
        case PowerManagement::SHUTDOWN:
            //: Shutdown the computer
            action_str = tr("Shutdown");
            break;
        case PowerManagement::SUSPEND:
            //: Suspend the computer (sleep to ram, standby)
            action_str = tr("Suspend");
            break;
        case PowerManagement::HIBERNATE:
            //: Hibernate the computer (sleep to disk, completely poweroff)
            action_str = tr("Hibernate");
            break;
        default:
            Q_ASSERT_X(false, __FUNCTION__, "Incorrect id! Be sure to handle every power action in switch().");
        }
        QMessageBox::critical(this, windowTitle(), tr("Operation Failed: %1").arg(action_str));
    }
}

void PoweroffDialog::dialog_rejected()
{
    m_timer->stop();
}

void PoweroffDialog::timer_event()
{
    if (--m_time <= 0) {
        m_timer->stop();
        accept();
    }
    refresh_message();
}

void PoweroffDialog::refresh_message()
{
    QString msg;

    switch (m_action) {
    case PowerManagement::SHUTDOWN:
        msg = tr("Shutting down in <b>%1</b> seconds").arg(m_time);
        break;
    case PowerManagement::SUSPEND:
        msg = tr("Suspending in <b>%1</b> seconds").arg(m_time);
        break;
    case PowerManagement::HIBERNATE:
        msg = tr("Hibernating in <b>%1</b> seconds").arg(m_time);
        break;
    default:
        Q_ASSERT_X(false, __FUNCTION__, "Incorrect id! Be sure to handle every power action in switch().");
    }

    ui->lblMessage->setText(msg);
}
