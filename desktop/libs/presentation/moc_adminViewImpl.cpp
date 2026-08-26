/****************************************************************************
** Meta object code from reading C++ file 'adminViewImpl.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "src/presentation/views/adminViewImpl.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'adminViewImpl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN13AdminViewImplE_t {};
} // unnamed namespace

template <> constexpr inline auto AdminViewImpl::qt_create_metaobjectdata<qt_meta_tag_ZN13AdminViewImplE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AdminViewImpl",
        "backToMainRequested",
        "",
        "userSelected",
        "userId",
        "searchUsersRequested",
        "deleteUserRequested",
        "updateUserRoleRequested",
        "UserRole",
        "role",
        "refreshUsersRequested",
        "filterUsersByRoleRequested",
        "updateUserDataRequested",
        "username",
        "email",
        "password",
        "confirmPassword"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'backToMainRequested'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'userSelected'
        QtMocHelpers::SignalData<void(int)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Signal 'searchUsersRequested'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'deleteUserRequested'
        QtMocHelpers::SignalData<void(int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Signal 'updateUserRoleRequested'
        QtMocHelpers::SignalData<void(int, UserRole)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { 0x80000000 | 8, 9 },
        }}),
        // Signal 'refreshUsersRequested'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'filterUsersByRoleRequested'
        QtMocHelpers::SignalData<void(UserRole)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Signal 'updateUserDataRequested'
        QtMocHelpers::SignalData<void(int, const QString &, const QString &, const QString &, const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { QMetaType::QString, 13 }, { QMetaType::QString, 14 }, { QMetaType::QString, 15 },
            { QMetaType::QString, 16 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AdminViewImpl, qt_meta_tag_ZN13AdminViewImplE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AdminViewImpl::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13AdminViewImplE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13AdminViewImplE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13AdminViewImplE_t>.metaTypes,
    nullptr
} };

void AdminViewImpl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AdminViewImpl *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->backToMainRequested(); break;
        case 1: _t->userSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->searchUsersRequested(); break;
        case 3: _t->deleteUserRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->updateUserRoleRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<UserRole>>(_a[2]))); break;
        case 5: _t->refreshUsersRequested(); break;
        case 6: _t->filterUsersByRoleRequested((*reinterpret_cast< std::add_pointer_t<UserRole>>(_a[1]))); break;
        case 7: _t->updateUserDataRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[5]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AdminViewImpl::*)()>(_a, &AdminViewImpl::backToMainRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminViewImpl::*)(int )>(_a, &AdminViewImpl::userSelected, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminViewImpl::*)()>(_a, &AdminViewImpl::searchUsersRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminViewImpl::*)(int )>(_a, &AdminViewImpl::deleteUserRequested, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminViewImpl::*)(int , UserRole )>(_a, &AdminViewImpl::updateUserRoleRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminViewImpl::*)()>(_a, &AdminViewImpl::refreshUsersRequested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminViewImpl::*)(UserRole )>(_a, &AdminViewImpl::filterUsersByRoleRequested, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminViewImpl::*)(int , const QString & , const QString & , const QString & , const QString & )>(_a, &AdminViewImpl::updateUserDataRequested, 7))
            return;
    }
}

const QMetaObject *AdminViewImpl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AdminViewImpl::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13AdminViewImplE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "AdminView"))
        return static_cast< AdminView*>(this);
    return QWidget::qt_metacast(_clname);
}

int AdminViewImpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void AdminViewImpl::backToMainRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void AdminViewImpl::userSelected(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void AdminViewImpl::searchUsersRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void AdminViewImpl::deleteUserRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void AdminViewImpl::updateUserRoleRequested(int _t1, UserRole _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void AdminViewImpl::refreshUsersRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void AdminViewImpl::filterUsersByRoleRequested(UserRole _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void AdminViewImpl::updateUserDataRequested(int _t1, const QString & _t2, const QString & _t3, const QString & _t4, const QString & _t5)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2, _t3, _t4, _t5);
}
QT_WARNING_POP
