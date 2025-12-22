/*
    Sistema de Inventario - Items y Objetos
*/

#if defined _inventory_system_included
    #endinput
#endif
#define _inventory_system_included

#define MAX_INVENTORY_SLOTS 20

// Weapon system constants
#define MAX_WEAPON_TYPES 50
#define MAX_MAGAZINE_TYPES 50

// Datos de armas
enum E_WEAPON_DATA
{
    wID,
    wName[64],
    wSlot,
    wSAMPID,
    wAmmoCapacity,
    wHandObject,
    wItemID
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
    mItemID
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
    invMetadata[128]
}

new PlayerInventory[MAX_PLAYERS][MAX_INVENTORY_SLOTS][E_INVENTORY_ITEM];
new PlayerInventoryCount[MAX_PLAYERS];

// Objetos equipados en manos
new PlayerHeldObjectLeft[MAX_PLAYERS] = {INVALID_OBJECT_ID, ...};
new PlayerHeldObjectRight[MAX_PLAYERS] = {INVALID_OBJECT_ID, ...};
new PlayerHeldItemSlotLeft[MAX_PLAYERS] = {-1, ...};
new PlayerHeldItemSlotRight[MAX_PLAYERS] = {-1, ...};

// Mantener compatibilidad
#define PlayerHeldObject PlayerHeldObjectRight
#define PlayerHeldItemSlot PlayerHeldItemSlotRight

// Cargar inventario del personaje
stock LoadPlayerInventory(playerid)
{
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
        
        printf("[Inventory] - Slot %d: ItemID=%d, Quantity=%d", 
            slotNum, PlayerInventory[playerid][slotNum][invItemID], PlayerInventory[playerid][slotNum][invQuantity]);
        
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
        }
    }
    
    return 1;
}

