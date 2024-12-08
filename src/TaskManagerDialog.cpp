#include "TaskManagerDialog.h"
#include <QHeaderView>
#include <QMessageBox>
//#include "../shared/NetworkManager.h"
#include "FileClient.h"

TaskManagerDialog::TaskManagerDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_taskList(nullptr)
    , m_clearCompletedBtn(nullptr)
    , m_closeBtn(nullptr)
    , m_contextMenu(nullptr)
    //, m_networkManager(qobject_cast<FileClient*>(parent)->networkManager())
{
    setupUI();
    setupConnections();
    refreshTaskList();
}

TaskManagerDialog::~TaskManagerDialog()
{
}

void TaskManagerDialog::setupUI()
{
    setWindowTitle(tr("传输任务管理"));
    resize(630, 400);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);  // 设置整体边距
    m_mainLayout->setSpacing(8);  // 设置控件间距

    // 任务列表
    m_taskList = new QTreeWidget(this);
    m_taskList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_taskList->setRootIsDecorated(false);
    m_taskList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_taskList->setAlternatingRowColors(true);

    QStringList headers;
    headers << tr("文件名") << tr("大小") << tr("进度") << tr("状态") 
           << tr("速度") << tr("剩余时间") << tr("优先级");
    m_taskList->setHeaderLabels(headers);
    m_taskList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 设置列宽和对齐方式
    m_taskList->setColumnWidth(0, 200);  // 文件名
    m_taskList->setColumnWidth(1, 80);   // 大小
    m_taskList->setColumnWidth(2, 100);  // 进度
    m_taskList->setColumnWidth(3, 80);   // 状态
    m_taskList->setColumnWidth(4, 80);   // 速度
    m_taskList->setColumnWidth(5, 80);   // 剩余时间
    m_taskList->setColumnWidth(6, 60);   // 优先级

    // 设置每列的对齐方式
    for (int i = 0; i < m_taskList->columnCount(); ++i) {
        m_taskList->headerItem()->setTextAlignment(i, Qt::AlignCenter);
    }

    m_mainLayout->addWidget(m_taskList, 1);  // 添加拉伸因子1

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    
    m_clearCompletedBtn = new QPushButton(tr("清除已完成"), this);
    m_closeBtn = new QPushButton(tr("关闭"), this);
    buttonLayout->addWidget(m_clearCompletedBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeBtn);

    m_mainLayout->addLayout(buttonLayout);  // 直接添加布局，不使用容器

    // 右键菜单
    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction(tr("暂停"), this, [this]() {
        for (QTreeWidgetItem* item : m_taskList->selectedItems()) {
            QString taskId = item->data(0, Qt::UserRole).toString();
            TransferQueue::instance().pauseTask(taskId);
        }
    });
    m_contextMenu->addAction(tr("继续"), this, [this]() {
        for (QTreeWidgetItem* item : m_taskList->selectedItems()) {
            QString taskId = item->data(0, Qt::UserRole).toString();
            TransferQueue::instance().resumeTask(taskId);
        }
    });
    m_contextMenu->addAction(tr("取消"), this, [this]() {
        if (QMessageBox::question(this, tr("确认"), tr("确定要取消选中的任务吗？")) 
            == QMessageBox::Yes) {
            for (QTreeWidgetItem* item : m_taskList->selectedItems()) {
                QString taskId = item->data(0, Qt::UserRole).toString();
                TransferQueue::instance().cancelTask(taskId);
            }
        }
    });
}

void TaskManagerDialog::setupConnections()
{
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_clearCompletedBtn, &QPushButton::clicked, this, [this]() {
        QList<QTreeWidgetItem*> items = m_taskList->findItems("*", Qt::MatchWildcard);
        for (QTreeWidgetItem* item : items) {
            QString status = item->text(3);
            if (status == tr("已完成") || status == tr("已取消")) {
                delete item;
            }
        }
    });

    connect(m_taskList, &QTreeWidget::customContextMenuRequested,
            this, &TaskManagerDialog::handleContextMenu);

    connect(&TransferQueue::instance(), &TransferQueue::taskStatusChanged,
            this, &TaskManagerDialog::handleTaskStatusChanged);
    connect(&TransferQueue::instance(), &TransferQueue::taskProgressUpdated,
            this, &TaskManagerDialog::handleTaskProgressUpdated);
}

