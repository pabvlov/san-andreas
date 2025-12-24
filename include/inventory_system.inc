/*
    Sistema de Inventario - Items y Objetos
*/

#if defined _inventory_system_included
    #endinput
#endif
#define _inventory_system_included

#define MAX_INVENTORY_SLOTS 9  // 2 manos + 1 espalda + 6 bolsillos pequeños

// Weapon system constants
#define MAX_WEAPON_TYPES 50
#define MAX_MAGAZINE_TYPES 50

// Tamaños de items
#define ITEM_SIZE_SMALL 0  // Pequeño - ocupa 1 slot pequeño
#define ITEM_SIZE_LARGE 1  // Grande - solo puede ir en espalda

// Tipos de slots
#define SLOT_TYPE_HAND_LEFT 0    // Mano izquierda (slot 0)
#define SLOT_TYPE_HAND_RIGHT 1   // Mano derecha (slot 1)
#define SLOT_TYPE_BACK 2         // Espalda (slot 2) - items grandes visibles
#define SLOT_TYPE_POCKET_SMALL 3 // Bolsillo pequeño (slots 3-8)

// Datos de armas
enum E_WEAPON_DATA
{
    wID,
    wName[64],
    wSlot,
    wSAMPID,
    wAmmoCapacity,
    wHandObject,
    wItemID,
    wItemSize // 0 = pequeño, 1 = grande (desde BD)
}

new WeaponData[MAX_WEAPON_TYPES][E_WEAPON_DATA];
new TotalWeapons = 0;

// Datos de cargadores
enum E_MAGAZINE_DATA
{
    mID,
    mWeaponID,
    mName[64],
    mCapacity,
    mHandObject,
    mItemID,
    mItemSize // 0 = pequeño, 1 = grande (desde BD)
}

new MagazineData[MAX_MAGAZINE_TYPES][E_MAGAZINE_DATA];
new TotalMagazines = 0;

// Variables de armas (usadas por weapon_inventory.inc)
new PlayerCurrentWeapon[MAX_PLAYERS] = {-1, ...};  // ID del arma equipada (-1 = ninguna)
new PlayerCurrentAmmo[MAX_PLAYERS] = {0, ...};     // Munición actual en el arma
new PlayerWeaponSlot[MAX_PLAYERS] = {-1, ...};     // Slot del inventario donde está el arma

#define ITEM_TYPE_KEY 1
#define ITEM_TYPE_WEAPON 2

// IDs de items (deben coincidir con la BD)
#define ITEM_VEHICLE_KEY_MASTER 1  // Llave maestra
#define ITEM_VEHICLE_KEY 2          // Llave normal
// IDs 9-35: Armas (ver database_weapons.sql)
// IDs 36-50: Cargadores (ver database_weapons.sql)
#define ITEM_PHONE 51
#define ITEM_REPAIR_KIT 52
#define ITEM_BURGER 53
#define ITEM_WATER 54

// Enumeración de items
enum E_INVENTORY_ITEM
{
    invItemID,
    invCharacterID,
    invSlot,
    invQuantity,
    invMetadata[128],
    invItemSize // ITEM_SIZE_SMALL o ITEM_SIZE_LARGE
}

// Enumeración de slots
enum E_INVENTORY_SLOT
{
    slotType,      // SLOT_TYPE_*
    slotSize,      // ITEM_SIZE_SMALL o ITEM_SIZE_LARGE
    slotOccupied,  // 0 = vacío, 1 = ocupado parcialmente, 2 = lleno
    slotItemCount  // Cantidad de items en el slot (máximo 2 para slots grandes con items pequeños)
}

new PlayerInventory[MAX_PLAYERS][MAX_INVENTORY_SLOTS][E_INVENTORY_ITEM];
new PlayerSlots[MAX_PLAYERS][MAX_INVENTORY_SLOTS][E_INVENTORY_SLOT];
new PlayerInventoryCount[MAX_PLAYERS];
new PlayerBackSlot[MAX_PLAYERS] = {-1, ...}; // Slot de espalda (-1 = vacío, >=0 = índice del item)

// Objetos equipados en manos
new PlayerHeldObjectLeft[MAX_PLAYERS] = {INVALID_OBJECT_ID, ...};
new PlayerHeldObjectRight[MAX_PLAYERS] = {INVALID_OBJECT_ID, ...};
new PlayerHeldItemSlotLeft[MAX_PLAYERS] = {-1, ...};
new PlayerHeldItemSlotRight[MAX_PLAYERS] = {-1, ...};
new bool:PlayerHoldingLargeItem[MAX_PLAYERS] = {false, ...}; // Si tiene item grande ocupando ambas manos

// Datos de items equipados (cuando se limpian del slot)
new PlayerHeldItemIDLeft[MAX_PLAYERS] = {0, ...};
new PlayerHeldItemIDRight[MAX_PLAYERS] = {0, ...};
new PlayerHeldItemQuantityLeft[MAX_PLAYERS] = {0, ...};
new PlayerHeldItemQuantityRight[MAX_PLAYERS] = {0, ...};
new PlayerHeldItemMetadataLeft[MAX_PLAYERS][256];
new PlayerHeldItemMetadataRight[MAX_PLAYERS][256];

// Posiciones personalizadas de objetos attachados
enum E_ATTACHED_OBJECT_POS
{
    Float:aoPosX,
    Float:aoPosY,
    Float:aoPosZ,
    Float:aoRotX,
    Float:aoRotY,
    Float:aoRotZ,
    Float:aoScaleX,
    Float:aoScaleY,
    Float:aoScaleZ
}

// Guardar posiciones por item_id y slot_index
// Key: "itemid_slotindex" (ej: "5_0" para item 5 en slot 0)
new PlayerAttachedPositions[MAX_PLAYERS][50][E_ATTACHED_OBJECT_POS]; // Máximo 50 items diferentes
new PlayerAttachedPositionsItemID[MAX_PLAYERS][50]; // IDs de items que tienen posición guardada
new PlayerAttachedPositionsSlot[MAX_PLAYERS][50]; // Slots correspondientes
new PlayerAttachedPositionsCount[MAX_PLAYERS]; // Cantidad de posiciones guardadas

// Mantener compatibilidad
#define PlayerHeldObject PlayerHeldObjectRight
#define PlayerHeldItemSlot PlayerHeldItemSlotRight

// Inicializar slots del jugador con sus tipos y tamaños
stock InitializePlayerSlots(playerid)
{
    // Slot 0: Mano izquierda
    PlayerSlots[playerid][0][slotType] = SLOT_TYPE_HAND_LEFT;
    PlayerSlots[playerid][0][slotSize] = ITEM_SIZE_SMALL;
    PlayerSlots[playerid][0][slotOccupied] = 0;
    PlayerSlots[playerid][0][slotItemCount] = 0;
    
    // Slot 1: Mano derecha
    PlayerSlots[playerid][1][slotType] = SLOT_TYPE_HAND_RIGHT;
    PlayerSlots[playerid][1][slotSize] = ITEM_SIZE_SMALL;
    PlayerSlots[playerid][1][slotOccupied] = 0;
    PlayerSlots[playerid][1][slotItemCount] = 0;
    
    // Slot 2: Espalda - slot grande visible para items grandes
    PlayerSlots[playerid][2][slotType] = SLOT_TYPE_BACK;
    PlayerSlots[playerid][2][slotSize] = ITEM_SIZE_LARGE;
    PlayerSlots[playerid][2][slotOccupied] = 0;
    PlayerSlots[playerid][2][slotItemCount] = 0;
    
    // Slots 3-8: Bolsillos pequeños (6 slots)
    for(new i = 3; i < MAX_INVENTORY_SLOTS; i++)
    {
        PlayerSlots[playerid][i][slotType] = SLOT_TYPE_POCKET_SMALL;
        PlayerSlots[playerid][i][slotSize] = ITEM_SIZE_SMALL;
        PlayerSlots[playerid][i][slotOccupied] = 0;
        PlayerSlots[playerid][i][slotItemCount] = 0;
    }
    
    PlayerBackSlot[playerid] = -1;
    PlayerHoldingLargeItem[playerid] = false;
}

// Obtener tamaño de un item
stock GetItemSize(itemid)
{
    // Verificar si es un arma - leer tamaño desde BD
    for(new i = 0; i < TotalWeapons; i++)
    {
        if(WeaponData[i][wItemID] == itemid)
        {
            // Retornar tamaño cargado desde la base de datos
            return WeaponData[i][wItemSize];
        }
    }
    
    // Verificar si es un cargador - leer tamaño desde BD
    for(new i = 0; i < TotalMagazines; i++)
    {
        if(MagazineData[i][mItemID] == itemid)
        {
            // Retornar tamaño cargado desde la base de datos
            return MagazineData[i][mItemSize];
        }
    }
    
    // Items específicos
    switch(itemid)
    {
        // Items pequeños
        case ITEM_VEHICLE_KEY_MASTER, ITEM_VEHICLE_KEY, ITEM_PHONE, ITEM_BURGER, ITEM_WATER:
            return ITEM_SIZE_SMALL;
        
        // Items grandes
        case ITEM_REPAIR_KIT:
            return ITEM_SIZE_LARGE;
    }
    
    // Por defecto, items pequeños
    return ITEM_SIZE_SMALL;
}

