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
#include "ui_mainwindow.h"
#include "configuredialog.h"
#include "joystick_factory.h"
#include "utils/dialog_helper.h"
#include "widgets/axis_widget.h"
#include "widgets/rudder_widget.h"
#include "widgets/throttle_widget.h"
#include "widgets/simplegraph.h"

#include <QDebug>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QTimer>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    configureDialog(nullptr),
    waveformAiCtrl(nullptr),
    scaledData(nullptr),
    rawDataBufferLength(0),
    instantAoCtrl(nullptr),
    joystick(nullptr),
    xAxisValue(0.0),
    yAxisValue(0.0),
    xAxisMapping(0),
    yAxisMapping(1),
    xChannelMapping(0),
    yChannelMapping(1),
    invertX(false),
    invertY(false),
    xScale(1.0),
    yScale(1.0),
    deadzone(0.05),
    joystickWidget(nullptr),
    rudderWidget(nullptr),
    throttleWidget(nullptr)
{
    ui->setupUi(this);

    // Initialize timer for regular updates
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::TimerTicked);

    // Initialize arrays
    aoData[0] = 0.0;
    aoData[1] = 0.0;

    // Connect button signals
    connect(ui->btnConfiguration, &QPushButton::clicked, this, &MainWindow::ButtonConfigureClicked);
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::ButtonStartClicked);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::ButtonStopClicked);
    connect(ui->btnCenter, &QPushButton::clicked, this, &MainWindow::ButtonCenterClicked);
    connect(ui->btnJoystickRefresh, &QPushButton::clicked, this, &MainWindow::JoystickRefreshClicked);
    connect(ui->btnJoystickCalibrate, &QPushButton::clicked, this, &MainWindow::JoystickCalibrateClicked);

    // Connect menu actions
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::OnMenuExit);
    connect(ui->actionConfigure, &QAction::triggered, this, &MainWindow::OnMenuConfigure);
    connect(ui->actionJoystickTest, &QAction::triggered, this, &MainWindow::OnMenuJoystickTest);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::OnMenuAbout);

    // Connect settings change signals
    connect(ui->cmbJoystick, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::OnJoystickSelectionChanged);
    connect(ui->cmbBackend, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::OnBackendSelectionChanged);
    connect(ui->cmbXAxis, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::OnXAxisMappingChanged);
    connect(ui->cmbYAxis, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::OnYAxisMappingChanged);
    connect(ui->cmbXChannel, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::OnXChannelChanged);
    connect(ui->cmbYChannel, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::OnYChannelChanged);
    connect(ui->chkInvertX, &QCheckBox::toggled, this, &MainWindow::OnInvertXChanged);
    connect(ui->chkInvertY, &QCheckBox::toggled, this, &MainWindow::OnInvertYChanged);
    connect(ui->spinXScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::OnXScaleChanged);
    connect(ui->spinYScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::OnYScaleChanged);
    connect(ui->spinDeadzone, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::OnDeadzoneChanged);

    // Set initial UI state
    ui->btnStop->setEnabled(false);
}

MainWindow::~MainWindow()
{
    // Stop any running operations
    if (waveformAiCtrl) {
        waveformAiCtrl->Stop();
    }

    // Clean up any allocated resources
    if (scaledData) {
        delete[] scaledData;
        scaledData = nullptr;
    }

    if (waveformAiCtrl) {
        waveformAiCtrl->Dispose();
        waveformAiCtrl = nullptr;
    }

    if (instantAoCtrl) {
        instantAoCtrl->Dispose();
        instantAoCtrl = nullptr;
    }

    // Delete the UI
    delete ui;
}

