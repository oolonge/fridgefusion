#include "recipeExplorerView.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>
#include <QComboBox>
#include <QSpinBox>
#include <QCompleter>
#include <QStringList>
#include <QMessageBox>
#include <QMouseEvent>

// Вспомогательный класс для размещения тегов в несколько строк
class QFlowLayout : public QLayout {
public:
    explicit QFlowLayout(QWidget *parent = nullptr, int margin = 0, int spacing = -1)
        : QLayout(parent) {
        setContentsMargins(margin, margin, margin, margin);
        setSpacing(spacing);
    }

    ~QFlowLayout() {
        QLayoutItem *item;
        while ((item = takeAt(0)))
            delete item;
    }

    void addItem(QLayoutItem *item) override {
        itemList.append(item);
    }

    int count() const override {
        return itemList.size();
    }

    QLayoutItem *itemAt(int index) const override {
        return itemList.value(index);
    }

    QLayoutItem *takeAt(int index) override {
        if (index >= 0 && index < itemList.size())
            return itemList.takeAt(index);
        return nullptr;
    }

    Qt::Orientations expandingDirections() const override {
        return {};
    }

    bool hasHeightForWidth() const override {
        return true;
    }

    int heightForWidth(int width) const override {
        int height = doLayout(QRect(0, 0, width, 0), true);
        return height;
    }

    void setGeometry(const QRect &rect) override {
        QLayout::setGeometry(rect);
        doLayout(rect, false);
    }

    QSize sizeHint() const override {
        return minimumSize();
    }

    QSize minimumSize() const override {
        QSize size;
        for (const QLayoutItem *item : itemList)
            size = size.expandedTo(item->minimumSize());

        size += QSize(2*this->contentsMargins().left(), 2*this->contentsMargins().top());
        return size;
    }

private:
    int doLayout(const QRect &rect, bool testOnly) const {
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
        int x = effectiveRect.x();
        int y = effectiveRect.y();
        int lineHeight = 0;

        for (QLayoutItem *item : itemList) {
            QWidget *wid = item->widget();
            int spaceX = spacing();
            int spaceY = spacing();
            if (spaceX == -1)
                spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
            if (spaceY == -1)
                spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
            int nextX = x + item->sizeHint().width() + spaceX;
            if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
                x = effectiveRect.x();
                y = y + lineHeight + spaceY;
                nextX = x + item->sizeHint().width() + spaceX;
                lineHeight = 0;
            }

            if (!testOnly)
                item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

            x = nextX;
            lineHeight = qMax(lineHeight, item->sizeHint().height());
        }
        return y + lineHeight - rect.y() + bottom;
    }

    QList<QLayoutItem *> itemList;
};

// Класс для представления выбранного элемента (тега)
class SelectedItemTag : public QFrame {
    Q_OBJECT

public:
    explicit SelectedItemTag(int id, const QString& name, QWidget* parent = nullptr)
        : QFrame(parent), m_id(id), m_name(name) {
        setFrameShape(QFrame::StyledPanel);
        setStyleSheet("background-color: #f0f0f0; border-radius: 4px; padding: 2px 4px;");

        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(4, 2, 4, 2);
        layout->setSpacing(4);

        QLabel* nameLabel = new QLabel(name);
        nameLabel->setStyleSheet("background-color: transparent;");

        QPushButton* removeButton = new QPushButton("✕");
        removeButton->setFixedSize(16, 16);
        removeButton->setStyleSheet("QPushButton { background-color: transparent; border: none; color: #666; } QPushButton:hover { color: #000; }");

        layout->addWidget(nameLabel);
        layout->addWidget(removeButton);

        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        connect(removeButton, &QPushButton::clicked, this, [this]() {
            emit removeRequested(m_id);
        });
    }

    int getId() const { return m_id; }
    QString getName() const { return m_name; }

signals:
    void removeRequested(int id);

private:
    int m_id;
    QString m_name;
};

