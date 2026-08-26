#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <memory>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QMessageBox>
#include <QPointer>

#include "application/interfaces/processors/IAuthProcessor.h"
#include "application/interfaces/processors/IRecipeExplorerProcessor.h"
#include "application/interfaces/processors/IUserProcessor.h"
#include "application/interfaces/processors/IFavoriteProcessor.h"
#include "application/interfaces/processors/IReviewProcessor.h"

#include "presentation/presenters/authPresenter.h"
#include "presentation/presenters/recipeExplorerPresenter.h"
#include "presentation/presenters/userPresenter.h"
#include "presentation/presenters/recipeDetailPresenter.h"
#include "presentation/presenters/adminPresenter.h"
#include "presentation/presenters/adminRecipePresenter.h"

#include "presentation/views/authView.h"
#include "presentation/views/recipeExplorerView.h"
#include "presentation/views/userView.h"
#include "presentation/views/recipeDetailView.h"
#include "presentation/views/adminView.h"
#include "presentation/views/adminRecipeView.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showAuthView();
    void showRecipeExplorerView();
    void showUserProfileView();
    void showRecipeDetailView(int recipeId);
    void logoutUser();
    void updateLoginState(bool isLoggedIn, const QString& username);
    void showAdminPanel();
    void onCreateRecipeRequested(const QString& name);

private:
    Ui::MainWindow *ui;
    QStackedWidget *mainStackedWidget;

    // Views
    QWidget *authViewWidget;
    QWidget *recipeExplorerViewWidget;
    QWidget *userProfileViewWidget;
    QWidget *recipeDetailViewWidget;
    QWidget *adminViewWidget;
    QWidget *adminRecipeViewWidget;

    QPushButton* adminButton = nullptr;
    void safeSetAdminButtonVisible(bool visible);

    // Presenters
    std::unique_ptr<AuthPresenter> authPresenter;
    std::unique_ptr<RecipeExplorerPresenter> recipeExplorerPresenter;
    std::unique_ptr<UserPresenter> userPresenter;
    std::unique_ptr<RecipeDetailPresenter> recipeDetailPresenter;
    std::unique_ptr<AdminPresenter> adminPresenter;
    std::unique_ptr<AdminRecipePresenter> adminRecipePresenter;

    std::shared_ptr<IPasswordHasher> passwordHasher;

    // Processors
    std::shared_ptr<IAuthProcessor> authProcessor;
    std::shared_ptr<IRecipeExplorerProcessor> recipeExplorerProcessor;
    std::shared_ptr<IUserProcessor> userProcessor;
    std::shared_ptr<IFavoriteProcessor> favoriteProcessor;
    std::shared_ptr<IReviewProcessor> reviewProcessor;
    std::shared_ptr<IAdminProcessor> adminProcessor;

    bool lastRecipeFromProfile = false;

    // Метод инициализации
    void setupDependencies();
    void setupUi();
};

#endif // MAINWINDOW_H
