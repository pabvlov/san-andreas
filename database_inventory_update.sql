-- Actualización de sistema de inventario con llaves maestra y normal
USE `san-andreas`;

-- Agregar columna subtype si no existe (ignorar error si ya existe)
ALTER TABLE `items` ADD COLUMN `subtype` VARCHAR(32) DEFAULT NULL COMMENT 'master_key, normal_key, etc';

-- Agregar campo item_size a la tabla items (0=pequeño, 1=grande)
ALTER TABLE `items` 
ADD COLUMN `item_size` TINYINT(1) DEFAULT 0 COMMENT '0=Pequeño (bolsillo), 1=Grande (espalda)';

-- Agregar campo item_capacity (todos los items tienen capacidad 1)
ALTER TABLE `items`
ADD COLUMN `item_capacity` INT DEFAULT 1 COMMENT 'Capacidad que ocupa el item (siempre 1)';

-- Limpiar items antiguos
DELETE FROM `items`;

-- Reinsertar items con los nuevos IDs y tamaños
INSERT INTO `items` (`id`, `name`, `item_type`, `model_id`, `max_stack`, `usable`, `description`, `subtype`, `item_size`, `item_capacity`) VALUES
(1, 'Llave maestra de vehículo', 'key', 19991, 1, 1, 'Llave principal - permite duplicados', 'master_key', 0, 1),
(2, 'Llave de vehículo', 'key', 19991, 1, 1, 'Llave para encender el motor', 'normal_key', 0, 1),
(3, 'Pistola 9mm', 'weapon', 346, 1, 1, 'Pistola calibre 9mm', NULL, 1, 1),
(4, 'Desert Eagle', 'weapon', 348, 1, 1, 'Pistola Desert Eagle', NULL, 1, 1),
(5, 'Teléfono móvil', 'tool', 330, 1, 1, 'Teléfono para comunicarse', NULL, 0, 1),
(6, 'Kit de reparación', 'tool', 1240, 1, 1, 'Repara vehículos dañados', NULL, 1, 1),
(7, 'Hamburguesa', 'food', 2703, 5, 1, 'Recupera 20 HP', NULL, 0, 1),
(8, 'Agua embotellada', 'food', 1484, 5, 1, 'Recupera 10 HP', NULL, 0, 1);

-- Agregar índice para mejorar búsquedas por slot si no existe
CREATE INDEX `idx_character_slot` ON `character_inventory` (`character_id`, `slot`);

-- Nota: Los slots ahora son:
-- Slot 0: Mano izquierda
-- Slot 1: Mano derecha  
-- Slot 2: Espalda (items grandes visibles)
-- Slots 3-8: Bolsillos pequeños (6 slots)

SELECT 'Sistema de inventario actualizado correctamente.' as STATUS;