// Измененный класс RecipeCard в файле src-presentation-views-recipeExplorerView.cpp
class RecipeCard : public QFrame {
    Q_OBJECT

public:
    explicit RecipeCard(int recipeId, const QString& name, int prepTime, int activeTime,
                        int calories, const QStringList& dietTags, bool isFavorite = false, QWidget* parent = nullptr)
        : QFrame(parent), m_recipeId(recipeId)
    {
        setFrameShape(QFrame::StyledPanel);
        setLineWidth(1);
        setCursor(Qt::PointingHandCursor);

        // Создаем вертикальный layout для всей карточки
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(10, 10, 10, 10);
        mainLayout->setSpacing(5);

        // Контейнер для заголовка и кнопки избранного (если нужна)
        QHBoxLayout* headerLayout = new QHBoxLayout();
        headerLayout->setSpacing(5);

        // Название рецепта с увеличенным шрифтом
        QLabel* nameLabel = new QLabel(name);
        QFont nameFont = nameLabel->font();
        nameFont.setPointSize(14);
        nameFont.setBold(true);
        nameLabel->setFont(nameFont);

        headerLayout->addWidget(nameLabel, 1);

        // Кнопка избранного (если пользователь авторизован)
        if (isFavorite) {
            favoriteButton = new QPushButton("★");
            favoriteButton->setFixedSize(30, 30);
            connect(favoriteButton, &QPushButton::clicked, this, &RecipeCard::favoriteToggled);
            headerLayout->addWidget(favoriteButton);
        } else {
            favoriteButton = nullptr;
        }

        mainLayout->addLayout(headerLayout);

        // Создаем две колонки для данных рецепта
        QGridLayout* infoLayout = new QGridLayout();
        infoLayout->setColumnStretch(0, 1); // Левая колонка
        infoLayout->setColumnStretch(1, 1); // Правая колонка
        infoLayout->setHorizontalSpacing(15); // Расстояние между колонками

        // Информация о времени и калориях
        QLabel* prepTimeLabel = new QLabel(QString("Время: %1 мин").arg(prepTime));
        QLabel* activeTimeLabel = new QLabel(QString("Активно: %1 мин").arg(activeTime));
        QLabel* caloriesLabel = new QLabel(QString("Калории: %1 ккал").arg(calories));

        // Информация о диете
        QString dietTagsStr = dietTags.join(", ");
        QLabel* dietTagsLabel = new QLabel(QString("Диета: %1").arg(dietTagsStr));

        // Добавляем информацию в левую колонку
        infoLayout->addWidget(prepTimeLabel, 0, 0);
        infoLayout->addWidget(caloriesLabel, 1, 0);

        // Добавляем информацию в правую колонку
        infoLayout->addWidget(activeTimeLabel, 0, 1);
        infoLayout->addWidget(dietTagsLabel, 1, 1);

        mainLayout->addLayout(infoLayout);
    }

    int getRecipeId() const { return m_recipeId; }

signals:
    void clicked(int recipeId);
    void favoriteToggled(int recipeId);

protected:
    void mousePressEvent(QMouseEvent* event) override {
        emit clicked(m_recipeId);
        QFrame::mousePressEvent(event);
    }

private:
    int m_recipeId;
    QPushButton* favoriteButton;
};

class RecipeExplorerViewImpl : public QWidget, public RecipeExplorerView {
    Q_OBJECT

public:
    explicit RecipeExplorerViewImpl(QWidget* parent = nullptr);
    ~RecipeExplorerViewImpl() override = default;

    // Реализация интерфейса RecipeExplorerView
    void displayRecipes(const QList<RecipePreviewDTO>& recipes) override;
    void displayCategories(const QList<Category>& categories) override;
    void displayIngredients(const QList<Ingredient>& ingredients) override;
    void showNoRecipesFound() override;
    void displaySearchResults(const QList<RecipePreviewDTO>& recipes) override;
    QList<int> getSelectedIngredientIds() override;
    QList<int> getSelectedCategoryIds() override;
    int getMaxCookingTime() override;
    void showRecipeDetails(int recipeId) override;

signals:
    void recipeSelected(int recipeId);
    void searchRequested();
    void favoritesToggled(int recipeId);
    void filterApplied();

public slots:
    void addIngredientFromSearch();
    void removeSelectedIngredient(int ingredientId);
    void onIngredientCompleterActivated(const QString& text);

    void addCategoryFromSearch();
    void removeSelectedCategory(int categoryId);
    void onCategoryCompleterActivated(const QString& text);

private:
    // UI элементы
    QLineEdit* ingredientSearchLineEdit;
    QLineEdit* categorySearchLineEdit;
    QPushButton* searchButton;
    QFrame* filterFrame;
    QSpinBox* maxCookingTimeSpinBox;
    QPushButton* applyFiltersButton;
    QPushButton* resetFiltersButton;
    QScrollArea* recipesScrollArea;
    QWidget* recipesContainer;
    QVBoxLayout* recipesLayout;

