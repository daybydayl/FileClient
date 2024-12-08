/****************************************************************************
** Meta object code from reading C++ file 'FileClient.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.12)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/FileClient.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FileClient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.12. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FileClient_t {
    QByteArrayData data[14];
    char stringdata0[172];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FileClient_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FileClient_t qt_meta_stringdata_FileClient = {
    {
QT_MOC_LITERAL(0, 0, 10), // "FileClient"
QT_MOC_LITERAL(1, 11, 13), // "handleConnect"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 16), // "handleDisconnect"
QT_MOC_LITERAL(4, 43, 12), // "handleUpload"
QT_MOC_LITERAL(5, 56, 14), // "handleDownload"
QT_MOC_LITERAL(6, 71, 18), // "handleNetworkError"
QT_MOC_LITERAL(7, 90, 5), // "error"
QT_MOC_LITERAL(8, 96, 15), // "showTaskManager"
QT_MOC_LITERAL(9, 112, 10), // "showConfig"
QT_MOC_LITERAL(10, 123, 11), // "showHistory"
QT_MOC_LITERAL(11, 135, 8), // "showHelp"
QT_MOC_LITERAL(12, 144, 9), // "showAbout"
QT_MOC_LITERAL(13, 154, 17) // "toggleWindowOnTop"

    },
    "FileClient\0handleConnect\0\0handleDisconnect\0"
    "handleUpload\0handleDownload\0"
    "handleNetworkError\0error\0showTaskManager\0"
    "showConfig\0showHistory\0showHelp\0"
    "showAbout\0toggleWindowOnTop"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FileClient[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x08 /* Private */,
       3,    0,   70,    2, 0x08 /* Private */,
       4,    0,   71,    2, 0x08 /* Private */,
       5,    0,   72,    2, 0x08 /* Private */,
       6,    1,   73,    2, 0x08 /* Private */,
       8,    0,   76,    2, 0x08 /* Private */,
       9,    0,   77,    2, 0x08 /* Private */,
      10,    0,   78,    2, 0x08 /* Private */,
      11,    0,   79,    2, 0x08 /* Private */,
      12,    0,   80,    2, 0x08 /* Private */,
      13,    0,   81,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void FileClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FileClient *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->handleConnect(); break;
        case 1: _t->handleDisconnect(); break;
        case 2: _t->handleUpload(); break;
        case 3: _t->handleDownload(); break;
        case 4: _t->handleNetworkError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->showTaskManager(); break;
        case 6: _t->showConfig(); break;
        case 7: _t->showHistory(); break;
        case 8: _t->showHelp(); break;
        case 9: _t->showAbout(); break;
        case 10: _t->toggleWindowOnTop(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject FileClient::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
    qt_meta_stringdata_FileClient.data,
    qt_meta_data_FileClient,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FileClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FileClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FileClient.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int FileClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
