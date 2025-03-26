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

#include "utils/evdev_helper.h"

#include <QKeySequence>
#include <QGuiApplication>
#include <linux/input.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <map>

template<class Enum>
class EnumBox
{
protected:
    std::string name;
    std::map<Enum, std::string> enum2string;
    std::map<std::string, Enum> string2enum;

protected:
    EnumBox(const std::string& name)
        : name(name)
    {
    }

    void add(Enum i, const std::string& name)
    {
        enum2string[i] = name;
        string2enum[name] = i;
    }

public:
    Enum operator[](const std::string& str) const
    {
        typename std::map<std::string, Enum>::const_iterator i = string2enum.find(str);
        if (i == string2enum.end())
        {
            std::istringstream in(str);
            Enum tmp;
            in >> tmp;
            if (in.fail())
            {
                std::ostringstream out;
                out << "Couldn't convert '" << str << "' to enum " << name << std::endl;
                throw std::runtime_error(out.str());
            }
            else
            {
                return tmp;
            }
        }
        else
        {
            return i->second;
        }
    }

    std::string operator[](Enum v) const {
        typename std::map<Enum, std::string>::const_iterator i = enum2string.find(v);
        if (i == enum2string.end())
        {
            // If we can't convert symbolic, just convert the integer to a
            // string
            std::ostringstream out;
            out << v;
            return out.str();
        }
        else
        {
            return i->second;
        }
    }
};


class EvDevRelEnum : public EnumBox<int>
{
public:
    EvDevRelEnum()
        : EnumBox<int>("EV_REL")
    {
        add(REL_X,               "REL_X");
        add(REL_Y,               "REL_Y");
        add(REL_Z,               "REL_Z");
        add(REL_RX,              "REL_RX");
        add(REL_RY,              "REL_RY");
        add(REL_RZ,              "REL_RZ");
        add(REL_HWHEEL,          "REL_HWHEEL");
        add(REL_DIAL,            "REL_DIAL");
        add(REL_WHEEL,           "REL_WHEEL");
        add(REL_MISC,            "REL_MISC");
    }
} evdev_rel_names;

class EvDevAbsEnum : public EnumBox<int>
{
public:
    EvDevAbsEnum()
        : EnumBox<int>("EV_ABS")
    {
        add(ABS_X,               "ABS_X");
        add(ABS_Y,               "ABS_Y");
        add(ABS_Z,               "ABS_Z");
        add(ABS_RX,              "ABS_RX");
        add(ABS_RY,              "ABS_RY");
        add(ABS_RZ,              "ABS_RZ");
        add(ABS_THROTTLE,        "ABS_THROTTLE");
        add(ABS_RUDDER,          "ABS_RUDDER");
        add(ABS_WHEEL,           "ABS_WHEEL");
        add(ABS_GAS,             "ABS_GAS");
        add(ABS_BRAKE,           "ABS_BRAKE");
        add(ABS_HAT0X,           "ABS_HAT0X");
        add(ABS_HAT0Y,           "ABS_HAT0Y");
        add(ABS_HAT1X,           "ABS_HAT1X");
        add(ABS_HAT1Y,           "ABS_HAT1Y");
        add(ABS_HAT2X,           "ABS_HAT2X");
        add(ABS_HAT2Y,           "ABS_HAT2Y");
        add(ABS_HAT3X,           "ABS_HAT3X");
        add(ABS_HAT3Y,           "ABS_HAT3Y");
        add(ABS_PRESSURE,        "ABS_PRESSURE");
        add(ABS_DISTANCE,        "ABS_DISTANCE");
        add(ABS_TILT_X,          "ABS_TILT_X");
        add(ABS_TILT_Y,          "ABS_TILT_Y");
        add(ABS_TOOL_WIDTH,      "ABS_TOOL_WIDTH");
        add(ABS_VOLUME,          "ABS_VOLUME");
        add(ABS_MISC,            "ABS_MISC");
    }
} evdev_abs_names;