    QWidget* selectedIngredientsContainer;
    QFlowLayout* selectedIngredientsLayout;

    QWidget* selectedCategoriesContainer;
    QFlowLayout* selectedCategoriesLayout;


    // Данные
    QMap<QString, int> ingredientNameToId;
    QMap<QString, int> categoryNameToId;
    QList<int> selectedIngredientIds;
    QList<int> selectedCategoryIds;
    QMap<int, SelectedItemTag*> selectedIngredientTags;
    QMap<int, SelectedItemTag*> selectedCategoryTags;

    void setupUi();
    QWidget* createRecipeCard(const RecipePreviewDTO& recipe);
    void clearRecipesList();
};

RecipeExplorerViewImpl::RecipeExplorerViewImpl(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void RecipeExplorerViewImpl::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10); // Уменьшаем вертикальные отступы

    // Создаем верхнюю панель с поиском по ингредиентам и категориям
    QGridLayout* searchBarLayout = new QGridLayout();
    searchBarLayout->setHorizontalSpacing(10);
    searchBarLayout->setVerticalSpacing(5);

    // Добавляем надписи и поля ввода в одну строку сетки
    QLabel* ingredientLabel = new QLabel("Ингредиенты:");
    searchBarLayout->addWidget(ingredientLabel, 0, 0, Qt::AlignVCenter | Qt::AlignRight);

    ingredientSearchLineEdit = new QLineEdit();
    ingredientSearchLineEdit->setPlaceholderText("Введите ингредиенты...");
    searchBarLayout->addWidget(ingredientSearchLineEdit, 0, 1);

    QLabel* categoryLabel = new QLabel("Категории:");
    searchBarLayout->addWidget(categoryLabel, 0, 2, Qt::AlignVCenter | Qt::AlignRight);

    categorySearchLineEdit = new QLineEdit();
    categorySearchLineEdit->setPlaceholderText("Введите категории...");
    searchBarLayout->addWidget(categorySearchLineEdit, 0, 3);

    searchButton = new QPushButton("Поиск");
    searchBarLayout->addWidget(searchButton, 0, 4);

    // Добавляем области для тегов во вторую строку
    QScrollArea* ingredientsScrollArea = new QScrollArea();
    ingredientsScrollArea->setWidgetResizable(true);
    ingredientsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ingredientsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ingredientsScrollArea->setFixedHeight(100);
    ingredientsScrollArea->setFrameShape(QFrame::StyledPanel);

    selectedIngredientsContainer = new QWidget();
    selectedIngredientsLayout = new QFlowLayout(selectedIngredientsContainer);
    selectedIngredientsLayout->setContentsMargins(5, 5, 5, 5);
    selectedIngredientsLayout->setSpacing(5);

    ingredientsScrollArea->setWidget(selectedIngredientsContainer);
    searchBarLayout->addWidget(ingredientsScrollArea, 1, 0, 1, 2);

    QScrollArea* categoriesScrollArea = new QScrollArea();
    categoriesScrollArea->setWidgetResizable(true);
    categoriesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    categoriesScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    categoriesScrollArea->setFixedHeight(100);
    categoriesScrollArea->setFrameShape(QFrame::StyledPanel);

    selectedCategoriesContainer = new QWidget();
    selectedCategoriesLayout = new QFlowLayout(selectedCategoriesContainer);
    selectedCategoriesLayout->setContentsMargins(5, 5, 5, 5);
    selectedCategoriesLayout->setSpacing(5);

    categoriesScrollArea->setWidget(selectedCategoriesContainer);
    searchBarLayout->addWidget(categoriesScrollArea, 1, 2, 1, 3);

    // Настройка расширения колонок (чтобы поля поиска занимали больше места)
    searchBarLayout->setColumnStretch(0, 0); // Надпись "Ингредиенты:" - не растягивается
    searchBarLayout->setColumnStretch(1, 2); // Поле поиска ингредиентов - растягивается с весом 2
    searchBarLayout->setColumnStretch(2, 0); // Надпись "Категории:" - не растягивается
    searchBarLayout->setColumnStretch(3, 2); // Поле поиска категорий - растягивается с весом 2
    searchBarLayout->setColumnStretch(4, 0); // Кнопка поиска - не растягивается

    // Добавляем layout поиска в основной layout
    mainLayout->addLayout(searchBarLayout);

    // Создаем панель фильтров (изначально скрытую)
    filterFrame = new QFrame();
    filterFrame->setFrameShape(QFrame::StyledPanel);
    filterFrame->setVisible(false);
    QGridLayout* filterLayout = new QGridLayout(filterFrame);
    filterLayout->setContentsMargins(5, 5, 5, 5);

    // Добавляем элементы фильтрации
    QLabel* maxTimeLabel = new QLabel("Макс. время приготовления (мин):");
    maxCookingTimeSpinBox = new QSpinBox();
    maxCookingTimeSpinBox->setRange(5, 180);
    maxCookingTimeSpinBox->setSingleStep(5);
    maxCookingTimeSpinBox->setValue(60);

    applyFiltersButton = new QPushButton("Применить");
    resetFiltersButton = new QPushButton("Сбросить");

    // Добавляем элементы в layout фильтров
    filterLayout->addWidget(maxTimeLabel, 0, 0);
    filterLayout->addWidget(maxCookingTimeSpinBox, 0, 1);

    QHBoxLayout* filterButtonsLayout = new QHBoxLayout();
    filterButtonsLayout->addWidget(resetFiltersButton);
    filterButtonsLayout->addWidget(applyFiltersButton);
    filterLayout->addLayout(filterButtonsLayout, 1, 0, 1, 2);

    // Добавляем панель фильтров в общий layout (с минимальной высотой)
    mainLayout->addWidget(filterFrame);

    // Создаем область прокрутки для рецептов
    recipesScrollArea = new QScrollArea();
    recipesScrollArea->setWidgetResizable(true);
    recipesScrollArea->setFrameShape(QFrame::NoFrame);
    recipesContainer = new QWidget();
    recipesLayout = new QVBoxLayout(recipesContainer);
    recipesLayout->setContentsMargins(0, 0, 0, 0);
    recipesLayout->setSpacing(10);
    recipesScrollArea->setWidget(recipesContainer);

    // Добавляем область прокрутки для рецептов в главный layout
    mainLayout->addWidget(recipesScrollArea, 1); // Добавляем stretch-фактор 1, чтобы эта область могла растягиваться

    // Подключаем сигналы
    connect(searchButton, &QPushButton::clicked, this, &RecipeExplorerViewImpl::searchRequested);
    connect(applyFiltersButton, &QPushButton::clicked, this, &RecipeExplorerViewImpl::filterApplied);
    connect(resetFiltersButton, &QPushButton::clicked, [this]() {
        maxCookingTimeSpinBox->setValue(60);
    });
    connect(ingredientSearchLineEdit, &QLineEdit::returnPressed, this, &RecipeExplorerViewImpl::addIngredientFromSearch);
    connect(categorySearchLineEdit, &QLineEdit::returnPressed, this, &RecipeExplorerViewImpl::addCategoryFromSearch);
}

