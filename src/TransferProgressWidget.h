#ifndef TRANSFERPROGRESSWIDGET_H
#define TRANSFERPROGRESSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include "TransferQueue.h"
//#include "../shared/NetworkManager.h"

/**
 * @brief 传输进度显示组件
 * 
 * 负责:
 * 1. 显示单个传输任务的详细进度
 * 2. 提供进度条和速度信息显示
 * 3. 支持暂停/恢复/取消操作
 * 4. 处理进度动画效果
 */
class TransferProgressWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TransferProgressWidget(QWidget *parent = nullptr);
    ~TransferProgressWidget();

    //void setNetworkManager(NetworkManager* manager) { m_networkManager = manager; }
    QString formatSpeed(qint64 bytesPerSecond) const;
    QString formatTime(int seconds) const;
    QString formatSize(qint64 bytes) const;

public slots:
    void updateProgress(const QString& taskId);
    void clearProgress();

private:
    void setupUI();

    //NetworkManager* m_networkManager;
    QVBoxLayout* m_layout;
    QLabel* m_taskLabel;
    QProgressBar* m_progressBar;
    QLabel* m_speedLabel;
    QLabel* m_timeLabel;
    QLabel* m_sizeLabel;
    QTimer* m_updateTimer;
    QString m_currentTaskId;
};

#endif // TRANSFERPROGRESSWIDGET_H