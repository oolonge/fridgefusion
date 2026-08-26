#include "adminRecipeViewImpl.h"
#include "dialogs/addRecipeDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QGroupBox>
#include <QSpacerItem>
#include <QLabel>
#include <QDebug>
#include <QCompleter>

class CategoryTag : public QFrame {
    Q_OBJECT

public:
    explicit CategoryTag(int id, const QString& name, QWidget* parent = nullptr)
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

// Реализация Flow Layout для отображения тегов категорий
class FlowLayout : public QLayout {
public:
    explicit FlowLayout(QWidget *parent = nullptr, int margin = 0, int spacing = -1)
        : QLayout(parent) {
        setContentsMargins(margin, margin, margin, margin);
        setSpacing(spacing);
    }

    ~FlowLayout() {
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

AdminRecipeViewImpl::AdminRecipeViewImpl(QWidget* parent)
    : QWidget(parent), selectedRecipeId(-1), blockSignalsInUpdate(false)
{
    setupUi();
}

void AdminRecipeViewImpl::setupUi() {
    // Основной лейаут
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    // Создаем TabWidget
    tabWidget = new QTabWidget();

    // Создаем вкладки
    setupRecipesListTab();
    setupRecipeDetailsTab();

    // Добавляем вкладки в TabWidget
    tabWidget->addTab(recipesListTab, "Список рецептов");
    tabWidget->addTab(recipeDetailsTab, "Детали рецепта");

    // По умолчанию вкладка деталей рецепта недоступна
    tabWidget->setTabEnabled(1, false);

    // Собираем основной лейаут
    mainLayout->addWidget(tabWidget);

    // Подключаем сигналы
    connect(searchButton, &QPushButton::clicked, this, &AdminRecipeViewImpl::searchRecipesRequested);
    connect(searchLineEdit, &QLineEdit::returnPressed, this, &AdminRecipeViewImpl::searchRecipesRequested);
    connect(refreshButton, &QPushButton::clicked, this, &AdminRecipeViewImpl::refreshRecipesRequested);
    connect(backToListButton, &QPushButton::clicked, [this]() {
        tabWidget->setCurrentIndex(0);
    });

    connect(recipesTableWidget, &QTableWidget::cellDoubleClicked, [this](int row, int) {
        QTableWidgetItem* idItem = recipesTableWidget->item(row, 0);
        if (idItem) {
            int recipeId = idItem->text().toInt();
            selectedRecipeId = recipeId;
            emit recipeSelected(recipeId);

            // Переключаемся на вкладку деталей и разблокируем её
            tabWidget->setTabEnabled(1, true);
            tabWidget->setCurrentIndex(1);
        }
    });

    connect(updateRecipeButton, &QPushButton::clicked, [this]() {
        if (selectedRecipeId > 0) {
            emit updateRecipeRequested(selectedRecipeId);
        } else {
            QMessageBox::warning(this, "Предупреждение", "Сначала выберите рецепт");
        }
    });

    connect(deleteRecipeButton, &QPushButton::clicked, [this]() {
        if (selectedRecipeId > 0) {
            // Запрос подтверждения
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Подтверждение удаления",
                "Вы уверены, что хотите удалить этот рецепт? Это действие нельзя отменить.",
                QMessageBox::Yes | QMessageBox::No
                );

            if (reply == QMessageBox::Yes) {
                emit deleteRecipeRequested(selectedRecipeId);
            }
        } else {
            QMessageBox::warning(this, "Предупреждение", "Сначала выберите рецепт");
        }
    });

    connect(addCategoryButton, &QPushButton::clicked, [this]() {
        if (selectedRecipeId <= 0) {
            QMessageBox::warning(this, "Предупреждение", "Сначала выберите рецепт");
            return;
        }

        QString categoryName = categorySearchLineEdit->text().trimmed();
        if (categoryName.isEmpty()) return;

        // Проверяем, существует ли такая категория
        if (categoryNameToId.contains(categoryName)) {
            int categoryId = categoryNameToId[categoryName];
            emit addCategoryToRecipeRequested(selectedRecipeId, categoryId);
            categorySearchLineEdit->clear();
        }
    });

    connect(addNewRecipeButton, &QPushButton::clicked, this, [this]() {
        AddRecipeDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            QString recipeName = dialog.getRecipeName();
            if (!recipeName.isEmpty()) {
                emit createRecipeRequested(recipeName);
            }
        }
    });

