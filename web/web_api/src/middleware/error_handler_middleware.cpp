#include "web_api/middleware/error_handler_middleware.h"
#include <iostream>
#include <ctime>

namespace web_api {

crow::response ErrorHandlerMiddleware::handleException(const std::exception& e) {
    std::string errorMsg = e.what();
    
    // Log the error
    std::cerr << "[ERROR] " << errorMsg << std::endl;
    
    // Return generic internal server error
    return ResponseBuilder::internalError("An unexpected error occurred");
}

void ErrorHandlerMiddleware::logError(const std::string& endpoint, const std::string& error) {
    std::time_t now = std::time(nullptr);
    char timeStr[100];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    
    std::cerr << "[" << timeStr << "] ERROR at " << endpoint << ": " << error << std::endl;
}

} // namespace web_api