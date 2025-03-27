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

#include "widgets/rudder_widget.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>

RudderWidget::RudderWidget(int width, int height, QWidget* parent)
    : QWidget(parent),
      pos(0.0),
      bgColor(Qt::black),
      axisColor(QColor(64, 64, 64)),
      barColor(QColor(0, 128, 255)),
      centerMarkerColor(QColor(255, 0, 0)),
      showValue(true)
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

void RudderWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Calculate position as normalized value [0,1]
    double p = (pos + 1.0) / 2.0;
    
    int w = width() - 10;
    int h = height() - 10;
    
    painter.translate(5, 5);
    
    // Create a path for the rudder frame with rounded corners
    QPainterPath rectPath;
    rectPath.addRoundedRect(0, 0, w, h, 4, 4);
    
    // Draw rudder frame
    painter.setPen(axisColor);
    painter.drawPath(rectPath);
    
    // Draw center line
    painter.setPen(QPen(axisColor, 1.0));
    painter.drawLine(w/2, 0, w/2, h);
    
    // Draw position indicator bar
    int barPos = static_cast<int>(w * p);
    
    // Create gradient fill for bar
    QLinearGradient gradient(0, 0, w, 0);
    gradient.setColorAt(0, QColor(0, 0, 192));
    gradient.setColorAt(0.5, QColor(0, 128, 255));
    gradient.setColorAt(1.0, QColor(0, 0, 192));
    
    // Draw the bar
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    
    QPainterPath barPath;
    barPath.addRoundedRect(0, 0, barPos, h, 2, 2);
    painter.drawPath(barPath);
    
    // Draw center marker
    painter.setPen(QPen(centerMarkerColor, 2.0));
    painter.drawLine(w/2, 0, w/2, h);
    
    // Draw value as text if enabled
    if (showValue) {
        QString valueText = QString::number(pos, 'f', 2);
        
        // Use system font for better scaling
        QFont font = painter.font();
        font.setPointSize(9);
        painter.setFont(font);
        
        QFontMetrics fm = painter.fontMetrics();
        QRect textRect = fm.boundingRect(valueText);
        
        // Position text at the bottom center
        int textX = (w - textRect.width()) / 2;
        int textY = h - fm.descent();
        
        // Draw background for text
        QRect bgRect = textRect.adjusted(-2, -2, 2, 2);
        bgRect.moveCenter(QPoint(w/2, textY - textRect.height()/2));
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 180));
        painter.drawRoundedRect(bgRect, 3, 3);
        
        // Draw text
        painter.setPen(Qt::white);
        painter.drawText(textX, textY, valueText);
    }
}

void RudderWidget::setPos(double p)
{
    // Clamp value to [-1, 1] range
    p = std::max(-1.0, std::min(1.0, p));
    
    if (pos != p) {
        pos = p;
        update();
    }
}

void RudderWidget::setShowValue(bool show)
{
    if (showValue != show) {
        showValue = show;
        update();
    }
}

// Set custom colors
void RudderWidget::setBarColor(const QColor& color)
{
    barColor = color;
    update();
}

void RudderWidget::setAxisColor(const QColor& color)
{
    axisColor = color;
    update();
}

void RudderWidget::setCenterMarkerColor(const QColor& color)
{
    centerMarkerColor = color;
    update();
}

void RudderWidget::setBackgroundColor(const QColor& color)
{
    bgColor = color;
    
    QPalette pal = palette();
    pal.setColor(QPalette::Window, bgColor);
    setPalette(pal);
    
    update();
}
