#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <QLabel>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QScreen>
#include <QApplication>

#include "Net_Tool.h"
#include "AddressBar.h"
#include "FileListView.h"
#include "LogWidget.h"
#include "ProgressWidget.h"
#include "TaskManagerDialog.h"
#include "ConfigDialog.h"
#include "TransferHistoryDialog.h"
#include "HelpDialog.h"
#include "ThemeManager.h"

class FileClient : public QMainWindow
{
    Q_OBJECT

public:
    static FileClient* instance() {
        static FileClient* instance = nullptr;
        if (!instance) {
            instance = new FileClient();
        }
        return instance;
    }

    FileListView* getLocalView() const { return m_localView; }
    FileListView* getRemoteView() const { return m_remoteView; }

    explicit FileClient(QWidget *parent = nullptr);
    ~FileClient();

    // 更新传输进度到UI
    void updateTransferProgress(
        const std::string& taskId,
        const std::string& fileName,
        uint64_t totalSize,
        int progress,
        const std::string& status,
        const std::string& speed,
        const std::string& remainingTime
    );

signals:
    // 添加传输进度更新信号
    void transferProgressUpdated(
        const QString& taskId,
        const QString& fileName,
        uint64_t totalSize,
        int progress,
        const QString& status,
        const QString& speed,
        const QString& remainingTime
    );

private:
    // 初始化界面组件的函数
    void initMenuBar();         // 初始化菜单栏
    void initToolBar();         // 初始化工具栏
    void initCentralWidget();   // 初始化中央部件
    void setupShortcuts();      // 设置快捷键
    void setupTheme();          // 设置主题
    
private slots:
    void handleConnect();       // 处理连接
    void handleDisconnect();    // 处理断开连接
    void handleUpload();        // 处理上传
    void handleDownload();      // 处理下载
    void handleDroppedFiles(const QList<QUrl>& urls, const QString& targetPath, bool isUpload);// 处理拖放操作
    void showTaskManager();     // 显示任务管理器
    void showConfig();          // 显示配置对话框
    void showHistory();         // 显示历史记录
    void showHelp();            // 显示帮助
    void showAbout();           // 显示关于对话框
    void toggleWindowOnTop();   // 切换窗口置顶状态
    void handleRemoteTabClosed();  // 添加这个槽函数
    
private:
    // 界面组件
    QMenuBar* m_menuBar;        // 菜单栏
    QToolBar* m_toolBar;        // 工具栏
    AddressBar* m_addressBar;   // 地址栏
    FileListView* m_localView;  // 本地文件视图
    FileListView* m_remoteView; // 远程文件视图
    LogWidget* m_logWidget;     // 日志窗口
    ProgressWidget* m_progressWidget; // 进度窗口
    
    QWidget* m_centralWidget;   // 中央部件
    QVBoxLayout* m_mainLayout;  // 主布局
    QSplitter* m_hSplitter;     // 水平分割器
    
    // 动作
    QAction* m_connectAction;
    QAction* m_disconnectAction;
    QAction* m_uploadAction;
    QAction* m_downloadAction;
    
    QPushButton* m_onTopButton;  // 窗口置顶按钮
    bool m_isOnTop;              // 窗口置顶状态

public:
    static Net_Tool* m_netTool;  // 网络交互工具,改成静态便于其他类使用
};

#endif // FILECLIENT_H