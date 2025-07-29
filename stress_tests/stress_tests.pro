QT += testlib
QT += gui
QT += sql
CONFIG += qt warn_on depend_includepath testcase

# Отключаем создание пакета приложения
CONFIG -= app_bundle

TEMPLATE = app
TARGET = stress_tests

macx {
    INCLUDEPATH += /opt/homebrew/opt/libpq/include
    LIBS += -L/opt/homebrew/opt/libpq/lib -lpq
    QMAKE_RPATHDIR += /opt/homebrew/opt/libpq/lib
    QMAKE_LFLAGS += -Wl,-rpath,/opt/homebrew/opt/libpq/lib
}

INCLUDEPATH += ../src/

SOURCES += \
    main.cpp \
    tst_stresstest.cpp \
    ../src/application/services/passwordHasher.cpp \
    ../src/application/services/recipeFilterService.cpp \
    ../src/application/processors/authProcessor.cpp \
    ../src/application/processors/userProcessor.cpp \
    ../src/application/processors/recipeExplorerProcessor.cpp \
    ../src/application/processors/favoriteProcessor.cpp \
    ../src/application/processors/reviewProcessor.cpp \
    ../src/domain/entities/user.cpp \
    ../src/infrastructure/persistence/database/dbContext.cpp \
    ../src/infrastructure/config/appConfig.cpp \
    ../src/infrastructure/persistence/postgresql/userRepository.cpp \
    ../src/infrastructure/persistence/postgresql/recipeRepository.cpp \
    ../src/infrastructure/persistence/postgresql/ingredientRepository.cpp \
    ../src/infrastructure/persistence/postgresql/favoriteRepository.cpp \
    ../src/infrastructure/persistence/postgresql/reviewRepository.cpp \
    ../src/utils/logging/BaseLogger.cpp \
    ../src/utils/logging/FileLogger.cpp \
    ../src/utils/logging/ConsoleLogger.cpp \
    ../src/utils/logging/LoggerService.cpp

HEADERS += \
    ../src/domain/entities/recipe.h \
    ../src/domain/entities/ingredient.h \
    ../src/domain/entities/user.h \
    ../src/domain/entities/recipeIngredient.h \
    ../src/domain/entities/category.h \
    ../src/domain/entities/favorite.h \
    ../src/domain/entities/review.h \
    ../src/domain/dtos/recipeDetailDTO.h \
    ../src/domain/dtos/recipePreviewDTO.h \
    ../src/domain/dtos/recipeFilterDTO.h \
    ../src/domain/dtos/userProfileDTO.h \
    ../src/domain/dtos/authResult.h \
    ../src/application/interfaces/processors/IRecipeExplorerProcessor.h \
    ../src/application/interfaces/processors/IUserProcessor.h \
    ../src/application/interfaces/processors/IAuthProcessor.h \
    ../src/application/interfaces/processors/IFavoriteProcessor.h \
    ../src/application/interfaces/processors/IReviewProcessor.h \
    ../src/application/interfaces/services/IRecipeFilterService.h \
    ../src/application/interfaces/services/IPasswordHasher.h \
    ../src/application/processors/recipeExplorerProcessor.h \
    ../src/application/processors/userProcessor.h \
    ../src/application/processors/authProcessor.h \
    ../src/application/processors/favoriteProcessor.h \
    ../src/application/processors/reviewProcessor.h \
    ../src/application/services/recipeFilterService.h \
    ../src/application/services/passwordHasher.h \
    ../src/infrastructure/repositories/IRecipeRepository.h \
    ../src/infrastructure/repositories/IUserRepository.h \
    ../src/infrastructure/repositories/IIngredientRepository.h \
    ../src/infrastructure/repositories/IFavoriteRepository.h \
    ../src/infrastructure/repositories/IReviewRepository.h \
    ../src/infrastructure/persistence/database/dbContext.h \
    ../src/infrastructure/config/appConfig.h \
    ../src/infrastructure/persistence/postgresql/userRepository.h \
    ../src/infrastructure/persistence/postgresql/recipeRepository.h \
    ../src/infrastructure/persistence/postgresql/ingredientRepository.h \
    ../src/infrastructure/persistence/postgresql/favoriteRepository.h \
    ../src/infrastructure/persistence/postgresql/reviewRepository.h \
    ../src/utils/logging/ILogger.h \
    ../src/utils/logging/BaseLogger.h \
    ../src/utils/logging/FileLogger.h \
    ../src/utils/logging/ConsoleLogger.h \
    ../src/utils/logging/LoggerService.h
