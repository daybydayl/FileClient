#include "FileClient.h"
#include <QMessageBox>
#include <QShortcut>
#include <QKeySequence>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include "AddressBar.h"
#include "FileListView.h"
#include "LogWidget.h"
#include "ProgressWidget.h"
#include "TaskManagerDialog.h"
#include "ConfigDialog.h"
#include "TransferHistoryDialog.h"
#include "HelpDialog.h"
#include "ThemeManager.h"
#include "AnimationHelper.h"
#include "NotificationWidget.h"
#include <windows.h>
#include <QGuiApplication>
#include <QScreen>

Net_Tool* FileClient::m_netTool = Net_Tool::getInstance();

FileClient::FileClient(QWidget *parent)
    : QMainWindow(parent)
    , m_menuBar(nullptr)
    , m_toolBar(nullptr)
    , m_addressBar(nullptr)
    , m_localView(nullptr)
    , m_remoteView(nullptr)
    , m_logWidget(nullptr)
    , m_progressWidget(nullptr)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_hSplitter(nullptr)
    , m_connectAction(nullptr)
    , m_disconnectAction(nullptr)
    , m_uploadAction(nullptr)
    , m_downloadAction(nullptr)
    , m_onTopButton(nullptr)
    , m_isOnTop(false)
    //, m_netTool(Net_Tool::getInstance())
{
    // 设置窗口标题和大小
    setWindowTitle(tr("File Transfer Client"));
    QScreen *screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->availableSize();
    resize(screenSize.width() * 0.5, screenSize.height() * 0.6);
    
    // 初始化界面组件
    initMenuBar();
    initToolBar();
    initCentralWidget();
    setupShortcuts();
    setupTheme();

    // 创建一个弹簧widget，将置顶按钮推到右边
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_menuBar->setCornerWidget(spacer, Qt::TopLeftCorner);

    // 创建一个容器来放置按钮
    QWidget* cornerWidget = new QWidget(this);
    cornerWidget->setStyleSheet("background: transparent;");
    QHBoxLayout* cornerLayout = new QHBoxLayout(cornerWidget);
    cornerLayout->setContentsMargins(2, 0, 2, 0);
    cornerLayout->setSpacing(0);

    // 创建置顶按钮
    m_onTopButton = new QPushButton(this);
    m_onTopButton->setFixedWidth(28);
    m_onTopButton->setIcon(QIcon(":/icons/icons/pin.png"));
    m_onTopButton->setIconSize(QSize(16, 16));
    m_onTopButton->setToolTip(tr("Keep Window on Top"));
    m_onTopButton->setCheckable(true);
    m_onTopButton->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    background: transparent;"
        "    padding: 3px;"
        "}"
        "QPushButton:hover {"
        "    background: rgba(255, 255, 255, 15);"
        "}"
        "QPushButton:checked {"
        "    background: rgba(107, 156, 255, 35);"
        "}"
        "QPushButton:checked:hover {"
        "    background: rgba(107, 156, 255, 20);"
        "}"
    );

    // 将按钮添加到容器中
    cornerLayout->addWidget(m_onTopButton);
    
    // 将容器添加到菜单栏右侧
    m_menuBar->setCornerWidget(cornerWidget, Qt::TopRightCorner);

    connect(m_onTopButton, &QPushButton::clicked, this, &FileClient::toggleWindowOnTop);
}

FileClient::~FileClient()
{
    // Qt的父子对象系统会自动删除子对象
}

