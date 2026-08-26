#include "web_api/controllers/auth_controller.h"
#include "web_api/utils/request_validator.h"
#include "web_api/utils/response_builder.h"
#include "web_api/utils/json_converter.h"

namespace web_api {

AuthController::AuthController(std::shared_ptr<IAuthProcessor> authProcessor,
                               std::shared_ptr<JWTMiddleware> jwtMiddleware)
    : authProcessor_(authProcessor), jwtMiddleware_(jwtMiddleware) {
}

crow::response AuthController::registerUser(const crow::request& req) {
    try {
        auto json = crow::json::load(req.body);
        if (!json) {
            return ResponseBuilder::badRequest("Invalid JSON");
        }
        
        // Validate input
        auto validation = RequestValidator::validateUserRegistration(json);
        if (!validation.isValid) {
            crow::json::wvalue details;
            details[validation.field] = validation.message;
            return ResponseBuilder::badRequest("Validation failed", details);
        }
        
        // Extract data
        QString username = QString::fromStdString(json["username"].s());
        QString email = QString::fromStdString(json["email"].s());
        QString password = QString::fromStdString(json["password"].s());
        
        // Register user
        AuthResult result = authProcessor_->registerUser(username, email, password);
        
        if (!result.isSuccess()) {
            // Check error type for appropriate response
            if (result.getErrorType() == AuthErrorType::EmailAlreadyExists) {
                return ResponseBuilder::conflict(result.getErrorMessage().toStdString());
            }
            return ResponseBuilder::badRequest(result.getErrorMessage().toStdString());
        }
        
        // Generate JWT token
        if (result.getUser().has_value()) {
            std::string token = jwtMiddleware_->generateToken(result.getUser().value());
            
            // Create response with token
            crow::json::wvalue responseJson;
            responseJson["success"] = true;
            responseJson["message"] = "User registered successfully";
            responseJson["token"] = token;
            responseJson["user"] = JsonConverter::userToJson(result.getUser().value());
            
            return ResponseBuilder::created(responseJson);
        }
        
        return ResponseBuilder::internalError("Registration succeeded but user data unavailable");
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response AuthController::login(const crow::request& req) {
    try {
        auto json = crow::json::load(req.body);
        if (!json) {
            return ResponseBuilder::badRequest("Invalid JSON");
        }
        
        // Validate input
        auto validation = RequestValidator::validateLogin(json);
        if (!validation.isValid) {
            crow::json::wvalue details;
            details[validation.field] = validation.message;
            return ResponseBuilder::badRequest("Validation failed", details);
        }
        
        // Extract data
        QString email = QString::fromStdString(json["email"].s());
        QString password = QString::fromStdString(json["password"].s());
        
        // Authenticate
        AuthResult result = authProcessor_->login(email, password);
        
        if (!result.isSuccess()) {
            return ResponseBuilder::unauthorized(result.getErrorMessage().toStdString());
        }
        
        // Generate JWT token
        if (result.getUser().has_value()) {
            std::string token = jwtMiddleware_->generateToken(result.getUser().value());
            
            // Create response with token
            crow::json::wvalue responseJson;
            responseJson["success"] = true;
            responseJson["message"] = "Login successful";
            responseJson["token"] = token;
            responseJson["user"] = JsonConverter::userToJson(result.getUser().value());
            
            return ResponseBuilder::ok(responseJson);
        }
        
        return ResponseBuilder::internalError("Login succeeded but user data unavailable");
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response AuthController::logout(const crow::request& req) {
    try {
        // Check if user is authenticated
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        // In stateless JWT, logout is handled client-side by removing token
        // Server doesn't need to do anything
        
        crow::json::wvalue responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "Logged out successfully";
        
        return ResponseBuilder::ok(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

} // namespace web_api