#ifndef RECIPEFILTERDTO_H
#define RECIPEFILTERDTO_H

#include <QList>
#include <QString>

class RecipeFilterDTO {
public:
    RecipeFilterDTO() = default;

    // Setters
    void setCategoryIds(const QList<int>& categoryIds) { this->categoryIds = categoryIds; }
    void setMaxCookingTime(int maxCookingTime) { this->maxCookingTime = maxCookingTime; }
    void setMaxPrepTime(int maxPrepTime) { this->maxPrepTime = maxPrepTime; }

    // Getters
    QList<int> getCategoryIds() const { return categoryIds; }
    int getMaxCookingTime() const { return maxCookingTime; }
    int getMaxPrepTime() const { return maxPrepTime; }

private:
    QList<int> categoryIds;
    int maxCookingTime = -1;
    int maxPrepTime = -1;
};

#endif // RECIPEFILTERDTO_H
