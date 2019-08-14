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

#include <QPainter>
#include <QMouseEvent>
#include "rangeselector.h"
#include "services/constants.h"

// rounded rectangle radius
#define ROUNDRECT_RADIUS Constants::getFloat("RangeSelector/RoundRectRadius")

#define COLOR(name) Constants::getColor("RangeSelector/Colors/" name)
// container color
#define COLOR_CONT_BG_GRAD_1  COLOR("Container/BackgroundGradient1")
#define COLOR_CONT_BG_GRAD_2  COLOR("Container/BackgroundGradient2")
#define COLOR_CONT_OUT_BORDER COLOR("Container/OuterBorder")
#define COLOR_CONT_IN_BORDER  COLOR("Container/InnerBorder")

// range indicator color
#define COLOR_RANGE_BG_GRAD_1  COLOR("Range/BackgroundGradient1")
#define COLOR_RANGE_BG_GRAD_2  COLOR("Range/BackgroundGradient2")
#define COLOR_RANGE_OUT_BORDER COLOR("Range/OuterBorder")
#define COLOR_RANGE_IN_BORDER  COLOR("Range/InnerBorder")

RangeSelector::RangeSelector(QWidget *parent) :
    QWidget(parent), m_max(255), m_min(0), m_val_begin(0), m_val_end(128),
    m_mouseDown(false)
{
    setMinimumSize(20, 20);
    setMaximumHeight(20);
    emit beginValueChanged(m_val_begin);
    emit endValueChanged(m_val_end);
}

RangeSelector::~RangeSelector()
{
}

int RangeSelector::beginValue() const
{
    return m_val_begin;
}

int RangeSelector::endValue() const
{
    return m_val_end;
}

int RangeSelector::minValue() const
{
    return m_min;
}

int RangeSelector::maxValue() const
{
    return m_max;
}

void RangeSelector::setBeginValue(int value)
{
    if (m_val_begin != value
            && m_min <= value && value <= m_max
            && value < m_val_end) {
        m_val_begin = value;
        repaint();
        emit valueChanged();
        emit beginValueChanged(m_val_begin);
    }
}

void RangeSelector::setEndValue(int value)
{
    if (m_val_end != value
            && m_min <= value && value <= m_max
            && value > m_val_begin) {
        m_val_end = value;
        repaint();
        emit valueChanged();
        emit endValueChanged(m_val_end);
    }
}

void RangeSelector::setMaxValue(int maxValue)
{
    if (maxValue > m_min) {
        m_max = maxValue;
        m_val_begin = m_min;
        m_val_end = m_max;
        repaint();
    }
}

void RangeSelector::mouseDown(QPoint pos)
{
    int newValue = pos_to_val(pos.x());
    m_dragEdge = edgeToMove(pos);
    if (m_dragEdge == EDGE_BEGIN)
        setBeginValue(newValue);
    else
        setEndValue(newValue);
}

void RangeSelector::mouseDrag(QPoint newpos)
{
    const int newval = pos_to_val(newpos.x());
    switch (m_dragEdge) {
    case EDGE_BEGIN:
        setBeginValue(newval);
        break;
    case EDGE_END:
        setEndValue(newval);
        break;
    default:
        Q_ASSERT(!"undefined value: m_dragEdge");
    }
}

void RangeSelector::mouseClick(QPoint)
{

}

int RangeSelector::pos_begin()
{
    return width() * m_val_begin / (m_max - m_min);
}

int RangeSelector::pos_end()
{
    return width() * m_val_end / (m_max - m_min);
}

int RangeSelector::pos_to_val(int pos)
{
    if (width() > 0)
        return pos * (m_max - m_min) / width();
    else
        return 0;
}

// which edge to be moved if the user clicks at pos
// returns EDGE_BEGIN or EDGE_END
RangeSelector::Edge RangeSelector::edgeToMove(const QPoint &pos)
{
    const int newval = pos_to_val(pos.x());
    const int distance_to_begin = abs(newval - m_val_begin);
    const int distance_to_end = abs(newval - m_val_end);
    if (distance_to_begin < distance_to_end)
        return EDGE_BEGIN; // pos is closer to the begin edge
    else
        return EDGE_END;   // pos is closer to the end edge
}

void RangeSelector::drawContainer(QPainter &painter, QPen &pen)
{
    // background
    QRect background_region(0, 0, width(), height());
    QLinearGradient background_gradient(0, 0, 0, background_region.bottom());
    background_gradient.setColorAt(0.0, COLOR_CONT_BG_GRAD_1);
    background_gradient.setColorAt(1.0, COLOR_CONT_BG_GRAD_2);
    painter.setBrush(background_gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(background_region, ROUNDRECT_RADIUS, ROUNDRECT_RADIUS);

    // outer border
    QRect outer_border_region(0, 0, width()-1, height()-1);
    pen.setColor(COLOR_CONT_OUT_BORDER);
    painter.setPen(pen);
    painter.drawRoundedRect(outer_border_region, ROUNDRECT_RADIUS, ROUNDRECT_RADIUS);

    // inner border
    QRect inner_border_region(1, 1, width()-3, height()-3);
    pen.setColor(COLOR_CONT_IN_BORDER);
    painter.setPen(pen);
    painter.drawRoundedRect(inner_border_region, ROUNDRECT_RADIUS, ROUNDRECT_RADIUS);

}

void RangeSelector::drawRange(QPainter &painter, QPen &pen)
{
    const int begin = pos_begin(), end = pos_end();

    // background
    QRect background_region(begin, 0, end - begin - 1, height() - 1);
    QLinearGradient background_gradient(0, 0, 0, background_region.bottom());
    background_gradient.setColorAt(0.0, COLOR_RANGE_BG_GRAD_1);
    background_gradient.setColorAt(1.0, COLOR_RANGE_BG_GRAD_2);
    painter.setBrush(background_gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(background_region, ROUNDRECT_RADIUS, ROUNDRECT_RADIUS);

    // outer border
    QRect outer_border_region(begin, 0, end-begin-1, height()-1);
    pen.setColor(COLOR_RANGE_OUT_BORDER);
    painter.setPen(pen);
    painter.drawRoundedRect(outer_border_region, ROUNDRECT_RADIUS, ROUNDRECT_RADIUS);

    // inner border
    QRect inner_border_region(begin+1, 1, end-begin-3, height()-3);
    pen.setColor(COLOR_RANGE_IN_BORDER);
    painter.setPen(pen);
    painter.drawRoundedRect(inner_border_region, ROUNDRECT_RADIUS, ROUNDRECT_RADIUS);
}

void RangeSelector::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen;
    drawContainer(painter, pen);
    drawRange(painter, pen);
}

void RangeSelector::mousePressEvent(QMouseEvent *e)
{
    m_mouseDownPos = e->pos();
    m_mouseDown = true;
    mouseDown(m_mouseDownPos);
}

void RangeSelector::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_mouseDown && m_mouseDownPos == e->pos()) {
        mouseClick(m_mouseDownPos); // click event
    }
    m_mouseDown = false;
}

void RangeSelector::mouseMoveEvent(QMouseEvent *e)
{
    if (m_mouseDown) {
        mouseDrag(e->pos());
    }
}
