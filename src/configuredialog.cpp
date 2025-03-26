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

    // Clean up temporary objects
    waveformAiCtrl->Dispose();
    supportedAiDevices->Dispose();
    instantAoCtrl->Dispose();
    supportedAoDevices->Dispose();
}

void ConfigureDialog::CheckError(ErrorCode errorCode)
{
    if (errorCode >= 0xE0000000 && errorCode != Success)
    {
        QString message = tr("Error: 0x") + QString::number(errorCode, 16).right(8).toUpper();
        QMessageBox::information(this, "Error", message);
    }
}

void ConfigureDialog::AIDeviceChanged(int index)
{
    if (index < 0 || ui->cmbAIDevice->count() == 0) return;

    // Clear combo boxes
    ui->aiCmbChannelCount->clear();
    ui->aiCmbChannelStart->clear();
    ui->aiCmbValueRange->clear();

    // Get selected device info
    std::wstring description = ui->cmbAIDevice->currentText().toStdWString();
    DeviceInformation selected(description.c_str());

    // Create temporary AI control to query device capabilities
    WaveformAiCtrl *waveformAiCtrl = WaveformAiCtrl::Create();
    ErrorCode errorCode = waveformAiCtrl->setSelectedDevice(selected);

    if (errorCode == Success)
    {
        // Device supports AI, populate options
        int channelCount = (waveformAiCtrl->getChannelCount() < 16) ?
            waveformAiCtrl->getChannelCount() : 16;
        int logicChannelCount = waveformAiCtrl->getChannelCount();

        // Channel start options
        for (int i = 0; i < logicChannelCount; i++)
        {
            ui->aiCmbChannelStart->addItem(QString("%1").arg(i));
        }

        // Channel count options
        for (int i = 0; i < channelCount; i++)
        {
            ui->aiCmbChannelCount->addItem(QString("%1").arg(i + 1));
        }

        // Value range options
        Array<ValueRange> *aiValueRanges = waveformAiCtrl->getFeatures()->getValueRanges();
        wchar_t vrgDescription[128];
        MathInterval ranges;
        ValueUnit valueUnit;

        for(int i = 0; i < aiValueRanges->getCount(); i++)
        {
            errorCode = AdxGetValueRangeInformation(aiValueRanges->getItem(i),
                sizeof(vrgDescription), vrgDescription, &ranges, &valueUnit);
            CheckError(errorCode);

            QString str = QString::fromWCharArray(vrgDescription);
            ui->aiCmbValueRange->addItem(str);
        }

        // Set defaults
        ui->aiCmbChannelStart->setCurrentIndex(0);
        ui->aiCmbChannelCount->setCurrentIndex(1); // Default to 2 channels
        ui->aiCmbValueRange->setCurrentIndex(0);

        ui->tabWidget->setTabEnabled(0, true);
    }
    else
    {
        ui->tabWidget->setTabEnabled(0, false);
    }

    // Enable OK if any tab is enabled
    ui->btnOK->setEnabled(ui->tabWidget->isTabEnabled(0) || ui->tabWidget->isTabEnabled(1));

    // Switch to an enabled tab if current is disabled
    if (!ui->tabWidget->isTabEnabled(ui->tabWidget->currentIndex()))
    {
        if (ui->tabWidget->isTabEnabled(0))
            ui->tabWidget->setCurrentIndex(0);
        else if (ui->tabWidget->isTabEnabled(1))
            ui->tabWidget->setCurrentIndex(1);
    }

    // Clean up
    waveformAiCtrl->Dispose();
}

void ConfigureDialog::AODeviceChanged(int index)
{
    if (index < 0 || ui->cmbAODevice->count() == 0) return;

    // Clear combo boxes
    ui->aoCmbChannelCount->clear();
    ui->aoCmbChannelStart->clear();
    ui->aoCmbValueRange->clear();

    // Get selected device
    std::wstring description = ui->cmbAODevice->currentText().toStdWString();
    DeviceInformation selected(description.c_str());

    // Create temporary AO control to query device capabilities
    InstantAoCtrl *instantAoCtrl = InstantAoCtrl::Create();
    ErrorCode errorCode = instantAoCtrl->setSelectedDevice(selected);

    if (errorCode == Success)
    {
        // Device supports AO, populate options
        int channelCount = (instantAoCtrl->getChannelCount() < 4) ?
            instantAoCtrl->getChannelCount() : 4;
        int logicChannelCount = instantAoCtrl->getChannelCount();

        // Channel start options
        for (int i = 0; i < logicChannelCount; i++)
        {
            ui->aoCmbChannelStart->addItem(QString("%1").arg(i));
        }

        // Channel count options
        for (int i = 0; i < channelCount; i++)
        {
            ui->aoCmbChannelCount->addItem(QString("%1").arg(i + 1));
        }

        // Value range options
        Array<ValueRange> *aoValueRanges = instantAoCtrl->getFeatures()->getValueRanges();
        wchar_t vrgDescription[128];
        MathInterval ranges;

        for (int i = 0; i < aoValueRanges->getCount(); i++)
        {
            if (aoValueRanges->getItem(i) < UserCustomizedVrgStart) {
                errorCode = AdxGetValueRangeInformation(aoValueRanges->getItem(i),
                    sizeof(vrgDescription), vrgDescription, &ranges, NULL);
                CheckError(errorCode);
                QString str = QString::fromWCharArray(vrgDescription);
                ui->aoCmbValueRange->addItem(str);
            }
        }

        // Set defaults
        ui->aoCmbChannelStart->setCurrentIndex(0);
        ui->aoCmbChannelCount->setCurrentIndex(1); // Default to 2 channels
        if (ui->aoCmbValueRange->count() > 0) {
            ui->aoCmbValueRange->setCurrentIndex(0);
        }

        ui->tabWidget->setTabEnabled(1, true);
    }
    else
    {
        ui->tabWidget->setTabEnabled(1, false);
    }

    // Enable OK if any tab is enabled
    ui->btnOK->setEnabled(ui->tabWidget->isTabEnabled(0) || ui->tabWidget->isTabEnabled(1));

    // Switch to an enabled tab if current is disabled
    if (!ui->tabWidget->isTabEnabled(ui->tabWidget->currentIndex()))
    {
        if (ui->tabWidget->isTabEnabled(0))
            ui->tabWidget->setCurrentIndex(0);
        else if (ui->tabWidget->isTabEnabled(1))
            ui->tabWidget->setCurrentIndex(1);
    }

    // Clean up
    instantAoCtrl->Dispose();
}

