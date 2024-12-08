#include "ProgressWidget.h"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>

ProgressWidget::ProgressWidget(QWidget* parent)
    : QWidget(parent)
    , m_treeWidget(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setAlternatingRowColors(true);
    
    // 设置表头
    QStringList headers;
    headers << tr("文件名") << tr("大小") << tr("传输进度") 
            << tr("状态") << tr("速度") << tr("剩余时间");
    m_treeWidget->setHeaderLabels(headers);
    
    // 设置列宽
    m_treeWidget->setColumnWidth(0, 200);  // 文件名列
    m_treeWidget->setColumnWidth(1, 80);   // 大小列
    m_treeWidget->setColumnWidth(2, 100);  // 进度列
    m_treeWidget->setColumnWidth(3, 80);   // 状态列
    m_treeWidget->setColumnWidth(4, 80);   // 速度列
    m_treeWidget->setColumnWidth(5, 80);   // 剩余时间列

    layout->addWidget(m_treeWidget);
}

void ProgressWidget::updateProgress(const QString& taskId, int progress, qint64 transferred, qint64 total)
{
    // 查找或创建进度条项
    if (!m_progressBars.contains(taskId)) {
        // 创建新的进度条项
        auto progressBar = new QProgressBar(this);
        auto label = new QLabel(taskId, this);
        // ... 设置布局等
        m_progressBars[taskId] = progressBar;
        m_labels[taskId] = label;
    }
    
    // 更新进度
    auto progressBar = m_progressBars[taskId];
    progressBar->setValue(progress);
    
    // 更新传输信息
    auto label = m_labels[taskId];
    QString sizeInfo = QString("%1 / %2")
        .arg(formatFileSize(transferred))
        .arg(formatFileSize(total));
    label->setText(sizeInfo);
    
    // 如果完成了，考虑在一定时间后移除进度条
    if (progress >= 100) {
        QTimer::singleShot(3000, this, [this, taskId]() {
            removeProgressBar(taskId);
        });
    }
}

void ProgressWidget::removeProgressBar(const QString& taskId)
{
    if (m_progressBars.contains(taskId)) {
        delete m_progressBars[taskId];
        m_progressBars.remove(taskId);
    }
    if (m_labels.contains(taskId)) {
        delete m_labels[taskId];
        m_labels.remove(taskId);
    }
}

QString ProgressWidget::formatFileSize(qint64 size) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (size < KB) {
        return QString("%1 B").arg(size);
    } else if (size < MB) {
        return QString("%1 KB").arg(size / KB);
    } else if (size < GB) {
        return QString("%1 MB").arg(size / MB);
    } else {
        return QString("%1 GB").arg(size / GB);
    }
} 