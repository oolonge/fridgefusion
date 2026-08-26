#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include "crow.h"
#include "application/interfaces/processors/IUserProcessor.h"
#include "web_api/middleware/jwt_middleware.h"
#include <memory>

namespace web_api {

class UserController {
public:
    UserController(std::shared_ptr<IUserProcessor> userProcessor,
                   std::shared_ptr<JWTMiddleware> jwtMiddleware);
    
    // GET /users/me
    crow::response getProfile(const crow::request& req);

    // PUT /users/me
    crow::response updateProfile(const crow::request& req);

    // DELETE /users/me
    crow::response deleteAccount(const crow::request& req);
    
    // GET /users/{id}
    crow::response getUserById(const crow::request& req, int id);

private:
    std::shared_ptr<IUserProcessor> userProcessor_;
    std::shared_ptr<JWTMiddleware> jwtMiddleware_;
};

} // namespace web_api

#endif // USER_CONTROLLER_H