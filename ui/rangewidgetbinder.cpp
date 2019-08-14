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

#include "rangewidgetbinder.h"

RangeWidgetBinder::RangeWidgetBinder(RangeSelector *sel,
                                     TimeRangeEdit *edit,
                                     QObject *parent) :
    QObject(parent),
    m_selector(sel),
    m_rangeEdit(edit)
{
    connect(m_selector, SIGNAL(valueChanged()), SLOT(sync_sel_to_edit()));
    connect(m_rangeEdit, SIGNAL(valueChanged()), SLOT(sync_edit_to_sel()));
}

// sync m_selector (visual) value to m_rangeEdit (text)
void RangeWidgetBinder::sync_sel_to_edit()
{
    int begin_time = m_selector->beginValue();
    int end_time = m_selector->endValue();
    bool from_begin = (begin_time == 0);
    bool to_end = (end_time == m_selector->maxValue());

    m_rangeEdit->setBeginTime(begin_time);
    m_rangeEdit->setEndTime(end_time);
    m_rangeEdit->setFromBegin(from_begin);
    m_rangeEdit->setToEnd(to_end);
}

// sync m_rangeEdit (text) value to m_selector (visual)
void RangeWidgetBinder::sync_edit_to_sel()
{
    int begin_time = m_rangeEdit->beginTime();
    int end_time = m_rangeEdit->endTime();

    if (m_rangeEdit->fromBegin())
        begin_time = 0;
    if (m_rangeEdit->toEnd())
        end_time = m_rangeEdit->maxTime();

    m_selector->setBeginValue(begin_time);
    m_selector->setEndValue(end_time);
}
