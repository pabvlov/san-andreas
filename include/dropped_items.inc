/*
    Sistema de Items Tirados al Suelo
*/

#if defined _dropped_items_included
    #endinput
#endif
#define _dropped_items_included

#define MAX_DROPPED_ITEMS 500
#define PICKUP_DISTANCE 2.0

// Enumeración de items tirados
enum E_DROPPED_ITEM
{
    dItemID,          // ID del item (de la tabla items)
    dQuantity,        // Cantidad
    dMetadata[128],   // Metadata (JSON)
    Float:dPosX,
    Float:dPosY,
    Float:dPosZ,
    dInterior,
    dVirtualWorld,
    dObjectID,        // ID del objeto 3D
    dDBID,            // ID en la base de datos
    bool:dActive
}

new DroppedItems[MAX_DROPPED_ITEMS][E_DROPPED_ITEM];

// Verificar si un item es un arma
stock IsItemWeapon(itemid)
{
    for(new i = 0; i < TotalWeapons; i++)
    {
        if(WeaponData[i][wItemID] == itemid)
            return 1;
    }
    return 0;
}

// Modelos de objetos para items (puedes personalizarlos)
stock GetItemObjectModel(itemid)
{
    // Buscar si es un arma
    for(new i = 0; i < TotalWeapons; i++)
    {
        if(WeaponData[i][wItemID] == itemid)
        {
            return WeaponData[i][wHandObject];
        }
    }
    
    // Buscar si es un cargador
    for(new i = 0; i < TotalMagazines; i++)
    {
        if(MagazineData[i][mItemID] == itemid)
        {
            return MagazineData[i][mHandObject];
        }
    }
    
    // Items genéricos
    switch(itemid)
    {
        case ITEM_VEHICLE_KEY, ITEM_VEHICLE_KEY_MASTER: return 2543; // Llave
        case ITEM_PHONE: return 330; // Teléfono
        case ITEM_REPAIR_KIT: return 1279; // Kit de herramientas
        case ITEM_BURGER: return 2703; // Hamburguesa
        case ITEM_WATER: return 1484; // Botella de agua
        default: return 1279; // Caja genérica
    }
    
    return 1279;
}

// Cargar items tirados del suelo desde BD
stock LoadDroppedItems()
{
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT id, item_id, quantity, metadata, pos_x, pos_y, pos_z, interior, virtual_world FROM dropped_items");
    
    new Cache:cache = mysql_query(g_MySQL, query);
    new rows = cache_num_rows();
    
    if(rows > 0)
    {
        printf("[Dropped Items] Cargando %d items del suelo...", rows);
        
        for(new i = 0; i < rows && i < MAX_DROPPED_ITEMS; i++)
        {
            cache_get_value_name_int(i, "id", DroppedItems[i][dDBID]);
            cache_get_value_name_int(i, "item_id", DroppedItems[i][dItemID]);
            cache_get_value_name_int(i, "quantity", DroppedItems[i][dQuantity]);
            cache_get_value_name(i, "metadata", DroppedItems[i][dMetadata], 128);
            cache_get_value_name_float(i, "pos_x", DroppedItems[i][dPosX]);
            cache_get_value_name_float(i, "pos_y", DroppedItems[i][dPosY]);
            cache_get_value_name_float(i, "pos_z", DroppedItems[i][dPosZ]);
            cache_get_value_name_int(i, "interior", DroppedItems[i][dInterior]);
            cache_get_value_name_int(i, "virtual_world", DroppedItems[i][dVirtualWorld]);
            
            // Crear objeto 3D (armas acostadas, otros items parados)
            new modelid = GetItemObjectModel(DroppedItems[i][dItemID]);
            new Float:rotX = IsItemWeapon(DroppedItems[i][dItemID]) ? 90.0 : 0.0;
            DroppedItems[i][dObjectID] = CreateDynamicObject(modelid,
                DroppedItems[i][dPosX], DroppedItems[i][dPosY], DroppedItems[i][dPosZ] - 0.9,
                rotX, 0.0, 0.0,  // Solo armas acostadas
                DroppedItems[i][dVirtualWorld], DroppedItems[i][dInterior]);
            
            DroppedItems[i][dActive] = true;
            
            printf("  - Slot %d: ItemID=%d, Cantidad=%d, Pos=(%.2f, %.2f, %.2f)",
                i, DroppedItems[i][dItemID], DroppedItems[i][dQuantity],
                DroppedItems[i][dPosX], DroppedItems[i][dPosY], DroppedItems[i][dPosZ]);
        }
    }
    
    cache_delete(cache);
    return rows;
}

