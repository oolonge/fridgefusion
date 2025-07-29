#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "application/processors/authProcessor.h"
#include "application/processors/recipeExplorerProcessor.h"
#include "application/processors/userProcessor.h"
#include "application/processors/favoriteProcessor.h"
#include "application/processors/reviewProcessor.h"
#include "application/processors/adminProcessor.h"

#include "application/services/passwordHasher.h"
#include "application/services/recipeFilterService.h"

#include "infrastructure/persistence/database/dbContext.h"
#include "infrastructure/persistence/postgresql/userRepository.h"
#include "infrastructure/persistence/postgresql/recipeRepository.h"
#include "infrastructure/persistence/postgresql/ingredientRepository.h"
#include "infrastructure/persistence/postgresql/favoriteRepository.h"
#include "infrastructure/persistence/postgresql/reviewRepository.h"

#include "presentation/views/authView.cpp"
#include "presentation/views/recipeExplorerView.cpp"
#include "presentation/views/userView.cpp"
#include "presentation/views/recipeDetailView.cpp"
#include "presentation/presenters/recipeDetailPresenter.cpp"
#include "presentation/presenters/adminPresenter.cpp"
#include "presentation/views/adminViewImpl.h"
#include "presentation/views/adminRecipeViewImpl.h"
#include "presentation/presenters/adminRecipePresenter.cpp"

