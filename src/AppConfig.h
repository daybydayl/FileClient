#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QObject>
#include <QSettings>
#include <QString>

/**
 * @brief 应用程序配置管理类
 * 
 * 负责:
 * 1. 管理和持久化应用程序配置
 * 2. 提供配置的读写接口
 * 3. 处理配置变更通知
 * 4. 维护默认配置值
 */
class AppConfig : public QObject
{
    Q_OBJECT
    
public:
    static AppConfig& instance();
    
    // 网络设置
    QString lastHost() const;
    void setLastHost(const QString& host);
    quint16 lastPort() const;
    void setLastPort(quint16 port);
    
    // 传输设置
    int maxRetryCount() const;
    void setMaxRetryCount(int count);
    int retryInterval() const;
    void setRetryInterval(int msec);
    qint64 speedLimit() const;
    void setSpeedLimit(qint64 bytesPerSecond);
    
    // 断点续传设置
    bool autoResume() const;
    void setAutoResume(bool enable);
    qint64 minResumeSize() const;
    void setMinResumeSize(qint64 bytes);
    
    // 界面设置
    QString defaultLocalPath() const;
    void setDefaultLocalPath(const QString& path);
    bool showHiddenFiles() const;
    void setShowHiddenFiles(bool show);

signals:
    void configChanged();  // 添加配置变更信号

private:
    explicit AppConfig(QObject *parent = nullptr);
    ~AppConfig();
    
    void load();
    void save();
    
    QSettings m_settings;
};

#endif // APPCONFIG_H 