void RecipeExplorerViewImpl::displayRecipes(const QList<RecipePreviewDTO>& recipes) {
    clearRecipesList();

    if (recipes.isEmpty()) {
        QLabel* noRecipesLabel = new QLabel("Нет доступных рецептов");
        noRecipesLabel->setAlignment(Qt::AlignCenter);
        recipesLayout->addWidget(noRecipesLabel);
    } else {
        for (const auto& recipe : recipes) {
            recipesLayout->addWidget(createRecipeCard(recipe));
        }
    }

    // Добавляем растягивающийся спейсер в конец, чтобы карточки группировались вверху
    recipesLayout->addStretch();
}

void RecipeExplorerViewImpl::displayCategories(const QList<Category>& categories) {
    categoryNameToId.clear();
    QStringList categoryNames;

    for (const auto& category : categories) {
        categoryNameToId[category.getName()] = category.getCategoryId();
        categoryNames << category.getName();
    }

    // Сначала отключаем старые соединения
    if (categorySearchLineEdit->completer()) {
        disconnect(categorySearchLineEdit->completer(), 0, this, 0);
    }

    // Устанавливаем автодополнение для поиска по категориям
    QCompleter* categoryCompleter = new QCompleter(categoryNames, this);
    categoryCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    categorySearchLineEdit->setCompleter(categoryCompleter);

    // Подключаем сигнал активации комплитера
    connect(categoryCompleter, QOverload<const QString&>::of(&QCompleter::activated),
            this, &RecipeExplorerViewImpl::onCategoryCompleterActivated);
}

