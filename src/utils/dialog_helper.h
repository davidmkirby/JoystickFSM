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

#ifndef DIALOG_HELPER_H
#define DIALOG_HELPER_H

#include <QObject>
#include <QWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <QApplication>
#include <QScreen>
#include <QProcess>
#include <QMessageBox>

// Helper class for managing dialogs and related operations
class DialogHelper : public QObject
{
    Q_OBJECT

public:
    static DialogHelper& instance() {
        static DialogHelper instance;
        return instance;
    }
    
    // Center a dialog on screen
    static void centerDialog(QDialog* dialog) {
        if (!dialog)
            return;
            
        QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
        int x = (screenGeometry.width() - dialog->width()) / 2;
        int y = (screenGeometry.height() - dialog->height()) / 2;
        dialog->move(x, y);
    }
    
    // Center a dialog relative to a parent widget
    static void centerDialogOnParent(QDialog* dialog, QWidget* parent) {
        if (!dialog || !parent)
            return;
            
        QRect parentGeometry = parent->geometry();
        int x = parentGeometry.x() + (parentGeometry.width() - dialog->width()) / 2;
        int y = parentGeometry.y() + (parentGeometry.height() - dialog->height()) / 2;
        dialog->move(x, y);
    }
    
    // Launch a dialog in a completely separate process
    static void launchExternalDialog(const QString& type, const QString& devicePath) {
        QString program = QApplication::applicationFilePath();
        QStringList arguments;
        arguments << "--external-dialog" << type << devicePath;
        
        QProcess* process = new QProcess();
        process->setProgram(program);
        process->setArguments(arguments);
        
        // Connect to finished signal to clean up
        QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                        process, &QProcess::deleteLater);
        
        process->start();
    }
    
    // Helper to create a joystick calibration dialog in a separate process
    static void showJoystickCalibration(const QString& devicePath) {
        launchExternalDialog("joystick-calibration", devicePath);
    }
    
    // Helper to create a joystick mapping dialog in a separate process
    static void showJoystickMapping(const QString& devicePath) {
        launchExternalDialog("joystick-mapping", devicePath);
    }
    
    // Helper to show an error message
    static void showError(QWidget* parent, const QString& title, const QString& message) {
        QMessageBox::critical(parent, title, message);
    }
    
    // Helper to show an information message
    static void showInfo(QWidget* parent, const QString& title, const QString& message) {
        QMessageBox::information(parent, title, message);
    }
    
    // Helper to show a confirmation dialog
    static bool confirmAction(QWidget* parent, const QString& title, const QString& message) {
        return QMessageBox::question(parent, title, message, 
                                 QMessageBox::Yes | QMessageBox::No, 
                                 QMessageBox::No) == QMessageBox::Yes;
    }

private:
    DialogHelper() {}
    ~DialogHelper() {}
    
    // Disable copying
    DialogHelper(const DialogHelper&) = delete;
    DialogHelper& operator=(const DialogHelper&) = delete;
};

#endif // DIALOG_HELPER_H
