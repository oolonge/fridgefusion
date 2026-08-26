#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>

enum class UserRole {
    GUEST,
    USER,
    ADMIN
};

QString userRoleToString(UserRole role);
UserRole stringToUserRole(const QString& roleStr);

class User {
public:
    User();
    User(int id, const QString& username, const QString& email,
         const QString& passwordHash, const QDateTime& registrationDate,
         UserRole role = UserRole::USER);

    int getId() const;
    QString getUsername() const;
    QString getEmail() const;
    QString getPasswordHash() const;
    QDateTime getRegistrationDate() const;
    UserRole getRole() const;

    void setId(int id);
    void setUsername(const QString& username);
    void setEmail(const QString& email);
    void setPasswordHash(const QString& passwordHash);
    void setRegistrationDate(const QDateTime& registrationDate);
    void setRole(UserRole role);

    bool isValid() const;
    bool isAdmin() const;

private:
    int id;
    QString username;
    QString email;
    QString passwordHash;
    QDateTime registrationDate;
    UserRole role;
};

#endif // USER_H
