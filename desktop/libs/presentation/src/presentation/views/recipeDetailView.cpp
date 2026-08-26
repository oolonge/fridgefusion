#include "recipeDetailView.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QGridLayout>
#include <QTableWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QDateTime>
#include <QHeaderView>

class RecipeDetailViewImpl : public QWidget, public RecipeDetailView {
    Q_OBJECT

public:
    explicit RecipeDetailViewImpl(QWidget* parent = nullptr);
    ~RecipeDetailViewImpl() override = default;

    // Реализация интерфейса RecipeDetailView
    void displayRecipeDetails(const RecipeDetailDTO& recipeDetail) override;
    void displayReviews(const QList<Review>& reviews, double averageRating) override;
    void updateFavoriteStatus(bool isFavorite) override;
    int getRating() override;
    QString getReviewText() override;
    void goBack() override;
    void setReturnToProfile(bool value) override;
    bool returnToProfile = false;

signals:
    void backRequested();
    void favoriteToggled(int recipeId);
    void reviewSubmitted(int recipeId);

private:
    // UI элементы
    QPushButton* backButton;
    QLabel* recipeNameLabel;
    QLabel* recipeImageLabel;
    QLabel* preparationTimeLabel;
    QLabel* cookingTimeLabel;
    QLabel* categoriesLabel;
    QTableWidget* ingredientsTableWidget;
    QTextEdit* instructionsTextEdit;
    QPushButton* favoriteButton;
    QComboBox* ratingComboBox;
    QLineEdit* reviewTextLineEdit;
    QPushButton* submitReviewButton;
    QScrollArea* reviewsScrollArea;
    QWidget* reviewsContainer;
    QVBoxLayout* reviewsLayout;

    // Данные
    int currentRecipeId;
    bool isCurrentlyFavorite;

    void setupUi();
    void clearReviewsList();
    QWidget* createReviewWidget(const Review& review);
};

RecipeDetailViewImpl::RecipeDetailViewImpl(QWidget* parent)
    : QWidget(parent), currentRecipeId(-1), isCurrentlyFavorite(false)
{
    setupUi();
}