// Dar item al jugador
stock GivePlayerItem(playerid, itemid, quantity = 1, const metadata[] = "")
{
    // Buscar slot libre
    new slot = -1;
    for(new i = 0; i < MAX_INVENTORY_SLOTS; i++)
    {
        if(PlayerInventory[playerid][i][invItemID] == 0)
        {
            slot = i;
            break;
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
        PlayerInventoryCount[playerid]++;
        
        printf("[Inventory] Item %d dado a %s en slot %d (cantidad: %d, metadata: %s)", 
            itemid, CharacterData[playerid][cName], slot, quantity, metadata);
        
        return 1;
    }
    
    printf("[Inventory] ERROR: No se pudo insertar item en BD");
    
    return 0;
}

// Obtener cantidad a mostrar (balas si es arma, cantidad normal si no)
stock GetItemDisplayQuantity(playerid, slot)
{
    new itemid = PlayerInventory[playerid][slot][invItemID];
    
    // Verificar si es un arma
    for(new i = 0; i < TotalWeapons; i++)
    {
        if(WeaponData[i][wItemID] == itemid)
        {
            // Es un arma, buscar balas en metadata
            new metadata[128];
            format(metadata, sizeof(metadata), "%s", PlayerInventory[playerid][slot][invMetadata]);
            
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
    
    // No es arma, retornar cantidad normal
    return PlayerInventory[playerid][slot][invQuantity];
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

// Equipar objeto en mano izquierda
stock HoldItemLeft(playerid, slot)
{
    if(slot < 0 || slot >= MAX_INVENTORY_SLOTS)
        return 0;
    
    new itemid = PlayerInventory[playerid][slot][invItemID];
    if(itemid == 0)
        return 0;
    
    // Verificar si es un cargador (debe ir en mano izquierda)
    if(IsMagazineItem(itemid))
    {
        return EquipMagazineFromInventory(playerid, slot);
    }
    
    // Verificar si es un arma (no puede ir en mano izquierda)
    if(IsWeaponItem(itemid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Las armas deben ir en la mano derecha. Usa la mano izquierda solo para cargadores.");
        return 0;
    }
    
    // Remover objeto actual si existe
    if(PlayerHeldObjectLeft[playerid] != INVALID_OBJECT_ID)
    {
        RemovePlayerAttachedObject(playerid, 1);
        PlayerHeldObjectLeft[playerid] = INVALID_OBJECT_ID;
        PlayerHeldItemSlotLeft[playerid] = -1;
    }
    
    new modelid;
    switch(itemid)
    {
        case ITEM_VEHICLE_KEY_MASTER: modelid = 11746; // DoorKey1
        case ITEM_VEHICLE_KEY: modelid = 11746; // DoorKey1
        case ITEM_PHONE: modelid = 330;
        case ITEM_REPAIR_KIT: modelid = 1242;
        default: return 0;
    }
    
    // Attachar objeto a la mano izquierda (bone 5)
    SetPlayerAttachedObject(playerid, 1, modelid, 5,
        -0.08, 0.03, 0.0,
        0.0, -90.0, -120.0,
        1.0, 1.0, 1.0);
    
    PlayerHeldObjectLeft[playerid] = modelid;
    PlayerHeldItemSlotLeft[playerid] = slot;
    
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
    
    // Verificar si es un arma (debe ir en mano derecha)
    if(IsWeaponItem(itemid))
    {
        return EquipWeaponFromInventory(playerid, slot);
    }
    
    // Verificar si es un cargador (no puede ir en mano derecha)
    if(IsMagazineItem(itemid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Los cargadores deben ir en la mano izquierda. Usa la mano derecha solo para armas.");
        return 0;
    }
    
    // Remover objeto actual si existe
    if(PlayerHeldObjectRight[playerid] != INVALID_OBJECT_ID)
    {
        RemovePlayerAttachedObject(playerid, 0);
        PlayerHeldObjectRight[playerid] = INVALID_OBJECT_ID;
        PlayerHeldItemSlotRight[playerid] = -1;
    }
    
    new modelid;
    switch(itemid)
    {
        case ITEM_VEHICLE_KEY_MASTER: modelid = 11746; // DoorKey1 - Llave maestra
        case ITEM_VEHICLE_KEY: modelid = 11746; // DoorKey1 - Llave normal
        case ITEM_PHONE: modelid = 330; // Teléfono
        case ITEM_REPAIR_KIT: modelid = 1242; // Caja de herramientas
        default: return 0;
    }
    
    // Attachar objeto a la mano derecha (bone 6)
    SetPlayerAttachedObject(playerid, 0, modelid, 6,
        0.08, 0.03, 0.0,
        0.0, 90.0, 120.0,
        1.0, 1.0, 1.0);
    
    PlayerHeldObjectRight[playerid] = modelid;
    PlayerHeldItemSlotRight[playerid] = slot;
    
    new itemName[64];
    GetItemName(itemid, itemName);
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Has equipado en mano derecha: %s", itemName);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    return 1;
}

// Desequipar objeto
stock UnholdItem(playerid)
{
    if(PlayerHeldObjectRight[playerid] == INVALID_OBJECT_ID)
        return 0;
    
    RemovePlayerAttachedObject(playerid, 0);
    PlayerHeldObjectRight[playerid] = INVALID_OBJECT_ID;
    PlayerHeldItemSlotRight[playerid] = -1;
    
    SendClientMessage(playerid, COLOR_INFO, "Has guardado el objeto de la mano derecha.");
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
    if(PlayerHeldItemSlotLeft[playerid] != -1)
    {
        new slot = PlayerHeldItemSlotLeft[playerid];
        GetItemName(PlayerInventory[playerid][slot][invItemID], itemName);
        new quantity = GetItemDisplayQuantity(playerid, slot);
        format(line, sizeof(line), "{00FF00}[Mano Izquierda]\t%s\t%d\n",
            itemName, quantity);
        strcat(dialogStr, line);
    }
    else
    {
        strcat(dialogStr, "{808080}[Mano Izquierda]\t{808080}Vacia\t-\n");
    }
    
    // Mano derecha
    if(PlayerHeldItemSlotRight[playerid] != -1)
    {
        new slot = PlayerHeldItemSlotRight[playerid];
        GetItemName(PlayerInventory[playerid][slot][invItemID], itemName);
        new quantity = GetItemDisplayQuantity(playerid, slot);
        format(line, sizeof(line), "{00FF00}[Mano Derecha]\t%s\t%d\n",
            itemName, quantity);
        strcat(dialogStr, line);
    }
    else
    {
        strcat(dialogStr, "{808080}[Mano Derecha]\t{808080}Vacia\t-\n");
    }
    
    // Items en bolsillos
    for(new i = 0; i < MAX_INVENTORY_SLOTS; i++)
    {
        if(PlayerInventory[playerid][i][invItemID] > 0)
        {
            // Omitir si ya esta equipado
            if(i == PlayerHeldItemSlotLeft[playerid] || i == PlayerHeldItemSlotRight[playerid])
                continue;
            
            GetItemName(PlayerInventory[playerid][i][invItemID], itemName);
            new quantity = GetItemDisplayQuantity(playerid, i);
            format(line, sizeof(line), "Bolsillo %d\t%s\t%d\n",
                i, itemName, quantity);  // Mostrar índice real (0-19)
            strcat(dialogStr, line);
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
    
    // Listitem 0 = Mano izquierda, 1 = Mano derecha, 2+ = bolsillos
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
    
    // Contar items en bolsillos
    new bolsilloIndex = 0;
    new targetSlot = -1;
    
    for(new i = 0; i < MAX_INVENTORY_SLOTS; i++)
    {
        if(PlayerInventory[playerid][i][invItemID] > 0)
        {
            if(i == PlayerHeldItemSlotLeft[playerid] || i == PlayerHeldItemSlotRight[playerid])
                continue;
            
            if(bolsilloIndex == (listitem - 2))
            {
                targetSlot = i;
                break;
            }
            bolsilloIndex++;
        }
    }
    
    if(targetSlot == -1) return 1;
    
    // Equipar en mano derecha si esta vacia, sino en izquierda
    if(PlayerHeldItemSlotRight[playerid] == -1)
    {
        HoldItem(playerid, targetSlot);
    }
    else if(PlayerHeldItemSlotLeft[playerid] == -1)
    {
        HoldItemLeft(playerid, targetSlot);
    }
    else
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Tus manos estan llenas. Guarda algo primero.");
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

// Comando /tirar - Tirar item al suelo
CMD:tirar(playerid, params[])
{
    if(!CharacterData[playerid][cSelected])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Debes estar logueado.");
    
    new slot;
    if(sscanf(params, "d", slot))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Uso: /tirar [slot] (usa /inv para ver los slots)");
    
    if(slot < 0 || slot >= MAX_INVENTORY_SLOTS)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Slot inválido (0-19).");
    
    new result = DropItemToGround(playerid, slot);
    
    if(result == -1)
        return 1;
    
    // Mostrar animación
    ApplyAnimation(playerid, "BOMBER", "BOM_PLANT", 4.1, false, false, false, false, 0);
    
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
