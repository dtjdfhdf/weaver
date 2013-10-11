/****************************************************************************
** Meta object code from reading C++ file 'weavermanualalign.h'
**
** Created: Thu Oct 21 11:45:50 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../weavermanualalign.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'weavermanualalign.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_WeaverManualAlign[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x08,
      50,   18,   18,   18, 0x08,
      74,   18,   18,   18, 0x08,
     109,   18,   18,   18, 0x08,
     153,  146,   18,   18, 0x08,
     192,  146,   18,   18, 0x08,
     233,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_WeaverManualAlign[] = {
    "WeaverManualAlign\0\0on_autoPosPushButton_clicked()\0"
    "on_pushButton_clicked()\0"
    "on_contrastSlider_sliderReleased()\0"
    "on_brightnessSlider_sliderReleased()\0"
    "action\0on_contrastSlider_actionTriggered(int)\0"
    "on_brightnessSlider_actionTriggered(int)\0"
    "on_submitPushButton_clicked()\0"
};

const QMetaObject WeaverManualAlign::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_WeaverManualAlign,
      qt_meta_data_WeaverManualAlign, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &WeaverManualAlign::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *WeaverManualAlign::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *WeaverManualAlign::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WeaverManualAlign))
        return static_cast<void*>(const_cast< WeaverManualAlign*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int WeaverManualAlign::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_autoPosPushButton_clicked(); break;
        case 1: on_pushButton_clicked(); break;
        case 2: on_contrastSlider_sliderReleased(); break;
        case 3: on_brightnessSlider_sliderReleased(); break;
        case 4: on_contrastSlider_actionTriggered((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: on_brightnessSlider_actionTriggered((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: on_submitPushButton_clicked(); break;
        default: ;
        }
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
