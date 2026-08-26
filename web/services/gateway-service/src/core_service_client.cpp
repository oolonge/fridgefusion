#include "core_service_client.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

CoreServiceClient::CoreServiceClient(const std::string& coreServiceUrl) : baseUrl_(coreServiceUrl) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

std::string CoreServiceClient::doRequest(const std::string& method, const std::string& path, const std::string& body) {
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

// ==================== AUTH OPERATIONS ====================

std::optional<json> CoreServiceClient::registerUser(const json& userData) {
    std::string response = doRequest("POST", "/api/core/auth/register", userData.dump());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> CoreServiceClient::loginUser(const std::string& email, const std::string& password) {
    json body = {{"email", email}, {"password", password}};
    std::string response = doRequest("POST", "/api/core/auth/login", body.dump());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> CoreServiceClient::getUserById(int userId) {
    std::string response = doRequest("GET", "/api/core/auth/user/" + std::to_string(userId));
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

// ==================== RECIPE OPERATIONS ====================

std::optional<json> CoreServiceClient::getRecipes(int limit, int offset, const std::string& category, const std::string& search) {
    std::stringstream path;
    path << "/api/core/recipes?limit=" << limit << "&offset=" << offset;
    if (!category.empty()) path << "&category=" << category;
    if (!search.empty()) path << "&search=" << search;

    std::string response = doRequest("GET", path.str());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> CoreServiceClient::getRecipeById(int recipeId) {
    std::string response = doRequest("GET", "/api/core/recipes/" + std::to_string(recipeId));
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> CoreServiceClient::createRecipe(const json& recipeData) {
    std::string response = doRequest("POST", "/api/core/recipes", recipeData.dump());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

bool CoreServiceClient::updateRecipe(int recipeId, const json& recipeData) {
    std::string response = doRequest("PUT", "/api/core/recipes/" + std::to_string(recipeId), recipeData.dump());
    return !response.empty();
}

bool CoreServiceClient::deleteRecipe(int recipeId) {
    doRequest("DELETE", "/api/core/recipes/" + std::to_string(recipeId));
    return true;
}

// ==================== CATEGORY & INGREDIENT OPERATIONS ====================

std::optional<json> CoreServiceClient::getCategories() {
    std::string response = doRequest("GET", "/api/core/categories");
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> CoreServiceClient::getIngredients() {
    std::string response = doRequest("GET", "/api/core/ingredients");
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> CoreServiceClient::getIngredientSubstitutes(int ingredientId, int limit) {
    std::stringstream path;
    path << "/api/core/ingredients/" << ingredientId << "/substitutes?limit=" << limit;
    std::string response = doRequest("GET", path.str());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

// ==================== USER PROFILE OPERATIONS ====================

std::optional<json> CoreServiceClient::getUserProfile(int userId) {
    std::string response = doRequest("GET", "/api/core/users/" + std::to_string(userId) + "/profile");
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

bool CoreServiceClient::updateUser(int userId, const json& userData) {
    std::string response = doRequest("PUT", "/api/core/users/" + std::to_string(userId), userData.dump());
    return !response.empty();
}

bool CoreServiceClient::deleteUser(int userId) {
    doRequest("DELETE", "/api/core/users/" + std::to_string(userId));
    return true;
}

// ==================== FAVORITE OPERATIONS ====================

std::optional<json> CoreServiceClient::getUserFavorites(int userId) {
    std::string response = doRequest("GET", "/api/core/users/" + std::to_string(userId) + "/favorites");
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

bool CoreServiceClient::addFavorite(int userId, int recipeId) {
    json body = {{"recipe_id", recipeId}};
    std::string response = doRequest("POST", "/api/core/users/" + std::to_string(userId) + "/favorites", body.dump());
    return !response.empty();
}

bool CoreServiceClient::removeFavorite(int userId, int recipeId) {
    doRequest("DELETE", "/api/core/users/" + std::to_string(userId) + "/favorites/" + std::to_string(recipeId));
    return true;
}

// ==================== REVIEW OPERATIONS ====================

std::optional<json> CoreServiceClient::getRecipeReviews(int recipeId) {
    std::string response = doRequest("GET", "/api/core/recipes/" + std::to_string(recipeId) + "/reviews");
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<json> CoreServiceClient::createReview(int recipeId, const json& reviewData) {
    std::string response = doRequest("POST", "/api/core/recipes/" + std::to_string(recipeId) + "/reviews", reviewData.dump());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

bool CoreServiceClient::updateReview(int recipeId, int reviewId, const json& reviewData) {
    std::string response = doRequest("PUT", "/api/core/recipes/" + std::to_string(recipeId) + "/reviews/" + std::to_string(reviewId), reviewData.dump());
    return !response.empty();
}

bool CoreServiceClient::deleteReview(int reviewId) {
    doRequest("DELETE", "/api/core/reviews/" + std::to_string(reviewId));
    return true;
}

// ==================== ADMIN OPERATIONS ====================

std::optional<json> CoreServiceClient::getAllUsers(int limit, int offset) {
    std::stringstream path;
    path << "/api/core/admin/users?limit=" << limit << "&offset=" << offset;
    std::string response = doRequest("GET", path.str());
    if (response.empty()) return std::nullopt;
    try {
        return json::parse(response);
    } catch (...) {
        return std::nullopt;
    }
}

bool CoreServiceClient::updateUserRole(int userId, int role) {
    json body = {{"role", role}};
    std::string response = doRequest("PATCH", "/api/core/admin/users/" + std::to_string(userId) + "/role", body.dump());
    return !response.empty();
}

bool CoreServiceClient::adminDeleteUser(int userId) {
    doRequest("DELETE", "/api/core/admin/users/" + std::to_string(userId));
    return true;
}
