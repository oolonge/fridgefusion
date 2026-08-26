#include <QtTest>
#include "application/processors/authProcessor.h"
#include "application/services/passwordHasher.h"
#include "utils/logging/ConsoleLogger.h"
#include <memory>

// Замените Mock классы в tst_authprocessor.cpp на эти исправленные версии:

class MockUserRepository : public IUserRepository {
public:
    std::optional<User> getUserById(int userId) override {
        if (userId == 1) {
            return User(1, "testuser", "test@example.com", "correct_password_hashed", QDateTime::currentDateTime(), UserRole::USER);
        }
        return std::nullopt;
    }

    std::optional<User> getUserByEmail(const QString& email) override {
        if (email == "test@example.com") {
            return User(1, "testuser", "test@example.com", "correct_password_hashed", QDateTime::currentDateTime(), UserRole::USER);
        } else if (email == "admin@example.com") {
            return User(2, "admin", "admin@example.com", "admin_password_hashed", QDateTime::currentDateTime(), UserRole::ADMIN);
        }
        return std::nullopt;
    }

    QList<User> getUsersByUsername(const QString& username) override {
        QList<User> users;
        if (username.contains("test")) {
            users.append(User(1, "testuser", "test@example.com", "correct_password_hashed", QDateTime::currentDateTime(), UserRole::USER));
        }
        if (username.contains("admin")) {
            users.append(User(2, "admin", "admin@example.com", "admin_password_hashed", QDateTime::currentDateTime(), UserRole::ADMIN));
        }
        return users;
    }

    QList<User> getUsersByRole(UserRole role) override {
        QList<User> users;
        if (role == UserRole::ADMIN) {
            users.append(User(2, "admin", "admin@example.com", "admin_password_hashed", QDateTime::currentDateTime(), UserRole::ADMIN));
        } else if (role == UserRole::USER) {
            users.append(User(1, "testuser", "test@example.com", "correct_password_hashed", QDateTime::currentDateTime(), UserRole::USER));
        }
        return users;
    }

    QList<User> getAllUsers(int limit = 100, int offset = 0) override {
        QList<User> users;
        users.append(User(1, "testuser", "test@example.com", "correct_password_hashed", QDateTime::currentDateTime(), UserRole::USER));
        users.append(User(2, "admin", "admin@example.com", "admin_password_hashed", QDateTime::currentDateTime(), UserRole::ADMIN));
        return users;
    }

    bool updateUserRole(int userId, UserRole role) override {
        return userId == 1 || userId == 2;
    }

    bool saveUser(User& user) override {
        // Simulate successful save and ID assignment
        if (user.getEmail() != "error@example.com") {
            user.setId(3); // Assign new ID
            return true;
        }
        return false;
    }

    bool updateUser(const User& user) override {
        // Only allow updating existing users (ID 1 and 2)
        return user.getId() == 1 || user.getId() == 2;
    }

    bool deleteUser(int userId) override {
        // Only allow deleting existing users (ID 1 and 2)
        return userId == 1 || userId == 2;
    }

    bool verifyCredentials(const QString& email, const QString& passwordHash) override {
        if (email == "test@example.com" && passwordHash == "correct_password_hashed") {
            return true;
        }
        if (email == "admin@example.com" && passwordHash == "admin_password_hashed") {
            return true;
        }
        return false;
    }
};

class MockPasswordHasher : public IPasswordHasher {
public:
    QString hashPassword(const QString& password) override {
        return password + "_hashed";
    }

    bool verifyPassword(const QString& password, const QString& hash) override {
        if (hash == "correct_password_hashed") {
            return password == "correct_password";
        }
        if (hash == "admin_password_hashed") {
            return password == "admin_password";
        }
        if (hash.endsWith("_hashed")) {
            QString expectedPassword = hash;
            expectedPassword.chop(7); // Убираем "_hashed"
            return password == expectedPassword;
        }
        return false;
    }
};

class AuthProcessorTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testLogin_Success();
    void testLogin_UserNotFound();
    void testLogin_InvalidPassword();
    void testRegistration_Success();
    void testRegistration_EmailExists();
    void testRegistration_InvalidInput();
    void testLogout();
    void testIsAdmin();
    void cleanupTestCase();

private:
    std::shared_ptr<AuthProcessor> authProcessor;
    std::shared_ptr<MockUserRepository> userRepository;
    std::shared_ptr<MockPasswordHasher> passwordHasher;
    std::shared_ptr<ILogger> logger;
};

void AuthProcessorTest::initTestCase() {
    // Setup mock repositories and services
    userRepository = std::make_shared<MockUserRepository>();
    passwordHasher = std::make_shared<MockPasswordHasher>();
    logger = std::make_shared<ConsoleLogger>(LogLevel::DEBUG);

    // Create the auth processor with mocks
    authProcessor = std::make_shared<AuthProcessor>(userRepository, passwordHasher, logger);
}

