-- FridgeFusion Database Schema
-- Lab #5 initialization script

-- Enable UUID extension
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- User roles enum
CREATE TYPE user_role AS ENUM ('user', 'admin');

-- Users table
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    registration_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    role user_role DEFAULT 'user',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Categories table
CREATE TABLE IF NOT EXISTS categories (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) UNIQUE NOT NULL,
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Ingredients table
CREATE TABLE IF NOT EXISTS ingredients (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) UNIQUE NOT NULL,
    category VARCHAR(50),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Recipes table
CREATE TABLE IF NOT EXISTS recipes (
    id SERIAL PRIMARY KEY,
    name VARCHAR(200) NOT NULL,
    description TEXT,
    preparation_time INT, -- in minutes
    cooking_time INT, -- in minutes
    instructions TEXT,
    servings INT DEFAULT 1,
    created_by INT REFERENCES users(id) ON DELETE SET NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Recipe categories (many-to-many)
CREATE TABLE IF NOT EXISTS recipe_categories (
    recipe_id INT REFERENCES recipes(id) ON DELETE CASCADE,
    category_id INT REFERENCES categories(id) ON DELETE CASCADE,
    PRIMARY KEY (recipe_id, category_id)
);

-- Unit measurements
CREATE TABLE IF NOT EXISTS units (
    id SERIAL PRIMARY KEY,
    name VARCHAR(50) UNIQUE NOT NULL,
    abbreviation VARCHAR(10)
);

-- Recipe ingredients (many-to-many with quantity)
CREATE TABLE IF NOT EXISTS recipe_ingredients (
    recipe_id INT REFERENCES recipes(id) ON DELETE CASCADE,
    ingredient_id INT REFERENCES ingredients(id) ON DELETE CASCADE,
    quantity DECIMAL(10, 2) NOT NULL,
    unit_id INT REFERENCES units(id) ON DELETE SET NULL,
    is_optional BOOLEAN DEFAULT FALSE,
    PRIMARY KEY (recipe_id, ingredient_id)
);

-- User favorites
CREATE TABLE IF NOT EXISTS favorites (
    user_id INT REFERENCES users(id) ON DELETE CASCADE,
    recipe_id INT REFERENCES recipes(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (user_id, recipe_id)
);

-- Reviews
CREATE TABLE IF NOT EXISTS reviews (
    id SERIAL PRIMARY KEY,
    recipe_id INT REFERENCES recipes(id) ON DELETE CASCADE,
    user_id INT REFERENCES users(id) ON DELETE CASCADE,
    rating INT CHECK (rating >= 1 AND rating <= 5),
    comment TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE (recipe_id, user_id)
);

-- User's fridge
CREATE TABLE IF NOT EXISTS fridge (
    id SERIAL PRIMARY KEY,
    user_id INT UNIQUE REFERENCES users(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Fridge items
CREATE TABLE IF NOT EXISTS fridge_items (
    id SERIAL PRIMARY KEY,
    fridge_id INT REFERENCES fridge(id) ON DELETE CASCADE,
    ingredient_id INT REFERENCES ingredients(id) ON DELETE CASCADE,
    quantity DECIMAL(10, 2) NOT NULL,
    unit_id INT REFERENCES units(id) ON DELETE SET NULL,
    expiry_date DATE,
    added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE (fridge_id, ingredient_id)
);

-- Indexes for better query performance
CREATE INDEX idx_recipes_created_by ON recipes(created_by);
CREATE INDEX idx_recipe_categories_recipe ON recipe_categories(recipe_id);
CREATE INDEX idx_recipe_categories_category ON recipe_categories(category_id);
CREATE INDEX idx_recipe_ingredients_recipe ON recipe_ingredients(recipe_id);
CREATE INDEX idx_recipe_ingredients_ingredient ON recipe_ingredients(ingredient_id);
CREATE INDEX idx_favorites_user ON favorites(user_id);
CREATE INDEX idx_favorites_recipe ON favorites(recipe_id);
CREATE INDEX idx_reviews_recipe ON reviews(recipe_id);
CREATE INDEX idx_reviews_user ON reviews(user_id);
CREATE INDEX idx_fridge_items_fridge ON fridge_items(fridge_id);
CREATE INDEX idx_fridge_items_ingredient ON fridge_items(ingredient_id);

-- Insert test data

-- Admin and test users
INSERT INTO users (username, email, password_hash, role) VALUES
('admin', 'admin@fridgefusion.com', '$2a$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi', 'admin'),
('testuser', 'test@fridgefusion.com', '$2a$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi', 'user'),
('john_doe', 'john@example.com', '$2a$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi', 'user');

-- Categories
INSERT INTO categories (name, description) VALUES
('Breakfast', 'Morning meals'),
('Lunch', 'Midday meals'),
('Dinner', 'Evening meals'),
('Dessert', 'Sweet treats'),
('Vegetarian', 'No meat dishes'),
('Quick & Easy', 'Under 30 minutes');

-- Units
INSERT INTO units (name, abbreviation) VALUES
('grams', 'g'),
('kilograms', 'kg'),
('milliliters', 'ml'),
('liters', 'l'),
('pieces', 'pcs'),
('tablespoon', 'tbsp'),
('teaspoon', 'tsp'),
('cups', 'cup');

-- Ingredients
INSERT INTO ingredients (name, category) VALUES
('Eggs', 'Dairy'),
('Milk', 'Dairy'),
('Flour', 'Grains'),
('Sugar', 'Sweeteners'),
('Salt', 'Seasonings'),
('Butter', 'Dairy'),
('Chicken Breast', 'Meat'),
('Tomato', 'Vegetables'),
('Onion', 'Vegetables'),
('Garlic', 'Seasonings'),
('Olive Oil', 'Oils'),
('Pasta', 'Grains'),
('Rice', 'Grains'),
('Cheese', 'Dairy'),
('Bread', 'Grains');

-- Recipes
INSERT INTO recipes (name, description, preparation_time, cooking_time, instructions, servings, created_by) VALUES
('Scrambled Eggs', 'Classic breakfast scrambled eggs', 5, 5,
 '1. Beat eggs in a bowl\n2. Heat butter in pan\n3. Pour eggs and stir\n4. Season with salt',
 2, 1),

('Pasta with Tomato Sauce', 'Simple Italian pasta', 10, 15,
 '1. Boil pasta\n2. Sauté garlic and onion\n3. Add tomatoes\n4. Mix with pasta',
 4, 1),

('Cheese Sandwich', 'Quick cheese sandwich', 5, 0,
 '1. Take two slices of bread\n2. Add cheese\n3. Optional: toast it',
 1, 2),

('Chicken Rice Bowl', 'Healthy chicken and rice', 15, 25,
 '1. Cook rice\n2. Season and grill chicken\n3. Combine and serve',
 2, 2);

-- Recipe categories mapping
INSERT INTO recipe_categories (recipe_id, category_id) VALUES
(1, 1), -- Scrambled Eggs -> Breakfast
(1, 6), -- Scrambled Eggs -> Quick & Easy
(2, 2), -- Pasta -> Lunch
(2, 5), -- Pasta -> Vegetarian
(3, 1), -- Cheese Sandwich -> Breakfast
(3, 6), -- Cheese Sandwich -> Quick & Easy
(4, 3); -- Chicken Rice Bowl -> Dinner

-- Recipe ingredients
INSERT INTO recipe_ingredients (recipe_id, ingredient_id, quantity, unit_id) VALUES
(1, 1, 3, 5), -- Scrambled Eggs: 3 eggs
(1, 6, 20, 1), -- Scrambled Eggs: 20g butter
(1, 5, 1, 7), -- Scrambled Eggs: 1 tsp salt
(2, 12, 400, 1), -- Pasta: 400g pasta
(2, 8, 500, 1), -- Pasta: 500g tomatoes
(2, 9, 1, 5), -- Pasta: 1 onion
(2, 10, 2, 5), -- Pasta: 2 cloves garlic
(2, 11, 30, 3), -- Pasta: 30ml olive oil
(3, 15, 2, 5), -- Sandwich: 2 slices bread
(3, 14, 50, 1), -- Sandwich: 50g cheese
(4, 7, 300, 1), -- Chicken Bowl: 300g chicken
(4, 13, 200, 1), -- Chicken Bowl: 200g rice
(4, 5, 1, 7), -- Chicken Bowl: 1 tsp salt
(4, 11, 20, 3); -- Chicken Bowl: 20ml olive oil

-- Test favorites
INSERT INTO favorites (user_id, recipe_id) VALUES
(2, 1),
(2, 3),
(3, 2);

-- Test reviews
INSERT INTO reviews (recipe_id, user_id, rating, comment) VALUES
(1, 2, 5, 'Perfect breakfast!'),
(2, 3, 4, 'Delicious and easy to make'),
(3, 2, 5, 'My go-to quick meal');

-- Test fridges
INSERT INTO fridge (user_id) VALUES (2), (3);

-- Test fridge items
INSERT INTO fridge_items (fridge_id, ingredient_id, quantity, unit_id, expiry_date) VALUES
(1, 1, 12, 5, CURRENT_DATE + INTERVAL '7 days'),
(1, 2, 1, 4, CURRENT_DATE + INTERVAL '5 days'),
(1, 15, 1, 5, CURRENT_DATE + INTERVAL '3 days'),
(1, 14, 200, 1, CURRENT_DATE + INTERVAL '14 days'),
(2, 7, 500, 1, CURRENT_DATE + INTERVAL '2 days'),
(2, 13, 1, 2, CURRENT_DATE + INTERVAL '30 days');

-- Grant permissions for replication
GRANT SELECT ON ALL TABLES IN SCHEMA public TO readonly_user;
GRANT SELECT ON ALL SEQUENCES IN SCHEMA public TO readonly_user;

-- Success message
DO $$
BEGIN
    RAISE NOTICE 'Database initialized successfully!';
    RAISE NOTICE 'Test users:';
    RAISE NOTICE '  - admin@fridgefusion.com (password: password)';
    RAISE NOTICE '  - test@fridgefusion.com (password: password)';
    RAISE NOTICE 'Total recipes: %', (SELECT COUNT(*) FROM recipes);
    RAISE NOTICE 'Total ingredients: %', (SELECT COUNT(*) FROM ingredients);
END $$;
