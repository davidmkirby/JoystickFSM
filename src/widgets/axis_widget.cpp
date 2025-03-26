/*
**  JoystickFSM - A Qt application for joystick-controlled FSM
**  
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "widgets/axis_widget.h"

#include <QPainter>
#include <QPainterPath>
#include <sstream>
#include <iomanip>
#include <cmath>

AxisWidget::AxisWidget(int width, int height, bool show_values_, QWidget* parent)
    : QWidget(parent),
      x(0), 
      y(0), 
      raw_x(0), 
      raw_y(0), 
      show_values(show_values_),
      show_deadzone(true),
      deadzone(0.05),
      show_limits(true),
      limit(1.0),
      bgColor(Qt::black),
      gridColor(QColor(100, 100, 100)),
      cursorColor(Qt::green),
      deadzoneColor(QColor(64, 64, 255, 128)),
      limitColor(QColor(255, 64, 64, 128)),
      cursorSize(5)
{
    setFixedSize(width, height);
    
    // Set widget attributes for better rendering
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAttribute(Qt::WA_TranslucentBackground, false);
    
    // Use a proper size policy
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    // Set black background
    QPalette pal = palette();
    pal.setColor(QPalette::Window, bgColor);
    setAutoFillBackground(true);
    setPalette(pal);
}

void AxisWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int w = width() - 10;
    int h = height() - 10;
    int px = w/2 + (w/2 * x);
    int py = h/2 + (h/2 * y);
    
    painter.translate(5, 5);
    
    // Outer Rectangle - use QPainterPath for better rendering
    QPainterPath rectPath;
    rectPath.addRect(0, 0, w, h);
    painter.setPen(gridColor);
    painter.drawPath(rectPath);
    
    // Outer limit circle - if enabled
    if (show_limits && limit < 1.0) {
        int limitRadius = qMin(w, h) / 2 * limit;
        painter.setPen(Qt::NoPen);
        painter.setBrush(limitColor);
        painter.drawEllipse(QPoint(w/2, h/2), limitRadius, limitRadius);
    }
    
    // Background Circle - use QPainterPath for better rendering
    QPainterPath circlePath;
    int circleRadius = qMin(w, h) / 2;
    circlePath.addEllipse(w/2 - circleRadius, h/2 - circleRadius, 
                          circleRadius * 2, circleRadius * 2);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(40, 40, 40)));
    painter.drawPath(circlePath);
    
    // Deadzone circle - if enabled
    if (show_deadzone && deadzone > 0.0) {
        int dzRadius = qMin(w, h) / 2 * deadzone;
        painter.setPen(Qt::NoPen);
        painter.setBrush(deadzoneColor);
        painter.drawEllipse(QPoint(w/2, h/2), dzRadius, dzRadius);
    }
    
    // Cross - keep simple line drawing for these thin lines
    painter.setPen(QPen(gridColor, 1));
    painter.drawLine(w/2, 0, w/2, h);
    painter.drawLine(0, h/2, w, h/2);
    
    // Cursor
    painter.setPen(QPen(cursorColor, 2.0));
    painter.setBrush(cursorColor);
    
    // Draw a crosshair at the current position
    painter.drawEllipse(QPoint(px, py), cursorSize, cursorSize);
    painter.drawLine(px - cursorSize*2, py, px - cursorSize, py);
    painter.drawLine(px + cursorSize, py, px + cursorSize*2, py);
    painter.drawLine(px, py - cursorSize*2, px, py - cursorSize);
    painter.drawLine(px, py + cursorSize, px, py + cursorSize*2);
    
    // Display values if enabled
    if (show_values) {
        std::ostringstream value_text;
        value_text << "X: " << std::setw(6) << raw_x << " Y: " << std::setw(6) << raw_y;
        
        // Use monospace font for alignment
        QFont monospaceFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        monospaceFont.setPointSize(9);
        painter.setFont(monospaceFont);
        
        QFontMetrics fm(monospaceFont);
        QString valueString = QString::fromStdString(value_text.str());
        QRect textRect = fm.boundingRect(valueString);
        
        // Text background for better readability
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(0, 0, 0, 200)));
        
        // Use QPainterPath for text background
        QPainterPath textBgPath;
        textBgPath.addRect(w/2 - textRect.width()/2 - 2, 
                         h - textRect.height() - 4,
                         textRect.width() + 4,
                         textRect.height() + 2);
        painter.drawPath(textBgPath);
        
        // Draw text
        painter.setPen(Qt::white);
        painter.drawText(QPointF(w/2 - textRect.width()/2, h - 4),
                         valueString);
        
        // Also display normalized values
        std::ostringstream norm_text;
        norm_text << "X: " << std::fixed << std::setprecision(2) << x 
                 << " Y: " << std::fixed << std::setprecision(2) << y;
        
        QString normString = QString::fromStdString(norm_text.str());
        QRect normRect = fm.boundingRect(normString);
        
        // Second text background
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(0, 0, 0, 200)));
        
        QPainterPath normBgPath;
        normBgPath.addRect(w/2 - normRect.width()/2 - 2, 
                         4,
                         normRect.width() + 4,
                         normRect.height() + 2);
        painter.drawPath(normBgPath);
        
        // Draw normalized text at the top
        painter.setPen(Qt::white);
        painter.drawText(QPointF(w/2 - normRect.width()/2, normRect.height() + 4),
                         normString);
    }
}

void AxisWidget::setXAxis(double x_)
{
    x = std::max(-1.0, std::min(1.0, x_));
    update();
}

void AxisWidget::setYAxis(double y_)
{
    y = std::max(-1.0, std::min(1.0, y_));
    update();
}

void AxisWidget::setRawX(int raw_x_value)
{
    raw_x = raw_x_value;
    x = raw_x_value / 32767.0;
    update();
}

void AxisWidget::setRawY(int raw_y_value)
{
    raw_y = raw_y_value;
    y = raw_y_value / 32767.0;
    update();
}

void AxisWidget::setShowValues(bool show)
{
    show_values = show;
    update();
}
