# Sistema de Inventario de Armas

Sistema completo de armas y cargadores para el servidor SA-MP. Las armas se almacenan en el inventario como items y deben equiparse en las manos para ser usadas.

## Características

- **Armas en inventario**: Las armas son items que deben ser equipados para usarse
- **Sistema de manos**: Mano derecha para armas, mano izquierda para cargadores
- **Recarga realista**: Los cargadores deben equiparse en la mano izquierda y usar `/usar` para recargar
- **Sin munición infinita**: Cada arma tiene capacidad limitada y consume balas al disparar
- **Sistema de compatibilidad**: Cada cargador es compatible solo con armas específicas

## Estructura de Base de Datos

### Tabla: weapons
```sql
CREATE TABLE IF NOT EXISTS weapons (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(64) NOT NULL,
    weapon_slot TINYINT NOT NULL,
    weapon_id_samp TINYINT NOT NULL,
    ammo_capacity INT NOT NULL,
    hand_object_id INT NOT NULL,
    item_id INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### Tabla: weapon_magazines
```sql
CREATE TABLE IF NOT EXISTS weapon_magazines (
    id INT PRIMARY KEY AUTO_INCREMENT,
    weapon_id INT NOT NULL,
    name VARCHAR(64) NOT NULL,
    magazine_capacity INT NOT NULL,
    hand_object_id INT NOT NULL,
    item_id INT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (weapon_id) REFERENCES weapons(id)
);
```

## Instalación

1. **Ejecutar SQL de items de cargadores**: 
   ```bash
   Importa components/database_weapon_magazines_items.sql en tu base de datos
   ```
   Esto creará los items de cargadores (IDs 36-50 si tus armas terminan en 35)

2. **Verificar IDs generados**:
   ```sql
   SELECT * FROM items WHERE item_type = 'misc' AND subtype = 'ammo';
   ```
   Anota los IDs generados para cada cargador

3. **Ejecutar SQL de armas**: 
   ```bash
   Importa components/database_weapons.sql en tu base de datos
   ```
   Esto creará las tablas `weapons` y `weapon_magazines` con las referencias correctas

4. **Ajustar IDs de cargadores** (si es necesario):
   Si los IDs de items de cargadores no son 36-50, actualiza la tabla weapon_magazines:
   ```sql
   UPDATE weapon_magazines SET item_id = [ID_REAL] WHERE id = 1;
   -- Repetir para cada cargador
   ```

5. **Compilar**: El sistema ya está incluido en derby.pwn

## Uso del Sistema

### Para Jugadores

#### Equipar un Arma
1. Obtén un arma en tu inventario (admin te la puede dar)
2. Abre el inventario con `/inventario` o tecla `Y`
3. Selecciona "Equipar en bolsillo"
4. El arma se equipará automáticamente en la mano derecha
5. **IMPORTANTE**: El arma comienza sin munición

#### Equipar un Cargador
1. Obtén un cargador compatible en tu inventario
2. Abre el inventario y selecciona el cargador
3. Selecciona "Equipar en mano izquierda"
4. El cargador se equipará en la mano izquierda

#### Recargar el Arma
1. Equipa el arma en mano derecha
2. Equipa un cargador compatible en mano izquierda
3. Usa el comando `/usar`
4. El arma se recargará automáticamente consumiendo el cargador

#### Guardar Arma o Cargador
- Usa el comando `/guardar` para desequipar el item de la mano

### Para Administradores

#### Dar un Arma
```pawn
/dararma [playerid] [weaponid] [cantidad]
```
- `weaponid` es el ID del arma en la tabla weapons (1-27)
- Ejemplo: `/dararma 0 1 1` (da 1 Pistola 9mm al jugador ID 0)
- Ejemplo: `/dararma 0 10 1` (da 1 AK-47 al jugador ID 0)

#### Dar un Cargador
```pawn
/darcargador [playerid] [magazineid] [cantidad]
```
- `magazineid` es el ID del cargador en la tabla weapon_magazines (1-15)
- Ejemplo: `/darcargador 0 1 5` (da 5 cargadores de 9mm al jugador ID 0)
- Ejemplo: `/darcargador 0 10 10` (da 10 cargadores de AK-47 al jugador ID 0)

**Atajo con item_id directamente:**
```pawn
/giveitem [playerid] [itemid] [cantidad]
```
- Ejemplo: `/giveitem 0 9 1` (da Pistola 9mm - item ID 9)
- Ejemplo: `/giveitem 0 36 5` (da 5 Cargadores 9mm - item ID 36)

#### Listar Armas Disponibles
```pawn
/listarmas
```
Muestra todas las armas con sus IDs, nombres, capacidades y item_id

#### Listar Cargadores Disponibles
```pawn
/listcargadores
```
Muestra todos los cargadores con sus IDs, nombres, compatibilidad y capacidades

## Armas Incluidas

### Pistolas (IDs en items: 9-11)
- **Arma ID 1 / Item 9**: Pistola 9mm (17 balas, SAMP ID 22)
- **Arma ID 2 / Item 10**: Pistola Silenciada (17 balas, SAMP ID 23)
- **Arma ID 3 / Item 11**: Desert Eagle (7 balas, SAMP ID 24)

### Escopetas (IDs en items: 12-14)
- **Arma ID 4 / Item 12**: Escopeta (1 bala, SAMP ID 25)
- **Arma ID 5 / Item 13**: Escopeta Recortada (2 balas, SAMP ID 26)
- **Arma ID 6 / Item 14**: Escopeta de Combate (7 balas, SAMP ID 27)

### Subfusiles (IDs en items: 15-17)
- **Arma ID 7 / Item 15**: Micro SMG (50 balas, SAMP ID 28)
- **Arma ID 8 / Item 16**: MP5 (30 balas, SAMP ID 29)
- **Arma ID 9 / Item 17**: Tec-9 (50 balas, SAMP ID 32)

### Rifles de Asalto (IDs en items: 18-19)
- **Arma ID 10 / Item 18**: AK-47 (30 balas, SAMP ID 30)
- **Arma ID 11 / Item 19**: M4 (50 balas, SAMP ID 31)

### Rifles (IDs en items: 20-21)
- **Arma ID 12 / Item 20**: Rifle de Caza (1 bala, SAMP ID 33)
- **Arma ID 13 / Item 21**: Rifle de Francotirador (1 bala, SAMP ID 34)

### Armas Pesadas (IDs en items: 22-23)
- **Arma ID 14 / Item 22**: Lanzacohetes (1 cohete, SAMP ID 35)
- **Arma ID 15 / Item 23**: Lanzacohetes HS (1 cohete, SAMP ID 36)

### Armas Cuerpo a Cuerpo (IDs en items: 24-32)
- **Arma ID 16 / Item 24**: Nudillos de Bronce (SAMP ID 1)
- **Arma ID 17 / Item 25**: Palo de Golf (SAMP ID 2)
- **Arma ID 18 / Item 26**: Porra (SAMP ID 3)
- **Arma ID 19 / Item 27**: Cuchillo (SAMP ID 4)
- **Arma ID 20 / Item 28**: Bate de Béisbol (SAMP ID 5)
- **Arma ID 21 / Item 29**: Pala (SAMP ID 6)
- **Arma ID 22 / Item 30**: Taco de Billar (SAMP ID 7)
- **Arma ID 23 / Item 31**: Katana (SAMP ID 8)
- **Arma ID 24 / Item 32**: Motosierra (SAMP ID 9)

### Armas Lanzables (IDs en items: 33-35)
- **Arma ID 25 / Item 33**: Granada (SAMP ID 16)
- **Arma ID 26 / Item 34**: Granada de Humo (SAMP ID 17)
- **Arma ID 27 / Item 35**: Molotov (SAMP ID 18)

## Cargadores Incluidos (IDs en items: 36-50)

1. **Cargador ID 1 / Item 36**: Cargador 9mm - Compatible con Pistola 9mm (17 balas)
2. **Cargador ID 2 / Item 37**: Cargador Pistola Silenciada - Compatible con Pistola Silenciada (17 balas)
3. **Cargador ID 3 / Item 38**: Cargador Desert Eagle - Compatible con Desert Eagle (7 balas)
4. **Cargador ID 4 / Item 39**: Cartuchos Escopeta - Compatible con Escopeta (1 bala)
5. **Cargador ID 5 / Item 40**: Cartuchos Escopeta Recortada - Compatible con Escopeta Recortada (2 balas)
6. **Cargador ID 6 / Item 41**: Cartuchos Escopeta Combate - Compatible con Escopeta de Combate (7 balas)
7. **Cargador ID 7 / Item 42**: Cargador Micro SMG - Compatible con Micro SMG (50 balas)
8. **Cargador ID 8 / Item 43**: Cargador MP5 - Compatible con MP5 (30 balas)
9. **Cargador ID 9 / Item 44**: Cargador Tec-9 - Compatible con Tec-9 (50 balas)
10. **Cargador ID 10 / Item 45**: Cargador AK-47 - Compatible con AK-47 (30 balas)
11. **Cargador ID 11 / Item 46**: Cargador M4 - Compatible con M4 (50 balas)
12. **Cargador ID 12 / Item 47**: Cartuchos Rifle de Caza - Compatible con Rifle de Caza (1 bala)
13. **Cargador ID 13 / Item 48**: Cartuchos Francotirador - Compatible con Francotirador (1 bala)
14. **Cargador ID 14 / Item 49**: Cohete RPG - Compatible con Lanzacohetes (1 cohete)
15. **Cargador ID 15 / Item 50**: Cohete HS - Compatible con Lanzacohetes HS (1 cohete)

## Flujo de Trabajo

```
1. Admin da arma y cargadores al jugador
   ↓
