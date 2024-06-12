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

#include <QHBoxLayout>
#include <QPainter>
#include <QDebug>
#include "progressbar.h"
#include "services/constants.h"

ProgressBar::ProgressBar(QWidget *parent) :
    QWidget(parent), m_percentage(0), m_active(false), m_show_text(false)
{
    //qDebug() << "ProgressBar Constructor";
}

ProgressBar::~ProgressBar()
{
    //qDebug() << "ProgressBar Destructor";
}

void ProgressBar::setValue(unsigned int value)
{
    if (value != m_percentage) {
        m_percentage = (value <= 100) ? value : 100;
        this->repaint();
    }
    m_show_text = false;
}

void ProgressBar::showText(const QString &str)
{
    m_show_text = true;
    m_text = str;
    this->repaint();
}

void ProgressBar::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        this->repaint();
    }
}

bool ProgressBar::isActive() const
{
    return m_active;
}

void ProgressBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QPen pen;
    QColor color_margin = Constants::getColor("ProgressBar/Colors/Margin");
    QColor color_center = Constants::getColor("ProgressBar/Colors/Center");
    QColor color_border = Constants::getColor("ProgressBar/Colors/Border");
    QColor color_text = Constants::getColor("ProgressBar/Colors/Text");
    QColor color_background = Constants::getColor("ProgressBar/Colors/Background");

    //if (m_percentage >= 0)
    {
        QRect rect_region(0, 0, width()-1, height()-1);

        // draw background
        QBrush background_brush(color_background);
        painter.setPen(Qt::NoPen);
        painter.setBrush(background_brush);
        painter.drawRect(rect_region);

        if (m_percentage > 0) {
            // draw progress bar
            QRect rect_progress(0, 0, (width()*m_percentage/100)-1, height()-1);
            QLinearGradient gradient(0, 0, 0, rect_progress.bottom());
            gradient.setColorAt(0, color_margin);
            gradient.setColorAt(0.5, color_center);
            gradient.setColorAt(1, color_margin);
            painter.setBrush(gradient);
            painter.setPen(Qt::NoPen);  // Don't draw the border.
            painter.drawRect(rect_progress);
        }

        // Restore the pen such that the text can be rendered.
        pen.setColor(color_text);
        painter.setPen(pen);

        if (!m_show_text) { // show percentage
            painter.drawText(rect_region,
                             QString::fromLatin1("%1%").arg(m_percentage),
                             QTextOption(Qt::AlignCenter));
        } else { // show custom text
            painter.drawText(rect_region, m_text, QTextOption(Qt::AlignCenter));
        }

        if (m_active) {
            pen.setColor(color_border);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush); // disable filling
            painter.drawRect(0, 0, width()-1, height()-1);
        }
    }
}
