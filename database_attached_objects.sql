-- Sistema de posiciones personalizadas de objetos attachados
-- Permite guardar las posiciones editadas con /editattachedobject

CREATE TABLE IF NOT EXISTS `character_attached_objects` (
    `id` INT AUTO_INCREMENT PRIMARY KEY,
    `character_id` INT NOT NULL,
    `item_id` INT NOT NULL,
    `slot_index` TINYINT NOT NULL COMMENT '0=mano derecha, 1=mano izquierda, 3=espalda',
    `offset_x` FLOAT NOT NULL DEFAULT 0.0,
    `offset_y` FLOAT NOT NULL DEFAULT 0.0,
    `offset_z` FLOAT NOT NULL DEFAULT 0.0,
    `rot_x` FLOAT NOT NULL DEFAULT 0.0,
    `rot_y` FLOAT NOT NULL DEFAULT 0.0,
    `rot_z` FLOAT NOT NULL DEFAULT 0.0,
    `scale_x` FLOAT NOT NULL DEFAULT 1.0,
    `scale_y` FLOAT NOT NULL DEFAULT 1.0,
    `scale_z` FLOAT NOT NULL DEFAULT 1.0,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    `updated_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    UNIQUE KEY `unique_character_item_slot` (`character_id`, `item_id`, `slot_index`),
    FOREIGN KEY (`character_id`) REFERENCES `characters`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Índices para optimizar búsquedas
CREATE INDEX idx_character_id ON character_attached_objects(character_id);
CREATE INDEX idx_item_id ON character_attached_objects(item_id);
