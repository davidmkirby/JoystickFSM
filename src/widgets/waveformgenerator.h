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

#ifndef WAVEFORM_GENERATOR_H
#define WAVEFORM_GENERATOR_H

#include <cmath>
#include <vector>

// Waveform style enumeration
enum WaveformStyle {
    SineWave = 0,
    TriangleWave = 1,
    SquareWave = 2,
    SawtoothWave = 3,
    RampWave = 4,
    NoiseWave = 5
};

// Parameter structure for waveform generation
struct WaveformParameter {
    WaveformStyle Style;     // Wave type
    double HighLevel;        // High level value
    double LowLevel;         // Low level value
    double Frequency;        // Frequency in Hz
    double PhaseOffset;      // Phase offset in degrees
    double DutyCycle;        // Duty cycle for square waves (0.0 to 1.0)
};

class WaveformGenerator
{
private:
    int m_pointsCount;       // Number of points in a complete waveform
    std::vector<double> m_noiseBuffer; // Pre-generated noise values
    
public:
    WaveformGenerator(int pointsCount = 400)
        : m_pointsCount(pointsCount)
    {
        // Generate random noise values
        m_noiseBuffer.resize(pointsCount);
        generateNoiseBuffer();
    }
    
    // Regenerate noise buffer with new random values
    void generateNoiseBuffer() {
        for (int i = 0; i < m_pointsCount; i++) {
            m_noiseBuffer[i] = (double)rand() / RAND_MAX;
        }
    }
    
    // Get a single point in the waveform
    double GetOnePoint(WaveformStyle style, int pointIndex, double highLevel, double lowLevel, 
                       double dutyCycle = 0.5, double phaseOffset = 0.0) {
        
        if (pointIndex < 0) {
            pointIndex = 0;
        }
        
        // Ensure index is within range
        pointIndex = pointIndex % m_pointsCount;
        
        // Normalize point index to [0,1] range
        double normalizedPosition = (double)pointIndex / m_pointsCount;
        
        // Apply phase offset (converting from degrees to [0,1] range)
        normalizedPosition += phaseOffset / 360.0;
        normalizedPosition = normalizedPosition - floor(normalizedPosition);
        
        double range = highLevel - lowLevel;
        double result = 0.0;
        
        switch (style) {
            case SineWave:
                // Sine wave: value = amplitude * sin(2π * position) + offset
                result = lowLevel + range * (0.5 + 0.5 * sin(2.0 * M_PI * normalizedPosition));
                break;
                
            case TriangleWave:
                // Triangle wave
                if (normalizedPosition < 0.5) {
                    // Rising part
                    result = lowLevel + range * (2.0 * normalizedPosition);
                } else {
                    // Falling part
                    result = lowLevel + range * (2.0 - 2.0 * normalizedPosition);
                }
                break;
                
            case SquareWave:
                // Square wave with duty cycle
                result = (normalizedPosition < dutyCycle) ? highLevel : lowLevel;
                break;
                
            case SawtoothWave:
                // Sawtooth wave
                result = lowLevel + range * normalizedPosition;
                break;
                
            case RampWave:
                // Ramp wave (inverse sawtooth)
                result = highLevel - range * normalizedPosition;
                break;
                
            case NoiseWave:
                // Random noise (use pre-generated values)
                result = lowLevel + range * m_noiseBuffer[pointIndex];
                break;
                
            default:
                // Default to sine wave
                result = lowLevel + range * (0.5 + 0.5 * sin(2.0 * M_PI * normalizedPosition));
                break;
        }
        
        return result;
    }
    
    // Generate a complete waveform
    void GenerateWaveform(WaveformStyle style, double* buffer, int bufferSize, 
                         double highLevel, double lowLevel, double dutyCycle = 0.5, 
                         double phaseOffset = 0.0) {
        
        if (!buffer || bufferSize <= 0) {
            return;
        }
        
        // Calculate scaling factor if buffer size doesn't match points count
        double scaleFactor = (double)m_pointsCount / bufferSize;
        
        // Generate each point in the buffer
        for (int i = 0; i < bufferSize; i++) {
            int scaledIndex = (int)(i * scaleFactor);
            buffer[i] = GetOnePoint(style, scaledIndex, highLevel, lowLevel, dutyCycle, phaseOffset);
        }
    }
};

#endif // WAVEFORM_GENERATOR_H
