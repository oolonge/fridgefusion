#include "passwordHasher.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>

PasswordHasher::PasswordHasher() {
}

QString PasswordHasher::hashPassword(const QString& password) {
    // Generate a random salt
    QByteArray salt = generateSalt();

    // Use PBKDF2 to derive a key from the password
    QByteArray passwordBytes = password.toUtf8();
    QByteArray derivedKey = pbkdf2(passwordBytes, salt);

    // Format: iterations:salt:hash
    QString hashString = QString("10000:%1:%2")
                             .arg(bytesToHex(salt))
                             .arg(bytesToHex(derivedKey));

    return hashString;
}

bool PasswordHasher::verifyPassword(const QString& password, const QString& hash) {
    // Parse the hash string
    QStringList parts = hash.split(':');
    if (parts.size() != 3) {
        qDebug() << "Invalid hash format";
        return false;
    }

    bool ok;
    int iterations = parts[0].toInt(&ok);
    if (!ok) {
        qDebug() << "Invalid iterations value";
        return false;
    }

    QByteArray salt = hexToBytes(parts[1]);
    QByteArray storedKey = hexToBytes(parts[2]);

    // Derive key from the provided password
    QByteArray derivedKey = pbkdf2(password.toUtf8(), salt, iterations);

    // Compare the derived key with the stored key
    return derivedKey == storedKey;
}

QByteArray PasswordHasher::generateSalt(int length) const {
    QByteArray salt;
    salt.resize(length);

    // Fill with random bytes
    for (int i = 0; i < length; ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }

    return salt;
}

QByteArray PasswordHasher::pbkdf2(const QByteArray& password, const QByteArray& salt,
                                  int iterations, int keyLength) const {
    // This is a simplified implementation of PBKDF2
    // In a production environment, consider using a more secure library like libsodium

    // Initial value is the salt
    QByteArray result;
    QByteArray lastBlock = salt;

    // Append an integer counter (1) to the salt
    lastBlock.append(char(0));
    lastBlock.append(char(0));
    lastBlock.append(char(0));
    lastBlock.append(char(1));

    QByteArray key;
    QByteArray innerHash;

    // Perform iterations
    for (int i = 0; i < iterations; ++i) {
        if (i == 0) {
            innerHash = QCryptographicHash::hash(lastBlock + password, QCryptographicHash::Sha256);
        } else {
            innerHash = QCryptographicHash::hash(innerHash + password, QCryptographicHash::Sha256);
        }

        // XOR result with key
        if (i == 0) {
            key = innerHash;
        } else {
            for (int j = 0; j < innerHash.size() && j < key.size(); ++j) {
                key[j] = key[j] ^ innerHash[j];
            }
        }
    }

    // Truncate to the desired length if necessary
    if (key.size() > keyLength) {
        key.resize(keyLength);
    }

    return key;
}

QString PasswordHasher::bytesToHex(const QByteArray& bytes) const {
    return bytes.toHex();
}

QByteArray PasswordHasher::hexToBytes(const QString& hex) const {
    return QByteArray::fromHex(hex.toLatin1());
}
