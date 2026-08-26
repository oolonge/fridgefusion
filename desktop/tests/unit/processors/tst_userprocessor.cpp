#include <QtTest>
#include "application/processors/userProcessor.h"
#include "utils/logging/ConsoleLogger.h"
#include <memory>

namespace UserTest {

class MockUserRepository : public IUserRepository {
public:
    std::optional<User> getUserById(int userId) override {
        if (userId == 1) {
            return User(1, "testuser", "test@example.com", "hashed_password", QDateTime::currentDateTime(), UserRole::USER);
        }
        return std::nullopt;
    }

    std::optional<User> getUserByEmail(const QString& email) override {
        if (email == "test@example.com") {
            return User(1, "testuser", "test@example.com", "hashed_password", QDateTime::currentDateTime(), UserRole::USER);
        } else if (email == "another@example.com") {
            return User(2, "anotheruser", "another@example.com", "another_hash", QDateTime::currentDateTime(), UserRole::USER);
        }
        return std::nullopt;
    }

    QList<User> getUsersByUsername(const QString& username) override {
        QList<User> users;
        if (username.contains("test")) {
            users.append(User(1, "testuser", "test@example.com", "hashed_password", QDateTime::currentDateTime(), UserRole::USER));
        }
        if (username.contains("another")) {
            users.append(User(2, "anotheruser", "another@example.com", "another_hash", QDateTime::currentDateTime(), UserRole::USER));
        }
        return users;
    }

    QList<User> getUsersByRole(UserRole role) override {
        QList<User> users;
        if (role == UserRole::ADMIN) {
            users.append(User(3, "admin", "admin@example.com", "admin_hash", QDateTime::currentDateTime(), UserRole::ADMIN));
        } else if (role == UserRole::USER) {
            users.append(User(1, "testuser", "test@example.com", "hashed_password", QDateTime::currentDateTime(), UserRole::USER));
            users.append(User(2, "anotheruser", "another@example.com", "another_hash", QDateTime::currentDateTime(), UserRole::USER));
        }
        return users;
    }

    QList<User> getAllUsers(int limit, int offset) override {
        QList<User> users;
        users.append(User(1, "testuser", "test@example.com", "hashed_password", QDateTime::currentDateTime(), UserRole::USER));
        users.append(User(2, "anotheruser", "another@example.com", "another_hash", QDateTime::currentDateTime(), UserRole::USER));
        users.append(User(3, "admin", "admin@example.com", "admin_hash", QDateTime::currentDateTime(), UserRole::ADMIN));
        return users;
    }

    bool updateUserRole(int userId, UserRole role) override {
        return userId > 0 && userId <= 3;
    }

    bool saveUser(User& user) override {
        // For testing purposes, assign an ID if it's a new user
        if (user.getId() == 0) {
            user.setId(3);
        }
        return true;
    }

    bool updateUser(const User& user) override {
        // In a real implementation, we would update the user in the database
        // For testing, just return true for valid user IDs
        return user.getId() > 0;
    }

    bool deleteUser(int userId) override {
        // For testing, we'll consider deleting user ID 1 and 2 as successful
        return userId == 1 || userId == 2;
    }

    bool verifyCredentials(const QString& email, const QString& passwordHash) override {
        if (email == "test@example.com" && passwordHash == "hashed_password") {
            return true;
        }
        return false;
    }
};

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
        return true;
    }

    bool isFavorite(int userId, int recipeId) override {
        return userId == 1 && (recipeId == 101 || recipeId == 102);
    }
};

class MockReviewRepository : public IReviewRepository {
public:
    QList<Review> getReviewsByRecipeId(int recipeId) override {
        QList<Review> reviews;
        return reviews;
    }

    QList<Review> getReviewsByUserId(int userId) override {
        QList<Review> reviews;
        if (userId == 1) {
            reviews.append(Review(1, 1, 101, 5, "Great recipe!", QDateTime::currentDateTime()));
            reviews.append(Review(2, 1, 102, 4, "Very good!", QDateTime::currentDateTime()));
            reviews.append(Review(3, 1, 103, 3, "Average", QDateTime::currentDateTime()));
        }
        return reviews;
    }

    bool addReview(const Review& review) override {
        return true;
    }

    bool updateReview(const Review& review) override {
        return true;
    }

    bool deleteReview(int reviewId) override {
        return true;
    }

    double getAverageRatingForRecipe(int recipeId) override {
        return 4.0;
    }
};

}

class UserProcessorTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testGetUserById();
    void testGetUserByEmail();
    void testGetUsersByUsername();
    void testGetUsersByRole();
    void testUpdateUserProfile();
    void testUpdateUserProfileWithNonExistentUser();
    void testUpdateUserProfileWithDuplicateEmail();
    void testDeleteUser();
    void testDeleteNonExistentUser();
    void testGetUserProfile();
    void testGetNonExistentUserProfile();
    void cleanupTestCase();

private:
    std::shared_ptr<UserProcessor> userProcessor;
    std::shared_ptr<UserTest::MockUserRepository> userRepository;
    std::shared_ptr<UserTest::MockFavoriteRepository> favoriteRepository;
    std::shared_ptr<UserTest::MockReviewRepository> reviewRepository;
    std::shared_ptr<ILogger> logger;
};

