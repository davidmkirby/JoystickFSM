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

#include "widgets/throttle_widget.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>

ThrottleWidget::ThrottleWidget(int width, int height, bool invert_, QWidget* parent)
    : QWidget(parent),
      invert(invert_),
      pos(0.0),
      bgColor(Qt::black),
      frameColor(QColor(64, 64, 64)),
      fillColor(QColor(0, 192, 0)),
      textColor(Qt::white),
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

void ThrottleWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Calculate fill level based on position and inversion setting
    double p = invert ? (1.0 - ((pos + 1.0) / 2.0)) : ((pos + 1.0) / 2.0);
    
    int w = width() - 10;
    int h = height() - 10;
    
    painter.translate(5, 5);
    
    // Create a path for the throttle frame with rounded corners
    QPainterPath framePath;
    framePath.addRoundedRect(0, 0, w, h, 4, 4);
    
    // Draw throttle frame
    painter.setPen(frameColor);
    painter.drawPath(framePath);
    
    // Calculate fill height
    int fillHeight = static_cast<int>(p * h);
    
    // Create fill gradient
    QLinearGradient gradient(0, h, 0, h - fillHeight);
    gradient.setColorAt(0, QColor(0, 192, 0));   // Green at bottom
    gradient.setColorAt(0.5, QColor(192, 192, 0)); // Yellow in middle
    gradient.setColorAt(1.0, QColor(192, 0, 0));  // Red at top
    
    // Draw the fill
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    
    QPainterPath fillPath;
    fillPath.addRoundedRect(0, h - fillHeight, w, fillHeight, 3, 3);
    painter.drawPath(fillPath);
    
    // Draw tick marks
    painter.setPen(QPen(frameColor, 1.0));
    for (int i = 1; i < 5; i++) {
        int y = h * i / 5;
        painter.drawLine(0, y, w / 6, y);      // Left side
        painter.drawLine(w - w / 6, y, w, y);  // Right side
    }
    
    // Draw value as text if enabled
    if (showValue) {
        QString valueText = QString::number(pos, 'f', 2);
        
        // Use system font for better scaling
        QFont font = painter.font();
        font.setPointSize(9);
        painter.setFont(font);
        
        QFontMetrics fm = painter.fontMetrics();
        QRect textRect = fm.boundingRect(valueText);
        
        // Position text at the middle
        int textX = (w - textRect.width()) / 2;
        int textY = h / 2 + fm.height() / 4;
        
        // Draw background for text
        QRect bgRect = textRect.adjusted(-4, -2, 4, 2);
        bgRect.moveCenter(QPoint(w/2, h/2));
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 180));
        painter.drawRoundedRect(bgRect, 3, 3);
        
        // Draw text
        painter.setPen(textColor);
        painter.drawText(textX, textY, valueText);
    }
}

void ThrottleWidget::setPos(double p)
{
    // Clamp value to [-1, 1] range
    p = std::max(-1.0, std::min(1.0, p));
    
    if (pos != p) {
        pos = p;
        update();
    }
}

void ThrottleWidget::setInvert(bool i)
{
    if (invert != i) {
        invert = i;
        update();
    }
}

void ThrottleWidget::setShowValue(bool show)
{
    if (showValue != show) {
        showValue = show;
        update();
    }
}

// Set custom colors
void ThrottleWidget::setFillColor(const QColor& color)
{
    fillColor = color;
    update();
}

void ThrottleWidget::setFrameColor(const QColor& color)
{
    frameColor = color;
    update();
}

void ThrottleWidget::setTextColor(const QColor& color)
{
    textColor = color;
    update();
}

void ThrottleWidget::setBackgroundColor(const QColor& color)
{
    bgColor = color;
    
    QPalette pal = palette();
    pal.setColor(QPalette::Window, bgColor);
    setPalette(pal);
    
    update();
}
