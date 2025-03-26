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

#ifndef BUTTON_WIDGET_H
#define BUTTON_WIDGET_H

#include <QWidget>
#include <QString>

class ButtonWidget : public QWidget
{
    Q_OBJECT

private:
    QString name;          // Button name/number
    bool down;             // Button state (pressed or not)
    QColor upColor;        // Color when button is up
    QColor downColor;      // Color when button is down
    QColor textColor;      // Text color
    QColor textDownColor;  // Text color when button is down
    int borderWidth;       // Border width in pixels

public:
    ButtonWidget(int width, int height, const QString& name, QWidget* parent = nullptr);

    void paintEvent(QPaintEvent* event) override;
    
    // Set custom colors
    void setUpColor(const QColor& color) { upColor = color; update(); }
    void setDownColor(const QColor& color) { downColor = color; update(); }
    void setTextColor(const QColor& color) { textColor = color; update(); }
    void setTextDownColor(const QColor& color) { textDownColor = color; update(); }
    
    // Getter for button state
    bool isDown() const { return down; }

public slots:
    void setDown(bool pressed);
    void toggle() { setDown(!down); }
};

#endif // BUTTON_WIDGET_H