void MainWindow::Initialize()
{
    // Create and set up axis widget for joystick visualization
    joystickWidget = new AxisWidget(200, 200, true, ui->joystickFrame);
    QVBoxLayout* joystickLayout = new QVBoxLayout(ui->joystickFrame);
    joystickLayout->addWidget(joystickWidget);
    joystickWidget->setShowValues(true);
    joystickWidget->setShowDeadzone(true);
    joystickWidget->setDeadzone(deadzone);
    joystickWidget->setShowLimits(true);
    joystickWidget->setLimit(0.9);  // Safety limit for mirror

    // Create rudder and throttle visualizations
    rudderWidget = new RudderWidget(200, 40, ui->graphFrame);
    rudderWidget->setShowValue(true);
    rudderWidget->hide(); // Hide initially, will show when needed

    throttleWidget = new ThrottleWidget(40, 200, false, ui->graphFrame);
    throttleWidget->setShowValue(true);
    throttleWidget->hide(); // Hide initially, will show when needed

    // Set up simple graph for mirror position visualization
    graph = new SimpleGraph(ui->graphFrame);
    QVBoxLayout* graphLayout = new QVBoxLayout(ui->graphFrame);
    graphLayout->addWidget(graph);

    // Set graph properties
    graph->m_yCordRangeMax = 10.0;
    graph->m_yCordRangeMin = -10.0;

    // Configure DAQ devices if available
    ConfigureDevice();

    // Scan for joysticks
    JoystickRefreshClicked();

    // Set initial UI values from configuration
    invertX = configure.invertX;
    invertY = configure.invertY;
    xScale = configure.xScale;
    yScale = configure.yScale;
    deadzone = configure.deadzone;

    ui->chkInvertX->setChecked(invertX);
    ui->chkInvertY->setChecked(invertY);
    ui->spinXScale->setValue(xScale);
    ui->spinYScale->setValue(yScale);
    ui->spinDeadzone->setValue(deadzone);

    // Set initial joystick backend selection
    if (configure.joystickBackend == "Legacy") {
        ui->cmbBackend->setCurrentIndex(1);
    } else if (configure.joystickBackend == "Libinput") {
        ui->cmbBackend->setCurrentIndex(2);
    } else { // Auto
        ui->cmbBackend->setCurrentIndex(0);
    }

    // Start the timer for regular updates (50Hz)
    timer->start(20);

    // Update status bar
    ui->lblStatus->setText("Status: Ready");

    // Update UI state
    UpdateUI();
}

void MainWindow::ConfigureDevice()
{
    // Configure analog input if specified
    if (!configure.aiDeviceName.isEmpty()) {
        ConfigureAI();
    }

    // Configure analog output if specified
    if (!configure.aoDeviceName.isEmpty()) {
        ConfigureAO();
    }

    // Configure graph
    ConfigureGraph();
}

void MainWindow::ConfigureAI()
{
    ErrorCode errorCode = Success;

    // Create waveform AI control if needed
    if (waveformAiCtrl == nullptr) {
        waveformAiCtrl = WaveformAiCtrl::Create();
    }

    // Set device
    DeviceInformation devInfo(configure.aiDeviceName.toStdWString().c_str());
    errorCode = waveformAiCtrl->setSelectedDevice(devInfo);
    CheckError(errorCode);

    // Load profile if specified
    if (!configure.aiProfilePath.isEmpty()) {
        errorCode = waveformAiCtrl->LoadProfile(configure.aiProfilePath.toStdWString().c_str());
        CheckError(errorCode);
    }

    // Configure the AI channels and parameters
    Conversion* conversion = waveformAiCtrl->getConversion();
    conversion->setChannelStart(configure.aiChannelStart);
    conversion->setChannelCount(configure.aiChannelCount);
    conversion->setClockRate(configure.clockRatePerChan);

    // Set the data range for each channel
    Array<ValueRange>* valueRanges = waveformAiCtrl->getChannelRanges();
    for (int i = 0; i < conversion->getChannelCount(); i++) {
        valueRanges->setItem(i, configure.aiValueRange);
    }

    // Calculate buffer size
    rawDataBufferLength = conversion->getChannelCount() * configure.sectionLength;
    if (scaledData != nullptr) {
        delete[] scaledData;
    }
    scaledData = new double[rawDataBufferLength];

    // Set up streaming parameters
    Record* record = waveformAiCtrl->getRecord();
    record->setSectionLength(configure.sectionLength);
    record->setSectionCount(0); // Continuous streaming

    // Register event handlers
    waveformAiCtrl->addDataReadyHandler(OnDataReadyEvent, this);
    waveformAiCtrl->addOverrunHandler(OnOverRunEvent, this);
    waveformAiCtrl->addCacheOverflowHandler(OnCacheOverflowEvent, this);
    waveformAiCtrl->addStoppedHandler(OnStoppedEvent, this);

    // Set time unit and increment value
    timeUnit = Millisecond;
    xInc = 1.0 / conversion->getClockRate();

    // Update the UI
    ui->lblAIDeviceValue->setText(configure.aiDeviceName);
    ui->lblAIChanValue->setText(QString("%1 - %2").arg(configure.aiChannelStart)
                               .arg(configure.aiChannelStart + configure.aiChannelCount - 1));
    ui->lblAIRateValue->setText(QString("%1 Hz").arg(configure.clockRatePerChan));
}

