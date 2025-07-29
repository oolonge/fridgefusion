QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Пути для PostgreSQL для macOS
macx {
    INCLUDEPATH += /opt/homebrew/opt/libpq/include
    LIBS += -L/opt/homebrew/opt/libpq/lib -lpq

    # Обеспечиваем поиск драйверов во время выполнения
    QMAKE_RPATHDIR += /opt/homebrew/opt/libpq/lib

    # Исправляем проблему с драйвером PSQL
    QMAKE_LFLAGS += -Wl,-rpath,/opt/homebrew/opt/libpq/lib
}

INCLUDEPATH += src/

SOURCES += \
    app/main.cpp \
    src/presentation/forms/mainwindow.cpp \
    src/application/processors/recipeExplorerProcessor.cpp \
    src/application/processors/userProcessor.cpp \
    src/application/processors/authProcessor.cpp \
    src/application/processors/favoriteProcessor.cpp \
    src/application/processors/reviewProcessor.cpp \
    src/application/services/recipeFilterService.cpp \
    src/application/services/ingredientService.cpp \
    src/application/services/passwordHasher.cpp \
    src/infrastructure/persistence/postgresql/recipeRepository.cpp \
    src/infrastructure/persistence/postgresql/userRepository.cpp \
    src/infrastructure/persistence/postgresql/ingredientRepository.cpp \
    src/infrastructure/persistence/postgresql/favoriteRepository.cpp \
    src/infrastructure/persistence/postgresql/reviewRepository.cpp \
    src/infrastructure/persistence/database/dbContext.cpp \
    src/infrastructure/config/appConfig.cpp \
    src/presentation/views/recipeExplorerView.cpp \
    src/presentation/views/userView.cpp \
    src/presentation/views/authView.cpp \
    src/presentation/views/recipeDetailView.cpp \
    src/presentation/presenters/recipeExplorerPresenter.cpp \
    src/presentation/presenters/userPresenter.cpp \
    src/presentation/presenters/authPresenter.cpp \
    src/presentation/presenters/recipeDetailPresenter.cpp \
    src/utils/logger.cpp \
    src/utils/logging/BaseLogger.cpp \
    src/utils/logging/FileLogger.cpp \
    src/utils/logging/ConsoleLogger.cpp \
    src/utils/logging/LoggerService.cpp \
    src/application/processors/adminProcessor.cpp \
    src/presentation/presenters/adminPresenter.cpp \
    src/presentation/views/adminView.cpp \
    src/presentation/views/adminViewImpl.cpp \
    src/presentation/views/adminUserViewImpl.cpp \
    src/presentation/views/adminRecipeView.cpp \
    src/presentation/views/adminRecipeViewImpl.cpp \
    src/presentation/views/dialogs/addRecipeDialog.cpp \
    src/presentation/presenters/adminRecipePresenter.cpp \
    src/domain/entities/user.cpp

HEADERS += \
    src/domain/dtos/recipeDetailDTO.h \
    src/domain/dtos/recipePreviewDTO.h \
    src/presentation/forms/mainwindow.h \
    src/domain/entities/recipe.h \
    src/domain/entities/ingredient.h \
    src/domain/entities/user.h \
    src/domain/entities/recipeIngredient.h \
    src/domain/entities/category.h \
    src/domain/entities/favorite.h \
    src/domain/entities/review.h \
    src/domain/dtos/recipeFilterDTO.h \
    src/domain/dtos/sortedRecipeDTO.h \
    src/domain/dtos/userProfileDTO.h \
    src/domain/dtos/userRegistrationDTO.h \
    src/domain/dtos/authResult.h \
    src/domain/value_objects/searchCriteria.h \
    src/application/interfaces/processors/IRecipeExplorerProcessor.h \
    src/application/interfaces/processors/IUserProcessor.h \
    src/application/interfaces/processors/IAuthProcessor.h \
    src/application/interfaces/processors/IFavoriteProcessor.h \
    src/application/interfaces/processors/IReviewProcessor.h \
    src/application/interfaces/services/IRecipeFilterService.h \
    src/application/interfaces/services/IIngredientService.h \
    src/application/interfaces/services/IPasswordHasher.h \
    src/application/processors/recipeExplorerProcessor.h \
    src/application/processors/userProcessor.h \
    src/application/processors/authProcessor.h \
    src/application/processors/favoriteProcessor.h \
    src/application/processors/reviewProcessor.h \
    src/application/services/recipeFilterService.h \
    src/application/services/ingredientService.h \
    src/application/services/passwordHasher.h \
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
    src/infrastructure/persistence/database/dbContext.h \
    src/infrastructure/config/appConfig.h \
    src/presentation/views/adminViewImpl.h \
    src/presentation/views/adminUserView.h \
    src/presentation/views/adminUserViewImpl.h \
    src/presentation/views/recipeExplorerView.h \
    src/presentation/views/userView.h \
    src/presentation/views/authView.h \
    src/presentation/views/recipeDetailView.h \
    src/presentation/presenters/recipeExplorerPresenter.h \
    src/presentation/presenters/userPresenter.h \
    src/presentation/presenters/authPresenter.h \
    src/presentation/presenters/recipeDetailPresenter.h \
    src/utils/logger.h \
    src/utils/exceptions.h \
    src/utils/helpers.h \
    src/utils/logging/ILogger.h \
    src/utils/logging/BaseLogger.h \
    src/utils/logging/FileLogger.h \
    src/utils/logging/ConsoleLogger.h \
    src/utils/logging/LoggerService.h \
    src/application/interfaces/processors/IAdminProcessor.h \
    src/application/processors/adminProcessor.h \
    src/presentation/presenters/adminPresenter.h \
    src/presentation/views/adminView.h \
    src/presentation/views/adminRecipeView.h \
    src/presentation/views/adminRecipeViewImpl.h \
    src/presentation/views/dialogs/addRecipeDialog.h \
    src/presentation/presenters/adminRecipePresenter.h

FORMS += \
    src/presentation/forms/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# RESOURCES += \
#    resources.qrc
