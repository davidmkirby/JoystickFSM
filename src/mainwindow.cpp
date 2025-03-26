#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configuredialog.h"
#include "joystick.h"
#include "joystick_factory.h"
#include "widgets/simplegraph.h"
#include "widgets/axis_widget.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QPalette>
#include <QButtonGroup>
#include <QList>
#include <QDebug>
#include <QTimer>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    configureDialog(nullptr),
    waveformAiCtrl(nullptr),
    scaledData(nullptr),
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
    timer(nullptr),
    joystickWidget(nullptr)
{
    ui->setupUi(this);
    
    // Set up the graph for AI visualization
    graph = new SimpleGraph(ui->graphFrame);
    graph->setFixedSize(ui->graphFrame->size());
    
    // Create the joystick visualization widget
    joystickWidget = new AxisWidget(200, 200);
    QVBoxLayout* joystickFrameLayout = new QVBoxLayout(ui->joystickFrame);
    joystickFrameLayout->addWidget(joystickWidget);
    
    // Set up timer for regular updates
    timer = new QTimer(this);
    
    // Initialize DAQ control handles
    waveformAiCtrl = WaveformAiCtrl::Create();
    instantAoCtrl = InstantAoCtrl::Create();
    
    // Register Advantech AI event handlers
    waveformAiCtrl->addDataReadyHandler(OnDataReadyEvent, this);
    waveformAiCtrl->addOverrunHandler(OnOverRunEvent, this);
    waveformAiCtrl->addCacheOverflowHandler(OnCacheOverflowEvent, this);
    waveformAiCtrl->addStoppedHandler(OnStoppedEvent, this);
    
    // Connect UI signals
    connect(ui->btnConfiguration, &QPushButton::clicked, this, &MainWindow::ButtonConfigureClicked);
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::ButtonStartClicked);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::ButtonStopClicked);
    connect(ui->btnCenter, &QPushButton::clicked, this, &MainWindow::ButtonCenterClicked);
    connect(ui->btnJoystickRefresh, &QPushButton::clicked, this, &MainWindow::JoystickRefreshClicked);
    connect(ui->btnJoystickCalibrate, &QPushButton::clicked, this, &MainWindow::JoystickCalibrateClicked);
    
    // Connect timer for updates
    connect(timer, &QTimer::timeout, this, &MainWindow::TimerTicked);
    
    // Connect menu actions
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::OnMenuExit);
    connect(ui->actionConfigure, &QAction::triggered, this, &MainWindow::OnMenuConfigure);
    connect(ui->actionJoystickTest, &QAction::triggered, this, &MainWindow::OnMenuJoystickTest);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::OnMenuAbout);
    
    // Connect settings signals
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
    connect(ui->chkInvertX, &QCheckBox::toggled,
            this, &MainWindow::OnInvertXChanged);
    connect(ui->chkInvertY, &QCheckBox::toggled,
            this, &MainWindow::OnInvertYChanged);
    connect(ui->spinXScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::OnXScaleChanged);
    connect(ui->spinYScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::OnYScaleChanged);
    connect(ui->spinDeadzone, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::OnDeadzoneChanged);
    
    // Initial UI state
    ui->btnStart->setEnabled(false);
    ui->btnStop->setEnabled(false);
    ui->btnCenter->setEnabled(false);
    ui->btnJoystickCalibrate->setEnabled(false);
}

MainWindow::~MainWindow()
{
    // Stop the timer if it's running
    if (timer) {
        timer->stop();
    }
    
    // Clean up AI resources
    if (waveformAiCtrl) {
        waveformAiCtrl->Dispose();
        waveformAiCtrl = nullptr;
    }
    
    if (scaledData) {
        delete[] scaledData;
        scaledData = nullptr;
    }
    
    // Clean up AO resources
    if (instantAoCtrl) {
        instantAoCtrl->Dispose();
        instantAoCtrl = nullptr;
    }
    
    // Clean up the graph
    if (graph) {
        delete graph;
        graph = nullptr;
    }
    
    delete ui;
}

