#include "AppConfig.h"
#include <QStandardPaths>

AppConfig& AppConfig::instance()
{
    static AppConfig config;
    return config;
}

AppConfig::AppConfig(QObject *parent)
    : QObject(parent)
    , m_settings("config.ini", QSettings::IniFormat)
{
    load();
}

AppConfig::~AppConfig()
{
    save();
}

void AppConfig::load()
{
    m_settings.beginGroup("Network");
    m_settings.endGroup();
    
    m_settings.beginGroup("Transfer");
    m_settings.endGroup();
    
    m_settings.beginGroup("Resume");
    m_settings.endGroup();
    
    m_settings.beginGroup("UI");
    m_settings.endGroup();
}

void AppConfig::save()
{
    m_settings.sync();
}

QString AppConfig::lastHost() const
{
    return m_settings.value("Network/LastHost", "localhost").toString();
}

void AppConfig::setLastHost(const QString& host)
{
    m_settings.setValue("Network/LastHost", host);
}

quint16 AppConfig::lastPort() const
{
    return m_settings.value("Network/LastPort", 21).toUInt();
}

void AppConfig::setLastPort(quint16 port)
{
    m_settings.setValue("Network/LastPort", port);
}

int AppConfig::maxRetryCount() const
{
    return m_settings.value("Transfer/MaxRetryCount", 3).toInt();
}

void AppConfig::setMaxRetryCount(int count)
{
    m_settings.setValue("Transfer/MaxRetryCount", count);
}

int AppConfig::retryInterval() const
{
    return m_settings.value("Transfer/RetryInterval", 1000).toInt();
}

void AppConfig::setRetryInterval(int msec)
{
    m_settings.setValue("Transfer/RetryInterval", msec);
}

qint64 AppConfig::speedLimit() const
{
    return m_settings.value("Transfer/SpeedLimit", 0).toLongLong();
}

void AppConfig::setSpeedLimit(qint64 bytesPerSecond)
{
    m_settings.setValue("Transfer/SpeedLimit", bytesPerSecond);
}

bool AppConfig::autoResume() const
{
    return m_settings.value("Resume/AutoResume", true).toBool();
}

void AppConfig::setAutoResume(bool enable)
{
    m_settings.setValue("Resume/AutoResume", enable);
}

qint64 AppConfig::minResumeSize() const
{
    return m_settings.value("Resume/MinSize", 1024 * 1024).toLongLong(); // 默认1MB
}

void AppConfig::setMinResumeSize(qint64 bytes)
{
    m_settings.setValue("Resume/MinSize", bytes);
}

QString AppConfig::defaultLocalPath() const
{
    return m_settings.value("UI/DefaultLocalPath", 
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
}

void AppConfig::setDefaultLocalPath(const QString& path)
{
    m_settings.setValue("UI/DefaultLocalPath", path);
}

bool AppConfig::showHiddenFiles() const
{
    return m_settings.value("UI/ShowHiddenFiles", false).toBool();
}

void AppConfig::setShowHiddenFiles(bool show)
{
    m_settings.setValue("UI/ShowHiddenFiles", show);
    emit configChanged();  // 发送配置变更信号
} 