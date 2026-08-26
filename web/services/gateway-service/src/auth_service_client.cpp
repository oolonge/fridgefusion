#include "auth_service_client.h"
#include <curl/curl.h>
#include <iostream>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

AuthServiceClient::AuthServiceClient(const std::string& baseUrl) : baseUrl_(baseUrl) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

std::string AuthServiceClient::doRequest(const std::string& method, const std::string& path, const std::string& body) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string url = baseUrl_ + path;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        }

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Auth Service request failed: " << curl_easy_strerror(res) << std::endl;
            response = "";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return response;
}

std::optional<json> AuthServiceClient::registerUser(const json& userData) {
    std::string response = doRequest("POST", "/auth/register", userData.dump());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> AuthServiceClient::loginUser(const std::string& email, const std::string& password) {
    json body = {
        {"email", email},
        {"password", password}
    };
    std::string response = doRequest("POST", "/auth/login", body.dump());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> AuthServiceClient::verifyToken(const std::string& token) {
    json body = {{"token", token}};
    std::string response = doRequest("POST", "/auth/verify", body.dump());
    if (response.empty()) return std::nullopt;
    try {
        auto result = json::parse(response);
        if (result.contains("valid") && result["valid"].get<bool>()) {
            return result["user"];
        }
        return std::nullopt;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> AuthServiceClient::refreshToken(const std::string& refreshToken) {
    json body = {{"refresh_token", refreshToken}};
    std::string response = doRequest("POST", "/auth/refresh", body.dump());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

bool AuthServiceClient::logout() {
    std::string response = doRequest("POST", "/auth/logout");
    return !response.empty();
}