void ConfigureDialog::TabChanged(int index)
{
    // Adjust dialog layout based on tab
    // Just a placeholder for now
}

void ConfigureDialog::ButtonOKClicked()
{
    if (ui->cmbAIDevice->count() == 0 && ui->cmbAODevice->count() == 0)
    {
        // No devices, reject
        reject();
        return;
    }

    // Get AI settings if available
    if (ui->tabWidget->isTabEnabled(0))
    {
        configure.aiDeviceName = ui->cmbAIDevice->currentText();
        configure.aiProfilePath = ui->txtAIProfilePath->text();

        std::wstring description = ui->cmbAIDevice->currentText().toStdWString();
        DeviceInformation selected(description.c_str());

        WaveformAiCtrl *waveformAiCtrl = WaveformAiCtrl::Create();
        ErrorCode errorCode = waveformAiCtrl->setSelectedDevice(selected);
        CheckError(errorCode);

        Array<ValueRange> *aiValueRanges = waveformAiCtrl->getFeatures()->getValueRanges();
        configure.aiChannelCount = ui->aiCmbChannelCount->currentText().toInt();
        configure.aiChannelStart = ui->aiCmbChannelStart->currentText().toInt();
        configure.aiValueRange = aiValueRanges->getItem(ui->aiCmbValueRange->currentIndex());
        configure.clockRatePerChan = ui->edtClockRatePerChan->text().toDouble();
        configure.sectionLength = ui->edtSectionLength->text().toInt();

        waveformAiCtrl->Dispose();
    }
    else
    {
        // Default AI settings if not available
        configure.aiDeviceName = "";
        configure.aiChannelCount = 0;
    }

    // Get AO settings if available
    if (ui->tabWidget->isTabEnabled(1))
    {
        configure.aoDeviceName = ui->cmbAODevice->currentText();
        configure.aoProfilePath = ui->txtAOProfilePath->text();

        std::wstring description = ui->cmbAODevice->currentText().toStdWString();
        DeviceInformation selected(description.c_str());

        InstantAoCtrl *instantAoCtrl = InstantAoCtrl::Create();
        ErrorCode errorCode = instantAoCtrl->setSelectedDevice(selected);
        CheckError(errorCode);

        Array<ValueRange> *aoValueRanges = instantAoCtrl->getFeatures()->getValueRanges();
        configure.aoChannelCount = ui->aoCmbChannelCount->currentText().toInt();
        configure.aoChannelStart = ui->aoCmbChannelStart->currentText().toInt();
        
        if (ui->aoCmbValueRange->currentIndex() < aoValueRanges->getCount()) {
            configure.aoValueRange = aoValueRanges->getItem(ui->aoCmbValueRange->currentIndex());
        } else {
            configure.aoValueRange = V_ExternalRefBipolar;
        }

        instantAoCtrl->Dispose();
    }
    else
    {
        // Default AO settings if not available
        configure.aoDeviceName = "";
        configure.aoChannelCount = 0;
    }

    accept();
}

void ConfigureDialog::ButtonCancelClicked()
{
    reject();
}

void ConfigureDialog::AIButtonBrowseClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, 
                                                  tr("Open AI Profile"), 
                                                  "../../profile", 
                                                  tr("Profile Files (*.xml)"));
    if (!filePath.isEmpty()) {
        ui->txtAIProfilePath->setText(filePath);
        configure.aiProfilePath = filePath;
    }
}

void ConfigureDialog::AOButtonBrowseClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, 
                                                  tr("Open AO Profile"), 
                                                  "../../profile", 
                                                  tr("Profile Files (*.xml)"));
    if (!filePath.isEmpty()) {
        ui->txtAOProfilePath->setText(filePath);
        configure.aoProfilePath = filePath;
    }
}

void ConfigureDialog::RefreshConfigureParameter()
{
    // Refresh AI parameters if available
    if (!configure.aiDeviceName.isEmpty())
    {
        std::wstring description = configure.aiDeviceName.toStdWString();
        DeviceInformation selected(description.c_str());

        WaveformAiCtrl *waveformAiCtrl = WaveformAiCtrl::Create();
        ErrorCode errorCode = waveformAiCtrl->setSelectedDevice(selected);
        if (errorCode == Success)
        {
            ui->edtClockRatePerChan->setText(QString::number(waveformAiCtrl->getConversion()->getClockRate(), 'f', 0));
            ui->edtSectionLength->setText(QString::number(waveformAiCtrl->getRecord()->getSectionLength(), 'f', 0));
        }
        waveformAiCtrl->Dispose();
    }
}
