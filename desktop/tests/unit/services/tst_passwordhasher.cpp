#include <QtTest>
#include "application/services/passwordHasher.h"

class PasswordHasherTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testHashPassword();
    void testVerifyPassword();
    void testInvalidPassword();
    void cleanupTestCase();

private:
    PasswordHasher* hasher;
};

void PasswordHasherTest::initTestCase() {
    hasher = new PasswordHasher();
}

void PasswordHasherTest::testHashPassword() {
    QString password = "testPassword123";
    QString hash = hasher->hashPassword(password);

    // Hash should not be empty
    QVERIFY(!hash.isEmpty());

    // Hash should contain two colons (iterations:salt:key)
    QStringList parts = hash.split(':');
    QCOMPARE(parts.size(), 3);

    // First part should be the iterations count (10000)
    QCOMPARE(parts[0], QString("10000"));

    // Salt and hash should not be empty
    QVERIFY(!parts[1].isEmpty());
    QVERIFY(!parts[2].isEmpty());

    // Different calls should produce different hashes (due to random salt)
    QString hash2 = hasher->hashPassword(password);
    QVERIFY(hash != hash2);
}

void PasswordHasherTest::testVerifyPassword() {
    QString password = "anotherPassword456";
    QString hash = hasher->hashPassword(password);

    // Verification should succeed with correct password
    QVERIFY(hasher->verifyPassword(password, hash));
}

void PasswordHasherTest::testInvalidPassword() {
    QString password = "correctPassword";
    QString wrongPassword = "wrongPassword";
    QString hash = hasher->hashPassword(password);

    // Verification should fail with incorrect password
    QVERIFY(!hasher->verifyPassword(wrongPassword, hash));

    // Test invalid hash format
    QVERIFY(!hasher->verifyPassword(password, "invalid:hash:format:with:too:many:parts"));
    QVERIFY(!hasher->verifyPassword(password, "invalid"));
    QVERIFY(!hasher->verifyPassword(password, "invalid:hash"));
    QVERIFY(!hasher->verifyPassword(password, "NaN:salt:hash"));
}

void PasswordHasherTest::cleanupTestCase() {
    delete hasher;
}

// QTEST_APPLESS_MAIN(PasswordHasherTest)
#include "tst_passwordhasher.moc"
