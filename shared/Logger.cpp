#include "Logger.h"
#include <QDateTime>
#include <QDebug>

// 单例实例获取
Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

// 构造函数
Logger::Logger(QObject* parent)
    : QObject(parent)
{
}

// 析构函数
Logger::~Logger() {
    // 清理资源
}

// 记录信息日志
void Logger::logInfo(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logMessage = QString("[INFO][%1] %2").arg(timestamp).arg(message);
    qDebug().noquote() << logMessage;
    
    emit logMessageReceived(logMessage, false);
}

// 记录错误日志
void Logger::logError(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logMessage = QString("[ERROR][%1] %2").arg(timestamp).arg(message);
    qDebug().noquote() << logMessage;
    
    emit logMessageReceived(logMessage, true);
}

// 记录调试日志
void Logger::logDebug(const QString& message) {
#ifdef QT_DEBUG
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logMessage = QString("[DEBUG][%1] %2").arg(timestamp).arg(message);
    qDebug().noquote() << logMessage;
    
    emit logMessageReceived(logMessage, false);
#endif
} 