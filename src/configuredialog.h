#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>
#include <QWidget>
#include "../../inc/bdaqctrl.h"

using namespace Automation::BDaq;

// Configuration parameters structure
struct ConfigureParameter
{
    // Separate device settings
    QString aiDeviceName;
    QString aiProfilePath;

    QString aoDeviceName;
    QString aoProfilePath;

    // AI specific parameters
    int aiChannelStart;
    int aiChannelCount;
    ValueRange aiValueRange;
    int32 clockRatePerChan;
    int32 sectionLength;

    // AO specific parameters
    int aoChannelStart;
    int aoChannelCount;
    ValueRange aoValueRange;
    int pointCountPerWave;

    // Joystick settings
    QString joystickBackend;  // "Auto", "Legacy", or "Libinput"
    double deadzone;
    double xScale;
    double yScale;
    bool invertX;
    bool invertY;

    // Constructor with default values
    ConfigureParameter() :
        aiDeviceName(""),
        aiProfilePath(""),
        aoDeviceName(""),
        aoProfilePath(""),
        aiChannelStart(0),
        aiChannelCount(2),
        aiValueRange(V_ExternalRefBipolar),
        clockRatePerChan(1000),
        sectionLength(1024),
        aoChannelStart(0),
        aoChannelCount(2),
        aoValueRange(V_ExternalRefBipolar),
        pointCountPerWave(400),
        joystickBackend("Auto"),
        deadzone(0.05),
        xScale(1.0),
        yScale(1.0),
        invertX(false),
        invertY(false)
    {}
};

namespace Ui {
    class ConfigureDialog;
}

class ConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    ConfigureDialog(QDialog *parent = nullptr);
    ~ConfigureDialog();
    
    void Initialization();
    void CheckError(ErrorCode errorCode);
    ConfigureParameter GetConfigureParameter() { return configure; }
    void RefreshConfigureParameter();

private:
    Ui::ConfigureDialog *ui;
    ConfigureParameter configure;

private slots:
    void AIDeviceChanged(int index);
    void AODeviceChanged(int index);
    void ButtonOKClicked();
    void ButtonCancelClicked();
    void AIButtonBrowseClicked();
    void AOButtonBrowseClicked();
    void TabChanged(int index);

    // New joystick-related slots
    void JoystickBackendChanged(int index);
    void DeadzoneChanged(double value);
    void XScaleChanged(double value);
    void YScaleChanged(double value);
    void InvertXChanged(bool checked);
    void InvertYChanged(bool checked);
};

#endif // CONFIGUREDIALOG_H
