cmake_minimum_required(VERSION 3.16)
project(gateway_service VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -O2)

set(FRIDGEFUSION_ROOT ${PROJECT_SOURCE_DIR}/../..)
set(THIRD_PARTY_DIR ${FRIDGEFUSION_ROOT}/third_party)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${THIRD_PARTY_DIR}/Crow/include
    ${THIRD_PARTY_DIR}/json/include
    ${THIRD_PARTY_DIR}/jwt-cpp/include
)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(gateway_service ${SOURCES})

target_link_libraries(gateway_service
    PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    CURL::libcurl
    pthread
)

if(APPLE)
    target_link_libraries(gateway_service PRIVATE "-framework CoreFoundation")
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/logs)

install(TARGETS gateway_service RUNTIME DESTINATION bin)
