#-------------------------------------------------
#
# Project created for Joystick-controlled Fast-Steering Mirror
#
#-------------------------------------------------
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = JoystickFSM
TEMPLATE = app

# Include Advantech DAQ Control library
INCLUDEPATH += ../../inc/bdaqctrl.h

# Sources
SOURCES += src/main.cpp\
           src/mainwindow.cpp\
           src/joystick.cpp\
           src/joystick_factory.cpp\
           src/configuredialog.cpp\
           src/libinput_joystick.cpp\
           src/utils/evdev_helper.cpp\
           src/utils/libinput_helper.cpp\
           src/widgets/axis_widget.cpp\
           src/widgets/button_widget.cpp\
           src/widgets/simplegraph.cpp

HEADERS += src/mainwindow.h\
           src/joystick.h\
           src/joystick_factory.h\
           src/joystick_description.h\
           src/configuredialog.h\
           src/libinput_joystick.h\
           src/utils/evdev_helper.h\
           src/utils/libinput_helper.h\
           src/utils/dialog_helper.h\
           src/widgets/axis_widget.h\
           src/widgets/button_widget.h\
           src/widgets/simplegraph.h\
           src/widgets/waveformgenerator.h\

FORMS   += src/mainwindow.ui \ 
           src/configuredialog.ui

RESOURCES += resources.qrc

CONFIG += debug_and_release

# Configure build directories
CONFIG(debug, debug|release){
        DESTDIR += $$PWD/bin/debug
        OBJECTS_DIR = $$PWD/debug
        UI_DIR      = $$PWD/debug/ui
        MOC_DIR     = $$PWD/debug/moc
        RCC_DIR     = $$PWD/debug/rcc

} else {
        DESTDIR += $$PWD/bin/release
        OBJECTS_DIR = $$PWD/release
        UI_DIR      = $$PWD/release/ui
        MOC_DIR     = $$PWD/release/moc
        RCC_DIR     = $$PWD/release/rcc
}

# Platform-specific configuration
unix: LIBS += -lbiodaq -ludev -linput
