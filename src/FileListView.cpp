#include "FileListView.h"
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDrag>
#include "TransferQueue.h"
#include <QToolButton>
#include <QStyle>
#include "AppConfig.h"
#include <QDir>
#include <QApplication>
#include <QDesktopServices>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QGuiApplication>
#include <QScreen>
#include <QFileIconProvider>

ChineseFileSystemModel::ChineseFileSystemModel(QObject* parent)
    : QFileSystemModel(parent)
{
}

QVariant ChineseFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("名称");
            case 1: return tr("大小");
            case 2: return tr("类型");
            case 3: return tr("修改日期");
            default: return QFileSystemModel::headerData(section, orientation, role);
        }
    }
    return QFileSystemModel::headerData(section, orientation, role);
}

QVariant ChineseFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole && index.column() == 2) {  // 类型列
        QFileInfo fileInfo(filePath(index));
        if (fileInfo.isDir()) {
            return tr("文件夹");
        } else {
            // 获取文件后缀并转换为小写
            QString suffix = fileInfo.suffix().toLower(); // suffix()获取文件后缀,toLower()转换为小写
            if (suffix.isEmpty()) {
                return tr("文件");
            } else if (suffix == "txt") {
                return tr("文本文档");
            } else if (suffix == "doc" || suffix == "docx") {
                return tr("Word 文档");
            } else if (suffix == "xls" || suffix == "xlsx") {
                return tr("Excel 工作表");
            } else if (suffix == "ppt" || suffix == "pptx") {
                return tr("PowerPoint 演示文稿");
            } else if (suffix == "pdf") {
                return tr("PDF 文档");
            } else if (suffix == "jpg" || suffix == "jpeg" || suffix == "png" || suffix == "gif" || suffix == "bmp") {
                return tr("图像文件");
            } else if (suffix == "mp3" || suffix == "wav" || suffix == "wma") {
                return tr("音频文件");
            } else if (suffix == "mp4" || suffix == "avi" || suffix == "mkv") {
                return tr("视频文件");
            } else if (suffix == "zip" || suffix == "rar" || suffix == "7z") {
                return tr("压缩文件");
            } else if (suffix == "exe") {
                return tr("可执行程序");
            } else if (suffix == "dll") {
                return tr("动态链接库");
            } else {
                // toUpper()是将字符串转换为大写的方法
                // 例如: "abc" -> "ABC"
                return tr("%1 文件").arg(suffix.toUpper());
            }
        }
    }
    return QFileSystemModel::data(index, role);
}

FileListView::FileListView(const QString& title, QWidget *parent)
    : QWidget(parent)
    , m_title(title)
    , m_layout(nullptr)
    , m_tabWidget(nullptr)
    , m_addTabButton(nullptr)
{
    initUI();
    setupConnections();
}

FileListView::~FileListView()
{
    // Qt的父子对象系统会自动删除子对象
}

void FileListView::initUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // 创建标签页组件
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_layout->addWidget(m_tabWidget);

    // 创建添加标签页按钮
    m_addTabButton = new QPushButton("+", this);
    m_addTabButton->setFixedSize(24, 24);  // 设置为正方形
    // 设置按钮的样式表    
    m_addTabButton->setStyleSheet(
        "QPushButton { "// QPushButton {} 定义按钮的默认样式
        "   border: none; "// - border: none 表示无边框
        "   background: transparent; "// - background: transparent 先设置透明背景
        "   background: rgba(0.2,0.2,0.2,0.2); "// - background: rgba(0.2,0.2,0.2,0.2) 再设置半透明灰色背景
        "   font-size: 16px; "// - font-size: 16px 设置字体大小
        "   font-weight: bold; "// - font-weight: bold 设置字体加粗
        "   padding: 0px; "// - padding: 0px 设置内边距为0
        "}"
        "QPushButton:hover { "// QPushButton:hover {} 定义鼠标悬停时的样式
        "   background: rgba(0.3,0.3,0.3,0.3); "// - background: rgba(0.3,0.3,0.3,0.3) 悬停时背景色加深
        "   border-radius: 3px; "// - border-radius: 3px 添加圆角效果
        "}"
    );
    
    // 将添加按钮直接放在标签栏上
    m_tabWidget->setStyleSheet(
        "QTabBar::tab:last {"
        "   padding-right: 24px;"  // 为添加按钮预留空间
        "}"
    );

    // 监听标签页变化，动态调整添加按钮位置
    connect(m_tabWidget, &QTabWidget::tabBarClicked, this, &FileListView::updateAddButtonPosition);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &FileListView::updateAddButtonPosition);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, [this](int) {
        QTimer::singleShot(0, this, &FileListView::updateAddButtonPosition);
    });

    // 创建第一个标签页，使用本地/远程作为基础标题
    //QString baseTitle = (m_title == tr("本地")) ? tr("本地") : tr("远程");
    if(m_title == tr("本地"))
    {//如果是左边本地，就首次创建tab
        createNewTabPage(AppConfig::instance().defaultLocalPath(), "");  // 本地标签页不传自定义标题
    }
    
}