class EvDevBtnEnum : public EnumBox<int>
{
public:
    EvDevBtnEnum()
        : EnumBox<int>("EV_KEY")
    {
        // Joystick buttons
        add(BTN_JOYSTICK,        "BTN_JOYSTICK");
        add(BTN_TRIGGER,         "BTN_TRIGGER");
        add(BTN_THUMB,           "BTN_THUMB");
        add(BTN_THUMB2,          "BTN_THUMB2");
        add(BTN_TOP,             "BTN_TOP");
        add(BTN_TOP2,            "BTN_TOP2");
        add(BTN_PINKIE,          "BTN_PINKIE");
        add(BTN_BASE,            "BTN_BASE");
        add(BTN_BASE2,           "BTN_BASE2");
        add(BTN_BASE3,           "BTN_BASE3");
        add(BTN_BASE4,           "BTN_BASE4");
        add(BTN_BASE5,           "BTN_BASE5");
        add(BTN_BASE6,           "BTN_BASE6");
        add(BTN_DEAD,            "BTN_DEAD");
        
        // Gamepad buttons
        add(BTN_GAMEPAD,         "BTN_GAMEPAD");
        add(BTN_A,               "BTN_A");
        add(BTN_B,               "BTN_B");
        add(BTN_C,               "BTN_C");
        add(BTN_X,               "BTN_X");
        add(BTN_Y,               "BTN_Y");
        add(BTN_Z,               "BTN_Z");
        add(BTN_TL,              "BTN_TL");
        add(BTN_TR,              "BTN_TR");
        add(BTN_TL2,             "BTN_TL2");
        add(BTN_TR2,             "BTN_TR2");
        add(BTN_SELECT,          "BTN_SELECT");
        add(BTN_START,           "BTN_START");
        add(BTN_MODE,            "BTN_MODE");
        add(BTN_THUMBL,          "BTN_THUMBL");
        add(BTN_THUMBR,          "BTN_THUMBR");
        
        // Generic buttons
        add(BTN_0,               "BTN_0");
        add(BTN_1,               "BTN_1");
        add(BTN_2,               "BTN_2");
        add(BTN_3,               "BTN_3");
        add(BTN_4,               "BTN_4");
        add(BTN_5,               "BTN_5");
        add(BTN_6,               "BTN_6");
        add(BTN_7,               "BTN_7");
        add(BTN_8,               "BTN_8");
        add(BTN_9,               "BTN_9");
    }
} evdev_btn_names;

// Qt-based implementation to replace X11-specific code
class KeycodeMapper {
private:
    std::map<QString, int> keymap;

public:
    KeycodeMapper() {
        // Map common key names to Linux input keycodes
        // This is a simplified mapping and may need expansion
        keymap["space"] = KEY_SPACE;
        keymap["escape"] = KEY_ESC;
        keymap["return"] = KEY_ENTER;
        keymap["tab"] = KEY_TAB;
        keymap["backspace"] = KEY_BACKSPACE;
        keymap["control"] = KEY_LEFTCTRL;
        keymap["shift"] = KEY_LEFTSHIFT;
        keymap["alt"] = KEY_LEFTALT;
        keymap["meta"] = KEY_LEFTMETA;
        
        // Add letter keys
        for (int i = 0; i < 26; i++) {
            QString key = QString(QChar('a' + i));
            keymap[key] = KEY_A + i;
        }
        
        // Add number keys
        for (int i = 0; i < 10; i++) {
            QString key = QString::number(i);
            keymap[key] = KEY_0 + i;
        }
        
        // Add function keys
        for (int i = 1; i <= 12; i++) {
            QString key = QString("f%1").arg(i);
            keymap[key] = KEY_F1 + (i - 1);
        }

        // Add navigation keys
        keymap["up"] = KEY_UP;
        keymap["down"] = KEY_DOWN;
        keymap["left"] = KEY_LEFT;
        keymap["right"] = KEY_RIGHT;
        keymap["home"] = KEY_HOME;
        keymap["end"] = KEY_END;
        keymap["pageup"] = KEY_PAGEUP;
        keymap["pagedown"] = KEY_PAGEDOWN;
        keymap["insert"] = KEY_INSERT;
        keymap["delete"] = KEY_DELETE;
    }

    int getKeycode(const QString& keyName) {
        QString lowerKey = keyName.toLower();
        auto it = keymap.find(lowerKey);
        if (it != keymap.end()) {
            return it->second;
        }
        return -1; // Key not found
    }
};

// Global instance
static KeycodeMapper g_keycodeMapper;

