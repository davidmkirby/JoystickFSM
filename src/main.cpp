#include "mainwindow.h"
#include "configuredialog.h"

#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <iostream>

int main(int argc, char *argv[])
{
    try {
        // Create the Qt application
        QApplication app(argc, argv);
        app.setApplicationName("JoystickFSM");
        app.setApplicationVersion("1.0.0");
        
        // Create main window and configuration dialog
        MainWindow mainWindow;
        ConfigureDialog configDialog;
        
        // Connect them together
        mainWindow.SetConfigureDialog(&configDialog);
        
        // Show the configuration dialog first
        int result = configDialog.exec();
        if (result == QDialog::Rejected) {
            return 0;
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
