/****************************************************************************
** Meta object code from reading C++ file 'adminRecipeViewImpl.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "src/presentation/views/adminRecipeViewImpl.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'adminRecipeViewImpl.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN19AdminRecipeViewImplE_t {};
} // unnamed namespace

template <> constexpr inline auto AdminRecipeViewImpl::qt_create_metaobjectdata<qt_meta_tag_ZN19AdminRecipeViewImplE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AdminRecipeViewImpl",
        "recipeSelected",
        "",
        "recipeId",
        "searchRecipesRequested",
        "deleteRecipeRequested",
        "updateRecipeRequested",
        "refreshRecipesRequested",
        "addCategoryToRecipeRequested",
        "categoryId",
        "removeCategoryFromRecipeRequested",
        "addIngredientToRecipeRequested",
        "ingredientId",
        "quantity",
        "removeIngredientFromRecipeRequested",
        "updateIngredientQuantityRequested",
        "createRecipeRequested",
        "name"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'recipeSelected'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'searchRecipesRequested'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'deleteRecipeRequested'
        QtMocHelpers::SignalData<void(int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'updateRecipeRequested'
        QtMocHelpers::SignalData<void(int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'refreshRecipesRequested'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'addCategoryToRecipeRequested'
        QtMocHelpers::SignalData<void(int, int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Int, 9 },
        }}),
        // Signal 'removeCategoryFromRecipeRequested'
        QtMocHelpers::SignalData<void(int, int)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Int, 9 },
        }}),
        // Signal 'addIngredientToRecipeRequested'
        QtMocHelpers::SignalData<void(int, int, double)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Int, 12 }, { QMetaType::Double, 13 },
        }}),
        // Signal 'removeIngredientFromRecipeRequested'
        QtMocHelpers::SignalData<void(int, int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Int, 12 },
        }}),
        // Signal 'updateIngredientQuantityRequested'
        QtMocHelpers::SignalData<void(int, int, double)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Int, 12 }, { QMetaType::Double, 13 },
        }}),
        // Signal 'createRecipeRequested'
        QtMocHelpers::SignalData<void(const QString &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 17 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AdminRecipeViewImpl, qt_meta_tag_ZN19AdminRecipeViewImplE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AdminRecipeViewImpl::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19AdminRecipeViewImplE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19AdminRecipeViewImplE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN19AdminRecipeViewImplE_t>.metaTypes,
    nullptr
} };

void AdminRecipeViewImpl::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AdminRecipeViewImpl *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->recipeSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->searchRecipesRequested(); break;
        case 2: _t->deleteRecipeRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->updateRecipeRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->refreshRecipesRequested(); break;
        case 5: _t->addCategoryToRecipeRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 6: _t->removeCategoryFromRecipeRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->addIngredientToRecipeRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3]))); break;
        case 8: _t->removeIngredientFromRecipeRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 9: _t->updateIngredientQuantityRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3]))); break;
        case 10: _t->createRecipeRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AdminRecipeViewImpl::*)(int )>(_a, &AdminRecipeViewImpl::recipeSelected, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminRecipeViewImpl::*)()>(_a, &AdminRecipeViewImpl::searchRecipesRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminRecipeViewImpl::*)(int )>(_a, &AdminRecipeViewImpl::deleteRecipeRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminRecipeViewImpl::*)(int )>(_a, &AdminRecipeViewImpl::updateRecipeRequested, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminRecipeViewImpl::*)()>(_a, &AdminRecipeViewImpl::refreshRecipesRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminRecipeViewImpl::*)(int , int )>(_a, &AdminRecipeViewImpl::addCategoryToRecipeRequested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminRecipeViewImpl::*)(int , int )>(_a, &AdminRecipeViewImpl::removeCategoryFromRecipeRequested, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminRecipeViewImpl::*)(int , int , double )>(_a, &AdminRecipeViewImpl::addIngredientToRecipeRequested, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminRecipeViewImpl::*)(int , int )>(_a, &AdminRecipeViewImpl::removeIngredientFromRecipeRequested, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminRecipeViewImpl::*)(int , int , double )>(_a, &AdminRecipeViewImpl::updateIngredientQuantityRequested, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (AdminRecipeViewImpl::*)(const QString & )>(_a, &AdminRecipeViewImpl::createRecipeRequested, 10))
            return;
    }
}

const QMetaObject *AdminRecipeViewImpl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AdminRecipeViewImpl::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19AdminRecipeViewImplE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "AdminRecipeView"))
        return static_cast< AdminRecipeView*>(this);
    return QWidget::qt_metacast(_clname);
}

int AdminRecipeViewImpl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void AdminRecipeViewImpl::recipeSelected(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void AdminRecipeViewImpl::searchRecipesRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AdminRecipeViewImpl::deleteRecipeRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void AdminRecipeViewImpl::updateRecipeRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void AdminRecipeViewImpl::refreshRecipesRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void AdminRecipeViewImpl::addCategoryToRecipeRequested(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2);
}

// SIGNAL 6
void AdminRecipeViewImpl::removeCategoryFromRecipeRequested(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2);
}

// SIGNAL 7
void AdminRecipeViewImpl::addIngredientToRecipeRequested(int _t1, int _t2, double _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2, _t3);
}

// SIGNAL 8
void AdminRecipeViewImpl::removeIngredientFromRecipeRequested(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1, _t2);
}

// SIGNAL 9
void AdminRecipeViewImpl::updateIngredientQuantityRequested(int _t1, int _t2, double _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1, _t2, _t3);
}

// SIGNAL 10
void AdminRecipeViewImpl::createRecipeRequested(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1);
}
QT_WARNING_POP