2. Jugador equipa arma en mano derecha (sin munición)
   ↓
3. Jugador equipa cargador en mano izquierda
   ↓
4. Jugador usa /usar para recargar
   ↓
5. El arma se recarga, consumiendo el cargador
   ↓
6. Jugador puede disparar hasta quedarse sin munición
   ↓
7. Repetir desde paso 3 para recargar nuevamente
```

## Notas Importantes

- **Sin /giveweapon**: El comando clásico no se usa, solo items del inventario
- **Compatibilidad**: Un cargador de AK-47 NO funciona con un M4
- **Consumo**: Los cargadores se consumen al recargar (1 cargador = 1 recarga)
- **Objetos 3D**: Las armas y cargadores se muestran en las manos del jugador
- **Munición limitada**: Al disparar, la munición disminuye automáticamente
- **Recarga parcial**: Si el arma tiene 5 balas y agregas un cargador de 17, solo se añaden 12
- **Manos ocupadas**: No puedes equipar si las manos están ocupadas, debes guardar primero

## Problemas Comunes

### "No tienes munición"
- Asegúrate de tener un cargador compatible en la mano izquierda
- Usa `/usar` para recargar

### "Este cargador no es compatible"
- Verifica que el cargador sea para el arma que tienes equipada
- Ejemplo: Cargador de Pistola 9mm solo funciona con Pistola 9mm

### "Tu mano está ocupada"
- Usa `/guardar` para liberar la mano
- No puedes tener dos items en la misma mano

### "item_id NULL en database"
- Debes crear los items en la tabla `items` primero
- Luego actualizar las tablas `weapons` y `weapon_magazines` con los item_id correctos

## Archivos del Sistema

- `include/weapon_inventory.inc` - Sistema principal de armas
- `include/inventory_system.inc` - Integración con inventario
- `components/database_weapons.sql` - Estructura de base de datos
- `gamemodes/derby.pwn` - Callbacks e inicialización

## Comandos Relacionados

- `/inventario` o `Y` - Ver inventario
- `/usar` - Recargar arma (si hay arma + cargador equipados)
- `/guardar` - Desequipar item de las manos
- `/dararma` - [Admin] Dar arma
- `/darcargador` - [Admin] Dar cargador
- `/listarmas` - [Admin] Lista de armas
- `/listcargadores` - [Admin] Lista de cargadores
