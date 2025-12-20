-- Actualización de la tabla characters para agregar age y gender

ALTER TABLE `characters` 
ADD COLUMN `age` TINYINT NOT NULL DEFAULT 25 AFTER `name`,
ADD COLUMN `gender` TINYINT NOT NULL DEFAULT 1 COMMENT '1=Masculino, 2=Femenino' AFTER `age`;

-- Actualizar personajes existentes con valores por defecto
UPDATE `characters` SET `age` = 25, `gender` = 1 WHERE `age` = 0;