void MainWindow::ConfigureAO()
{
    ErrorCode errorCode = Success;

    // Create instant AO control if needed
    if (instantAoCtrl == nullptr) {
        instantAoCtrl = InstantAoCtrl::Create();
    }

    // Set device
    DeviceInformation devInfo(configure.aoDeviceName.toStdWString().c_str());
    errorCode = instantAoCtrl->setSelectedDevice(devInfo);
    CheckError(errorCode);

    // Load profile if specified
    if (!configure.aoProfilePath.isEmpty()) {
        errorCode = instantAoCtrl->LoadProfile(configure.aoProfilePath.toStdWString().c_str());
        CheckError(errorCode);
    }

    // Configure AO channels
    aoChannelStart = configure.aoChannelStart;
    aoChannelCount = configure.aoChannelCount;

    // Set the channel data range
    Array<ValueRange>* valueRanges = instantAoCtrl->getChannelRanges();
    for (int i = 0; i < aoChannelCount; i++) {
        valueRanges->setItem(i, configure.aoValueRange);
    }

    // Update the UI
    ui->lblAODeviceValue->setText(configure.aoDeviceName);
    ui->lblAOChanValue->setText(QString("%1 - %2").arg(aoChannelStart)
                               .arg(aoChannelStart + aoChannelCount - 1));

    // Populate the channel combo boxes
    ui->cmbXChannel->clear();
    ui->cmbYChannel->clear();

    for (int i = aoChannelStart; i < aoChannelStart + aoChannelCount; i++) {
        QString channelName = QString("Channel %1").arg(i);
        ui->cmbXChannel->addItem(channelName, i);
        ui->cmbYChannel->addItem(channelName, i);
    }

    // Set default mappings
    if (aoChannelCount >= 1) {
        ui->cmbXChannel->setCurrentIndex(0);
        xChannelMapping = aoChannelStart;
    }

    if (aoChannelCount >= 2) {
        ui->cmbYChannel->setCurrentIndex(1);
        yChannelMapping = aoChannelStart + 1;
    }
}

void MainWindow::ConfigureGraph()
{
    if (graph) {
        // Setup the graph for visualization
        graph->Clear();

        // Set Y-coordinate range to match the AO value range
        if (!configure.aoDeviceName.isEmpty()) {
            Array<ValueRange>* valueRanges = instantAoCtrl->getChannelRanges();
            ValueRange range = valueRanges->getItem(0);

            // Extract min and max from the range
            double min = 0.0;
            double max = 0.0;

            switch (range) {
                case V_Neg10To10:
                    min = -10.0;
                    max = 10.0;
                    break;
                case V_Neg5To5:
                    min = -5.0;
                    max = 5.0;
                    break;
                case V_0To10:
                    min = 0.0;
                    max = 10.0;
                    break;
                default:
                    min = -10.0;
                    max = 10.0;
                    break;
            }

            graph->m_yCordRangeMin = min;
            graph->m_yCordRangeMax = max;
        }

        // Update labels
        QString ranges[3];
        graph->GetYCordRange(ranges, graph->m_yCordRangeMax, graph->m_yCordRangeMin, Voltage);

        ui->lblYCoordinateMax->setText(ranges[0]);
        ui->lblXCoordinateStart->setText("0s");
        ui->lblXCoordinateEnd->setText("10s");
    }
}

