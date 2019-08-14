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

/* Linux Implementation of PowerManagement class */

#include "powermanagement.h"
#include <QDBusMessage>
#include <QDBusInterface>
#include <QProcess>
#include <QDebug>

namespace
{

/* The following power management functions were taken from qshutdown:
 *
 *   power_suspend()
 *   power_shutdown()
 *   power_hibernate()
 *
 * qshutdown is a program to shutdown/reboot/suspend/hibernate the computer.
 * For more information, please visit
 * https://launchpad.net/~hakaishi/+archive/qshutdown
 */

const bool verbose = true;

bool power_suspend()
{
    bool gnome_power1 = false;
    bool gnome_power2 = false;
    bool hal_works = false;
    QDBusMessage response;

    gnome_power1 = QProcess::startDetached("gnome-power-cmd.sh suspend");
    gnome_power2 = QProcess::startDetached("gnome-power-cmd suspend");
    if (!gnome_power1 && !gnome_power2 && verbose)
        qWarning() <<
            "W: gnome-power-cmd and gnome-power-cmd.sh didn't work";

    if (!gnome_power1 && !gnome_power2) {
        QDBusInterface powermanagement("org.freedesktop.Hal",
                           "/org/freedesktop/Hal/devices/computer",
                           "org.freedesktop.Hal.Device.SystemPowerManagement",
                           QDBusConnection::systemBus());
        response = powermanagement.call("Suspend", 0);
        if (response.type() == QDBusMessage::ErrorMessage) {
            if (verbose)
                qWarning() << "W: " << response.errorName()
                           << ":" << response.errorMessage();
        } else
            hal_works = true;
    }

    if (!hal_works && !gnome_power1 && !gnome_power2) {
        QDBusInterface
            powermanagement("org.freedesktop.DeviceKit.Power",
                    "/org/freedesktop/DeviceKit/Power",
                    "org.freedesktop.DeviceKit.Power",
                    QDBusConnection::systemBus());
        response = powermanagement.call("Suspend");
        if (response.type() == QDBusMessage::ErrorMessage) {
            if (verbose)
                qWarning() << "W: " << response.
                    errorName() << ":" << response.
                    errorMessage();
        } else
            hal_works = true;
    }

    if (!hal_works) {
        QDBusInterface
                powermanagement("org.freedesktop.PowerManagement",
                                "/org/freedesktop/PowerManagement",
                                "org.freedesktop.PowerManagement",
                                QDBusConnection::sessionBus());
        response = powermanagement.call("Suspend");
        if (response.type() == QDBusMessage::ErrorMessage) {
            if (verbose)
                qWarning() << "W: " << response.errorName()
                           << ":" << response.errorMessage();
        } else
            hal_works = true;
    }

    return hal_works;
}

bool power_shutdown()
{
    bool shutdown_works = false;
    bool gnome_power1 = false;
    bool gnome_power2 = false;
    bool hal_works = false;
    QDBusMessage response;

    QDBusInterface gnomeSessionManager("org.gnome.SessionManager",
                       "/org/gnome/SessionManager",
                       "org.gnome.SessionManager",
                       QDBusConnection::sessionBus());
    response = gnomeSessionManager.call("RequestShutdown");
    if (response.type() == QDBusMessage::ErrorMessage) {
        if (verbose)
            qWarning() << "W: " << response.
                errorName() << ":" << response.errorMessage();
        gnome_power1 =
            QProcess::startDetached("gnome-power-cmd.sh shutdown");
        gnome_power2 =
            QProcess::startDetached("gnome-power-cmd shutdown");
        if (verbose && !gnome_power1 && !gnome_power2)
            qWarning() <<
                "W: gnome-power-cmd and gnome-power-cmd.sh didn't work";
    } else
        shutdown_works = true;

    if (!shutdown_works) {
        QDBusInterface kdeSessionManager("org.kde.ksmserver", "/KSMServer",
                         "org.kde.KSMServerInterface",
                         QDBusConnection::sessionBus());
        response = kdeSessionManager.call("logout", 0, 2, 2);
        if (response.type() == QDBusMessage::ErrorMessage) {
            if (verbose)
                qWarning() << "W: " << response.
                    errorName() << ":" << response.errorMessage();
        } else
            shutdown_works = true;
    }

    if (!shutdown_works && !gnome_power1 && !gnome_power2) {
        QDBusInterface powermanagement("org.freedesktop.Hal",
                           "/org/freedesktop/Hal/devices/computer",
                           "org.freedesktop.Hal.Device.SystemPowerManagement",
                           QDBusConnection::systemBus());
        response = powermanagement.call("Shutdown");
        if (response.type() == QDBusMessage::ErrorMessage) {
            if (verbose)
                qWarning() << "W: " << response.
                    errorName() << ":" << response.
                    errorMessage();
        } else
            hal_works = true;
    }

    if (!hal_works && !shutdown_works && !gnome_power1 && !gnome_power2) {
        QDBusInterface powermanagement("org.freedesktop.ConsoleKit",
                           "/org/freedesktop/ConsoleKit/Manager",
                           "org.freedesktop.ConsoleKit.Manager",
                           QDBusConnection::systemBus());
        response = powermanagement.call("Stop");
        if (response.type() == QDBusMessage::ErrorMessage) {
            if (verbose)
                qWarning() << "W: " << response.
                    errorName() << ":" << response.
                    errorMessage();
            QProcess::startDetached("sudo shutdown -P now");
        }
    }

    return shutdown_works;
}

bool power_hibernate()
{
    bool gnome_power1 = false;
    bool gnome_power2 = false;
    bool hal_works = false;
    QDBusMessage response;

    gnome_power1 = QProcess::startDetached("gnome-power-cmd.sh hibernate");
    gnome_power2 = QProcess::startDetached("gnome-power-cmd hibernate");
    if (!gnome_power1 && !gnome_power2 && verbose)
        qWarning() <<
            "W: gnome-power-cmd and gnome-power-cmd.sh didn't work";

    if (!gnome_power1 && !gnome_power2) {
        QDBusInterface powermanagement("org.freedesktop.Hal",
                           "/org/freedesktop/Hal/devices/computer",
                           "org.freedesktop.Hal.Device.SystemPowerManagement",
                           QDBusConnection::systemBus());
        response = powermanagement.call("Hibernate");
        if (response.type() == QDBusMessage::ErrorMessage) {
            if (verbose)
                qWarning() << "W: " << response.
                    errorName() << ":" << response.
                    errorMessage();
        } else
            hal_works = true;
    }

    if (!hal_works && !gnome_power1 && !gnome_power2) {
        QDBusInterface
            powermanagement("org.freedesktop.DeviceKit.Power",
                    "/org/freedesktop/DeviceKit/Power",
                    "org.freedesktop.DeviceKit.Power",
                    QDBusConnection::systemBus());
        response = powermanagement.call("Hibernate");
        if (response.type() == QDBusMessage::ErrorMessage) {
            if (verbose)
                qWarning() << "W: " << response.
                    errorName() << ":" << response.
                    errorMessage();
        }
    }

    if (!hal_works) {
        QDBusInterface
                powermanagement("org.freedesktop.PowerManagement",
                                "/org/freedesktop/PowerManagement",
                                "org.freedesktop.PowerManagement",
                                QDBusConnection::sessionBus());
        response = powermanagement.call("Hibernate");
        if (response.type() == QDBusMessage::ErrorMessage) {
            if (verbose)
                qWarning() << "W: " << response.errorName()
                           << ":" << response.errorMessage();
        } else
            hal_works = true;
    }

    return hal_works;
}

} // anonymous namespace

bool PowerManagement::sendRequest(int action)
{
    switch (action) {
    case SHUTDOWN:
        return power_shutdown();
    case SUSPEND:
        return power_suspend();
    case HIBERNATE:
        return power_hibernate();
    }
    return false;
}

bool PowerManagement::implemented()
{
    return true;
}
