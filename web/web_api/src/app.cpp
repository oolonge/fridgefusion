#include "web_api/app.h"
#include "web_api/utils/response_builder.h"
#include "infrastructure/config/appConfig.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <locale>
#include <clocale>

namespace web_api {

FridgeFusionApp::FridgeFusionApp() {
    // Критически важно: устанавливаем C-локаль для чисел
    // Это гарантирует, что double будет сериализован с точкой, а не запятой
    std::setlocale(LC_NUMERIC, "C");
    std::locale::global(std::locale::classic());
}

FridgeFusionApp::~FridgeFusionApp() {
}

void FridgeFusionApp::initialize() {
    std::cout << "Initializing FridgeFusion API..." << std::endl;

    loadConfigFromAppConfig();
    initializeInfrastructure();
    initializeApplication();
    initializeMiddleware();
    initializeControllers();

    std::cout << "Initialization complete!" << std::endl;
}

void FridgeFusionApp::loadConfigFromAppConfig() {
    std::cout << "Loading configuration from AppConfig..." << std::endl;

    // Используем глобальный AppConfig singleton (явно указываем ::AppConfig)
    ::AppConfig& appConfig = ::AppConfig::getInstance();

    // API Server settings
    config_.host = appConfig.getApiHost().toStdString();
    config_.port = appConfig.getApiPort();
    config_.threads = appConfig.getApiThreads();

    // Database settings (используются из AppConfig, но не используются напрямую в web_api)
    config_.dbHost = appConfig.getDbHost().toStdString();
    config_.dbPort = appConfig.getDbPort();
    config_.dbName = appConfig.getDbName().toStdString();
    config_.dbUser = appConfig.getDbUsername().toStdString();
    config_.dbPassword = appConfig.getDbPassword().toStdString();

    // JWT settings
    config_.jwtSecret = appConfig.getJwtSecret().toStdString();
    config_.jwtExpirationHours = appConfig.getJwtExpirationHours();
    config_.jwtIssuer = appConfig.getJwtIssuer().toStdString();

    std::cout << "Configuration loaded from " << appConfig.getConfigFilePath().toStdString() << std::endl;
    std::cout << "  - API Server: " << config_.host << ":" << config_.port << std::endl;
    std::cout << "  - Database: " << config_.dbName << " @ " << config_.dbHost << ":" << config_.dbPort << std::endl;
    std::cout << "  - JWT Issuer: " << config_.jwtIssuer << std::endl;
}

void FridgeFusionApp::initializeInfrastructure() {
    std::cout << "Initializing infrastructure layer..." << std::endl;
    
    // Initialize database context
    dbContext_ = std::make_shared<DbContext>();
    
    // Initialize repositories
    userRepository_ = std::make_shared<UserRepository>(dbContext_);
    recipeRepository_ = std::make_shared<RecipeRepository>(dbContext_);
    ingredientRepository_ = std::make_shared<IngredientRepository>(dbContext_);
    favoriteRepository_ = std::make_shared<FavoriteRepository>(dbContext_.get());
    reviewRepository_ = std::make_shared<ReviewRepository>(dbContext_.get());
    
    std::cout << "Infrastructure layer initialized" << std::endl;
}

void FridgeFusionApp::initializeApplication() {
    std::cout << "Initializing application layer..." << std::endl;
    
    // Initialize services
    passwordHasher_ = std::make_shared<PasswordHasher>();
    recipeFilterService_ = std::make_shared<RecipeFilterService>();
    
    // Initialize processors
    authProcessor_ = std::make_shared<AuthProcessor>(userRepository_, passwordHasher_);
    recipeProcessor_ = std::make_shared<RecipeExplorerProcessor>(
        recipeRepository_, ingredientRepository_, recipeFilterService_
    );
    
    // UserProcessor requires 3 repositories
    userProcessor_ = std::make_shared<UserProcessor>(
        userRepository_, 
        favoriteRepository_, 
        reviewRepository_
    );
    
    favoriteProcessor_ = std::make_shared<FavoriteProcessor>(favoriteRepository_.get());
    reviewProcessor_ = std::make_shared<ReviewProcessor>(reviewRepository_.get());
    
    // AdminProcessor requires 4 repositories
    adminProcessor_ = std::make_shared<AdminProcessor>(
        userRepository_,
        favoriteRepository_,
        reviewRepository_,
        recipeRepository_
    );
    
    std::cout << "Application layer initialized" << std::endl;
}

void FridgeFusionApp::initializeMiddleware() {
    std::cout << "Initializing middleware..." << std::endl;
    
    jwtMiddleware_ = std::make_shared<JWTMiddleware>(config_.jwtSecret, config_.jwtIssuer);
    
    std::cout << "Middleware initialized" << std::endl;
}

void FridgeFusionApp::initializeControllers() {
    std::cout << "Initializing controllers..." << std::endl;
    
    authController_ = std::make_unique<AuthController>(authProcessor_, jwtMiddleware_);
    recipeController_ = std::make_unique<RecipeController>(recipeProcessor_, jwtMiddleware_);
    userController_ = std::make_unique<UserController>(userProcessor_, jwtMiddleware_);
    favoriteController_ = std::make_unique<FavoriteController>(favoriteProcessor_, recipeProcessor_, jwtMiddleware_);
    reviewController_ = std::make_unique<ReviewController>(reviewProcessor_, jwtMiddleware_);
    adminController_ = std::make_unique<AdminController>(adminProcessor_, jwtMiddleware_);
    
    std::cout << "Controllers initialized" << std::endl;
}

void FridgeFusionApp::setupRoutes() {
    std::cout << "Setting up routes..." << std::endl;
    
    // Universal OPTIONS handler for preflight requests
    CROW_ROUTE(app_, "/<path>")
    .methods("OPTIONS"_method)
    ([](const crow::request&, std::string) {
        return crow::response(204);
    });
    
    setupHealthCheck();
    setupSwaggerUI();
    setupAuthRoutes();
    setupRecipeRoutes();
    setupUserRoutes();
    setupFavoriteRoutes();
    setupReviewRoutes();
    setupAdminRoutes();
    
    std::cout << "Routes configured" << std::endl;
}

void FridgeFusionApp::setupHealthCheck() {
    CROW_ROUTE(app_, "/api/v2/health")
    ([]() -> crow::response {
        crow::json::wvalue response;
        response["status"] = "ok";
        response["service"] = "FridgeFusion API";
        response["version"] = "2.0.0";
        return ResponseBuilder::ok(response);
    });
}

void FridgeFusionApp::setupSwaggerUI() {
    std::cout << "Setting up Swagger UI..." << std::endl;
    
    // Redirect /api/docs to /swagger_ui/index.html
    CROW_ROUTE(app_, "/api/docs")
    ([]() {
        crow::response res(307);
        res.set_header("Location", "/swagger_ui/index.html");
        return res;
    });
    
    // Serve static files from swagger_ui directory
    CROW_ROUTE(app_, "/swagger_ui/<path>")
    ([](std::string path) {
        // Construct file path
        std::string filepath = "swagger_ui/" + path;
        
        // Read file
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            return crow::response(404, "File not found");
        }
        
        // Read content
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        
        // Determine content type based on file extension
        std::string content_type = "text/plain";
        
        // Helper lambda for checking suffix (C++17 compatible)
        auto ends_with = [](const std::string& str, const std::string& suffix) {
            if (suffix.size() > str.size()) return false;
            return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        };
        
        if (ends_with(path, ".html")) {
            content_type = "text/html";
        } else if (ends_with(path, ".css")) {
            content_type = "text/css";
        } else if (ends_with(path, ".js")) {
            content_type = "application/javascript";
        } else if (ends_with(path, ".json")) {
            content_type = "application/json";
        } else if (ends_with(path, ".png")) {
            content_type = "image/png";
        } else if (ends_with(path, ".jpg") || ends_with(path, ".jpeg")) {
            content_type = "image/jpeg";
        } else if (ends_with(path, ".svg")) {
            content_type = "image/svg+xml";
        } else if (ends_with(path, ".ico")) {
            content_type = "image/x-icon";
        } else if (ends_with(path, ".yaml") || ends_with(path, ".yml")) {
            content_type = "text/yaml";
        }
        
        crow::response res(200, content);
        res.set_header("Content-Type", content_type);
        return res;
    });
    