void MainWindow::Initialize()
{
    // Set window title
    QString title = "Joystick FSM Control";
    if (!configure.aiDeviceName.isEmpty() && !configure.aoDeviceName.isEmpty()) {
        title += " (AI: " + configure.aiDeviceName + ", AO: " + configure.aoDeviceName + ")";
    } else if (!configure.aiDeviceName.isEmpty()) {
        title += " (AI: " + configure.aiDeviceName + ")";
    } else if (!configure.aoDeviceName.isEmpty()) {
        title += " (AO: " + configure.aoDeviceName + ")";
    }
    setWindowTitle(title);
    
    // Configure devices
    ConfigureDevice();
    
    // Update UI settings
    UpdateUI();
    
    // Refresh joystick list
    RefreshJoystickList();
    
    // Initialize the graph
    if (!configure.aiDeviceName.isEmpty()) {
        ConfigureGraph();
    }
    
    // Start the timer for regular updates
    timer->start(20); // 50Hz update rate
    
    // Enable buttons based on configuration
    ui->btnStart->setEnabled(!configure.aiDeviceName.isEmpty() || !configure.aoDeviceName.isEmpty());
    ui->btnConfiguration->setEnabled(true);
}

void MainWindow::ConfigureDevice()
{
    // Configure both AI and AO
    ConfigureAI();
    ConfigureAO();
}

void MainWindow::ConfigureAI()
{
    if (configure.aiDeviceName.isEmpty()) return;
    
    ErrorCode errorCode = Success;
    
    // Clean up existing buffer
    if (scaledData) {
        delete[] scaledData;
        scaledData = nullptr;
    }
    
    // Allocate buffer for AI data
    rawDataBufferLength = configure.sectionLength * configure.aiChannelCount;
    scaledData = new double[rawDataBufferLength];
    if (!scaledData) {
        QMessageBox::critical(this, "Error", "Failed to allocate memory for AI data buffer");
        return;
    }
    
    // Select the AI device
    std::wstring description = configure.aiDeviceName.toStdWString();
    DeviceInformation selected(description.c_str());
    
    errorCode = waveformAiCtrl->setSelectedDevice(selected);
    CheckError(errorCode);
    
    // Load profile if specified
    if (!configure.aiProfilePath.isEmpty()) {
        std::wstring profile = configure.aiProfilePath.toStdWString();
        errorCode = waveformAiCtrl->LoadProfile(profile.c_str());
        CheckError(errorCode);
    }
    
    // Configure AI settings
    errorCode = waveformAiCtrl->getConversion()->setChannelCount(configure.aiChannelCount);
    CheckError(errorCode);
    
    errorCode = waveformAiCtrl->getConversion()->setChannelStart(configure.aiChannelStart);
    CheckError(errorCode);
    
    errorCode = waveformAiCtrl->getConversion()->setClockRate(configure.clockRatePerChan);
    CheckError(errorCode);
    
    errorCode = waveformAiCtrl->getRecord()->setSectionLength(configure.sectionLength);
    CheckError(errorCode);
    
    errorCode = waveformAiCtrl->getRecord()->setSectionCount(0); // Streaming mode
    CheckError(errorCode);
    
    // Set value range for all channels
    for (int i = 0; i < waveformAiCtrl->getChannels()->getCount(); i++) {
        errorCode = waveformAiCtrl->getChannels()->getItem(i).setValueRange(configure.aiValueRange);
        CheckError(errorCode);
    }
    
    // Prepare the device
    errorCode = waveformAiCtrl->Prepare();
    CheckError(errorCode);
    
    // Update UI
    ui->lblAIDeviceValue->setText(configure.aiDeviceName);
    ui->lblAIChanValue->setText(QString("%1 - %2").arg(configure.aiChannelStart)
                                .arg(configure.aiChannelStart + configure.aiChannelCount - 1));
    ui->lblAIRateValue->setText(QString("%1 Hz").arg(configure.clockRatePerChan));
}

