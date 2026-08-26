#include "web_api/utils/request_validator.h"
#include <regex>

namespace web_api {

ValidationResult RequestValidator::validateUserRegistration(const crow::json::rvalue& json) {
    if (!hasField(json, "username")) {
        return ValidationResult::error("username", "Username is required");
    }
    std::string username = json["username"].s();
    if (!isValidUsername(username)) {
        return ValidationResult::error("username", "Username must be 3-50 characters");
    }
    
    if (!hasField(json, "email")) {
        return ValidationResult::error("email", "Email is required");
    }
    std::string email = json["email"].s();
    if (!isValidEmail(email)) {
        return ValidationResult::error("email", "Invalid email format");
    }
    
    if (!hasField(json, "password")) {
        return ValidationResult::error("password", "Password is required");
    }
    std::string password = json["password"].s();
    if (!isValidPassword(password)) {
        return ValidationResult::error("password", "Password must be at least 6 characters");
    }
    
    return ValidationResult::ok();
}

ValidationResult RequestValidator::validateLogin(const crow::json::rvalue& json) {
    if (!hasField(json, "email")) {
        return ValidationResult::error("email", "Email is required");
    }
    
    if (!hasField(json, "password")) {
        return ValidationResult::error("password", "Password is required");
    }
    
    return ValidationResult::ok();
}

ValidationResult RequestValidator::validateUserUpdate(const crow::json::rvalue& json) {
    if (hasField(json, "username")) {
        std::string username = json["username"].s();
        if (!isValidUsername(username)) {
            return ValidationResult::error("username", "Username must be 3-50 characters");
        }
    }
    
    if (hasField(json, "email")) {
        std::string email = json["email"].s();
        if (!isValidEmail(email)) {
            return ValidationResult::error("email", "Invalid email format");
        }
    }
    
    if (hasField(json, "password")) {
        std::string password = json["password"].s();
        if (!isValidPassword(password)) {
            return ValidationResult::error("password", "Password must be at least 6 characters");
        }
    }
    
    return ValidationResult::ok();
}

ValidationResult RequestValidator::validateRecipeCreate(const crow::json::rvalue& json) {
    if (!hasField(json, "name") || json["name"].s().size() == 0) {
        return {false, "name", "Name is required"};
    }
    
    if (!hasField(json, "description") || json["description"].s().size() == 0) {
        return {false, "description", "Description is required"};
    }
    
    if (!hasField(json, "preparation_time")) {
        return ValidationResult::error("preparation_time", "Preparation time is required");
    }
    if (!isPositiveInteger(json["preparation_time"].i())) {
        return ValidationResult::error("preparation_time", "Must be a positive integer");
    }
    
    if (!hasField(json, "cooking_time")) {
        return ValidationResult::error("cooking_time", "Cooking time is required");
    }
    if (!isPositiveInteger(json["cooking_time"].i())) {
        return ValidationResult::error("cooking_time", "Must be a positive integer");
    }
    
    if (!hasField(json, "instructions") || json["instructions"].size() == 0) {
        return ValidationResult::error("instructions", "At least one instruction is required");
    }
    
    if (!hasField(json, "ingredients") || json["ingredients"].size() == 0) {
        return ValidationResult::error("ingredients", "At least one ingredient is required");
    }
    
    // Validate each ingredient
    for (const auto& ingredient : json["ingredients"]) {
        if (!hasField(ingredient, "ingredient_id")) {
            return ValidationResult::error("ingredients", "Each ingredient must have ingredient_id");
        }
        if (!hasField(ingredient, "quantity")) {
            return ValidationResult::error("ingredients", "Each ingredient must have quantity");
        }
        if (!hasField(ingredient, "unit_id")) {
            return ValidationResult::error("ingredients", "Each ingredient must have unit_id");
        }
    }
    
    return ValidationResult::ok();
}

ValidationResult RequestValidator::validateRecipeUpdate(const crow::json::rvalue& json) {
    return validateRecipeCreate(json);
}

ValidationResult RequestValidator::validateRecipePatch(const crow::json::rvalue& json) {
    if (hasField(json, "preparation_time")) {
        if (!isPositiveInteger(json["preparation_time"].i())) {
            return ValidationResult::error("preparation_time", "Must be a positive integer");
        }
    }
    
    if (hasField(json, "cooking_time")) {
        if (!isPositiveInteger(json["cooking_time"].i())) {
            return ValidationResult::error("cooking_time", "Must be a positive integer");
        }
    }
    
    return ValidationResult::ok();
}

ValidationResult RequestValidator::validateReviewCreate(const crow::json::rvalue& json) {
    if (!hasField(json, "rating")) {
        return ValidationResult::error("rating", "Rating is required");
    }
    
    int rating = json["rating"].i();
    if (!isValidRating(rating)) {
        return ValidationResult::error("rating", "Rating must be between 1 and 5");
    }
    
    return ValidationResult::ok();
}

ValidationResult RequestValidator::validateReviewUpdate(const crow::json::rvalue& json) {
    if (hasField(json, "rating")) {
        int rating = json["rating"].i();
        if (!isValidRating(rating)) {
            return ValidationResult::error("rating", "Rating must be between 1 and 5");
        }
    }
    
    return ValidationResult::ok();
}

ValidationResult RequestValidator::validatePagination(int limit, int offset) {
    if (limit < 1 || limit > 100) {
        return ValidationResult::error("limit", "Limit must be between 1 and 100");
    }
    
    if (offset < 0) {
        return ValidationResult::error("offset", "Offset must be non-negative");
    }
    
    return ValidationResult::ok();
}

bool RequestValidator::isValidEmail(const std::string& email) {
    const std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, pattern);
}

bool RequestValidator::isValidPassword(const std::string& password) {
    return password.length() >= 6;
}

bool RequestValidator::isValidUsername(const std::string& username) {
    return username.length() >= 3 && username.length() <= 50;
}

bool RequestValidator::isValidRating(int rating) {
    return rating >= 1 && rating <= 5;
}

bool RequestValidator::hasField(const crow::json::rvalue& json, const std::string& field) {
    return json.has(field) && json[field];
}

bool RequestValidator::isPositiveInteger(int value) {
    return value > 0;
}

} // namespace web_api