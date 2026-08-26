#include "cli/config_manager.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

using json = nlohmann::json;

namespace cli {

ConfigManager::ConfigManager()
    : token_(""),
      apiUrl_("http://localhost:8080/api/v2"),
      userId_(-1),
      verbose_(false) {
  configPath_ = getConfigPath();
}

ConfigManager::~ConfigManager() {}

std::string ConfigManager::getConfigPath() const {
  const char* home = getenv("HOME");
  if (!home) {
    home = getpwuid(getuid())->pw_dir;
  }
  return std::string(home) + "/.config/fridgefusion/cli.json";
}

bool ConfigManager::load() {
  std::ifstream file(configPath_);
  if (!file.is_open()) {
    // Файл не существует - создадим дефолтную конфигурацию
    return save();
  }

  try {
    json j;
    file >> j;

    if (j.contains("api_url")) {
      apiUrl_ = j["api_url"].get<std::string>();
    }
    if (j.contains("token")) {
      token_ = j["token"].get<std::string>();
    }
    if (j.contains("user_id")) {
      userId_ = j["user_id"].get<int>();
    }
    if (j.contains("verbose")) {
      verbose_ = j["verbose"].get<bool>();
    }

    return true;
  } catch (const std::exception& e) {
    std::cerr << "Ошибка чтения конфигурации: " << e.what() << std::endl;
    return false;
  }
}

bool ConfigManager::save() {
  // Создаем директорию, если её нет
  std::string dir = configPath_.substr(0, configPath_.find_last_of('/'));
  struct stat st;
  if (stat(dir.c_str(), &st) != 0) {
    std::string mkdirCmd = "mkdir -p " + dir;
    system(mkdirCmd.c_str());
  }

  try {
    json j;
    j["api_url"] = apiUrl_;
    j["token"] = token_;
    j["user_id"] = userId_;
    j["verbose"] = verbose_;

    std::ofstream file(configPath_);
    if (!file.is_open()) {
      std::cerr << "Не удалось открыть файл конфигурации для записи"
                << std::endl;
      return false;
    }

    file << j.dump(2);
    return true;
  } catch (const std::exception& e) {
    std::cerr << "Ошибка сохранения конфигурации: " << e.what() << std::endl;
    return false;
  }
}

std::string ConfigManager::getToken() const { return token_; }

std::string ConfigManager::getApiUrl() const { return apiUrl_; }

int ConfigManager::getUserId() const { return userId_; }

bool ConfigManager::isVerbose() const { return verbose_; }

void ConfigManager::setToken(const std::string& token) { token_ = token; }

void ConfigManager::setUserId(int userId) { userId_ = userId; }

void ConfigManager::setVerbose(bool verbose) { verbose_ = verbose; }

}  // namespace cli
