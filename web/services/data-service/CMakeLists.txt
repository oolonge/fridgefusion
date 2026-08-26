cmake_minimum_required(VERSION 3.16)
project(data_service VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -O2)

set(FRIDGEFUSION_ROOT ${PROJECT_SOURCE_DIR}/../..)
set(THIRD_PARTY_DIR ${FRIDGEFUSION_ROOT}/third_party)
set(LIBS_DIR ${FRIDGEFUSION_ROOT}/libs)
set(LIB_DIR ${FRIDGEFUSION_ROOT}/lib)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${THIRD_PARTY_DIR}/Crow/include
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

include_directories(${Boost_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(data_service ${SOURCES})

target_link_libraries(data_service
    PRIVATE
    ${LIB_DIR}/libapplication.a
    ${LIB_DIR}/libinfrastructure.a
    ${LIB_DIR}/libutils.a
    ${LIB_DIR}/libcore.a
    Qt6::Core
    Qt6::Sql
    OpenSSL::SSL
    OpenSSL::Crypto
    PostgreSQL::PostgreSQL
    Threads::Threads
    pthread
    z
)

if(APPLE)
    target_link_libraries(data_service PRIVATE "-framework CoreFoundation")
endif()

# Copy config
configure_file(
    ${FRIDGEFUSION_ROOT}/config.json
    ${CMAKE_CURRENT_BINARY_DIR}/config.json
    COPYONLY
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/logs)

install(TARGETS data_service RUNTIME DESTINATION bin)
