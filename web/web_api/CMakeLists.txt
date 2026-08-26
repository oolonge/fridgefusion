cmake_minimum_required(VERSION 3.16)
project(fridgefusion_api VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -O2)

set(FRIDGEFUSION_ROOT ${PROJECT_SOURCE_DIR}/..)
set(THIRD_PARTY_DIR ${FRIDGEFUSION_ROOT}/third_party)
set(LIBS_DIR ${FRIDGEFUSION_ROOT}/libs)
set(LIB_DIR ${FRIDGEFUSION_ROOT}/lib)

set(CMAKE_PREFIX_PATH "/opt/homebrew")

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${THIRD_PARTY_DIR}/Crow/include
    ${THIRD_PARTY_DIR}/jwt-cpp/include
    ${THIRD_PARTY_DIR}/json/include
    ${LIBS_DIR}/core/src
    ${LIBS_DIR}/application/src
    ${LIBS_DIR}/infrastructure/src
    ${LIBS_DIR}/utils/src   
)

find_package(OpenSSL REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found. Install with: brew install boost")
endif()

include_directories(${Boost_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES 
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(fridgefusion_api ${SOURCES})

target_link_libraries(fridgefusion_api
    PRIVATE
    ${LIB_DIR}/libutils.a
    ${LIB_DIR}/libapplication.a
    ${LIB_DIR}/libinfrastructure.a
    ${LIB_DIR}/libcore.a
    Qt6::Core     # Qt6 вместо Qt5
    Qt6::Sql      # Qt6 вместо Qt5
    OpenSSL::SSL
    OpenSSL::Crypto
    PostgreSQL::PostgreSQL
    Threads::Threads
    pthread
    z
)

if(APPLE)
    target_link_libraries(fridgefusion_api PRIVATE "-framework CoreFoundation")
endif()

# Копируем корневой config.json в директорию сборки
configure_file(
    ${FRIDGEFUSION_ROOT}/config.json
    ${CMAKE_CURRENT_BINARY_DIR}/config.json
    COPYONLY
)

file(COPY ${PROJECT_SOURCE_DIR}/swagger_ui
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/logs)

install(TARGETS fridgefusion_api
    RUNTIME DESTINATION bin
)