// Tirar item al suelo
stock DropItemToGround(playerid, slot)
{
    if(slot < 0 || slot >= MAX_INVENTORY_SLOTS)
        return -1;
    
    if(PlayerInventory[playerid][slot][invItemID] == 0)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Este slot está vacío.");
        return -1;
    }
    
    // Verificar si el item está equipado
    if(PlayerHeldItemSlotLeft[playerid] == slot || PlayerHeldItemSlotRight[playerid] == slot)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Debes guardar el item antes de tirarlo.");
        return -1;
    }
    
    // Buscar slot libre
    new dropSlot = -1;
    for(new i = 0; i < MAX_DROPPED_ITEMS; i++)
    {
        if(!DroppedItems[i][dActive])
        {
            dropSlot = i;
            break;
        }
    }
    
    if(dropSlot == -1)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No hay espacio para más items en el suelo.");
        return -1;
    }
    
    // Obtener posición del jugador
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
    // Posición al frente del jugador
    new Float:angle;
    GetPlayerFacingAngle(playerid, angle);
    x += (1.5 * floatsin(-angle, degrees));
    y += (1.5 * floatcos(-angle, degrees));
    
    new interior = GetPlayerInterior(playerid);
    new vworld = GetPlayerVirtualWorld(playerid);
    
    // Guardar datos del item
    DroppedItems[dropSlot][dItemID] = PlayerInventory[playerid][slot][invItemID];
    DroppedItems[dropSlot][dQuantity] = PlayerInventory[playerid][slot][invQuantity];
    format(DroppedItems[dropSlot][dMetadata], 128, "%s", PlayerInventory[playerid][slot][invMetadata]);
    DroppedItems[dropSlot][dPosX] = x;
    DroppedItems[dropSlot][dPosY] = y;
    DroppedItems[dropSlot][dPosZ] = z;
    DroppedItems[dropSlot][dInterior] = interior;
    DroppedItems[dropSlot][dVirtualWorld] = vworld;
    DroppedItems[dropSlot][dActive] = true;
    
    // Crear objeto 3D en el mundo (armas acostadas, otros items parados)
    new modelid = GetItemObjectModel(DroppedItems[dropSlot][dItemID]);
    new Float:rotX = IsItemWeapon(DroppedItems[dropSlot][dItemID]) ? 90.0 : 0.0;
    DroppedItems[dropSlot][dObjectID] = CreateDynamicObject(modelid, x, y, z - 0.9, rotX, 0.0, 0.0, vworld, interior);
    
    // Guardar en BD
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO dropped_items (item_id, quantity, metadata, pos_x, pos_y, pos_z, interior, virtual_world) VALUES (%d, %d, '%e', %f, %f, %f, %d, %d)",
        DroppedItems[dropSlot][dItemID], DroppedItems[dropSlot][dQuantity], DroppedItems[dropSlot][dMetadata],
        x, y, z, interior, vworld);
    
    new Cache:cache = mysql_query(g_MySQL, query);
    DroppedItems[dropSlot][dDBID] = cache_insert_id();
    cache_delete(cache);
    
    // Remover del inventario
    RemovePlayerItem(playerid, slot);
    
    // Mensaje
    new itemName[64];
    GetItemName(DroppedItems[dropSlot][dItemID], itemName, sizeof(itemName));
    new string[128];
    format(string, sizeof(string), "{FFD700}Has tirado: {FFFFFF}%s {AAAAAA}(x%d) {FFD700}al suelo", itemName, DroppedItems[dropSlot][dQuantity]);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    printf("[Dropped Items] %s tiró ItemID=%d (x%d) en pos=(%.2f, %.2f, %.2f)",
        CharacterData[playerid][cName], DroppedItems[dropSlot][dItemID], DroppedItems[dropSlot][dQuantity], x, y, z);
    
    return dropSlot;
}

