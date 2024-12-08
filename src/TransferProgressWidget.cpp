#include "TransferProgressWidget.h"

TransferProgressWidget::TransferProgressWidget(QWidget *parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_taskLabel(nullptr)
    , m_progressBar(nullptr)
    , m_speedLabel(nullptr)
    , m_timeLabel(nullptr)
    , m_sizeLabel(nullptr)
    , m_updateTimer(nullptr)
{
    setupUI();
    
    // 创建更新定时器
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(1000); // 每秒更新一次
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        if (!m_currentTaskId.isEmpty()) {
            updateProgress(m_currentTaskId);
        }
    });
}

TransferProgressWidget::~TransferProgressWidget()
{
}

void TransferProgressWidget::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(5, 2, 5, 2);
    m_layout->setSpacing(1);  // 减小间距

    // 任务标签和进度条
    QWidget* progressContainer = new QWidget(this);
    QHBoxLayout* progressLayout = new QHBoxLayout(progressContainer);
    progressLayout->setContentsMargins(0, 0, 0, 0);
    progressLayout->setSpacing(5);

    m_taskLabel = new QLabel(this);
    m_taskLabel->setWordWrap(false);  // 禁用自动换行
    m_taskLabel->setMinimumWidth(200);
    progressLayout->addWidget(m_taskLabel);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    progressLayout->addWidget(m_progressBar);

    m_layout->addWidget(progressContainer);

    // 状态信息
    QWidget* statsContainer = new QWidget(this);
    QHBoxLayout* statsLayout = new QHBoxLayout(statsContainer);
    statsLayout->setContentsMargins(0, 0, 0, 0);
    statsLayout->setSpacing(10);

    m_speedLabel = new QLabel(this);
    m_timeLabel = new QLabel(this);
    m_sizeLabel = new QLabel(this);

    statsLayout->addWidget(m_speedLabel);
    statsLayout->addWidget(m_timeLabel);
    statsLayout->addWidget(m_sizeLabel);
    statsLayout->addStretch();

    m_layout->addWidget(statsContainer);
}

void TransferProgressWidget::updateProgress(const QString& taskId)
{
    // if (!m_networkManager) {
    //     return;
    // }

    if (taskId != m_currentTaskId) {
        m_currentTaskId = taskId;
    }

    TransferTask task = TransferQueue::instance().getTaskInfo(taskId);
    if (task.id.isEmpty()) {
        return;
    }

    // 更新任务标签
    QString direction = task.isUpload ? tr("上传") : tr("下载");
    m_taskLabel->setText(tr("%1: %2").arg(direction, QFileInfo(task.sourcePath).fileName()));

    // 更新进度条
    int progress = task.fileSize > 0 ? 
        static_cast<int>((task.transferredSize * 100) / task.fileSize) : 0;
    m_progressBar->setValue(progress);

    // 获取传输统计信息，使用当前任务ID
    //TransferStats stats = m_networkManager->getTransferStats(m_currentTaskId);

    // 更新速度标签
    //m_speedLabel->setText(tr("速度: %1/s").arg(formatSpeed(stats.currentSpeed)));

    // 更新时间标签
    //m_timeLabel->setText(tr("剩余时间: %1").arg(formatTime(stats.estimatedSeconds)));

    // 更新大小标签
    m_sizeLabel->setText(tr("%1 / %2")
        .arg(formatSize(task.transferredSize))
        .arg(formatSize(task.fileSize)));

    // 启动更新定时器
    if (!m_updateTimer->isActive()) {
        m_updateTimer->start();
    }
}

void TransferProgressWidget::clearProgress()
{
    m_currentTaskId.clear();
    m_taskLabel->clear();
    m_progressBar->setValue(0);
    m_speedLabel->clear();
    m_timeLabel->clear();
    m_sizeLabel->clear();
    m_updateTimer->stop();
}

QString TransferProgressWidget::formatSpeed(qint64 bytesPerSecond) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;

    if (bytesPerSecond >= GB) {
        return QString::number(bytesPerSecond / static_cast<double>(GB), 'f', 2) + " GB";
    } else if (bytesPerSecond >= MB) {
        return QString::number(bytesPerSecond / static_cast<double>(MB), 'f', 2) + " MB";
    } else if (bytesPerSecond >= KB) {
        return QString::number(bytesPerSecond / static_cast<double>(KB), 'f', 2) + " KB";
    } else {
        return QString::number(bytesPerSecond) + " B";
    }
}

QString TransferProgressWidget::formatTime(int seconds) const
{
    if (seconds < 0) {
        return tr("未知");
    }

    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    if (hours > 0) {
        return tr("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    } else {
        return tr("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    }
}

QString TransferProgressWidget::formatSize(qint64 bytes) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;

    if (bytes >= GB) {
        return QString::number(bytes / static_cast<double>(GB), 'f', 2) + " GB";
    } else if (bytes >= MB) {
        return QString::number(bytes / static_cast<double>(MB), 'f', 2) + " MB";
    } else if (bytes >= KB) {
        return QString::number(bytes / static_cast<double>(KB), 'f', 2) + " KB";
    } else {
        return QString::number(bytes) + " B";
    }
} 