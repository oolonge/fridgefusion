#ifndef CATEGORY_H
#define CATEGORY_H

#include <QString>

/**
 * @brief Класс, представляющий категорию ингредиентов или рецептов
 */
class Category
{
public:
    /**
     * @brief Конструктор по умолчанию
     */
    Category() : m_categoryId(-1) {}

    /**
     * @brief Конструктор с параметрами
     * @param categoryId Идентификатор категории
     * @param name Название категории
     * @param description Описание категории
     */
    Category(int categoryId, const QString& name, const QString& description)
        : m_categoryId(categoryId)
        , m_name(name)
        , m_description(description)
    {}

    // Геттеры
    int getCategoryId() const { return m_categoryId; }
    QString getName() const { return m_name; }
    QString getDescription() const { return m_description; }

    // Сеттеры
    void setCategoryId(int categoryId) { m_categoryId = categoryId; }
    void setName(const QString& name) { m_name = name; }
    void setDescription(const QString& description) { m_description = description; }

private:
    int m_categoryId;      ///< Идентификатор категории
    QString m_name;        ///< Название категории
    QString m_description; ///< Описание категории
};

#endif // CATEGORY_H
