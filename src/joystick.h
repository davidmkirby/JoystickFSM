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

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>
#include <QSocketNotifier>
#include <QString>
#include <vector>
#include <linux/joystick.h>

#include "joystick_description.h"

/**
 * Class that represents a joystick device and provides access to its state
 */
class Joystick : public QObject
{
    Q_OBJECT

public:
    /**
     * Structure containing calibration data for a joystick axis
     */
    struct CalibrationData {
        bool calibrate;   // Whether calibration is enabled
        bool invert;      // Whether to invert the axis
        int center_min;   // Minimum value of the center deadzone
        int center_max;   // Maximum value of the center deadzone
        int range_min;    // Minimum value of the range
        int range_max;    // Maximum value of the range
    };

protected:
    int fd;                 // File descriptor for the joystick device

    std::string filename;   // Path to the device file
    std::string orig_name;  // Original name from the device
    QString name;           // Human-readable name of the joystick
    int axis_count;         // Number of axes
    int button_count;       // Number of buttons

    std::vector<int> axis_state;  // Current state of each axis
    std::vector<CalibrationData> orig_calibration_data;  // Original calibration data

    QSocketNotifier* notifier;  // Socket notifier for monitoring joystick events

public:
    /**
     * Constructor
     * @param filename Path to the joystick device
     */
    Joystick(const std::string& filename);
    
    /**
     * Destructor
     */
    virtual ~Joystick();

    /**
     * Get the file descriptor for the joystick
     * @return File descriptor
     */
    virtual int getFd() const { return fd; }

    /**
     * Update joystick state by reading events from the device
     */
    virtual void update();

    /**
     * Get the path to the joystick device
     * @return Device path
     */
    virtual std::string getFilename() const { return filename; }
    
    /**
     * Get the human-readable name of the joystick
     * @return Joystick name
     */
    virtual QString getName() const { return name; }
    
    /**
     * Get the number of axes on the joystick
     * @return Number of axes
     */
    virtual int getAxisCount() const { return axis_count; }
    
    /**
     * Get the number of buttons on the joystick
     * @return Number of buttons
     */
    virtual int getButtonCount() const { return button_count; }

    /**
     * Get the current state of an axis
     * @param id Axis ID
     * @return Current value of the axis (-32767 to 32767)
     */
    virtual int getAxisState(int id);

    /**
     * Get a list of available joysticks
     * @return List of joystick descriptions
     */
    static std::vector<JoystickDescription> getJoysticks();

    /**
     * Get the current calibration data for all axes
     * @return Vector of calibration data
     */
    virtual std::vector<CalibrationData> getCalibration();
    
    /**
     * Set the calibration data for all axes
     * @param data Vector of calibration data
     */
    virtual void setCalibration(const std::vector<CalibrationData>& data);
    
    /**
     * Reset to the original calibration data
     */
    virtual void resetCalibration();

    /**
     * Clear all calibration data (raw input)
     */
    virtual void clearCalibration();

    /**
     * Get the current button mapping
     * @return Vector of button mappings
     */
    virtual std::vector<int> getButtonMapping();
    
    /**
     * Get the current axis mapping
     * @return Vector of axis mappings
     */
    virtual std::vector<int> getAxisMapping();

    /**
     * Set the button mapping
     * @param mapping Vector of button mappings
     */
    virtual void setButtonMapping(const std::vector<int>& mapping);
    
    /**
     * Set the axis mapping
     * @param mapping Vector of axis mappings
     */
    virtual void setAxisMapping(const std::vector<int>& mapping);

    /**
     * Correct calibration data after remapping axes
     * @param mapping_old Old axis mapping
     * @param mapping_new New axis mapping
     */
    virtual void correctCalibration(const std::vector<int>& mapping_old, const std::vector<int>& mapping_new);

    /**
     * Get the evdev device path for this joystick
     * @return Evdev device path
     */
    virtual std::string getEvdev() const;

signals:
    /**
     * Signal emitted when an axis value changes
     * @param number Axis number
     * @param value New axis value (-32767 to 32767)
     */
    void axisChanged(int number, int value);
    
    /**
     * Signal emitted when a button state changes
     * @param number Button number
     * @param value New button state (true = pressed, false = released)
     */
    void buttonChanged(int number, bool value);

protected:
    /**
     * Protected constructor for derived classes
     */
    Joystick();
    
private slots:
    /**
     * Slot called when there is activity on the joystick device
     * @param socket File descriptor
     */
    void onSocketActivated(int socket);

private:
    Joystick(const Joystick&) = delete;
    Joystick& operator=(const Joystick&) = delete;
};

#endif // JOYSTICK_H
