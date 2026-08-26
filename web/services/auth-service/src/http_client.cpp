#include "http_client.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

HttpClient::HttpClient(const std::string& baseUrl) : baseUrl_(baseUrl) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

std::string HttpClient::doRequest(const std::string& method, const std::string& path, const std::string& body) {
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
        } else if (method == "PUT") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        } else if (method == "DELETE") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        } else if (method == "PATCH") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        }

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "HTTP request failed: " << curl_easy_strerror(res) << std::endl;
            response = "";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return response;
}

std::optional<json> HttpClient::get(const std::string& path) {
    std::string response = doRequest("GET", path);
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> HttpClient::post(const std::string& path, const json& body) {
    std::string response = doRequest("POST", path, body.dump());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> HttpClient::put(const std::string& path, const json& body) {
    std::string response = doRequest("PUT", path, body.dump());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

bool HttpClient::del(const std::string& path) {
    std::string response = doRequest("DELETE", path);
    return true; // 204 No Content returns empty
}

std::optional<json> HttpClient::patch(const std::string& path, const json& body) {
    std::string response = doRequest("PATCH", path, body.dump());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}
