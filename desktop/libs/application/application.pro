QT += sql
CONFIG += c++17 staticlib

TARGET = application
TEMPLATE = lib

INCLUDEPATH += src/ \
    ../core/src/ \
    ../infrastructure/src/ \
    ../utils/src/

SOURCES += \
    src/application/processors/authProcessor.cpp \
    src/application/processors/recipeExplorerProcessor.cpp \
    src/application/processors/userProcessor.cpp \
    src/application/processors/favoriteProcessor.cpp \
    src/application/processors/reviewProcessor.cpp \
    src/application/processors/adminProcessor.cpp \
    src/application/services/recipeFilterService.cpp \
    src/application/services/passwordHasher.cpp \
    src/application/services/ingredientService.cpp

HEADERS += \
    src/application/interfaces/processors/IAuthProcessor.h \
    src/application/interfaces/processors/IRecipeExplorerProcessor.h \
    src/application/interfaces/processors/IUserProcessor.h \
    src/application/interfaces/processors/IFavoriteProcessor.h \
    src/application/interfaces/processors/IReviewProcessor.h \
    src/application/interfaces/processors/IAdminProcessor.h \
    src/application/interfaces/services/IRecipeFilterService.h \
    src/application/interfaces/services/IPasswordHasher.h \
    src/application/interfaces/services/IIngredientService.h \
    src/application/processors/authProcessor.h \
    src/application/processors/recipeExplorerProcessor.h \
    src/application/processors/userProcessor.h \
    src/application/processors/favoriteProcessor.h \
    src/application/processors/reviewProcessor.h \
    src/application/processors/adminProcessor.h \
    src/application/services/recipeFilterService.h \
    src/application/services/passwordHasher.h \
    src/application/services/ingredientService.h

LIBS += -L$$OUT_PWD/../../lib -lcore -linfrastructure -lutils

# Установка выходного каталога
DESTDIR = $$OUT_PWD/../../lib
