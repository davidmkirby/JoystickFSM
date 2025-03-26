#include "configuredialog.h"
#include "ui_configuredialog.h"
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>

#define MAXCLOCKRATE 500000000

ConfigureDialog::ConfigureDialog(QDialog *parent)
    : QDialog(parent),
    ui(new Ui::ConfigureDialog)
{
    ui->setupUi(this);

    // Set window flags
    this->setWindowFlags(Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // Connect signals
    connect(ui->cmbAIDevice, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ConfigureDialog::AIDeviceChanged);
    connect(ui->cmbAODevice, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ConfigureDialog::AODeviceChanged);
    connect(ui->btnOK, &QPushButton::clicked, this, &ConfigureDialog::ButtonOKClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &ConfigureDialog::ButtonCancelClicked);
    connect(ui->btnAIBrowse, &QPushButton::clicked, this, &ConfigureDialog::AIButtonBrowseClicked);
    connect(ui->btnAOBrowse, &QPushButton::clicked, this, &ConfigureDialog::AOButtonBrowseClicked);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &ConfigureDialog::TabChanged);

    // Joystick-specific signal connections
    connect(ui->cmbJoystickBackend, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ConfigureDialog::JoystickBackendChanged);
    connect(ui->spinDeadzone, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ConfigureDialog::DeadzoneChanged);
    connect(ui->spinXScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ConfigureDialog::XScaleChanged);
    connect(ui->spinYScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &ConfigureDialog::YScaleChanged);
    connect(ui->chkInvertX, &QCheckBox::toggled, 
            this, &ConfigureDialog::InvertXChanged);
    connect(ui->chkInvertY, &QCheckBox::toggled, 
            this, &ConfigureDialog::InvertYChanged);

    // Set the maximum value of clock rate per channel 500MHz
    ui->edtClockRatePerChan->setValidator(new QDoubleValidator(1, MAXCLOCKRATE, 2, this));

    // Initialize UI
    Initialization();
}

ConfigureDialog::~ConfigureDialog()
{
    delete ui;
}

void ConfigureDialog::Initialization()
{
    // Clear device lists
    ui->cmbAIDevice->clear();
    ui->cmbAODevice->clear();

    // Create temporary device controls to get supported devices
    WaveformAiCtrl *waveformAiCtrl = WaveformAiCtrl::Create();
    InstantAoCtrl *instantAoCtrl = InstantAoCtrl::Create();

    // Get supported device lists
    Array<DeviceTreeNode> *supportedAiDevices = waveformAiCtrl->getSupportedDevices();
    Array<DeviceTreeNode> *supportedAoDevices = instantAoCtrl->getSupportedDevices();

    // Check for available devices
    if (supportedAiDevices->getCount() == 0 && supportedAoDevices->getCount() == 0)
    {
        QMessageBox::information(this, tr("Warning Information"),
            tr("No Advantech devices found that support the required functionality."));
            
        // Disable OK button
        ui->btnOK->setEnabled(false);
    }
    else
    {
        // Add AI devices to combo box
        ui->cmbAIDevice->blockSignals(true);
        for (int i = 0; i < supportedAiDevices->getCount(); i++)
        {
            DeviceTreeNode const &node = supportedAiDevices->getItem(i);
            QString description = QString::fromWCharArray(node.Description);
            
            // Only add devices that support waveform AI
            ui->cmbAIDevice->addItem(description);
        }
        ui->cmbAIDevice->blockSignals(false);

        // Add AO devices to combo box
        ui->cmbAODevice->blockSignals(true);
        for (int i = 0; i < supportedAoDevices->getCount(); i++)
        {
            DeviceTreeNode const &node = supportedAoDevices->getItem(i);
            QString description = QString::fromWCharArray(node.Description);
            
            // Add AO devices
            ui->cmbAODevice->addItem(description);
        }
        ui->cmbAODevice->blockSignals(false);

        // Set initial selections
        if (ui->cmbAIDevice->count() > 0) {
            ui->cmbAIDevice->setCurrentIndex(0);
            AIDeviceChanged(0);
            ui->tabWidget->setTabEnabled(0, true);
        } else {
            ui->tabWidget->setTabEnabled(0, false);
        }

        if (ui->cmbAODevice->count() > 0) {
            ui->cmbAODevice->setCurrentIndex(0);
            AODeviceChanged(0);
            ui->tabWidget->setTabEnabled(1, true);
        } else {
            ui->tabWidget->setTabEnabled(1, false);
        }
    }

    // Set initial joystick configuration values
    ui->cmbJoystickBackend->setCurrentText(configure.joystickBackend);
    ui->spinDeadzone->setValue(configure.deadzone);
    ui->spinXScale->setValue(configure.xScale);
    ui->spinYScale->setValue(configure.yScale);
    ui->chkInvertX->setChecked(configure.invertX);
    ui->chkInvertY->setChecked(configure.invertY);

    // Clean up temporary objects
    waveformAiCtrl->Dispose();
    supportedAiDevices->Dispose();
    instantAoCtrl->Dispose();
    supportedAoDevices->Dispose();
}

// New Joystick-specific slot implementations
void ConfigureDialog::JoystickBackendChanged(int index)
{
    configure.joystickBackend = ui->cmbJoystickBackend->currentText();
}

void ConfigureDialog::DeadzoneChanged(double value)
{
    configure.deadzone = value;
}

void ConfigureDialog::XScaleChanged(double value)
{
    configure.xScale = value;
}

void ConfigureDialog::YScaleChanged(double value)
{
    configure.yScale = value;
}

void ConfigureDialog::InvertXChanged(bool checked)
{
    configure.invertX = checked;
}

void ConfigureDialog::InvertYChanged(bool checked)
{
    configure.invertY = checked;
}

// Existing methods would remain the same, but update ButtonOKClicked to include joystick settings
void ConfigureDialog::ButtonOKClicked()
{
    // Existing AI and AO device configuration code remains the same

    // Set joystick configuration
    configure.joystickBackend = ui->cmbJoystickBackend->currentText();
    configure.deadzone = ui->spinDeadzone->value();
    configure.xScale = ui->spinXScale->value();
    configure.yScale = ui->spinYScale->value();
    configure.invertX = ui->chkInvertX->isChecked();
    configure.invertY = ui->chkInvertY->isChecked();

    accept();
}
