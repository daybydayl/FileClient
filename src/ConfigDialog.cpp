#include "ConfigDialog.h"
#include <QFileDialog>
#include <QMessageBox>

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
    , m_tabWidget(nullptr)
    , m_defaultHostEdit(nullptr)
    , m_defaultPortSpin(nullptr)
    , m_retryCountSpin(nullptr)
    , m_retryIntervalSpin(nullptr)
    , m_speedLimitCombo(nullptr)
    , m_autoResumeCheck(nullptr)
    , m_minResumeSizeCombo(nullptr)
    , m_localPathEdit(nullptr)
    , m_showHiddenCheck(nullptr)
    , m_okBtn(nullptr)
    , m_cancelBtn(nullptr)
{
    setupUI();
    loadConfig();
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::setupUI()
{
    setWindowTitle(tr("配置"));
    resize(400, 300);

    m_tabWidget = new QTabWidget(this);
    
    createNetworkTab();
    createTransferTab();
    createUITab();
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_tabWidget);
    
    // 修改按钮布局，移除应用按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    m_okBtn = new QPushButton(tr("确定"), this);
    m_cancelBtn = new QPushButton(tr("取消"), this);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okBtn);
    buttonLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_okBtn, &QPushButton::clicked, this, [this]() {
        saveConfig();
        accept();
    });
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void ConfigDialog::createNetworkTab()
{
    QWidget* networkTab = new QWidget(this);
    QFormLayout* layout = new QFormLayout(networkTab);
    
    m_defaultHostEdit = new QLineEdit(networkTab);
    m_defaultPortSpin = new QSpinBox(networkTab);
    m_defaultPortSpin->setRange(1, 65535);
    
    layout->addRow(tr("默认主机:"), m_defaultHostEdit);
    layout->addRow(tr("默认端口:"), m_defaultPortSpin);
    
    m_tabWidget->addTab(networkTab, tr("网络"));
}

void ConfigDialog::createTransferTab()
{
    QWidget* transferTab = new QWidget(this);
    QFormLayout* layout = new QFormLayout(transferTab);
    
    m_retryCountSpin = new QSpinBox(transferTab);
    m_retryCountSpin->setRange(0, 10);
    
    m_retryIntervalSpin = new QSpinBox(transferTab);
    m_retryIntervalSpin->setRange(100, 10000);
    m_retryIntervalSpin->setSingleStep(100);
    m_retryIntervalSpin->setSuffix(tr(" 毫秒"));
    
    m_speedLimitCombo = new QComboBox(transferTab);
    m_speedLimitCombo->addItem(tr("不限制"), 0);
    m_speedLimitCombo->addItem(tr("1 MB/s"), 1024 * 1024);
    m_speedLimitCombo->addItem(tr("2 MB/s"), 2 * 1024 * 1024);
    m_speedLimitCombo->addItem(tr("5 MB/s"), 5 * 1024 * 1024);
    m_speedLimitCombo->addItem(tr("10 MB/s"), 10 * 1024 * 1024);
    
    m_autoResumeCheck = new QCheckBox(tr("自动断点续传"), transferTab);
    
    m_minResumeSizeCombo = new QComboBox(transferTab);
    m_minResumeSizeCombo->addItem(tr("1 MB"), 1024 * 1024);
    m_minResumeSizeCombo->addItem(tr("5 MB"), 5 * 1024 * 1024);
    m_minResumeSizeCombo->addItem(tr("10 MB"), 10 * 1024 * 1024);
    m_minResumeSizeCombo->addItem(tr("50 MB"), 50 * 1024 * 1024);
    
    layout->addRow(tr("重试次数:"), m_retryCountSpin);
    layout->addRow(tr("重试间隔:"), m_retryIntervalSpin);
    layout->addRow(tr("速度限制:"), m_speedLimitCombo);
    layout->addRow("", m_autoResumeCheck);
    layout->addRow(tr("最小续传大小:"), m_minResumeSizeCombo);
    
    m_tabWidget->addTab(transferTab, tr("传输"));
}

void ConfigDialog::createUITab()
{
    QWidget* uiTab = new QWidget(this);
    QFormLayout* layout = new QFormLayout(uiTab);
    
    QHBoxLayout* pathLayout = new QHBoxLayout;
    m_localPathEdit = new QLineEdit(uiTab);
    QPushButton* browseBtn = new QPushButton(tr("浏览..."), uiTab);
    pathLayout->addWidget(m_localPathEdit);
    pathLayout->addWidget(browseBtn);
    
    m_showHiddenCheck = new QCheckBox(tr("显示隐藏文件"), uiTab);
    
    layout->addRow(tr("默认本地路径:"), pathLayout);
    layout->addRow("", m_showHiddenCheck);
    
    connect(browseBtn, &QPushButton::clicked, this, &ConfigDialog::browseLocalPath);
    
    m_tabWidget->addTab(uiTab, tr("界面"));
}

void ConfigDialog::loadConfig()
{
    AppConfig& config = AppConfig::instance();
    
    // 网络设置
    m_defaultHostEdit->setText(config.lastHost());
    m_defaultPortSpin->setValue(config.lastPort());
    
    // 传输设置
    m_retryCountSpin->setValue(config.maxRetryCount());
    m_retryIntervalSpin->setValue(config.retryInterval());
    
    int speedLimitIndex = m_speedLimitCombo->findData(config.speedLimit());
    m_speedLimitCombo->setCurrentIndex(speedLimitIndex >= 0 ? speedLimitIndex : 0);
    
    m_autoResumeCheck->setChecked(config.autoResume());
    
    int minResumeSizeIndex = m_minResumeSizeCombo->findData(config.minResumeSize());
    m_minResumeSizeCombo->setCurrentIndex(minResumeSizeIndex >= 0 ? minResumeSizeIndex : 0);
    
    // 界面设置
    m_localPathEdit->setText(config.defaultLocalPath());
    m_showHiddenCheck->setChecked(config.showHiddenFiles());
}

void ConfigDialog::saveConfig()
{
    AppConfig& config = AppConfig::instance();
    
    // 网络设置
    config.setLastHost(m_defaultHostEdit->text());
    config.setLastPort(m_defaultPortSpin->value());
    
    // 传输设置
    config.setMaxRetryCount(m_retryCountSpin->value());
    config.setRetryInterval(m_retryIntervalSpin->value());
    config.setSpeedLimit(m_speedLimitCombo->currentData().toLongLong());
    config.setAutoResume(m_autoResumeCheck->isChecked());
    config.setMinResumeSize(m_minResumeSizeCombo->currentData().toLongLong());
    
    // 界面设置
    config.setDefaultLocalPath(m_localPathEdit->text());
    config.setShowHiddenFiles(m_showHiddenCheck->isChecked());
}

void ConfigDialog::browseLocalPath()
{
    QString dir = QFileDialog::getExistingDirectory(this,
        tr("选择默认本地路径"), m_localPathEdit->text());
    if (!dir.isEmpty()) {
        m_localPathEdit->setText(dir);
    }
} 