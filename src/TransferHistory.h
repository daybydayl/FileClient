#ifndef TRANSFERHISTORY_H
#define TRANSFERHISTORY_H

#include <QObject>
#include <QList>
#include <QDateTime>
#include <QJsonObject>
#include "TransferQueue.h"

/**
 * @brief 传输记录数据结构
 * 
 * 负责:
 * 1. 存储单个传输任务的完整信息
 * 2. 提供数据序列化和反序列化
 * 3. 管理传输记录的时间戳
 * 4. 维护传输状态和错误信息
 */
struct TransferRecord {
    QString taskId;
    QString fileName;
    QString sourcePath;
    QString targetPath;
    qint64 fileSize;
    bool isSuccess;
    QString errorMessage;
    QDateTime timestamp;
    QDateTime startTime;
    QDateTime endTime;
    
    TransferRecord()
        : fileSize(0)
        , isSuccess(false)
    {
        timestamp = QDateTime::currentDateTime();
        startTime = timestamp;
        endTime = timestamp;
    }
    
    QJsonObject toJson() const;
    static TransferRecord fromJson(const QJsonObject& json);
};
/**
 * @brief 传输历史记录管理类
 * 
 * 负责:
 * 1. 记录文件传输历史
 * 2. 提供历史记录的查询和筛选
 * 3. 管理历史记录的持久化存储
 * 4. 维护历史记录的最大数量限制
 */
class TransferHistory : public QObject
{
    Q_OBJECT
    
public:
    static TransferHistory& instance();
    
    void addRecord(const TransferRecord& record);
    QList<TransferRecord> getRecords(const QDateTime& from = QDateTime(), 
                                   const QDateTime& to = QDateTime()) const;
    void clear();
    void clearRecords();
    
    // 保存和加载历史记录
    bool save() const;
    bool load();

private:
    TransferHistory(QObject* parent = nullptr);
    ~TransferHistory();
    
    QList<TransferRecord> m_records;
    QString m_historyFile;
    static const int MAX_RECORDS = 1000;
};

#endif // TRANSFERHISTORY_H 