void MainWindow::UpdateUI()
{
    bool hasAI = !configure.aiDeviceName.isEmpty();
    bool hasAO = !configure.aoDeviceName.isEmpty();
    bool hasJoystick = joystick != nullptr;

    // Enable/disable buttons based on state
    ui->btnStart->setEnabled(hasAI || hasAO);
    ui->btnStop->setEnabled(false); // Will be enabled when started
    ui->btnCenter->setEnabled(hasAO);
    ui->btnJoystickCalibrate->setEnabled(hasJoystick);

    // Update channel visualizations
    if (hasAO) {
        ui->lblXVoltage->setText(QString("X Voltage: %.2fV").arg(aoData[0]));
        ui->lblYVoltage->setText(QString("Y Voltage: %.2fV").arg(aoData[1]));
    } else {
        ui->lblXVoltage->setText("X Voltage: N/A");
        ui->lblYVoltage->setText("Y Voltage: N/A");
    }

    // Update joystick info
    if (hasJoystick) {
        ui->joystickLabel->setText(QString("%1 (%2 axes, %3 buttons)")
                                 .arg(joystick->getName())
                                 .arg(joystick->getAxisCount())
                                 .arg(joystick->getButtonCount()));
    } else {
        ui->joystickLabel->setText("No joystick connected");
    }
}

void MainWindow::ButtonConfigureClicked()
{
    OnMenuConfigure();
}

void MainWindow::ButtonStartClicked()
{
    // Disable configuration during operation
    ui->btnConfiguration->setEnabled(false);
    ui->btnStart->setEnabled(false);
    ui->btnStop->setEnabled(true);

    // Start AI acquisition if configured
    if (!configure.aiDeviceName.isEmpty() && waveformAiCtrl) {
        ErrorCode errorCode = waveformAiCtrl->Start();
        if (BioFailed(errorCode)) {
            CheckError(errorCode);
            return;
        }
        xInc = 1.0 / waveformAiCtrl->getConversion()->getClockRate();
    }

    // Update status
    ui->lblStatus->setText("Status: Running");
    ui->lblStatus->setStyleSheet("color: green");
}

void MainWindow::ButtonStopClicked()
{
    // Stop AI acquisition if configured
    if (!configure.aiDeviceName.isEmpty() && waveformAiCtrl) {
        ErrorCode errorCode = waveformAiCtrl->Stop();
        CheckError(errorCode);
        graph->Clear();
    }

    // Center the mirror
    ButtonCenterClicked();

    // Re-enable configuration
    ui->btnConfiguration->setEnabled(true);
    ui->btnStart->setEnabled(true);
    ui->btnStop->setEnabled(false);

    // Update status
    ui->lblStatus->setText("Status: Stopped");
    ui->lblStatus->setStyleSheet("color: black");
}

void MainWindow::ButtonCenterClicked()
{
    // Set joystick values to zero
    xAxisValue = 0.0;
    yAxisValue = 0.0;

    // Update visualizations
    joystickWidget->setXAxis(0.0);
    joystickWidget->setYAxis(0.0);

    if (rudderWidget && rudderWidget->isVisible()) {
        rudderWidget->setPos(0.0);
    }

    if (throttleWidget && throttleWidget->isVisible()) {
        throttleWidget->setPos(0.0);
    }

    // Send zeros to mirror
    UpdateMirrorPosition(0.0, 0.0);

    // Update status
    ui->lblStatus->setText("Status: Mirror centered");
}

void MainWindow::DivValueChanged(int value)
{
    if (graph) {
        graph->Div(value);
    }
}

