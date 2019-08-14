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

#ifndef TIMERANGEEDIT_H
#define TIMERANGEEDIT_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QTimeEdit;
class QCheckBox;
QT_END_NAMESPACE

class TimeRangeEdit : public QWidget
{
    Q_OBJECT
public:
    explicit TimeRangeEdit(QWidget *parent = nullptr);

    int maxTime() const;

    /**
     * @brief begin time
     * @return If fromBegin() is false, this function returns the begin time.
     *       If fromBegin() is true, the return value is 0.
     */
    int beginTime() const;

    /**
     * @brief endTime
     * @return If toEnd() is false, this fuction returns the end time.
     *       If toEnd() is true, the return value is the same as maxTime().
     */
    int endTime() const;

    /**
     * @brief whether the "from begin" checkbox is checked
     */
    bool fromBegin() const;

    /**
     * @brief whether the "to end" checkbox is checked
     */
    bool toEnd() const;

public slots:

    /**
     * @brief Set the maximum duration to @a sec
     * @param sec the maximum duration in seconds
     */
    void setMaxTime(int sec);

    /**
     * @brief Set the begin time to @a sec
     */
    void setBeginTime(int sec);

    /**
     * @brief Set the end time to @a sec
     */
    void setEndTime(int sec);

    void setFromBegin(bool);

    void setToEnd(bool);

signals:
    void valueChanged();

private slots:
    void time_changed();

private:
    QTimeEdit *m_timeBegin;
    QTimeEdit *m_timeEnd;
    QCheckBox *m_chkFromBegin;
    QCheckBox *m_chkToEnd;
};

#endif // TIMERANGEEDIT_H
