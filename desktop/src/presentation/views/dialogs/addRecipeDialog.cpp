#include "addRecipeDialog.h"

AddRecipeDialog::AddRecipeDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Создание нового рецепта");
    setMinimumWidth(400);

    // Основной layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Заголовок
    QLabel* titleLabel = new QLabel("Введите название нового рецепта:", this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    // Поле ввода названия
    recipeNameLineEdit = new QLineEdit(this);
    recipeNameLineEdit->setPlaceholderText("Название рецепта...");

    // Лейбл для ошибок
    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("color: red;");
    errorLabel->setVisible(false);

    // Кнопки
    QHBoxLayout* buttonsLayout = new QHBoxLayout();

    cancelButton = new QPushButton("Назад", this);
    createButton = new QPushButton("Создать", this);
    createButton->setEnabled(false);
    createButton->setDefault(true);

    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(createButton);

    // Добавляем все элементы в основной layout
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(recipeNameLineEdit);
    mainLayout->addWidget(errorLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonsLayout);

    // Подключаем сигналы
    connect(recipeNameLineEdit, &QLineEdit::textChanged, this, &AddRecipeDialog::validateInput);
    connect(createButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QString AddRecipeDialog::getRecipeName() const {
    return recipeNameLineEdit->text().trimmed();
}

void AddRecipeDialog::validateInput() {
    QString name = getRecipeName();

    if (name.isEmpty()) {
        createButton->setEnabled(false);
        errorLabel->setText("Название рецепта не может быть пустым");
        errorLabel->setVisible(true);
    } else {
        createButton->setEnabled(true);
        errorLabel->setVisible(false);
    }
}
