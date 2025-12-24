-- Sistema de Heridas por Parte del Cuerpo
-- Registra cada impacto de bala por bodypart con el arma usada

CREATE TABLE IF NOT EXISTS `character_wounds` (
    `id` INT AUTO_INCREMENT PRIMARY KEY,
    `character_id` INT NOT NULL,
    `bodypart` TINYINT NOT NULL COMMENT '3=Torso, 4=Ingle, 5=Brazo Izq, 6=Brazo Der, 7=Pierna Izq, 8=Pierna Der, 9=Cabeza',
    `weapon_id` TINYINT NOT NULL COMMENT 'ID del arma que causó la herida',
    `damage` FLOAT NOT NULL COMMENT 'Daño causado por esta herida',
    `timestamp` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    `healed` BOOLEAN DEFAULT FALSE COMMENT 'Si la herida fue curada',
    INDEX `idx_character` (`character_id`),
    INDEX `idx_healed` (`healed`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tabla para configuración de daño por arma y bodypart
CREATE TABLE IF NOT EXISTS `weapon_damage_config` (
    `weapon_id` TINYINT PRIMARY KEY,
    `weapon_name` VARCHAR(50),
    `base_damage` FLOAT DEFAULT 10.0,
    `head_multiplier` FLOAT DEFAULT 3.0,
    `torso_multiplier` FLOAT DEFAULT 1.0,
    `groin_multiplier` FLOAT DEFAULT 1.2,
    `arm_multiplier` FLOAT DEFAULT 0.5,
    `leg_multiplier` FLOAT DEFAULT 0.7
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Configuración inicial de armas comunes
INSERT INTO `weapon_damage_config` (weapon_id, weapon_name, base_damage, head_multiplier, torso_multiplier, groin_multiplier, arm_multiplier, leg_multiplier) VALUES
(22, 'Colt 45', 8.0, 3.5, 1.0, 1.2, 0.5, 0.7),
(23, 'Silenced Pistol', 10.0, 3.5, 1.0, 1.2, 0.5, 0.7),
(24, 'Desert Eagle', 15.0, 4.0, 1.0, 1.2, 0.5, 0.7),
(25, 'Shotgun', 20.0, 5.0, 1.5, 1.8, 0.8, 1.0),
(26, 'Sawnoff Shotgun', 18.0, 5.0, 1.5, 1.8, 0.8, 1.0),
(27, 'Combat Shotgun', 22.0, 5.0, 1.5, 1.8, 0.8, 1.0),
(28, 'Micro Uzi', 6.0, 3.0, 1.0, 1.2, 0.5, 0.7),
(29, 'MP5', 8.0, 3.0, 1.0, 1.2, 0.5, 0.7),
(30, 'AK-47', 12.0, 3.5, 1.0, 1.2, 0.5, 0.7),
(31, 'M4', 11.0, 3.5, 1.0, 1.2, 0.5, 0.7),
(33, 'Rifle', 25.0, 6.0, 1.5, 1.8, 0.8, 1.0),
(34, 'Sniper Rifle', 40.0, 8.0, 2.0, 2.5, 1.0, 1.5)
ON DUPLICATE KEY UPDATE weapon_name=VALUES(weapon_name);
