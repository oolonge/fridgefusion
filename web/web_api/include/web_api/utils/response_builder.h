#ifndef RESPONSE_BUILDER_H
#define RESPONSE_BUILDER_H

#include "crow.h"
#include <string>
#include <optional>

namespace web_api {

class ResponseBuilder {
public:
    // Success responses
    static crow::response ok(crow::json::wvalue data);
    static crow::response created(crow::json::wvalue data);
    static crow::response noContent();
    
    // Error responses
    static crow::response badRequest(const std::string& message,
                                     std::optional<crow::json::wvalue> details = std::nullopt);
    static crow::response unauthorized(const std::string& message = "Authentication required");
    static crow::response forbidden(const std::string& message = "Access forbidden");
    static crow::response notFound(const std::string& message = "Resource not found");
    static crow::response conflict(const std::string& message = "Resource conflict");
    static crow::response internalError(const std::string& message = "Internal server error");
    
    // Custom error
    static crow::response error(int statusCode, const std::string& error,
                               const std::string& message);
};

} // namespace web_api

#endif // RESPONSE_BUILDER_H