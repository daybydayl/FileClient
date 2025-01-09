#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include <QMap>
#include <QProgressBar>
#include <QLabel>
#include <QMenu>

/**
 * @brief 进度显示组件
 * 
 * 负责:
 * 1. 显示文件传输进度
 * 2. 提供进度条和详细信息显示
 * 3. 支持多任务进度同时显示
 * 4. 处理进度更新和动画效果
 */
class ProgressWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ProgressWidget(QWidget* parent = nullptr);

public slots:
    void updateProgress(const QString& taskId, int progress, qint64 transferred, qint64 total);
    void onTransferProgressUpdated(
        const QString& taskId,
        const QString& fileName,
        uint64_t totalSize,
        int progress,
        const QString& status,
        const QString& speed,
        const QString& remainingTime
    );
    
private slots:
    void showContextMenu(const QPoint& pos);
    void clearAllProgress();

private:
    void removeProgressBar(const QString& taskId);
    QString formatFileSize(qint64 size) const;

private:
    QTreeWidget* m_treeWidget;
    QMap<QString, QProgressBar*> m_progressBars;
    QMap<QString, QLabel*> m_labels;
    QMenu* m_contextMenu;
};

#endif // PROGRESSWIDGET_H 