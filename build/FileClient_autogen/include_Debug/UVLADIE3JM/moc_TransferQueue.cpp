/****************************************************************************
** Meta object code from reading C++ file 'TransferQueue.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.12)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/TransferQueue.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TransferQueue.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.12. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TransferQueue_t {
    QByteArrayData data[14];
    char stringdata0[157];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TransferQueue_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TransferQueue_t qt_meta_stringdata_TransferQueue = {
    {
QT_MOC_LITERAL(0, 0, 13), // "TransferQueue"
QT_MOC_LITERAL(1, 14, 9), // "taskAdded"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 12), // "TransferTask"
QT_MOC_LITERAL(4, 38, 4), // "task"
QT_MOC_LITERAL(5, 43, 17), // "taskStatusChanged"
QT_MOC_LITERAL(6, 61, 6), // "taskId"
QT_MOC_LITERAL(7, 68, 14), // "TransferStatus"
QT_MOC_LITERAL(8, 83, 6), // "status"
QT_MOC_LITERAL(9, 90, 19), // "taskProgressUpdated"
QT_MOC_LITERAL(10, 110, 15), // "transferredSize"
QT_MOC_LITERAL(11, 126, 13), // "taskCompleted"
QT_MOC_LITERAL(12, 140, 10), // "taskFailed"
QT_MOC_LITERAL(13, 151, 5) // "error"

    },
    "TransferQueue\0taskAdded\0\0TransferTask\0"
    "task\0taskStatusChanged\0taskId\0"
    "TransferStatus\0status\0taskProgressUpdated\0"
    "transferredSize\0taskCompleted\0taskFailed\0"
    "error"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TransferQueue[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       5,    2,   42,    2, 0x06 /* Public */,
       9,    2,   47,    2, 0x06 /* Public */,
      11,    1,   52,    2, 0x06 /* Public */,
      12,    2,   55,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString, 0x80000000 | 7,    6,    8,
    QMetaType::Void, QMetaType::QString, QMetaType::LongLong,    6,   10,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,   13,

       0        // eod
};

void TransferQueue::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TransferQueue *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->taskAdded((*reinterpret_cast< const TransferTask(*)>(_a[1]))); break;
        case 1: _t->taskStatusChanged((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< TransferStatus(*)>(_a[2]))); break;
        case 2: _t->taskProgressUpdated((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< qint64(*)>(_a[2]))); break;
        case 3: _t->taskCompleted((*reinterpret_cast< const TransferTask(*)>(_a[1]))); break;
        case 4: _t->taskFailed((*reinterpret_cast< const TransferTask(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TransferQueue::*)(const TransferTask & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TransferQueue::taskAdded)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TransferQueue::*)(const QString & , TransferStatus );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TransferQueue::taskStatusChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TransferQueue::*)(const QString & , qint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TransferQueue::taskProgressUpdated)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TransferQueue::*)(const TransferTask & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TransferQueue::taskCompleted)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (TransferQueue::*)(const TransferTask & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TransferQueue::taskFailed)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TransferQueue::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_TransferQueue.data,
    qt_meta_data_TransferQueue,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TransferQueue::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TransferQueue::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TransferQueue.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TransferQueue::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void TransferQueue::taskAdded(const TransferTask & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void TransferQueue::taskStatusChanged(const QString & _t1, TransferStatus _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void TransferQueue::taskProgressUpdated(const QString & _t1, qint64 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void TransferQueue::taskCompleted(const TransferTask & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void TransferQueue::taskFailed(const TransferTask & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