void FileListView::updateAddButtonPosition()
{
    if (!m_tabWidget->count()) return;

    // 获取标签栏组件,用于后续定位添加按钮
    QTabBar* tabBar = m_tabWidget->tabBar();
    // 获取最后一个标签页的矩形区域(x,y,width,height)
    QRect lastTabRect = tabBar->tabRect(m_tabWidget->count() - 1);
    
    // 将按钮放在最后一个标签页的右边
    m_addTabButton->move(
        tabBar->x() + lastTabRect.right() + 4,  // 4px的间距
        tabBar->y() + (lastTabRect.height() - m_addTabButton->height()) / 2
    );
    m_addTabButton->raise();  // 确保按在最上层
    m_addTabButton->show();
}

void FileListView::createNewTabPage(const QString& path, const QString& customTitle)
{
    bool isRemote = !customTitle.isEmpty();  // 如果有自定义标题，说明是远程标签页
    FileTabPage* page = new FileTabPage(this, isRemote);
    
    // 设置路径
    QString tabPath;
    if (path.isEmpty()) {
        // 如果是本地标签页且没有指定路径，使用默认本地路径
        tabPath = isRemote ? "/" : AppConfig::instance().defaultLocalPath();
    } else {
        tabPath = path;
    }
    page->setRootPath(tabPath);
    
    // 设置标题
    QString title;
    if (!customTitle.isEmpty()) {
        title = customTitle;
    } else {
        // 为新标签页生成标题（本地2、本地3...或远程2、远程3...）
        QString baseTitle = (m_title == tr("本地")) ? tr("本地") : tr("远程");
        int num = m_tabWidget->count() + 1;
        title = baseTitle + QString::number(num);
    }
    
    int index = m_tabWidget->addTab(page, title);
    m_tabWidget->setCurrentIndex(index);
    
    // 更新添加按钮位置
    QTimer::singleShot(0, this, &FileListView::updateAddButtonPosition);
}

void FileListView::setupConnections()
{
    connect(m_addTabButton, &QPushButton::clicked, this, &FileListView::addNewTab);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &FileListView::closeTab);
}

void FileListView::setRootPath(const QString& path)
{
    FileTabPage* currentPage = qobject_cast<FileTabPage*>(m_tabWidget->currentWidget());
    if (currentPage) {
        currentPage->setRootPath(path);
        
        // 更新标签页标题
        QString tabName = QDir(path).dirName();
        if (tabName.isEmpty()) {
            tabName = path;
        }
        m_tabWidget->setTabText(m_tabWidget->currentIndex(), tabName);
    }
}

QString FileListView::rootPath() const
{
    FileTabPage* currentPage = qobject_cast<FileTabPage*>(m_tabWidget->currentWidget());
    return currentPage ? currentPage->rootPath() : QString();
}

QString FileListView::filePath(const QModelIndex& index) const
{
    FileTabPage* currentPage = qobject_cast<FileTabPage*>(m_tabWidget->currentWidget());
    return currentPage ? currentPage->filePath(index) : QString();
}

