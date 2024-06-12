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

#include <QCheckBox>
#include <QTimeEdit>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "timerangeedit.h"

#define SECS_TO_QTIME(s) QTime(0, 0).addSecs(s)
#define QTIME_TO_SECS(t) (t).hour()*3600 + (t).minute()*60 + (t).second()

TimeRangeEdit::TimeRangeEdit(QWidget *parent) :
    QWidget(parent),
    m_timeBegin(new QTimeEdit(this)),
    m_timeEnd(new QTimeEdit(this)),
    m_chkFromBegin(new QCheckBox(this)),
    m_chkToEnd(new QCheckBox(this))
{
    m_chkFromBegin->setText(tr("From Begin"));
    m_chkToEnd->setText(tr("To End"));
    m_timeBegin->setDisplayFormat(QString::fromLatin1("hh:mm:ss"));
    m_timeBegin->setSelectedSection(QTimeEdit::SecondSection);
    m_timeEnd->setDisplayFormat(QString::fromLatin1("hh:mm:ss"));
    m_timeEnd->setSelectedSection(QTimeEdit::SecondSection);

    QHBoxLayout *main_layout = new QHBoxLayout(this);
    QVBoxLayout *left_layout = new QVBoxLayout(this);
    QVBoxLayout *right_layout = new QVBoxLayout(this);

    left_layout->addWidget(m_chkFromBegin);
    left_layout->addWidget(m_timeBegin);
    right_layout->addWidget(m_chkToEnd);
    right_layout->addWidget(m_timeEnd);

    main_layout->addLayout(left_layout);
    main_layout->addLayout(right_layout);

    setLayout(main_layout);

    connect(m_timeBegin, SIGNAL(timeChanged(QTime)), SLOT(time_changed()));
    connect(m_timeEnd, SIGNAL(timeChanged(QTime)), SLOT(time_changed()));
    connect(m_chkFromBegin, SIGNAL(toggled(bool)), SLOT(time_changed()));
    connect(m_chkToEnd, SIGNAL(toggled(bool)), SLOT(time_changed()));

    setFromBegin(true);
    setToEnd(true);
}

int TimeRangeEdit::maxTime() const
{
    return QTIME_TO_SECS(m_timeEnd->maximumTime());
}

int TimeRangeEdit::beginTime() const
{
    if (m_chkFromBegin->isChecked())
        return QTIME_TO_SECS(m_timeBegin->minimumTime());
    else
        return QTIME_TO_SECS(m_timeBegin->time());
}

int TimeRangeEdit::endTime() const
{
    if (m_chkToEnd->isChecked())
        return QTIME_TO_SECS(m_timeEnd->maximumTime());
    else
        return QTIME_TO_SECS(m_timeEnd->time());
}

bool TimeRangeEdit::fromBegin() const
{
    return m_chkFromBegin->isChecked();
}

bool TimeRangeEdit::toEnd() const
{
    return m_chkToEnd->isChecked();
}

void TimeRangeEdit::setMaxTime(int sec)
{
    m_timeBegin->setMinimumTime(SECS_TO_QTIME(0));
    m_timeEnd->setMaximumTime(SECS_TO_QTIME(sec));
    emit valueChanged();
}

void TimeRangeEdit::setBeginTime(int sec)
{
    if (sec != beginTime()) {
        m_timeBegin->setTime(SECS_TO_QTIME(sec));
        m_chkFromBegin->setChecked((sec == 0));
    }
}

void TimeRangeEdit::setEndTime(int sec)
{
    if (sec != endTime()) {
        m_timeEnd->setTime(SECS_TO_QTIME(sec));
        m_chkToEnd->setChecked((sec == QTIME_TO_SECS(m_timeEnd->maximumTime())));
    }
}

void TimeRangeEdit::setFromBegin(bool from_begin)
{
    m_chkFromBegin->setChecked(from_begin);
}

void TimeRangeEdit::setToEnd(bool to_end)
{
    m_chkToEnd->setChecked(to_end);
}

void TimeRangeEdit::time_changed()
{
    // the latest allowed begin time is end time
    QTime begin_max = m_timeEnd->time().addSecs(-1);
    if (m_chkToEnd->isChecked())
        begin_max = m_timeEnd->maximumTime(); // end time is media duration

    // the earliest allowed end time is begin time
    QTime end_min = m_timeBegin->time().addSecs(1);
    if (m_chkFromBegin->isChecked())
        end_min = SECS_TO_QTIME(0); // begin time is zero

    m_timeBegin->setMaximumTime(begin_max);
    m_timeEnd->setMinimumTime(end_min);
    m_timeBegin->setDisabled(m_chkFromBegin->isChecked());
    m_timeEnd->setDisabled(m_chkToEnd->isChecked());
    emit valueChanged();
}
