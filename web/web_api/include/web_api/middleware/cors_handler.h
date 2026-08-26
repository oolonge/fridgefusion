#ifndef CORS_HANDLER_H
#define CORS_HANDLER_H

#include "crow.h"

namespace web_api {

// CORS Middleware для Crow
struct CORSHandler {
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context& ctx) {
        
    }

    void after_handle(crow::request& req, crow::response& res, context& ctx) {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With, Accept");
        res.add_header("Access-Control-Max-Age", "86400");
    }
};

} // namespace web_api

#endif // CORS_HANDLER_H
