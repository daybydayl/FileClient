#ifndef FILELISTVIEW_H
#define FILELISTVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTreeView>
#include <QFileSystemModel>
#include <QFileIconProvider>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>
#include <QTimer>
#include <QSplitter>
#include <QTreeWidget>
#include <QTextEdit>
#include "RemoteFileSystemModel.h"
#include "protos/transfer.pb.h"
#include "FileClient.h"
/**
 * @brief 中文文件系统模型类
 * 
 * 负责:
 * 1. 继承并扩展Qt的文件系统模型
 * 2. 提供中文本地化的文件信息显示
 * 3. 自定义文件属性的显示格式
 * 4. 处理特殊文件类型的图标显示
 */
class ChineseFileSystemModel : public QFileSystemModel
{
    Q_OBJECT
    
public:
    explicit ChineseFileSystemModel(QObject* parent = nullptr);
    
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};
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

/**
 * @brief 单个文件标签页的内容组件
 */
class FileTabPage : public QWidget
{
    Q_OBJECT
    
public:
    // 构造函数,创建文件标签页
    explicit FileTabPage(QWidget* parent = nullptr, bool isRemote = false);
    // 设置标签页的根目录路径
    void setRootPath(const QString& path);
    // 获取当前根目录路径
    QString rootPath() const;
    // 获取树形视图控件
    QTreeView* treeView() const { return m_treeView; }
    // 获取当前使用的数据模型(本地/远程)
    QAbstractItemModel* model() const { return m_isRemote ? static_cast<QAbstractItemModel*>(m_remoteModel) 
                                                        : static_cast<QAbstractItemModel*>(m_model); }
    // 判断是否为远程标签页                                                    
    bool isRemote() const { return m_isRemote; }
    // 获取本地文件系统模型
    ChineseFileSystemModel* fileSystemModel() const { return m_model; }
    
    // 获取指定索引的文件路径
    QString filePath(const QModelIndex& index) const;

private slots:
    // 显示右键菜单
    void showContextMenu(const QPoint& pos);
    // 配置变更时更新视图
    void onConfigChanged();
    
private:
    // 复制选中的文件
    void copySelectedFiles();
    // 粘贴文件
    void pasteFiles();
    // 显示文件属性对话框
    void showProperties(const QString& filePath);
    // 格式化文件大小显示
    QString formatFileSize(qint64 size);
    // 更新隐藏文件的过滤设置
    void updateHiddenFilesFilter();
    // 删除选中的文件
    void deleteSelectedFiles();
    // 在指定路径创建新文件夹
    void createNewFolder(const QString& parentPath);
    // 在指定路径创建新文件
    void createNewFile(const QString& parentPath);
    // 格式化时间显示
    QString formatTime(qint64 milliseconds);

    QVBoxLayout* m_layout;              // 主布局
    QTreeView* m_treeView;             // 树形视图控件
    ChineseFileSystemModel* m_model;    // 本地文件系统模型
    RemoteFileSystemModel* m_remoteModel; // 远程文件系统模型
    bool m_isRemote;                    // 是否为远程标签页
};

#endif // FILELISTVIEW_H 