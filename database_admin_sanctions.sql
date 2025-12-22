-- ================================================
-- SISTEMA DE SANCIONES ADMINISTRATIVAS
-- ================================================
-- Este archivo agrega el sistema completo de registro
-- de sanciones administrativas y advertencias
-- ================================================

-- Agregar columna de advertencias a la tabla users
ALTER TABLE `users` 
ADD COLUMN `warnings` INT DEFAULT 0 AFTER `admin_level`;

-- Crear tabla para el historial de sanciones
CREATE TABLE IF NOT EXISTS `admin_sanctions` (
    `id` INT NOT NULL AUTO_INCREMENT,
    `user_id` INT NOT NULL,
    `admin_id` INT NOT NULL,
    `admin_name` VARCHAR(50) NOT NULL,
    `sanction_type` VARCHAR(20) NOT NULL COMMENT 'WARN, KICK, BAN, JAIL',
    `reason` VARCHAR(255) NOT NULL,
    `duration` INT DEFAULT 0 COMMENT 'Para BAN (días) o JAIL (minutos). 0 = permanente',
    `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    INDEX `user_id` (`user_id`),
    INDEX `admin_id` (`admin_id`),
    INDEX `sanction_type` (`sanction_type`),
    FOREIGN KEY (`user_id`) REFERENCES `users`(`id`) ON DELETE CASCADE,
    FOREIGN KEY (`admin_id`) REFERENCES `users`(`id`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Registro de todas las sanciones administrativas';

-- Índice compuesto para consultas rápidas por usuario y tipo
CREATE INDEX `idx_user_type` ON `admin_sanctions` (`user_id`, `sanction_type`);

-- Índice para consultas por fecha
CREATE INDEX `idx_created_at` ON `admin_sanctions` (`created_at`);

-- ================================================
-- CONSULTAS ÚTILES
-- ================================================

-- Ver historial completo de un usuario
-- SELECT * FROM admin_sanctions WHERE user_id = ? ORDER BY created_at DESC;

-- Ver advertencias de un usuario
-- SELECT * FROM admin_sanctions WHERE user_id = ? AND sanction_type = 'WARN' ORDER BY created_at DESC;

-- Ver estadísticas de sanciones de un admin
-- SELECT sanction_type, COUNT(*) as total FROM admin_sanctions WHERE admin_id = ? GROUP BY sanction_type;

-- Ver últimas 50 sanciones globales
-- SELECT s.*, u.nickname as user_nickname FROM admin_sanctions s
-- JOIN users u ON s.user_id = u.id
-- ORDER BY s.created_at DESC LIMIT 50;

-- Ver jugadores con más de 2 advertencias
-- SELECT u.nickname, u.warnings FROM users u WHERE u.warnings >= 2 ORDER BY u.warnings DESC;