// Cargar inventario del personaje
stock LoadPlayerInventory(playerid)
{
    // Inicializar estructura de slots
    InitializePlayerSlots(playerid);
    
    // Cargar posiciones personalizadas de objetos attachados
    LoadPlayerAttachedPositions(playerid);
    
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT ci.id, ci.item_id, ci.quantity, ci.slot, ci.metadata \
        FROM character_inventory ci WHERE ci.character_id = %d ORDER BY ci.slot",
        CharacterData[playerid][cID]);
    
    new Cache:result = mysql_query(g_MySQL, query);
    new rows = cache_num_rows();
    
    PlayerInventoryCount[playerid] = 0;
    
    for(new i = 0; i < rows && i < MAX_INVENTORY_SLOTS; i++)
    {
        new slotNum;
        cache_get_value_name_int(i, "slot", slotNum);
        
        // Cargar en el slot correcto (no por índice i)
        cache_get_value_name_int(i, "item_id", PlayerInventory[playerid][slotNum][invItemID]); // FIX: era ci.id, ahora ci.item_id
        PlayerInventory[playerid][slotNum][invCharacterID] = CharacterData[playerid][cID];
        PlayerInventory[playerid][slotNum][invSlot] = slotNum;
        cache_get_value_name_int(i, "quantity", PlayerInventory[playerid][slotNum][invQuantity]);
        cache_get_value_name(i, "metadata", PlayerInventory[playerid][slotNum][invMetadata], 128);
        
        // Asignar tamaño del item
        PlayerInventory[playerid][slotNum][invItemSize] = GetItemSize(PlayerInventory[playerid][slotNum][invItemID]);
        
        printf("[Inventory] - Slot %d: ItemID=%d, Quantity=%d, Size=%d", 
            slotNum, PlayerInventory[playerid][slotNum][invItemID], 
            PlayerInventory[playerid][slotNum][invQuantity],
            PlayerInventory[playerid][slotNum][invItemSize]);
        
        PlayerInventoryCount[playerid]++;
    }
    
    cache_delete(result);
    
    printf("[Inventory] Cargados %d items para %s", PlayerInventoryCount[playerid], CharacterData[playerid][cName]);
    
    // Auto-equipar arma si tenía el flag "equipped"
    for(new i = 0; i < MAX_INVENTORY_SLOTS; i++)
    {
        if(PlayerInventory[playerid][i][invItemID] > 0)
        {
            new metadata[128];
            format(metadata, sizeof(metadata), "%s", PlayerInventory[playerid][i][invMetadata]);
            
            // Buscar flag equipped:1
            if(strfind(metadata, "\"equipped\":1") != -1)
            {
                // Es un arma que estaba equipada, equiparla automáticamente
                for(new w = 0; w < TotalWeapons; w++)
                {
                    if(WeaponData[w][wItemID] == PlayerInventory[playerid][i][invItemID])
                    {
                        EquipWeaponFromInventory(playerid, i);
                        printf("[Inventory] Auto-equipando arma del slot %d (estaba equipada al desconectar)", i);
                        break;
                    }
                }
            }
            
            // Auto-attachar item grande en la espalda (slot 2)
            if(i == 2 && GetItemSize(PlayerInventory[playerid][i][invItemID]) == ITEM_SIZE_LARGE)
            {
                AttachItemToBack(playerid, i);
                printf("[Inventory] Auto-attaching item grande en espalda del slot %d", i);
            }
        }
    }
    
    return 1;
}

// Dar item al jugador
stock GivePlayerItem(playerid, itemid, quantity = 1, const metadata[] = "")
{
    new itemSize = GetItemSize(itemid);
    new slot = -1;
    
    // Si es un item grande, intentar colocarlo en el slot 2 (espalda) primero
    if(itemSize == ITEM_SIZE_LARGE)
    {
        if(PlayerInventory[playerid][2][invItemID] == 0)
        {
            slot = 2;
        }
        else
        {
            SendClientMessage(playerid, COLOR_ERROR, "Tu espalda ya tiene un item grande. Guárdalo primero.");
            return 0;
        }
    }
    else
    {
        // Items pequeños: buscar slot libre (preferir bolsillos 3-8)
        for(new i = 3; i < MAX_INVENTORY_SLOTS; i++)
        {
            if(PlayerInventory[playerid][i][invItemID] == 0)
            {
                slot = i;
                break;
            }
        }
        
        // Si no hay bolsillos libres, buscar en slots 0-1 (manos)
        if(slot == -1)
        {
            for(new i = 0; i < 2; i++)
            {
                if(PlayerInventory[playerid][i][invItemID] == 0)
                {
                    slot = i;
                    break;
                }
            }
        }
    }
    
    if(slot == -1)
    {
        SendClientMessage(playerid, COLOR_ERROR, "Tu inventario está lleno.");
        return 0;
    }
    
    // Insertar en base de datos
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO character_inventory (character_id, item_id, quantity, slot, metadata) \
        VALUES (%d, %d, %d, %d, '%e')",
        CharacterData[playerid][cID], itemid, quantity, slot, metadata);
    
    printf("[Inventory] Ejecutando query: %s", query);
    
    new Cache:result = mysql_query(g_MySQL, query);
    new insertid = cache_insert_id();
    cache_delete(result);
    
    printf("[Inventory] Insert ID: %d, MySQL errno: %d", insertid, mysql_errno(g_MySQL));
    
    if(insertid > 0)
    {
        PlayerInventory[playerid][slot][invItemID] = itemid;
        PlayerInventory[playerid][slot][invCharacterID] = CharacterData[playerid][cID];
        PlayerInventory[playerid][slot][invSlot] = slot;
        PlayerInventory[playerid][slot][invQuantity] = quantity;
        format(PlayerInventory[playerid][slot][invMetadata], 128, "%s", metadata);
        PlayerInventory[playerid][slot][invItemSize] = GetItemSize(itemid); // Asignar tamaño del item
        PlayerInventoryCount[playerid]++;
        
        printf("[Inventory] Item %d dado a %s en slot %d (cantidad: %d, tamaño: %d, metadata: %s)", 
            itemid, CharacterData[playerid][cName], slot, quantity, 
            PlayerInventory[playerid][slot][invItemSize], metadata);
        
        // Si es un item grande en el slot de espalda, attacharlo visualmente
        if(itemSize == ITEM_SIZE_LARGE && slot == 2)
        {
            AttachItemToBack(playerid, slot);
        }
        
        return 1;
    }
    
    printf("[Inventory] ERROR: No se pudo insertar item en BD");
    
    return 0;
}

// Obtener cantidad a mostrar (balas si es arma/cargador, cantidad normal si no)
stock GetItemDisplayQuantity(playerid, slot)
{
    new itemid = PlayerInventory[playerid][slot][invItemID];
    new metadata[128];
    format(metadata, sizeof(metadata), "%s", PlayerInventory[playerid][slot][invMetadata]);
    
    return GetItemDisplayQuantityByData(itemid, metadata);
}

// Versión que acepta itemid y metadata directamente
stock GetItemDisplayQuantityByData(itemid, const metadata[])
{
    // Verificar si es un arma
    for(new i = 0; i < TotalWeapons; i++)
    {
        if(WeaponData[i][wItemID] == itemid)
        {
            // Es un arma, buscar balas en metadata
            if(strlen(metadata) > 0)
            {
                // Parsear JSON para obtener ammo
                new pos = strfind(metadata, "ammo\":");
                if(pos != -1)
                {
                    pos += 6; // Saltar "ammo":
                    new ammoStr[16];
                    new idx = 0;
                    while(metadata[pos] >= '0' && metadata[pos] <= '9' && idx < 15)
                    {
                        ammoStr[idx++] = metadata[pos++];
                    }
                    ammoStr[idx] = 0;
                    return strval(ammoStr);
                }
            }
            return 0; // Arma sin balas
        }
    }
    
    // Verificar si es un cargador
    for(new i = 0; i < TotalMagazines; i++)
    {
        if(MagazineData[i][mItemID] == itemid)
        {
            // Es un cargador, buscar balas en metadata
            if(strlen(metadata) > 0)
            {
                // Parsear JSON para obtener bullets
                new pos = strfind(metadata, "bullets\":");
                if(pos != -1)
                {
                    pos += 9; // Saltar "bullets":
                    new bulletsStr[16];
                    new idx = 0;
                    while(metadata[pos] >= '0' && metadata[pos] <= '9' && idx < 15)
                    {
                        bulletsStr[idx++] = metadata[pos++];
                    }
                    bulletsStr[idx] = 0;
                    return strval(bulletsStr);
                }
            }
            return MagazineData[i][mCapacity]; // Cargador lleno por defecto
        }
    }
    
    // No es arma ni cargador, retornar 1
    return 1;
}

