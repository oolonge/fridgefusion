#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include "crow.h"
#include "application/interfaces/processors/IAuthProcessor.h"
#include "web_api/middleware/jwt_middleware.h"
#include <memory>

namespace web_api {

class AuthController {
public:
    AuthController(std::shared_ptr<IAuthProcessor> authProcessor,
                   std::shared_ptr<JWTMiddleware> jwtMiddleware);
    
    // POST /auth/register
    crow::response registerUser(const crow::request& req);
    
    // POST /auth/login
    crow::response login(const crow::request& req);
    
    // POST /auth/logout
    crow::response logout(const crow::request& req);

private:
    std::shared_ptr<IAuthProcessor> authProcessor_;
    std::shared_ptr<JWTMiddleware> jwtMiddleware_;
};

} // namespace web_api

#endif // AUTH_CONTROLLER_H