    std::cout << "Swagger UI configured at /api/docs" << std::endl;
}

void FridgeFusionApp::setupAuthRoutes() {
    // POST /api/v2/auth/register
    CROW_ROUTE(app_, "/api/v2/auth/register")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        return authController_->registerUser(req);
    });
    
    // POST /api/v2/auth/login
    CROW_ROUTE(app_, "/api/v2/auth/login")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        return authController_->login(req);
    });
    
    // POST /api/v2/auth/logout
    CROW_ROUTE(app_, "/api/v2/auth/logout")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        return authController_->logout(req);
    });
}

void FridgeFusionApp::setupRecipeRoutes() {
    // GET /api/v2/recipes
    CROW_ROUTE(app_, "/api/v2/recipes")
    .methods("GET"_method)
    ([this](const crow::request& req) {
        return recipeController_->getRecipes(req);
    });
    
    // POST /api/v2/recipes
    CROW_ROUTE(app_, "/api/v2/recipes")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        return recipeController_->createRecipe(req);
    });
    
    // GET /api/v2/recipes/<id>
    CROW_ROUTE(app_, "/api/v2/recipes/<int>")
    .methods("GET"_method)
    ([this](const crow::request& req, int id) {
        return recipeController_->getRecipeById(req, id);
    });
    
    // PUT /api/v2/recipes/<id>
    CROW_ROUTE(app_, "/api/v2/recipes/<int>")
    .methods("PUT"_method)
    ([this](const crow::request& req, int id) {
        return recipeController_->updateRecipe(req, id);
    });
    
    // PATCH /api/v2/recipes/<id>
    CROW_ROUTE(app_, "/api/v2/recipes/<int>")
    .methods("PATCH"_method)
    ([this](const crow::request& req, int id) {
        return recipeController_->patchRecipe(req, id);
    });
    
    // DELETE /api/v2/recipes/<id>
    CROW_ROUTE(app_, "/api/v2/recipes/<int>")
    .methods("DELETE"_method)
    ([this](const crow::request& req, int id) {
        return recipeController_->deleteRecipe(req, id);
    });

    // ==================== INGREDIENT ROUTES ====================
    
    // GET /api/v2/ingredients
    CROW_ROUTE(app_, "/api/v2/ingredients")
    .methods("GET"_method)
    ([this](const crow::request& req) {
        return recipeController_->getIngredients(req);
    });
    
    // GET /api/v2/ingredients/<id>/substitutes
    CROW_ROUTE(app_, "/api/v2/ingredients/<int>/substitutes")
    .methods("GET"_method)
    ([this](const crow::request& req, int id) {
        return recipeController_->getIngredientSubstitutes(req, id);
    });
    
    // ==================== CATEGORY ROUTES ====================
    
    // GET /api/v2/categories
    CROW_ROUTE(app_, "/api/v2/categories")
    .methods("GET"_method)
    ([this](const crow::request& req) {
        return recipeController_->getCategories(req);
    });
}