int qt_keysym2keycode(const std::string& name) {
    if (name.length() <= 3) {
        return -1;
    }
    
    QString keyName = QString::fromStdString(name.substr(3));
    int keycode = g_keycodeMapper.getKeycode(keyName);
    
    if (keycode == -1) {
        throw std::runtime_error("qt_keysym2keycode: Couldn't convert name '" + name + "' to keycode");
    }
    
    return keycode;
}

bool str2event(const std::string& name, int& type, int& code)
{
    if (name == "void" || name == "none")
    {
        type = -1;
        code = -1;
        return true;
    }
    else if (name.compare(0, 3, "REL") == 0)
    {
        type = EV_REL;
        code = evdev_rel_names[name];
        return true;
    }
    else if (name.compare(0, 3, "ABS") == 0)
    {
        type = EV_ABS;
        code = evdev_abs_names[name];
        return true;
    }
    else if (name.compare(0, 2, "XK") == 0)
    {
        type = EV_KEY;
        code = qt_keysym2keycode(name);
        return true;
    }
    else if (name.compare(0, 2, "JS") == 0)
    {
        int int_value = 0;
        std::istringstream(name.substr(3)) >> int_value;

        type = EV_KEY;
        code = BTN_JOYSTICK + int_value;

        return true;
    }
    else if (name.compare(0, 3, "KEY") == 0 ||
            name.compare(0, 3, "BTN") == 0)
    {
        type = EV_KEY;
        code = evdev_btn_names[name];
        return true;
    }
    else
    {
        return false;
    }
}

std::string btn2str(int i)
{
    return evdev_btn_names[i];
}

std::string abs2str(int i)
{
    return evdev_abs_names[i];
}

std::string rel2str(int i)
{
    return evdev_rel_names[i];
}

int linux_key_to_qt_key(int code)
{
    // This is a simplified mapping from Linux input key codes to Qt key codes
    switch (code) {
        case KEY_SPACE: return Qt::Key_Space;
        case KEY_ESC: return Qt::Key_Escape;
        case KEY_ENTER: 
        case KEY_KPENTER: return Qt::Key_Return;
        case KEY_TAB: return Qt::Key_Tab;
        case KEY_BACKSPACE: return Qt::Key_Backspace;
        case KEY_LEFTCTRL: 
        case KEY_RIGHTCTRL: return Qt::Key_Control;
        case KEY_LEFTSHIFT: 
        case KEY_RIGHTSHIFT: return Qt::Key_Shift;
        case KEY_LEFTALT: 
        case KEY_RIGHTALT: return Qt::Key_Alt;
        case KEY_LEFTMETA: 
        case KEY_RIGHTMETA: return Qt::Key_Meta;
        default:
            if (code >= KEY_A && code <= KEY_Z) {
                return Qt::Key_A + (code - KEY_A);
            }
            else if (code >= KEY_0 && code <= KEY_9) {
                return Qt::Key_0 + (code - KEY_0);
            }
            else if (code >= KEY_F1 && code <= KEY_F12) {
                return Qt::Key_F1 + (code - KEY_F1);
            }
            break;
    }
    
    return Qt::Key_unknown;
}

int qt_key_to_linux_key(int code)
{
    // This is a simplified mapping from Qt key codes to Linux input key codes
    switch (code) {
        case Qt::Key_Space: return KEY_SPACE;
        case Qt::Key_Escape: return KEY_ESC;
        case Qt::Key_Return: return KEY_ENTER;
        case Qt::Key_Tab: return KEY_TAB;
        case Qt::Key_Backspace: return KEY_BACKSPACE;
        case Qt::Key_Control: return KEY_LEFTCTRL;
        case Qt::Key_Shift: return KEY_LEFTSHIFT;
        case Qt::Key_Alt: return KEY_LEFTALT;
        case Qt::Key_Meta: return KEY_LEFTMETA;
        default:
            if (code >= Qt::Key_A && code <= Qt::Key_Z) {
                return KEY_A + (code - Qt::Key_A);
            }
            else if (code >= Qt::Key_0 && code <= Qt::Key_9) {
                return KEY_0 + (code - Qt::Key_0);
            }
            else if (code >= Qt::Key_F1 && code <= Qt::Key_F12) {
                return KEY_F1 + (code - Qt::Key_F1);
            }
            break;
    }
    
    return -1;
}
