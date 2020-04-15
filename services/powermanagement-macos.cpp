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

/* Mac Implementation of PowerManagement class */

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
    //todo:
    bool hal_works = false;
    return hal_works;
}

bool power_shutdown()
{
    //todo:
    bool hal_works = false;
    return hal_works;
}

bool power_hibernate()
{
    //todo:
    bool hal_works = false;
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