void FridgeFusionApp::setupUserRoutes() {
    // GET /api/v2/users/me
    CROW_ROUTE(app_, "/api/v2/users/me")
    .methods("GET"_method)
    ([this](const crow::request& req) {
        return userController_->getProfile(req);
    });
    
    // PUT /api/v2/users/me
    CROW_ROUTE(app_, "/api/v2/users/me")
    .methods("PUT"_method)
    ([this](const crow::request& req) {
        return userController_->updateProfile(req);
    });
    
    // DELETE /api/v2/users/me
    CROW_ROUTE(app_, "/api/v2/users/me")
    .methods("DELETE"_method)
    ([this](const crow::request& req) {
        return userController_->deleteAccount(req);
    });
    
    // GET /api/v2/users/<id>
    CROW_ROUTE(app_, "/api/v2/users/<int>")
    .methods("GET"_method)
    ([this](const crow::request& req, int id) {
        return userController_->getUserById(req, id);
    });
}

void FridgeFusionApp::setupFavoriteRoutes() {
    // GET /api/v2/favorites
    CROW_ROUTE(app_, "/api/v2/favorites")
    .methods("GET"_method)
    ([this](const crow::request& req) {
        return favoriteController_->getFavorites(req);
    });
    
    // POST /api/v2/favorites
    CROW_ROUTE(app_, "/api/v2/favorites")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        return favoriteController_->addFavorite(req);
    });
    
    // DELETE /api/v2/favorites/<recipe_id>
    CROW_ROUTE(app_, "/api/v2/favorites/<int>")
    .methods("DELETE"_method)
    ([this](const crow::request& req, int recipeId) {
        return favoriteController_->removeFavorite(req, recipeId);
    });
}

