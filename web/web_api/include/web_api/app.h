#ifndef FRIDGEFUSION_APP_H
#define FRIDGEFUSION_APP_H

#include "crow.h"
#include "web_api/middleware/jwt_middleware.h"
#include "web_api/middleware/cors_handler.h"

// Infrastructure
#include "infrastructure/persistence/database/dbContext.h"
#include "infrastructure/persistence/postgresql/userRepository.h"
#include "infrastructure/persistence/postgresql/recipeRepository.h"
#include "infrastructure/persistence/postgresql/ingredientRepository.h"
#include "infrastructure/persistence/postgresql/favoriteRepository.h"
#include "infrastructure/persistence/postgresql/reviewRepository.h"

// Application
#include "application/processors/authProcessor.h"
#include "application/processors/recipeExplorerProcessor.h"
#include "application/processors/userProcessor.h"
#include "application/processors/favoriteProcessor.h"
#include "application/processors/reviewProcessor.h"
#include "application/processors/adminProcessor.h"
#include "application/services/passwordHasher.h"
#include "application/services/recipeFilterService.h"

// Controllers
#include "web_api/controllers/auth_controller.h"
#include "web_api/controllers/recipe_controller.h"
#include "web_api/controllers/user_controller.h"
#include "web_api/controllers/favorite_controller.h"
#include "web_api/controllers/review_controller.h"
#include "web_api/controllers/admin_controller.h"

#include <memory>
#include <string>

namespace web_api {

struct AppConfig {
    std::string host;
    int port;
    int threads;
    
    std::string dbHost;
    int dbPort;
    std::string dbName;
    std::string dbUser;
    std::string dbPassword;
    
    std::string jwtSecret;
    int jwtExpirationHours;
    std::string jwtIssuer;
};

class FridgeFusionApp {
public:
    FridgeFusionApp();
    ~FridgeFusionApp();
    
    // Initialize all components
    void initialize();

    // Setup all routes
    void setupRoutes();
    
    // Run the server
    void run();

private:
    // Configuration
    AppConfig config_;
    
    // Crow application with CORS middleware
    crow::App<CORSHandler> app_;
    
    // Infrastructure Layer
    std::shared_ptr<DbContext> dbContext_;
    std::shared_ptr<IUserRepository> userRepository_;
    std::shared_ptr<IRecipeRepository> recipeRepository_;
    std::shared_ptr<IIngredientRepository> ingredientRepository_;
    std::shared_ptr<IFavoriteRepository> favoriteRepository_;
    std::shared_ptr<IReviewRepository> reviewRepository_;
    
    // Application Layer - Services
    std::shared_ptr<IPasswordHasher> passwordHasher_;
    std::shared_ptr<IRecipeFilterService> recipeFilterService_;
    
    // Application Layer - Processors
    std::shared_ptr<IAuthProcessor> authProcessor_;
    std::shared_ptr<IRecipeExplorerProcessor> recipeProcessor_;
    std::shared_ptr<IUserProcessor> userProcessor_;
    std::shared_ptr<IFavoriteProcessor> favoriteProcessor_;
    std::shared_ptr<IReviewProcessor> reviewProcessor_;
    std::shared_ptr<IAdminProcessor> adminProcessor_;
    
    // Middleware
    std::shared_ptr<JWTMiddleware> jwtMiddleware_;
    
    // Presentation Layer - Controllers
    std::unique_ptr<AuthController> authController_;
    std::unique_ptr<RecipeController> recipeController_;
    std::unique_ptr<UserController> userController_;
    std::unique_ptr<FavoriteController> favoriteController_;
    std::unique_ptr<ReviewController> reviewController_;
    std::unique_ptr<AdminController> adminController_;
    
    // Initialization helpers
    void loadConfigFromAppConfig();
    void initializeInfrastructure();
    void initializeApplication();
    void initializeMiddleware();
    void initializeControllers();
    
    // Route setup helpers
    void setupAuthRoutes();
    void setupRecipeRoutes();
    void setupUserRoutes();
    void setupFavoriteRoutes();
    void setupReviewRoutes();
    void setupAdminRoutes();
    void setupHealthCheck();
    void setupSwaggerUI();
};

} // namespace web_api

#endif // FRIDGEFUSION_APP_H