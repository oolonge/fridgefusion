#ifndef ADMINRECIPEVIEWIMPL_H
#define ADMINRECIPEVIEWIMPL_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QScrollArea>
#include <QSpinBox>
#include <QTextEdit>
#include "adminRecipeView.h"

class AdminRecipeViewImpl : public QWidget, public AdminRecipeView {
    Q_OBJECT

public:
    explicit AdminRecipeViewImpl(QWidget* parent = nullptr);
    ~AdminRecipeViewImpl() override = default;

    // Реализация AdminRecipeView
    void displayRecipes(const QList<RecipePreviewDTO>& recipes) override;
    void displayRecipeDetails(const RecipeDetailDTO& recipeDetail) override;
    void displayCategories(const QList<Category>& categories) override;
    void displayIngredients(const QList<Ingredient>& ingredients) override;
    void showRecipeDeletedStatus(bool success, const QString& message) override;
    void showRecipeUpdatedStatus(bool success, const QString& message) override;
    void showError(const QString& errorMessage) override;
    void showMessage(const QString& message) override;
    void switchToMain() override;
    int getSelectedRecipeId() const override;

    // Помощники для получения данных из формы
    QString getSearchQuery() const;
    RecipeFormData getRecipeFormData() const override;
    QString getSelectedUnit() const override;

signals:
    void recipeSelected(int recipeId);
    void searchRecipesRequested();
    void deleteRecipeRequested(int recipeId);
    void updateRecipeRequested(int recipeId);
    void refreshRecipesRequested();
    void addCategoryToRecipeRequested(int recipeId, int categoryId);
    void removeCategoryFromRecipeRequested(int recipeId, int categoryId);
    void addIngredientToRecipeRequested(int recipeId, int ingredientId, double quantity);
    void removeIngredientFromRecipeRequested(int recipeId, int ingredientId);
    void updateIngredientQuantityRequested(int recipeId, int ingredientId, double quantity);
    void createRecipeRequested(const QString& name);

private:
    // UI элементы
    QTabWidget* tabWidget;

    // Вкладка списка рецептов
    QWidget* recipesListTab;
    QLineEdit* searchLineEdit;
    QPushButton* searchButton;
    QTableWidget* recipesTableWidget;
    QPushButton* refreshButton;
    QPushButton* addNewRecipeButton;

    // Вкладка деталей рецепта
    QWidget* recipeDetailsTab;
    QLabel* recipeNameLabel;
    QLineEdit* editNameLineEdit;
    QTextEdit* editDescriptionTextEdit;
    QSpinBox* editPrepTimeSpinBox;
    QSpinBox* editCookTimeSpinBox;
    QTextEdit* editInstructionsTextEdit;

    // Секция категорий
    QLineEdit* categorySearchLineEdit;
    QPushButton* addCategoryButton;
    QScrollArea* categoriesScrollArea;
    QWidget* categoriesContainer;
    QLayout* categoriesLayout;

    // Секция ингредиентов
    QLineEdit* ingredientSearchLineEdit;
    QDoubleSpinBox* ingredientQuantitySpinBox;
    QComboBox* ingredientUnitComboBox;
    QPushButton* addIngredientButton;
    QTableWidget* ingredientsTableWidget;

    // Кнопки действий
    QPushButton* updateRecipeButton;
    QPushButton* deleteRecipeButton;
    QPushButton* backToListButton;

    // Данные
    int selectedRecipeId;
    QMap<QString, int> categoryNameToId;
    QMap<QString, int> ingredientNameToId;
    
    // Флаг для блокировки обработки сигналов во время обновления данных
    bool blockSignalsInUpdate;

    void setupUi();
    void setupRecipesListTab();
    void setupRecipeDetailsTab();
    void clearRecipeDetails();
};

#endif // ADMINRECIPEVIEWIMPL_H