void MainWindow::JoystickRefreshClicked()
{
    // Clear the combo box
    ui->cmbJoystick->clear();

    // Get the selected backend
    JoystickBackend backend = JoystickBackend::AUTO;
    switch (ui->cmbBackend->currentIndex()) {
    case 1:
        backend = JoystickBackend::LEGACY;
        break;
    case 2:
        backend = JoystickBackend::LIBINPUT;
        break;
    default:
        backend = JoystickBackend::AUTO;
        break;
    }

    // Get available joysticks
    std::vector<JoystickDescription> joysticks = JoystickFactory::getJoysticks(backend);

    if (joysticks.empty()) {
        ui->joystickLabel->setText("No joysticks found");
        return;
    }

    // Add joysticks to combo box
    for (const auto& desc : joysticks) {
        QString name = QString::fromStdString(desc.name);
        QString entry = QString("%1 (%2 axes, %3 buttons)")
                        .arg(name)
                        .arg(desc.axis_count)
                        .arg(desc.button_count);

        // Store the device path as user data
        ui->cmbJoystick->addItem(entry, QString::fromStdString(desc.filename));
    }

    // Select the first joystick
    if (ui->cmbJoystick->count() > 0) {
        ui->cmbJoystick->setCurrentIndex(0);
    }
}

void MainWindow::JoystickCalibrateClicked()
{
    if (!joystick) {
        QMessageBox::warning(this, "Warning", "No joystick connected");
        return;
    }

    try {
        // Show a dialog to get calibration settings
        DialogHelper::showJoystickCalibration(QString::fromStdString(joystick->getFilename()));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error",
                             QString("Failed to launch calibration: %1").arg(e.what()));
    }
}

void MainWindow::OnJoystickAxisChanged(int number, int value)
{
    // Store the normalized value (-1.0 to 1.0)
    double normalizedValue = value / 32767.0;

    // Limit to valid range
    if (normalizedValue < -1.0) normalizedValue = -1.0;
    if (normalizedValue > 1.0) normalizedValue = 1.0;

    // Update axes array
    if (number >= 0 && number < joystickAxes.size()) {
        joystickAxes[number] = normalizedValue;

        // Update main axis values if this axis is mapped
        if (number == xAxisMapping) {
            xAxisValue = normalizedValue;
        }

        if (number == yAxisMapping) {
            yAxisValue = normalizedValue;
        }
    }
}

void MainWindow::OnJoystickButtonChanged(int number, bool value)
{
    // Store button state
    if (number >= 0 && number < joystickButtons.size()) {
        joystickButtons[number] = value;
    }
}

void MainWindow::TimerTicked()
{
    // Update the AO outputs based on joystick position
    if (!configure.aoDeviceName.isEmpty() && instantAoCtrl) {
        // Apply deadzone to current values
        double deadzoneX = xAxisValue;
        double deadzoneY = yAxisValue;
        ApplyDeadzone(deadzoneX, deadzoneY);

        // Apply inversion if needed
        if (invertX) deadzoneX = -deadzoneX;
        if (invertY) deadzoneY = -deadzoneY;

        // Apply scaling
        deadzoneX *= xScale;
        deadzoneY *= yScale;

        // Update joystick visualization
        joystickWidget->setXAxis(deadzoneX);
        joystickWidget->setYAxis(deadzoneY);

        // Apply smoothing if needed
        static double smoothedX = 0.0;
        static double smoothedY = 0.0;
        double smoothingFactor = 0.3;  // 0.0-1.0, higher is less smoothing

        smoothedX = smoothedX * (1.0 - smoothingFactor) + deadzoneX * smoothingFactor;
        smoothedY = smoothedY * (1.0 - smoothingFactor) + deadzoneY * smoothingFactor;

        // Update supplementary visualizations if available
        if (rudderWidget && rudderWidget->isVisible()) {
            rudderWidget->setPos(smoothedX);
        }

        if (throttleWidget && throttleWidget->isVisible()) {
            throttleWidget->setPos(smoothedY);
        }

        // Update mirror position
        UpdateMirrorPosition(smoothedX, smoothedY);
    }
}

void MainWindow::OnMenuExit()
{
    close();
}

