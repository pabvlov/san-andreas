# 📦 Sistema de Items Tirados al Suelo

## Descripción General

Sistema que permite a los jugadores **tirar** items de su inventario al suelo y **recoger** items que otros jugadores han dejado. Los items aparecen como objetos 3D en el mundo del juego y se guardan en la base de datos para persistencia.

## 🎮 Comandos del Sistema

### `/tirar [slot]`
Tira un item del inventario al suelo.

**Uso:**
```
/tirar 5    # Tira el item del slot 5
```

**Validaciones:**
- El jugador debe estar logueado
- El slot debe estar en el rango válido (0-19)
- El slot no debe estar vacío
- El item no debe estar equipado (en manos del jugador)
- Debe haber espacio disponible en el sistema (máximo 500 items en el suelo)

**Comportamiento:**
1. El item se coloca al frente del jugador (1.5 metros)
2. Se crea un objeto 3D visual en el mundo
3. Las armas se colocan acostadas (rotación 90°)
4. Los items normales se colocan parados
5. Se elimina del inventario del jugador
6. Se guarda en la base de datos
7. Se reproduce una animación de plantar

### `/recoger`
Recoge el item más cercano del suelo.

**Uso:**
```
/recoger    # Recoge el item más cercano
```

**Validaciones:**
- El jugador debe estar logueado
- Debe haber un item cerca (máximo 2 metros)
- El jugador debe tener espacio en su inventario

**Comportamiento:**
1. Busca el item más cercano dentro del radio de 2 metros
2. Verifica que esté en el mismo interior y mundo virtual
3. Añade el item al inventario del jugador
4. Elimina el objeto 3D del mundo
5. Elimina el registro de la base de datos
6. Se reproduce una animación de agacharse

## 🗄️ Estructura de Base de Datos

### Tabla: `dropped_items`

