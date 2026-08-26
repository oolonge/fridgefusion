#ifndef ADMIN_CONTROLLER_H
#define ADMIN_CONTROLLER_H

#include "crow.h"
#include "application/interfaces/processors/IAdminProcessor.h"
#include "web_api/middleware/jwt_middleware.h"
#include <memory>

namespace web_api {

class AdminController {
public:
    AdminController(std::shared_ptr<IAdminProcessor> adminProcessor,
                    std::shared_ptr<JWTMiddleware> jwtMiddleware);
    
    // GET /admin/users
    crow::response getUsers(const crow::request& req);
    
    // PATCH /admin/users/{id}/role
    crow::response updateUserRole(const crow::request& req, int id);
    
    // DELETE /admin/users/{id}
    crow::response deleteUser(const crow::request& req, int id);

private:
    std::shared_ptr<IAdminProcessor> adminProcessor_;
    std::shared_ptr<JWTMiddleware> jwtMiddleware_;
};

} // namespace web_api

#endif // ADMIN_CONTROLLER_H