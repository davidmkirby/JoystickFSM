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

#include "mainwindow.h"
#include "configuredialog.h"

#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QCommandLineParser>
#include <QLoggingCategory>
#include <iostream>

// Define version information
#define APP_VERSION "1.0.0"
#define APP_NAME "JoystickFSM"
#define ORG_NAME "JoystickFSM"
#define ORG_DOMAIN "joystickfsm.org"

int main(int argc, char *argv[])
{
    try {
        // Create the Qt application
        QApplication app(argc, argv);
        app.setApplicationName(APP_NAME);
        app.setApplicationVersion(APP_VERSION);
        app.setOrganizationName(ORG_NAME);
        app.setOrganizationDomain(ORG_DOMAIN);
        
        // Set up command line parser
        QCommandLineParser parser;
        parser.setApplicationDescription("Joystick-controlled Fast-Steering Mirror (FSM) Application");
        parser.addHelpOption();
        parser.addVersionOption();
        
        // Add custom command line options
        QCommandLineOption debugOption("debug", "Enable debug output");
        parser.addOption(debugOption);
        
        QCommandLineOption waylandOption("wayland", "Force Wayland platform plugin");
        parser.addOption(waylandOption);
        
        QCommandLineOption noConfigOption("no-config", "Skip configuration dialog");
        parser.addOption(noConfigOption);
        
        // Process the command line arguments
        parser.process(app);
        
        // Configure logging based on debug option
        if (parser.isSet(debugOption)) {
            QLoggingCategory::setFilterRules("*.debug=true");
            qDebug() << "Debug logging enabled";
        } else {
            QLoggingCategory::setFilterRules("*.debug=false");
        }
        
        // Set platform to Wayland if requested
        if (parser.isSet(waylandOption)) {
            qputenv("QT_QPA_PLATFORM", "wayland");
        }
        
        // Create main window and configuration dialog
        MainWindow mainWindow;
        ConfigureDialog configDialog;
        
        // Connect them together
        mainWindow.SetConfigureDialog(&configDialog);
        
        // Show the configuration dialog first, unless --no-config is specified
        if (!parser.isSet(noConfigOption)) {
            int result = configDialog.exec();
            if (result == QDialog::Rejected) {
                return 0;
            }
        } else {
            // Use default configuration
            configDialog.Initialization();
        }
        
        // Initialize the main window with the configuration
        mainWindow.SetConfigureParameter(configDialog.GetConfigureParameter());
        mainWindow.Initialize();
        
        // Show the main window
        mainWindow.show();
        
        // Run the event loop
        return app.exec();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        QMessageBox::critical(nullptr, "Fatal Error", 
                             QString("An unhandled exception occurred: %1").arg(e.what()));
        return 1;
    }
}
