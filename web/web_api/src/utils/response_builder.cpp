#include "web_api/utils/response_builder.h"

namespace web_api {

crow::response ResponseBuilder::ok(crow::json::wvalue data) {
    crow::response res(200);
    res.set_header("Content-Type", "application/json");
    res.body = data.dump();
    return res;
}

crow::response ResponseBuilder::created(crow::json::wvalue data) {
    crow::response res(201);
    res.set_header("Content-Type", "application/json");
    res.body = data.dump();
    return res;
}

crow::response ResponseBuilder::noContent() {
    return crow::response(204);
}

crow::response ResponseBuilder::badRequest(const std::string& message,
                                           std::optional<crow::json::wvalue> details) {
    crow::json::wvalue json;
    json["error"] = "BadRequest";
    json["message"] = message;
    
    if (details.has_value()) {
        json["details"] = std::move(details.value());
    }
    
    crow::response res(400);
    res.set_header("Content-Type", "application/json");
    res.body = json.dump();
    return res;
}

crow::response ResponseBuilder::unauthorized(const std::string& message) {
    crow::json::wvalue json;
    json["error"] = "Unauthorized";
    json["message"] = message;
    
    crow::response res(401);
    res.set_header("Content-Type", "application/json");
    res.body = json.dump();
    return res;
}

crow::response ResponseBuilder::forbidden(const std::string& message) {
    crow::json::wvalue json;
    json["error"] = "Forbidden";
    json["message"] = message;
    
    crow::response res(403);
    res.set_header("Content-Type", "application/json");
    res.body = json.dump();
    return res;
}

crow::response ResponseBuilder::notFound(const std::string& message) {
    crow::json::wvalue json;
    json["error"] = "NotFound";
    json["message"] = message;
    
    crow::response res(404);
    res.set_header("Content-Type", "application/json");
    res.body = json.dump();
    return res;
}

crow::response ResponseBuilder::conflict(const std::string& message) {
    crow::json::wvalue json;
    json["error"] = "Conflict";
    json["message"] = message;
    
    crow::response res(409);
    res.set_header("Content-Type", "application/json");
    res.body = json.dump();
    return res;
}

crow::response ResponseBuilder::internalError(const std::string& message) {
    crow::json::wvalue json;
    json["error"] = "InternalServerError";
    json["message"] = message;
    
    crow::response res(500);
    res.set_header("Content-Type", "application/json");
    res.body = json.dump();
    return res;
}

crow::response ResponseBuilder::error(int statusCode, const std::string& error,
                                      const std::string& message) {
    crow::json::wvalue json;
    json["error"] = error;
    json["message"] = message;
    
    crow::response res(statusCode);
    res.set_header("Content-Type", "application/json");
    res.body = json.dump();
    return res;
}

} // namespace web_api