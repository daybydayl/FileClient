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
 * 3. 支持文件���操作
 * 4. 管理文系统模型和过滤器
 */
class FileListView : public QWidget
{
    Q_OBJECT
    
public:
    explicit FileListView(const QString& title, QWidget *parent = nullptr);
    ~FileListView();

    void setRootPath(const QString& path);
    QString rootPath() const;
    QString filePath(const QModelIndex& index) const;
    QModelIndexList selectedIndexes() const;
    
    void clear();
    void addServerTab(const QString& serverAddress, const QString& rootPath);
    bool isEmpty() const { return m_tabWidget->count() == 0; }
    void show() { QWidget::show(); }
    void hide() { if(m_tabWidget->count()==0) QWidget::hide(); }

    void addFileEntry(const QString& name, const QString& path, bool isDir, 
                     qint64 size, const QDateTime& modTime);
    bool isVisible() const { return QWidget::isVisible(); }

public slots:
    void closeTab(int index);

signals:
    void remoteTabClosed();

private slots:
    void addNewTab();
    void navigateUp();
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
 * 
 * 负责:
 * 1. 管理单个文件视图的显示和操作
 * 2. 处理右键菜单和文件操作
 * 3. 提供文件属性查看功能
 */
class FileTabPage : public QWidget
{
    Q_OBJECT
    
public:
    explicit FileTabPage(QWidget* parent = nullptr, bool isRemote = false);
    void setRootPath(const QString& path);
    QString rootPath() const;
    QTreeView* treeView() const { return m_treeView; }
    QAbstractItemModel* model() const { return m_isRemote ? static_cast<QAbstractItemModel*>(m_remoteModel) 
                                                        : static_cast<QAbstractItemModel*>(m_model); }
    bool isRemote() const { return m_isRemote; }
    ChineseFileSystemModel* fileSystemModel() const { return m_model; }
    
    QString filePath(const QModelIndex& index) const {
        if (!index.isValid()) return QString();
        
        if (m_isRemote) {
            if (auto* model = qobject_cast<RemoteFileSystemModel*>(m_remoteModel)) {
                const RemoteFileInfo& fileInfo = model->fileInfo(index);
                return model->currentPath() + "/" + fileInfo.name;
            }
            return QString();
        } else {
            return m_model->filePath(index);
        }
    }

private slots:
    void showContextMenu(const QPoint& pos);
    void onConfigChanged();
    
private:
    void copySelectedFiles();
    void pasteFiles();
    void showProperties(const QString& filePath);
    QString formatFileSize(qint64 size);
    void updateHiddenFilesFilter();
    void deleteSelectedFiles();
    void createNewFolder(const QString& parentPath);
    void createNewFile(const QString& parentPath);
    QString formatTime(qint64 milliseconds);  // 添加时间格式化方法

    QVBoxLayout* m_layout;
    QTreeView* m_treeView;
    ChineseFileSystemModel* m_model;
    RemoteFileSystemModel* m_remoteModel;
    bool m_isRemote;
};

#endif // FILELISTVIEW_H 