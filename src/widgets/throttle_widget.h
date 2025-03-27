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

#ifndef THROTTLE_WIDGET_H
#define THROTTLE_WIDGET_H

#include <QWidget>
#include <QColor>

/**
 * Widget that displays a vertical throttle/slider control
 * Used to visualize throttle-like joystick controls or other vertical axes
 */
class ThrottleWidget : public QWidget
{
    Q_OBJECT

private:
    bool invert;        // Whether to invert the display (1.0 at bottom, -1.0 at top)
    double pos;         // Position value: -1.0 to 1.0
    QColor bgColor;     // Background color
    QColor frameColor;  // Frame color
    QColor fillColor;   // Fill color
    QColor textColor;   // Text color
    bool showValue;     // Whether to show the value as text

public:
    /**
     * Constructor
     * @param width Widget width
     * @param height Widget height
     * @param invert Whether to invert the display
     * @param parent Parent widget
     */
    ThrottleWidget(int width, int height, bool invert = false, QWidget* parent = nullptr);

    /**
     * Paint event handler
     * @param event Paint event
     */
    void paintEvent(QPaintEvent* event) override;
    
    /**
     * Set inversion state
     * @param i True to invert (1.0 at bottom, -1.0 at top)
     */
    void setInvert(bool i);
    
    /**
     * Set whether to show the numeric value
     * @param show True to show value
     */
    void setShowValue(bool show);
    
    /**
     * Set the fill color
     * @param color New color
     */
    void setFillColor(const QColor& color);
    
    /**
     * Set the frame color
     * @param color New color
     */
    void setFrameColor(const QColor& color);
    
    /**
     * Set the text color
     * @param color New color
     */
    void setTextColor(const QColor& color);
    
    /**
     * Set the background color
     * @param color New color
     */
    void setBackgroundColor(const QColor& color);

public slots:
    /**
     * Set the position value
     * @param p Position value: -1.0 to 1.0
     */
    void setPos(double p);
};

#endif // THROTTLE_WIDGET_H