    connect(addIngredientButton, &QPushButton::clicked, [this]() {
        if (selectedRecipeId <= 0) {
            QMessageBox::warning(this, "Предупреждение", "Сначала выберите рецепт");
            return;
        }

        QString ingredientName = ingredientSearchLineEdit->text().trimmed();
        if (ingredientName.isEmpty()) return;

        // Проверяем, существует ли такой ингредиент
        if (ingredientNameToId.contains(ingredientName)) {
            int ingredientId = ingredientNameToId[ingredientName];
            double quantity = ingredientQuantitySpinBox->value();
            emit addIngredientToRecipeRequested(selectedRecipeId, ingredientId, quantity);
            ingredientSearchLineEdit->clear();
            ingredientQuantitySpinBox->setValue(1.0);
        }
    });

    // Очищаем детали рецепта
    clearRecipeDetails();
}

void AdminRecipeViewImpl::setupRecipesListTab() {
    recipesListTab = new QWidget();
    QVBoxLayout* recipesListLayout = new QVBoxLayout(recipesListTab);

    // Панель поиска
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLineEdit = new QLineEdit();
    searchLineEdit->setPlaceholderText("Поиск рецептов...");
    searchButton = new QPushButton("Поиск");

    searchLayout->addWidget(searchLineEdit);
    searchLayout->addWidget(searchButton);

    // Таблица рецептов
    recipesTableWidget = new QTableWidget();
    recipesTableWidget->setColumnCount(5);
    QStringList headers = {"ID", "Название", "Время приг.", "Время готовки", "Автор"};
    recipesTableWidget->setHorizontalHeaderLabels(headers);
    recipesTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    recipesTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    recipesTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recipesTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Панель кнопок
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    refreshButton = new QPushButton("Обновить список");
    addNewRecipeButton = new QPushButton("Добавить рецепт");

    buttonsLayout->addWidget(refreshButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(addNewRecipeButton);

    // Собираем вкладку списка рецептов
    recipesListLayout->addLayout(searchLayout);
    recipesListLayout->addWidget(recipesTableWidget);
    recipesListLayout->addLayout(buttonsLayout);
}

void AdminRecipeViewImpl::setupRecipeDetailsTab() {
    recipeDetailsTab = new QWidget();
    QVBoxLayout* recipeDetailsLayout = new QVBoxLayout(recipeDetailsTab);

    // Создаем основной grid layout для двух колонок
    QGridLayout* mainGridLayout = new QGridLayout();

    // Левая колонка - основная информация и категории
    QVBoxLayout* leftColumnLayout = new QVBoxLayout();

    // Основная информация о рецепте
    QGroupBox* basicInfoGroup = new QGroupBox("Основная информация");
    QFormLayout* basicInfoLayout = new QFormLayout(basicInfoGroup);
    
    recipeNameLabel = new QLabel();
    editNameLineEdit = new QLineEdit();
    editDescriptionTextEdit = new QTextEdit();
    editDescriptionTextEdit->setMaximumHeight(100);
    editPrepTimeSpinBox = new QSpinBox();
    editPrepTimeSpinBox->setRange(1, 999);
    editPrepTimeSpinBox->setSuffix(" мин");
    editCookTimeSpinBox = new QSpinBox();
    editCookTimeSpinBox->setRange(1, 999);
    editCookTimeSpinBox->setSuffix(" мин");
    
    basicInfoLayout->addRow("Название:", editNameLineEdit);
    basicInfoLayout->addRow("Описание:", editDescriptionTextEdit);
    basicInfoLayout->addRow("Время подготовки:", editPrepTimeSpinBox);
    basicInfoLayout->addRow("Время приготовления:", editCookTimeSpinBox);
    
    leftColumnLayout->addWidget(basicInfoGroup);

    QGroupBox* categoriesGroup = new QGroupBox("Категории");
    QVBoxLayout* categoriesGroupLayout = new QVBoxLayout(categoriesGroup);
    
    // Строка поиска и добавления категорий
    QHBoxLayout* categorySearchLayout = new QHBoxLayout();
    categorySearchLineEdit = new QLineEdit();
    categorySearchLineEdit->setPlaceholderText("Поиск категории...");
    addCategoryButton = new QPushButton("Добавить");
    
    categorySearchLayout->addWidget(categorySearchLineEdit);
    categorySearchLayout->addWidget(addCategoryButton);
    
    categoriesGroupLayout->addLayout(categorySearchLayout);
    
    // Контейнер для отображения выбранных категорий
    categoriesScrollArea = new QScrollArea();
    categoriesScrollArea->setWidgetResizable(true);
    categoriesScrollArea->setMaximumHeight(150);
    categoriesScrollArea->setFrameShape(QFrame::StyledPanel);
    
    categoriesContainer = new QWidget();
    categoriesLayout = new FlowLayout(categoriesContainer, 5, 5);
    
    categoriesScrollArea->setWidget(categoriesContainer);
    categoriesGroupLayout->addWidget(categoriesScrollArea);
    
    leftColumnLayout->addWidget(categoriesGroup);
    
    // Шаги приготовления
    QGroupBox* instructionsGroup = new QGroupBox("Шаги приготовления");
    QVBoxLayout* instructionsLayout = new QVBoxLayout(instructionsGroup);
    
    editInstructionsTextEdit = new QTextEdit();
    
    instructionsLayout->addWidget(editInstructionsTextEdit);
    
    leftColumnLayout->addWidget(instructionsGroup);
    leftColumnLayout->addStretch();

    // Правая колонка - ингредиенты
    QVBoxLayout* rightColumnLayout = new QVBoxLayout();

    // Секция ингредиентов
    QGroupBox* ingredientsGroup = new QGroupBox("Ингредиенты");
    QVBoxLayout* ingredientsGroupLayout = new QVBoxLayout(ingredientsGroup);

    // Строка поиска и добавления ингредиентов
    QHBoxLayout* ingredientSearchLayout = new QHBoxLayout();
    ingredientSearchLineEdit = new QLineEdit();
    ingredientSearchLineEdit->setPlaceholderText("Поиск ингредиента...");
    
    QLabel* quantityLabel = new QLabel("Количество:");
    ingredientQuantitySpinBox = new QDoubleSpinBox();
    ingredientQuantitySpinBox->setRange(0.01, 9999.99);
    ingredientQuantitySpinBox->setValue(1.0);
    ingredientQuantitySpinBox->setDecimals(2);
    ingredientQuantitySpinBox->setSingleStep(0.5);
    
    ingredientUnitComboBox = new QComboBox();
    ingredientUnitComboBox->addItem("г", "г");
    ingredientUnitComboBox->addItem("мл", "мл");
    ingredientUnitComboBox->addItem("шт", "шт");
    ingredientUnitComboBox->addItem("ст.л.", "ст.л.");
    ingredientUnitComboBox->addItem("ч.л.", "ч.л.");
    ingredientUnitComboBox->addItem("стак.", "стак.");
    ingredientUnitComboBox->addItem("щеп.", "щеп.");
    ingredientUnitComboBox->addItem("по вкусу", "по вкусу");
    
    addIngredientButton = new QPushButton("Добавить");
    
    ingredientSearchLayout->addWidget(ingredientSearchLineEdit);
    ingredientSearchLayout->addWidget(quantityLabel);
    ingredientSearchLayout->addWidget(ingredientQuantitySpinBox);
    ingredientSearchLayout->addWidget(ingredientUnitComboBox);
    ingredientSearchLayout->addWidget(addIngredientButton);
    
    ingredientsGroupLayout->addLayout(ingredientSearchLayout);
    
    // Таблица ингредиентов
    ingredientsTableWidget = new QTableWidget();
    ingredientsTableWidget->setColumnCount(4);
    QStringList ingredientHeaders = {"Ингредиент", "Количество", "Единица", "Действия"};
    ingredientsTableWidget->setHorizontalHeaderLabels(ingredientHeaders);
    ingredientsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ingredientsTableWidget->setEditTriggers(QAbstractItemView::DoubleClicked);

    // Новые настройки для растягивания столбцов
    ingredientsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // Для последней колонки с кнопками можно установить фиксированную ширину
    ingredientsTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    ingredientsTableWidget->setColumnWidth(3, 100); // Примерная ширина для колонки с кнопками

    // Установка политики размера для виджета, чтобы он занимал максимум доступного пространства
    ingredientsTableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Установка минимальной высоты для таблицы
    ingredientsTableWidget->setMinimumHeight(355);
    
    ingredientsGroupLayout->addWidget(ingredientsTableWidget);
    
    rightColumnLayout->addWidget(ingredientsGroup);
    rightColumnLayout->addStretch();

    // Добавляем колонки в основной grid layout
    mainGridLayout->addLayout(leftColumnLayout, 0, 0);
    mainGridLayout->addLayout(rightColumnLayout, 0, 1);
    mainGridLayout->setColumnStretch(0, 1);
    mainGridLayout->setColumnStretch(1, 1);

    // Кнопки действий внизу
    QHBoxLayout* actionButtonsLayout = new QHBoxLayout();
    
    // Кнопка возврата к списку
    backToListButton = new QPushButton("Вернуться к списку");
    
    // Кнопки обновления и удаления
    updateRecipeButton = new QPushButton("Сохранить изменения");
    // updateRecipeButton->setStyleSheet("background-color: #4CAF50; color: white;");
    
    deleteRecipeButton = new QPushButton("Удалить рецепт");
    // deleteRecipeButton->setStyleSheet("background-color: #f44336; color: white;");
    
    actionButtonsLayout->addWidget(backToListButton);
    actionButtonsLayout->addStretch();
    actionButtonsLayout->addWidget(updateRecipeButton);
    actionButtonsLayout->addWidget(deleteRecipeButton);

    // Собираем все на вкладке
    recipeDetailsLayout->addLayout(mainGridLayout);
    recipeDetailsLayout->addLayout(actionButtonsLayout);
}

// Изменить реализацию метода на:
void AdminRecipeViewImpl::displayRecipes(const QList<RecipePreviewDTO>& recipes) {
    recipesTableWidget->setRowCount(0); // Очищаем таблицу

    for (int i = 0; i < recipes.size(); ++i) {
        const RecipePreviewDTO& recipe = recipes[i];

        recipesTableWidget->insertRow(i);

        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(recipe.id));
        QTableWidgetItem* nameItem = new QTableWidgetItem(recipe.name);
        QTableWidgetItem* prepTimeItem = new QTableWidgetItem(QString("%1 мин").arg(recipe.totalTime / 2)); // Приблизительно
        QTableWidgetItem* cookTimeItem = new QTableWidgetItem(QString("%1 мин").arg(recipe.totalTime / 2)); // Приблизительно

        // Отображаем имя автора рецепта
        QTableWidgetItem* authorItem = new QTableWidgetItem(recipe.authorName.isEmpty() ? "—" : recipe.authorName);

        recipesTableWidget->setItem(i, 0, idItem);
        recipesTableWidget->setItem(i, 1, nameItem);
        recipesTableWidget->setItem(i, 2, prepTimeItem);
        recipesTableWidget->setItem(i, 3, cookTimeItem);
        recipesTableWidget->setItem(i, 4, authorItem);
    }

    // Если нет рецептов, показываем сообщение
    if (recipes.isEmpty()) {
        recipesTableWidget->setRowCount(1);
        QTableWidgetItem* noDataItem = new QTableWidgetItem("Нет данных для отображения");
        noDataItem->setTextAlignment(Qt::AlignCenter);
        recipesTableWidget->setSpan(0, 0, 1, 5);
        recipesTableWidget->setItem(0, 0, noDataItem);
    }
}

void AdminRecipeViewImpl::displayRecipeDetails(const RecipeDetailDTO& recipeDetail) {
    // Устанавливаем флаг, чтобы игнорировать сигналы во время обновления таблицы
    blockSignalsInUpdate = true;
    
    // Отключаем сигналы от таблицы на время обновления
    ingredientsTableWidget->disconnect();
    
    selectedRecipeId = recipeDetail.id;

    // Заполняем информацию о рецепте
    recipeNameLabel->setText(recipeDetail.name);
    editNameLineEdit->setText(recipeDetail.name);
    editDescriptionTextEdit->setText(recipeDetail.fullDescription);
    editPrepTimeSpinBox->setValue(recipeDetail.preparationTime);
    editCookTimeSpinBox->setValue(recipeDetail.cookingTime);
    
    // Собираем шаги приготовления в одну строку
    QString instructionsText = recipeDetail.steps.join("\n\n");
    editInstructionsTextEdit->setText(instructionsText);
    
    // Очищаем предыдущие категории
    QLayoutItem* child;
    while ((child = categoriesLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
    
    // Добавляем категории
    for (const QString& category : recipeDetail.categories) {
        if (categoryNameToId.contains(category)) {
            int categoryId = categoryNameToId[category];
            CategoryTag* tag = new CategoryTag(categoryId, category);
            categoriesLayout->addWidget(tag);
            
            connect(tag, &CategoryTag::removeRequested, [this, categoryId](int id) {
                emit removeCategoryFromRecipeRequested(selectedRecipeId, id);
            });
        }
    }
    
    // Очищаем таблицу ингредиентов
    ingredientsTableWidget->setRowCount(0);
    
    // Заполняем таблицу ингредиентов
    for (int i = 0; i < recipeDetail.ingredients.size(); ++i) {
        const auto& ingredient = recipeDetail.ingredients[i];
        
        ingredientsTableWidget->insertRow(i);
        
        QTableWidgetItem* nameItem = new QTableWidgetItem(ingredient.name);
        nameItem->setData(Qt::UserRole, ingredient.id); // Сохраняем ID ингредиента
        QTableWidgetItem* quantityItem = new QTableWidgetItem(QString::number(ingredient.quantity));
        QTableWidgetItem* unitItem = new QTableWidgetItem(ingredient.unit);
        
        // Создаем кнопку удаления
        QPushButton* removeButton = new QPushButton("Удалить");
        removeButton->setProperty("ingredient_id", ingredient.id);
        
        connect(removeButton, &QPushButton::clicked, [this, ingredient]() {
            emit removeIngredientFromRecipeRequested(selectedRecipeId, ingredient.id);
        });
        
        ingredientsTableWidget->setItem(i, 0, nameItem);
        ingredientsTableWidget->setItem(i, 1, quantityItem);
        ingredientsTableWidget->setItem(i, 2, unitItem);
        ingredientsTableWidget->setCellWidget(i, 3, removeButton);
    }
    
    // Создаем копию ингредиентов для использования в лямбда-функции
    QList<RecipeDetailDTO::IngredientInfo> ingredientsCopy = recipeDetail.ingredients;
    
    // Подключаем изменение количества ингредиента при редактировании ячейки
    connect(ingredientsTableWidget, &QTableWidget::cellChanged, [this, ingredientsCopy](int row, int column) {
        // Игнорируем изменения, если происходит массовое обновление данных
        if (blockSignalsInUpdate) return;
        
        if (column == 1) { // Колонка с количеством
            QTableWidgetItem* nameItem = ingredientsTableWidget->item(row, 0);
            QTableWidgetItem* quantityItem = ingredientsTableWidget->item(row, 1);
            
            if (nameItem && quantityItem) {
                int ingredientId = nameItem->data(Qt::UserRole).toInt();
                bool ok;
                double quantity = quantityItem->text().toDouble(&ok);
                
                if (ok && quantity > 0) {
                    emit updateIngredientQuantityRequested(selectedRecipeId, ingredientId, quantity);
                } else {
                    // Восстанавливаем предыдущее значение при ошибке
                    QMessageBox::warning(this, "Ошибка", "Введите корректное числовое значение больше 0");
                    // Обновляем ячейку с правильным значением
                    for (const auto& ingredient : ingredientsCopy) {
                        if (ingredient.id == ingredientId) {
                            blockSignalsInUpdate = true;
                            quantityItem->setText(QString::number(ingredient.quantity));
                            blockSignalsInUpdate = false;
                            break;
                        }
                    }
                }
            }
        }
    });
    
    // Переключаемся на вкладку деталей и разблокируем её
    tabWidget->setTabEnabled(1, true);
    tabWidget->setCurrentIndex(1);
    
    // Сбрасываем флаг, чтобы разрешить обработку сигналов
    blockSignalsInUpdate = false;
}

void AdminRecipeViewImpl::displayCategories(const QList<Category>& categories) {
    categoryNameToId.clear();
    QStringList categoryNames;

    for (const auto& category : categories) {
        categoryNameToId[category.getName()] = category.getCategoryId();
        categoryNames << category.getName();
    }

    // Настраиваем автодополнение для поиска категорий
    QCompleter* categoryCompleter = new QCompleter(categoryNames, this);
    categoryCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    categorySearchLineEdit->setCompleter(categoryCompleter);
    
    // Подключаем выбор элемента из выпадающего списка
    connect(categoryCompleter, QOverload<const QString&>::of(&QCompleter::activated), 
            [this](const QString& text) {
        categorySearchLineEdit->setText(text);
        if (selectedRecipeId > 0 && categoryNameToId.contains(text)) {
            emit addCategoryToRecipeRequested(selectedRecipeId, categoryNameToId[text]);
            categorySearchLineEdit->clear();
        }
    });
}

void AdminRecipeViewImpl::displayIngredients(const QList<Ingredient>& ingredients) {
    ingredientNameToId.clear();
    QStringList ingredientNames;

    for (const auto& ingredient : ingredients) {
        ingredientNameToId[ingredient.getName()] = ingredient.getIngredientId();
        ingredientNames << ingredient.getName();
    }

    // Настраиваем автодополнение для поиска ингредиентов
    QCompleter* ingredientCompleter = new QCompleter(ingredientNames, this);
    ingredientCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    ingredientSearchLineEdit->setCompleter(ingredientCompleter);
    
    // Подключаем выбор элемента из выпадающего списка
    connect(ingredientCompleter, QOverload<const QString&>::of(&QCompleter::activated), 
            [this](const QString& text) {
        ingredientSearchLineEdit->setText(text);
    });
}

void AdminRecipeViewImpl::showRecipeDeletedStatus(bool success, const QString& message) {
    if (success) {
        QMessageBox::information(this, "Удаление рецепта", message);
        // Возвращаемся к списку рецептов
        tabWidget->setCurrentIndex(0);
        tabWidget->setTabEnabled(1, false); // Блокируем вкладку деталей
        clearRecipeDetails();
        selectedRecipeId = -1;

        // Обновляем список рецептов
        emit refreshRecipesRequested();
    } else {
        QMessageBox::warning(this, "Ошибка удаления", message);
    }
}

void AdminRecipeViewImpl::showRecipeUpdatedStatus(bool success, const QString& message) {
    if (success) {
        QMessageBox::information(this, "Обновление рецепта", message);
        // Обновляем информацию о рецепте
        emit recipeSelected(selectedRecipeId);
    } else {
        QMessageBox::warning(this, "Ошибка обновления", message);
    }
}

void AdminRecipeViewImpl::showError(const QString& errorMessage) {
    QMessageBox::critical(this, "Ошибка", errorMessage);
}

void AdminRecipeViewImpl::showMessage(const QString& message) {
    QMessageBox::information(this, "Информация", message);
}

void AdminRecipeViewImpl::switchToMain() {
    tabWidget->setCurrentIndex(0);
    emit refreshRecipesRequested();
}

int AdminRecipeViewImpl::getSelectedRecipeId() const {
    return selectedRecipeId;
}

QString AdminRecipeViewImpl::getSearchQuery() const {
    return searchLineEdit->text();
}

RecipeFormData AdminRecipeViewImpl::getRecipeFormData() const {
    RecipeFormData formData;
    formData.name = editNameLineEdit->text();
    formData.description = editDescriptionTextEdit->toPlainText();
    formData.preparationTime = editPrepTimeSpinBox->value();
    formData.cookingTime = editCookTimeSpinBox->value();
    formData.instructions = editInstructionsTextEdit->toPlainText();
    return formData;
}

QString AdminRecipeViewImpl::getSelectedUnit() const {
    return ingredientUnitComboBox->currentData().toString();
}

void AdminRecipeViewImpl::clearRecipeDetails() {
    recipeNameLabel->clear();
    editNameLineEdit->clear();
    editDescriptionTextEdit->clear();
    editPrepTimeSpinBox->setValue(10);
    editCookTimeSpinBox->setValue(30);
    editInstructionsTextEdit->clear();
    
    // Очищаем категории
    QLayoutItem* child;
    while ((child = categoriesLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
    
    // Очищаем таблицу ингредиентов
    ingredientsTableWidget->setRowCount(0);
}

// Включаем сгенерированные MOC файлы
#include "adminRecipeViewImpl.moc"
