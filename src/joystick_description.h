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

#ifndef JOYSTICK_DESCRIPTION_H
#define JOYSTICK_DESCRIPTION_H

#include <string>

/**
 * Class that holds information about a joystick device
 * Used when enumerating available joysticks
 */
class JoystickDescription
{
private:
public:
    std::string filename;   // Path to the device file
    std::string name;       // Name of the joystick (e.g., "Logitech Gamepad F310")
    int axis_count;         // Number of axes
    int button_count;       // Number of buttons
    bool has_ff;            // Whether force feedback is supported
    int vendor_id;          // USB vendor ID
    int product_id;         // USB product ID

    JoystickDescription(const std::string& filename_,
                        const std::string& name_,
                        int axis_count_,
                        int button_count_,
                        bool has_ff_ = false,
                        int vendor_id_ = 0,
                        int product_id_ = 0)
        : filename(filename_),
          name(name_),
          axis_count(axis_count_),
          button_count(button_count_),
          has_ff(has_ff_),
          vendor_id(vendor_id_),
          product_id(product_id_)
    {}
};

#endif // JOYSTICK_DESCRIPTION_H
