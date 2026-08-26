#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>
#include <optional>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class HttpClient {
public:
    HttpClient(const std::string& baseUrl);

    std::optional<json> get(const std::string& path);
    std::optional<json> post(const std::string& path, const json& body);
    std::optional<json> put(const std::string& path, const json& body);
    bool del(const std::string& path);
    std::optional<json> patch(const std::string& path, const json& body);

private:
    std::string baseUrl_;
    std::string doRequest(const std::string& method, const std::string& path, const std::string& body = "");
};

#endif // HTTP_CLIENT_H
