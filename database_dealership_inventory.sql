-- Tabla de inventario de concesionarias
USE `san-andreas`;

CREATE TABLE IF NOT EXISTS `dealership_inventory` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `property_id` INT NOT NULL,
    `vehicle_model` INT NOT NULL,
    `stock` INT NOT NULL DEFAULT 0,
    `sale_price` INT NOT NULL,
    `last_restock` DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `dealership_vehicle` (`property_id`, `vehicle_model`),
    FOREIGN KEY (`property_id`) REFERENCES `properties`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Insertar stock inicial en la concesionaria ID 1 (ejemplo)
-- Ajusta el property_id según tu concesionaria
INSERT INTO `dealership_inventory` (`property_id`, `vehicle_model`, `stock`, `sale_price`) VALUES
-- Autos económicos
(1, 400, 5, 15000),   -- Landstalker
(1, 401, 3, 25000),   -- Bravura
(1, 404, 2, 18000),   -- Perennial
(1, 405, 4, 35000),   -- Sentinel
-- Autos deportivos
(1, 411, 2, 95000),   -- Infernus
(1, 415, 3, 85000),   -- Cheetah
(1, 429, 2, 90000),   -- Banshee
(1, 451, 2, 120000),  -- Turismo
(1, 477, 3, 45000),   -- ZR-350
(1, 506, 2, 110000),  -- Super GT
(1, 541, 3, 75000),   -- Bullet
(1, 559, 4, 35000),   -- Jester
(1, 560, 5, 60000),   -- Sultan
(1, 562, 3, 55000),   -- Elegy
-- SUVs
(1, 400, 5, 25000),   -- Landstalker
(1, 470, 3, 35000),   -- Patriot
(1, 495, 4, 32000),   -- Sandking
-- Camionetas
(1, 422, 5, 18000),   -- Bobcat
(1, 478, 4, 22000),   -- Walton
(1, 543, 3, 28000);   -- Sadler

SELECT 'Tabla dealership_inventory creada con stock inicial.' as STATUS;
