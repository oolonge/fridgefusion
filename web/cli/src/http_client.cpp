#include "cli/http_client.h"

#include <curl/curl.h>

#include <iostream>

namespace cli {

// Callback для записи данных ответа
static size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                             void* userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

HttpClient::HttpClient(const std::string& baseUrl) : baseUrl_(baseUrl) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

HttpClient::~HttpClient() { curl_global_cleanup(); }

HttpResponse HttpClient::get(const std::string& endpoint,
                              const std::string& token) {
  return performRequest(baseUrl_ + endpoint, "GET", "", token);
}

HttpResponse HttpClient::post(const std::string& endpoint,
                               const std::string& body,
                               const std::string& token) {
  return performRequest(baseUrl_ + endpoint, "POST", body, token);
}

HttpResponse HttpClient::put(const std::string& endpoint,
                              const std::string& body,
                              const std::string& token) {
  return performRequest(baseUrl_ + endpoint, "PUT", body, token);
}

HttpResponse HttpClient::deleteRequest(const std::string& endpoint,
                                        const std::string& token) {
  return performRequest(baseUrl_ + endpoint, "DELETE", "", token);
}

HttpResponse HttpClient::performRequest(const std::string& url,
                                         const std::string& method,
                                         const std::string& body,
                                         const std::string& token) {
  HttpResponse response;
  response.success = false;
  response.statusCode = 0;

  CURL* curl = curl_easy_init();
  if (!curl) {
    response.body = "Failed to initialize CURL";
    return response;
  }

  std::string responseString;
  struct curl_slist* headers = nullptr;

  // Установка заголовков
  headers = curl_slist_append(headers, "Content-Type: application/json");
  if (!token.empty()) {
    std::string authHeader = "Authorization: Bearer " + token;
    headers = curl_slist_append(headers, authHeader.c_str());
  }

  // Настройка CURL
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

  // Установка метода
  if (method == "POST") {
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
  } else if (method == "PUT") {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
  } else if (method == "DELETE") {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
  }

  // Выполнение запроса
  CURLcode res = curl_easy_perform(curl);

  if (res == CURLE_OK) {
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    response.statusCode = static_cast<int>(httpCode);
    response.body = responseString;
    response.success = (httpCode >= 200 && httpCode < 300);
  } else {
    response.body = curl_easy_strerror(res);
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  return response;
}

}  // namespace cli
