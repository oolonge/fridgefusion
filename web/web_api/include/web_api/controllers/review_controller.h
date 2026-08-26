#ifndef REVIEW_CONTROLLER_H
#define REVIEW_CONTROLLER_H

#include "crow.h"
#include "application/interfaces/processors/IReviewProcessor.h"
#include "web_api/middleware/jwt_middleware.h"
#include <memory>

namespace web_api {

class ReviewController {
public:
    ReviewController(std::shared_ptr<IReviewProcessor> reviewProcessor,
                     std::shared_ptr<JWTMiddleware> jwtMiddleware);
    
    // GET /recipes/{recipe_id}/reviews
    crow::response getRecipeReviews(const crow::request& req, int recipeId);
    
    // POST /recipes/{recipe_id}/reviews
    crow::response createReview(const crow::request& req, int recipeId);
    
    // PUT /recipes/{recipe_id}/reviews
    crow::response updateReview(const crow::request& req, int recipeId);
    
    // DELETE /reviews/{id}
    crow::response deleteReview(const crow::request& req, int id);

private:
    std::shared_ptr<IReviewProcessor> reviewProcessor_;
    std::shared_ptr<JWTMiddleware> jwtMiddleware_;
};

} // namespace web_api

#endif // REVIEW_CONTROLLER_H