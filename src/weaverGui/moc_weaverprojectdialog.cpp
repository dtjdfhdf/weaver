/****************************************************************************
** Meta object code from reading C++ file 'weaverprojectdialog.h'
**
** Created: Tue Feb 22 12:08:30 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "weaverprojectdialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'weaverprojectdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_weaverProjectDialog[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x08,
      55,   20,   20,   20, 0x08,
      94,   20,   20,   20, 0x08,
     135,   20,   20,   20, 0x08,
     168,   20,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_weaverProjectDialog[] = {
    "weaverProjectDialog\0\0"
    "on_startStitchingButton_clicked()\0"
    "on_choosePiecelistFileButton_clicked()\0"
    "on_chooseOutputImageFileButton_clicked()\0"
    "on_chooseTempDirButton_clicked()\0"
    "on_exitButton_clicked()\0"
};

const QMetaObject weaverProjectDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_weaverProjectDialog,
      qt_meta_data_weaverProjectDialog, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &weaverProjectDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *weaverProjectDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *weaverProjectDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_weaverProjectDialog))
        return static_cast<void*>(const_cast< weaverProjectDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int weaverProjectDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_startStitchingButton_clicked(); break;
        case 1: on_choosePiecelistFileButton_clicked(); break;
        case 2: on_chooseOutputImageFileButton_clicked(); break;
        case 3: on_chooseTempDirButton_clicked(); break;
        case 4: on_exitButton_clicked(); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
