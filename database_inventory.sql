-- Sistema de Inventario
USE `san-andreas`;

-- Tabla de items disponibles en el servidor
CREATE TABLE IF NOT EXISTS `items` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `name` VARCHAR(64) NOT NULL,
    `description` VARCHAR(255),
    `item_type` ENUM('weapon', 'key', 'tool', 'food', 'misc') NOT NULL,
    `model_id` INT NOT NULL, -- ID del modelo 3D del objeto
    `max_stack` INT DEFAULT 1,
    `usable` TINYINT(1) DEFAULT 1,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Inventario de personajes
CREATE TABLE IF NOT EXISTS `character_inventory` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `character_id` INT NOT NULL,
    `item_id` INT NOT NULL,
    `quantity` INT DEFAULT 1,
    `metadata` VARCHAR(255), -- JSON para datos extra (ej: vehicle_id para llaves)
    `slot` INT NOT NULL,
    PRIMARY KEY (`id`),
    FOREIGN KEY (`character_id`) REFERENCES `characters`(`id`) ON DELETE CASCADE,
    FOREIGN KEY (`item_id`) REFERENCES `items`(`id`) ON DELETE CASCADE,
    UNIQUE KEY `character_slot` (`character_id`, `slot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Insertar items base
INSERT INTO `items` (`name`, `description`, `item_type`, `model_id`, `max_stack`, `usable`) VALUES
('Llave de Vehiculo', 'Llave para encender un vehiculo', 'key', 19991, 1, 1),
('Pistola 9mm', 'Pistola calibre 9mm', 'weapon', 348, 1, 1),
('Desert Eagle', 'Pistola calibre .50', 'weapon', 348, 1, 1),
('Telefono Celular', 'Para comunicarse', 'tool', 330, 1, 1),
('Kit de Reparacion', 'Repara vehiculos', 'tool', 1242, 5, 1),
('Hamburguesa', 'Restaura 20 de salud', 'food', 2880, 10, 1),
('Agua', 'Restaura sed', 'food', 1484, 10, 1);

SELECT 'Sistema de inventario creado exitosamente.' as STATUS;
