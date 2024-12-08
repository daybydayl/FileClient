/****************************************************************************
** Meta object code from reading C++ file 'TaskManagerDialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.12)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/TaskManagerDialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TaskManagerDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.12. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TaskManagerDialog_t {
    QByteArrayData data[11];
    char stringdata0[152];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TaskManagerDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TaskManagerDialog_t qt_meta_stringdata_TaskManagerDialog = {
    {
QT_MOC_LITERAL(0, 0, 17), // "TaskManagerDialog"
QT_MOC_LITERAL(1, 18, 15), // "refreshTaskList"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 17), // "handleContextMenu"
QT_MOC_LITERAL(4, 53, 3), // "pos"
QT_MOC_LITERAL(5, 57, 23), // "handleTaskStatusChanged"
QT_MOC_LITERAL(6, 81, 6), // "taskId"
QT_MOC_LITERAL(7, 88, 14), // "TransferStatus"
QT_MOC_LITERAL(8, 103, 6), // "status"
QT_MOC_LITERAL(9, 110, 25), // "handleTaskProgressUpdated"
QT_MOC_LITERAL(10, 136, 15) // "transferredSize"

    },
    "TaskManagerDialog\0refreshTaskList\0\0"
    "handleContextMenu\0pos\0handleTaskStatusChanged\0"
    "taskId\0TransferStatus\0status\0"
    "handleTaskProgressUpdated\0transferredSize"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TaskManagerDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x08 /* Private */,
       3,    1,   35,    2, 0x08 /* Private */,
       5,    2,   38,    2, 0x08 /* Private */,
       9,    2,   43,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,    4,
    QMetaType::Void, QMetaType::QString, 0x80000000 | 7,    6,    8,
    QMetaType::Void, QMetaType::QString, QMetaType::LongLong,    6,   10,

       0        // eod
};

void TaskManagerDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TaskManagerDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->refreshTaskList(); break;
        case 1: _t->handleContextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 2: _t->handleTaskStatusChanged((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< TransferStatus(*)>(_a[2]))); break;
        case 3: _t->handleTaskProgressUpdated((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TaskManagerDialog::staticMetaObject = { {
    &QDialog::staticMetaObject,
    qt_meta_stringdata_TaskManagerDialog.data,
    qt_meta_data_TaskManagerDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TaskManagerDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TaskManagerDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TaskManagerDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int TaskManagerDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