void FileClient::initMenuBar()
{
    m_menuBar = menuBar();
    
    // 文件菜单
    QMenu* fileMenu = m_menuBar->addMenu(tr("文件(&F)"));
    fileMenu->addAction(tr("连接(&C)"));
    fileMenu->addAction(tr("断开连接(&D)"));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("设置(&S)"), this, &FileClient::showConfig);
    fileMenu->addAction(tr("历史记录(&H)"), this, &FileClient::showHistory);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("退出(&Q)"));
    
    // 编辑菜单
    QMenu* editMenu = m_menuBar->addMenu(tr("编辑(&E)"));
    editMenu->addAction(tr("复制(&C)"));
    editMenu->addAction(tr("粘贴(&V)"));
    
    // 视图菜单
    QMenu* viewMenu = m_menuBar->addMenu(tr("视图(&V)"));
    viewMenu->addAction(tr("刷新(&R)"));
    
    QMenu* themeMenu = viewMenu->addMenu(tr("主题(&T)"));
    QActionGroup* themeGroup = new QActionGroup(this);
    
    QAction* lightTheme = themeMenu->addAction(tr("  浅色(&L)"));
    lightTheme->setCheckable(true);
    lightTheme->setActionGroup(themeGroup);
    connect(lightTheme, &QAction::triggered, this, [](){ 
        ThemeManager::instance().setTheme(ThemeManager::Light); 
    });
    
    QAction* darkTheme = themeMenu->addAction(tr("  深色(&D)"));
    darkTheme->setCheckable(true);
    darkTheme->setActionGroup(themeGroup);
    connect(darkTheme, &QAction::triggered, this, [](){ 
        ThemeManager::instance().setTheme(ThemeManager::Dark); 
    });
    
    QAction* systemTheme = themeMenu->addAction(tr("  跟随系统(&S)"));
    systemTheme->setCheckable(true);
    systemTheme->setActionGroup(themeGroup);
    connect(systemTheme, &QAction::triggered, this, [](){ 
        ThemeManager::instance().setTheme(ThemeManager::System); 
    });
    
    // 根据当前主题设置选中状态
    switch (ThemeManager::instance().currentTheme()) {
        case ThemeManager::Light: lightTheme->setChecked(true); break;
        case ThemeManager::Dark: darkTheme->setChecked(true); break;
        case ThemeManager::System: systemTheme->setChecked(true); break;
    }
    
    // 帮助菜单
    QMenu* helpMenu = m_menuBar->addMenu(tr("帮助(&H)"));
    helpMenu->addAction(tr("帮助内容(&H)"), this, &FileClient::showHelp);
    helpMenu->addSeparator();
    helpMenu->addAction(tr("关于(&A)"), this, &FileClient::showAbout);
}

void FileClient::initToolBar()
{
    m_toolBar = addToolBar(tr("工具栏"));
    m_toolBar->setMovable(true); //工具栏可移动
    
    // 调整工具栏图标大小
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpi = screen->logicalDotsPerInch();
    int iconSize = qRound(16 * (dpi / 96.0));  // 从18减小到16
    m_toolBar->setIconSize(QSize(iconSize, iconSize));
    
    // 添加工具栏按钮
    m_connectAction = m_toolBar->addAction(QIcon(":/icons/icons/connect.png"), tr("连接"));
    m_disconnectAction = m_toolBar->addAction(QIcon(":/icons/icons/disconnect.png"), tr("断开"));
    m_toolBar->addSeparator();
    m_uploadAction = m_toolBar->addAction(QIcon(":/icons/icons/upload.png"), tr("上传"));
    m_downloadAction = m_toolBar->addAction(QIcon(":/icons/icons/download.png"), tr("下载"));
    m_toolBar->addSeparator();
    QAction* taskManagerAction = m_toolBar->addAction(QIcon(":/icons/icons/task.png"), tr("任务管理"));
    QAction* refreshAction = m_toolBar->addAction(QIcon(":/icons/icons/refresh.png"), tr("刷新"));
    
    // 设置初始状态
    m_disconnectAction->setEnabled(false);
    m_uploadAction->setEnabled(false);
    m_downloadAction->setEnabled(false);
    
    // 连接动作信号
    connect(m_connectAction, &QAction::triggered, this, &FileClient::handleConnect);
    connect(m_disconnectAction, &QAction::triggered, this, &FileClient::handleDisconnect);
    connect(m_uploadAction, &QAction::triggered, this, &FileClient::handleUpload);
    connect(m_downloadAction, &QAction::triggered, this, &FileClient::handleDownload);
    connect(taskManagerAction, &QAction::triggered, this, &FileClient::showTaskManager);
    connect(refreshAction, &QAction::triggered, this, [this]() {
        m_localView->setRootPath(m_localView->rootPath());
    });
}