// Obtener metadata de un item
stock GetPlayerInventoryMetadata(playerid, slot, metadata[], maxlen)
{
    if(slot < 0 || slot >= MAX_INVENTORY_SLOTS)
    {
        metadata[0] = 0;
        return 0;
    }
    
    if(PlayerInventory[playerid][slot][invItemID] == 0)
    {
        metadata[0] = 0;
        return 0;
    }
    
    format(metadata, maxlen, "%s", PlayerInventory[playerid][slot][invMetadata]);
    return 1;
}

// Remover item del inventario
stock RemovePlayerItem(playerid, slot)
{
    if(slot < 0 || slot >= MAX_INVENTORY_SLOTS)
        return 0;
    
    if(PlayerInventory[playerid][slot][invItemID] == 0)
        return 0;
    
    // Eliminar de BD
    new query[128];
    mysql_format(g_MySQL, query, sizeof(query),
        "DELETE FROM character_inventory WHERE character_id = %d AND slot = %d",
        CharacterData[playerid][cID], slot);
    mysql_query(g_MySQL, query);
    
    // Limpiar memoria
    PlayerInventory[playerid][slot][invItemID] = 0;
    PlayerInventory[playerid][slot][invQuantity] = 0;
    PlayerInventory[playerid][slot][invMetadata][0] = 0;
    PlayerInventoryCount[playerid]--;
    
    return 1;
}

// Attachar item en la espalda (slot 3 = attached object)
stock AttachItemToBack(playerid, slot)
{
    if(slot < 0 || slot >= MAX_INVENTORY_SLOTS)
        return 0;
    
    new itemid = PlayerInventory[playerid][slot][invItemID];
    if(itemid == 0)
        return 0;
    
    // Verificar que sea un item grande
    if(GetItemSize(itemid) != ITEM_SIZE_LARGE)
        return 0;
    
    // Buscar el modelo del arma/item
    new modelid = 0;
    
    // Verificar si es un arma
    for(new i = 0; i < TotalWeapons; i++)
    {
        if(WeaponData[i][wItemID] == itemid)
        {
            modelid = GetWeaponObjectModel(WeaponData[i][wSAMPID]);
            break;
        }
    }
    
    // Si no se encontró el modelo, usar modelo genérico
    if(modelid == 0)
    {
        switch(itemid)
        {
            case ITEM_REPAIR_KIT: modelid = 1279; // Caja de herramientas
            default: return 0;
        }
    }
    
    // Remover object existente en la espalda si hay alguno
    if(IsPlayerAttachedObjectSlotUsed(playerid, 3))
    {
        RemovePlayerAttachedObject(playerid, 3);
    }
    
    // Valores por defecto
    new Float:posX = -0.15, Float:posY = -0.12, Float:posZ = 0.0;
    new Float:rotX = 0.0, Float:rotY = 0.0, Float:rotZ = 90.0;
    new Float:scaleX = 1.0, Float:scaleY = 1.0, Float:scaleZ = 1.0;
    
    // Verificar si hay posición personalizada guardada (slot 3 = espalda)
    if(GetAttachedObjectPosition(playerid, itemid, 3, posX, posY, posZ, rotX, rotY, rotZ, scaleX, scaleY, scaleZ))
    {
        printf("[Attached] Usando posición personalizada para item %d en espalda", itemid);
    }
    
    // Attachar objeto a la espalda (bone 1 = spine)
    SetPlayerAttachedObject(playerid, 3, modelid, 1,
        posX, posY, posZ,    // X, Y, Z offset
        rotX, rotY, rotZ,    // RX, RY, RZ rotation
        scaleX, scaleY, scaleZ);  // Scale
    
    PlayerBackSlot[playerid] = slot;
    
    new itemName[64];
    GetItemName(itemid, itemName);
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Has guardado en tu espalda: %s", itemName);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    return 1;
}

// Remover item de la espalda
stock RemoveItemFromBack(playerid)
{
    if(!IsPlayerAttachedObjectSlotUsed(playerid, 3))
        return 0;
    
    RemovePlayerAttachedObject(playerid, 3);
    PlayerBackSlot[playerid] = -1;
    
    SendClientMessage(playerid, COLOR_INFO, "Has sacado el item de tu espalda.");
    return 1;
}

// Cargar posiciones personalizadas de objetos attachados desde la base de datos
stock LoadPlayerAttachedPositions(playerid)
{
    new characterID = CharacterData[playerid][cID];
    if(characterID == 0)
        return 0;
    
    PlayerAttachedPositionsCount[playerid] = 0;
    
    new query[256];
    format(query, sizeof(query), 
        "SELECT item_id, slot_index, offset_x, offset_y, offset_z, rot_x, rot_y, rot_z, scale_x, scale_y, scale_z FROM character_attached_objects WHERE character_id = %d",
        characterID);
    
    new Cache:result = mysql_query(g_MySQL, query);
    new rows = cache_num_rows();
    
    for(new i = 0; i < rows && i < 50; i++)
    {
        cache_get_value_name_int(i, "item_id", PlayerAttachedPositionsItemID[playerid][i]);
        cache_get_value_name_int(i, "slot_index", PlayerAttachedPositionsSlot[playerid][i]);
        cache_get_value_name_float(i, "offset_x", PlayerAttachedPositions[playerid][i][aoPosX]);
        cache_get_value_name_float(i, "offset_y", PlayerAttachedPositions[playerid][i][aoPosY]);
        cache_get_value_name_float(i, "offset_z", PlayerAttachedPositions[playerid][i][aoPosZ]);
        cache_get_value_name_float(i, "rot_x", PlayerAttachedPositions[playerid][i][aoRotX]);
        cache_get_value_name_float(i, "rot_y", PlayerAttachedPositions[playerid][i][aoRotY]);
        cache_get_value_name_float(i, "rot_z", PlayerAttachedPositions[playerid][i][aoRotZ]);
        cache_get_value_name_float(i, "scale_x", PlayerAttachedPositions[playerid][i][aoScaleX]);
        cache_get_value_name_float(i, "scale_y", PlayerAttachedPositions[playerid][i][aoScaleY]);
        cache_get_value_name_float(i, "scale_z", PlayerAttachedPositions[playerid][i][aoScaleZ]);
        PlayerAttachedPositionsCount[playerid]++;
    }
    
    cache_delete(result);
    
    printf("[Attached Objects] Cargadas %d posiciones personalizadas para %s", 
        PlayerAttachedPositionsCount[playerid], CharacterData[playerid][cName]);
    
    return 1;
}

// Obtener posición personalizada guardada para un item
stock GetAttachedObjectPosition(playerid, itemid, slotIndex, &Float:posX, &Float:posY, &Float:posZ, &Float:rotX, &Float:rotY, &Float:rotZ, &Float:scaleX, &Float:scaleY, &Float:scaleZ)
{
    for(new i = 0; i < PlayerAttachedPositionsCount[playerid]; i++)
    {
        if(PlayerAttachedPositionsItemID[playerid][i] == itemid && PlayerAttachedPositionsSlot[playerid][i] == slotIndex)
        {
            posX = PlayerAttachedPositions[playerid][i][aoPosX];
            posY = PlayerAttachedPositions[playerid][i][aoPosY];
            posZ = PlayerAttachedPositions[playerid][i][aoPosZ];
            rotX = PlayerAttachedPositions[playerid][i][aoRotX];
            rotY = PlayerAttachedPositions[playerid][i][aoRotY];
            rotZ = PlayerAttachedPositions[playerid][i][aoRotZ];
            scaleX = PlayerAttachedPositions[playerid][i][aoScaleX];
            scaleY = PlayerAttachedPositions[playerid][i][aoScaleY];
            scaleZ = PlayerAttachedPositions[playerid][i][aoScaleZ];
            return 1;
        }
    }
    return 0;
}