QModelIndexList FileListView::selectedIndexes() const
{
    FileTabPage* currentPage = qobject_cast<FileTabPage*>(m_tabWidget->currentWidget());
    return currentPage ? currentPage->treeView()->selectionModel()->selectedIndexes() : QModelIndexList();
}

void FileListView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void FileListView::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void FileListView::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        handleDroppedFiles(event->mimeData()->urls());
        event->acceptProposedAction();
    }
}

void FileListView::startDrag(Qt::DropActions supportedActions)
{
    FileTabPage* currentPage = qobject_cast<FileTabPage*>(m_tabWidget->currentWidget());
    if (!currentPage) return;

    QModelIndexList indexes = currentPage->treeView()->selectionModel()->selectedIndexes();
    if (indexes.isEmpty()) {
        return;
    }

    QMimeData* mimeData = new QMimeData;
    QList<QUrl> urls;
    
    for (const QModelIndex& index : indexes) {
        if (index.column() == 0) {  // 只处理第一列
            QString path = currentPage->filePath(index);
            if (!path.isEmpty()) {
                urls << QUrl::fromLocalFile(path);
            }
        }
    }
    
    if (!urls.isEmpty()) {
        mimeData->setUrls(urls);
        QDrag* drag = new QDrag(this);
        drag->setMimeData(mimeData);
        
        // 设置拖拽图标
        if (urls.count() == 1) {
            QFileInfo fileInfo(urls.first().toLocalFile());
            QIcon icon;
            if (currentPage->isRemote()) {
                static QFileIconProvider iconProvider;
                icon = fileInfo.isDir() ? iconProvider.icon(QFileIconProvider::Folder)
                                      : iconProvider.icon(QFileIconProvider::File);
            } else {
                icon = currentPage->fileSystemModel()->fileIcon(indexes.first());
            }
            drag->setPixmap(icon.pixmap(32, 32));
        }
        
        drag->exec(supportedActions);
    }
}

void FileListView::handleDroppedFiles(const QList<QUrl>& urls)
{
    FileTabPage* currentPage = qobject_cast<FileTabPage*>(m_tabWidget->currentWidget());
    if (!currentPage) return;

    for (const QUrl& url : urls) {
        QString filePath = url.toLocalFile();
        if (filePath.isEmpty()) {
            continue;
        }

        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists()) {
            continue;
        }

        QString targetPath = currentPage->rootPath() + "/" + fileInfo.fileName();

        TransferQueue::instance().addTask(
            filePath,                    // 源路径
            targetPath,                  // 目标路径
            TransferPriority::Normal,    // 优先级
            true                         // 是否为上传
        );
    }
}

void FileListView::addNewTab()
{
    // 为新标签页生成标题
    QString baseTitle = (m_title == tr("本地")) ? tr("本地") : tr("远程");
    int num = m_tabWidget->count() + 1;
    QString newTitle;
    createNewTabPage(AppConfig::instance().defaultLocalPath(), newTitle);
}

void FileListView::closeTab(int index)
{
    if (m_tabWidget->count() > 0) {
        FileTabPage* page = qobject_cast<FileTabPage*>(m_tabWidget->widget(index));
        if (page) {
            bool isRemote = page->isRemote();
            m_tabWidget->removeTab(index);
            delete page;

            // 如果关闭的是远程标签页，并且没有其他远程标签页了，发出信号
            if (isRemote) {
                bool hasRemoteTab = false;
                for (int i = 0; i < m_tabWidget->count(); ++i) {
                    if (FileTabPage* p = qobject_cast<FileTabPage*>(m_tabWidget->widget(i))) {
                        if (p->isRemote()) {
                            hasRemoteTab = true;
                            break;
                        }
                    }
                }
                if (!hasRemoteTab) {
                    emit remoteTabClosed();
                }
            }
        }
    }
}

