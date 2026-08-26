Да, JWT-авторизация полностью реализована. Вот как это работает:

JWT-авторизация в проекте
Основные файлы
| Компонент | Файл | Назначение | |-----------|------|------------| | Middleware | web_api/include/web_api/middleware/jwt_middleware.h | Заголовок JWT middleware | | | web_api/src/middleware/jwt_middleware.cpp | Реализация генерации/валидации токенов | | Конфигурация | libs/infrastructure/src/infrastructure/config/appConfig.h | JWT secret, issuer, expiration | | Auth Controller | web_api/src/controllers/auth_controller.cpp | Login/Register/Logout endpoints |

Как работает
1. Генерация токена (при логине)
// jwt_middleware.cpp
std::string JWTMiddleware::generateToken(const User& user) {
    auto token = jwt::create()
        .set_issuer(issuer_)
        .set_payload_claim("user_id", jwt::claim(std::to_string(user.getId())))
        .set_payload_claim("username", jwt::claim(user.getUsername().toStdString()))
        .set_payload_claim("email", jwt::claim(user.getEmail().toStdString()))
        .set_payload_claim("role", jwt::claim(roleToString(user.getRole())))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours{24})
        .sign(jwt::algorithm::hs256{secret_});
    return token;
}
2. Валидация токена (при каждом запросе)
// jwt_middleware.cpp
std::optional<JWTPayload> JWTMiddleware::validateToken(const std::string& token) {
    auto verifier = jwt::verify()
        .allow_algorithm(jwt::algorithm::hs256{secret_})
        .with_issuer(issuer_);
    
    auto decoded = jwt::decode(token);
    verifier.verify(decoded);
    
    // Извлечение payload: user_id, username, email, role
}
3. Проверка в контроллерах
// Пример из recipe_controller.cpp
crow::response RecipeController::createRecipe(const crow::request& req) {
    // Проверка авторизации
    if (!jwtMiddleware_->isAuthenticated(req)) {
        return ResponseBuilder::unauthorized();  // 401
    }
    
    auto userId = jwtMiddleware_->getUserId(req);
    // ... бизнес-логика
}
Защищённые vs Публичные endpoints
Публичные (без токена):
POST /api/v2/auth/login - вход
POST /api/v2/auth/register - регистрация
GET /api/v2/recipes - список рецептов
GET /api/v2/recipes/{id} - детали рецепта
GET /api/v2/ingredients - список ингредиентов
GET /api/v2/categories - список категорий
GET /api/v2/health - health check
Требуют авторизации (Bearer token):
POST /api/v2/auth/logout - выход
GET /api/v2/users/me - профиль
PUT /api/v2/users/me - обновление профиля
GET /api/v2/favorites - избранное
POST /api/v2/favorites - добавить в избранное
DELETE /api/v2/favorites/{id} - удалить из избранного
POST /api/v2/recipes/{id}/reviews - добавить отзыв
PUT/DELETE /api/v2/reviews/{id} - изменить/удалить отзыв
Требуют роли ADMIN:
GET /api/v2/admin/users - все пользователи
PATCH /api/v2/admin/users/{id}/role - изменить роль
DELETE /api/v2/admin/users/{id} - удалить пользователя
Конфигурация (config.json)
{
  "jwt": {
    "secret": "your-secret-key",
    "expirationHours": 24,
    "issuer": "FridgeFusion API v2.0"
  }
}
Использование в CLI
CLI сохраняет токен в ~/.config/fridgefusion/cli.json и добавляет его в заголовок:

Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
Ключевые методы JWTMiddleware
| Метод | Описание | |-------|----------| | generateToken(user) | Создаёт JWT токен | | validateToken(token) | Проверяет и декодирует токен | | isAuthenticated(req) | Проверяет наличие валидного токена | | getUserId(req) | Извлекает user_id из токена | | hasRole(req, role) | Проверяет роль пользователя |

Вывод: Бонус #1 полностью реализован. JWT-авторизация работает с разграничением доступа для публичных, авторизованных и админских endpoints.