void MainWindow::OnMenuConfigure()
{
    if (configureDialog) {
        // Save current state to configuration
        configure.invertX = invertX;
        configure.invertY = invertY;
        configure.xScale = xScale;
        configure.yScale = yScale;
        configure.deadzone = deadzone;

        switch (ui->cmbBackend->currentIndex()) {
        case 1:
            configure.joystickBackend = "Legacy";
            break;
        case 2:
            configure.joystickBackend = "Libinput";
            break;
        default:
            configure.joystickBackend = "Auto";
            break;
        }

        // Update configuration dialog
        configureDialog->RefreshConfigureParameter();

        // Show the dialog
        if (configureDialog->exec() == QDialog::Accepted) {
            // Get new configuration
            configure = configureDialog->GetConfigureParameter();

            // Reconfigure devices
            ConfigureDevice();

            // Update UI settings from configuration
            invertX = configure.invertX;
            invertY = configure.invertY;
            xScale = configure.xScale;
            yScale = configure.yScale;
            deadzone = configure.deadzone;

            ui->chkInvertX->setChecked(invertX);
            ui->chkInvertY->setChecked(invertY);
            ui->spinXScale->setValue(xScale);
            ui->spinYScale->setValue(yScale);
            ui->spinDeadzone->setValue(deadzone);
            joystickWidget->setDeadzone(deadzone);

            // Set joystick backend
            if (configure.joystickBackend == "Legacy") {
                ui->cmbBackend->setCurrentIndex(1);
            } else if (configure.joystickBackend == "Libinput") {
                ui->cmbBackend->setCurrentIndex(2);
            } else { // Auto
                ui->cmbBackend->setCurrentIndex(0);
            }

            // Refresh joystick list
            JoystickRefreshClicked();

            // Update UI
            UpdateUI();
        }
    }
}

void MainWindow::OnMenuJoystickTest()
{
    if (!joystick) {
        QMessageBox::warning(this, "Warning", "No joystick connected");
        return;
    }

    // Launch joystick test dialog (not implemented in this example)
    QMessageBox::information(this, "Joystick Test",
                           "The joystick test feature is not implemented yet.");
}

void MainWindow::OnMenuAbout()
{
    QString aboutText =
        "Joystick FSM Control\n\n"
        "A Qt application for controlling Fast-Steering Mirrors using joystick input.\n\n"
        "Version: 1.0.0\n"
        "License: GNU GPL v3\n\n"
        "Copyright © 2025";

    QMessageBox::about(this, "About Joystick FSM Control", aboutText);
}

void MainWindow::OnJoystickSelectionChanged(int index)
{
    if (index < 0 || index >= ui->cmbJoystick->count()) {
        return;
    }

    // Disconnect current joystick if any
    joystick.reset();

    // Get joystick path
    QString path = ui->cmbJoystick->itemData(index).toString();

    try {
        // Get joystick backend from UI
        JoystickBackend backend = JoystickBackend::AUTO;
        switch (ui->cmbBackend->currentIndex()) {
        case 1:
            backend = JoystickBackend::LEGACY;
            break;
        case 2:
            backend = JoystickBackend::LIBINPUT;
            break;
        default:
            backend = JoystickBackend::AUTO;
            break;
        }

        // Create the joystick
        joystick = JoystickFactory::createJoystick(path.toStdString(), backend);

        // Initialize axis and button lists
        joystickAxes.resize(joystick->getAxisCount(), 0.0);
        joystickButtons.resize(joystick->getButtonCount(), false);

        // Connect signals
        connect(joystick.get(), &Joystick::axisChanged, this, &MainWindow::OnJoystickAxisChanged);
        connect(joystick.get(), &Joystick::buttonChanged, this, &MainWindow::OnJoystickButtonChanged);

        // Update UI
        ui->joystickLabel->setText(QString("Connected: %1 (%2 axes, %3 buttons)")
                                .arg(joystick->getName())
                                .arg(joystick->getAxisCount())
                                .arg(joystick->getButtonCount()));

        ui->btnJoystickCalibrate->setEnabled(true);

        // Update axis mappings comboboxes
        ui->cmbXAxis->clear();
        ui->cmbYAxis->clear();

        for (int i = 0; i < joystick->getAxisCount(); i++) {
            QString axisName = QString("Axis %1").arg(i);
            ui->cmbXAxis->addItem(axisName, i);
            ui->cmbYAxis->addItem(axisName, i);
        }

        // Set defaults
        if (joystick->getAxisCount() >= 1) {
            ui->cmbXAxis->setCurrentIndex(0);
            xAxisMapping = 0;
        }

        if (joystick->getAxisCount() >= 2) {
            ui->cmbYAxis->setCurrentIndex(1);
            yAxisMapping = 1;
        }

        // Update status
        ui->lblStatus->setText(QString("Joystick connected: %1").arg(joystick->getName()));

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to open joystick: %1").arg(e.what()));
        ui->joystickLabel->setText("Failed to connect joystick");
        ui->btnJoystickCalibrate->setEnabled(false);

        // Update status
        ui->lblStatus->setText(QString("Failed to connect joystick: %1").arg(e.what()));
        ui->lblStatus->setStyleSheet("color: red");
    }
}

