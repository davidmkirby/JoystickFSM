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

#include "widgets/simplegraph.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QString>
#include <QDebug>
#include <cmath>

// Include the Advantech DAQ typedefs
typedef enum ValueUnit
{
    Voltage = 0,
    Amp,
    Watt,
    Celsius
} ValueUnit;

typedef enum TimeUnit
{
    Nanosecond = 0,
    Microsecond,
    Millisecond,
    Second
} TimeUnit;

// Initialize static colors for graph lines
QColor SimpleGraph::lineColor[16] = {
    Qt::red, Qt::green, Qt::blue, Qt::cyan,
    Qt::magenta, Qt::yellow, Qt::white, QColor(255, 128, 0),  // Orange
    QColor(128, 255, 0), QColor(0, 255, 128), QColor(0, 128, 255), QColor(128, 0, 255),
    QColor(255, 0, 128), QColor(128, 128, 0), QColor(0, 128, 128), QColor(128, 0, 128)
};

SimpleGraph::SimpleGraph(QWidget *parent)
    : QWidget(parent),
      m_xCordTimeDiv(200.0),
      m_xCordTimeOffset(0.0),
      m_yCordRangeMax(10.0),
      m_yCordRangeMin(-10.0),
      m_circleRadius(3),
      m_timeInc(0.001),
      m_channelCount(0),
      m_maxPoints(10000),
      m_backgroundColor(Qt::black),
      m_gridColor(QColor(64, 64, 64)),
      m_showGrid(true),
      m_gridDivisions(10.0)
{
    // Set black background
    QPalette pal = palette();
    pal.setColor(QPalette::Window, m_backgroundColor);
    setAutoFillBackground(true);
    setPalette(pal);
    
    // Initialize empty point arrays
    for (int i = 0; i < 16; i++) {
        m_points[i].clear();
    }
}

SimpleGraph::~SimpleGraph()
{
    // Nothing special to clean up
}

void SimpleGraph::Clear()
{
    // Clear all data points
    for (int i = 0; i < 16; i++) {
        m_points[i].clear();
    }
    
    // Trigger a repaint
    update();
}

void SimpleGraph::ClearChannel(int channel)
{
    if (channel >= 0 && channel < 16) {
        m_points[channel].clear();
        update();
    }
}

void SimpleGraph::AddPoint(int channel, double x, double y)
{
    if (channel >= 0 && channel < 16) {
        // Add new point
        m_points[channel].append(QPointF(x, y));
        
        // Limit number of points to prevent excessive memory usage
        if (m_points[channel].size() > m_maxPoints) {
            m_points[channel].removeFirst();
        }
        
        // Update channel count if needed
        if (channel >= m_channelCount) {
            m_channelCount = channel + 1;
        }
        
        // Trigger a repaint
        update();
    }
}

void SimpleGraph::Chart(const double* data, int channels, int points, double timeInc)
{
    if (!data || channels <= 0 || points <= 0) {
        return;
    }
    
    m_timeInc = timeInc;
    m_channelCount = qMin(channels, 16); // Limit to 16 channels
    
    // Process each channel
    for (int ch = 0; ch < m_channelCount; ch++) {
        // Add new data points
        double timestamp = 0.0;
        for (int i = 0; i < points; i++) {
            m_points[ch].append(QPointF(timestamp, data[i * channels + ch]));
            timestamp += timeInc;
        }
        
        // Limit points to prevent excessive memory usage
        while (m_points[ch].size() > m_maxPoints) {
            m_points[ch].removeFirst();
        }
    }
    
    // Trigger a repaint
    update();
}

