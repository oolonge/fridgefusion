#include "web_api/controllers/admin_controller.h"
#include "web_api/utils/response_builder.h"
#include "web_api/utils/json_converter.h"

namespace web_api {

AdminController::AdminController(std::shared_ptr<IAdminProcessor> adminProcessor,
                                 std::shared_ptr<JWTMiddleware> jwtMiddleware)
    : adminProcessor_(adminProcessor), jwtMiddleware_(jwtMiddleware) {
}

crow::response AdminController::getUsers(const crow::request& req) {
    try {
        // Check authentication and admin role
        if (!jwtMiddleware_->hasRole(req, UserRole::ADMIN)) {
            return ResponseBuilder::forbidden("Admin access required");
        }
        
        // Parse pagination
        int limit = 100;
        int offset = 0;
        
        auto limitParam = req.url_params.get("limit");
        if (limitParam) {
            limit = std::stoi(limitParam);
        }
        
        auto offsetParam = req.url_params.get("offset");
        if (offsetParam) {
            offset = std::stoi(offsetParam);
        }
        
        // Check if filtering by role
        auto roleParam = req.url_params.get("role");
        QList<User> users;
        
        if (roleParam) {
            std::string roleStr = roleParam;
            UserRole roleFilter;
            
            if (roleStr == "ADMIN") {
                roleFilter = UserRole::ADMIN;
            } else if (roleStr == "USER") {
                roleFilter = UserRole::USER;
            } else {
                return ResponseBuilder::badRequest("Invalid role parameter. Must be USER or ADMIN");
            }
            
            // Get users by role using dedicated method
            users = adminProcessor_->getUsersByRole(roleFilter);
            
            // Manual pagination for role-filtered results
            if (offset >= users.size()) {
                users.clear();
            } else {
                qsizetype endIndex = std::min(static_cast<qsizetype>(offset + limit), users.size());
                QList<User> paginatedUsers;
                for (int i = offset; i < endIndex; ++i) {
                    paginatedUsers.append(users[i]);
                }
                users = paginatedUsers;
            }
        } else {
            // Get all users with pagination
            users = adminProcessor_->getAllUsers(limit, offset);
        }
        
        // Convert to JSON
        std::vector<crow::json::wvalue> usersJson;
        for (const auto& user : users) {
            usersJson.push_back(JsonConverter::userToJson(user));
        }
        
        return ResponseBuilder::ok(crow::json::wvalue(usersJson));
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response AdminController::updateUserRole(const crow::request& req, int id) {
    try {
        // Check authentication and admin role
        if (!jwtMiddleware_->hasRole(req, UserRole::ADMIN)) {
            return ResponseBuilder::forbidden("Admin access required");
        }
        
        auto json = crow::json::load(req.body);
        if (!json) {
            return ResponseBuilder::badRequest("Invalid JSON");
        }
        
        if (!json.has("role")) {
            return ResponseBuilder::badRequest("Role field is required");
        }
        
        std::string roleStr = json["role"].s();
        UserRole newRole;
        
        if (roleStr == "ADMIN") {
            newRole = UserRole::ADMIN;
        } else if (roleStr == "USER") {
            newRole = UserRole::USER;
        } else {
            return ResponseBuilder::badRequest("Invalid role. Must be USER or ADMIN");
        }
        
        // Update role
        bool success = adminProcessor_->updateUserRole(id, newRole);
        
        if (!success) {
            return ResponseBuilder::notFound("User not found");
        }
        
        crow::json::wvalue responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "User role updated successfully";
        
        return ResponseBuilder::ok(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response AdminController::deleteUser(const crow::request& req, int id) {
    try {
        // Check authentication and admin role
        if (!jwtMiddleware_->hasRole(req, UserRole::ADMIN)) {
            return ResponseBuilder::forbidden("Admin access required");
        }
        
        // Prevent admin from deleting themselves
        auto currentUserId = jwtMiddleware_->getUserId(req);
        if (currentUserId.has_value() && currentUserId.value() == id) {
            return ResponseBuilder::badRequest("Cannot delete your own account");
        }
        
        // Delete user
        bool success = adminProcessor_->deleteUser(id);
        
        if (!success) {
            return ResponseBuilder::notFound("User not found");
        }
        
        return ResponseBuilder::noContent();
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

} // namespace web_api