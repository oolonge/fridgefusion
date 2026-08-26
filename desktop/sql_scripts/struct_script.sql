-- Часть 1: Создание независимых таблиц

DO $$ 
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'user_role') THEN
        CREATE TYPE user_role AS ENUM ('GUEST', 'USER', 'ADMIN');
    END IF;
END $$;

-- Создание таблицы пользователей с ролью
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username TEXT NOT NULL,
    email TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    role user_role NOT NULL DEFAULT 'USER',
    registration_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Создание таблицы категорий
CREATE TABLE categories (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT
);

-- Создание таблицы единиц измерения
CREATE TABLE units (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    abbreviation TEXT NOT NULL,
    description TEXT
);

-- Часть 2: Создание таблиц с зависимостями первого уровня

-- Создание таблицы ингредиентов
CREATE TABLE ingredients (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    default_unit_id INTEGER,
    calories REAL,
    FOREIGN KEY (default_unit_id) REFERENCES units(id)
);

-- Создание таблицы рецептов
CREATE TABLE recipes (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT,
    preparation_time INTEGER, -- время в минутах
    cooking_time INTEGER, -- время в минутах
    instructions TEXT, -- список шагов в формате текста/JSON
    author_id INTEGER,
    FOREIGN KEY (author_id) REFERENCES users(id)
);

-- Часть 3: Создание таблиц с зависимостями второго уровня

-- Создание таблицы отзывов
CREATE TABLE reviews (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL,
    recipe_id INTEGER NOT NULL,
    rating INTEGER CHECK(rating BETWEEN 1 AND 5),
    comment TEXT,
    date_posted TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (recipe_id) REFERENCES recipes(id)
);

-- Создание таблицы близости ингредиентов
CREATE TABLE ingredient_similarity (
    id SERIAL PRIMARY KEY,
    ingredient_id_1 INTEGER NOT NULL,
    ingredient_id_2 INTEGER NOT NULL,
    similarity_score REAL CHECK(similarity_score BETWEEN 0 AND 1),
    FOREIGN KEY (ingredient_id_1) REFERENCES ingredients(id),
    FOREIGN KEY (ingredient_id_2) REFERENCES ingredients(id),
    CONSTRAINT unique_pair UNIQUE (ingredient_id_1, ingredient_id_2)
);

-- Создание связующей таблицы между рецептами и ингредиентами
CREATE TABLE recipe_ingredients (
    id SERIAL PRIMARY KEY,
    recipe_id INTEGER NOT NULL,
    ingredient_id INTEGER NOT NULL,
    quantity REAL NOT NULL,
    unit_id INTEGER NOT NULL,
    is_optional BOOLEAN DEFAULT false,
    FOREIGN KEY (recipe_id) REFERENCES recipes(id),
    FOREIGN KEY (ingredient_id) REFERENCES ingredients(id),
    FOREIGN KEY (unit_id) REFERENCES units(id)
);

-- Создание связующей таблицы между рецептами и категориями
CREATE TABLE recipe_categories (
    id SERIAL PRIMARY KEY,
    recipe_id INTEGER NOT NULL,
    category_id INTEGER NOT NULL,
    FOREIGN KEY (recipe_id) REFERENCES recipes(id),
    FOREIGN KEY (category_id) REFERENCES categories(id),
    CONSTRAINT unique_recipe_category UNIQUE (recipe_id, category_id)
);

-- Создание таблицы избранных рецептов
CREATE TABLE favorites (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL,
    recipe_id INTEGER NOT NULL,
    date_added TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (recipe_id) REFERENCES recipes(id),
    CONSTRAINT unique_favorite UNIQUE (user_id, recipe_id)
);
