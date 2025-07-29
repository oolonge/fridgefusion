CREATE OR REPLACE FUNCTION reassign_user_content()
RETURNS TRIGGER AS $$
BEGIN
    -- Переназначаем авторство рецептов на пользователя с ID 1
    UPDATE recipes
    SET author_id = 1
    WHERE author_id = OLD.id;
    
    -- Переназначаем авторство отзывов на пользователя с ID 1
    UPDATE reviews
    SET user_id = 1
    WHERE user_id = OLD.id;
    
    -- Удаляем избранное пользователя
    DELETE FROM favorites
    WHERE user_id = OLD.id;
    
    RETURN OLD;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE TRIGGER before_user_delete
BEFORE DELETE ON users
FOR EACH ROW
EXECUTE FUNCTION reassign_user_content();