#include <QtTest>
#include "application/processors/favoriteProcessor.h"

class MockFavoriteRepository : public IFavoriteRepository {
public:
    QList<Favorite> getFavoritesByUserId(int userId) override {
        QList<Favorite> favorites;
        if (userId == 1) {
            favorites.append(Favorite(1, 1, 101, QDateTime::currentDateTime()));
            favorites.append(Favorite(2, 1, 102, QDateTime::currentDateTime()));
        }
        return favorites;
    }

    bool addFavorite(const Favorite& favorite) override {
        return true;
    }

    bool removeFavorite(int userId, int recipeId) override {
        return (userId == 1 && recipeId == 101);
    }

    bool isFavorite(int userId, int recipeId) override {
        return (userId == 1 && recipeId == 101);
    }
};

class FavoriteProcessorTest : public QObject {
    Q_OBJECT

private slots:
    void testGetUserFavorites();
    void testAddToFavorites();
    void testRemoveFromFavorites();
    void testIsRecipeFavorite();
};

void FavoriteProcessorTest::testGetUserFavorites() {
    MockFavoriteRepository repository;
    FavoriteProcessor processor(&repository);

    QList<Favorite> favorites = processor.getUserFavorites(1);
    QCOMPARE(favorites.size(), 2);
    QCOMPARE(favorites[0].getRecipeId(), 101);
    QCOMPARE(favorites[1].getRecipeId(), 102);

    QList<Favorite> emptyFavorites = processor.getUserFavorites(2);
    QCOMPARE(emptyFavorites.size(), 0);
}

void FavoriteProcessorTest::testAddToFavorites() {
    MockFavoriteRepository repository;
    FavoriteProcessor processor(&repository);

    QVERIFY(processor.addToFavorites(1, 103));
}

void FavoriteProcessorTest::testRemoveFromFavorites() {
    MockFavoriteRepository repository;
    FavoriteProcessor processor(&repository);

    QVERIFY(processor.removeFromFavorites(1, 101));
    QVERIFY(!processor.removeFromFavorites(1, 999));
}

void FavoriteProcessorTest::testIsRecipeFavorite() {
    MockFavoriteRepository repository;
    FavoriteProcessor processor(&repository);

    QVERIFY(processor.isRecipeFavorite(1, 101));
    QVERIFY(!processor.isRecipeFavorite(1, 999));
}

// QTEST_APPLESS_MAIN(FavoriteProcessorTest)
#include "tst_favoriteprocessor.moc"