void FileListView::navigateUp()
{
    FileTabPage* currentPage = qobject_cast<FileTabPage*>(m_tabWidget->currentWidget());
    if (currentPage) {
        QDir currentDir(currentPage->rootPath());
        if (currentDir.cdUp()) {
            currentPage->setRootPath(currentDir.absolutePath());
            
            // 更新标签页标题
            QString tabName = currentDir.dirName();
            if (tabName.isEmpty()) {
                tabName = currentDir.absolutePath();
            }
            m_tabWidget->setTabText(m_tabWidget->currentIndex(), tabName);
        }
    }
}

void FileListView::clear()
{
    while (m_tabWidget->count() > 0) {
        QWidget* widget = m_tabWidget->widget(0);
        m_tabWidget->removeTab(0);
        delete widget;
    }
}

void FileListView::addServerTab(const QString& serverAddress, const QString& rootPath)
{
    // 检查是否已存在相同服务器的标签页
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        if (m_tabWidget->tabText(i).startsWith(serverAddress)) {
            m_tabWidget->setCurrentIndex(i);
            FileTabPage* page = qobject_cast<FileTabPage*>(m_tabWidget->widget(i));
            if (page) {
                page->setRootPath(rootPath);
            }
            return;
        }
    }
    
    // 创建新的标签页
    QString tabName = serverAddress;
    createNewTabPage(rootPath, tabName);
}

void FileListView::addFileEntry(const QString& name, const QString& path, 
                              bool isDir, qint64 size, const QDateTime& modTime)
{
    FileTabPage* currentPage = qobject_cast<FileTabPage*>(m_tabWidget->currentWidget());
    if (!currentPage) return;

    RemoteFileInfo fileInfo;
    fileInfo.name = name;
    fileInfo.path = path;
    fileInfo.isDirectory = isDir;
    fileInfo.size = size;
    fileInfo.modifyTime = modTime;
    
    if (auto* model = qobject_cast<RemoteFileSystemModel*>(currentPage->model())) {
        model->addFile(fileInfo);
    }
}

