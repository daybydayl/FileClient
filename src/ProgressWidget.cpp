#include "ProgressWidget.h"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QMenu>
#include <QAction>
//#include <QThread>

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
            << tr("状态") ;//<< tr("速度") << tr("剩余时间");
    m_treeWidget->setHeaderLabels(headers);
    
    // 设置列宽
    m_treeWidget->setColumnWidth(0, 200);  // 文件名列
    m_treeWidget->setColumnWidth(1, 80);   // 大小列
    m_treeWidget->setColumnWidth(2, 100);  // 进度列
    m_treeWidget->setColumnWidth(3, 80);   // 状态列
    // m_treeWidget->setColumnWidth(4, 80);   // 速度列
    // m_treeWidget->setColumnWidth(5, 80);   // 剩余时间列

    // 初始化右键菜单
    m_contextMenu = new QMenu(this);
    QAction* clearAction = new QAction(tr("清除所有"), this);
    m_contextMenu->addAction(clearAction);
    
    // 连接信号槽
    connect(clearAction, &QAction::triggered, this, &ProgressWidget::clearAllProgress);//清除
    connect(m_treeWidget, &QTreeWidget::customContextMenuRequested,
            this, &ProgressWidget::showContextMenu);//右键菜单
            
    // 设置右键菜单策略
    m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

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

void ProgressWidget::onTransferProgressUpdated(
    const QString& taskId,
    const QString& fileName,
    uint64_t totalSize,
    int progress,
    const QString& status,
    const QString& speed,
    const QString& remainingTime
) {
    // qDebug() << "Slot called with fileName:" << fileName
    //          << "progress:" << progress;
             
    // 更新进度列表中的项目
    // 如果任务ID不存在则添加新项目，否则更新现有项目
    
    // 示例实现:
    QTreeWidgetItem* item = nullptr;
    
    // 查找是否已存在该任务的项目
    for(int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
        if(m_treeWidget->topLevelItem(i)->data(0, Qt::UserRole).toString() == taskId) {
            item = m_treeWidget->topLevelItem(i);
            break;
        }
    }
    
    // 如果不存在则创建新项目
    if(!item) {
        item = new QTreeWidgetItem(m_treeWidget);
        item->setData(0, Qt::UserRole, taskId);
    }
    
    // 更新项目内容
    item->setText(0, fileName);
    
    // 格式化文件大小显示
    QString sizeStr;
    if(totalSize < 1024) {
        sizeStr = QString("%1 B").arg(totalSize);
    } else if(totalSize < 1024*1024) {
        sizeStr = QString("%1 KB").arg(totalSize/1024);
    } else {
        sizeStr = QString("%1 MB").arg(totalSize/(1024*1024));
    }
    item->setText(1, sizeStr);
    
    // 设置进度条
    QProgressBar* progressBar = qobject_cast<QProgressBar*>(
        m_treeWidget->itemWidget(item, 2));
    if(!progressBar) {
        progressBar = new QProgressBar();
        progressBar->setTextVisible(true);
        progressBar->setAlignment(Qt::AlignCenter);
        m_treeWidget->setItemWidget(item, 2, progressBar);
    }
    progressBar->setValue(progress);
    
    item->setText(3, status);
    // item->setText(4, speed);
    // item->setText(5, remainingTime);
    
    if(status == "已完成") {
        //睡眠1秒
        // QThread::msleep(1000);
        // delete item;
    }
}

void ProgressWidget::showContextMenu(const QPoint& pos)
{
    if (m_treeWidget->topLevelItemCount() > 0) {
        m_contextMenu->exec(m_treeWidget->mapToGlobal(pos));
    }
}

void ProgressWidget::clearAllProgress()
{
    // 删除所有项目
    while (m_treeWidget->topLevelItemCount() > 0) {
        QTreeWidgetItem* item = m_treeWidget->takeTopLevelItem(0);
        
        // 获取进度条并删除
        QProgressBar* progressBar = qobject_cast<QProgressBar*>(
            m_treeWidget->itemWidget(item, 2));
        if (progressBar) {
            m_treeWidget->removeItemWidget(item, 2);
            delete progressBar;
        }
        
        delete item;
    }
    
    // 清除进度条和标签的映射
    qDeleteAll(m_progressBars);
    m_progressBars.clear();
    qDeleteAll(m_labels);
    m_labels.clear();
} 