void FileClient::initCentralWidget()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 调整地址栏容器高度
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpi = screen->logicalDotsPerInch();
    int addressBarHeight = qRound(28 * (dpi / 96.0));  // 从42改为28
    
    // 创建地址栏容器
    QWidget* addressContainer = new QWidget(this);
    addressContainer->setFixedHeight(addressBarHeight);
    QHBoxLayout* addressLayout = new QHBoxLayout(addressContainer);
    addressLayout->setContentsMargins(0, 0, 0, 0);
    addressLayout->setSpacing(0);
    
    // 创建地址栏
    m_addressBar = new AddressBar(this);
    addressLayout->addWidget(m_addressBar);
    m_mainLayout->addWidget(addressContainer);
    
    // 创建垂直分割器
    QSplitter* vSplitter = new QSplitter(Qt::Vertical, this);
    m_mainLayout->addWidget(vSplitter);
    
    // 创建水平分割器
    m_hSplitter = new QSplitter(Qt::Horizontal, this);
    vSplitter->addWidget(m_hSplitter);  // 添加到垂直分割器
    
    // 创建本地文件视图
    m_localView = new FileListView(tr("本地"), m_hSplitter);
    m_localView->setRootPath(AppConfig::instance().defaultLocalPath());
    
    // 创建远程文件视图但先不添加到分割器
    m_remoteView = new FileListView(tr("远程"), nullptr);
    
    // 只添加本地视到分割器
    m_hSplitter->addWidget(m_localView);
    
    // 创建底部标签页
    QTabWidget* bottomTabs = new QTabWidget(this);
    // 设置标签页显示在上部
    bottomTabs->setTabPosition(QTabWidget::North);
    
    // 调整底部标签页最小高度
    int minTabHeight = qRound(150 * (dpi / 96.0));
    bottomTabs->setMinimumHeight(minTabHeight);
    
    // 创建传输进度页
    m_progressWidget = new ProgressWidget(this);
    bottomTabs->addTab(m_progressWidget, QIcon(":/icons/icons/transfer.png"), tr("传输"));
    
    // 创建日志页
    m_logWidget = new LogWidget(this);
    bottomTabs->addTab(m_logWidget, QIcon(":/icons/icons/log.png"), tr("日志"));
    
    // 将底部标签页添加到垂直分割器
    vSplitter->addWidget(bottomTabs);
    
    // 设置分割器的初始大小比例
    vSplitter->setStretchFactor(0, 7);  // 上部分占7
    vSplitter->setStretchFactor(1, 3);  // 下部分占3

    // 连接地址栏的信号
    connect(m_addressBar->connectButton(), &QPushButton::clicked, this, &FileClient::handleConnect);
    // 连接远程标签页关闭信号
    connect(m_remoteView, &FileListView::remoteTabClosed, this, &FileClient::handleRemoteTabClosed);
}

void FileClient::handleNetworkError(const QString& error)
{
    m_logWidget->appendLog(error, true);
}