// Recoger item del suelo
stock PickupDroppedItem(playerid, dropSlot)
{
    if(dropSlot < 0 || dropSlot >= MAX_DROPPED_ITEMS)
        return 0;
    
    if(!DroppedItems[dropSlot][dActive])
        return 0;
    
    // Verificar distancia
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    new Float:distance = floatsqroot(
        ((x - DroppedItems[dropSlot][dPosX]) * (x - DroppedItems[dropSlot][dPosX])) +
        ((y - DroppedItems[dropSlot][dPosY]) * (y - DroppedItems[dropSlot][dPosY])) +
        ((z - DroppedItems[dropSlot][dPosZ]) * (z - DroppedItems[dropSlot][dPosZ]))
    );
    
    if(distance > PICKUP_DISTANCE)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Estás muy lejos del item.");
        return 0;
    }
    
    // Dar item al jugador
    new result = GivePlayerItem(playerid, DroppedItems[dropSlot][dItemID], DroppedItems[dropSlot][dQuantity], DroppedItems[dropSlot][dMetadata]);
    
    if(result == -1)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes espacio en el inventario.");
        return 0;
    }
    
    // Eliminar objeto del mundo
    DestroyDynamicObject(DroppedItems[dropSlot][dObjectID]);
    
    // Eliminar de BD
    new query[128];
    mysql_format(g_MySQL, query, sizeof(query),
        "DELETE FROM dropped_items WHERE id = %d", DroppedItems[dropSlot][dDBID]);
    mysql_query(g_MySQL, query);
    
    // Limpiar slot
    DroppedItems[dropSlot][dActive] = false;
    DroppedItems[dropSlot][dObjectID] = 0;
    DroppedItems[dropSlot][dDBID] = 0;
    
    // Mensaje
    new itemName[64];
    GetItemName(DroppedItems[dropSlot][dItemID], itemName, sizeof(itemName));
    new string[128];
    format(string, sizeof(string), "{00FF00}Has recogido: {FFFFFF}%s {AAAAAA}(x%d)", itemName, DroppedItems[dropSlot][dQuantity]);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    printf("[Dropped Items] %s recogió ItemID=%d (x%d)", CharacterData[playerid][cName], DroppedItems[dropSlot][dItemID], DroppedItems[dropSlot][dQuantity]);
    
    return 1;
}

// Buscar item más cercano
stock GetNearestDroppedItem(playerid, Float:maxDistance = PICKUP_DISTANCE)
{
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
    new nearestSlot = -1;
    new Float:nearestDistance = maxDistance;
    
    for(new i = 0; i < MAX_DROPPED_ITEMS; i++)
    {
        if(!DroppedItems[i][dActive])
            continue;
        
        if(DroppedItems[i][dVirtualWorld] != GetPlayerVirtualWorld(playerid))
            continue;
        
        if(DroppedItems[i][dInterior] != GetPlayerInterior(playerid))
            continue;
        
        new Float:distance = floatsqroot(
            ((x - DroppedItems[i][dPosX]) * (x - DroppedItems[i][dPosX])) +
            ((y - DroppedItems[i][dPosY]) * (y - DroppedItems[i][dPosY])) +
            ((z - DroppedItems[i][dPosZ]) * (z - DroppedItems[i][dPosZ]))
        );
        
        if(distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestSlot = i;
        }
    }
    
    return nearestSlot;
}