void FridgeFusionApp::setupReviewRoutes() {
    // GET /api/v2/recipes/<recipe_id>/reviews
    CROW_ROUTE(app_, "/api/v2/recipes/<int>/reviews")
    .methods("GET"_method)
    ([this](const crow::request& req, int recipeId) {
        return reviewController_->getRecipeReviews(req, recipeId);
    });
    
    // POST /api/v2/recipes/<recipe_id>/reviews
    CROW_ROUTE(app_, "/api/v2/recipes/<int>/reviews")
    .methods("POST"_method)
    ([this](const crow::request& req, int recipeId) {
        return reviewController_->createReview(req, recipeId);
    });
    
    // PUT /api/v2/recipes/<recipe_id>/reviews
    CROW_ROUTE(app_, "/api/v2/recipes/<int>/reviews")
    .methods("PUT"_method)
    ([this](const crow::request& req, int recipeId) {
        return reviewController_->updateReview(req, recipeId);
    });
    
    // DELETE /api/v2/reviews/<id>
    CROW_ROUTE(app_, "/api/v2/reviews/<int>")
    .methods("DELETE"_method)
    ([this](const crow::request& req, int id) {
        return reviewController_->deleteReview(req, id);
    });
}

void FridgeFusionApp::setupAdminRoutes() {
    // GET /api/v2/admin/users
    CROW_ROUTE(app_, "/api/v2/admin/users")
    .methods("GET"_method)
    ([this](const crow::request& req) {
        return adminController_->getUsers(req);
    });
    
    // PATCH /api/v2/admin/users/<id>/role
    CROW_ROUTE(app_, "/api/v2/admin/users/<int>/role")
    .methods("PATCH"_method)
    ([this](const crow::request& req, int id) {
        return adminController_->updateUserRole(req, id);
    });
    
    // DELETE /api/v2/admin/users/<id>
    CROW_ROUTE(app_, "/api/v2/admin/users/<int>")
    .methods("DELETE"_method)
    ([this](const crow::request& req, int id) {
        return adminController_->deleteUser(req, id);
    });
}

void FridgeFusionApp::run() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  FridgeFusion REST API v2.0" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Server: http://" << config_.host << ":" << config_.port << std::endl;
    std::cout << "API Base: /api/v2" << std::endl;
    std::cout << "Swagger UI: http://localhost:" << config_.port << "/api/docs" << std::endl;
    std::cout << "Health Check: http://localhost:" << config_.port << "/api/v2/health" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    app_.port(config_.port)
        .multithreaded()
        .concurrency(config_.threads)
        .run();
}

} // namespace web_api