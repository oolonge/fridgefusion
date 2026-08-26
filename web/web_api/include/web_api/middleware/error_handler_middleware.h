#ifndef ERROR_HANDLER_MIDDLEWARE_H
#define ERROR_HANDLER_MIDDLEWARE_H

#include "crow.h"
#include "web_api/utils/response_builder.h"
#include <exception>
#include <string>

namespace web_api {

class ErrorHandlerMiddleware {
public:
    // Handle exception and return appropriate response
    static crow::response handleException(const std::exception& e);
    
    // Log error
    static void logError(const std::string& endpoint, const std::string& error);
};

} // namespace web_api

#endif // ERROR_HANDLER_MIDDLEWARE_H