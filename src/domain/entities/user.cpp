#include "user.h"

QString userRoleToString(UserRole role) {
    switch (role) {
    case UserRole::GUEST: return "GUEST";
    case UserRole::USER: return "USER";
    case UserRole::ADMIN: return "ADMIN";
    default: return "USER";
    }
}

UserRole stringToUserRole(const QString& roleStr) {
    if (roleStr.toUpper() == "ADMIN") return UserRole::ADMIN;
    if (roleStr.toUpper() == "GUEST") return UserRole::GUEST;
    return UserRole::USER;  // По умолчанию USER
}

User::User()
    : id(0), registrationDate(QDateTime::currentDateTime()), role(UserRole::USER) {
}

User::User(int id, const QString& username, const QString& email,
           const QString& passwordHash, const QDateTime& registrationDate,
           UserRole role)
    : id(id), username(username), email(email),
    passwordHash(passwordHash), registrationDate(registrationDate), role(role) {
}

int User::getId() const {
    return id;
}

QString User::getUsername() const {
    return username;
}

QString User::getEmail() const {
    return email;
}

QString User::getPasswordHash() const {
    return passwordHash;
}

QDateTime User::getRegistrationDate() const {
    return registrationDate;
}

UserRole User::getRole() const {
    return role;
}

void User::setId(int id) {
    this->id = id;
}

void User::setUsername(const QString& username) {
    this->username = username;
}

void User::setEmail(const QString& email) {
    this->email = email;
}

void User::setPasswordHash(const QString& passwordHash) {
    this->passwordHash = passwordHash;
}

void User::setRegistrationDate(const QDateTime& registrationDate) {
    this->registrationDate = registrationDate;
}

void User::setRole(UserRole role) {
    this->role = role;
}

bool User::isValid() const {
    return !username.isEmpty() && !email.isEmpty() && !passwordHash.isEmpty();
}

bool User::isAdmin() const {
    return role == UserRole::ADMIN;
}