// void RecipeExplorerViewImpl::displayIngredients(const QList<Ingredient>& ingredients) {
//     ingredientNameToId.clear();
//     QStringList ingredientNames;

//     for (const auto& ingredient : ingredients) {
//         ingredientNameToId[ingredient.getName()] = ingredient.getIngredientId();
//         ingredientNames << ingredient.getName();
//     }

//     // Устанавливаем автодополнение для поиска по ингредиентам
//     QCompleter* ingredientCompleter = new QCompleter(ingredientNames, this);
//     ingredientCompleter->setCaseSensitivity(Qt::CaseInsensitive);
//     ingredientSearchLineEdit->setCompleter(ingredientCompleter);
// }

void RecipeExplorerViewImpl::showNoRecipesFound() {
    clearRecipesList();

    QLabel* noRecipesLabel = new QLabel("По вашему запросу ничего не найдено");
    noRecipesLabel->setAlignment(Qt::AlignCenter);
    recipesLayout->addWidget(noRecipesLabel);
    recipesLayout->addStretch();
}

void RecipeExplorerViewImpl::displaySearchResults(const QList<RecipePreviewDTO>& recipes) {
    displayRecipes(recipes);
}

QList<int> RecipeExplorerViewImpl::getSelectedCategoryIds() {
    return selectedCategoryTags.keys();
}

int RecipeExplorerViewImpl::getMaxCookingTime() {
    return maxCookingTimeSpinBox->value();
}

void RecipeExplorerViewImpl::showRecipeDetails(int recipeId) {
    emit recipeSelected(recipeId);
}

QWidget* RecipeExplorerViewImpl::createRecipeCard(const RecipePreviewDTO& recipe) {
    // Создаем карточку рецепта (теперь без фотографии)
    RecipeCard* card = new RecipeCard(
        recipe.id,
        recipe.name,
        recipe.totalTime,                    // Общее время
        recipe.totalTime / 2,                // Приблизительно активное время как половина от общего
        0,                                   // Калории не хранятся в DTO
        recipe.mainIngredients,              // Используем mainIngredients как диетические теги
        recipe.isFavorite
        );

    // Подключаем сигналы
    connect(card, &RecipeCard::clicked, this, &RecipeExplorerViewImpl::recipeSelected);
    connect(card, &RecipeCard::favoriteToggled, this, &RecipeExplorerViewImpl::favoritesToggled);

    return card;
}

