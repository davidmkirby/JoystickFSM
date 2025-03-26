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

#include "widgets/button_widget.h"

#include <QPainter>
#include <QPainterPath>

ButtonWidget::ButtonWidget(int width, int height, const QString& name_, QWidget* parent)
    : QWidget(parent),
      name(name_),
      down(false),
      upColor(Qt::transparent),
      downColor(Qt::green),
      textColor(Qt::black),
      textDownColor(Qt::white),
      borderWidth(2)
{
    setFixedSize(width, height);
    
    // Set widget attributes for better rendering
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAttribute(Qt::WA_TranslucentBackground, false);
    
    // Use a proper size policy
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void ButtonWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int padding = 2;
    int w = width() - 2 * padding;
    int h = height() - 2 * padding;
    
    painter.translate(padding, padding);
    
    // Create a path for the button rectangle with rounded corners
    QPainterPath rectPath;
    rectPath.addRoundedRect(0, 0, w, h, 4, 4);
    
    // Draw button outline
    painter.setPen(QPen(down ? downColor : Qt::black, borderWidth));
    
    // Fill based on state
    if (down) {
        painter.setBrush(QBrush(downColor));
    } else {
        painter.setBrush(QBrush(upColor));
    }
    
    painter.drawPath(rectPath);
    
    // Set text color based on button state
    painter.setPen(down ? textDownColor : textColor);
    
    // Use system font for better scaling on HiDPI displays
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    
    // Center the text
    QFontMetrics fm = painter.fontMetrics();
    QRect textRect = fm.boundingRect(name);
    int textX = (w - textRect.width()) / 2;
    int textY = (h + textRect.height()) / 2 - fm.descent();
    
    painter.drawText(textX, textY, name);
}

void ButtonWidget::setDown(bool pressed)
{
    if (down != pressed) {
        down = pressed;
        update();
    }
}
