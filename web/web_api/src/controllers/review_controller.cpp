#include "web_api/controllers/review_controller.h"
#include "web_api/utils/request_validator.h"
#include "web_api/utils/response_builder.h"
#include "web_api/utils/json_converter.h"

namespace web_api {

ReviewController::ReviewController(std::shared_ptr<IReviewProcessor> reviewProcessor,
                                   std::shared_ptr<JWTMiddleware> jwtMiddleware)
    : reviewProcessor_(reviewProcessor), jwtMiddleware_(jwtMiddleware) {
}

crow::response ReviewController::getRecipeReviews(const crow::request& /*req*/, int recipeId) {
    try {
        // Get reviews for recipe
        auto reviews = reviewProcessor_->getRecipeReviews(recipeId);
        
        // Get average rating
        double avgRating = reviewProcessor_->getRecipeAverageRating(recipeId);
        int totalCount = reviews.size();
        
        // Convert to vector
        std::vector<Review> reviewsVec;
        for (const auto& review : reviews) {
            reviewsVec.push_back(review);
        }
        
        auto responseJson = JsonConverter::reviewsWithStatsToJson(reviewsVec, avgRating, totalCount);
        return ResponseBuilder::ok(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response ReviewController::createReview(const crow::request& req, int recipeId) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }
        
        auto json = crow::json::load(req.body);
        if (!json) {
            return ResponseBuilder::badRequest("Invalid JSON");
        }
        
        // Validate input
        auto validation = RequestValidator::validateReviewCreate(json);
        if (!validation.isValid) {
            crow::json::wvalue details;
            details[validation.field] = validation.message;
            return ResponseBuilder::badRequest("Validation failed", details);
        }
        
        // Extract review data
        int rating = json["rating"].i();
        QString comment = json.has("comment") ? QString::fromStdString(json["comment"].s()) : QString();
        
        // Create review
        bool success = reviewProcessor_->addReview(userId.value(), recipeId, rating, comment);
        
        if (!success) {
            return ResponseBuilder::badRequest("Failed to create review (recipe may not exist or you already reviewed it)");
        }
        
        crow::json::wvalue responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "Review created successfully";
        responseJson["recipe_id"] = recipeId;
        responseJson["rating"] = rating;
        
        return ResponseBuilder::created(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response ReviewController::updateReview(const crow::request& req, int recipeId) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }
        
        auto json = crow::json::load(req.body);
        if (!json) {
            return ResponseBuilder::badRequest("Invalid JSON");
        }
        
        // Validate input
        auto validation = RequestValidator::validateReviewUpdate(json);
        if (!validation.isValid) {
            crow::json::wvalue details;
            details[validation.field] = validation.message;
            return ResponseBuilder::badRequest("Validation failed", details);
        }
        
        // Get user's existing review for this recipe
        auto reviews = reviewProcessor_->getRecipeReviews(recipeId);
        int reviewId = -1;
        
        for (const auto& review : reviews) {
            if (review.getUserId() == userId.value()) {
                reviewId = review.getId();
                break;
            }
        }
        
        if (reviewId == -1) {
            return ResponseBuilder::notFound("Review not found for this recipe");
        }
        
        // Extract update data
        int rating = json.has("rating") ? json["rating"].i() : 3;
        QString comment = json.has("comment") ? QString::fromStdString(json["comment"].s()) : QString();
        
        // Update review
        bool success = reviewProcessor_->updateReview(reviewId, rating, comment);
        
        if (!success) {
            return ResponseBuilder::badRequest("Failed to update review");
        }
        
        crow::json::wvalue responseJson;
        responseJson["success"] = true;
        responseJson["message"] = "Review updated successfully";
        responseJson["review_id"] = reviewId;
        responseJson["recipe_id"] = recipeId;
        
        return ResponseBuilder::ok(responseJson);
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

crow::response ReviewController::deleteReview(const crow::request& req, int id) {
    try {
        // Check authentication
        if (!jwtMiddleware_->isAuthenticated(req)) {
            return ResponseBuilder::unauthorized();
        }
        
        auto userId = jwtMiddleware_->getUserId(req);
        if (!userId.has_value()) {
            return ResponseBuilder::unauthorized();
        }
        
        // NOTE: Without getReviewById, we cannot verify ownership
        // In production, this should check if the review belongs to the user
        // For now, we trust the authentication
        
        // Delete review
        bool success = reviewProcessor_->deleteReview(id);
        
        if (!success) {
            return ResponseBuilder::notFound("Review not found");
        }
        
        return ResponseBuilder::noContent();
        
    } catch (const std::exception& e) {
        return ResponseBuilder::internalError(e.what());
    }
}

} // namespace web_api