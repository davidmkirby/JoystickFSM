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

#ifndef SIMPLEGRAPH_H
#define SIMPLEGRAPH_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QString>

// Forward declaration of Advantech DAQ types
typedef enum ValueUnit ValueUnit;
typedef enum TimeUnit TimeUnit;

class SimpleGraph : public QWidget
{
    Q_OBJECT

public:
    SimpleGraph(QWidget *parent = nullptr);
    ~SimpleGraph();

    void Clear();
    void Chart(const double* data, int channels, int points, double timeInc);
    void GetXCordRange(QString* range, double max, double min, TimeUnit unit);
    void GetYCordRange(QString* range, double max, double min, ValueUnit unit);
    void Div(int value);
    
    // Point tracking for visualization
    void AddPoint(int channel, double x, double y);
    void ClearChannel(int channel);
    
    // Drawing configuration
    double m_xCordTimeDiv;
    double m_xCordTimeOffset;
    double m_yCordRangeMax;
    double m_yCordRangeMin;
    
    // Channel colors
    static QColor lineColor[16];

private:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    
    double valueToY(double value);
    double xToPixel(double x);
    double yToPixel(double y);
    
    int m_circleRadius;
    QVector<QPointF> m_points[16];  // Data points for up to 16 channels
    double m_timeInc;
    int m_channelCount;
    int m_maxPoints;
    QColor m_backgroundColor;
    QColor m_gridColor;
    bool m_showGrid;
    double m_gridDivisions;
};

#endif // SIMPLEGRAPH_H
