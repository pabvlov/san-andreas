-- Tabla de items tirados en el suelo
CREATE TABLE IF NOT EXISTS `dropped_items` (
  `id` INT AUTO_INCREMENT PRIMARY KEY,
  `item_id` INT NOT NULL,
  `quantity` INT NOT NULL DEFAULT 1,
  `metadata` TEXT NULL,
  `pos_x` FLOAT NOT NULL,
  `pos_y` FLOAT NOT NULL,
  `pos_z` FLOAT NOT NULL,
  `interior` INT NOT NULL DEFAULT 0,
  `virtual_world` INT NOT NULL DEFAULT 0,
  `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  INDEX `idx_position` (`pos_x`, `pos_y`, `pos_z`),
  INDEX `idx_item` (`item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
