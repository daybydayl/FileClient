#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include "TransferHistory.h"
#include "Net_Tool.h"
#include "../protos/transfer.pb.h"

// 前向声明
class AddressBar;
class FileListView;
class LogWidget;
class ProgressWidget;

/**
 * @brief 文件传输客户端的主窗口类
 * 
 * 负责:
 * 1. 创建和管理主界面布局(菜单栏、工具栏、地址栏、文件视图等)
 * 2. 处理文件传输相关操作(连接、断开、上传、下载等)
 * 3. 管理窗口状态(置顶、主题等)
 * 4. 提供各种对话框功能(设置、历史记录、帮助等)
 */
class FileClient : public QMainWindow
{
    Q_OBJECT

public:
    explicit FileClient(QWidget *parent = nullptr);
    ~FileClient();

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
    void handleNetworkError(const QString& error); // 处理网络错误
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

    Net_Tool* m_netTool;         // 网络交互工具
};

#endif // FILECLIENT_H