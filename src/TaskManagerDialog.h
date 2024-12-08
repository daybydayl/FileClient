#ifndef TASKMANAGERDIALOG_H
#define TASKMANAGERDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QMenu>
#include "TransferQueue.h"
//#include "../shared/NetworkManager.h"

/**
 * @brief 任务管理器对话框
 * 
 * 负责:
 * 1. 显示所有传输任务的状态
 * 2. 提供任务的暂停/恢复/取消操作
 * 3. 显示任务的详细信息和进度
 * 4. 支持任务的优先级调整
 */
class TaskManagerDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TaskManagerDialog(QWidget *parent = nullptr);
    ~TaskManagerDialog();

private slots:
    void refreshTaskList();
    void handleContextMenu(const QPoint& pos);
    void handleTaskStatusChanged(const QString& taskId, TransferStatus status);
    void handleTaskProgressUpdated(const QString& taskId, qint64 transferredSize);

private:
    void setupUI();
    void setupConnections();
    QTreeWidgetItem* findTaskItem(const QString& taskId);
    void updateTaskItem(QTreeWidgetItem* item, const TransferTask& task);
    QString getStatusString(TransferStatus status) const;

    // 辅助函数
    QString formatSize(qint64 bytes) const;
    QString formatSpeed(qint64 bytesPerSecond) const;
    QString formatTime(int seconds) const;

    QVBoxLayout* m_mainLayout;
    QTreeWidget* m_taskList;
    QPushButton* m_clearCompletedBtn;
    QPushButton* m_closeBtn;
    QMenu* m_contextMenu;
    //NetworkManager* m_networkManager;
};

#endif // TASKMANAGERDIALOG_H