#ifndef FILETABPAGE_H
#define FILETABPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTreeView>
#include <QFileSystemModel>
#include <QFileIconProvider>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include <QDesktopServices>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QScreen>
#include <QApplication>

#include "AppConfig.h"
#include "LocalFileSystemModel.h"
#include "RemoteFileSystemModel.h"


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
    QAbstractItemModel* model() const { 
        return m_isRemote ? 
            reinterpret_cast<QAbstractItemModel*>(m_remoteModel) : 
            reinterpret_cast<QAbstractItemModel*>(m_model); 
    }
    // 判断是否为远程标签页                                                    
    bool isRemote() const { return m_isRemote; }
    // 获取本地文件系统模型
    LocalFileSystemModel* fileSystemModel() const { return m_model; }
    
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
    LocalFileSystemModel* m_model;    // 本地文件系统模型
    RemoteFileSystemModel* m_remoteModel; // 远程文件系统模型
    bool m_isRemote;                    // 是否为远程标签页
};

#endif // FILETABPAGE_H 