void MainWindow::ConfigureAO()
{
    if (configure.aoDeviceName.isEmpty()) return;
    
    ErrorCode errorCode = Success;
    
    // Select the AO device
    std::wstring description = configure.aoDeviceName.toStdWString();
    DeviceInformation selected(description.c_str());
    
    errorCode = instantAoCtrl->setSelectedDevice(selected);
    CheckError(errorCode);
    
    // Load profile if specified
    if (!configure.aoProfilePath.isEmpty()) {
        std::wstring profile = configure.aoProfilePath.toStdWString();
        errorCode = instantAoCtrl->LoadProfile(profile.c_str());
        CheckError(errorCode);
    }
    
    // Set value range for all channels
    for (int i = 0; i < instantAoCtrl->getChannels()->getCount(); i++) {
        errorCode = instantAoCtrl->getChannels()->getItem(i).setValueRange(configure.aoValueRange);
        CheckError(errorCode);
    }
    
    // Store channel settings
    aoChannelStart = configure.aoChannelStart;
    aoChannelCount = configure.aoChannelCount;
    
    // Update UI
    ui->lblAODeviceValue->setText(configure.aoDeviceName);
    ui->lblAOChanValue->setText(QString("%1 - %2").arg(configure.aoChannelStart)
                                .arg(configure.aoChannelStart + configure.aoChannelCount - 1));
    
    // Update channel selection combos
    ui->cmbXChannel->clear();
    ui->cmbYChannel->clear();
    
    for (int i = 0; i < aoChannelCount; i++) {
        QString channelText = QString("Channel %1").arg(aoChannelStart + i);
        ui->cmbXChannel->addItem(channelText);
        ui->cmbYChannel->addItem(channelText);
    }
    
    // Set defaults
    if (aoChannelCount >= 1) {
        ui->cmbXChannel->setCurrentIndex(0);
        xChannelMapping = 0;
    }
    
    if (aoChannelCount >= 2) {
        ui->cmbYChannel->setCurrentIndex(1);
        yChannelMapping = 1;
    }
}

void MainWindow::ConfigureGraph()
{
    double clockRate = waveformAiCtrl->getConversion()->getClockRate();
    int tUnit = (int)Millisecond;
    double timeInterval = 100.0 * graph->rect().width() / clockRate;
    
    while (clockRate >= 10 * 1000) {
        timeInterval *= 1000;
        clockRate /= 1000;
        --tUnit;
    }
    
    timeUnit = (TimeUnit)tUnit;
    
    int divValue = (int)timeInterval;
    int divMin = divValue / 10;
    if (divMin == 0) {
        divMin = 1;
    }
    
    SetXCord();
    
    ValueUnit unit;
    MathInterval rangeY;
    QString yRanges[3];
    
    ErrorCode errorCode = AdxGetValueRangeInformation(configure.aiValueRange, 0, NULL,
                                                     &rangeY, &unit);
    CheckError(errorCode);
    
    graph->GetYCordRange(yRanges, rangeY.Max, rangeY.Min, unit);
    ui->lblYCoordinateMax->setText(yRanges[0]);
    ui->lblYCoordinateMin->setText(yRanges[1]);
    
    graph->m_yCordRangeMax = rangeY.Max;
    graph->m_yCordRangeMin = rangeY.Min;
    graph->Clear();
}

void MainWindow::SetXCord()
{
    int divValue = 200; // Default division value
    graph->m_xCordTimeDiv = (double)divValue;
    
    QString xRanges[2];
    double shiftMaxValue = qRound(graph->m_xCordTimeDiv * 10 + graph->m_xCordTimeOffset);
    
    graph->GetXCordRange(xRanges, shiftMaxValue, graph->m_xCordTimeOffset, timeUnit);
    ui->lblXCoordinateStart->setText(xRanges[1]);
    ui->lblXCoordinateEnd->setText(xRanges[0]);
}

