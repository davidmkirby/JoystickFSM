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

#ifndef RUDDER_WIDGET_H
#define RUDDER_WIDGET_H

#include <QWidget>
#include <QColor>

/**
 * Widget that displays a horizontal rudder/slider control
 * Used to visualize rudder-like joystick controls or similar horizontal axes
 */
class RudderWidget : public QWidget
{
    Q_OBJECT

private:
    double pos;               // Position value: -1.0 (left) to 1.0 (right)
    QColor bgColor;           // Background color
    QColor axisColor;         // Color of the axis lines
    QColor barColor;          // Color of the position bar
    QColor centerMarkerColor; // Color of the center marker
    bool showValue;           // Whether to display the current value

public:
    /**
     * Constructor
     * @param width Widget width
     * @param height Widget height
     * @param parent Parent widget
     */
    RudderWidget(int width, int height, QWidget* parent = nullptr);

    /**
     * Paint event handler
     * @param event Paint event
     */
    void paintEvent(QPaintEvent* event) override;
    
    /**
     * Set whether to show the numeric value
     * @param show True to show the value
     */
    void setShowValue(bool show);
    
    /**
     * Set the bar color
     * @param color New color
     */
    void setBarColor(const QColor& color);
    
    /**
     * Set the axis line color
     * @param color New color
     */
    void setAxisColor(const QColor& color);
    
    /**
     * Set the center marker color
     * @param color New color
     */
    void setCenterMarkerColor(const QColor& color);
    
    /**
     * Set the background color
     * @param color New color
     */
    void setBackgroundColor(const QColor& color);

public slots:
    /**
     * Set the position
     * @param p Position value: -1.0 (left) to 1.0 (right)
     */
    void setPos(double p);
};

#endif // RUDDER_WIDGET_H