void MainWindow::OnBackendSelectionChanged(int index)
{
    // Update joystick list with the new backend
    JoystickRefreshClicked();
}

void MainWindow::OnXAxisMappingChanged(int index)
{
    if (index >= 0 && index < ui->cmbXAxis->count()) {
        xAxisMapping = ui->cmbXAxis->itemData(index).toInt();
    }
}

void MainWindow::OnYAxisMappingChanged(int index)
{
    if (index >= 0 && index < ui->cmbYAxis->count()) {
        yAxisMapping = ui->cmbYAxis->itemData(index).toInt();
    }
}

void MainWindow::OnXChannelChanged(int index)
{
    if (index >= 0 && index < ui->cmbXChannel->count()) {
        xChannelMapping = ui->cmbXChannel->itemData(index).toInt();
    }
}

void MainWindow::OnYChannelChanged(int index)
{
    if (index >= 0 && index < ui->cmbYChannel->count()) {
        yChannelMapping = ui->cmbYChannel->itemData(index).toInt();
    }
}

void MainWindow::OnInvertXChanged(bool checked)
{
    invertX = checked;
}

void MainWindow::OnInvertYChanged(bool checked)
{
    invertY = checked;
}

void MainWindow::OnXScaleChanged(double value)
{
    xScale = value;
}

void MainWindow::OnYScaleChanged(double value)
{
    yScale = value;
}

void MainWindow::OnDeadzoneChanged(double value)
{
    deadzone = value;
    if (joystickWidget) {
        joystickWidget->setDeadzone(value);
    }
}

void MainWindow::CheckError(ErrorCode errorCode)
{
    if (BioFailed(errorCode)) {
        QString message = tr("Error: 0x") + QString::number(errorCode, 16).right(8).toUpper();
        QMessageBox::critical(this, "Error", message);

        // Update status bar
        ui->lblStatus->setText(tr("Status: Error - 0x%1").arg(QString::number(errorCode, 16).right(8).toUpper()));
        ui->lblStatus->setStyleSheet("color: red");
    }
}

void MainWindow::RefreshJoystickList()
{
    JoystickRefreshClicked();
}

void MainWindow::ApplyDeadzone(double &x, double &y)
{
    // Calculate distance from center
    double distance = sqrt(x * x + y * y);

    if (distance < deadzone) {
        // Inside deadzone - set to zero
        x = 0.0;
        y = 0.0;
    } else {
        // Outside deadzone - rescale to remove discontinuity
        double factor = (distance - deadzone) / (1.0 - deadzone);
        x = x * factor / distance;
        y = y * factor / distance;
    }
}

