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

#ifndef POWERMANAGEMENT_H
#define POWERMANAGEMENT_H

class PowerManagement
{
public:

    /* Power Management Function ID
     */
    enum PowerAction {
        SHUTDOWN = 0,
        SUSPEND,
        HIBERNATE,
        ACTION_COUNT
    };

    /**
     * Send the power management action.
     * @param action a PowerAction indicating the desired action
     */
    static bool sendRequest(int action);

    /*
       This function is used to check whether there's an implementation
       for the current build. You should always return true in this function
       when implementing PowerManagement for another system/environment.
     */
    static bool implemented();

private:
    PowerManagement();
};

#endif // POWERMANAGEMENT_H
