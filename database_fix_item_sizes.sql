-- Actualizar tamaños de items existentes
USE `san-andreas`;

-- Items pequeños (tamaño 0, capacidad 1)
UPDATE `items` SET `item_size` = 0, `item_capacity` = 1 
WHERE `item_type` IN ('key', 'food', 'misc');

-- Teléfono es pequeño
UPDATE `items` SET `item_size` = 0, `item_capacity` = 1 
WHERE `id` = 5;

-- Todos los cargadores son pequeños (IDs 36-50)
UPDATE `items` SET `item_size` = 0, `item_capacity` = 1 
WHERE `id` BETWEEN 36 AND 50;

-- Items grandes (tamaño 1, capacidad 1)
-- Todas las armas son grandes (IDs 9-35)
UPDATE `items` SET `item_size` = 1, `item_capacity` = 1 
WHERE `id` BETWEEN 9 AND 35;

-- Kit de reparación es grande
UPDATE `items` SET `item_size` = 1, `item_capacity` = 1 
WHERE `id` = 6;

-- Verificar resultados
SELECT 
    id, 
    name, 
    item_type,
    item_size,
    item_capacity,
    CASE 
        WHEN item_size = 0 THEN 'Pequeño (bolsillo)'
        WHEN item_size = 1 THEN 'Grande (espalda)'
    END as Tamaño
FROM `items` 
ORDER BY id;
