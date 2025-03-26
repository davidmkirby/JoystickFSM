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

#ifndef AXIS_WIDGET_H
#define AXIS_WIDGET_H

#include <QWidget>
#include <QFontDatabase>

class AxisWidget : public QWidget
{
    Q_OBJECT

private:
    double x;          // Normalized X value (-1.0 to 1.0)
    double y;          // Normalized Y value (-1.0 to 1.0)
    int raw_x;         // Raw X value from joystick (-32767 to 32767)
    int raw_y;         // Raw Y value from joystick (-32767 to 32767)
    bool show_values;  // Whether to display raw values
    bool show_deadzone;// Whether to display deadzone
    double deadzone;   // Deadzone radius (0.0 to 1.0)
    bool show_limits;  // Whether to display outer limits
    double limit;      // Outer limit radius (0.0 to 1.0)
    
    // Visual settings
    QColor bgColor;
    QColor gridColor;
    QColor cursorColor;
    QColor deadzoneColor;
    QColor limitColor;
    int cursorSize;

public:
    AxisWidget(int width, int height, bool show_values = true, QWidget* parent = nullptr);
    
    // Set the deadzone circle
    void setDeadzone(double value) { deadzone = value; update(); }
    bool isShowingDeadzone() const { return show_deadzone; }
    void setShowDeadzone(bool show) { show_deadzone = show; update(); }
    
    // Set the outer limit circle
    void setLimit(double value) { limit = value; update(); }
    bool isShowingLimits() const { return show_limits; }
    void setShowLimits(bool show) { show_limits = show; update(); }

protected:
    void paintEvent(QPaintEvent* event) override;

public slots:
    void setXAxis(double x);      // Set normalized X-axis value (-1.0 to 1.0)
    void setYAxis(double y);      // Set normalized Y-axis value (-1.0 to 1.0)
    void setRawX(int raw_x_value); // Set raw X-axis value (-32767 to 32767)
    void setRawY(int raw_y_value); // Set raw Y-axis value (-32767 to 32767)
    void setShowValues(bool show); // Set whether to show values
};

#endif // AXIS_WIDGET_H
