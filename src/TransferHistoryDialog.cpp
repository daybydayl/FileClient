#include "TransferHistoryDialog.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>

TransferHistoryDialog::TransferHistoryDialog(QWidget *parent)
    : QDialog(parent)
    , m_historyList(nullptr)
    , m_startDateEdit(nullptr)
    , m_endDateEdit(nullptr)
    , m_exportBtn(nullptr)
    , m_clearBtn(nullptr)
    , m_closeBtn(nullptr)
    , m_statsLabel(nullptr)
{
    setupUI();
    setupConnections();
    refreshHistory();
}

TransferHistoryDialog::~TransferHistoryDialog()
{
}

void TransferHistoryDialog::setupUI()
{
    setWindowTitle(tr("传输历史记录"));
    resize(800, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 过滤器布局
    QHBoxLayout* filterLayout = new QHBoxLayout;
    QLabel* dateLabel = new QLabel(tr("日期范围:"), this);
    m_startDateEdit = new QDateTimeEdit(this);
    m_endDateEdit = new QDateTimeEdit(this);
    
    m_startDateEdit->setCalendarPopup(true);
    m_endDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDateTime(QDateTime::currentDateTime().addDays(-7));
    m_endDateEdit->setDateTime(QDateTime::currentDateTime());
    
    filterLayout->addWidget(dateLabel);
    filterLayout->addWidget(m_startDateEdit);
    filterLayout->addWidget(new QLabel(tr("至"), this));
    filterLayout->addWidget(m_endDateEdit);
    filterLayout->addStretch();
    
    mainLayout->addLayout(filterLayout);

    // 历史记录列表
    m_historyList = new QTreeWidget(this);
    m_historyList->setRootIsDecorated(false);
    m_historyList->setAlternatingRowColors(true);
    m_historyList->setSortingEnabled(true);

    QStringList headers;
    headers << tr("文件名") << tr("源路径") << tr("目标路径") 
           << tr("大小") << tr("开始时间") << tr("结束时间") 
           << tr("耗时") << tr("状态");
    m_historyList->setHeaderLabels(headers);
    m_historyList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    mainLayout->addWidget(m_historyList);

    // 统计信息
    m_statsLabel = new QLabel(this);
    mainLayout->addWidget(m_statsLabel);

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    m_exportBtn = new QPushButton(tr("导出..."), this);
    m_clearBtn = new QPushButton(tr("清除"), this);
    m_closeBtn = new QPushButton(tr("关闭"), this);
    
    buttonLayout->addWidget(m_exportBtn);
    buttonLayout->addWidget(m_clearBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeBtn);
    mainLayout->addLayout(buttonLayout);
}

void TransferHistoryDialog::setupConnections()
{
    connect(m_startDateEdit, &QDateTimeEdit::dateTimeChanged,
            this, &TransferHistoryDialog::filterChanged);
    connect(m_endDateEdit, &QDateTimeEdit::dateTimeChanged,
            this, &TransferHistoryDialog::filterChanged);
    
    connect(m_exportBtn, &QPushButton::clicked,
            this, &TransferHistoryDialog::exportHistory);
    connect(m_clearBtn, &QPushButton::clicked,
            this, &TransferHistoryDialog::clearHistory);
    connect(m_closeBtn, &QPushButton::clicked,
            this, &QDialog::accept);
}

void TransferHistoryDialog::refreshHistory()
{
    QList<TransferRecord> records = TransferHistory::instance().getRecords(
        m_startDateEdit->dateTime(), m_endDateEdit->dateTime());
    updateHistoryList(records);
}

void TransferHistoryDialog::updateHistoryList(const QList<TransferRecord>& records)
{
    m_historyList->clear();
    
    qint64 totalSize = 0;
    qint64 totalTime = 0;
    int successCount = 0;
    
    for (const TransferRecord& record : records) {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_historyList);
        item->setText(0, QFileInfo(record.fileName).fileName());
        item->setText(1, record.sourcePath);
        item->setText(2, record.targetPath);
        item->setText(3, formatSize(record.fileSize));
        item->setText(4, record.startTime.toString("yyyy-MM-dd hh:mm:ss"));
        item->setText(5, record.endTime.toString("yyyy-MM-dd hh:mm:ss"));
        item->setText(6, formatDuration(record.startTime, record.endTime));
        item->setText(7, record.isSuccess ? tr("成功") : tr("失败"));
        
        if (record.isSuccess) {
            totalSize += record.fileSize;
            totalTime += record.startTime.secsTo(record.endTime);
            successCount++;
        }
    }
    
    // 更新统计信息
    QString statsText = tr("总计: %1个文件, %2, 平均速度: %3/s")
        .arg(successCount)
        .arg(formatSize(totalSize))
        .arg(totalTime > 0 ? formatSize(totalSize / totalTime) : tr("N/A"));
    m_statsLabel->setText(statsText);
}

void TransferHistoryDialog::exportHistory()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("导出历史记录"), QString(),
        tr("CSV文件 (*.csv);;所有文件 (*.*)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"),
            tr("无法创建文件: %1").arg(fileName));
        return;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    // 写入表头
    out << "文件名,源路径,目标路径,大小,开始时间,结束时间,耗时,状态\n";
    
    // 写入数据
    for (int i = 0; i < m_historyList->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = m_historyList->topLevelItem(i);
        for (int j = 0; j < item->columnCount(); ++j) {
            out << "\"" << item->text(j) << "\"";
            if (j < item->columnCount() - 1) {
                out << ",";
            }
        }
        out << "\n";
    }
    
    file.close();
}

void TransferHistoryDialog::clearHistory()
{
    if (QMessageBox::question(this, tr("确认"),
        tr("确定要清除所有历史记录吗？")) == QMessageBox::Yes) {
        TransferHistory::instance().clearRecords();
        refreshHistory();
    }
}

void TransferHistoryDialog::filterChanged()
{
    refreshHistory();
}

QString TransferHistoryDialog::formatSize(qint64 bytes) const
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

QString TransferHistoryDialog::formatDuration(const QDateTime& start, const QDateTime& end) const
{
    qint64 secs = start.secsTo(end);
    int hours = secs / 3600;
    int minutes = (secs % 3600) / 60;
    int seconds = secs % 60;

    if (hours > 0) {
        return tr("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    } else {
        return tr("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }
} 