void AuthProcessorTest::testLogin_Success() {
    // Test successful login
    AuthResult result = authProcessor->login("test@example.com", "correct_password");

    QVERIFY(result.isSuccess());
    QVERIFY(result.getUser().has_value());
    QCOMPARE(result.getUser()->getEmail(), QString("test@example.com"));
    QCOMPARE(result.getUser()->getUsername(), QString("testuser"));
    QVERIFY(authProcessor->isUserLoggedIn());
    QCOMPARE(authProcessor->getCurrentUserId(), 1);
}

void AuthProcessorTest::testLogin_UserNotFound() {
    // Logout first
    authProcessor->logout();

    // Test login with non-existent email
    AuthResult result = authProcessor->login("nonexistent@example.com", "any_password");

    QVERIFY(!result.isSuccess());
    QCOMPARE(result.getErrorType(), AuthErrorType::UserNotFound);
    QVERIFY(!result.getErrorMessage().isEmpty());
    QVERIFY(!authProcessor->isUserLoggedIn());
}

void AuthProcessorTest::testLogin_InvalidPassword() {
    // Test login with wrong password
    AuthResult result = authProcessor->login("test@example.com", "wrong_password");

    QVERIFY(!result.isSuccess());
    QCOMPARE(result.getErrorType(), AuthErrorType::InvalidPassword);
    QVERIFY(!result.getErrorMessage().isEmpty());
}

void AuthProcessorTest::testRegistration_Success() {
    // Logout first
    authProcessor->logout();

    // Test successful registration
    AuthResult result = authProcessor->registerUser("newuser", "new@example.com", "password123");

    QVERIFY(result.isSuccess());
    QVERIFY(result.getUser().has_value());
    QCOMPARE(result.getUser()->getEmail(), QString("new@example.com"));
    QCOMPARE(result.getUser()->getUsername(), QString("newuser"));
    QVERIFY(authProcessor->isUserLoggedIn());
}

void AuthProcessorTest::testRegistration_EmailExists() {
    // Logout first
    authProcessor->logout();

    // Test registration with existing email
    AuthResult result = authProcessor->registerUser("duplicate", "test@example.com", "password123");

    QVERIFY(!result.isSuccess());
    QCOMPARE(result.getErrorType(), AuthErrorType::EmailAlreadyExists);
    QVERIFY(!result.getErrorMessage().isEmpty());
}

void AuthProcessorTest::testRegistration_InvalidInput() {
    // Test registration with empty fields
    AuthResult result = authProcessor->registerUser("", "email@example.com", "password");

    QVERIFY(!result.isSuccess());
    QCOMPARE(result.getErrorType(), AuthErrorType::InvalidInput);

    result = authProcessor->registerUser("username", "", "password");
    QVERIFY(!result.isSuccess());
    QCOMPARE(result.getErrorType(), AuthErrorType::InvalidInput);

    result = authProcessor->registerUser("username", "email@example.com", "");
    QVERIFY(!result.isSuccess());
    QCOMPARE(result.getErrorType(), AuthErrorType::InvalidInput);
}

void AuthProcessorTest::testLogout() {
    // Login first
    authProcessor->login("test@example.com", "correct_password");
    QVERIFY(authProcessor->isUserLoggedIn());

    // Test logout
    bool result = authProcessor->logout();

    QVERIFY(result);
    QVERIFY(!authProcessor->isUserLoggedIn());
    QVERIFY(!authProcessor->getCurrentUser().has_value());
    QCOMPARE(authProcessor->getCurrentUserId(), -1);
}

void AuthProcessorTest::testIsAdmin() {
    // Login as regular user
    authProcessor->login("test@example.com", "correct_password");
    QVERIFY(!authProcessor->isCurrentUserAdmin());

    // Test permissions for regular user
    QVERIFY(authProcessor->hasPermission("user.view"));   // Разрешено для всех
    QVERIFY(authProcessor->hasPermission("recipe.view")); // Разрешено для всех
    QVERIFY(!authProcessor->hasPermission("user.edit"));  // НЕ разрешено для обычных пользователей

    // Login as admin
    authProcessor->login("admin@example.com", "admin_password");
    QVERIFY(authProcessor->isCurrentUserAdmin());

    // Test permissions for admin (admin has all permissions)
    QVERIFY(authProcessor->hasPermission("user.view"));
    QVERIFY(authProcessor->hasPermission("user.edit"));
    QVERIFY(authProcessor->hasPermission("user.delete"));
    QVERIFY(authProcessor->hasPermission("any_permission")); // Админ может всё

    // Logout and check permissions
    authProcessor->logout();
    QVERIFY(!authProcessor->hasPermission("user.view"));
    QVERIFY(!authProcessor->isCurrentUserAdmin());
}

void AuthProcessorTest::cleanupTestCase() {
    // Nothing to clean up now that we don't have session storage
}

// QTEST_APPLESS_MAIN(AuthProcessorTest)
#include "tst_authprocessor.moc"