// Guardar posición personalizada de objeto attachado
stock SaveAttachedObjectPosition(playerid, itemid, slotIndex, Float:posX, Float:posY, Float:posZ, Float:rotX, Float:rotY, Float:rotZ, Float:scaleX, Float:scaleY, Float:scaleZ)
{
    new characterID = CharacterData[playerid][cID];
    if(characterID == 0)
        return 0;
    
    // Buscar si ya existe en memoria
    new idx = -1;
    for(new i = 0; i < PlayerAttachedPositionsCount[playerid]; i++)
    {
        if(PlayerAttachedPositionsItemID[playerid][i] == itemid && PlayerAttachedPositionsSlot[playerid][i] == slotIndex)
        {
            idx = i;
            break;
        }
    }
    
    // Si no existe, agregar nuevo
    if(idx == -1)
    {
        if(PlayerAttachedPositionsCount[playerid] >= 50)
            return 0; // Límite alcanzado
        
        idx = PlayerAttachedPositionsCount[playerid];
        PlayerAttachedPositionsItemID[playerid][idx] = itemid;
        PlayerAttachedPositionsSlot[playerid][idx] = slotIndex;
        PlayerAttachedPositionsCount[playerid]++;
    }
    
    // Actualizar en memoria
    PlayerAttachedPositions[playerid][idx][aoPosX] = posX;
    PlayerAttachedPositions[playerid][idx][aoPosY] = posY;
    PlayerAttachedPositions[playerid][idx][aoPosZ] = posZ;
    PlayerAttachedPositions[playerid][idx][aoRotX] = rotX;
    PlayerAttachedPositions[playerid][idx][aoRotY] = rotY;
    PlayerAttachedPositions[playerid][idx][aoRotZ] = rotZ;
    PlayerAttachedPositions[playerid][idx][aoScaleX] = scaleX;
    PlayerAttachedPositions[playerid][idx][aoScaleY] = scaleY;
    PlayerAttachedPositions[playerid][idx][aoScaleZ] = scaleZ;
    
    // Guardar en base de datos (INSERT ... ON DUPLICATE KEY UPDATE)
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO character_attached_objects (character_id, item_id, slot_index, offset_x, offset_y, offset_z, rot_x, rot_y, rot_z, scale_x, scale_y, scale_z) \
        VALUES (%d, %d, %d, %f, %f, %f, %f, %f, %f, %f, %f, %f) \
        ON DUPLICATE KEY UPDATE offset_x=%f, offset_y=%f, offset_z=%f, rot_x=%f, rot_y=%f, rot_z=%f, scale_x=%f, scale_y=%f, scale_z=%f",
        characterID, itemid, slotIndex, posX, posY, posZ, rotX, rotY, rotZ, scaleX, scaleY, scaleZ,
        posX, posY, posZ, rotX, rotY, rotZ, scaleX, scaleY, scaleZ);
    mysql_tquery(g_MySQL, query);
    
    return 1;
}

// Remover item de la espalda

// Equipar objeto en mano izquierda
stock HoldItemLeft(playerid, slot)
{
    if(slot < 0 || slot >= MAX_INVENTORY_SLOTS)
        return 0;
    
    new itemid = PlayerInventory[playerid][slot][invItemID];
    if(itemid == 0)
        return 0;
    
    // Los cargadores SIEMPRE pueden equiparse en mano izquierda
    if(IsMagazineItem(itemid))
    {
        return EquipMagazineFromInventory(playerid, slot);
    }
    
    // Para otros items, verificar si el jugador tiene un item grande
    if(PlayerHoldingLargeItem[playerid])
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Tienes un item grande ocupando ambas manos. Guárdalo primero.");
        return 0;
    }
    
    // Verificar si es un item grande
    if(GetItemSize(itemid) == ITEM_SIZE_LARGE)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Items grandes solo pueden ir en la espalda.");
        return 0;
    }
    
    // Verificar si es un arma (no puede ir en mano izquierda)
    if(IsWeaponItem(itemid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Las armas deben ir en la mano derecha. Usa la mano izquierda solo para cargadores.");
        return 0;
    }
    
    // Si hay algo equipado, desequipar primero (restaurar al slot)
    if(PlayerHeldObjectLeft[playerid] != INVALID_OBJECT_ID)
    {
        UnholdItemLeft(playerid);
    }
    
    new modelid;
    switch(itemid)
    {
        case ITEM_VEHICLE_KEY_MASTER: modelid = 11746; // DoorKey1
        case ITEM_VEHICLE_KEY: modelid = 11746; // DoorKey1
        case ITEM_PHONE: modelid = 330;
        default: return 0;
    }
    
    // Guardar datos del item antes de limpiar el slot
    PlayerHeldItemIDLeft[playerid] = itemid;
    PlayerHeldItemQuantityLeft[playerid] = PlayerInventory[playerid][slot][invQuantity];
    format(PlayerHeldItemMetadataLeft[playerid], 256, "%s", PlayerInventory[playerid][slot][invMetadata]);
    
    // Valores por defecto para mano izquierda
    new Float:posX = -0.08, Float:posY = 0.03, Float:posZ = 0.0;
    new Float:rotX = 0.0, Float:rotY = -90.0, Float:rotZ = -120.0;
    new Float:scaleX = 1.0, Float:scaleY = 1.0, Float:scaleZ = 1.0;
    
    // Verificar si hay posición personalizada guardada (slot 1 = mano izquierda)
    if(GetAttachedObjectPosition(playerid, itemid, 1, posX, posY, posZ, rotX, rotY, rotZ, scaleX, scaleY, scaleZ))
    {
        printf("[Attached] Usando posición personalizada para item %d en mano izquierda", itemid);
    }
    
    // Attachar objeto a la mano izquierda (bone 5)
    SetPlayerAttachedObject(playerid, 1, modelid, 5,
        posX, posY, posZ,
        rotX, rotY, rotZ,
        scaleX, scaleY, scaleZ);
    
    PlayerHeldObjectLeft[playerid] = modelid;
    PlayerHeldItemSlotLeft[playerid] = slot;
    
    // LIMPIAR el slot del inventario - el item se MUEVE a la mano
    new characterID = PlayerInventory[playerid][slot][invCharacterID];
    PlayerInventory[playerid][slot][invItemID] = 0;
    PlayerInventory[playerid][slot][invQuantity] = 0;
    PlayerInventory[playerid][slot][invMetadata][0] = EOS;
    PlayerSlots[playerid][slot][slotOccupied] = 0;
    PlayerInventoryCount[playerid]--;
    
    // Eliminar de la base de datos
    new query[256];
    format(query, sizeof(query), "DELETE FROM character_inventory WHERE character_id = %d AND slot = %d", 
        characterID, slot);
    mysql_tquery(g_MySQL, query);
    
    new itemName[64];
    GetItemName(itemid, itemName);
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Has equipado en mano izquierda: %s", itemName);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    return 1;
}

// Equipar objeto en mano derecha
stock HoldItem(playerid, slot)
{
    if(slot < 0 || slot >= MAX_INVENTORY_SLOTS)
        return 0;
    
    new itemid = PlayerInventory[playerid][slot][invItemID];
    if(itemid == 0)
        return 0;
    
    // Si el item está en la espalda (slot 2), removerlo visualmente primero
    if(slot == 2 && PlayerBackSlot[playerid] == 2)
    {
        RemoveItemFromBack(playerid);
    }
    
    // Verificar si es un arma (debe ir en mano derecha)
    if(IsWeaponItem(itemid))
    {
        return EquipWeaponFromInventory(playerid, slot);
    }
    
    // Verificar si es un cargador (auto-equipar en mano izquierda)
    if(IsMagazineItem(itemid))
    {
        return HoldItemLeft(playerid, slot);
    }
    
    // Items grandes no pueden equiparse en manos (solo pequeños como llaves, teléfono)
    new itemSize = GetItemSize(itemid);
    if(itemSize == ITEM_SIZE_LARGE)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Este item es muy grande para equipar en la mano.");
        return 0;
    }
    
    // Si hay algo equipado, desequipar primero (restaurar al slot)
    if(PlayerHeldObjectRight[playerid] != INVALID_OBJECT_ID)
    {
        UnholdItem(playerid);
    }
    
    new modelid;
    switch(itemid)
    {
        case ITEM_VEHICLE_KEY_MASTER: modelid = 11746; // DoorKey1 - Llave maestra
        case ITEM_VEHICLE_KEY: modelid = 11746; // DoorKey1 - Llave normal
        case ITEM_PHONE: modelid = 330; // Teléfono
        default: return 0;
    }
    
    // Guardar datos del item antes de limpiar el slot
    PlayerHeldItemIDRight[playerid] = itemid;
    PlayerHeldItemQuantityRight[playerid] = PlayerInventory[playerid][slot][invQuantity];
    format(PlayerHeldItemMetadataRight[playerid], 256, "%s", PlayerInventory[playerid][slot][invMetadata]);
    
    // Valores por defecto para mano derecha
    new Float:posX = 0.08, Float:posY = 0.03, Float:posZ = 0.0;
    new Float:rotX = 0.0, Float:rotY = 90.0, Float:rotZ = 120.0;
    new Float:scaleX = 1.0, Float:scaleY = 1.0, Float:scaleZ = 1.0;
    
    // Verificar si hay posición personalizada guardada (slot 0 = mano derecha)
    if(GetAttachedObjectPosition(playerid, itemid, 0, posX, posY, posZ, rotX, rotY, rotZ, scaleX, scaleY, scaleZ))
    {
        printf("[Attached] Usando posición personalizada para item %d en mano derecha", itemid);
    }
    
    // Attachar objeto a la mano derecha (bone 6)
    SetPlayerAttachedObject(playerid, 0, modelid, 6,
        posX, posY, posZ,
        rotX, rotY, rotZ,
        scaleX, scaleY, scaleZ);
    
    PlayerHeldObjectRight[playerid] = modelid;
    PlayerHeldItemSlotRight[playerid] = slot;
    
    // LIMPIAR el slot del inventario - el item se MUEVE a la mano
    new characterID = PlayerInventory[playerid][slot][invCharacterID];
    PlayerInventory[playerid][slot][invItemID] = 0;
    PlayerInventory[playerid][slot][invQuantity] = 0;
    PlayerInventory[playerid][slot][invMetadata][0] = EOS;
    PlayerSlots[playerid][slot][slotOccupied] = 0;
    PlayerInventoryCount[playerid]--;
    
    // Eliminar de la base de datos
    new query[256];
    format(query, sizeof(query), "DELETE FROM character_inventory WHERE character_id = %d AND slot = %d", 
        characterID, slot);
    mysql_tquery(g_MySQL, query);
    
    new itemName[64];
    GetItemName(itemid, itemName);
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Has equipado en mano derecha: %s", itemName);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    return 1;
}

