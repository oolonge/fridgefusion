QT -= gui
QT += sql
CONFIG += c++17 staticlib

TARGET = core
TEMPLATE = lib

INCLUDEPATH += src/ \
    src/domain/ \
    src/domain/entities/ \
    src/domain/dtos/ \
    src/domain/value_objects/

SOURCES += \
    src/domain/entities/user.cpp

HEADERS += \
    src/domain/entities/category.h \
    src/domain/entities/ingredient.h \
    src/domain/entities/recipe.h \
    src/domain/entities/recipeIngredient.h \
    src/domain/entities/user.h \
    src/domain/entities/favorite.h \
    src/domain/entities/review.h \
    src/domain/dtos/authResult.h \
    src/domain/dtos/missingIngredientInfo.h \
    src/domain/dtos/recipeDetailDTO.h \
    src/domain/dtos/recipeFilterDTO.h \
    src/domain/dtos/recipePreviewDTO.h \
    src/domain/dtos/sortedRecipeDTO.h \
    src/domain/dtos/userProfileDTO.h \
    src/domain/dtos/userRegistrationDTO.h \
    src/domain/value_objects/searchCriteria.h

# Установка выходного каталога
DESTDIR = $$OUT_PWD/../../lib
