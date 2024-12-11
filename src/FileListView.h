#ifndef FILELISTVIEW_H
#define FILELISTVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTimer>
#include <QTabBar>

#include "FileTabPage.h"

// 前向声明
class FileClient;

/**
 * @brief 文件列表视图组件
 * 
 * 负责:
 * 1. 显示本地或远程文件系统的树形视图
 * 2. 提供文件操作功能(复制、粘贴、删除等)
 * 3. 支持文件操作
 * 4. 管理文系统模型和过滤器
 */
class FileListView : public QWidget
{
    Q_OBJECT
    
public:
    // 构造函数,创建文件列表视图
    explicit FileListView(const QString& title, QWidget *parent = nullptr);
    // 析构函数
    ~FileListView();

    // 设置根目录路径
    void setRootPath(const QString& path);
    // 获取当前根目录路径
    QString rootPath() const;
    // 获取指定索引的文件路径
    QString filePath(const QModelIndex& index) const;
    // 获取当前选中的所有项目索引
    QModelIndexList selectedIndexes() const;
    
    // 清空所有标签页
    void clear();
    // 添加远程服务器标签页
    void addServerTab(const QString& serverAddress, const QString& rootPath);
    // 判断是否没有标签页
    bool isEmpty() const { return m_tabWidget->count() == 0; }
    // 显示视图
    void show() { QWidget::show(); }
    // 如果没有标签页则隐藏视图
    void hide() { if(m_tabWidget->count()==0) QWidget::hide(); }
    
    // 远程响应分发
    void dispatchRemoteResponse(const transfer::DirectoryResponse& response);

    // 判断视图是否可见                 
    bool isVisible() const { return QWidget::isVisible(); }

public slots:
    // 关闭指定索引的标签页
    void closeTab(int index);

signals:
    // 当最后一个远程标签页关闭时发出信号
    void remoteTabClosed();

private slots:
    // 添加新标签页
    void addNewTab();
    // 导航到上级目录
    void navigateUp();
    // 更新添加按钮位置
    void updateAddButtonPosition();

private:
    void initUI();
    void setupConnections();
    void createNewTabPage(const QString& path = QString(), const QString& customTitle = QString());
    
    // 拖放事件处理
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void startDrag(Qt::DropActions supportedActions);
    void handleDroppedFiles(const QList<QUrl>& urls);

private:
    QString m_title;           // 视图标题
    QVBoxLayout* m_layout;     // 主布局
    QTabWidget* m_tabWidget;   // 标签页组件
    QPushButton* m_addTabButton; // 添加标签页按钮
};


#endif // FILELISTVIEW_H 