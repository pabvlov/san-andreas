-- =====================================================
-- San Andreas Roleplay - Base de Datos Completa
-- =====================================================

-- Crear la base de datos
CREATE DATABASE IF NOT EXISTS `san-andreas` CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;

-- Usar la base de datos
USE `san-andreas`;

-- =====================================================
-- SISTEMA DE USUARIOS Y PERSONAJES
-- =====================================================

-- Tabla de usuarios (cuentas principales con nickname)
CREATE TABLE IF NOT EXISTS `users` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `nickname` VARCHAR(24) NOT NULL,
    `password` VARCHAR(64) NOT NULL,
    `salt` VARCHAR(32) NOT NULL,
    `email` VARCHAR(100) DEFAULT NULL,
    `admin_level` INT DEFAULT 0 COMMENT '0=Jugador, 1=Helper, 2=Moderador, 3=Admin, 4=Owner',
    `registered` DATETIME DEFAULT CURRENT_TIMESTAMP,
    `last_login` DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `nickname` (`nickname`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Tabla de personajes (cada usuario puede tener múltiples personajes)
CREATE TABLE IF NOT EXISTS `characters` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `user_id` INT NOT NULL,
    `name` VARCHAR(24) NOT NULL,
    `age` TINYINT NOT NULL DEFAULT 25,
    `gender` TINYINT NOT NULL DEFAULT 1 COMMENT '1=Masculino, 2=Femenino',
    `money` INT DEFAULT 5000,
    `bank` INT DEFAULT 10000,
    `pos_x` FLOAT DEFAULT 1759.0189,
    `pos_y` FLOAT DEFAULT -1898.1260,
    `pos_z` FLOAT DEFAULT 13.5622,
    `pos_a` FLOAT DEFAULT 266.4503,
    `interior` INT DEFAULT 0,
    `virtual_world` INT DEFAULT 0,
    `health` FLOAT DEFAULT 100.0,
    `armour` FLOAT DEFAULT 0.0,
    `skin` INT DEFAULT 0,
    `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP,
    `last_played` DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `name` (`name`),
    FOREIGN KEY (`user_id`) REFERENCES `users`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- =====================================================
-- SISTEMA ECONOMICO - PROPIEDADES
-- =====================================================

-- Tipos de propiedades (empresas, tiendas, casas)
CREATE TABLE IF NOT EXISTS `property_types` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `name` VARCHAR(50) NOT NULL,
    `category` ENUM('business', 'shop', 'house') NOT NULL,
    `description` VARCHAR(255),
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Catalogo de productos disponibles en el servidor
CREATE TABLE IF NOT EXISTS `products` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `name` VARCHAR(100) NOT NULL,
    `description` VARCHAR(255),
    `base_price` INT NOT NULL DEFAULT 0,
    `category` ENUM('food', 'drink', 'weapon', 'clothing', 'tool', 'vehicle_part', 'other') NOT NULL,
    `is_stackable` TINYINT(1) DEFAULT 1,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Propiedades base (casas, empresas, tiendas)
CREATE TABLE IF NOT EXISTS `properties` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `type_id` INT NOT NULL,
    `name` VARCHAR(100) NOT NULL,
    `owner_type` ENUM('state', 'character', 'faction') NOT NULL DEFAULT 'state',
    `owner_id` INT NULL DEFAULT NULL, -- character_id o faction_id segun owner_type (NULL = Estado)
    `fiscal_price` INT NOT NULL, -- Precio fijo del sistema
    `market_price` INT NULL, -- Precio definido por el dueno (NULL = no en venta)
    `pos_x` FLOAT NOT NULL,
    `pos_y` FLOAT NOT NULL,
    `pos_z` FLOAT NOT NULL,
    `interior` INT DEFAULT 0,
    `interior_id` INT DEFAULT 0, -- 0-2 para elegir entre 3 interiores por tipo
    `virtual_world` INT DEFAULT 0,
    `is_locked` TINYINT(1) DEFAULT 1,
    `bank_balance` INT DEFAULT 0, -- Caja de la propiedad
    `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP,
    `last_used` DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    FOREIGN KEY (`type_id`) REFERENCES `property_types`(`id`)
    -- Note: owner_id no tiene FK porque puede referenciar characters o factions
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Datos especificos de EMPRESAS (talleres, concesionarias, etc)
CREATE TABLE IF NOT EXISTS `businesses` (
    `property_id` INT NOT NULL,
    `service_type` ENUM('mechanic', 'dealership', 'driving_school', 'trucking', 'taxi', 'other') NOT NULL,
    `service_capacity` INT DEFAULT 100, -- Capacidad de servicios (en vez de stock)
    `service_price` INT DEFAULT 0, -- Precio base del servicio
    `reputation` INT DEFAULT 50, -- Reputacion (0-100)
    PRIMARY KEY (`property_id`),
    FOREIGN KEY (`property_id`) REFERENCES `properties`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Datos especificos de TIENDAS (24/7, ammunation, etc)
CREATE TABLE IF NOT EXISTS `shops` (
    `property_id` INT NOT NULL,
    `shop_category` ENUM('general_store', 'ammunation', 'clothing', 'hardware', 'other') NOT NULL,
    `markup_percent` INT DEFAULT 20, -- Porcentaje de ganancia sobre precio base
    PRIMARY KEY (`property_id`),
    FOREIGN KEY (`property_id`) REFERENCES `properties`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Inventario de tiendas (stock de productos)
CREATE TABLE IF NOT EXISTS `shop_inventory` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `shop_id` INT NOT NULL,
    `product_id` INT NOT NULL,
    `quantity` INT NOT NULL DEFAULT 0,
    `purchase_price` INT NOT NULL, -- Precio al que la tienda compro el producto
    `sell_price` INT NOT NULL, -- Precio al que la tienda vende (puede ser modificado)
    `last_restock` DATETIME DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    UNIQUE KEY `shop_product` (`shop_id`, `product_id`),
    FOREIGN KEY (`shop_id`) REFERENCES `shops`(`property_id`) ON DELETE CASCADE,
    FOREIGN KEY (`product_id`) REFERENCES `products`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Datos especificos de CASAS
CREATE TABLE IF NOT EXISTS `houses` (
    `property_id` INT NOT NULL,
    `bedrooms` INT DEFAULT 1,
    `garage_slots` INT DEFAULT 0,
    `storage_slots` INT DEFAULT 10, -- Espacios de almacenamiento
    PRIMARY KEY (`property_id`),
    FOREIGN KEY (`property_id`) REFERENCES `properties`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- =====================================================
-- SISTEMA DE VEHICULOS
-- =====================================================

-- Vehiculos (privados y de renta del Estado)
-- IMPORTANTE: Los vehiculos persisten fisicamente en el mundo hasta ser guardados en garaje
CREATE TABLE IF NOT EXISTS `vehicles` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `model` INT NOT NULL,
    `owner_type` ENUM('state', 'character', 'property', 'faction') NOT NULL DEFAULT 'state',
    `owner_id` INT NULL, -- character_id, property_id o faction_id segun owner_type
    `plate` VARCHAR(8) NOT NULL UNIQUE,
    `pos_x` FLOAT NOT NULL,
    `pos_y` FLOAT NOT NULL,
    `pos_z` FLOAT NOT NULL,
    `pos_a` FLOAT NOT NULL,
    `interior` INT DEFAULT 0,
    `virtual_world` INT DEFAULT 0,
    `color1` INT DEFAULT -1,
    `color2` INT DEFAULT -1,
    `fuel` INT DEFAULT 100,
    `health` FLOAT DEFAULT 1000.0,
    `locked` TINYINT(1) DEFAULT 1,
    `is_spawned` TINYINT(1) DEFAULT 1, -- 1 = en el mundo, 0 = guardado en garaje
    `garage_id` INT NULL, -- ID del garaje/propiedad donde esta guardado (NULL = en el mundo)
    `fiscal_price` INT NOT NULL, -- Precio del Estado para compra
    `market_price` INT NULL, -- Precio de venta entre jugadores (NULL = no en venta)
    `is_rental` TINYINT(1) DEFAULT 0, -- Vehiculo de renta del Estado
    `rental_price_hour` INT DEFAULT 0,
    `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP,
    `last_used` DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    INDEX `idx_owner` (`owner_type`, `owner_id`),
    INDEX `idx_spawned` (`is_spawned`)
    -- Note: owner_id no tiene FK porque puede referenciar characters, properties o factions
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- =====================================================
-- SISTEMA DE TRANSACCIONES
-- =====================================================

-- Transacciones economicas (log de compras/ventas)
CREATE TABLE IF NOT EXISTS `transactions` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `type` ENUM('property_purchase', 'property_sale', 'vehicle_purchase', 'vehicle_sale', 'shop_purchase', 'service_purchase', 'salary', 'tax', 'faction_deposit', 'faction_withdraw', 'other') NOT NULL,
    `from_type` ENUM('character', 'property', 'faction', 'system') NOT NULL,
    `from_id` INT NULL,
    `to_type` ENUM('character', 'property', 'faction', 'system') NOT NULL,
    `to_id` INT NULL,
    `amount` INT NOT NULL,
    `description` VARCHAR(255),
    `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    INDEX `idx_from` (`from_type`, `from_id`),
    INDEX `idx_to` (`to_type`, `to_id`),
    INDEX `idx_created` (`created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- =====================================================
-- SISTEMA DE FACCIONES
-- =====================================================

-- Facciones (organizaciones)
CREATE TABLE IF NOT EXISTS `factions` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `name` VARCHAR(64) NOT NULL UNIQUE,
    `type` ENUM('legal', 'illegal', 'government') NOT NULL,
    `color` INT NOT NULL, -- Color HEX para el chat
    `bank_balance` INT DEFAULT 0,
    `max_members` INT DEFAULT 50,
    `spawn_x` FLOAT DEFAULT 0.0,
    `spawn_y` FLOAT DEFAULT 0.0,
    `spawn_z` FLOAT DEFAULT 0.0,
    `spawn_angle` FLOAT DEFAULT 0.0,
    `spawn_interior` INT DEFAULT 0,
    `spawn_virtual_world` INT DEFAULT 0,
    `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Rangos de facciones
CREATE TABLE IF NOT EXISTS `faction_ranks` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `faction_id` INT NOT NULL,
    `name` VARCHAR(64) NOT NULL,
    `level` TINYINT NOT NULL, -- 1 = mas bajo, 6 = lider
    `salary` INT DEFAULT 0,
    `permissions` INT DEFAULT 0, -- Bitmask: 1=invitar, 2=expulsar, 4=promover, 8=banco, 16=propiedades, 32=vehiculos
    PRIMARY KEY (`id`),
    FOREIGN KEY (`faction_id`) REFERENCES `factions`(`id`) ON DELETE CASCADE,
    UNIQUE KEY `unique_faction_level` (`faction_id`, `level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Miembros de facciones
CREATE TABLE IF NOT EXISTS `faction_members` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `character_id` INT NOT NULL,
    `faction_id` INT NOT NULL,
    `rank_id` INT NOT NULL,
    `joined_at` DATETIME DEFAULT CURRENT_TIMESTAMP,
    `is_on_duty` TINYINT(1) DEFAULT 0,
    PRIMARY KEY (`id`),
    FOREIGN KEY (`character_id`) REFERENCES `characters`(`id`) ON DELETE CASCADE,
    FOREIGN KEY (`faction_id`) REFERENCES `factions`(`id`) ON DELETE CASCADE,
    FOREIGN KEY (`rank_id`) REFERENCES `faction_ranks`(`id`),
    UNIQUE KEY `unique_character_faction` (`character_id`) -- Un personaje solo puede estar en una faccion
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Vehiculos de facciones
CREATE TABLE IF NOT EXISTS `faction_vehicles` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `vehicle_id` INT NOT NULL,
    `faction_id` INT NOT NULL,
    `rank_required` TINYINT DEFAULT 1, -- Rango minimo para usar el vehiculo
    PRIMARY KEY (`id`),
    FOREIGN KEY (`vehicle_id`) REFERENCES `vehicles`(`id`) ON DELETE CASCADE,
    FOREIGN KEY (`faction_id`) REFERENCES `factions`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- =====================================================
-- DATOS INICIALES - TIPOS DE PROPIEDADES
-- =====================================================

INSERT INTO `property_types` (`name`, `category`, `description`) VALUES
('Casa Pequeña', 'house', 'Casa de 1 habitacion con garaje pequeño'),
('Casa Mediana', 'house', 'Casa de 2-3 habitaciones con garaje'),
('Casa Grande', 'house', 'Casa de 4+ habitaciones con garaje grande'),
('Mansion', 'house', 'Propiedad de lujo con multiples habitaciones'),
('Taller Mecanico', 'business', 'Reparacion y modificacion de vehiculos'),
('Concesionaria', 'business', 'Compra y venta de vehiculos'),
('Escuela de Conduccion', 'business', 'Enseñanza y licencias de conducir'),
('Empresa de Taxis', 'business', 'Transporte de pasajeros'),
('Empresa de Camiones', 'business', 'Transporte de carga'),
('24/7 Market', 'shop', 'Tienda de conveniencia'),
('Ammunation', 'shop', 'Tienda de armas y municiones'),
('Tienda de Ropa', 'shop', 'Ropa y accesorios'),
('Ferreteria', 'shop', 'Herramientas y materiales');

-- =====================================================
-- DATOS INICIALES - PRODUCTOS
-- =====================================================

INSERT INTO `products` (`name`, `description`, `base_price`, `category`, `is_stackable`) VALUES
-- Comida
('Hamburguesa', 'Restaura 20 de salud', 50, 'food', 1),
('Pizza', 'Restaura 25 de salud', 70, 'food', 1),
('Hot Dog', 'Restaura 15 de salud', 30, 'food', 1),
-- Bebidas
('Agua', 'Restaura 10 de sed', 20, 'drink', 1),
('Refresco', 'Restaura 15 de sed', 30, 'drink', 1),
('Cafe', 'Restaura 10 de sed y energia', 40, 'drink', 1),
-- Herramientas
('Llave Inglesa', 'Para reparaciones basicas', 150, 'tool', 0),
('Kit de Reparacion', 'Repara vehiculos', 500, 'tool', 1),
('Telefono Movil', 'Comunicacion', 300, 'other', 0),
-- Partes de vehiculos
('Rueda', 'Repuesto para vehiculos', 200, 'vehicle_part', 1),
('Bateria', 'Bateria de auto', 350, 'vehicle_part', 1),
('Aceite de Motor', 'Para mantenimiento', 80, 'vehicle_part', 1);

-- =====================================================
-- DATOS INICIALES - FACCIONES
-- =====================================================

-- Facciones legales
INSERT INTO `factions` (`name`, `type`, `color`, `bank_balance`, `spawn_x`, `spawn_y`, `spawn_z`, `spawn_angle`) VALUES
('Los Santos Police Department', 'legal', 0x0080FF, 100000, 1555.1, -1675.6, 16.2, 90.0),
('Los Santos Fire Department', 'legal', 0xFF4500, 50000, 1776.7, -1459.5, 13.5, 270.0),
('Los Santos Medical Center', 'legal', 0xFF69B4, 75000, 1172.0, -1323.2, 15.4, 270.0),
('San Andreas Government', 'government', 0xFFD700, 500000, 1478.9, -1772.4, 18.8, 0.0);

-- Facciones ilegales
INSERT INTO `factions` (`name`, `type`, `color`, `bank_balance`, `spawn_x`, `spawn_y`, `spawn_z`, `spawn_angle`) VALUES
('Grove Street Families', 'illegal', 0x00AA00, 25000, 2495.0, -1687.5, 13.5, 0.0),
('Ballas', 'illegal', 0xAA00AA, 25000, 2000.0, -1114.0, 27.1, 180.0),
('Los Santos Vagos', 'illegal', 0xFFFF00, 25000, 2786.5, -1926.5, 13.5, 180.0),
('La Cosa Nostra', 'illegal', 0x000000, 50000, 2350.0, -1181.0, 27.9, 90.0);

-- Rangos para LSPD (id=1)
INSERT INTO `faction_ranks` (`faction_id`, `name`, `level`, `salary`, `permissions`) VALUES
(1, 'Cadet', 1, 1500, 0),
(1, 'Officer', 2, 2000, 1),
(1, 'Senior Officer', 3, 2500, 1),
(1, 'Sergeant', 4, 3000, 3),
(1, 'Lieutenant', 5, 3500, 15),
(1, 'Chief of Police', 6, 5000, 63);

-- Rangos para LSFD (id=2)
INSERT INTO `faction_ranks` (`faction_id`, `name`, `level`, `salary`, `permissions`) VALUES
(2, 'Trainee', 1, 1200, 0),
(2, 'Firefighter', 2, 1800, 1),
(2, 'Senior Firefighter', 3, 2200, 1),
(2, 'Captain', 4, 2800, 3),
(2, 'Battalion Chief', 5, 3200, 15),
(2, 'Fire Chief', 6, 4000, 63);

-- Rangos para Medical Center (id=3)
INSERT INTO `faction_ranks` (`faction_id`, `name`, `level`, `salary`, `permissions`) VALUES
(3, 'Intern', 1, 1300, 0),
(3, 'Paramedic', 2, 1900, 1),
(3, 'Doctor', 3, 2500, 1),
(3, 'Senior Doctor', 4, 3000, 3),
(3, 'Surgeon', 5, 3500, 15),
(3, 'Chief of Medicine', 6, 4500, 63);

-- Rangos para Government (id=4)
INSERT INTO `faction_ranks` (`faction_id`, `name`, `level`, `salary`, `permissions`) VALUES
(4, 'Employee', 1, 2000, 0),
(4, 'Official', 2, 2500, 1),
(4, 'Manager', 3, 3000, 3),
(4, 'Director', 4, 4000, 15),
(4, 'Vice Governor', 5, 5000, 31),
(4, 'Governor', 6, 7000, 63);

-- Rangos para Grove Street (id=5)
INSERT INTO `faction_ranks` (`faction_id`, `name`, `level`, `salary`, `permissions`) VALUES
(5, 'Street Thug', 1, 500, 0),
(5, 'Homie', 2, 800, 1),
(5, 'Gangster', 3, 1200, 1),
(5, 'Veteran', 4, 1500, 3),
(5, 'Warlord', 5, 2000, 31),
(5, 'OG', 6, 3000, 63);

-- Rangos para Ballas (id=6)
INSERT INTO `faction_ranks` (`faction_id`, `name`, `level`, `salary`, `permissions`) VALUES
(6, 'Prospect', 1, 500, 0),
(6, 'Member', 2, 800, 1),
(6, 'Soldier', 3, 1200, 1),
(6, 'Enforcer', 4, 1500, 3),
(6, 'Lieutenant', 5, 2000, 31),
(6, 'Shot Caller', 6, 3000, 63);

-- Rangos para Vagos (id=7)
INSERT INTO `faction_ranks` (`faction_id`, `name`, `level`, `salary`, `permissions`) VALUES
(7, 'Peon', 1, 500, 0),
(7, 'Vato', 2, 800, 1),
(7, 'Cholo', 3, 1200, 1),
(7, 'Veterano', 4, 1500, 3),
(7, 'Jefe', 5, 2000, 31),
(7, 'El Patron', 6, 3000, 63);

-- Rangos para La Cosa Nostra (id=8)
INSERT INTO `faction_ranks` (`faction_id`, `name`, `level`, `salary`, `permissions`) VALUES
(8, 'Associate', 1, 1000, 0),
(8, 'Soldier', 2, 1500, 1),
(8, 'Caporegime', 3, 2000, 3),
(8, 'Underboss', 4, 3000, 15),
(8, 'Consigliere', 5, 3500, 31),
(8, 'Don', 6, 5000, 63);

-- =====================================================
-- CONFIRMACION
-- =====================================================

SELECT 'Base de datos creada exitosamente! Todas las tablas y datos iniciales han sido insertados.' as STATUS;
