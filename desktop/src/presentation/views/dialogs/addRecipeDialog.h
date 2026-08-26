#ifndef ADDRECIPEDIALOG_H
#define ADDRECIPEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class AddRecipeDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddRecipeDialog(QWidget* parent = nullptr);
    QString getRecipeName() const;

private slots:
    void validateInput();

private:
    QLineEdit* recipeNameLineEdit;
    QPushButton* createButton;
    QPushButton* cancelButton;
    QLabel* errorLabel;
};

#endif // ADDRECIPEDIALOG_H