void RecipeDetailViewImpl::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Верхняя панель с кнопкой назад, заголовком и кнопкой "В избранное"
    QHBoxLayout* topBarLayout = new QHBoxLayout();

    // Кнопка "Назад"
    backButton = new QPushButton("< Назад");

    // Название рецепта
    recipeNameLabel = new QLabel("Название рецепта");
    QFont recipeTitleFont = recipeNameLabel->font();
    recipeTitleFont.setPointSize(18);
    recipeTitleFont.setBold(true);
    recipeNameLabel->setFont(recipeTitleFont);

    // Кнопка "В избранное" - теперь в верхнем баре справа
    favoriteButton = new QPushButton("В избранное");

    // Добавляем элементы в верхний бар
    topBarLayout->addWidget(backButton);
    topBarLayout->addWidget(recipeNameLabel, 1); // Название рецепта занимает всё доступное пространство
    topBarLayout->addWidget(favoriteButton);     // Кнопка справа

    mainLayout->addLayout(topBarLayout);

    // Верхняя информационная секция (время подготовки, приготовления, категории)
    QFrame* infoFrame = new QFrame();
    infoFrame->setFrameShape(QFrame::StyledPanel);
    QGridLayout* infoLayout = new QGridLayout(infoFrame);
    infoLayout->setColumnStretch(0, 0); // Метка - не растягивается
    infoLayout->setColumnStretch(1, 1); // Значение - растягивается

    // Основная информация
    QLabel* prepTimeTitleLabel = new QLabel("Время подготовки:");
    preparationTimeLabel = new QLabel("—");

    QLabel* cookTimeTitleLabel = new QLabel("Время приготовления:");
    cookingTimeLabel = new QLabel("—");

    QLabel* categoriesTitleLabel = new QLabel("Категории:");
    categoriesLabel = new QLabel("—");

    infoLayout->addWidget(prepTimeTitleLabel, 0, 0);
    infoLayout->addWidget(preparationTimeLabel, 0, 1);
    infoLayout->addWidget(cookTimeTitleLabel, 1, 0);
    infoLayout->addWidget(cookingTimeLabel, 1, 1);
    infoLayout->addWidget(categoriesTitleLabel, 2, 0);
    infoLayout->addWidget(categoriesLabel, 2, 1);

    mainLayout->addWidget(infoFrame);

    // Создаем шрифт для заголовков секций
    QFont sectionFont;
    sectionFont.setPointSize(14);
    sectionFont.setBold(true);

    // Создаем горизонтальный layout для ингредиентов и инструкций
    QHBoxLayout* contentLayout = new QHBoxLayout();

    // === ЛЕВАЯ КОЛОНКА: ИНГРЕДИЕНТЫ ===
    QVBoxLayout* ingredientsColumn = new QVBoxLayout();

    // Заголовок секции ингредиентов
    QLabel* ingredientsTitle = new QLabel("Ингредиенты:");
    ingredientsTitle->setFont(sectionFont);
    ingredientsColumn->addWidget(ingredientsTitle);

    // Таблица ингредиентов
    ingredientsTableWidget = new QTableWidget();
    ingredientsTableWidget->setColumnCount(3);
    ingredientsTableWidget->setHorizontalHeaderLabels(QStringList({"Название", "Количество", "Ед. изм."}));
    ingredientsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ingredientsTableWidget->horizontalHeader()->setStretchLastSection(true);
    ingredientsTableWidget->verticalHeader()->setVisible(true);
    ingredientsTableWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    ingredientsColumn->addWidget(ingredientsTableWidget);

    // Добавляем колонку ингредиентов в общий layout
    contentLayout->addLayout(ingredientsColumn);

    // === ПРАВАЯ КОЛОНКА: ИНСТРУКЦИИ ===
    QVBoxLayout* instructionsColumn = new QVBoxLayout();

    // Заголовок секции инструкций
    QLabel* instructionsTitle = new QLabel("Инструкции по приготовлению:");
    instructionsTitle->setFont(sectionFont);
    instructionsColumn->addWidget(instructionsTitle);

    // Текстовое поле инструкций
    instructionsTextEdit = new QTextEdit();
    instructionsTextEdit->setReadOnly(true);
    instructionsTextEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    instructionsColumn->addWidget(instructionsTextEdit);

    // Добавляем колонку инструкций в общий layout
    contentLayout->addLayout(instructionsColumn);

    // Добавляем layout с ингредиентами и инструкциями в основной layout
    mainLayout->addLayout(contentLayout);

    // === СЕКЦИЯ ОТЗЫВОВ ===
    QLabel* reviewsTitle = new QLabel("Отзывы:");
    reviewsTitle->setFont(sectionFont);
    mainLayout->addWidget(reviewsTitle);

    // Форма добавления отзыва
    QHBoxLayout* addReviewLayout = new QHBoxLayout();

    QLabel* ratingLabel = new QLabel("Оценка:");
    ratingComboBox = new QComboBox();
    for (int i = 1; i <= 5; i++) {
        ratingComboBox->addItem(QString("%1 ★").arg(i), i);
    }

    reviewTextLineEdit = new QLineEdit();
    reviewTextLineEdit->setPlaceholderText("Оставьте свой отзыв...");
    submitReviewButton = new QPushButton("Отправить");

    addReviewLayout->addWidget(ratingLabel);
    addReviewLayout->addWidget(ratingComboBox);
    addReviewLayout->addWidget(reviewTextLineEdit, 1);
    addReviewLayout->addWidget(submitReviewButton);
    mainLayout->addLayout(addReviewLayout);

    // Список отзывов с прокруткой
    reviewsScrollArea = new QScrollArea();
    reviewsScrollArea->setWidgetResizable(true);
    reviewsScrollArea->setMinimumHeight(200);
    reviewsScrollArea->setFrameShape(QFrame::StyledPanel);
    reviewsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Контейнер для отзывов
    reviewsContainer = new QWidget();
    reviewsLayout = new QVBoxLayout(reviewsContainer);
    reviewsLayout->setContentsMargins(5, 5, 5, 5);
    reviewsLayout->setSpacing(10);
    reviewsScrollArea->setWidget(reviewsContainer);

    // Добавляем область прокрутки в основной layout с stretch-фактором
    mainLayout->addWidget(reviewsScrollArea, 1);

    // Подключение сигналов
    connect(backButton, &QPushButton::clicked, this, &RecipeDetailViewImpl::backRequested);
    connect(favoriteButton, &QPushButton::clicked, [this]() {
        emit favoriteToggled(currentRecipeId);
    });
    connect(submitReviewButton, &QPushButton::clicked, [this]() {
        emit reviewSubmitted(currentRecipeId);
    });
}

