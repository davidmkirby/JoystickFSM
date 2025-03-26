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

#ifndef EVDEV_HELPER_H
#define EVDEV_HELPER_H

#include <string>

// Define bit manipulation macros for working with input devices
#ifndef BITS_PER_LONG
#define BITS_PER_LONG (sizeof(long) * 8)
#endif

#ifndef NBITS
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#endif

#ifndef NLONGS
#define NLONGS(x) (((x) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#endif

#ifndef BIT_WORD
#define BIT_WORD(nr) ((nr) / BITS_PER_LONG)
#endif

#ifndef BIT_MASK
#define BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))
#endif

/**
 * Convert an event name to a type/code pair
 * 
 * @param name Event name (e.g. "BTN_A", "ABS_X")
 * @param type Event type (e.g. EV_ABS, EV_KEY)
 * @param code Event code (e.g. ABS_X, BTN_A)
 * @return true if conversion was successful, false otherwise
 */
bool str2event(const std::string& name, int& type, int& code);

/**
 * Convert a button code to a string
 * 
 * @param code Button code
 * @return Button name (e.g. "BTN_A")
 */
std::string btn2str(int code);

/**
 * Convert an absolute axis code to a string
 * 
 * @param code Absolute axis code
 * @return Absolute axis name (e.g. "ABS_X")
 */
std::string abs2str(int code);

/**
 * Convert a relative axis code to a string
 * 
 * @param code Relative axis code
 * @return Relative axis name (e.g. "REL_X")
 */
std::string rel2str(int code);

/**
 * Convert a Linux input key code to a Qt key code
 * 
 * @param code Linux input key code
 * @return Qt key code equivalent
 */
int linux_key_to_qt_key(int code);

/**
 * Convert a Qt key code to a Linux input key code
 * 
 * @param code Qt key code
 * @return Linux input key code equivalent
 */
int qt_key_to_linux_key(int code);

/**
 * Convert a keysym string to a keycode
 * 
 * @param name Keysym name
 * @return Keycode value, or -1 if conversion failed
 */
int qt_keysym2keycode(const std::string& name);

#endif // EVDEV_HELPER_H