void MainWindow::UpdateMirrorPosition(double x, double y)
{
    // Convert normalized values (-1 to 1) to voltage range
    Array<ValueRange>* valueRanges = instantAoCtrl->getChannelRanges();
    ValueRange range = valueRanges->getItem(0);

    double minV = 0.0;
    double maxV = 0.0;
    double midV = 0.0;

    // Extract min and max from the range
    switch (range) {
        case V_Neg10To10:
            minV = -10.0;
            maxV = 10.0;
            break;
        case V_Neg5To5:
            minV = -5.0;
            maxV = 5.0;
            break;
        case V_0To10:
            minV = 0.0;
            maxV = 10.0;
            break;
        default:
            minV = -10.0;
            maxV = 10.0;
            break;
    }

    // Calculate midpoint
    midV = (maxV + minV) / 2.0;

    // Convert normalized values to voltage
    double xVolts = midV + x * (maxV - midV);
    double yVolts = midV + y * (maxV - midV);

    // Clamp to valid range
    if (xVolts < minV) xVolts = minV;
    if (xVolts > maxV) xVolts = maxV;
    if (yVolts < minV) yVolts = minV;
    if (yVolts > maxV) yVolts = maxV;

    // Set data array based on channel mapping
    if (xChannelMapping - aoChannelStart >= 0 && xChannelMapping - aoChannelStart < aoChannelCount) {
        aoData[xChannelMapping - aoChannelStart] = xVolts;
    }

    if (yChannelMapping - aoChannelStart >= 0 && yChannelMapping - aoChannelStart < aoChannelCount) {
        aoData[yChannelMapping - aoChannelStart] = yVolts;
    }

    // Write to the DAQ
    ErrorCode errorCode = instantAoCtrl->Write(aoChannelStart, aoChannelCount, aoData);
    if (BioFailed(errorCode)) {
        CheckError(errorCode);
    }

    // Update voltage labels
    ui->lblXVoltage->setText(QString("X Voltage: %.2fV").arg(xVolts));
    ui->lblYVoltage->setText(QString("Y Voltage: %.2fV").arg(yVolts));

    // Update graph with mirror position
    if (graph) {
        // Add point to the position trace (channel 0 for X, channel 1 for Y)
        static double time = 0.0;
        time += 0.02; // 20ms per timer tick

        // If time exceeds the display window, reset
        if (time > 10.0) {
            graph->Clear();
            time = 0.0;
        }

        graph->AddPoint(0, time, xVolts);
        graph->AddPoint(1, time, yVolts);
    }
}

// Static callbacks for Advantech AI events
void BDAQCALL MainWindow::OnDataReadyEvent(void *sender, BfdAiEventArgs *args, void *userParam)
{
    MainWindow* mainWindow = (MainWindow*)userParam;

    // Get data from the device
    if (mainWindow && mainWindow->waveformAiCtrl) {
        mainWindow->waveformAiCtrl->GetData(args->Count, mainWindow->scaledData);

        // Update graph with the acquired data
        if (mainWindow->graph) {
            int channelCount = mainWindow->waveformAiCtrl->getConversion()->getChannelCount();
            mainWindow->graph->Chart(mainWindow->scaledData, channelCount, args->Count / channelCount, mainWindow->xInc);
        }
    }
}

void BDAQCALL MainWindow::OnOverRunEvent(void *sender, BfdAiEventArgs *args, void *userParam)
{
    MainWindow* mainWindow = (MainWindow*)userParam;

    if (mainWindow) {
        QMetaObject::invokeMethod(mainWindow, [mainWindow]() {
            QMessageBox::warning(mainWindow, "Warning", "AI Overrun detected!");
            mainWindow->ui->lblStatus->setText("Status: AI Overrun detected!");
            mainWindow->ui->lblStatus->setStyleSheet("color: red");
        }, Qt::QueuedConnection);
    }
}

void BDAQCALL MainWindow::OnCacheOverflowEvent(void *sender, BfdAiEventArgs *args, void *userParam)
{
    MainWindow* mainWindow = (MainWindow*)userParam;

    if (mainWindow) {
        QMetaObject::invokeMethod(mainWindow, [mainWindow]() {
            QMessageBox::warning(mainWindow, "Warning", "AI Cache Overflow detected!");
            mainWindow->ui->lblStatus->setText("Status: AI Cache Overflow detected!");
            mainWindow->ui->lblStatus->setStyleSheet("color: red");
        }, Qt::QueuedConnection);
    }
}

void BDAQCALL MainWindow::OnStoppedEvent(void *sender, BfdAiEventArgs *args, void *userParam)
{
    MainWindow* mainWindow = (MainWindow*)userParam;

    if (mainWindow) {
        QMetaObject::invokeMethod(mainWindow, [mainWindow]() {
            mainWindow->ui->lblStatus->setText("Status: AI Acquisition Stopped");
            mainWindow->ui->lblStatus->setStyleSheet("color: black");
            mainWindow->ui->btnStart->setEnabled(true);
            mainWindow->ui->btnStop->setEnabled(false);
        }, Qt::QueuedConnection);
    }
}