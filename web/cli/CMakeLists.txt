cmake_minimum_required(VERSION 3.15)
project(fridgefusion_cli CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Зависимости
find_package(CURL REQUIRED)

# nlohmann_json - попытка найти через CMake config, иначе используем системный header
find_package(nlohmann_json 3.2.0 QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json CMake config not found, using system headers")
endif()

# Исходники
set(CLI_SOURCES
    src/main.cpp
    src/cli_app.cpp
    src/http_client.cpp
    src/config_manager.cpp
    src/auth_handler.cpp
    src/recipe_handler.cpp
    src/favorite_handler.cpp
    src/user_handler.cpp
    src/console_view.cpp
)

# Исполняемый файл
add_executable(ff ${CLI_SOURCES})

target_include_directories(ff PRIVATE include)

# Линковка зависимостей
target_link_libraries(ff PRIVATE CURL::libcurl)

# Добавление nlohmann_json если найден через CMake
if(nlohmann_json_FOUND)
    target_link_libraries(ff PRIVATE nlohmann_json::nlohmann_json)
endif()

# Установка (опционально)
install(TARGETS ff DESTINATION bin)
