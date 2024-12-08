#include "TransferQueue.h"
#include <QUuid>
#include <QFileInfo>
#include <QDateTime>
#include <algorithm>

TransferQueue& TransferQueue::instance()
{
    static TransferQueue queue;
    return queue;
}

TransferQueue::TransferQueue(QObject *parent) : QObject(parent)
{
}

QString TransferQueue::addTask(const QString& sourcePath, const QString& targetPath, 
                             TransferPriority priority, bool isUpload)
{
    QMutexLocker locker(&m_mutex);
    
    // 创建新任务
    TransferTask task;
    task.id = QUuid::createUuid().toString();
    task.sourcePath = sourcePath;
    task.targetPath = targetPath;
    task.priority = priority;
    task.status = TransferStatus::Waiting;
    task.fileSize = QFileInfo(sourcePath).size();
    task.transferredSize = 0;
    task.createTime = QDateTime::currentDateTime();
    task.isUpload = isUpload;
    
    // 添加到任务列表和等待队列
    m_tasks[task.id] = task;
    m_waitingQueue.enqueue(task.id);
    
    // 重新排序队列
    reorderQueue();
    
    emit taskAdded(task);
    return task.id;
}

TransferTask TransferQueue::getNextTask()
{
    QMutexLocker locker(&m_mutex);
    if (m_waitingQueue.isEmpty()) {
        return TransferTask();
    }
    
    QString taskId = m_waitingQueue.dequeue();
    TransferTask& task = m_tasks[taskId];
    task.status = TransferStatus::Running;
    emit taskStatusChanged(taskId, task.status);
    
    return task;
}

void TransferQueue::updateTaskStatus(const QString& taskId, TransferStatus status)
{
    QMutexLocker locker(&m_mutex);
    if (!m_tasks.contains(taskId)) {
        return;
    }
    
    TransferTask& task = m_tasks[taskId];
    task.status = status;
    
    if (status == TransferStatus::Completed) {
        emit taskCompleted(task);
    }
    
    emit taskStatusChanged(taskId, status);
}

void TransferQueue::updateTaskProgress(const QString& taskId, qint64 transferredSize)
{
    QMutexLocker locker(&m_mutex);
    if (!m_tasks.contains(taskId)) {
        return;
    }
    
    TransferTask& task = m_tasks[taskId];
    task.transferredSize = transferredSize;
    emit taskProgressUpdated(taskId, transferredSize);
}

void TransferQueue::pauseTask(const QString& taskId)
{
    QMutexLocker locker(&m_mutex);
    if (!m_tasks.contains(taskId)) {
        return;
    }
    
    TransferTask& task = m_tasks[taskId];
    if (task.status == TransferStatus::Running) {
        task.status = TransferStatus::Paused;
        emit taskStatusChanged(taskId, task.status);
    }
}

void TransferQueue::resumeTask(const QString& taskId)
{
    QMutexLocker locker(&m_mutex);
    if (!m_tasks.contains(taskId)) {
        return;
    }
    
    TransferTask& task = m_tasks[taskId];
    if (task.status == TransferStatus::Paused) {
        task.status = TransferStatus::Waiting;
        m_waitingQueue.enqueue(taskId);
        reorderQueue();
        emit taskStatusChanged(taskId, task.status);
    }
}

void TransferQueue::pauseAllTasks()
{
    QMutexLocker locker(&m_mutex);
    for (auto& task : m_tasks) {
        if (task.status == TransferStatus::Running || 
            task.status == TransferStatus::Waiting) {
            task.status = TransferStatus::Paused;
            emit taskStatusChanged(task.id, task.status);
        }
    }
    m_waitingQueue.clear();
}

void TransferQueue::resumeAllTasks()
{
    QMutexLocker locker(&m_mutex);
    for (auto& task : m_tasks) {
        if (task.status == TransferStatus::Paused) {
            task.status = TransferStatus::Waiting;
            m_waitingQueue.enqueue(task.id);
            emit taskStatusChanged(task.id, task.status);
        }
    }
    reorderQueue();
}

void TransferQueue::cancelTask(const QString& taskId)
{
    QMutexLocker locker(&m_mutex);
    if (!m_tasks.contains(taskId)) {
        return;
    }
    
    TransferTask task = m_tasks[taskId];
    m_tasks.remove(taskId);
    m_waitingQueue.removeOne(taskId);
    
    emit taskFailed(task, tr("任务已取消"));
}

TransferTask TransferQueue::getTaskInfo(const QString& taskId) const
{
    QMutexLocker locker(&m_mutex);
    return m_tasks.value(taskId);
}

QList<TransferTask> TransferQueue::getAllTasks() const
{
    QMutexLocker locker(&m_mutex);
    return m_tasks.values();
}

void TransferQueue::reorderQueue()
{
    // 将等待队列转换为任务列表
    QList<TransferTask> waitingTasks;
    for (const QString& taskId : m_waitingQueue) {
        waitingTasks.append(m_tasks[taskId]);
    }
    
    // 按优先级和创建时间排序
    std::sort(waitingTasks.begin(), waitingTasks.end(), 
        [](const TransferTask& a, const TransferTask& b) {
            if (static_cast<int>(a.priority) != static_cast<int>(b.priority)) {
                return static_cast<int>(a.priority) > static_cast<int>(b.priority);
            }
            return a.createTime < b.createTime;
        });
    
    // 更新等待队列
    m_waitingQueue.clear();
    for (const TransferTask& task : waitingTasks) {
        m_waitingQueue.enqueue(task.id);
    }
} 