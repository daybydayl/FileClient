#ifndef REMOTEFILESYSTEMMODEL_H
#define REMOTEFILESYSTEMMODEL_H

#include <QAbstractItemModel>
#include <QDateTime>
#include <QVector>
#include "protos/transfer.pb.h"

struct RemoteFileInfo {
    QString name;
    QString path;
    bool isDirectory;
    qint64 size;
    QDateTime modifyTime;
    QString permissions;
    QString taskId;
    int status;
};

class RemoteFileSystemModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit RemoteFileSystemModel(QObject* parent = nullptr);

    // 基本的模型接口
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // 添加和清理数据的方法
    void clear();
    void addFile(const RemoteFileInfo& fileInfo);
    void setCurrentPath(const QString& path) { m_currentPath = path; }
    QString currentPath() const { return m_currentPath; }

    // 添加 fileInfo 方法
    const RemoteFileInfo& fileInfo(const QModelIndex& index) const {
        return m_files[index.row()];
    }

    // 使用响应数据更新模型
    void updateModel(const transfer::DirectoryResponse& response);

private:
    QVector<RemoteFileInfo> m_files;
    QString m_currentPath;
};

#endif // REMOTEFILESYSTEMMODEL_H 