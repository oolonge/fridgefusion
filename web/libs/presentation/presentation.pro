QT += widgets sql
CONFIG += c++17 staticlib

TARGET = presentation
TEMPLATE = lib

INCLUDEPATH += src/ \
    ../core/src/ \
    ../infrastructure/src/ \
    ../application/src/ \
    ../utils/src/

SOURCES += \
    src/presentation/forms/mainwindow.cpp \
    src/presentation/views/authView.cpp \
    src/presentation/views/recipeExplorerView.cpp \
    src/presentation/views/userView.cpp \
    src/presentation/views/recipeDetailView.cpp \
    src/presentation/views/adminView.cpp \
    src/presentation/views/adminViewImpl.cpp \
    src/presentation/views/adminUserViewImpl.cpp \
    src/presentation/views/adminRecipeView.cpp \
    src/presentation/views/adminRecipeViewImpl.cpp \
    src/presentation/views/dialogs/addRecipeDialog.cpp \
    src/presentation/presenters/authPresenter.cpp \
    src/presentation/presenters/recipeExplorerPresenter.cpp \
    src/presentation/presenters/userPresenter.cpp \
    src/presentation/presenters/recipeDetailPresenter.cpp \
    src/presentation/presenters/adminPresenter.cpp \
    src/presentation/presenters/adminRecipePresenter.cpp

HEADERS += \
    src/presentation/forms/mainwindow.h \
    src/presentation/views/authView.h \
    src/presentation/views/recipeExplorerView.h \
    src/presentation/views/userView.h \
    src/presentation/views/recipeDetailView.h \
    src/presentation/views/adminView.h \
    src/presentation/views/adminViewImpl.h \
    src/presentation/views/adminUserView.h \
    src/presentation/views/adminUserViewImpl.h \
    src/presentation/views/adminRecipeView.h \
    src/presentation/views/adminRecipeViewImpl.h \
    src/presentation/views/dialogs/addRecipeDialog.h \
    src/presentation/presenters/authPresenter.h \
    src/presentation/presenters/recipeExplorerPresenter.h \
    src/presentation/presenters/userPresenter.h \
    src/presentation/presenters/recipeDetailPresenter.h \
    src/presentation/presenters/adminPresenter.h \
    src/presentation/presenters/adminRecipePresenter.h

FORMS += \
    src/presentation/forms/mainwindow.ui

LIBS += -L$$OUT_PWD/../../lib -lcore -lapplication -linfrastructure -lutils

# Установка выходного каталога
DESTDIR = $$OUT_PWD/../../lib