// Desequipar objeto de mano derecha
stock UnholdItem(playerid)
{
    if(PlayerHeldObjectRight[playerid] == INVALID_OBJECT_ID)
        return 0;
    
    new originalSlot = PlayerHeldItemSlotRight[playerid];
    new itemid = PlayerHeldItemIDRight[playerid];
    new quantity = PlayerHeldItemQuantityRight[playerid];
    new metadata[256];
    format(metadata, sizeof(metadata), "%s", PlayerHeldItemMetadataRight[playerid]);
    
    // Verificar si el slot original está libre
    new targetSlot = -1;
    if(PlayerInventory[playerid][originalSlot][invItemID] == 0)
    {
        // Slot original libre, restaurar ahí
        targetSlot = originalSlot;
    }
    else
    {
        // Slot ocupado, buscar otro slot libre compatible
        new itemSize = GetItemSize(itemid);
        for(new i = 0; i < MAX_INVENTORY_SLOTS; i++)
        {
            if(PlayerInventory[playerid][i][invItemID] == 0)
            {
                // Verificar si el item cabe en este slot
                if(itemSize == ITEM_SIZE_LARGE && PlayerSlots[playerid][i][slotType] != SLOT_TYPE_BACK)
                    continue; // Item grande solo en espalda
                if(itemSize == ITEM_SIZE_SMALL && PlayerSlots[playerid][i][slotType] == SLOT_TYPE_BACK)
                    continue; // Preferir bolsillos para items pequeños
                
                targetSlot = i;
                break;
            }
        }
    }
    
    // Si no hay slots libres, no permitir desequipar
    if(targetSlot == -1)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes espacio en el inventario. Suelta algo primero con /tirar.");
        return 0;
    }
    
    // Remover objeto visual
    RemovePlayerAttachedObject(playerid, 0);
    PlayerHeldObjectRight[playerid] = INVALID_OBJECT_ID;
    PlayerHeldItemSlotRight[playerid] = -1;
    PlayerHoldingLargeItem[playerid] = false;
    
    // Restaurar item al slot
    new characterID = CharacterData[playerid][cID];
    PlayerInventory[playerid][targetSlot][invItemID] = itemid;
    PlayerInventory[playerid][targetSlot][invCharacterID] = characterID;
    PlayerInventory[playerid][targetSlot][invSlot] = targetSlot;
    PlayerInventory[playerid][targetSlot][invQuantity] = quantity;
    format(PlayerInventory[playerid][targetSlot][invMetadata], 256, "%s", metadata);
    PlayerSlots[playerid][targetSlot][slotOccupied] = 1;
    PlayerInventoryCount[playerid]++;
    
    // Guardar en base de datos
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO character_inventory (character_id, item_id, slot, quantity, metadata) VALUES (%d, %d, %d, %d, '%e')",
        characterID, itemid, targetSlot, quantity, metadata);
    mysql_tquery(g_MySQL, query);
    
    // Limpiar datos temporales
    PlayerHeldItemIDRight[playerid] = 0;
    PlayerHeldItemQuantityRight[playerid] = 0;
    PlayerHeldItemMetadataRight[playerid][0] = EOS;
    
    new itemName[64];
    GetItemName(itemid, itemName);
    
    new string[128];
    if(targetSlot != originalSlot)
        format(string, sizeof(string), "{00FF00}Has guardado %s en otro slot (el original estaba ocupado).", itemName);
    else
        format(string, sizeof(string), "{00FF00}Has guardado %s.", itemName);
    SendClientMessage(playerid, COLOR_INFO, string);
    
    return 1;
}

// Desequipar objeto de mano izquierda
stock UnholdItemLeft(playerid)
{
    if(PlayerHeldObjectLeft[playerid] == INVALID_OBJECT_ID)
        return 0;
    
    new originalSlot = PlayerHeldItemSlotLeft[playerid];
    new itemid = PlayerHeldItemIDLeft[playerid];
    new quantity = PlayerHeldItemQuantityLeft[playerid];
    new metadata[256];
    format(metadata, sizeof(metadata), "%s", PlayerHeldItemMetadataLeft[playerid]);
    
    // Verificar si el slot original está libre
    new targetSlot = -1;
    if(PlayerInventory[playerid][originalSlot][invItemID] == 0)
    {
        // Slot original libre, restaurar ahí
        targetSlot = originalSlot;
    }
    else
    {
        // Slot ocupado, buscar otro slot libre compatible
        new itemSize = GetItemSize(itemid);
        for(new i = 0; i < MAX_INVENTORY_SLOTS; i++)
        {
            if(PlayerInventory[playerid][i][invItemID] == 0)
            {
                // Verificar si el item cabe en este slot
                if(itemSize == ITEM_SIZE_LARGE && PlayerSlots[playerid][i][slotType] != SLOT_TYPE_BACK)
                    continue; // Item grande solo en espalda
                if(itemSize == ITEM_SIZE_SMALL && PlayerSlots[playerid][i][slotType] == SLOT_TYPE_BACK)
                    continue; // Preferir bolsillos para items pequeños
                
                targetSlot = i;
                break;
            }
        }
    }
    
    // Si no hay slots libres, no permitir desequipar
    if(targetSlot == -1)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes espacio en el inventario. Suelta algo primero con /tirar.");
        return 0;
    }
    
    // Remover objeto visual
    RemovePlayerAttachedObject(playerid, 1);
    PlayerHeldObjectLeft[playerid] = INVALID_OBJECT_ID;
    PlayerHeldItemSlotLeft[playerid] = -1;
    
    // Restaurar item al slot
    new characterID = CharacterData[playerid][cID];
    PlayerInventory[playerid][targetSlot][invItemID] = itemid;
    PlayerInventory[playerid][targetSlot][invCharacterID] = characterID;
    PlayerInventory[playerid][targetSlot][invSlot] = targetSlot;
    PlayerInventory[playerid][targetSlot][invQuantity] = quantity;
    format(PlayerInventory[playerid][targetSlot][invMetadata], 256, "%s", metadata);
    PlayerSlots[playerid][targetSlot][slotOccupied] = 1;
    PlayerInventoryCount[playerid]++;
    
    // Guardar en base de datos
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO character_inventory (character_id, item_id, slot, quantity, metadata) VALUES (%d, %d, %d, %d, '%e')",
        characterID, itemid, targetSlot, quantity, metadata);
    mysql_tquery(g_MySQL, query);
    
    // Limpiar datos temporales
    PlayerHeldItemIDLeft[playerid] = 0;
    PlayerHeldItemQuantityLeft[playerid] = 0;
    PlayerHeldItemMetadataLeft[playerid][0] = EOS;
    
    new itemName[64];
    GetItemName(itemid, itemName);
    
    new string[128];
    if(targetSlot != originalSlot)
        format(string, sizeof(string), "{00FF00}Has guardado %s en otro slot (el original estaba ocupado).", itemName);
    else
        format(string, sizeof(string), "{00FF00}Has guardado %s.", itemName);
    SendClientMessage(playerid, COLOR_INFO, string);
    
    return 1;
}

