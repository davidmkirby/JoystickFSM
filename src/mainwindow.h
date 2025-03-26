#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QVector>
#include <memory>

// Advantech DAQ headers
#include "../../inc/bdaqctrl.h"
using namespace Automation::BDaq;

// Forward declarations
class QButtonGroup;
class SimpleGraph;
class Joystick;
class ConfigureDialog;
class AxisWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void Initialize();
    void SetConfigureDialog(ConfigureDialog *dialog) { configureDialog = dialog; }
    void SetConfigureParameter(const ConfigureParameter& value) { configure = value; }

private slots:
    // DAQ related slots
    void ButtonConfigureClicked();
    void ButtonStartClicked();
    void ButtonStopClicked();
    void ButtonCenterClicked();
    
    // AI related slots
    void DivValueChanged(int value);
    
    // Joystick related slots
    void JoystickRefreshClicked();
    void JoystickCalibrateClicked();
    void OnJoystickAxisChanged(int number, int value);
    void OnJoystickButtonChanged(int number, bool value);
    
    // Timer tick for updating outputs
    void TimerTicked();
    
    // Menu actions
    void OnMenuExit();
    void OnMenuConfigure();
    void OnMenuJoystickTest();
    void OnMenuAbout();
    
    // Settings changes
    void OnJoystickSelectionChanged(int index);
    void OnBackendSelectionChanged(int index);
    void OnXAxisMappingChanged(int index);
    void OnYAxisMappingChanged(int index);
    void OnXChannelChanged(int index);
    void OnYChannelChanged(int index);
    void OnInvertXChanged(bool checked);
    void OnInvertYChanged(bool checked);
    void OnXScaleChanged(double value);
    void OnYScaleChanged(double value);
    void OnDeadzoneChanged(double value);

private:
    // Helper methods
    void ConfigureDevice();
    void ConfigureAI();
    void ConfigureAO();
    void ConfigureGraph();
    void UpdateAxisValues();
    void SetXCord();
    void CheckError(ErrorCode errorCode);
    void RefreshJoystickList();
    void ConnectJoystick(int index);
    void UpdateUI();
    void ApplyDeadzone(double &x, double &y);
    void UpdateMirrorPosition(double x, double y);
    
    // Static callbacks for Advantech AI events
    static void BDAQCALL OnDataReadyEvent(void *sender, BfdAiEventArgs *args, void *userParam);
    static void BDAQCALL OnOverRunEvent(void *sender, BfdAiEventArgs *args, void *userParam);
    static void BDAQCALL OnCacheOverflowEvent(void *sender, BfdAiEventArgs *args, void *userParam);
    static void BDAQCALL OnStoppedEvent(void *sender, BfdAiEventArgs *args, void *userParam);

private:
    Ui::MainWindow *ui;
    ConfigureDialog *configureDialog;
    ConfigureParameter configure;
    
    // AI related members
    WaveformAiCtrl *waveformAiCtrl;
    double *scaledData;
    int rawDataBufferLength;
    TimeUnit timeUnit;
    double xInc;
    SimpleGraph *graph;
    
    // AO related members
    InstantAoCtrl *instantAoCtrl;
    int aoChannelStart;
    int aoChannelCount;
    double aoData[2];  // Data for analog output channels
    
    // Joystick related members
    std::unique_ptr<Joystick> joystick;
    QVector<double> joystickAxes;    // Normalized values (-1.0 to 1.0)
    QVector<bool> joystickButtons;   // Button states
    double xAxisValue;               // Current X axis value
    double yAxisValue;               // Current Y axis value
    
    // Mapping settings
    int xAxisMapping;                // Which joystick axis maps to X output
    int yAxisMapping;                // Which joystick axis maps to Y output
    int xChannelMapping;             // Which AO channel for X
    int yChannelMapping;             // Which AO channel for Y
    bool invertX;                    // Whether to invert X axis
    bool invertY;                    // Whether to invert Y axis
    double xScale;                   // Scaling factor for X
    double yScale;                   // Scaling factor for Y
    double deadzone;                 // Deadzone radius
    
    // Timer for regular updates
    QTimer *timer;
    
    // Custom widgets
    AxisWidget *joystickWidget;      // Widget showing joystick position
};

#endif // MAINWINDOW_H
