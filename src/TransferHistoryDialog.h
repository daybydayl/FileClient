#ifndef TRANSFERHISTORYDIALOG_H
#define TRANSFERHISTORYDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QLabel>
#include "TransferHistory.h"

/**
 * @brief 传输历史记录对话框
 * 
 * 负责:
 * 1. 显示文件传输历史记录
 * 2. 提供历史记录的搜索和过滤
 * 3. 支持历史记录的导出功能
 * 4. 提供历史记录的清理功能
 */
class TransferHistoryDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TransferHistoryDialog(QWidget *parent = nullptr);
    ~TransferHistoryDialog();

private slots:
    void refreshHistory();
    void exportHistory();
    void clearHistory();
    void filterChanged();

private:
    void setupUI();
    void setupConnections();
    void updateHistoryList(const QList<TransferRecord>& records);
    QString formatSize(qint64 bytes) const;
    QString formatDuration(const QDateTime& start, const QDateTime& end) const;

    QTreeWidget* m_historyList;
    QDateTimeEdit* m_startDateEdit;
    QDateTimeEdit* m_endDateEdit;
    QPushButton* m_exportBtn;
    QPushButton* m_clearBtn;
    QPushButton* m_closeBtn;
    QLabel* m_statsLabel;
};

#endif // TRANSFERHISTORYDIALOG_H 