// Obtener nombre del item
stock GetItemName(itemid, name[], size = sizeof(name))
{
    // Intentar buscar en armas primero
    for(new i = 0; i < TotalWeapons; i++)
    {
        if(WeaponData[i][wItemID] == itemid)
        {
            format(name, size, "%s", WeaponData[i][wName]);
            return 1;
        }
    }
    
    // Intentar buscar en cargadores
    for(new i = 0; i < TotalMagazines; i++)
    {
        if(MagazineData[i][mItemID] == itemid)
        {
            format(name, size, "%s", MagazineData[i][mName]);
            return 1;
        }
    }
    
    // Items fijos del sistema
    switch(itemid)
    {
        case ITEM_VEHICLE_KEY_MASTER: format(name, size, "Llave Maestra");
        case ITEM_VEHICLE_KEY: format(name, size, "Llave Normal");
        case ITEM_PHONE: format(name, size, "Telefono Celular");
        case ITEM_REPAIR_KIT: format(name, size, "Kit de Reparacion");
        case ITEM_BURGER: format(name, size, "Hamburguesa");
        case ITEM_WATER: format(name, size, "Agua");
        default: format(name, size, "Item Desconocido (ID:%d)", itemid);
    }
    return 1;
}

// Verificar si tiene llave del vehículo
stock PlayerHasVehicleKey(playerid, vehicleid)
{
    new metadata[128];
    for(new i = 0; i < MAX_INVENTORY_SLOTS; i++)
    {
        if(PlayerInventory[playerid][i][invItemID] == ITEM_VEHICLE_KEY)
        {
            format(metadata, sizeof(metadata), "%s", PlayerInventory[playerid][i][invMetadata]);
            
            // Metadata contiene "vehicleid:X"
            new vid;
            if(sscanf(metadata, "vehicleid:%d", vid))
                continue;
            
            if(vid == vehicleid)
                return 1;
        }
    }
    return 0;
}

// ============================================================================
// COMANDOS
// ============================================================================

CMD:inventario(playerid, params[])
{
    new dialogStr[2048], itemName[64], line[128];
    strcat(dialogStr, "Ubicacion\tItem\tCantidad\n");
    
    // Mano izquierda
    if(PlayerHeldItemSlotLeft[playerid] != -1 && PlayerHeldItemIDLeft[playerid] != 0)
    {
        // Usar datos guardados en variables temporales
        GetItemName(PlayerHeldItemIDLeft[playerid], itemName);
        new quantity = GetItemDisplayQuantityByData(PlayerHeldItemIDLeft[playerid], PlayerHeldItemMetadataLeft[playerid]);
        format(line, sizeof(line), "{00FF00}[Mano Izquierda]\t%s\t%d\n",
            itemName, quantity);
        strcat(dialogStr, line);
    }
    else
    {
        strcat(dialogStr, "{808080}[Mano Izquierda]\t{808080}Vacia\t-\n");
    }
    
    // Mano derecha
    if(PlayerHeldItemSlotRight[playerid] != -1 && (PlayerHeldItemIDRight[playerid] != 0 || PlayerCurrentWeapon[playerid] != -1))
    {
        // Si es un arma, obtener datos del arma
        if(PlayerCurrentWeapon[playerid] != -1)
        {
            new weaponIdx = PlayerCurrentWeapon[playerid];
            format(itemName, sizeof(itemName), "%s", WeaponData[weaponIdx][wName]);
            new quantity = GetItemDisplayQuantityByData(WeaponData[weaponIdx][wItemID], PlayerHeldItemMetadataRight[playerid]);
            format(line, sizeof(line), "{00FF00}[Mano Derecha]\t%s\t%d\n",
                itemName, quantity);
            strcat(dialogStr, line);
        }
        else
        {
            // Item normal, usar datos guardados en variables temporales
            GetItemName(PlayerHeldItemIDRight[playerid], itemName);
            new quantity = GetItemDisplayQuantityByData(PlayerHeldItemIDRight[playerid], PlayerHeldItemMetadataRight[playerid]);
            format(line, sizeof(line), "{00FF00}[Mano Derecha]\t%s\t%d\n",
                itemName, quantity);
            strcat(dialogStr, line);
        }
    }
    else
    {
        strcat(dialogStr, "{808080}[Mano Derecha]\t{808080}Vacia\t-\n");
    }
    
    // Slot de espalda (slot 2)
    if(PlayerInventory[playerid][2][invItemID] > 0)
    {
        GetItemName(PlayerInventory[playerid][2][invItemID], itemName);
        new quantity = GetItemDisplayQuantity(playerid, 2);
        format(line, sizeof(line), "{FFD700}[Espalda]\t%s\t%d\n",
            itemName, quantity);
        strcat(dialogStr, line);
    }
    else
    {
        strcat(dialogStr, "{808080}[Espalda]\t{808080}Vacia\t-\n");
    }
    
    // Separador visual
    strcat(dialogStr, "{696969}---\t---\t---\n");
    
    // Bolsillos pequeños (slots 3-8) - Mostrar TODOS los slots individualmente
    // EXCEPTO los que están equipados en manos
    new pocketDisplay = 1; // Contador visual para mostrar Bolsillo 1, 2, 3, etc.
    for(new i = 3; i < MAX_INVENTORY_SLOTS; i++)
    {
        // Verificar si este slot está equipado en alguna mano
        new bool:isEquipped = (PlayerHeldItemSlotLeft[playerid] == i || PlayerHeldItemSlotRight[playerid] == i);
        
        if(PlayerInventory[playerid][i][invItemID] > 0 && !isEquipped)
        {
            GetItemName(PlayerInventory[playerid][i][invItemID], itemName);
            new quantity = GetItemDisplayQuantity(playerid, i);
            format(line, sizeof(line), "Bolsillo %d\t%s\t%d\n",
                pocketDisplay, itemName, quantity);
            strcat(dialogStr, line);
            pocketDisplay++;
        }
        else if(!isEquipped)
        {
            // Mostrar bolsillo vacío
            format(line, sizeof(line), "{808080}Bolsillo %d\t{808080}Vacio\t{808080}-\n", pocketDisplay);
            strcat(dialogStr, line);
            pocketDisplay++;
        }
    }
    
    ShowPlayerDialog(playerid, DIALOG_INVENTORY, DIALOG_STYLE_TABLIST_HEADERS,
        "Inventario", dialogStr, "Equipar/Usar", "Cerrar");
    
    return 1;
}

CMD:inv(playerid, params[]) return cmd_inventario(playerid, params);

// Handler del dialog de inventario
stock OnDialogInventory(playerid, response, listitem)
{
    if(!response) return 1;
    
    // listitem 0 = Mano izquierda
    // listitem 1 = Mano derecha
    // listitem 2 = Espalda
    // listitem 3 = Separador (---)
    // listitem 4+ = Bolsillos pequeños o línea de vacíos
    
    if(listitem == 0) // Mano izquierda
    {
        if(PlayerHeldItemSlotLeft[playerid] != -1)
        {
            // Desequipar mano izquierda
            if(PlayerHeldObjectLeft[playerid] != INVALID_OBJECT_ID)
            {
                RemovePlayerAttachedObject(playerid, 1);
                PlayerHeldObjectLeft[playerid] = INVALID_OBJECT_ID;
                PlayerHeldItemSlotLeft[playerid] = -1;
                SendClientMessage(playerid, COLOR_INFO, "Has guardado el objeto de la mano izquierda.");
            }
        }
        return 1;
    }
    
    if(listitem == 1) // Mano derecha
    {
        if(PlayerHeldItemSlotRight[playerid] != -1)
        {
            // Si es un arma, ejecutar UnequipWeapon para guardar munición
            if(PlayerCurrentWeapon[playerid] != -1)
            {
                UnequipWeapon(playerid);
            }
            else
            {
                // Si no es arma, desequipar normal
                UnholdItem(playerid);
            }
        }
        return 1;
    }
    
    if(listitem == 2) // Espalda
    {
        // Si hay item en la espalda
        if(PlayerInventory[playerid][2][invItemID] > 0)
        {
            // PRIMERO verificar si la mano derecha está libre
            if(PlayerHeldItemSlotRight[playerid] == -1)
            {
                // Si el item está en la espalda visualmente, removerlo antes de equipar
                if(PlayerBackSlot[playerid] == 2)
                {
                    RemoveItemFromBack(playerid);
                }
                
                // Equipar en mano derecha
                HoldItem(playerid, 2);
            }
            else
            {
                // Mano derecha ocupada - INTERCAMBIAR items entre espalda y mano
                SendClientMessage(playerid, COLOR_INFO, "{FFD700}Intercambiando items entre espalda y mano derecha...");
                
                // Primero desequipar el item de la mano (lo guarda en el slot original)
                if(PlayerCurrentWeapon[playerid] != -1)
                {
                    UnequipWeapon(playerid); // Si es arma, usar función especial
                }
                else
                {
                    UnholdItem(playerid); // Si es item normal
                }
                
                // Ahora equipar el de la espalda (mano ya está libre)
                if(PlayerBackSlot[playerid] == 2)
                {
                    RemoveItemFromBack(playerid);
                }
                HoldItem(playerid, 2);
            }
        }
        else
        {
            SendClientMessage(playerid, COLOR_INFO, "No tienes ningún item en la espalda.");
        }
        return 1;
    }
    
    if(listitem == 3) // Separador (---)
    {
        return 1;
    }
    
    // listitem 4+ son los bolsillos pequeños
    // Necesitamos mapear el listitem al slot real
    new dialogLine = 4; // Empieza después del separador
    
    // Iterar por los bolsillos (slots 3-8)
    for(new slot = 3; slot < MAX_INVENTORY_SLOTS; slot++)
    {
        if(PlayerInventory[playerid][slot][invItemID] > 0)
        {
            if(dialogLine == listitem)
            {
                // Este es el slot seleccionado
                // Equipar en mano derecha si está vacía, sino en izquierda
                if(PlayerHeldItemSlotRight[playerid] == -1)
                {
                    HoldItem(playerid, slot);
                }
                else if(PlayerHeldItemSlotLeft[playerid] == -1)
                {
                    HoldItemLeft(playerid, slot);
                }
                else
                {
                    SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Tus manos están llenas. Guarda algo primero.");
                }
                return 1;
            }
            dialogLine++;
        }
    }
    
    // Si llegó aquí, probablemente clickeó la línea de "Bolsillos Vacíos"
    return 1;
}