// 实现FileTabPage类
FileTabPage::FileTabPage(QWidget* parent, bool isRemote)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_treeView(nullptr)
    , m_model(nullptr)
    , m_remoteModel(nullptr)
    , m_isRemote(isRemote)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    
    m_treeView = new QTreeView(this);
    m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);// 设置选择模式为扩展选择,允许用户通过Ctrl/Shift选择多个项目
    m_treeView->setDragEnabled(true);// 启用拖拽功能,允许从视图中拖拽项目
    m_treeView->setAcceptDrops(true);// 允许接受拖放操作,可以接收拖拽的项目
    m_treeView->setDropIndicatorShown(true);// 显示拖放指示器,拖拽时显示可放置位置的提示
    m_treeView->setDragDropMode(QAbstractItemView::DragDrop);// 设置拖放模式为DragDrop,允许同时进行拖拽和放置操作
    m_treeView->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);// 设置编辑触发条件:按F2键或双击选中项可以编辑
    
    if (m_isRemote) {
        m_remoteModel = new RemoteFileSystemModel(this);
        m_treeView->setModel(m_remoteModel);
    } else {
        m_model = new ChineseFileSystemModel(this);
    	m_model->setReadOnly(false);
    	updateHiddenFilesFilter();
        m_treeView->setModel(m_model);
        
        // 禁用树形视图的展开/折叠功能
        m_treeView->setRootIsDecorated(false);  // 移除展开/折叠按钮
        m_treeView->setItemsExpandable(false);  // 禁止项目展开
        
        // 设置选择模式
        m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    }
    
    // 根据 DPI 调整列宽
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpi = screen->logicalDotsPerInch();
    qreal scale = dpi / 96.0;
    
    m_treeView->setColumnWidth(0, qRound(200 * scale));  // 名称列
    m_treeView->setColumnWidth(1, qRound(100 * scale));  // 大小列
    m_treeView->setColumnWidth(2, qRound(100 * scale));  // 类型列
    m_treeView->setColumnWidth(3, qRound(150 * scale));  // 修改日期列
    
    // 调整高
    // 这是Qt样式表(QSS)的写法,类似于CSS样式
    // 使用QString的arg()方法动态设置item的高度
    // QTreeView::item 选择器用于设置树视图中所有项目的样式
    // QLineEdit 选择器用于设置编辑框的样式
    m_treeView->setStyleSheet(QString(
        "QTreeView::item { "
        " height: %1px; "         // 设置每个项目的高度,值由scale计算得出
        " font-size: 10px;"       // 设置字体大小为10像素
        " padding: 2px 2px;"      // 设置内边距:上下2px,左右2px
        "}"
        // "QLineEdit { "
        // " height: %1px;"          // 设置编辑框高度为28像
        // " font-size: 10px;"       // 设置编辑框字体大小为16像素
        // " padding: 2px 4px;"      // 设置编辑框内边距
        // "}"
    ).arg(qRound(25 * scale)));   // 将计算后的高度值替换到%1的位置
    
    m_layout->addWidget(m_treeView);

    // 连接信号
    connect(m_treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
        if(!m_isRemote)
        {//双击本地目录或文件
            QString path = m_model->filePath(index);
            QFileInfo fileInfo(path);
            
            if (path == "..") {
                // 处理返回上级目录
                QString currentPath = m_model->rootPath();
                QDir currentDir(currentPath);
                
                if (currentDir.isRoot()) {
                    // 如果当前是根目录（如 D:/），返回到"此电脑"
                    setRootPath("");
                } else {
                    // 否则返回上级目录
                    currentDir.cdUp();
                    setRootPath(currentDir.absolutePath());
                }
            } else if (fileInfo.isDir()) {
                // 进入选中的目录
                setRootPath(path);
            } else {
                // 打开文件
                QDesktopServices::openUrl(QUrl::fromLocalFile(path));
            }
        }
        else
        {//双击远程目录或文件
            //发送目录请求报文
            auto* model = qobject_cast<RemoteFileSystemModel*>(m_remoteModel);
            const RemoteFileInfo& fileInfo = model->fileInfo(index);
            bool isDir = fileInfo.isDirectory;
            if(isDir)
            {//双击目录
                //发送目录请求给服务端
                int a = 0;//test
            }
            else
            {//双击文件
                QMessageBox msgBox(QMessageBox::Warning, "通知", "远程文件无法直接打开。");
                msgBox.exec();
            }
        }
        
    });

    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);  // 设置右键菜单策略为自定义,允我们自己理右键菜单
    // 连接右键菜单信号 - 当用户在树视图中点击右键时,会触发showContextMenu函数来显示上下文菜单
    connect(m_treeView, &QTreeView::customContextMenuRequested,
            this, &FileTabPage::showContextMenu);

    // 连接配置变更信号 - 当应用配置发生改变时,会触发onConfigChanged函数来更新视图
    connect(&AppConfig::instance(), &AppConfig::configChanged,
            this, &FileTabPage::onConfigChanged);
}

void FileTabPage::updateHiddenFilesFilter()
{
    bool showHidden = AppConfig::instance().showHiddenFiles();
    if (showHidden) {
        m_model->setFilter(QDir::AllEntries | QDir::NoDot | QDir::Hidden);
    } else {
        m_model->setFilter(QDir::AllEntries | QDir::NoDot);
    }
}

void FileTabPage::onConfigChanged()
{
    updateHiddenFilesFilter();
    // 刷新当前视图
    setRootPath(rootPath());
}

void FileTabPage::setRootPath(const QString& path)
{
    if (m_isRemote) {
        if (auto* model = qobject_cast<RemoteFileSystemModel*>(m_remoteModel)) {
            model->setCurrentPath(path);
        }
    } else {
        if (path.isEmpty()) {
            // 显示驱动器列表（"此电脑"视图）
            m_model->setRootPath("");
            m_model->setFilter(QDir::Drives | QDir::NoDotAndDotDot);
            m_treeView->setRootIndex(m_model->index(""));
        } else {
            // 在所有目录中（包括根目录）都显示文件和".."
            m_model->setFilter(QDir::AllEntries | QDir::NoDot);  // 显示所有文件和".."
            QModelIndex rootIndex = m_model->setRootPath(path);
            m_treeView->setRootIndex(rootIndex);
        }
    }
}

