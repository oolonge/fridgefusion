#ifndef IPASSWORDHASHER_H
#define IPASSWORDHASHER_H

#include <QString>

class IPasswordHasher {
public:
    virtual ~IPasswordHasher() = default;

    // Hash a password
    virtual QString hashPassword(const QString& password) = 0;

    // Verify a password against a hash
    virtual bool verifyPassword(const QString& password, const QString& hash) = 0;
};

#endif // IPASSWORDHASHER_H