void SimpleGraph::GetXCordRange(QString* range, double max, double min, TimeUnit unit)
{
    if (!range) {
        return;
    }
    
    QString unitStr;
    switch (unit) {
        case Nanosecond:
            unitStr = "ns";
            break;
        case Microsecond:
            unitStr = "µs";
            break;
        case Millisecond:
            unitStr = "ms";
            break;
        case Second:
            unitStr = "s";
            break;
        default:
            unitStr = "s";
            break;
    }
    
    range[0] = QString("%1%2").arg(max, 0, 'f', 1).arg(unitStr);
    range[1] = QString("%1%2").arg(min, 0, 'f', 1).arg(unitStr);
}

void SimpleGraph::GetYCordRange(QString* range, double max, double min, ValueUnit unit)
{
    if (!range) {
        return;
    }
    
    QString unitStr;
    switch (unit) {
        case Voltage:
            unitStr = "V";
            break;
        case Amp:
            unitStr = "A";
            break;
        case Watt:
            unitStr = "W";
            break;
        case Celsius:
            unitStr = "°C";
            break;
        default:
            unitStr = "V";
            break;
    }
    
    range[0] = QString("%1%2").arg(max, 0, 'f', 1).arg(unitStr);
    range[1] = QString("%1%2").arg(min, 0, 'f', 1).arg(unitStr);
    range[2] = QString("%1%2").arg((max + min) / 2.0, 0, 'f', 1).arg(unitStr);
}

void SimpleGraph::Div(int value)
{
    m_xCordTimeDiv = value;
    update();
}

double SimpleGraph::valueToY(double value)
{
    double range = m_yCordRangeMax - m_yCordRangeMin;
    if (range <= 0) {
        range = 1.0;
    }
    
    return (value - m_yCordRangeMin) / range;
}

double SimpleGraph::xToPixel(double x)
{
    return x * width() / m_xCordTimeDiv;
}

double SimpleGraph::yToPixel(double y)
{
    return height() * (1.0 - valueToY(y));
}

void SimpleGraph::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}

void SimpleGraph::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(rect(), m_backgroundColor);
    
    // Draw grid
    if (m_showGrid) {
        painter.setPen(QPen(m_gridColor, 1, Qt::DotLine));
        
        // Vertical grid lines
        int numVerticals = 10;
        for (int i = 1; i < numVerticals; i++) {
            int x = width() * i / numVerticals;
            painter.drawLine(x, 0, x, height());
        }
        
        // Horizontal grid lines
        int numHorizontals = 8;
        for (int i = 1; i < numHorizontals; i++) {
            int y = height() * i / numHorizontals;
            painter.drawLine(0, y, width(), y);
        }
        
        // Center lines (thicker)
        painter.setPen(QPen(m_gridColor, 2, Qt::SolidLine));
        painter.drawLine(width() / 2, 0, width() / 2, height());
        painter.drawLine(0, height() / 2, width(), height() / 2);
    }
    
    // Draw data for each channel
    for (int ch = 0; ch < m_channelCount; ch++) {
        if (m_points[ch].isEmpty()) {
            continue;
        }
        
        painter.setPen(QPen(lineColor[ch % 16], 2));
        
        QPainterPath path;
        bool firstPoint = true;
        
        for (const QPointF& point : m_points[ch]) {
            double x = xToPixel(point.x());
            double y = yToPixel(point.y());
            
            // Skip points outside the visible area
            if (x < 0 || x > width() || y < 0 || y > height()) {
                continue;
            }
            
            if (firstPoint) {
                path.moveTo(x, y);
                firstPoint = false;
            } else {
                path.lineTo(x, y);
            }
        }
        
        painter.drawPath(path);
        
        // Draw a circle at the last point
        if (!m_points[ch].isEmpty()) {
            const QPointF& lastPoint = m_points[ch].last();
            double x = xToPixel(lastPoint.x());
            double y = yToPixel(lastPoint.y());
            
            if (x >= 0 && x <= width() && y >= 0 && y <= height()) {
                painter.setBrush(lineColor[ch % 16]);
                painter.drawEllipse(QPointF(x, y), m_circleRadius, m_circleRadius);
            }
        }
    }
}