QString FileTabPage::rootPath() const
{
    if (m_isRemote) {
        if (auto* model = qobject_cast<RemoteFileSystemModel*>(m_remoteModel)) {
            return model->currentPath();
        }
        return QString();
    } else {
        return m_model->rootPath();
    }
}

void FileTabPage::showContextMenu(const QPoint& pos)
{
    QModelIndex index = m_treeView->indexAt(pos);
    QString currentPath;
    
    if (m_isRemote) 
    {
        if (auto* model = qobject_cast<RemoteFileSystemModel*>(m_remoteModel)) 
        {
            currentPath = model->currentPath();
            
            QMenu menu(this);
            
            if (index.isValid()) 
            {
                // 获取选中的文件/目录信息
                const RemoteFileInfo& fileInfo = model->fileInfo(index);
                bool isDir = fileInfo.isDirectory;
                
                if (isDir) 
                {
                    // 目录右键菜单
                    QAction* openAction = menu.addAction(QIcon(":/icons/icons/folder.png"), tr("打开"));
                    menu.addSeparator();
                    QAction* propertiesAction = menu.addAction(QIcon(":/icons/icons/properties.png"), tr("属性"));
                    
                    // 显示菜单并处理选择的动作
                    QAction* action = menu.exec(m_treeView->viewport()->mapToGlobal(pos));
                    
                    if (action == openAction) 
                    {
                        // TODO: 发送目录请求到服务器
                        QString newPath = currentPath + "/" + fileInfo.name;
                        if(newPath.contains("//")) 
                        {
                            newPath = newPath.replace("//", "/");
                        }
                        setRootPath(newPath);
                    }
                    else if (action == propertiesAction) 
                    {
                        showProperties(fileInfo.path);
                    }
                } 
                else 
                {
                    // 文件右键菜单
                    QAction* downloadAction = menu.addAction(QIcon(":/icons/icons/download.png"), tr("下载"));
                    menu.addSeparator();
                    QAction* propertiesAction = menu.addAction(QIcon(":/icons/icons/properties.png"), tr("属性"));
                    
                    // 显示菜单并处理选择的动作
                    QAction* action = menu.exec(m_treeView->viewport()->mapToGlobal(pos));
                    
                    if (action == downloadAction) 
                    {
                        // TODO: 发送下载请求到服务器
                        //emit downloadFile(fileInfo.path);
                    }
                    else if (action == propertiesAction) 
                    {
                        showProperties(fileInfo.path);
                    }
                }
            }
        }
    }
    else
    {
        // 本地文件系统的右键菜单处理
        currentPath = index.isValid() ? 
            QFileInfo(m_model->filePath(index)).isDir() ? m_model->filePath(index) : m_model->filePath(index.parent()) :
            m_model->filePath(m_treeView->rootIndex());

        QMenu menu(this);
        
        // 添加"新建"子菜单
        QMenu* newMenu = menu.addMenu(QIcon(":/icons/icons/new.png"), tr("新建"));
        QAction* newFolderAction = newMenu->addAction(QIcon(":/icons/icons/folder.png"), tr("文件夹"));
        newMenu->addSeparator();
        QAction* newTextFileAction = newMenu->addAction(QIcon(":/icons/icons/text.png"), tr("文本文档"));
        
        menu.addSeparator();
        
        if (index.isValid()) {
            // 获取选中的文件/目录信息
            QString filePath = m_model->filePath(index);
            QFileInfo fileInfo(filePath);
            bool isDir = fileInfo.isDir();
            
            // 添加其他菜单项
            QAction* openAction = menu.addAction(QIcon(":/icons/icons/open.png"), tr("打开"));
            openAction->setEnabled(!isDir || fileInfo.isExecutable());
            
            menu.addSeparator();
            
            QAction* copyAction = menu.addAction(QIcon(":/icons/icons/copy.png"), tr("复制"));
            QAction* pasteAction = menu.addAction(QIcon(":/icons/icons/paste.png"), tr("粘贴"));
            pasteAction->setEnabled(QApplication::clipboard()->mimeData()->hasUrls());
            
            menu.addSeparator();
            
            QAction* deleteAction = menu.addAction(QIcon(":/icons/icons/delete.png"), tr("删除"));
            
            menu.addSeparator();
            
            QAction* propertiesAction = menu.addAction(QIcon(":/icons/icons/properties.png"), tr("属性"));
            
            // 显示菜单并处理选择的动作
            QAction* action = menu.exec(m_treeView->viewport()->mapToGlobal(pos));
            
            if (action == openAction) {
                if (isDir) {
                    setRootPath(filePath);
                } else {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
                }
            }
            else if (action == copyAction) {
                copySelectedFiles();
            }
            else if (action == pasteAction) {
                pasteFiles();
            }
            else if (action == deleteAction) {
                deleteSelectedFiles();
            }
            else if (action == propertiesAction) {
                showProperties(filePath);
            }
            else if (action == newFolderAction) {
                createNewFolder(currentPath);
            }
            else if (action == newTextFileAction) {
                createNewFile(currentPath);
            }
        } else {
            // 如果在空白处右击，只显示新建和粘贴
            QAction* pasteAction = menu.addAction(QIcon(":/icons/icons/paste.png"), tr("粘贴"));
            pasteAction->setEnabled(QApplication::clipboard()->mimeData()->hasUrls());
            
            // 显示菜单并处理选择的动作
            QAction* action = menu.exec(m_treeView->viewport()->mapToGlobal(pos));
            
            if (action == newFolderAction) {
                createNewFolder(currentPath);
            }
            else if (action == newTextFileAction) {
                createNewFile(currentPath);
            }
            else if (action == pasteAction) {
                pasteFiles();
            }
        }
    }
    
    
}