// Callback para diálogo de tirar item
stock OnDialogInventoryDrop(playerid, response, listitem)
{
    if(!response) return 1; // Canceló
    
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
    // Mapeo de listitem a slot/ubicación:
    // 0 = Mano izquierda (equipado)
    // 1 = Mano derecha (equipado)
    // 2 = Espalda (slot 2)
    // 3 = Separador
    // 4+ = Bolsillos (slots 3-8)
    
    if(listitem == 0) // Mano izquierda
    {
        if(PlayerHeldItemSlotLeft[playerid] == -1 || PlayerHeldItemIDLeft[playerid] == 0)
        {
            SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Tu mano izquierda está vacía.");
            return 1;
        }
        
        // Tirar item de mano izquierda
        new itemid = PlayerHeldItemIDLeft[playerid];
        new quantity = PlayerHeldItemQuantityLeft[playerid];
        
        CreateDroppedItemAt(itemid, quantity, PlayerHeldItemMetadataLeft[playerid], x, y, z - 0.9, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));
        
        // Remover objeto visual
        RemovePlayerAttachedObject(playerid, 1);
        PlayerHeldObjectLeft[playerid] = INVALID_OBJECT_ID;
        PlayerHeldItemSlotLeft[playerid] = -1;
        PlayerHeldItemIDLeft[playerid] = 0;
        PlayerHeldItemQuantityLeft[playerid] = 0;
        PlayerHeldItemMetadataLeft[playerid][0] = EOS;
        
        new itemName[64];
        GetItemName(itemid, itemName);
        
        new string[128];
        format(string, sizeof(string), "{00FF00}Has tirado: {FFFFFF}%s", itemName);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        
        ApplyAnimation(playerid, "BOMBER", "BOM_PLANT", 4.1, false, false, false, false, 0);
        return 1;
    }
    
    if(listitem == 1) // Mano derecha
    {
        if(PlayerHeldItemSlotRight[playerid] == -1 || (PlayerHeldItemIDRight[playerid] == 0 && PlayerCurrentWeapon[playerid] == -1))
        {
            SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Tu mano derecha está vacía.");
            return 1;
        }
        
        // Si es un arma
        if(PlayerCurrentWeapon[playerid] != -1)
        {
            new weaponIdx = PlayerCurrentWeapon[playerid];
            new itemid = WeaponData[weaponIdx][wItemID];
            
            CreateDroppedItemAt(itemid, 1, PlayerHeldItemMetadataRight[playerid], x, y, z - 0.9, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));
            
            // Remover arma
            RemovePlayerWeapon(playerid, WEAPON:WeaponData[weaponIdx][wSAMPID]);
            RemovePlayerAttachedObject(playerid, 0);
            PlayerHeldObjectRight[playerid] = INVALID_OBJECT_ID;
            PlayerHeldItemSlotRight[playerid] = -1;
            PlayerCurrentWeapon[playerid] = -1;
            PlayerCurrentAmmo[playerid] = 0;
            PlayerWeaponSlot[playerid] = -1;
            PlayerHeldItemIDRight[playerid] = 0;
            PlayerHeldItemQuantityRight[playerid] = 0;
            PlayerHeldItemMetadataRight[playerid][0] = EOS;
            
            new string[128];
            format(string, sizeof(string), "{00FF00}Has tirado: {FFFFFF}%s", WeaponData[weaponIdx][wName]);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
        }
        else
        {
            // Item normal
            new itemid = PlayerHeldItemIDRight[playerid];
            new quantity = PlayerHeldItemQuantityRight[playerid];
            
            CreateDroppedItemAt(itemid, quantity, PlayerHeldItemMetadataRight[playerid], x, y, z - 0.9, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));
            
            // Remover objeto visual
            RemovePlayerAttachedObject(playerid, 0);
            PlayerHeldObjectRight[playerid] = INVALID_OBJECT_ID;
            PlayerHeldItemSlotRight[playerid] = -1;
            PlayerHeldItemIDRight[playerid] = 0;
            PlayerHeldItemQuantityRight[playerid] = 0;
            PlayerHeldItemMetadataRight[playerid][0] = EOS;
            
            new itemName[64];
            GetItemName(itemid, itemName);
            
            new string[128];
            format(string, sizeof(string), "{00FF00}Has tirado: {FFFFFF}%s", itemName);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
        }
        
        ApplyAnimation(playerid, "BOMBER", "BOM_PLANT", 4.1, false, false, false, false, 0);
        return 1;
    }
    
    if(listitem == 2) // Espalda
    {
        if(PlayerInventory[playerid][2][invItemID] == 0)
        {
            SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Tu espalda está vacía.");
            return 1;
        }
        
        // Remover visual de espalda si existe
        if(PlayerBackSlot[playerid] == 2)
        {
            RemoveItemFromBack(playerid);
        }
        
        // Tirar item de espalda
        DropItemToGround(playerid, 2);
        ApplyAnimation(playerid, "BOMBER", "BOM_PLANT", 4.1, false, false, false, false, 0);
        return 1;
    }
    
    if(listitem == 3) // Separador
    {
        return 1;
    }
    
    // listitem 4+ son los bolsillos
    // Mapear a slots 3-8
    new slotNum = listitem - 4 + 3;
    
    if(slotNum >= 3 && slotNum < MAX_INVENTORY_SLOTS)
    {
        if(PlayerInventory[playerid][slotNum][invItemID] == 0)
        {
            new pocketNum = slotNum - 2;
            new string[64];
            format(string, sizeof(string), "{FF0000}El bolsillo %d está vacío.", pocketNum);
            SendClientMessage(playerid, COLOR_ERROR, string);
            return 1;
        }
        
        // Tirar item del bolsillo
        DropItemToGround(playerid, slotNum);
        ApplyAnimation(playerid, "BOMBER", "BOM_PLANT", 4.1, false, false, false, false, 0);
    }
    
    return 1;
}

CMD:usar(playerid, params[])
{
    // PRIORIDAD 1: Si tiene arma en mano derecha y cargador en mano izquierda, recargar
    if(PlayerCurrentWeapon[playerid] != -1 && PlayerHeldItemSlotLeft[playerid] != -1)
    {
        new magSlot = PlayerHeldItemSlotLeft[playerid];
        new magItemID = PlayerInventory[playerid][magSlot][invItemID];
        
        if(IsMagazineItem(magItemID))
        {
            return ReloadWeaponFromMagazine(playerid);
        }
    }
    
    // PRIORIDAD 2: Usar item del slot especificado
    new slot;
    if(sscanf(params, "d", slot))
        return SendClientMessage(playerid, COLOR_ERROR, "Uso: /usar [slot]");
    
    slot--; // Ajustar (usuario usa 1-20, sistema usa 0-19)
    
    if(slot < 0 || slot >= MAX_INVENTORY_SLOTS)
        return SendClientMessage(playerid, COLOR_ERROR, "Slot inválido (1-20)");
    
    new itemid = PlayerInventory[playerid][slot][invItemID];
    if(itemid == 0)
        return SendClientMessage(playerid, COLOR_ERROR, "No tienes nada en ese slot.");
    
    switch(itemid)
    {
        case ITEM_VEHICLE_KEY_MASTER:
        {
            HoldItem(playerid, slot);
            SendClientMessage(playerid, COLOR_SUCCESS, "{FFD700}Llave maestra equipada. {FFFFFF}Permite crear duplicados.");
        }
        case ITEM_VEHICLE_KEY:
        {
            HoldItem(playerid, slot);
            SendClientMessage(playerid, COLOR_SUCCESS, "{00FF00}Llave del vehículo equipada.");
        }
        case ITEM_PHONE:
        {
            HoldItem(playerid, slot);
        }
        case ITEM_REPAIR_KIT:
        {
            new vehicleid = GetPlayerVehicleID(playerid);
            if(vehicleid == 0)
                return SendClientMessage(playerid, COLOR_ERROR, "Debes estar en un vehículo.");
            
            SetVehicleHealth(vehicleid, 1000.0);
            RemovePlayerItem(playerid, slot);
            SendClientMessage(playerid, COLOR_SUCCESS, "Has reparado el vehículo.");
            GameTextForPlayer(playerid, "~g~Vehiculo reparado!", 3000, 3);
        }
        case ITEM_BURGER:
        {
            new Float:health;
            GetPlayerHealth(playerid, health);
            SetPlayerHealth(playerid, health + 20.0);
            RemovePlayerItem(playerid, slot);
            SendClientMessage(playerid, COLOR_SUCCESS, "Has comido una hamburguesa. +20 HP");
            ApplyAnimation(playerid, "FOOD", "EAT_Burger", 4.1, false, false, false, false, 0);
        }
        case ITEM_WATER:
        {
            new Float:health;
            GetPlayerHealth(playerid, health);
            SetPlayerHealth(playerid, health + 10.0);
            RemovePlayerItem(playerid, slot);
            SendClientMessage(playerid, COLOR_SUCCESS, "Has bebido agua. +10 HP");
            ApplyAnimation(playerid, "VENDING", "VEND_Drink2_P", 4.1, false, false, false, false, 0);
        }
        default:
        {
            SendClientMessage(playerid, COLOR_ERROR, "Este item no se puede usar así.");
        }
    }
    
    return 1;
}

