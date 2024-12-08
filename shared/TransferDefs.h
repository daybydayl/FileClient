#ifndef TRANSFERDEFS_H
#define TRANSFERDEFS_H

#include <QString>
#include <QDateTime>

// 传输优先级枚举
enum class TransferPriority {
    Low,
    Normal,
    High,
    Urgent
};

// 传输任务状态
enum class TransferStatus {
    Waiting,    // 等待中
    Running,    // 传输中
    Paused,     // 已暂停
    Completed,  // 已完成
    Failed      // 失败
};

// 传输统计信息
struct TransferStats {
    qint64 startTime;          // 开始时间
    qint64 lastUpdateTime;     // 最后更新时间
    qint64 lastBytes;          // 上次统计的字节数
    qint64 transferredBytes;   // 已传输字节数
    qint64 currentSpeed;       // 当前速度(字节/秒)
    qint64 averageSpeed;       // 平均速度(字节/秒)
    qint64 estimatedSeconds;   // 预计剩余时间(秒)
    int retryCount;           // 添加重试次数字段
    
    TransferStats()
        : startTime(QDateTime::currentMSecsSinceEpoch())
        , lastUpdateTime(startTime)
        , lastBytes(0)
        , transferredBytes(0)
        , currentSpeed(0)
        , averageSpeed(0)
        , estimatedSeconds(0)
        , retryCount(0)  // 初始化重试次数
    {}
};

#endif // TRANSFERDEFS_H 