void FileTabPage::copySelectedFiles()
{
    QModelIndexList indexes = m_treeView->selectionModel()->selectedIndexes();
    if (indexes.isEmpty()) return;
    
    QList<QUrl> urls;
    for (const QModelIndex& index : indexes) {
        if (index.column() == 0) { // 只处理第一列
            QString filePath = m_model->filePath(index);
            urls << QUrl::fromLocalFile(filePath);
        }
    }
    
    QMimeData* mimeData = new QMimeData;
    mimeData->setUrls(urls);
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);
}

void FileTabPage::pasteFiles()
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData->hasUrls()) return;
    
    QString targetDir = m_model->filePath(m_treeView->rootIndex());
    QList<QUrl> urls = mimeData->urls();
    
    for (const QUrl& url : urls) {
        QString sourcePath = url.toLocalFile();
        QString fileName = QFileInfo(sourcePath).fileName();
        QString targetPath = targetDir + "/" + fileName;
        
        QFile::copy(sourcePath, targetPath);
    }
}

void FileTabPage::showProperties(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    
    QDialog dialog(this);
    dialog.setWindowTitle(tr("文件属性"));
    dialog.setMinimumWidth(400);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    // 创建属性表单
    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow(tr("名称:"), new QLabel(fileInfo.fileName()));
    formLayout->addRow(tr("类型:"), new QLabel(fileInfo.isDir() ? tr("文件夹") : tr("文件")));
    formLayout->addRow(tr("位置:"), new QLabel(fileInfo.path()));
    
    // 计算文件或目录大小
    // 获取文件或目录的初始大小
    qint64 totalSize = fileInfo.size();
    // 如果是目录,则需要递归计算其中所有文件的大小
    if(fileInfo.isDir()) {
        // 创建目录对象
        QDir dir(filePath);
        // 获取目录下所有文件和子目录,包括隐藏文件和系统文件
        // QDir::DirsFirst表示目录优先排序
        QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::System | QDir::AllDirs | QDir::NoDotAndDotDot, QDir::DirsFirst);
        // 遍历目录中的所有项目
        for(const QFileInfo& fi : fileList) {
            if(fi.isFile()) {
                // 如果是文件,直接加上文件大小
                totalSize += fi.size();
            } else if(fi.isDir()) {
                // 如果是子目录,使用QDirIterator递归遍历其中的所有文件
                QDirIterator it(fi.absoluteFilePath(), QDir::Files | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
                while(it.hasNext()) {
                    it.next();
                    // 累加子目录中每个文件的大小
                    totalSize += it.fileInfo().size();
                }
            }
        }
    }
    formLayout->addRow(tr("大小:"), new QLabel(formatFileSize(totalSize)));
    
    formLayout->addRow(tr("创建时间:"), new QLabel(fileInfo.birthTime().toString()));
    formLayout->addRow(tr("修改时间:"), new QLabel(fileInfo.lastModified().toString()));
    formLayout->addRow(tr("访问时间:"), new QLabel(fileInfo.lastRead().toString()));
    
    layout->addLayout(formLayout);
    
    // 添加确定按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttonBox);
    
    dialog.exec();
}

