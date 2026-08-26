/****************************************************************************
** Meta object code from reading C++ file 'adminUserViewImpl.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "src/presentation/views/adminUserViewImpl.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'adminUserViewImpl.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN17AdminUserViewImplE_t {};
} // unnamed namespace

template <> constexpr inline auto AdminUserViewImpl::qt_create_metaobjectdata<qt_meta_tag_ZN17AdminUserViewImplE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AdminUserViewImpl",
        "userSelected",
        "",
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
        // Signal 'userSelected'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'searchUsersRequested'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'deleteUserRequested'
        QtMocHelpers::SignalData<void(int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'updateUserRoleRequested'
        QtMocHelpers::SignalData<void(int, UserRole)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { 0x80000000 | 7, 8 },
        }}),
        // Signal 'refreshUsersRequested'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'filterUsersByRoleRequested'
        QtMocHelpers::SignalData<void(UserRole)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Signal 'updateUserDataRequested'
        QtMocHelpers::SignalData<void(int, const QString &, const QString &, const QString &, const QString &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::QString, 12 }, { QMetaType::QString, 13 }, { QMetaType::QString, 14 },
            { QMetaType::QString, 15 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AdminUserViewImpl, qt_meta_tag_ZN17AdminUserViewImplE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AdminUserViewImpl::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AdminUserViewImplE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AdminUserViewImplE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17AdminUserViewImplE_t>.metaTypes,
    nullptr
} };

void AdminUserViewImpl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AdminUserViewImpl *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->userSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->searchUsersRequested(); break;
        case 2: _t->deleteUserRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->updateUserRoleRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<UserRole>>(_a[2]))); break;
        case 4: _t->refreshUsersRequested(); break;
        case 5: _t->filterUsersByRoleRequested((*reinterpret_cast< std::add_pointer_t<UserRole>>(_a[1]))); break;
        case 6: _t->updateUserDataRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[5]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AdminUserViewImpl::*)(int )>(_a, &AdminUserViewImpl::userSelected, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminUserViewImpl::*)()>(_a, &AdminUserViewImpl::searchUsersRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminUserViewImpl::*)(int )>(_a, &AdminUserViewImpl::deleteUserRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminUserViewImpl::*)(int , UserRole )>(_a, &AdminUserViewImpl::updateUserRoleRequested, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminUserViewImpl::*)()>(_a, &AdminUserViewImpl::refreshUsersRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminUserViewImpl::*)(UserRole )>(_a, &AdminUserViewImpl::filterUsersByRoleRequested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminUserViewImpl::*)(int , const QString & , const QString & , const QString & , const QString & )>(_a, &AdminUserViewImpl::updateUserDataRequested, 6))
            return;
    }
}

const QMetaObject *AdminUserViewImpl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AdminUserViewImpl::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AdminUserViewImplE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "AdminUserView"))
        return static_cast< AdminUserView*>(this);
    return QWidget::qt_metacast(_clname);
}

int AdminUserViewImpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void AdminUserViewImpl::userSelected(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void AdminUserViewImpl::searchUsersRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AdminUserViewImpl::deleteUserRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void AdminUserViewImpl::updateUserRoleRequested(int _t1, UserRole _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void AdminUserViewImpl::refreshUsersRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void AdminUserViewImpl::filterUsersByRoleRequested(UserRole _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void AdminUserViewImpl::updateUserDataRequested(int _t1, const QString & _t2, const QString & _t3, const QString & _t4, const QString & _t5)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2, _t3, _t4, _t5);
}
QT_WARNING_POP
