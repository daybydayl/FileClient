#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>

// 定义日志宏
#define LOG_INFO(msg) Logger::instance().logInfo(msg)
#define LOG_ERROR(msg) Logger::instance().logError(msg)
#define LOG_DEBUG(msg) Logger::instance().logDebug(msg)

/**
 * @brief 日志管理类
 * 
 * 负责:
 * 1. 提供统一的日志记录接口
 * 2. 支持不同级别的日志(信息、错误、调试)
 * 3. 管理日志的输出格式和存储
 */
class Logger : public QObject
{
    Q_OBJECT
    
public:
    static Logger& instance();
    
    void logInfo(const QString& message);
    void logError(const QString& message);
    void logDebug(const QString& message);

signals:
    void logMessageReceived(const QString& message, bool isError);
    
private:
    explicit Logger(QObject* parent = nullptr);
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#endif // LOGGER_H 