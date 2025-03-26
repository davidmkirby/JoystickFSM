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
};

#endif // CONFIGUREDIALOG_H
