#include "FileListView.h"
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDrag>
#include <QToolButton>
#include <QStyle>
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
#include "AppConfig.h"
#include "TransferQueue.h"

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
        
        // 更新标签页标题为目录名
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

void FileListView::dispatchRemoteResponse(const transfer::DirectoryResponse& response)
{
    FileTabPage* currentPage = qobject_cast<FileTabPage*>(m_tabWidget->currentWidget());
    if (!currentPage) return;

    if (auto* model = qobject_cast<RemoteFileSystemModel*>(currentPage->model())) {
        model->updateModel(response);
    }
}
