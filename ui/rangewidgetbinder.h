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

#ifndef RANGEWIDGETBINDER_H
#define RANGEWIDGETBINDER_H

#include <QObject>
#include "rangeselector.h"
#include "timerangeedit.h"

/**
 * @brief Automatically sync between a RangeSelector and a TimeRangeEdit.
 */
class RangeWidgetBinder : public QObject
{
    Q_OBJECT
public:
    explicit RangeWidgetBinder(RangeSelector *sel,
                               TimeRangeEdit *edit,
                               QObject *parent = nullptr);

private slots:
    void sync_sel_to_edit();
    void sync_edit_to_sel();

private:
    RangeSelector *m_selector;
    TimeRangeEdit *m_rangeEdit;

};

#endif // RANGEWIDGETBINDER_H
