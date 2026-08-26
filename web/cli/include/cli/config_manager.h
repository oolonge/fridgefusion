#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>

namespace cli {

// Управление конфигурацией приложения
class ConfigManager {
 public:
  ConfigManager();
  ~ConfigManager();

  // Загрузка и сохранение конфигурации
  bool load();
  bool save();

  // Геттеры
  std::string getToken() const;
  std::string getApiUrl() const;
  int getUserId() const;
  bool isVerbose() const;

  // Сеттеры
  void setToken(const std::string& token);
  void setUserId(int userId);
  void setVerbose(bool verbose);

 private:
  std::string configPath_;
  std::string token_;
  std::string apiUrl_;
  int userId_;
  bool verbose_;

  // Получение пути к конфигурационному файлу
  std::string getConfigPath() const;
};

}  // namespace cli

#endif  // CONFIG_MANAGER_H