#include "utils/logging/LoggerService.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    adminButton(nullptr)
{
    ui->setupUi(this);

    // Установка заголовка окна
    setWindowTitle("FridgeFusion");

    // Установка минимального размера окна
    setMinimumSize(1280, 768);

    // Настройка зависимостей
    setupDependencies();

    // Настройка интерфейса
    setupUi();

    // По умолчанию показываем экран рецептов
    showRecipeExplorerView();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupDependencies()
{
    // Создаем DbContext
    auto dbContext = std::make_shared<DbContext>();
    dbContext->connect();

    // Создаем репозитории
    auto userRepository = std::make_shared<UserRepository>(dbContext);
    auto recipeRepository = std::make_shared<RecipeRepository>(dbContext);
    auto ingredientRepository = std::make_shared<IngredientRepository>(dbContext);
    auto favoriteRepository = std::make_shared<FavoriteRepository>(dbContext.get());
    auto reviewRepository = std::make_shared<ReviewRepository>(dbContext.get());

    // Создаем сервисы
    passwordHasher = std::make_shared<PasswordHasher>();
    auto recipeFilterService = std::make_shared<RecipeFilterService>();

    // Создаем процессоры
    authProcessor = std::make_shared<AuthProcessor>(
        userRepository,
        passwordHasher
        );

    recipeExplorerProcessor = std::make_shared<RecipeExplorerProcessor>(
        recipeRepository,
        ingredientRepository,
        recipeFilterService
        );

    userProcessor = std::make_shared<UserProcessor>(
        userRepository,
        favoriteRepository,
        reviewRepository
        );

    favoriteProcessor = std::make_shared<FavoriteProcessor>(
        favoriteRepository.get()
        );

    reviewProcessor = std::make_shared<ReviewProcessor>(
        reviewRepository.get()
        );

    adminProcessor = std::make_shared<AdminProcessor>(
        userRepository,
        favoriteRepository,
        reviewRepository,
        recipeRepository,
        LoggerService::getInstance().getLogger("file")
        );
}

void MainWindow::setupUi()
{
    // Создаем центральный виджет и основной stacked widget
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    mainStackedWidget = new QStackedWidget(centralWidget);

    // Создаем верхнюю панель
    QToolBar *topToolBar = new QToolBar(this);
    topToolBar->setMovable(false);
    topToolBar->setFloatable(false);

    // Создаем метку с названием приложения
    QLabel *appTitleLabel = new QLabel("FridgeFusion", topToolBar);
    QFont titleFont = appTitleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    appTitleLabel->setFont(titleFont);

    // Создаем кнопки для верхней панели
    QPushButton *profileButton = new QPushButton("Войти", topToolBar);
    profileButton->setObjectName("profileButton"); // Добавляем имя объекта для поиска

    QPushButton *favoritesButton = new QPushButton("Избранное", topToolBar);
    favoritesButton->setObjectName("favoritesButton"); // Добавляем имя объекта для поиска
    favoritesButton->setVisible(false); // По умолчанию скрыта

    // Добавляем растягивающийся спейсер между заголовком и кнопками
    QWidget* spacer = new QWidget(topToolBar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    this->adminButton = new QPushButton("Админ-панель", topToolBar);
    this->adminButton->setObjectName("adminButton");
    this->adminButton->setVisible(true);

    // Добавляем элементы в тулбар
    topToolBar->addWidget(appTitleLabel);
    topToolBar->addWidget(spacer);
    topToolBar->addWidget(favoritesButton);
    topToolBar->addWidget(profileButton);
    topToolBar->addWidget(adminButton);

    // Добавляем тулбар в главное окно
    addToolBar(topToolBar);

    // Создаем представления
    // 1. Представление авторизации
    AuthViewImpl *authViewImpl = new AuthViewImpl();
    authViewWidget = authViewImpl;

    // 2. Представление рецептов
    RecipeExplorerViewImpl *recipeExplorerViewImpl = new RecipeExplorerViewImpl();
    recipeExplorerViewWidget = recipeExplorerViewImpl;

    // 3. Представление профиля пользователя
    UserViewImpl *userViewImpl = new UserViewImpl();
    userProfileViewWidget = userViewImpl;

    // 4. Создание представления админ-панели
    AdminViewImpl* adminViewImpl = new AdminViewImpl();
    adminViewWidget = adminViewImpl;

    // 5. Представление деталей рецепта
    recipeDetailViewWidget = new QWidget(); // Временная заглушка

    // Добавляем представления в стек
    mainStackedWidget->addWidget(recipeExplorerViewWidget);
    mainStackedWidget->addWidget(authViewWidget);
    mainStackedWidget->addWidget(userProfileViewWidget);
    mainStackedWidget->addWidget(recipeDetailViewWidget);
    mainStackedWidget->addWidget(adminViewWidget); // Добавляем контейнер админ-панели

    // Создаем презентеры
    authPresenter = std::make_unique<AuthPresenter>(authProcessor, authViewImpl);
    recipeExplorerPresenter = std::make_unique<RecipeExplorerPresenter>(
        recipeExplorerProcessor,
        favoriteProcessor,
        reviewProcessor,
        recipeExplorerViewImpl
        );
    userPresenter = std::make_unique<UserPresenter>(
        userProcessor,
        favoriteProcessor,
        recipeExplorerProcessor,
        userViewImpl
        );

    // Получаем доступ к компонентам админ-панели
    AdminUserViewImpl* userManagementTab = dynamic_cast<AdminUserViewImpl*>(
        adminViewImpl->findChild<AdminUserViewImpl*>());
    
    AdminRecipeViewImpl* recipeManagementTab = dynamic_cast<AdminRecipeViewImpl*>(
        adminViewImpl->findChild<AdminRecipeViewImpl*>());

    adminPresenter = std::make_unique<AdminPresenter>(
        adminProcessor,
        authProcessor,
        userProcessor,
        passwordHasher,
        adminViewImpl
        );

    adminRecipePresenter = std::make_unique<AdminRecipePresenter>(
        recipeExplorerProcessor,
        authProcessor,
        adminProcessor,
        recipeManagementTab
        );

    // Подключаем сигналы
    connect(profileButton, &QPushButton::clicked, [this]() {
        if (authProcessor->isUserLoggedIn()) {
            showUserProfileView();
        } else {
            showAuthView();
        }
    });

    connect(favoritesButton, &QPushButton::clicked, [this, userViewImpl]() {
        showUserProfileView();
        // TODO: Переключиться на вкладку избранного
    });

    connect(authViewImpl, &AuthViewImpl::loginRequested, [this]() {
        authPresenter->login();
    });

    connect(authViewImpl, &AuthViewImpl::registerRequested, [this]() {
        authPresenter->registerUser();
    });

    connect(authViewImpl, &AuthViewImpl::backToMainRequested, [this]() {
        showRecipeExplorerView();
    });

    connect(recipeExplorerViewImpl, &RecipeExplorerViewImpl::recipeSelected, [this](int recipeId) {
        showRecipeDetailView(recipeId);
    });

    connect(recipeExplorerViewImpl, &RecipeExplorerViewImpl::searchRequested, [this]() {
        recipeExplorerPresenter->searchRecipes();
    });

    connect(userViewImpl, &UserViewImpl::backToMainRequested, [this]() {
        showRecipeExplorerView();
    });

    connect(userViewImpl, &UserViewImpl::logoutRequested, this, &MainWindow::logoutUser);

    // Подключаем сигнал для возврата из админ-панели
    connect(adminViewImpl, &AdminViewImpl::backToMainRequested, [this]() {
        showRecipeExplorerView();
    });

    // Подключаем сигналы от userManagementTab через adminViewImpl
    if (userManagementTab) {
        connect(adminViewImpl, &AdminViewImpl::userSelected,
                [this](int userId) { this->adminPresenter->loadUserProfile(userId); });

        connect(adminViewImpl, &AdminViewImpl::searchUsersRequested,
                [this, adminViewImpl]() { this->adminPresenter->searchUsers(
                            adminViewImpl->getSearchQuery()); });

        connect(adminViewImpl, &AdminViewImpl::deleteUserRequested,
                [this](int userId) { this->adminPresenter->deleteUser(userId); });

        connect(adminViewImpl, &AdminViewImpl::updateUserRoleRequested,
                [this](int userId, UserRole role) { this->adminPresenter->updateUserRole(userId, role); });

        connect(adminViewImpl, &AdminViewImpl::refreshUsersRequested,
                [this]() { this->adminPresenter->loadAllUsers(); });

        connect(adminViewImpl, &AdminViewImpl::filterUsersByRoleRequested,
                [this](UserRole role) { this->adminPresenter->filterUsersByRole(role); });

        connect(adminViewImpl, &AdminViewImpl::updateUserDataRequested,
                [this](int userId, const QString& username, const QString& email,
                    const QString& password, const QString& confirmPassword) {
                    this->adminPresenter->updateUserData(userId, username, email, password, confirmPassword);
                });
    }

    // Подключаем сигналы для управления рецептами
    if (recipeManagementTab) {
        connect(recipeManagementTab, &AdminRecipeViewImpl::recipeSelected, [this](int recipeId) {
            adminRecipePresenter->loadRecipeDetails(recipeId);
        });

        connect(recipeManagementTab, &AdminRecipeViewImpl::searchRecipesRequested, [this]() {
            adminRecipePresenter->searchRecipes(
                static_cast<AdminRecipeViewImpl*>(adminRecipeViewWidget)->getSearchQuery()
                );
        });

        connect(recipeManagementTab, &AdminRecipeViewImpl::refreshRecipesRequested, [this]() {
            adminRecipePresenter->loadAllRecipes();
        });

        connect(recipeManagementTab, &AdminRecipeViewImpl::deleteRecipeRequested, [this](int recipeId) {
            adminRecipePresenter->deleteRecipe(recipeId);
        });

        connect(recipeManagementTab, &AdminRecipeViewImpl::updateRecipeRequested, [this](int recipeId) {
            adminRecipePresenter->updateRecipe(recipeId);
        });

        connect(recipeManagementTab, &AdminRecipeViewImpl::addCategoryToRecipeRequested,
                [this](int recipeId, int categoryId) {
                    adminRecipePresenter->addCategoryToRecipe(recipeId, categoryId);
                });

        connect(recipeManagementTab, &AdminRecipeViewImpl::removeCategoryFromRecipeRequested,
                [this](int recipeId, int categoryId) {
                    adminRecipePresenter->removeCategoryFromRecipe(recipeId, categoryId);
                });

        connect(recipeManagementTab, &AdminRecipeViewImpl::addIngredientToRecipeRequested,
                [this](int recipeId, int ingredientId, double quantity) {
                    adminRecipePresenter->addIngredientToRecipe(recipeId, ingredientId, quantity);
                });

        connect(recipeManagementTab, &AdminRecipeViewImpl::removeIngredientFromRecipeRequested,
                [this](int recipeId, int ingredientId) {
                    adminRecipePresenter->removeIngredientFromRecipe(recipeId, ingredientId);
                });

        connect(recipeManagementTab, &AdminRecipeViewImpl::updateIngredientQuantityRequested,
                [this](int recipeId, int ingredientId, double quantity) {
                    adminRecipePresenter->updateIngredientQuantity(recipeId, ingredientId, quantity);
                });

        connect(recipeManagementTab, SIGNAL(createRecipeRequested(const QString&)),
                this, SLOT(onCreateRecipeRequested(const QString&)));
    }

    connect(userViewImpl, &UserViewImpl::profileUpdateRequested, [this]() {
        userPresenter->updateUserProfile(authProcessor->getCurrentUserId());
    });

    connect(userViewImpl, &UserViewImpl::recipeSelected, [this](int recipeId) {
        showRecipeDetailView(recipeId);
    });

    connect(userViewImpl, &UserViewImpl::removeFromFavoritesRequested, [this](int recipeId) {
        if (authProcessor->isUserLoggedIn()) {
            int userId = authProcessor->getCurrentUserId();
            userPresenter->removeFromFavorites(userId, recipeId);
        }
    });

    connect(authViewImpl, &AuthViewImpl::loginStateChanged,
            [this, profileButton, favoritesButton](bool isLoggedIn, const QString& username) {
                if (isLoggedIn) {
                    profileButton->setText(username);
                    favoritesButton->setVisible(true);

                    QPointer<QPushButton> adminBtn = this->adminButton;
                    if (adminBtn) {
                        adminBtn->setVisible(authProcessor->isCurrentUserAdmin());
                    }
                } else {
                    profileButton->setText("Войти");
                    favoritesButton->setVisible(false);

                    QPointer<QPushButton> adminBtn = this->adminButton;
                    if (adminBtn) {
                        adminBtn->setVisible(false);
                    }
                }
            });

    // Подключаем кнопку "Админ-панель" к показу админ-панели
    QObject::connect(this->adminButton, &QPushButton::clicked, this, &MainWindow::showAdminPanel);

    // Добавляем стек в главный лейаут
    mainLayout->addWidget(mainStackedWidget);

    // Устанавливаем центральный виджет
    setCentralWidget(centralWidget);

    // Устанавливаем статусбар
    statusBar()->showMessage("Готово к использованию");

    // Загружаем начальные данные
    recipeExplorerPresenter->loadAllRecipes();
    recipeExplorerPresenter->loadCategories();
    recipeExplorerPresenter->loadIngredients();
}

void MainWindow::updateLoginState(bool isLoggedIn, const QString& username) {
    if (isLoggedIn) {
        // Находим кнопку профиля по имени
        QPushButton* profileButton = findChild<QPushButton*>("profileButton");
        if (profileButton) {
            profileButton->setText(username);
        }

        // Находим кнопку избранного
        QPushButton* favoritesButton = findChild<QPushButton*>("favoritesButton");
        if (favoritesButton) {
            favoritesButton->setVisible(true);
        }

        safeSetAdminButtonVisible(authProcessor->isCurrentUserAdmin());
    } else {
        // Находим кнопку профиля по имени
        QPushButton* profileButton = findChild<QPushButton*>("profileButton");
        if (profileButton) {
            profileButton->setText("Войти");

            if (profileButton->menu()) {
                delete profileButton->menu();
                profileButton->setMenu(nullptr);
            }
        }

        // Находим кнопку избранного
        QPushButton* favoritesButton = findChild<QPushButton*>("favoritesButton");
        if (favoritesButton) {
            favoritesButton->setVisible(false);
        }

        // Безопасно устанавливаем видимость кнопки администратора
        safeSetAdminButtonVisible(false);
    }
}

void MainWindow::showAuthView()
{
    mainStackedWidget->setCurrentWidget(authViewWidget);
}

void MainWindow::showRecipeExplorerView()
{
    mainStackedWidget->setCurrentWidget(recipeExplorerViewWidget);
}

void MainWindow::showUserProfileView()
{
    if (authProcessor->isUserLoggedIn()) {
        int userId = authProcessor->getCurrentUserId();
        userPresenter->loadUserProfile(userId);
        userPresenter->loadFavoriteRecipes(userId);
        mainStackedWidget->setCurrentWidget(userProfileViewWidget);
    } else {
        showAuthView();
    }
}

void MainWindow::showRecipeDetailView(int recipeId) {
    RecipeDetailViewImpl* recipeDetailViewImpl = dynamic_cast<RecipeDetailViewImpl*>(recipeDetailViewWidget);
    if (!recipeDetailViewImpl) {
        recipeDetailViewImpl = new RecipeDetailViewImpl();
        recipeDetailViewWidget = recipeDetailViewImpl;
        mainStackedWidget->addWidget(recipeDetailViewWidget);

        // Создаем презентер для деталей рецепта
        recipeDetailPresenter = std::make_unique<RecipeDetailPresenter>(
            recipeExplorerProcessor,
            favoriteProcessor,
            reviewProcessor,
            authProcessor,
            recipeDetailViewImpl
            );

        // Подключаем сигналы
        connect(recipeDetailViewImpl, &RecipeDetailViewImpl::backRequested, [this]() {
            // Проверяем, откуда был переход к рецепту
            RecipeDetailViewImpl* detailView = dynamic_cast<RecipeDetailViewImpl*>(recipeDetailViewWidget);
            if (detailView && detailView->returnToProfile) {
                showUserProfileView();
            } else {
                showRecipeExplorerView();
            }
        });

        connect(recipeDetailViewImpl, &RecipeDetailViewImpl::favoriteToggled, [this](int recipeId) {
            recipeDetailPresenter->toggleFavorite(recipeId);
        });

        connect(recipeDetailViewImpl, &RecipeDetailViewImpl::reviewSubmitted, [this](int recipeId) {
            recipeDetailPresenter->submitReview(recipeId);
        });
    }

    // Устанавливаем флаг возврата в профиль
    recipeDetailViewImpl->setReturnToProfile(lastRecipeFromProfile);

    // Загружаем детали рецепта
    recipeDetailPresenter->loadRecipeDetails(recipeId);

    // Показываем экран деталей рецепта
    mainStackedWidget->setCurrentWidget(recipeDetailViewWidget);
}

void MainWindow::showAdminPanel() {
    if (!adminButton) {
        QMessageBox::warning(this, "Ошибка", "Кнопка админ-панели не инициализирована");
        return;
    }

    if (authProcessor->isUserLoggedIn() && authProcessor->isCurrentUserAdmin()) {
        // Загружаем данные для обеих вкладок
        adminPresenter->loadAllUsers();
        adminRecipePresenter->loadAllRecipes();
        adminRecipePresenter->loadCategories();
        adminRecipePresenter->loadIngredients();

        // Показываем админ-панель
        mainStackedWidget->setCurrentWidget(adminViewWidget);
    } else {
        QMessageBox::warning(this, "Отказ в доступе",
                            "У вас нет прав администратора для доступа к этому разделу");
    }
}

void MainWindow::safeSetAdminButtonVisible(bool visible) {
    if (adminButton) {
        adminButton->setVisible(visible);
    }
}

void MainWindow::onCreateRecipeRequested(const QString& name) {
    adminRecipePresenter->createRecipe(name);
}

void MainWindow::logoutUser() {
    authPresenter->logout();
    showRecipeExplorerView();
}