QString FileTabPage::formatFileSize(qint64 size)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (size < KB) {
        return QString("%1 B").arg(size);
    } else if (size < MB) {
        return QString("%1 KB").arg(size / KB);
    } else if (size < GB) {
        return QString("%1 MB").arg(size / MB);
    } else {
        return QString("%1 GB").arg(size / GB);
    }
}

void FileTabPage::deleteSelectedFiles()
{
    QModelIndexList indexes = m_treeView->selectionModel()->selectedRows();
    if (indexes.isEmpty()) return;
    
    // 构建确认对话框的消息
    QString message;
    if (indexes.count() == 1) {
        QString filePath = m_model->filePath(indexes.first());
        QFileInfo fileInfo(filePath);
        message = tr("确定要删除 \"%1\" %2吗？")
            .arg(fileInfo.fileName())
            .arg(fileInfo.isDir() ? tr("及包含的所有文件") : "");
    } else {
        message = tr("确定要删除选中的 %1 个项目吗？").arg(indexes.count());
    }
    
    // 显示确认对话框
    QMessageBox::StandardButton reply = QMessageBox::warning(this,
        tr("确认删除"),
        message,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // 执行删除操作
        for (const QModelIndex& index : indexes) {
            QString filePath = m_model->filePath(index);
            QFileInfo fileInfo(filePath);
            
            bool success;
            if (fileInfo.isDir()) {
                success = QDir(filePath).removeRecursively();
            } else {
                success = QFile::remove(filePath);
            }
            
            if (!success) {
                QMessageBox::critical(this,
                    tr("删除失败"),
                    tr("无法删除 \"%1\"").arg(fileInfo.fileName()));
            }
        }
    }
}

void FileTabPage::createNewFolder(const QString& parentPath)
{
    QString folderName = tr("新建文件夹");
    QString newPath = parentPath + "/" + folderName;
    
    // 如果文件夹已存在，添加数字后缀
    int suffix = 1;
    while (QDir(newPath).exists()) {
        newPath = parentPath + "/" + folderName + QString(" (%1)").arg(suffix++);
    }
    
    if (QDir().mkdir(newPath)) {
        // 创建成功后，选中并开始编辑文件夹名称
        QModelIndex index = m_model->index(newPath);
        m_treeView->setCurrentIndex(index);
        m_treeView->edit(index);
    } else {
        QMessageBox::critical(this, tr("错误"), tr("无法创建文件夹"));
    }
}

void FileTabPage::createNewFile(const QString& parentPath)
{
    QString fileName = tr("新建文本文档.txt");
    QString newPath = parentPath + "/" + fileName;
    
    // 如果文件已存在，添加数字后缀
    int suffix = 1;
    while (QFile::exists(newPath)) {
        newPath = parentPath + "/" + tr("新建文本文档") + QString(" (%1).txt").arg(suffix++);
    }
    
    QFile file(newPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.close();
        // 创建成功后，选中并开始编辑文件名
        QModelIndex index = m_model->index(newPath);
        m_treeView->setCurrentIndex(index);
        m_treeView->edit(index);
    } else {
        QMessageBox::critical(this, tr("错误"), tr("无法创建文件"));
    }
} 