void FileClient::handleConnect()
{
    // 从地址栏获取服务器地址和端口
    QString serverIP = m_addressBar->host();
    uint16_t port = m_addressBar->port();
    
    // 使用Net_Tool连接服务器
    if (m_netTool->connectToServer(serverIP.toStdString(), port)) {
        // 更新UI状态
        m_connectAction->setEnabled(false);
        m_disconnectAction->setEnabled(true);
        m_uploadAction->setEnabled(true);
        m_downloadAction->setEnabled(true);

        // 发送目录请求报文
        transfer::DirectoryRequest request;
        request.mutable_header()->set_type(transfer::DIRECTORY);
        request.set_current_path("");
        request.set_dir_name("");
        request.set_is_parent(false);
        
        transfer::DirectoryResponse response = m_netTool->sendDirectoryRequest(request);

        // 设置错误回调
        m_netTool->setErrorCallback([this](const std::string& error) {
            m_logWidget->appendLog(QString::fromStdString(error), true);
        });

        // 将远程视图添加到分割器中并显示
        if (!m_remoteView->isVisible()) {
            m_hSplitter->addWidget(m_remoteView);
            m_hSplitter->setSizes({1, 1}); // 使用初始化列表,比例1:1即可
            m_remoteView->show();
        }

        // 添加或更新服务器标签页
        QString serverAddress = QString("%1:%2").arg(serverIP).arg(port);
        m_remoteView->addServerTab(serverAddress, "/");

        m_remoteView->dispatchRemoteResponse(response);
        
        m_logWidget->appendLog(tr("成功连接到服务器 %1:%2").arg(serverIP).arg(port));
    } else {
        m_logWidget->appendLog(tr("连接服务器失败"), true);
    }
}

void FileClient::handleDisconnect()
{
    m_netTool->disconnect();
    
    // 更新UI状态
    m_connectAction->setEnabled(true);
    m_disconnectAction->setEnabled(false);
    m_uploadAction->setEnabled(false);
    m_downloadAction->setEnabled(false);
    
    // 清理远程视图
    m_remoteView->clear();
    m_remoteView->hide();
    
    m_logWidget->appendLog(tr("已断开服务器连接"));
}

void FileClient::handleUpload()
{
    QModelIndexList selectedIndexes = m_localView->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        m_logWidget->appendLog(tr("请选择要上传的文件"), true);
        return;
    }
    
    for (const QModelIndex& index : selectedIndexes) {
        QString filePath = m_localView->filePath(index);
        QString targetPath = m_remoteView->rootPath();
        
        // 开始上传任务
        m_netTool->startUploadTask(
            filePath.toStdString(),
            targetPath.toStdString(),
            [this](const transfer::TransferProgressResponse& progress) {
                // 更新进度
                m_progressWidget->updateProgress(
                    QString::fromStdString(progress.task_id()),
                    progress.progress(),
                    progress.transferred_size(),
                    progress.total_size()
                );
                
                if (progress.progress() == 100) {
                    m_logWidget->appendLog(tr("文件上传完成"));
                }
            }
        );
        
        m_logWidget->appendLog(tr("开始上传: %1").arg(filePath));
    }
}

void FileClient::handleDownload()
{
    QModelIndexList selectedIndexes = m_remoteView->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        m_logWidget->appendLog(tr("请选择要下载的文件"), true);
        return;
    }
    
    for (const QModelIndex& index : selectedIndexes) {
        QString filePath = m_remoteView->filePath(index);
        QString savePath = m_localView->rootPath() + "/" + QFileInfo(filePath).fileName();
        
        // 开始下载任务
        m_netTool->startDownloadTask(
            filePath.toStdString(),
            savePath.toStdString(),
            [this](const transfer::TransferProgressResponse& progress) {
                // 更新进度
                m_progressWidget->updateProgress(
                    QString::fromStdString(progress.task_id()),
                    progress.progress(),
                    progress.transferred_size(),
                    progress.total_size()
                );
                
                if (progress.progress() == 100) {
                    m_logWidget->appendLog(tr("文件下载完成"));
                }
            }
        );
        
        m_logWidget->appendLog(tr("开始下载: %1").arg(filePath));
    }
}

void FileClient::showTaskManager()
{
    TaskManagerDialog dialog(this);
    dialog.exec();
}

void FileClient::showConfig()
{
    ConfigDialog dialog(this);
    dialog.exec();
}

