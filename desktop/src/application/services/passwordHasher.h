#ifndef PASSWORDHASHER_H
#define PASSWORDHASHER_H

#include "application/interfaces/services/IPasswordHasher.h"
#include <QCryptographicHash>
#include <QByteArray>

class PasswordHasher : public IPasswordHasher {
public:
    PasswordHasher();
    ~PasswordHasher() override = default;

    // Hash a password using PBKDF2 or similar algorithm
    QString hashPassword(const QString& password) override;

    // Verify a password against a hash
    bool verifyPassword(const QString& password, const QString& hash) override;

private:
    // Generate a random salt
    QByteArray generateSalt(int length = 16) const;

    // PBKDF2 key derivation
    QByteArray pbkdf2(const QByteArray& password, const QByteArray& salt,
                      int iterations = 10000, int keyLength = 32) const;

    // Helper methods
    QString bytesToHex(const QByteArray& bytes) const;
    QByteArray hexToBytes(const QString& hex) const;
};

#endif // PASSWORDHASHER_H