void UserProcessorTest::initTestCase() {
    // Инициализируем мок-репозитории
    userRepository = std::make_shared<UserTest::MockUserRepository>();
    favoriteRepository = std::make_shared<UserTest::MockFavoriteRepository>();
    reviewRepository = std::make_shared<UserTest::MockReviewRepository>();
    logger = std::make_shared<ConsoleLogger>(LogLevel::DEBUG);

    // Создаем тестируемый процессор
    userProcessor = std::make_shared<UserProcessor>(
        userRepository,
        favoriteRepository,
        reviewRepository,
        logger
        );
}

void UserProcessorTest::testGetUserById() {
    // Получение существующего пользователя
    auto user = userProcessor->getUserById(1);
    QVERIFY(user.has_value());
    QCOMPARE(user->getId(), 1);
    QCOMPARE(user->getUsername(), QString("testuser"));
    QCOMPARE(user->getEmail(), QString("test@example.com"));

    // Получение несуществующего пользователя
    auto nonExistentUser = userProcessor->getUserById(999);
    QVERIFY(!nonExistentUser.has_value());
}

void UserProcessorTest::testGetUserByEmail() {
    // Получение существующего пользователя по email
    auto user = userProcessor->getUserByEmail("test@example.com");
    QVERIFY(user.has_value());
    QCOMPARE(user->getId(), 1);
    QCOMPARE(user->getUsername(), QString("testuser"));

    // Получение несуществующего пользователя по email
    auto nonExistentUser = userProcessor->getUserByEmail("nonexistent@example.com");
    QVERIFY(!nonExistentUser.has_value());
}

void UserProcessorTest::testGetUsersByUsername() {
    // Получение пользователей с именами, содержащими "test"
    QList<User> testUsers = userProcessor->getUsersByUsername("test");
    QCOMPARE(testUsers.size(), 1);
    QCOMPARE(testUsers[0].getId(), 1);

    // Получение пользователей с именами, содержащими "another"
    QList<User> anotherUsers = userProcessor->getUsersByUsername("another");
    QCOMPARE(anotherUsers.size(), 1);
    QCOMPARE(anotherUsers[0].getId(), 2);

    // Поиск с несколькими совпадениями
    QList<User> allUsers = userProcessor->getUsersByUsername("user");
    QCOMPARE(allUsers.size(), 0); // В нашем моке нет пользователей с "user" в имени
}

void UserProcessorTest::testGetUsersByRole() {
    // Получение администраторов
    QList<User> adminUsers = userProcessor->getUsersByRole(UserRole::ADMIN);
    QCOMPARE(adminUsers.size(), 1);
    QCOMPARE(adminUsers[0].getId(), 3);
    QCOMPARE(adminUsers[0].getRole(), UserRole::ADMIN);

    // Получение обычных пользователей
    QList<User> regularUsers = userProcessor->getUsersByRole(UserRole::USER);
    QCOMPARE(regularUsers.size(), 2);
    QCOMPARE(regularUsers[0].getRole(), UserRole::USER);
    QCOMPARE(regularUsers[1].getRole(), UserRole::USER);
}

void UserProcessorTest::testUpdateUserProfile() {
    // Создаем обновленного пользователя
    User updatedUser(1, "testuser_updated", "test@example.com", "hashed_password", QDateTime::currentDateTime(), UserRole::USER);

    // Обновляем профиль
    bool result = userProcessor->updateUserProfile(updatedUser);
    QVERIFY(result);
}

void UserProcessorTest::testUpdateUserProfileWithNonExistentUser() {
    // Создаем несуществующего пользователя
    User nonExistentUser(999, "nonexistent", "nonexistent@example.com", "hash", QDateTime::currentDateTime(), UserRole::USER);

    // Пытаемся обновить профиль
    bool result = userProcessor->updateUserProfile(nonExistentUser);
    QVERIFY(!result);
}

void UserProcessorTest::testUpdateUserProfileWithDuplicateEmail() {
    // Создаем пользователя с email, который уже используется другим пользователем
    User user(1, "testuser", "another@example.com", "hashed_password", QDateTime::currentDateTime(), UserRole::USER);

    // Пытаемся обновить профиль
    bool result = userProcessor->updateUserProfile(user);
    QVERIFY(!result);
}

void UserProcessorTest::testDeleteUser() {
    // Удаляем существующего пользователя
    bool result = userProcessor->deleteUser(1);
    QVERIFY(result);
}

void UserProcessorTest::testDeleteNonExistentUser() {
    // Пытаемся удалить несуществующего пользователя
    bool result = userProcessor->deleteUser(999);
    QVERIFY(!result);
}

void UserProcessorTest::testGetUserProfile() {
    // Получаем профиль существующего пользователя
    auto profile = userProcessor->getUserProfile(1);
    QVERIFY(profile.has_value());
    QCOMPARE(profile->id, 1);
    QCOMPARE(profile->username, QString("testuser"));
    QCOMPARE(profile->email, QString("test@example.com"));
    QCOMPARE(profile->favoriteRecipesCount, 2);
    QCOMPARE(profile->reviewsCount, 3);
}

void UserProcessorTest::testGetNonExistentUserProfile() {
    // Пытаемся получить профиль несуществующего пользователя
    auto profile = userProcessor->getUserProfile(999);
    QVERIFY(!profile.has_value());
}

void UserProcessorTest::cleanupTestCase() {
    // Очистка ресурсов, если необходимо
    userProcessor.reset();
    userRepository.reset();
    favoriteRepository.reset();
    reviewRepository.reset();
}

// QTEST_APPLESS_MAIN(UserProcessorTest)
#include "tst_userprocessor.moc"
