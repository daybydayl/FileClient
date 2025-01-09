#include "RemoteFileSystemModel.h"
#include <QFileIconProvider>
#include <QDateTime>

RemoteFileSystemModel::RemoteFileSystemModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

QModelIndex RemoteFileSystemModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex RemoteFileSystemModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int RemoteFileSystemModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_files.size();
}

int RemoteFileSystemModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 4; // 名称、大小、类型、修改时间
}

QVariant RemoteFileSystemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_files.size())
        return QVariant();

    const RemoteFileInfo& fileInfo = m_files[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return fileInfo.name;
            case 1: return fileInfo.isDirectory ? QVariant() : QString::number(fileInfo.size);
            case 2: return fileInfo.isDirectory ? tr("文件夹") : tr("文件");
            case 3: return fileInfo.modifyTime.toString("yyyy-MM-dd hh:mm:ss");
        }
    }
    else if (role == Qt::DecorationRole && index.column() == 0) {
        static QFileIconProvider iconProvider;
        return fileInfo.isDirectory ? iconProvider.icon(QFileIconProvider::Folder)
                                  : iconProvider.icon(QFileIconProvider::File);
    }

    return QVariant();
}

QVariant RemoteFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("名称");
            case 1: return tr("大小");
            case 2: return tr("类型");
            case 3: return tr("修改时间");
        }
    }
    return QVariant();
}

void RemoteFileSystemModel::clear()
{
    beginResetModel();
    m_files.clear();
    endResetModel();
}

void RemoteFileSystemModel::addFile(const RemoteFileInfo& fileInfo)
{
    beginInsertRows(QModelIndex(), m_files.size(), m_files.size());
    m_files.append(fileInfo);
    endInsertRows();
}

void RemoteFileSystemModel::updateModel(const transfer::DirectoryResponse& response) 
{
    if(response.header().success()) {
        // 清空当前数据
        clear();
        
        // 遍历并添加新的文件条目
        // 创建两个向量，分别存储目录文件和普通文件信息
        std::vector<RemoteFileInfo> directoryFiles;
        std::vector<RemoteFileInfo> normalFiles;
        // 遍历响应中的所有文件
        for(const auto& file : response.files()) {
            RemoteFileInfo fileInfo;
            // 设置文件信息的名称和路径
            fileInfo.name = QString::fromStdString(file.name());
            fileInfo.path = QString::fromStdString(response.path());
            // 过滤掉当前目录和根目录下的上级目录项
            if(fileInfo.name=="." || (response.path()=="/"&&fileInfo.name == ".."))
            {//.项和根目录下..项不显示
                continue;
            }
            // 根据文件类型设置是否为目录
            fileInfo.isDirectory = file.is_directory();
            // 设置文件大小
            fileInfo.size = file.size();
            // 设置文件修改时间
            fileInfo.modifyTime = QDateTime::fromString(QString::fromStdString(file.modify_time()), Qt::ISODate);
            
            // 根据文件类型将文件信息添加到对应的向量中
            if(fileInfo.isDirectory)
                directoryFiles.push_back(fileInfo);
            else
                normalFiles.push_back(fileInfo);
        }
        // 对目录文件和普通文件进行排序，以便按名称排序显示
        std::sort(directoryFiles.begin(), directoryFiles.end(), [](const RemoteFileInfo& a, const RemoteFileInfo& b) { return a.name < b.name; });
        std::sort(normalFiles.begin(), normalFiles.end(), [](const RemoteFileInfo& a, const RemoteFileInfo& b) { return a.name < b.name; });
        // 将排序后的目录文件和普通文件添加到模型中
        for(const auto& file : directoryFiles)
            addFile(file);
        for(const auto& file : normalFiles)
            addFile(file);
    }
} 