void RecipeDetailViewImpl::displayRecipeDetails(const RecipeDetailDTO& recipeDetail) {
    currentRecipeId = recipeDetail.id;
    isCurrentlyFavorite = recipeDetail.isFavorite;

    // Обновляем основные данные рецепта
    recipeNameLabel->setText(recipeDetail.name);
    preparationTimeLabel->setText(QString("%1 мин").arg(recipeDetail.preparationTime));
    cookingTimeLabel->setText(QString("%1 мин").arg(recipeDetail.cookingTime));
    categoriesLabel->setText(recipeDetail.categories.join(", "));

    // Обновляем кнопку избранного
    updateFavoriteStatus(recipeDetail.isFavorite);

    // Обновляем таблицу ингредиентов
    ingredientsTableWidget->setRowCount(recipeDetail.ingredients.size());
    for (int i = 0; i < recipeDetail.ingredients.size(); ++i) {
        const auto& ingredient = recipeDetail.ingredients[i];

        // Добавляем номер строки в вертикальный заголовок
        ingredientsTableWidget->setVerticalHeaderItem(i, new QTableWidgetItem(QString::number(i + 1)));

        // Заполняем ячейки таблицы
        ingredientsTableWidget->setItem(i, 0, new QTableWidgetItem(ingredient.name));
        ingredientsTableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(ingredient.quantity)));
        ingredientsTableWidget->setItem(i, 2, new QTableWidgetItem(ingredient.unit));
    }

    // Обновляем инструкции по приготовлению
    QString instructionsText;
    for (int i = 0; i < recipeDetail.steps.size(); ++i) {
        // Заменяем символы \n на реальные переносы строк
        QString step = recipeDetail.steps[i];
        step = step.replace("\\n", "\n");
        instructionsText += QString("%1. %2\n\n").arg(i + 1).arg(step);
    }
    instructionsTextEdit->setText(instructionsText);
}

void RecipeDetailViewImpl::displayReviews(const QList<Review>& reviews, double averageRating) {
    clearReviewsList();

    // Добавляем заголовок с информацией о среднем рейтинге
    QLabel* ratingInfoLabel = new QLabel(QString("Средний рейтинг: %1 ★ (всего отзывов: %2)")
                                             .arg(averageRating, 0, 'f', 1)
                                             .arg(reviews.size()));
    ratingInfoLabel->setAlignment(Qt::AlignCenter);
    QFont ratingFont = ratingInfoLabel->font();
    ratingFont.setBold(true);
    ratingInfoLabel->setFont(ratingFont);
    reviewsLayout->addWidget(ratingInfoLabel);

    if (reviews.isEmpty()) {
        QLabel* noReviewsLabel = new QLabel("Нет отзывов. Будьте первым, кто оставит отзыв!");
        noReviewsLabel->setAlignment(Qt::AlignCenter);
        reviewsLayout->addWidget(noReviewsLabel);
    } else {
        for (const auto& review : reviews) {
            reviewsLayout->addWidget(createReviewWidget(review));
        }
    }

    // Добавляем растягивающийся спейсер в конец
    reviewsLayout->addStretch();
}

void RecipeDetailViewImpl::updateFavoriteStatus(bool isFavorite) {
    if (isFavorite) {
        favoriteButton->setText("★ В избранном");
        favoriteButton->setStyleSheet("background-color: gold; color: black; font-weight: bold;");
    } else {
        favoriteButton->setText("☆ В избранное");
        favoriteButton->setStyleSheet("");
    }
}

int RecipeDetailViewImpl::getRating() {
    return ratingComboBox->currentData().toInt();
}

QString RecipeDetailViewImpl::getReviewText() {
    return reviewTextLineEdit->text();
}

void RecipeDetailViewImpl::goBack() {
    emit backRequested();
}

void RecipeDetailViewImpl::clearReviewsList() {
    // Удаляем все виджеты из контейнера отзывов
    QLayoutItem* child;
    while ((child = reviewsLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
}

QWidget* RecipeDetailViewImpl::createReviewWidget(const Review& review) {
    QFrame* reviewFrame = new QFrame();
    reviewFrame->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout* reviewLayout = new QVBoxLayout(reviewFrame);

    // Верхний ряд: имя пользователя, рейтинг и дата
    QHBoxLayout* headerLayout = new QHBoxLayout();

    // Имя пользователя
    QString username = review.getUsername();
    if (username.isEmpty()) {
        username = QString("Пользователь #%1").arg(review.getUserId());
    }
    QLabel* userLabel = new QLabel(username);
    QFont userFont = userLabel->font();
    userFont.setBold(true);
    userLabel->setFont(userFont);

    // Рейтинг в виде звезд
    QString stars;
    for (int i = 0; i < review.getRating(); ++i) {
        stars += "★";
    }
    QLabel* ratingLabel = new QLabel(stars);
    ratingLabel->setStyleSheet("color: gold;");

    // Дата отзыва
    QLabel* dateLabel = new QLabel(review.getDatePosted().toString("dd.MM.yyyy"));

    headerLayout->addWidget(userLabel);
    headerLayout->addWidget(ratingLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(dateLabel);

    // Текст отзыва
    QLabel* commentLabel = new QLabel(review.getComment());
    commentLabel->setWordWrap(true);

    reviewLayout->addLayout(headerLayout);
    reviewLayout->addWidget(commentLabel);

    return reviewFrame;
}

void RecipeDetailViewImpl::setReturnToProfile(bool value) {
    returnToProfile = value;
}

#include "recipeDetailView.moc"
