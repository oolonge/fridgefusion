#ifndef REQUEST_VALIDATOR_H
#define REQUEST_VALIDATOR_H

#include "crow.h"
#include <string>
#include <vector>
#include <optional>

namespace web_api {

struct ValidationResult {
    bool isValid;
    std::string field;
    std::string message;
    
    static ValidationResult ok() {
        return {true, "", ""};
    }
    
    static ValidationResult error(const std::string& field, const std::string& message) {
        return {false, field, message};
    }
};

class RequestValidator {
public:
    // User validation
    static ValidationResult validateUserRegistration(const crow::json::rvalue& json);
    static ValidationResult validateLogin(const crow::json::rvalue& json);
    static ValidationResult validateUserUpdate(const crow::json::rvalue& json);
    
    // Recipe validation
    static ValidationResult validateRecipeCreate(const crow::json::rvalue& json);
    static ValidationResult validateRecipeUpdate(const crow::json::rvalue& json);
    static ValidationResult validateRecipePatch(const crow::json::rvalue& json);
    
    // Review validation
    static ValidationResult validateReviewCreate(const crow::json::rvalue& json);
    static ValidationResult validateReviewUpdate(const crow::json::rvalue& json);
    
    // Pagination validation
    static ValidationResult validatePagination(int limit, int offset);
    
    // Field validators
    static bool isValidEmail(const std::string& email);
    static bool isValidPassword(const std::string& password);
    static bool isValidUsername(const std::string& username);
    static bool isValidRating(int rating);
    
    // Helper methods
    static bool hasField(const crow::json::rvalue& json, const std::string& field);
    static bool isPositiveInteger(int value);
};

} // namespace web_api

#endif // REQUEST_VALIDATOR_H