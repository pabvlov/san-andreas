-- Actualización de sistema de inventario con llaves maestra y normal
USE `san-andreas`;

-- Agregar columna subtype si no existe
ALTER TABLE `items` ADD COLUMN IF NOT EXISTS `subtype` VARCHAR(32) DEFAULT NULL COMMENT 'master_key, normal_key, etc';

-- Limpiar items antiguos
DELETE FROM `items`;

-- Reinsertar items con los nuevos IDs
INSERT INTO `items` (`id`, `name`, `item_type`, `model_id`, `max_stack`, `usable`, `description`, `subtype`) VALUES
(1, 'Llave maestra de vehículo', 'key', 19991, 1, 1, 'Llave principal - permite duplicados', 'master_key'),
(2, 'Llave de vehículo', 'key', 19991, 1, 1, 'Llave para encender el motor', 'normal_key'),
(3, 'Pistola 9mm', 'weapon', 346, 1, 1, 'Pistola calibre 9mm', NULL),
(4, 'Desert Eagle', 'weapon', 348, 1, 1, 'Pistola Desert Eagle', NULL),
(5, 'Teléfono móvil', 'tool', 330, 1, 1, 'Teléfono para comunicarse', NULL),
(6, 'Kit de reparación', 'tool', 1240, 1, 1, 'Repara vehículos dañados', NULL),
(7, 'Hamburguesa', 'food', 2703, 5, 1, 'Recupera 20 HP', NULL),
(8, 'Agua embotellada', 'food', 1484, 5, 1, 'Recupera 10 HP', NULL);