void MainWindow::DivValueChanged(int value)
{
    graph->Div(value);
    SetXCord();
}

void MainWindow::CheckError(ErrorCode errorCode)
{
    if (BioFailed(errorCode)) {
        QString message = tr("Error: 0x") + QString::number(errorCode, 16).right(8).toUpper();
        QMessageBox::critical(this, "Error", message);
    }
}

void MainWindow::ButtonConfigureClicked()
{
    // Stop AI if running
    if (!configure.aiDeviceName.isEmpty() && ui->btnStop->isEnabled()) {
        ButtonStopClicked();
    }
    
    // Show configuration dialog
    if (configureDialog) {
        int result = configureDialog->exec();
        if (result == QDialog::Accepted) {
            configure = configureDialog->GetConfigureParameter();
            Initialize();
        }
    }
}

void MainWindow::ButtonStartClicked()
{
    // Disable configure button
    ui->btnConfiguration->setEnabled(false);
    ui->btnStart->setEnabled(false);
    ui->btnStop->setEnabled(true);
    
    // Start AI acquisition if configured
    if (!configure.aiDeviceName.isEmpty()) {
        ErrorCode errorCode = waveformAiCtrl->Start();
        CheckError(errorCode);
        xInc = 1.0 / waveformAiCtrl->getConversion()->getClockRate();
    }
    
    // Update status
    ui->lblStatus->setText("Status: Running");
}

void MainWindow::ButtonStopClicked()
{
    // Stop AI acquisition if running
    if (!configure.aiDeviceName.isEmpty()) {
        ErrorCode errorCode = waveformAiCtrl->Stop();
        CheckError(errorCode);
        graph->Clear();
    }
    
    // Update UI
    ui->btnConfiguration->setEnabled(true);
    ui->btnStart->setEnabled(true);
    ui->btnStop->setEnabled(false);
    
    // Update status
    ui->lblStatus->setText("Status: Stopped");
}

void MainWindow::ButtonCenterClicked()
{
    // Set joystick values to zero
    xAxisValue = 0.0;
    yAxisValue = 0.0;
    
    // Update the joystick widget
    joystickWidget->setXAxis(0.0);
    joystickWidget->setYAxis(0.0);
    
    // Send zeros to both channels
    UpdateMirrorPosition(0.0, 0.0);
}

void MainWindow::TimerTicked()
{
    // Update the AO outputs based on joystick position
    if (!configure.aoDeviceName.isEmpty() && joystick) {
        // Apply the axis values to the outputs
        UpdateMirrorPosition(xAxisValue, yAxisValue);
    }
}

void MainWindow::UpdateMirrorPosition(double x, double y)
{
    // Apply scaling and inversion
    double scaledX = x * xScale * (invertX ? -1.0 : 1.0);
    double scaledY = y * yScale * (invertY ? -1.0 : 1.0);
    
    // Limit to (-1, 1) range
    scaledX = std::max(-1.0, std::min(1.0, scaledX));
    scaledY = std::max(-1.0, std::min(1.0, scaledY));
    
    // Convert normalized values to voltage
    // Assuming a bipolar output range (e.g., -10V to +10V)
    ValueUnit unit;
    MathInterval rangeY;
    
    ErrorCode errorCode = AdxGetValueRangeInformation(configure.aoValueRange, 0, NULL,
                                                     &rangeY, &unit);
    if (BioFailed(errorCode)) {
        return;
    }
    
    // Map from [-1, 1] to [min, max] voltage range
    double halfRange = (rangeY.Max - rangeY.Min) / 2.0;
    double midPoint = (rangeY.Max + rangeY.Min) / 2.0;
    
    double xVoltage = midPoint + (scaledX * halfRange);
    double yVoltage = midPoint + (scaledY * halfRange);
    
    // Update voltage display
    ui->lblXVoltage->setText(QString("X Voltage: %1V").arg(xVoltage, 0, 'f', 2));
    ui->lblYVoltage->setText(QString("Y Voltage: %1V").arg(yVoltage, 0, 'f', 2));
    
    // Send values to AO channels
    aoData[xChannelMapping] = xVoltage;
    aoData[yChannelMapping] = yVoltage;
    
    // Write to device
    errorCode = instantAoCtrl->Write(aoChannelStart, aoChannelCount, aoData);
    CheckError(errorCode);
}

