#ifndef TRANSFERQUEUE_H
#define TRANSFERQUEUE_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QMap>
#include <QFileInfo>
#include "TransferDefs.h"
#include "TransferHistory.h"

// 传输任务
struct TransferTask {
    QString id;                 // 任务ID
    QString sourcePath;         // 源路径
    QString targetPath;         // 目标路径
    TransferPriority priority;  // 优先级
    TransferStatus status;      // 状态
    qint64 fileSize;           // 文件大小
    qint64 transferredSize;    // 已传输大小
    QDateTime createTime;       // 创建时间
    bool isUpload;             // 是否为上传任务
};

/**
 * @brief 传输队列管理类
 * 
 * 负责:
 * 1. 管理文件传输任务队列
 * 2. 控制任务优先级和执行顺序
 * 3. 提供任务状态更新和进度通知
 * 4. 支持任务的暂停、恢复和取消
 */
class TransferQueue : public QObject
{
    Q_OBJECT
    
public:
    static TransferQueue& instance();
    
    // 添加传输任务
    QString addTask(const QString& sourcePath, const QString& targetPath, 
                   TransferPriority priority = TransferPriority::Normal,
                   bool isUpload = true);
    
    // 获取下一个任务
    TransferTask getNextTask();
    
    // 更新任务状态
    void updateTaskStatus(const QString& taskId, TransferStatus status);
    void updateTaskProgress(const QString& taskId, qint64 transferredSize);
    
    // 暂停/恢复任务
    void pauseTask(const QString& taskId);
    void resumeTask(const QString& taskId);
    void pauseAllTasks();
    void resumeAllTasks();
    
    // 取消任务
    void cancelTask(const QString& taskId);
    
    // 获取任务信息
    TransferTask getTaskInfo(const QString& taskId) const;
    QList<TransferTask> getAllTasks() const;
    
signals:
    void taskAdded(const TransferTask& task);
    void taskStatusChanged(const QString& taskId, TransferStatus status);
    void taskProgressUpdated(const QString& taskId, qint64 transferredSize);
    void taskCompleted(const TransferTask& task);
    void taskFailed(const TransferTask& task, const QString& error);
    
private:
    explicit TransferQueue(QObject *parent = nullptr);
    
    // 重新排序队列
    void reorderQueue();
    
    QMap<QString, TransferTask> m_tasks;  // 所有任务
    QQueue<QString> m_waitingQueue;       // 等待队列
    mutable QMutex m_mutex;               // 线程同步
};

#endif // TRANSFERQUEUE_H 