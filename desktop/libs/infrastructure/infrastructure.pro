QT += sql
CONFIG += c++17 staticlib

TARGET = infrastructure
TEMPLATE = lib

INCLUDEPATH += src/ \
    ../core/src/ \
    ../utils/src/

SOURCES += \
    src/infrastructure/persistence/database/dbContext.cpp \
    src/infrastructure/persistence/postgresql/recipeRepository.cpp \
    src/infrastructure/persistence/postgresql/userRepository.cpp \
    src/infrastructure/persistence/postgresql/ingredientRepository.cpp \
    src/infrastructure/persistence/postgresql/favoriteRepository.cpp \
    src/infrastructure/persistence/postgresql/reviewRepository.cpp \
    src/infrastructure/config/appConfig.cpp

HEADERS += \
    src/infrastructure/persistence/database/dbContext.h \
    src/infrastructure/repositories/IRecipeRepository.h \
    src/infrastructure/repositories/IUserRepository.h \
    src/infrastructure/repositories/IIngredientRepository.h \
    src/infrastructure/repositories/IFavoriteRepository.h \
    src/infrastructure/repositories/IReviewRepository.h \
    src/infrastructure/persistence/postgresql/recipeRepository.h \
    src/infrastructure/persistence/postgresql/userRepository.h \
    src/infrastructure/persistence/postgresql/ingredientRepository.h \
    src/infrastructure/persistence/postgresql/favoriteRepository.h \
    src/infrastructure/persistence/postgresql/reviewRepository.h \
    src/infrastructure/config/appConfig.h

LIBS += -L$$OUT_PWD/../../lib -lcore -lutils

# Установка выходного каталога
DESTDIR = $$OUT_PWD/../../lib