CMD:guardar(playerid, params[])
{
    // Verificar si tiene arma equipada
    if(PlayerCurrentWeapon[playerid] != -1)
    {
        UnequipWeapon(playerid);
        return 1;
    }
    
    // Verificar si tiene cargador en mano izquierda
    if(PlayerHeldItemSlotLeft[playerid] != -1)
    {
        new magSlot = PlayerHeldItemSlotLeft[playerid];
        new magItemID = PlayerInventory[playerid][magSlot][invItemID];
        
        if(IsMagazineItem(magItemID))
        {
            UnequipMagazine(playerid);
            return 1;
        }
    }
    
    // Items normales
    if(PlayerHeldObject[playerid] == INVALID_OBJECT_ID)
        return SendClientMessage(playerid, COLOR_ERROR, "No tienes nada equipado.");
    
    UnholdItem(playerid);
    return 1;
}

// Dar item a jugador (admin)
CMD:giveitem(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "No tienes permisos.");
    
    new targetid, itemid, quantity;
    if(sscanf(params, "ddd", targetid, itemid, quantity))
        return SendClientMessage(playerid, COLOR_ERROR, "Uso: /giveitem [playerid] [itemid] [cantidad]");
    
    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_ERROR, "Jugador no conectado.");
    
    if(itemid < 1 || itemid > 7)
        return SendClientMessage(playerid, COLOR_ERROR, "Item ID inválido (1-7)");
    
    if(GivePlayerItem(targetid, itemid, quantity, ""))
    {
        new itemName[64];
        GetItemName(itemid, itemName);
        
        new string[144];
        format(string, sizeof(string), "{00FF00}Le has dado %d x %s a %s",
            quantity, itemName, CharacterData[targetid][cName]);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        
        format(string, sizeof(string), "{00FF00}Has recibido %d x %s",
            quantity, itemName);
        SendClientMessage(targetid, COLOR_SUCCESS, string);
    }
    else
    {
        SendClientMessage(playerid, COLOR_ERROR, "Error al dar el item.");
    }
    
    return 1;
}

// Comando /tirar - Mostrar diálogo para tirar item
CMD:tirar(playerid, params[])
{
    if(!CharacterData[playerid][cSelected])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Debes estar logueado.");
    
    // Construir el mismo diálogo que el inventario (igual que CMD:inventario pero para tirar)
    new dialogStr[2048], line[256], itemName[64];
    strcat(dialogStr, "Ubicacion\tItem\tCantidad\n");
    
    new bool:hasItems = false;
    
    // Mano izquierda
    if(PlayerHeldItemSlotLeft[playerid] != -1 && PlayerHeldItemIDLeft[playerid] != 0)
    {
        GetItemName(PlayerHeldItemIDLeft[playerid], itemName);
        new quantity = GetItemDisplayQuantityByData(PlayerHeldItemIDLeft[playerid], PlayerHeldItemMetadataLeft[playerid]);
        format(line, sizeof(line), "{00FF00}[Mano Izquierda]\t%s\t%d\n", itemName, quantity);
        strcat(dialogStr, line);
        hasItems = true;
    }
    else
    {
        strcat(dialogStr, "{808080}[Mano Izquierda]\t{808080}Vacia\t-\n");
    }
    
    // Mano derecha
    if(PlayerHeldItemSlotRight[playerid] != -1 && (PlayerHeldItemIDRight[playerid] != 0 || PlayerCurrentWeapon[playerid] != -1))
    {
        // Si es un arma, obtener datos del arma
        if(PlayerCurrentWeapon[playerid] != -1)
        {
            new weaponIdx = PlayerCurrentWeapon[playerid];
            format(itemName, sizeof(itemName), "%s", WeaponData[weaponIdx][wName]);
            new quantity = GetItemDisplayQuantityByData(WeaponData[weaponIdx][wItemID], PlayerHeldItemMetadataRight[playerid]);
            format(line, sizeof(line), "{00FF00}[Mano Derecha]\t%s\t%d\n", itemName, quantity);
            strcat(dialogStr, line);
        }
        else
        {
            // Item normal, usar datos guardados en variables temporales
            GetItemName(PlayerHeldItemIDRight[playerid], itemName);
            new quantity = GetItemDisplayQuantityByData(PlayerHeldItemIDRight[playerid], PlayerHeldItemMetadataRight[playerid]);
            format(line, sizeof(line), "{00FF00}[Mano Derecha]\t%s\t%d\n", itemName, quantity);
            strcat(dialogStr, line);
        }
        hasItems = true;
    }
    else
    {
        strcat(dialogStr, "{808080}[Mano Derecha]\t{808080}Vacia\t-\n");
    }
    
    // Slot de espalda (slot 2)
    if(PlayerInventory[playerid][2][invItemID] > 0)
    {
        GetItemName(PlayerInventory[playerid][2][invItemID], itemName);
        new quantity = GetItemDisplayQuantity(playerid, 2);
        format(line, sizeof(line), "{FFD700}[Espalda]\t%s\t%d\n", itemName, quantity);
        strcat(dialogStr, line);
        hasItems = true;
    }
    else
    {
        strcat(dialogStr, "{808080}[Espalda]\t{808080}Vacia\t-\n");
    }
    
    // Separador visual
    strcat(dialogStr, "{696969}---\t---\t---\n");
    
    // Bolsillos pequeños (slots 3-8, mostrados como 1-6)
    for(new i = 3; i < MAX_INVENTORY_SLOTS; i++)
    {
        if(PlayerInventory[playerid][i][invItemID] > 0)
        {
            GetItemName(PlayerInventory[playerid][i][invItemID], itemName);
            new quantity = GetItemDisplayQuantity(playerid, i);
            new pocketNum = i - 2; // 3->1, 4->2, 5->3, 6->4, 7->5, 8->6
            format(line, sizeof(line), "{00FF00}Bolsillo %d\t%s\t%d\n", pocketNum, itemName, quantity);
            strcat(dialogStr, line);
            hasItems = true;
        }
        else
        {
            new pocketNum = i - 2; // 3->1, 4->2, 5->3, 6->4, 7->5, 8->6
            format(line, sizeof(line), "{808080}Bolsillo %d\t{808080}Vacio\t-\n", pocketNum);
            strcat(dialogStr, line);
        }
    }
    
    if(!hasItems)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Tu inventario está vacío.");
        return 1;
    }
    
    ShowPlayerDialog(playerid, DIALOG_INVENTORY_DROP, DIALOG_STYLE_TABLIST_HEADERS,
        "{FF6347}Tirar Item - Selecciona que tirar", dialogStr, "Tirar", "Cancelar");
    
    return 1;
}

// Comando /recoger - Recoger item del suelo
CMD:recoger(playerid, params[])
{
    if(!CharacterData[playerid][cSelected])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Debes estar logueado.");
    
    new dropSlot = GetNearestDroppedItem(playerid, 2.0); // 2 metros de distancia
    
    if(dropSlot == -1)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No hay ningún item cerca.");
    
    PickupDroppedItem(playerid, dropSlot);
    
    // Animación de agacharse
    ApplyAnimation(playerid, "BOMBER", "BOM_PLANT", 4.1, false, false, false, false, 0);
    
    return 1;
}
