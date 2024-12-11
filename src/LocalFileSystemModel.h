#ifndef LOCALFILESYSTEMMODEL_H
#define LOCALFILESYSTEMMODEL_H

#include <QFileSystemModel>
/**
 * @brief 本地文件系统模型类
 * 
 * 负责:
 * 1. 继承并扩展Qt的文件系统模型
 * 2. 提供中文本地化的文件信息显示
 * 3. 自定义文件属性的显示格式
 * 4. 处理特殊文件类型的图标显示
 */
class LocalFileSystemModel : public QFileSystemModel
{
    Q_OBJECT
    
public:
    explicit LocalFileSystemModel(QObject* parent = nullptr);
    
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif // LOCALFILESYSTEMMODEL_H