void RecipeExplorerViewImpl::clearRecipesList() {
    // Удаляем все виджеты из контейнера рецептов
    QLayoutItem* child;
    while ((child = recipesLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
}

// Метод для добавления ингредиента из строки поиска
void RecipeExplorerViewImpl::addIngredientFromSearch() {
    QString ingredientName = ingredientSearchLineEdit->text().trimmed();
    if (ingredientName.isEmpty()) return;

    // Проверяем, существует ли такой ингредиент
    if (ingredientNameToId.contains(ingredientName)) {
        int ingredientId = ingredientNameToId[ingredientName];

        // Проверяем, не выбран ли он уже
        if (!selectedIngredientTags.contains(ingredientId)) {
            // Добавляем тег
            SelectedItemTag* tag = new SelectedItemTag(ingredientId, ingredientName);
            selectedIngredientsLayout->addWidget(tag);
            selectedIngredientTags[ingredientId] = tag;

            // Подключаем сигнал удаления
            connect(tag, &SelectedItemTag::removeRequested, this, &RecipeExplorerViewImpl::removeSelectedIngredient);

            // Очищаем поле поиска
            ingredientSearchLineEdit->clear();

            qDebug() << "Добавлен ингредиент:" << ingredientName << "с ID:" << ingredientId;
        }
    } else if (!ingredientName.isEmpty()) {
        // Пытаемся найти ближайшее соответствие в автодополнении
        QCompleter* completer = ingredientSearchLineEdit->completer();
        if (completer) {
            QString completionPrefix = ingredientName;
            completer->setCompletionPrefix(completionPrefix);

            // Если есть хотя бы одно соответствие, используем его
            if (completer->completionCount() > 0) {
                QString firstCompletion = completer->currentCompletion();
                ingredientSearchLineEdit->setText(firstCompletion);
                addIngredientFromSearch();  // Рекурсивно добавляем найденный ингредиент
            }
        }
    }
}

// Метод для удаления выбранного ингредиента
void RecipeExplorerViewImpl::removeSelectedIngredient(int ingredientId) {
    if (selectedIngredientTags.contains(ingredientId)) {
        SelectedItemTag* tag = selectedIngredientTags[ingredientId];
        selectedIngredientsLayout->removeWidget(tag);
        selectedIngredientTags.remove(ingredientId);
        delete tag;

        qDebug() << "Удален ингредиент с ID:" << ingredientId;
    }
}

// Переопределенный метод для получения выбранных ID ингредиентов
QList<int> RecipeExplorerViewImpl::getSelectedIngredientIds() {
    return selectedIngredientTags.keys();
}

// Метод, вызываемый при выборе элемента из выпадающего списка completer
void RecipeExplorerViewImpl::onIngredientCompleterActivated(const QString& text) {
    // Устанавливаем выбранный текст в поле поиска
    ingredientSearchLineEdit->setText(text);

    // Добавляем ингредиент в выбранные
    addIngredientFromSearch();
}

void RecipeExplorerViewImpl::displayIngredients(const QList<Ingredient>& ingredients) {
    ingredientNameToId.clear();
    QStringList ingredientNames;

    for (const auto& ingredient : ingredients) {
        ingredientNameToId[ingredient.getName()] = ingredient.getIngredientId();
        ingredientNames << ingredient.getName();
    }

    // Сначала отключаем старые соединения
    if (ingredientSearchLineEdit->completer()) {
        disconnect(ingredientSearchLineEdit->completer(), 0, this, 0);
    }

    // Устанавливаем автодополнение для поиска по ингредиентам
    QCompleter* ingredientCompleter = new QCompleter(ingredientNames, this);
    ingredientCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    ingredientSearchLineEdit->setCompleter(ingredientCompleter);

    // Подключаем сигнал активации комплитера
    connect(ingredientCompleter, QOverload<const QString&>::of(&QCompleter::activated),
            this, &RecipeExplorerViewImpl::onIngredientCompleterActivated);
}

// Метод для добавления категории из строки поиска
void RecipeExplorerViewImpl::addCategoryFromSearch() {
    QString categoryName = categorySearchLineEdit->text().trimmed();
    if (categoryName.isEmpty()) return;

    // Проверяем, существует ли такая категория
    if (categoryNameToId.contains(categoryName)) {
        int categoryId = categoryNameToId[categoryName];

        // Проверяем, не выбрана ли она уже
        if (!selectedCategoryTags.contains(categoryId)) {
            // Добавляем тег
            SelectedItemTag* tag = new SelectedItemTag(categoryId, categoryName);
            selectedCategoriesLayout->addWidget(tag);
            selectedCategoryTags[categoryId] = tag;

            // Подключаем сигнал удаления
            connect(tag, &SelectedItemTag::removeRequested, this, &RecipeExplorerViewImpl::removeSelectedCategory);

            // Очищаем поле поиска
            categorySearchLineEdit->clear();

            qDebug() << "Добавлена категория:" << categoryName << "с ID:" << categoryId;
        }
    } else if (!categoryName.isEmpty()) {
        // Пытаемся найти ближайшее соответствие в автодополнении
        QCompleter* completer = categorySearchLineEdit->completer();
        if (completer) {
            QString completionPrefix = categoryName;
            completer->setCompletionPrefix(completionPrefix);

            // Если есть хотя бы одно соответствие, используем его
            if (completer->completionCount() > 0) {
                QString firstCompletion = completer->currentCompletion();
                categorySearchLineEdit->setText(firstCompletion);
                addCategoryFromSearch();  // Рекурсивно добавляем найденную категорию
            }
        }
    }
}

// Метод для удаления выбранной категории
void RecipeExplorerViewImpl::removeSelectedCategory(int categoryId) {
    if (selectedCategoryTags.contains(categoryId)) {
        SelectedItemTag* tag = selectedCategoryTags[categoryId];
        selectedCategoriesLayout->removeWidget(tag);
        selectedCategoryTags.remove(categoryId);
        delete tag;

        qDebug() << "Удалена категория с ID:" << categoryId;
    }
}

// Метод, вызываемый при выборе элемента из выпадающего списка completer
void RecipeExplorerViewImpl::onCategoryCompleterActivated(const QString& text) {
    // Устанавливаем выбранный текст в поле поиска
    categorySearchLineEdit->setText(text);

    // Добавляем категорию в выбранные
    addCategoryFromSearch();
}

#include "recipeExplorerView.moc"
