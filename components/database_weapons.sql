CREATE TABLE IF NOT EXISTS weapons (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(64) NOT NULL,
    weapon_slot TINYINT NOT NULL COMMENT 'Slot SA-MP (0-12)',
    weapon_id_samp TINYINT NOT NULL COMMENT 'ID del arma en SA-MP (0-46)',
    ammo_capacity INT NOT NULL COMMENT 'Capacidad máxima de munición',
    hand_object_id INT NOT NULL COMMENT 'ID del objeto 3D para la mano',
    item_id INT COMMENT 'FK a items table',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (item_id) REFERENCES items(id)
);

CREATE TABLE IF NOT EXISTS weapon_magazines (
    id INT PRIMARY KEY AUTO_INCREMENT,
    weapon_id INT NOT NULL COMMENT 'FK a weapons',
    name VARCHAR(64) NOT NULL,
    magazine_capacity INT NOT NULL COMMENT 'Balas por cargador',
    hand_object_id INT NOT NULL COMMENT 'ID del objeto 3D para la mano',
    item_id INT NOT NULL COMMENT 'FK a items table',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (weapon_id) REFERENCES weapons(id),
    FOREIGN KEY (item_id) REFERENCES items(id)
);

INSERT INTO weapons (name, weapon_slot, weapon_id_samp, ammo_capacity, hand_object_id, item_id) VALUES
('Pistola 9mm', 2, 22, 17, 346, 9),
('Pistola Silenciada', 2, 23, 17, 347, 10),
('Desert Eagle', 2, 24, 7, 348, 11);

INSERT INTO weapons (name, weapon_slot, weapon_id_samp, ammo_capacity, hand_object_id, item_id) VALUES
('Escopeta', 3, 25, 1, 349, 12),
('Escopeta Recortada', 3, 26, 2, 350, 13),
('Escopeta de Combate', 3, 27, 7, 351, 14);

INSERT INTO weapons (name, weapon_slot, weapon_id_samp, ammo_capacity, hand_object_id, item_id) VALUES
('Micro SMG', 4, 28, 50, 352, 15),
('MP5', 4, 29, 30, 353, 16),
('Tec-9', 4, 32, 50, 372, 17);

INSERT INTO weapons (name, weapon_slot, weapon_id_samp, ammo_capacity, hand_object_id, item_id) VALUES
('AK-47', 5, 30, 30, 355, 18),
('M4', 5, 31, 50, 356, 19);

INSERT INTO weapons (name, weapon_slot, weapon_id_samp, ammo_capacity, hand_object_id, item_id) VALUES
('Rifle de Caza', 6, 33, 1, 357, 20),
('Rifle de Francotirador', 6, 34, 1, 358, 21);

INSERT INTO weapons (name, weapon_slot, weapon_id_samp, ammo_capacity, hand_object_id, item_id) VALUES
('Lanzacohetes', 7, 35, 1, 359, 22),
('Lanzacohetes HS', 7, 36, 1, 360, 23);

INSERT INTO weapons (name, weapon_slot, weapon_id_samp, ammo_capacity, hand_object_id, item_id) VALUES
('Nudillos de Bronce', 1, 1, 1, 331, 24),
('Palo de Golf', 1, 2, 1, 333, 25),
('Porra', 1, 3, 1, 334, 26),
('Cuchillo', 1, 4, 1, 335, 27),
('Bate de Béisbol', 1, 5, 1, 336, 28),
('Pala', 1, 6, 1, 337, 29),
('Taco de Billar', 1, 7, 1, 338, 30),
('Katana', 1, 8, 1, 339, 31),
('Motosierra', 1, 9, 1, 341, 32);

INSERT INTO weapons (name, weapon_slot, weapon_id_samp, ammo_capacity, hand_object_id, item_id) VALUES
('Granada', 8, 16, 1, 342, 33),
('Granada de Humo', 8, 17, 1, 343, 34),
('Molotov', 8, 18, 1, 344, 35);

INSERT INTO weapon_magazines (weapon_id, name, magazine_capacity, hand_object_id, item_id) VALUES
(1, 'Cargador 9mm', 17, 2358, 36),
(2, 'Cargador Pistola Silenciada', 17, 2358, 37),
(3, 'Cargador Desert Eagle', 7, 2358, 38),
(4, 'Cartuchos Escopeta', 1, 2040, 39),
(5, 'Cartuchos Escopeta Recortada', 2, 2040, 40),
(6, 'Cartuchos Escopeta Combate', 7, 2040, 41),
(7, 'Cargador Micro SMG', 50, 2358, 42),
(8, 'Cargador MP5', 30, 2358, 43),
(9, 'Cargador Tec-9', 50, 2358, 44),
(10, 'Cargador AK-47', 30, 2358, 45),
(11, 'Cargador M4', 50, 2358, 46),
(12, 'Cartuchos Rifle de Caza', 1, 2040, 47),
(13, 'Cartuchos Francotirador', 1, 2040, 48),
(14, 'Cohete RPG', 1, 359, 49),
(15, 'Cohete HS', 1, 360, 50);

