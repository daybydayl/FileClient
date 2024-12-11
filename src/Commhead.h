#ifndef COMMHEAD_H
#define COMMHEAD_H

// Qt 基础组件 (多个类共用的)
#include <QWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

// Qt 数据模型 (文件系统相关类共用)
#include <QAbstractItemModel>
#include <QModelIndex>

// Qt 容器 (多个类共用)
#include <QString>
#include <QVector>
#include <QMap>

// 项目自定义头文件 (核心功能相关)
#include "protos/transfer.pb.h"
#include "TransferDefs.h"

#endif // COMMHEAD_H
