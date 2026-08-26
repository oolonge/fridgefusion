#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>

namespace cli {

// Структура HTTP-ответа
struct HttpResponse {
  int statusCode;
  std::string body;
  bool success;
};

// HTTP-клиент для работы с REST API
class HttpClient {
 public:
  explicit HttpClient(const std::string& baseUrl);
  ~HttpClient();

  // HTTP методы
  HttpResponse get(const std::string& endpoint, const std::string& token = "");
  HttpResponse post(const std::string& endpoint, const std::string& body,
                    const std::string& token = "");
  HttpResponse put(const std::string& endpoint, const std::string& body,
                   const std::string& token = "");
  HttpResponse deleteRequest(const std::string& endpoint,
                              const std::string& token = "");

 private:
  std::string baseUrl_;

  // Вспомогательный метод для выполнения запросов
  HttpResponse performRequest(const std::string& url, const std::string& method,
                               const std::string& body = "",
                               const std::string& token = "");
};

}  // namespace cli

#endif  // HTTP_CLIENT_H