void MainWindow::RefreshJoystickList()
{
    // Disconnect current joystick if any
    joystick.reset();
    
    // Clear dropdown
    ui->cmbJoystick->clear();
    
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
    
    // Get list of joysticks
    std::vector<JoystickDescription> joysticks = JoystickFactory::getJoysticks(backend);
    
    // Populate dropdown
    for (const auto& js : joysticks) {
        QString label = QString::fromStdString(js.name) + 
                       " (" + QString::fromStdString(js.filename) + ")";
        ui->cmbJoystick->addItem(label, QString::fromStdString(js.filename));
    }
    
    // Update UI
    if (joysticks.empty()) {
        ui->joystickLabel->setText("No joystick detected");
        ui->btnJoystickCalibrate->setEnabled(false);
    } else {
        // Select the first joystick
        ui->cmbJoystick->setCurrentIndex(0);
        ConnectJoystick(0);
    }
}

void MainWindow::ConnectJoystick(int index)
{
    if (index < 0 || index >= ui->cmbJoystick->count()) {
        ui->joystickLabel->setText("No joystick selected");
        ui->btnJoystickCalibrate->setEnabled(false);
        return;
    }
    
    // Get joystick path
    QString path = ui->cmbJoystick->itemData(index).toString();
    
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
    
    try {
        // Create the joystick
        joystick = JoystickFactory::createJoystick(path.toStdString(), backend);
        
        // Initialize axis and button lists
        joystickAxes.resize(joystick->getAxisCount());
        joystickButtons.resize(joystick->getButtonCount());
        
        // Clear values
        joystickAxes.fill(0.0);
        joystickButtons.fill(false);
        
        // Connect signals
        connect(joystick.get(), &Joystick::axisChanged, this, &MainWindow::OnJoystickAxisChanged);
        connect(joystick.get(), &Joystick::buttonChanged, this, &MainWindow::OnJoystickButtonChanged);
        
        // Update UI
        ui->joystickLabel->setText(QString("Connected: %1 (%2 axes, %3 buttons)")
                                 .arg(joystick->getName())
                                 .arg(joystick->getAxisCount())
                                 .arg(joystick->getButtonCount()));
        
        ui->btnJoystickCalibrate->setEnabled(true);
        
        // Update axis mappings
        ui->cmbXAxis->clear();
        ui->cmbYAxis->clear();
        
        for (int i = 0; i < joystick->getAxisCount(); i++) {
            QString axisName = QString("Axis %1").arg(i);
            ui->cmbXAxis->addItem(axisName);
            ui->cmbYAxis->addItem(axisName);
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
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to open joystick: %1").arg(e.what()));
        ui->joystickLabel->setText("Failed to connect joystick");
        ui->btnJoystickCalibrate->setEnabled(false);
    }
}

void MainWindow::OnJoystickAxisChanged(int number, int value)
{
    if (number < 0 || number >= joystickAxes.size()) {
        return;
    }
    
    // Convert to normalized value between -1 and 1
    double normalizedValue = value / 32767.0;
    joystickAxes[number] = normalizedValue;
    
    // Check if this is one of our mapped axes
    if (number == xAxisMapping) {
        xAxisValue = normalizedValue;
    }
    
    if (number == yAxisMapping) {
        yAxisValue = normalizedValue;
    }
    
    // Apply deadzone
    double displayX = xAxisValue;
    double displayY = yAxisValue;
    ApplyDeadzone(displayX, displayY);
    
    // Update the joystick widget
    joystickWidget->setXAxis(displayX);
    joystickWidget->setYAxis(displayY);
}

void MainWindow::OnJoystickButtonChanged(int number, bool value)
{
    if (number < 0 || number >= joystickButtons.size()) {
        return;
    }
    
    joystickButtons[number] = value;
    
    // Check for specific button functionality
    // For example, center on button 0
    if (number == 0 && value) {
        ButtonCenterClicked();
    }
}

void MainWindow::ApplyDeadzone(double &x, double &y)
{
    // Calculate distance from center
    double distance = std::sqrt(x*x + y*y);
    
    // Apply deadzone
    if (distance < deadzone) {
        // Inside deadzone, set to zero
        x = 0.0;
        y = 0.0;
    } else {
        // Outside deadzone, rescale
        double scale = (distance - deadzone) / (1.0 - deadzone);
        scale = scale / distance; // Normalize
        
        x *= scale;
        y *= scale;
    }
}

void MainWindow::UpdateUI()
{
    // Update checkboxes
    ui->chkInvertX->setChecked(invertX);
    ui->chkInvertY->setChecked(invertY);
    
    // Update spinboxes
    ui->spinXScale->setValue(xScale);
    ui->spinYScale->setValue(yScale);
    ui->spinDeadzone->setValue(deadzone);
}

void MainWindow::JoystickRefreshClicked()
{
    RefreshJoystickList();
}

void MainWindow::JoystickCalibrateClicked()
{
    if (!joystick) {
        return;
    }
    
    // Show a simple message about calibration
    QMessageBox::information(this, "Joystick Calibration",
                            "1. Move the joystick to all extreme positions\n"
                            "2. Return to center position\n"
                            "3. Press OK when done");
    
    // Get current calibration
    std::vector<Joystick::CalibrationData> origData = joystick->getCalibration();
    
    // Clear calibration
    joystick->clearCalibration();
    
    // Get new min/max values from the current state
    std::vector<int> min_vals;
    std::vector<int> max_vals;
    std::vector<int> center_vals;
    
    min_vals.resize(joystick->getAxisCount());
    max_vals.resize(joystick->getAxisCount());
    center_vals.resize(joystick->getAxisCount());
    
    // Get current values as center
    for (int i = 0; i < joystick->getAxisCount(); i++) {
        center_vals[i] = joystick->getAxisState(i);
        min_vals[i] = center_vals[i];
        max_vals[i] = center_vals[i];
    }
    
    // Wait a bit for user to move joystick
    QMessageBox calibrationBox(QMessageBox::Information, "Calibrating",
                              "Move joystick to all extreme positions...", 
                              QMessageBox::Ok | QMessageBox::Cancel);
    
    // Create a timer to update the calibration values
    QTimer calibrationTimer;
    connect(&calibrationTimer, &QTimer::timeout, [&]() {
        // Update min/max values
        for (int i = 0; i < joystick->getAxisCount(); i++) {
            int value = joystick->getAxisState(i);
            min_vals[i] = std::min(min_vals[i], value);
            max_vals[i] = std::max(max_vals[i], value);
        }
    });
    
    calibrationTimer.start(20); // 50Hz update
    
    int result = calibrationBox.exec();
    calibrationTimer.stop();
    
    if (result == QMessageBox::Ok) {
        // Create calibration data
        std::vector<Joystick::CalibrationData> data;
        
        for (int i = 0; i < joystick->getAxisCount(); i++) {
            Joystick::CalibrationData axis;
            axis.calibrate = true;
            axis.invert = false;
            axis.center_min = center_vals[i] - 100; // Add a small deadzone
            axis.center_max = center_vals[i] + 100;
            axis.range_min = min_vals[i];
            axis.range_max = max_vals[i];
            
            data.push_back(axis);
        }
        
        joystick->setCalibration(data);
    } else {
        // Restore original calibration
        joystick->setCalibration(origData);
    }
}

void MainWindow::OnJoystickSelectionChanged(int index)
{
    ConnectJoystick(index);
}

void MainWindow::OnBackendSelectionChanged(int index)
{
    // Will be applied on next refresh
    JoystickFactory::setDefaultBackend(static_cast<JoystickBackend>(index));
}

void MainWindow::OnXAxisMappingChanged(int index)
{
    if (index >= 0 && joystick && index < joystick->getAxisCount()) {
        xAxisMapping = index;
        xAxisValue = joystickAxes.value(index, 0.0);
    }
}

void MainWindow::OnYAxisMappingChanged(int index)
{
    if (index >= 0 && joystick && index < joystick->getAxisCount()) {
        yAxisMapping = index;
        yAxisValue = joystickAxes.value(index, 0.0);
    }
}

void MainWindow::OnXChannelChanged(int index)
{
    if (index >= 0 && index < aoChannelCount) {
        xChannelMapping = index;
    }
}

void MainWindow::OnYChannelChanged(int index)
{
    if (index >= 0 && index < aoChannelCount) {
        yChannelMapping = index;
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
}

void MainWindow::OnMenuExit()
{
    close();
}

void MainWindow::OnMenuConfigure()
{
    ButtonConfigureClicked();
}

void MainWindow::OnMenuJoystickTest()
{
    // Show a simple joystick test dialog
    if (!joystick) {
        QMessageBox::information(this, "Joystick Test", "No joystick connected");
        return;
    }
    
    QString message = QString("Joystick: %1\n"
                             "Axes: %2\n"
                             "Buttons: %3\n\n"
                             "Axis values are shown in the main window.\n"
                             "Test buttons by pressing them.")
                     .arg(joystick->getName())
                     .arg(joystick->getAxisCount())
                     .arg(joystick->getButtonCount());
    
    QMessageBox::information(this, "Joystick Test", message);
}

void MainWindow::OnMenuAbout()
{
    QString message = "Joystick FSM Control\n\n"
                     "A Qt application for controlling a Fast-Steering Mirror (FSM) "
                     "using a joystick and Advantech DAQ hardware.\n\n"
                     "Uses code from the following projects:\n"
                     "- Advantech CombinedAOAI example\n"
                     "- jstest-qt joystick test application";
    
    QMessageBox::about(this, "About", message);
}

// Static AI event handlers
void BDAQCALL MainWindow::OnDataReadyEvent(void* sender, BfdAiEventArgs* args, void* userParam)
{
    MainWindow* window = static_cast<MainWindow*>(userParam);
    int32 remainingCount = args->Count;
    int32 getDataCount = 0, returnedCount = 0;
    int32 bufSize = window->configure.sectionLength * window->configure.aiChannelCount;
    
    do {
        getDataCount = qMin(bufSize, remainingCount);
        ErrorCode ret = ((WaveformAiCtrl*)sender)->GetData(getDataCount, window->scaledData, 
                                                        0, &returnedCount, NULL, NULL, NULL);
        remainingCount -= returnedCount;
        
        if (ret != Success && ret != WarningRecordEnd) {
            QString message = QObject::tr("Error: 0x") + 
                            QString::number(ret, 16).right(8).toUpper();
            QMessageBox::critical(window, "Error", message);
            return;
        }
        
        // Display the data on the graph
        window->graph->Chart(window->scaledData, window->configure.aiChannelCount, 
                           returnedCount / window->configure.aiChannelCount, window->xInc);
        
    } while(remainingCount > 0);
}

void BDAQCALL MainWindow::OnOverRunEvent(void* sender, BfdAiEventArgs* args, void* userParam)
{
    // Optional error handling
}

void BDAQCALL MainWindow::OnCacheOverflowEvent(void* sender, BfdAiEventArgs* args, void* userParam)
{
    // Optional error handling
}

void BDAQCALL MainWindow::OnStoppedEvent(void* sender, BfdAiEventArgs* args, void* userParam)
{
    // Optional event handling
}