```sql
CREATE TABLE `dropped_items` (
  `id` INT AUTO_INCREMENT PRIMARY KEY,
  `item_id` INT NOT NULL,               -- ID del item (referencia a items)
  `quantity` INT NOT NULL DEFAULT 1,    -- Cantidad del item
  `metadata` TEXT NULL,                 -- Metadatos (JSON, munición, etc.)
  `pos_x` FLOAT NOT NULL,               -- Posición X
  `pos_y` FLOAT NOT NULL,               -- Posición Y
  `pos_z` FLOAT NOT NULL,               -- Posición Z
  `interior` INT NOT NULL DEFAULT 0,    -- Interior del mundo
  `virtual_world` INT NOT NULL DEFAULT 0, -- Mundo virtual
  `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  INDEX `idx_position` (`pos_x`, `pos_y`, `pos_z`),
  INDEX `idx_item` (`item_id`)
);
```

## 📋 Estructura del Código

### Archivo: `include/dropped_items.inc`

#### Constantes
```pawn
#define MAX_DROPPED_ITEMS 500    // Máximo de items en el suelo simultáneamente
#define PICKUP_DISTANCE 2.0      // Distancia máxima para recoger (metros)
```

#### Enumeración: `E_DROPPED_ITEM`
```pawn
enum E_DROPPED_ITEM
{
    dItemID,          // ID del item
    dQuantity,        // Cantidad
    dMetadata[128],   // Metadatos en JSON
    Float:dPosX,      // Posición X
    Float:dPosY,      // Posición Y
    Float:dPosZ,      // Posición Z
    dInterior,        // Interior
    dVirtualWorld,    // Mundo virtual
    dObjectID,        // ID del objeto 3D en el mundo
    dDBID,            // ID en la base de datos
    bool:dActive      // Si el slot está activo
}
```

#### Array Global
```pawn
new DroppedItems[MAX_DROPPED_ITEMS][E_DROPPED_ITEM];
```

### Funciones Principales

#### `LoadDroppedItems()`
Carga todos los items tirados desde la base de datos al iniciar el servidor.

**Proceso:**
1. Consulta todos los items de `dropped_items`
2. Crea los objetos 3D en el mundo
3. Configura la rotación (armas acostadas, otros parados)
4. Marca los slots como activos

**Retorna:** Cantidad de items cargados

---

#### `DropItemToGround(playerid, slot)`
Tira un item del inventario al suelo.

**Parámetros:**
- `playerid` - ID del jugador
- `slot` - Slot del inventario (0-19)

**Proceso:**
1. Valida el slot y que no esté vacío
2. Verifica que el item no esté equipado
3. Busca un slot libre en `DroppedItems`
4. Calcula la posición frente al jugador
5. Crea el objeto 3D en el mundo
6. Guarda en la base de datos
7. Elimina el item del inventario

**Retorna:** Índice del slot en `DroppedItems` o -1 si falla

---

#### `PickupDroppedItem(playerid, dropSlot)`
Recoge un item específico del suelo.

**Parámetros:**
- `playerid` - ID del jugador
- `dropSlot` - Índice en el array `DroppedItems`

**Proceso:**
1. Verifica que el slot esté activo
2. Calcula la distancia al jugador
3. Valida distancia máxima (2 metros)
4. Añade el item al inventario del jugador
5. Destruye el objeto 3D
6. Elimina de la base de datos
7. Libera el slot

**Retorna:** 1 si éxito, 0 si falla

---

#### `GetNearestDroppedItem(playerid, Float:maxDistance)`
Busca el item más cercano al jugador.

**Parámetros:**
- `playerid` - ID del jugador
- `maxDistance` - Distancia máxima de búsqueda (default: 2.0)

**Proceso:**
1. Obtiene la posición del jugador
2. Itera todos los items activos
3. Filtra por interior y mundo virtual
4. Calcula distancia euclidiana 3D
5. Retorna el más cercano dentro del radio

**Retorna:** Índice del item más cercano o -1 si no hay ninguno

---

#### `GetItemObjectModel(itemid)`
Obtiene el modelo 3D del objeto según el tipo de item.

**Parámetros:**
- `itemid` - ID del item

**Proceso:**
1. Busca en la tabla de armas
2. Busca en la tabla de cargadores
3. Usa modelos específicos para items conocidos
4. Modelo genérico (1279 - caja) para desconocidos

**Modelos Específicos:**
- Llaves de vehículo: 2543
- Teléfono: 330
- Kit de reparación: 1279
- Hamburguesa: 2703
- Agua: 1484

**Retorna:** ID del modelo del objeto

---

#### `IsItemWeapon(itemid)`
Verifica si un item es un arma.

**Parámetros:**
- `itemid` - ID del item

**Retorna:** 1 si es arma, 0 si no

## 🔧 Integración con Otros Sistemas

### Sistema de Inventario
- Usa `RemovePlayerItem()` para eliminar items del inventario
- Usa `GivePlayerItem()` para añadir items al inventario
- Respeta el límite de `MAX_INVENTORY_SLOTS`

### Sistema de Armas
- Detecta automáticamente si un item es un arma
- Usa el modelo de objeto correcto para cada arma
- Coloca las armas acostadas (rotación 90°) para mayor realismo

### Sistema de Mundos Virtuales
- Los items respetan el `interior` y `virtual_world`
- Solo se pueden recoger items del mismo mundo e interior

## 🎨 Características Visuales

### Objetos 3D
- **Armas:** Se colocan acostadas (rotación X: 90°)
- **Items normales:** Se colocan parados (rotación X: 0°)
- **Altura:** Se ajusta -0.9 unidades para estar al nivel del suelo
- **Modelos:** Específicos según el tipo de item

### Animaciones
- **Tirar:** Animación "BOMBER/BOM_PLANT" (plantar bomba)
- **Recoger:** Animación "BOMBER/BOM_PLANT" (agacharse)

### Mensajes
- **Tirar exitoso:** `{FFD700}Has tirado: {FFFFFF}[Item] {AAAAAA}(x[cantidad]) {FFD700}al suelo`
- **Recoger exitoso:** `{00FF00}Has recogido: {FFFFFF}[Item] {AAAAAA}(x[cantidad])`
- **Errores:** Mensajes en rojo con descripción clara

## 📝 Logs del Sistema

### Servidor
```
[Dropped Items] Cargando X items del suelo...
[Dropped Items] PlayerName tiró ItemID=X (xX) en pos=(X.XX, X.XX, X.XX)
[Dropped Items] PlayerName recogió ItemID=X (xX)
```

## ⚙️ Configuración

### Límites del Sistema
- **Máximo items en el suelo:** 500 items simultáneamente
- **Distancia de recogida:** 2.0 metros
- **Distancia de tirado:** 1.5 metros frente al jugador

### Persistencia
- Todos los items se guardan en la base de datos
- Se cargan automáticamente al iniciar el servidor
- Persisten entre reinicios

## 🚀 Mejoras Futuras Potenciales

1. **Sistema de Despawn Automático**
   - Timer para eliminar items después de X tiempo
   - Configurable por tipo de item

2. **Límites por Zona**
   - Restringir tirar items en zonas específicas
   - Límite de items por área

3. **Efectos Visuales**
   - Partículas al tirar/recoger
   - Iluminación en items raros

4. **Sistema de Búsqueda**
   - Comando `/buscar` para ver items cercanos
   - Lista con distancias

5. **Stack Automático**
   - Combinar items iguales en el suelo
   - Optimización de slots

## 📚 Dependencias

- **Sistema de Inventario** (`inventory_system.inc`)
- **Sistema de Armas** (`weapon_inventory.inc`)
- **MySQL Plugin** (conexión a base de datos)
- **Streamer Plugin** (objetos dinámicos)
- **ZCMD** (procesamiento de comandos)

## 🔍 Debugging

### Verificar Items en el Suelo
```pawn
// Contar items activos
new count = 0;
for(new i = 0; i < MAX_DROPPED_ITEMS; i++)
{
    if(DroppedItems[i][dActive])
        count++;
}
printf("Items activos en el suelo: %d", count);
```

### Ver Items Cercanos
```pawn
// En modo debug, mostrar items cercanos con distancia
for(new i = 0; i < MAX_DROPPED_ITEMS; i++)
{
    if(!DroppedItems[i][dActive]) continue;
    
    new Float:dist = GetPlayerDistanceFromPoint(playerid, 
        DroppedItems[i][dPosX], 
        DroppedItems[i][dPosY], 
        DroppedItems[i][dPosZ]);
    
    if(dist <= 10.0)
    {
        printf("Item %d: ID=%d, Dist=%.2f", 
            i, DroppedItems[i][dItemID], dist);
    }
}
```

## ✅ Testing

### Casos de Prueba

1. **Tirar Item Normal**
   - Tener un item en el inventario
   - `/tirar [slot]`
   - Verificar que aparece en el suelo
   - Verificar que se elimina del inventario

2. **Tirar Item Equipado**
   - Equipar un item
   - `/tirar [slot]`
   - Debe mostrar error

3. **Recoger Item**
   - Acercarse a un item (< 2m)
   - `/recoger`
   - Verificar que se añade al inventario
   - Verificar que desaparece del suelo

4. **Recoger Item Lejos**
   - Estar lejos de un item (> 2m)
   - `/recoger`
   - Debe mostrar error

5. **Persistencia**
   - Tirar items
   - Reiniciar servidor
   - Verificar que los items siguen en el suelo

6. **Mundo Virtual**
   - Tirar item en VW 0
   - Cambiar a VW 1
   - `/recoger` no debe funcionar
   - Volver a VW 0
   - `/recoger` debe funcionar

---

**Fecha de Implementación:** Diciembre 2025  
**Versión:** 1.0  
**Autor:** Sistema de Inventario SA-MP