void FileClient::showHistory()
{
    TransferHistoryDialog dialog(this);
    dialog.exec();
}

void FileClient::showHelp()
{
    HelpDialog dialog(this);
    dialog.exec();
}

void FileClient::showAbout()
{
    QDialog aboutDialog(this);
    aboutDialog.setObjectName("aboutDialog");  // 设置对象名以便应用QSS样式
    aboutDialog.setWindowTitle(tr("关于"));
    aboutDialog.setMinimumSize(400, 300);
    
    QVBoxLayout* layout = new QVBoxLayout(&aboutDialog);
    layout->setSpacing(10);
    layout->setContentsMargins(20, 30, 20, 20);
    
    // Logo标签
    QLabel* logoLabel = new QLabel;
    logoLabel->setPixmap(QPixmap(":/icons/icons/app.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);
    
    // 内容标签
    QLabel* contentLabel = new QLabel(
        tr("<div style='text-align: center;'>"
           "<h2 style='margin: 20px 0; color: #2196F3;'>文件传输客户端</h2>"
           "<p style='margin: 15px 0; font-size: 11pt; line-height: 1.5;'>版本 1.0</p>"
           "<p style='margin: 15px 0; font-size: 11pt; line-height: 1.5;'>基于Qt %1构建</p>"
           "<p style='margin: 15px 0; font-size: 11pt; line-height: 1.5;'>Copyright 2024</p>"
           "</div>")
        .arg(QT_VERSION_STR));
    contentLabel->setAlignment(Qt::AlignCenter);
    contentLabel->setTextFormat(Qt::RichText);
    
    // 按钮盒子
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, &aboutDialog, &QDialog::accept);
    
    // 添加部件到布局
    layout->addWidget(logoLabel);
    layout->addWidget(contentLabel);
    layout->addStretch();  // 添加弹性空间
    layout->addWidget(buttonBox);
    layout->setAlignment(buttonBox, Qt::AlignCenter);
    
    aboutDialog.exec();
}

void FileClient::setupShortcuts()
{
    // 文件操作
    QShortcut* connectShortcut = new QShortcut(QKeySequence("Ctrl+N"), this);
    connect(connectShortcut, &QShortcut::activated, this, &FileClient::handleConnect);
    
    QShortcut* disconnectShortcut = new QShortcut(QKeySequence("Ctrl+D"), this);
    connect(disconnectShortcut, &QShortcut::activated, this, &FileClient::handleDisconnect);
    
    // 传输操作
    QShortcut* uploadShortcut = new QShortcut(QKeySequence("Ctrl+U"), this);
    connect(uploadShortcut, &QShortcut::activated, this, &FileClient::handleUpload);
    
    QShortcut* downloadShortcut = new QShortcut(QKeySequence("Ctrl+L"), this);
    connect(downloadShortcut, &QShortcut::activated, this, &FileClient::handleDownload);
    
    // 其他功能
    QShortcut* refreshShortcut = new QShortcut(QKeySequence("F5"), this);
    connect(refreshShortcut, &QShortcut::activated, this, [this]() {
        m_localView->setRootPath(m_localView->rootPath());
    });
    
    QShortcut* helpShortcut = new QShortcut(QKeySequence("F1"), this);
    connect(helpShortcut, &QShortcut::activated, this, &FileClient::showHelp);
}

void FileClient::setupTheme()
{
    // 连接主题变更信号
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this](ThemeManager::Theme theme) {
        // 更新界面样式
        style()->polish(this);
    });
}

void FileClient::toggleWindowOnTop()
{
    m_isOnTop = !m_isOnTop;
    
    HWND hwnd = (HWND)this->winId();
    SetWindowPos(hwnd, 
                m_isOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    
    m_onTopButton->setChecked(m_isOnTop);
}

void FileClient::handleRemoteTabClosed()
{
    // 断开连接
    handleDisconnect();
}