void TaskManagerDialog::refreshTaskList()
{
    m_taskList->clear();
    QList<TransferTask> tasks = TransferQueue::instance().getAllTasks();

    for (const TransferTask& task : tasks) {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_taskList);
        updateTaskItem(item, task);
    }
}

void TaskManagerDialog::handleContextMenu(const QPoint& pos)
{
    if (!m_taskList->selectedItems().isEmpty()) {
        m_contextMenu->popup(m_taskList->viewport()->mapToGlobal(pos));
    }
}

void TaskManagerDialog::handleTaskStatusChanged(const QString& taskId, TransferStatus status)
{
    QTreeWidgetItem* item = findTaskItem(taskId);
    if (item) {
        item->setText(3, getStatusString(status));
    }
}

void TaskManagerDialog::handleTaskProgressUpdated(const QString& taskId, qint64 transferredSize)
{
    QTreeWidgetItem* item = findTaskItem(taskId);
    if (item) {
        TransferTask task = TransferQueue::instance().getTaskInfo(taskId);
        updateTaskItem(item, task);
    }
}

QTreeWidgetItem* TaskManagerDialog::findTaskItem(const QString& taskId)
{
    QList<QTreeWidgetItem*> items = m_taskList->findItems("*", Qt::MatchWildcard);
    for (QTreeWidgetItem* item : items) {
        if (item->data(0, Qt::UserRole).toString() == taskId) {
            return item;
        }
    }
    return nullptr;
}

void TaskManagerDialog::updateTaskItem(QTreeWidgetItem* item, const TransferTask& task)
{
    item->setData(0, Qt::UserRole, task.id);
    item->setText(0, QFileInfo(task.sourcePath).fileName());
    item->setText(1, formatSize(task.fileSize));

    int progress = task.fileSize > 0 ? 
        static_cast<int>((task.transferredSize * 100) / task.fileSize) : 0;
    item->setText(2, QString("%1%").arg(progress));

    item->setText(3, getStatusString(task.status));

    // 获取传输统计信息，使用任务ID
    //TransferStats stats = m_networkManager->getTransferStats(task.id);
    // item->setText(4, formatSpeed(stats.currentSpeed));
    // item->setText(5, formatTime(stats.estimatedSeconds));

    QString priority;
    switch (task.priority) {
        case TransferPriority::Low: priority = tr("低"); break;
        case TransferPriority::Normal: priority = tr("普通"); break;
        case TransferPriority::High: priority = tr("高"); break;
        case TransferPriority::Urgent: priority = tr("紧急"); break;
    }
    item->setText(6, priority);

    // 设置每列的对齐方式
    for (int i = 0; i < item->columnCount(); ++i) {
        item->setTextAlignment(i, Qt::AlignCenter);
    }
}

QString TaskManagerDialog::formatSize(qint64 bytes) const
{
    if (bytes < 0) return "0 B";
    
    static const QStringList units = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double size = bytes;
    
    while (size >= 1024.0 && unit < units.size() - 1) {
        size /= 1024.0;
        unit++;
    }
    
    return QString("%1 %2").arg(size, 0, 'f', unit > 0 ? 1 : 0).arg(units[unit]);
}

QString TaskManagerDialog::formatSpeed(qint64 bytesPerSecond) const
{
    if (bytesPerSecond <= 0) return tr("0 B/s");
    return formatSize(bytesPerSecond) + "/s";
}

QString TaskManagerDialog::formatTime(int seconds) const
{
    if (seconds < 0) return tr("未知");
    if (seconds == 0) return tr("0秒");

    QStringList parts;
    
    // 天
    if (seconds >= 86400) {
        parts << QString("%1天").arg(seconds / 86400);
        seconds %= 86400;
    }
    
    // 小时
    if (seconds >= 3600) {
        parts << QString("%1小时").arg(seconds / 3600);
        seconds %= 3600;
    }
    
    // 分钟
    if (seconds >= 60) {
        parts << QString("%1分").arg(seconds / 60);
        seconds %= 60;
    }
    
    // 秒
    if (seconds > 0 || parts.isEmpty()) {
        parts << QString("%1秒").arg(seconds);
    }
    
    return parts.join(" ");
}

QString TaskManagerDialog::getStatusString(TransferStatus status) const
{
    switch (status) {
        case TransferStatus::Waiting: return tr("等待中");
        case TransferStatus::Running: return tr("传输中");
        case TransferStatus::Paused: return tr("已暂停");
        case TransferStatus::Completed: return tr("已完成");
        case TransferStatus::Failed: return tr("已失败");
        default: return tr("未知");
    }
} 