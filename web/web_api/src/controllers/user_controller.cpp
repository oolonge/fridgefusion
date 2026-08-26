#include "web_api/controllers/user_controller.h"
#include "web_api/utils/request_validator.h"
#include "web_api/utils/response_builder.h"
#include "web_api/utils/json_converter.h"

namespace web_api {

UserController::UserController(std::shared_ptr<IUserProcessor> userProcessor,
                               std::shared_ptr<JWTMiddleware> jwtMiddleware)
    : userProcessor_(userProcessor), jwtMiddleware_(jwtMiddleware) {
}

crow::response UserController::getProfile(const crow::request& req) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }
        
        // Get user profile
        auto profileOpt = userProcessor_->getUserProfile(userId.value());
        
        if (!profileOpt.has_value()) {
            return ResponseBuilder::notFound("User profile not found");
        }
        
        auto responseJson = JsonConverter::userProfileToJson(profileOpt.value());
        return ResponseBuilder::ok(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response UserController::updateProfile(const crow::request& req) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }
        
        auto json = crow::json::load(req.body);
        if (!json) {
            return ResponseBuilder::badRequest("Invalid JSON");
        }
        
        // Validate input
        auto validation = RequestValidator::validateUserUpdate(json);
        if (!validation.isValid) {
            crow::json::wvalue details;
            details[validation.field] = validation.message;
            return ResponseBuilder::badRequest("Validation failed", details);
        }
        
        // Get current user
        auto userOpt = userProcessor_->getUserById(userId.value());
        if (!userOpt.has_value()) {
            return ResponseBuilder::notFound("User not found");
        }
        
        User user = userOpt.value();
        
        // Update fields if provided
        if (json.has("username")) {
            user.setUsername(QString::fromStdString(json["username"].s()));
        }
        if (json.has("email")) {
            user.setEmail(QString::fromStdString(json["email"].s()));
        }
        if (json.has("password")) {
            // NOTE: Password update requires hashing, which should be done in the processor
            // For now, we'll skip password update as it requires IPasswordHasher
            // In production, this would need to be handled properly
            crow::json::wvalue responseJson;
            responseJson["error"] = "NotImplemented";
            responseJson["message"] = "Password update is not yet supported in this API version";
            return ResponseBuilder::error(501, "Not Implemented", 
                                         "Password update requires proper hashing implementation");
        }
        
        // Update user profile
        bool success = userProcessor_->updateUserProfile(user);
        
        if (!success) {
            return ResponseBuilder::badRequest("Failed to update profile");
        }
        
        // Get updated profile
        auto profileOpt = userProcessor_->getUserProfile(userId.value());
        if (!profileOpt.has_value()) {
            return ResponseBuilder::internalError("Profile updated but couldn't retrieve");
        }
        
        auto responseJson = JsonConverter::userProfileToJson(profileOpt.value());
        return ResponseBuilder::ok(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response UserController::deleteAccount(const crow::request& req) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }
        
        // Delete user account
        bool success = userProcessor_->deleteUser(userId.value());
        
        if (!success) {
            return ResponseBuilder::badRequest("Failed to delete account");
        }
        
        return ResponseBuilder::noContent();
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response UserController::getUserById(const crow::request& /*req*/, int id) {
    try {
        // Get public user info (no auth required)
        auto userOpt = userProcessor_->getUserById(id);
        
        if (!userOpt.has_value()) {
            return ResponseBuilder::notFound("User not found");
        }
        
        auto responseJson = JsonConverter::userPublicToJson(userOpt.value());
        return ResponseBuilder::ok(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

} // namespace web_api