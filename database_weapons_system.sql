ALTER TABLE items ADD COLUMN weapon_id INT DEFAULT NULL;
ALTER TABLE items ADD COLUMN weapon_ammo_capacity INT DEFAULT 0;
ALTER TABLE items ADD COLUMN weapon_object_id INT DEFAULT 0;
ALTER TABLE items ADD COLUMN is_weapon TINYINT(1) DEFAULT 0;
ALTER TABLE items ADD COLUMN ammo_type VARCHAR(32) DEFAULT NULL;

INSERT INTO items (name, item_type, description, model_id, is_weapon, weapon_id, weapon_ammo_capacity, weapon_object_id, ammo_type, max_stack) 
VALUES 
('Pistola 9mm', 'weapon', 'Pistola estándar 9mm', 346, 1, 22, 17, 346, 'pistol', 1),
('Pistola Silenciada', 'weapon', 'Pistola con silenciador', 347, 1, 23, 17, 347, 'pistol', 1),
('Desert Eagle', 'weapon', 'Pistola calibre .50', 348, 1, 24, 7, 348, 'pistol', 1);

INSERT INTO items (name, item_type, description, model_id, is_weapon, weapon_id, weapon_ammo_capacity, weapon_object_id, ammo_type, max_stack)
VALUES
('Escopeta', 'weapon', 'Escopeta estándar', 349, 1, 25, 1, 349, 'shotgun', 1),
('Escopeta Recortada', 'weapon', 'Escopeta de cañón recortado', 350, 1, 26, 2, 350, 'shotgun', 1),
('Escopeta de Combate', 'weapon', 'Escopeta SPAS-12', 351, 1, 27, 7, 351, 'shotgun', 1);

INSERT INTO items (name, item_type, description, model_id, is_weapon, weapon_id, weapon_ammo_capacity, weapon_object_id, ammo_type, max_stack)
VALUES
('Micro SMG', 'weapon', 'Micro Uzi', 352, 1, 28, 50, 352, 'smg', 1),
('MP5', 'weapon', 'Subfusil MP5', 353, 1, 29, 30, 353, 'smg', 1),
('Tec-9', 'weapon', 'Tec-9', 372, 1, 32, 50, 372, 'smg', 1);

INSERT INTO items (name, item_type, description, model_id, is_weapon, weapon_id, weapon_ammo_capacity, weapon_object_id, ammo_type, max_stack)
VALUES
('AK-47', 'weapon', 'Rifle de asalto AK-47', 355, 1, 30, 30, 355, 'rifle', 1),
('M4', 'weapon', 'Rifle de asalto M4', 356, 1, 31, 50, 356, 'rifle', 1);

INSERT INTO items (name, item_type, description, model_id, is_weapon, weapon_id, weapon_ammo_capacity, weapon_object_id, ammo_type, max_stack)
VALUES
('Rifle de Caza', 'weapon', 'Rifle de caza', 357, 1, 33, 1, 357, 'rifle', 1),
('Rifle de Francotirador', 'weapon', 'Rifle de francotirador', 358, 1, 34, 1, 358, 'rifle', 1);

INSERT INTO items (name, item_type, description, model_id, is_weapon, weapon_id, weapon_ammo_capacity, weapon_object_id, ammo_type, max_stack)
VALUES
('Lanzacohetes', 'weapon', 'Lanzacohetes RPG', 359, 1, 35, 1, 359, 'heavy', 1),
('Lanzacohetes HS', 'weapon', 'Lanzacohetes HS', 360, 1, 36, 1, 360, 'heavy', 1);

INSERT INTO items (name, item_type, description, model_id, is_weapon, weapon_id, weapon_ammo_capacity, weapon_object_id, ammo_type, max_stack)
VALUES
('Nudillos de Bronce', 'weapon', 'Nudillos metálicos', 331, 1, 1, 1, 331, NULL, 1),
('Palo de Golf', 'weapon', 'Palo de golf', 333, 1, 2, 1, 333, NULL, 1),
('Porra', 'weapon', 'Porra policial', 334, 1, 3, 1, 334, NULL, 1),
('Cuchillo', 'weapon', 'Cuchillo de combate', 335, 1, 4, 1, 335, NULL, 1),
('Bate de Béisbol', 'weapon', 'Bate de béisbol', 336, 1, 5, 1, 336, NULL, 1),
('Pala', 'weapon', 'Pala', 337, 1, 6, 1, 337, NULL, 1),
('Taco de Billar', 'weapon', 'Taco de billar', 338, 1, 7, 1, 338, NULL, 1),
('Katana', 'weapon', 'Espada katana', 339, 1, 8, 1, 339, NULL, 1),
('Motosierra', 'weapon', 'Motosierra', 341, 1, 9, 1, 341, NULL, 1);

INSERT INTO items (name, item_type, description, model_id, is_weapon, weapon_id, weapon_ammo_capacity, weapon_object_id, ammo_type, max_stack)
VALUES
('Granada', 'weapon', 'Granada de fragmentación', 342, 1, 16, 1, 342, NULL, 50),
('Granada de Humo', 'weapon', 'Granada de gas lacrimógeno', 343, 1, 17, 1, 343, NULL, 50),
('Molotov', 'weapon', 'Cóctel molotov', 344, 1, 18, 1, 344, NULL, 50);

INSERT INTO items (name, item_type, description, model_id, is_weapon, weapon_id, weapon_ammo_capacity, weapon_object_id, ammo_type, max_stack)
VALUES
('Cargador de Pistola', 'ammo', 'Munición para pistolas (17 balas)', 2061, 0, NULL, 17, 2061, 'pistol', 50),
('Cargador de Escopeta', 'ammo', 'Cartuchos para escopeta (7 cartuchos)', 2061, 0, NULL, 7, 2061, 'shotgun', 50),
('Cargador de SMG', 'ammo', 'Munición para subfusiles (30 balas)', 2061, 0, NULL, 30, 2061, 'smg', 50),
('Cargador de Rifle', 'ammo', 'Munición para rifles (30 balas)', 2061, 0, NULL, 30, 2061, 'rifle', 50),
('Munición Pesada', 'ammo', 'Munición para armas pesadas (1 proyectil)', 2061, 0, NULL, 1, 2061, 'heavy', 20);

ALTER TABLE character_inventory ADD COLUMN current_ammo INT DEFAULT 0;

UPDATE items SET is_weapon = 0 WHERE is_weapon IS NULL;
