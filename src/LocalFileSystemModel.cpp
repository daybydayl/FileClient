#include "LocalFileSystemModel.h"

LocalFileSystemModel::LocalFileSystemModel(QObject* parent)
    : QFileSystemModel(parent)
{
}

QVariant LocalFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QVariant LocalFileSystemModel::data(const QModelIndex &index, int role) const
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