#include "FileTabPage.h"
#include "FileClient.h"


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
        m_model = new LocalFileSystemModel(this);
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
                transfer::DirectoryRequest request;
                request.mutable_header()->set_type(transfer::DIRECTORY);
                request.set_current_path(fileInfo.path.toStdString());//当前路径
                request.set_dir_name(fileInfo.name.toStdString());//当前目录文件
                request.set_is_parent(false);
                if(fileInfo.name=="..")
                {
                    request.set_is_parent(true);
                }   
                
                transfer::DirectoryResponse response = FileClient::m_netTool->sendDirectoryRequest(request);

                // 更新当前路径
                //QString newPath = fileInfo.path + "/" + fileInfo.name;
                QString newPath = response.path().c_str();
                if(newPath.contains("//")) {
                    newPath = newPath.replace("//", "/");
                }
                model->setCurrentPath(newPath);

                // 更新模型数据
                model->updateModel(response);
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

QString FileTabPage::filePath(const QModelIndex& index) const
{
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
                        QModelIndexList indexes = m_treeView->selectionModel()->selectedIndexes();
                        QString localPath = FileClient::instance()->getLocalView()->rootPath();
                        handleDownloadFiles(indexes, localPath);
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

// 添加新的成员函数用于检查是否包含目录
bool FileTabPage::containsDirectory(const QModelIndexList& indexes) const
{
    for (const QModelIndex& index : indexes) {
        if (index.column() == 0) { // 只检查第一列
            if (m_isRemote) {
                auto* model = qobject_cast<RemoteFileSystemModel*>(m_remoteModel);
                const RemoteFileInfo& fileInfo = model->fileInfo(index);
                if (fileInfo.isDirectory) {
                    return true;
                }
            } else {
                QString filePath = m_model->filePath(index);
                QFileInfo fileInfo(filePath);
                if (fileInfo.isDir()) {
                    return true;
                }
            }
        }
    }
    return false;
}

// 添加上传文件的处理函数
void FileTabPage::handleUploadFiles(const QModelIndexList& indexes, const QString& targetPath)
{
    // 检查是否包含目录
    if (containsDirectory(indexes)) {
        QMessageBox::warning(this, tr("上传提示"), 
            tr("选中的文件中包含目录，本次不进行上传操作。"));
        return;
    }

    // 遍历选中的文件进行上传
    for (const QModelIndex& index : indexes) {
        if (index.column() == 0) { // 只处理第一列
            QString sourcePathFileName = m_model->filePath(index);
            QString fileName = QFileInfo(sourcePathFileName).fileName();
            QString fullTargetPath = targetPath;
            
            // 调用Net_Tool开始上传任务
            FileClient::m_netTool->startUploadTask(
                sourcePathFileName.toStdString(),
                fullTargetPath.toStdString(),
                std::bind(&FileTabPage::onTransferProgress, this, 
                    std::placeholders::_1)
            );
        }
    }
}

// 添加下载文件的处理函数
void FileTabPage::handleDownloadFiles(const QModelIndexList& indexes, const QString& targetPath)
{
    // 检查是否包含目录
    if (containsDirectory(indexes)) {
        QMessageBox::warning(this, tr("下载提示"), 
            tr("选中的文件中包含目录，本次不进行下载操作。"));
        return;
    }

    // 遍历选中的文件进行下载
    for (const QModelIndex& index : indexes) {
        if (index.column() == 0) { // 只处理第一列
            auto* model = qobject_cast<RemoteFileSystemModel*>(m_remoteModel);
            const RemoteFileInfo& fileInfo = model->fileInfo(index);
            QString fullTargetPath = targetPath + "/" + fileInfo.name;

            // 调用Net_Tool开始下载任务
            FileClient::m_netTool->startDownloadTask(
                fileInfo.path.toStdString(),
                fullTargetPath.toStdString(),
                std::bind(&FileTabPage::onTransferProgress, this, 
                    std::placeholders::_1)
            );
        }
    }
}

// 传输进度回调函数
void FileTabPage::onTransferProgress(const transfer::TransferProgressResponse& progress)
{
    // 这里可以发送信号给进度窗口更新进度
    emit transferProgressUpdated(
        QString::fromStdString(progress.task_id()),
        progress.progress(),
        progress.transferred_size(),
        progress.total_size()
    );
} 