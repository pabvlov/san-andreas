/*
    Sistema de Inventario de Armas
    Maneja armas y cargadores en el inventario con sistema de recarga
    - Mano derecha: Solo armas
    - Mano izquierda: Solo cargadores
    - /usar con arma en derecha + cargador en izquierda = recargar
*/

#if defined _weapon_inventory_included
    #endinput
#endif
#define _weapon_inventory_included

// Forward declarations
forward bool:IsWeaponItem(itemid);
forward bool:IsMagazineItem(itemid);

// Note: Weapon/Magazine data structures declared in inventory_system.inc
// WeaponData, MagazineData, TotalWeapons, TotalMagazines
// PlayerCurrentWeapon, PlayerCurrentAmmo, PlayerWeaponSlot

// Obtener modelo de objeto visible para el arma (modelos de armas reales de SA-MP)
stock GetWeaponObjectModel(weaponSAMPID)
{
    // Mapear weapon IDs a modelos de objetos visibles
    switch(weaponSAMPID)
    {
        case 22: return 346; // Colt 45
        case 23: return 347; // Silenced Pistol
        case 24: return 348; // Desert Eagle
        case 25: return 349; // Shotgun
        case 26: return 350; // Sawnoff Shotgun
        case 27: return 351; // Combat Shotgun
        case 28: return 352; // Micro SMG (Uzi)
        case 29: return 353; // MP5
        case 30: return 355; // AK-47
        case 31: return 356; // M4
        case 32: return 372; // Tec-9
        case 33: return 357; // Rifle
        case 34: return 358; // Sniper Rifle
    }
    return 355; // Por defecto AK-47
}

// Cargar datos de armas desde BD
stock LoadWeaponData()
{
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query), 
        "SELECT id, name, weapon_id, weapon_ammo_capacity, weapon_object_id, item_size FROM items WHERE is_weapon = 1 ORDER BY id");
    
    new Cache:result = mysql_query(g_MySQL, query);
    new rows = cache_num_rows();
    
    printf("[Weapon System] Cargando %d armas desde items...", rows);
    
    TotalWeapons = 0;
    for(new i = 0; i < rows && i < MAX_WEAPON_TYPES; i++)
    {
        // wID es el ID interno del array, wItemID es el id de la tabla items
        cache_get_value_name_int(i, "id", WeaponData[i][wItemID]);
        cache_get_value_name(i, "name", WeaponData[i][wName], 64);
        cache_get_value_name_int(i, "weapon_id", WeaponData[i][wSAMPID]);
        cache_get_value_name_int(i, "weapon_ammo_capacity", WeaponData[i][wAmmoCapacity]);
        cache_get_value_name_int(i, "weapon_object_id", WeaponData[i][wHandObject]);
        cache_get_value_name_int(i, "item_size", WeaponData[i][wItemSize]); // NUEVO: cargar tamaño desde BD
        
        WeaponData[i][wID] = i; // ID interno del array
        WeaponData[i][wSlot] = 0; // No usado
        
        printf("[Weapon System] - Arma %d: %s (ItemID:%d, SAMP:%d, Size:%d)", 
            i, WeaponData[i][wName], WeaponData[i][wItemID], WeaponData[i][wSAMPID], WeaponData[i][wItemSize]);
        
        TotalWeapons++;
    }
    
    cache_delete(result);
    printf("[Weapon System] %d armas cargadas desde items", TotalWeapons);
}

stock LoadMagazineData()
{
    new query[512];
    // JOIN con weapons y items para obtener el item_id del arma y el item_size del cargador
    mysql_format(g_MySQL, query, sizeof(query), 
        "SELECT wm.id, w.item_id as weapon_item_id, wm.name, wm.magazine_capacity, wm.hand_object_id, wm.item_id, i.item_size \
        FROM weapon_magazines wm \
        JOIN weapons w ON wm.weapon_id = w.id \
        JOIN items i ON wm.item_id = i.id \
        ORDER BY wm.id");
    
    new Cache:result = mysql_query(g_MySQL, query);
    new rows = cache_num_rows();
    
    printf("[Weapon System] Cargando %d cargadores...", rows);
    
    TotalMagazines = 0;
    for(new i = 0; i < rows && i < MAX_MAGAZINE_TYPES; i++)
    {
        cache_get_value_name_int(i, "id", MagazineData[i][mID]);
        cache_get_value_name_int(i, "weapon_item_id", MagazineData[i][mWeaponID]); // Ahora es el item_id del arma
        cache_get_value_name(i, "name", MagazineData[i][mName], 64);
        cache_get_value_name_int(i, "magazine_capacity", MagazineData[i][mCapacity]);
        cache_get_value_name_int(i, "hand_object_id", MagazineData[i][mHandObject]);
        cache_get_value_name_int(i, "item_id", MagazineData[i][mItemID]);
        cache_get_value_name_int(i, "item_size", MagazineData[i][mItemSize]); // NUEVO: cargar tamaño desde BD
        
        printf("[Weapon System] - Cargador %d: %s (WeaponItemID:%d, ItemID:%d, Size:%d)",
            i, MagazineData[i][mName], MagazineData[i][mWeaponID], 
            MagazineData[i][mItemID], MagazineData[i][mItemSize]);
        
        TotalMagazines++;
    }
    
    cache_delete(result);
    printf("[Weapon System] %d cargadores cargados", TotalMagazines);
}

// Verificar si un item es un arma
stock bool:IsWeaponItem(itemid)
{
    for(new i = 0; i < TotalWeapons; i++)
    {
        if(WeaponData[i][wItemID] == itemid)
            return true;
    }
    return false;
}

// Verificar si un item es un cargador
stock bool:IsMagazineItem(itemid)
{
    for(new i = 0; i < TotalMagazines; i++)
    {
        if(MagazineData[i][mItemID] == itemid)
            return true;
    }
    return false;
}

// Obtener datos del arma por item ID
stock GetWeaponDataByItem(itemid)
{
    for(new i = 0; i < TotalWeapons; i++)
    {
        if(WeaponData[i][wItemID] == itemid)
            return i;
    }
    return -1;
}

// Obtener datos del cargador por item ID
stock GetMagazineDataByItem(itemid)
{
    for(new i = 0; i < TotalMagazines; i++)
    {
        if(MagazineData[i][mItemID] == itemid)
            return i;
    }
    return -1;
}
// Verificar si un cargador es compatible con un arma
stock bool:IsMagazineCompatible(magazineIdx, weaponIdx)
{
    if(magazineIdx < 0 || magazineIdx >= TotalMagazines)
        return false;
    
    if(weaponIdx < 0 || weaponIdx >= TotalWeapons)
        return false;
    
    // mWeaponID debe ser el item_id del arma (ej: 10 para Pistola Silenciada)
    // Comparar con wItemID que es el id de items
    new bool:compatible = (MagazineData[magazineIdx][mWeaponID] == WeaponData[weaponIdx][wItemID]);
    
    printf("[Weapon Debug] Compatibilidad: Cargador[%d] weaponID=%d vs Arma[%d] itemID=%d = %s",
        magazineIdx, MagazineData[magazineIdx][mWeaponID],
        weaponIdx, WeaponData[weaponIdx][wItemID],
        compatible ? "Compatible" : "Incompatible");
    
    return compatible;
}

// Equipar arma desde el inventario (mano derecha)
stock EquipWeaponFromInventory(playerid, slot)
{
    if(slot < 0 || slot >= MAX_INVENTORY_SLOTS)
        return 0;
    
    new itemid = PlayerInventory[playerid][slot][invItemID];
    if(itemid == 0)
        return 0;
    
    new weaponIdx = GetWeaponDataByItem(itemid);
    if(weaponIdx == -1)
        return 0;
    
    // Verificar si ya tiene un arma equipada
    if(PlayerCurrentWeapon[playerid] != -1)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Ya tienes un arma equipada. Guárdala primero con /guardar");
        return 0;
    }
    
    // Verificar si mano derecha está ocupada
    if(PlayerHeldItemSlotRight[playerid] != -1 && PlayerHeldObjectRight[playerid] != INVALID_OBJECT_ID)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Tu mano derecha está ocupada. Guarda el item primero.");
        return 0;
    }
    
    // Si el arma está en la espalda (slot 2), removerla visualmente
    if(slot == 2 && PlayerBackSlot[playerid] == 2)
    {
        if(IsPlayerAttachedObjectSlotUsed(playerid, 3))
        {
            RemovePlayerAttachedObject(playerid, 3);
        }
        PlayerBackSlot[playerid] = -1;
    }
    
    // Guardar datos del arma antes de limpiar el slot
    PlayerHeldItemIDRight[playerid] = itemid;
    PlayerHeldItemQuantityRight[playerid] = PlayerInventory[playerid][slot][invQuantity];
    format(PlayerHeldItemMetadataRight[playerid], 256, "%s", PlayerInventory[playerid][slot][invMetadata]);
    
    // Las armas no marcan flag de item grande (van a la espalda cuando no se usan)
    
    // NO dar el arma real - solo mostrar objeto 3D
    // El arma se dará automáticamente al disparar
    
    // Obtener modelo de objeto correcto
    new objectModel = GetWeaponObjectModel(WeaponData[weaponIdx][wSAMPID]);
    
    // Debug
    printf("[Weapon] Equipando arma '%s' (SAMP ID:%d) - Object Model: %d", 
        WeaponData[weaponIdx][wName], WeaponData[weaponIdx][wSAMPID], objectModel);
    
    // Valores por defecto para attached object
    new Float:posX = 0.0, Float:posY = 0.0, Float:posZ = 0.0;
    new Float:rotX = 2.50, Float:rotY = 2.09, Float:rotZ = -0.60;
    new Float:scaleX = 1.0, Float:scaleY = 0.95, Float:scaleZ = 1.10;
    
    // Verificar si hay posición personalizada guardada (slot 0 = mano derecha)
    if(GetAttachedObjectPosition(playerid, itemid, 0, posX, posY, posZ, rotX, rotY, rotZ, scaleX, scaleY, scaleZ))
    {
        printf("[Weapon] Usando posición personalizada para arma %d en mano", itemid);
    }
    
    // Usar SetPlayerAttachedObject en lugar de CreatePlayerObject (más visible)
    // Índice 0 para arma, bone 6 = mano derecha
    SetPlayerAttachedObject(playerid, 0, objectModel, 6,
        posX, posY, posZ,       // X, Y, Z (posición)
        rotX, rotY, rotZ,       // RX, RY, RZ (rotación ajustada)
        scaleX, scaleY, scaleZ); // ScaleX, ScaleY, ScaleZ
    
    PlayerHeldObjectRight[playerid] = 1; // Marcador de que hay objeto (no es ID real con SetPlayerAttachedObject)
    
    printf("[Weapon] Objeto modelo %d attachado a jugador %s en bone 6 (index 0)", 
        objectModel, CharacterData[playerid][cName]);
    
    PlayerHeldItemSlotRight[playerid] = slot;
    PlayerCurrentWeapon[playerid] = weaponIdx;
    PlayerWeaponSlot[playerid] = slot;
    
    // LIMPIAR el slot del inventario - el arma se MUEVE a la mano
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
    
    // Cargar munición desde metadata guardada
    new ammo = 0;
    if(strlen(PlayerHeldItemMetadataRight[playerid]) > 0)
    {
        // Parsear JSON simple para obtener ammo
        new pos = strfind(PlayerHeldItemMetadataRight[playerid], "ammo\":");
        if(pos != -1)
        {
            pos += 6; // Saltar "ammo":
            new ammoStr[16];
            new idx = 0;
            while(PlayerHeldItemMetadataRight[playerid][pos] >= '0' && PlayerHeldItemMetadataRight[playerid][pos] <= '9' && idx < 15)
            {
                ammoStr[idx++] = PlayerHeldItemMetadataRight[playerid][pos++];
            }
            ammoStr[idx] = 0;
            ammo = strval(ammoStr);
            
            printf("[Weapon] Cargando arma con %d balas desde metadata: %s", ammo, PlayerHeldItemMetadataRight[playerid]);
        }
    }
    
    PlayerCurrentAmmo[playerid] = ammo;
    
    // Si tiene munición, dar el arma real
    if(ammo > 0)
    {
        GivePlayerWeapon(playerid, WEAPON:WeaponData[weaponIdx][wSAMPID], ammo);
    }
    
    new string[128];
    if(ammo > 0)
    {
        format(string, sizeof(string), "{00FF00}Has equipado: {FFFFFF}%s {00FF00}(%d balas)", WeaponData[weaponIdx][wName], ammo);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
    }
    else
    {
        format(string, sizeof(string), "{00FF00}Has equipado: {FFFFFF}%s {FF0000}(0 balas)", WeaponData[weaponIdx][wName]);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        SendClientMessage(playerid, COLOR_INFO, "{FFD700}INFO: {FFFFFF}El arma no tiene munición. Equipa un cargador en la mano izquierda y usa /usar");
    }
    
    return 1;
}

// Equipar cargador en mano izquierda
stock EquipMagazineFromInventory(playerid, slot)
{
    if(slot < 0 || slot >= MAX_INVENTORY_SLOTS)
        return 0;
    
    new itemid = PlayerInventory[playerid][slot][invItemID];
    if(itemid == 0)
        return 0;
    
    new magIdx = GetMagazineDataByItem(itemid);
    if(magIdx == -1)
        return 0;
    
    // Verificar si mano izquierda está ocupada
    if(PlayerHeldItemSlotLeft[playerid] != -1 && PlayerHeldObjectLeft[playerid] != INVALID_OBJECT_ID)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Tu mano izquierda está ocupada. Guarda el item primero.");
        return 0;
    }
    
    // Si hay algo equipado, desequipar primero
    if(PlayerHeldObjectLeft[playerid] != INVALID_OBJECT_ID)
    {
        UnholdItemLeft(playerid);
    }
    
    // Los cargadores pueden equiparse incluso si hay item grande
    
    // Guardar datos del cargador antes de limpiar el slot
    PlayerHeldItemIDLeft[playerid] = itemid;
    PlayerHeldItemQuantityLeft[playerid] = PlayerInventory[playerid][slot][invQuantity];
    format(PlayerHeldItemMetadataLeft[playerid], 256, "%s", PlayerInventory[playerid][slot][invMetadata]);
    
    // Equipar objeto en mano izquierda
    PlayerHeldObjectLeft[playerid] = CreatePlayerObject(playerid, MagazineData[magIdx][mHandObject],
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    
    AttachPlayerObjectToPlayer(PlayerHeldObjectLeft[playerid], playerid, 5,
        -0.08, 0.03, -0.05,  // X, Y, Z (negativo para mano izquierda)
        88.0, 0.0, 90.0);    // RX, RY, RZ
    
    PlayerHeldItemSlotLeft[playerid] = slot;
    
    // LIMPIAR el slot del inventario - el cargador se MUEVE a la mano
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
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Has equipado: {FFFFFF}%s", MagazineData[magIdx][mName]);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    SendClientMessage(playerid, COLOR_INFO, "{FFD700}INFO: {FFFFFF}Usa /usar para recargar tu arma si tienes una en la mano derecha");
    
    return 1;
}

// Desequipar arma
stock UnequipWeapon(playerid)
{
    if(PlayerCurrentWeapon[playerid] == -1)
        return 0;
    
    // Quitar arma del jugador
    new weaponIdx = PlayerCurrentWeapon[playerid];
    new currentAmmo = PlayerCurrentAmmo[playerid];
    SetPlayerAmmo(playerid, WEAPON:WeaponData[weaponIdx][wSAMPID], 0);
    
    // Guardar slot antes de limpiar
    new originalSlot = PlayerWeaponSlot[playerid];
    new itemid = PlayerHeldItemIDRight[playerid];
    
    // Actualizar metadata con munición actual
    new metadata[64];
    format(metadata, sizeof(metadata), "{\"ammo\":%d}", currentAmmo);
    format(PlayerHeldItemMetadataRight[playerid], 256, "%s", metadata);
    
    // Remover objeto attachado (SetPlayerAttachedObject usa índice 0)
    if(IsPlayerAttachedObjectSlotUsed(playerid, 0))
    {
        RemovePlayerAttachedObject(playerid, 0);
    }
    PlayerHeldObjectRight[playerid] = INVALID_OBJECT_ID;
    
    // Limpiar variables de arma equipada
    PlayerHeldItemSlotRight[playerid] = -1;
    PlayerCurrentWeapon[playerid] = -1;
    PlayerCurrentAmmo[playerid] = 0;
    PlayerWeaponSlot[playerid] = -1;
    
    // RESTAURAR al inventario con el sistema de UnholdItem
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
    
    // Si no hay slots libres, forzar al slot original (esto no debería pasar)
    if(targetSlot == -1)
    {
        targetSlot = originalSlot;
    }
    
    // Restaurar item al slot
    new characterID = CharacterData[playerid][cID];
    PlayerInventory[playerid][targetSlot][invItemID] = itemid;
    PlayerInventory[playerid][targetSlot][invCharacterID] = characterID;
    PlayerInventory[playerid][targetSlot][invSlot] = targetSlot;
    PlayerInventory[playerid][targetSlot][invQuantity] = 1;
    format(PlayerInventory[playerid][targetSlot][invMetadata], 128, "%s", metadata);
    PlayerSlots[playerid][targetSlot][slotOccupied] = 1;
    PlayerInventoryCount[playerid]++;
    
    // Guardar en base de datos
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO character_inventory (character_id, item_id, slot, quantity, metadata) VALUES (%d, %d, %d, %d, '%e')",
        characterID, itemid, targetSlot, 1, metadata);
    mysql_tquery(g_MySQL, query);
    
    // Si el arma está en el slot 2 (espalda), attacharla visualmente
    if(targetSlot == 2)
    {
        new itemSize = GetItemSize(itemid);
        if(itemSize == ITEM_SIZE_LARGE)
        {
            AttachItemToBack(playerid, targetSlot);
        }
    }
    
    // Limpiar datos temporales
    PlayerHeldItemIDRight[playerid] = 0;
    PlayerHeldItemQuantityRight[playerid] = 0;
    PlayerHeldItemMetadataRight[playerid][0] = EOS;
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Has guardado el arma {FFFFFF}(%d balas guardadas)", currentAmmo);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    return 1;
}

// Desequipar cargador
stock UnequipMagazine(playerid)
{
    if(PlayerHeldItemSlotLeft[playerid] == -1)
        return 0;
    
    new originalSlot = PlayerHeldItemSlotLeft[playerid];
    new itemid = PlayerHeldItemIDLeft[playerid];
    new quantity = PlayerHeldItemQuantityLeft[playerid];
    new metadata[256];
    format(metadata, sizeof(metadata), "%s", PlayerHeldItemMetadataLeft[playerid]);
    
    // Destruir objeto de mano izquierda
    if(PlayerHeldObjectLeft[playerid] != INVALID_OBJECT_ID)
    {
        DestroyPlayerObject(playerid, PlayerHeldObjectLeft[playerid]);
        PlayerHeldObjectLeft[playerid] = INVALID_OBJECT_ID;
    }
    
    PlayerHeldItemSlotLeft[playerid] = -1;
    
    // RESTAURAR al inventario
    // Verificar si el slot original está libre
    new targetSlot = -1;
    if(PlayerInventory[playerid][originalSlot][invItemID] == 0)
    {
        targetSlot = originalSlot;
    }
    else
    {
        // Buscar slot libre
        new itemSize = GetItemSize(itemid);
        for(new i = 0; i < MAX_INVENTORY_SLOTS; i++)
        {
            if(PlayerInventory[playerid][i][invItemID] == 0)
            {
                if(itemSize == ITEM_SIZE_LARGE && PlayerSlots[playerid][i][slotType] != SLOT_TYPE_BACK)
                    continue;
                if(itemSize == ITEM_SIZE_SMALL && PlayerSlots[playerid][i][slotType] == SLOT_TYPE_BACK)
                    continue;
                
                targetSlot = i;
                break;
            }
        }
    }
    
    if(targetSlot == -1)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes espacio en el inventario.");
        return 0;
    }
    
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
    
    SendClientMessage(playerid, COLOR_SUCCESS, "{00FF00}Has guardado el cargador.");
    
    return 1;
}

// Recargar arma con cargador (se llama con /usar)
stock ReloadWeaponFromMagazine(playerid)
{
    // Verificar que tiene un arma equipada
    if(PlayerCurrentWeapon[playerid] == -1)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes un arma equipada en la mano derecha.");
        return 0;
    }
    
    // Verificar que tiene un cargador en la mano izquierda
    if(PlayerHeldItemSlotLeft[playerid] == -1)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes un cargador en la mano izquierda.");
        return 0;
    }
    
    // IMPORTANTE: Cuando el cargador está equipado, el item está en PlayerHeldItemIDLeft, NO en el slot
    new magSlot = PlayerHeldItemSlotLeft[playerid];
    new magItemID = PlayerHeldItemIDLeft[playerid];
    new magIdx = GetMagazineDataByItem(magItemID);
    
    if(magIdx == -1)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes un cargador válido en la mano izquierda.");
        return 0;
    }
    
    new weaponIdx = PlayerCurrentWeapon[playerid];
    
    // Verificar compatibilidad
    if(!IsMagazineCompatible(magIdx, weaponIdx))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Este cargador no es compatible con tu arma.");
        return 0;
    }
    
    // Verificar si el arma ya está llena
    if(PlayerCurrentAmmo[playerid] >= WeaponData[weaponIdx][wAmmoCapacity])
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El arma ya está llena.");
        return 0;
    }
    
    // NUEVO: Obtener balas del cargador actual (parsear metadata de la variable temporal)
    new bulletsInMag = MagazineData[magIdx][mCapacity]; // Valor por defecto
    new metadata[256];
    format(metadata, sizeof(metadata), "%s", PlayerHeldItemMetadataLeft[playerid]);
    
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
            bulletsInMag = strval(bulletsStr);
        }
    }
    
    // Calcular cuántas balas se pueden recargar
    new ammosNeeded = WeaponData[weaponIdx][wAmmoCapacity] - PlayerCurrentAmmo[playerid];
    new ammosToAdd = (ammosNeeded < bulletsInMag) ? ammosNeeded : bulletsInMag;
    
    // Añadir munición
    PlayerCurrentAmmo[playerid] += ammosToAdd;
    
    // IMPORTANTE: Dar el arma con la munición TOTAL (no sumar, establecer)
    // SetPlayerAmmo primero para limpiar, luego dar con la cantidad correcta
    SetPlayerAmmo(playerid, WEAPON:WeaponData[weaponIdx][wSAMPID], PlayerCurrentAmmo[playerid]);
    SetPlayerArmedWeapon(playerid, WEAPON:WeaponData[weaponIdx][wSAMPID]);
    
    // Actualizar metadata del arma con la nueva munición
    if(PlayerWeaponSlot[playerid] != -1)
    {
        new weaponSlot = PlayerWeaponSlot[playerid];
        new query[256];
        new metadataWeapon[64];
        format(metadataWeapon, sizeof(metadataWeapon), "{\"ammo\":%d}", PlayerCurrentAmmo[playerid]);
        
        mysql_format(g_MySQL, query, sizeof(query),
            "UPDATE character_inventory SET metadata = '%e' WHERE character_id = %d AND slot = %d",
            metadataWeapon, CharacterData[playerid][cID], weaponSlot);
        mysql_tquery(g_MySQL, query);
        
        // Actualizar en memoria también
        format(PlayerInventory[playerid][weaponSlot][invMetadata], 128, "%s", metadataWeapon);
        
        printf("[Weapon] Actualizando metadata del arma en slot %d con %d balas", weaponSlot, PlayerCurrentAmmo[playerid]);
    }
    
    // NUEVO: Dropear cargador actual al suelo (conservando balas restantes si tiene)
    new Float:x, Float:y, Float:z, Float:angle;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, angle);
    x += (1.0 * floatsin(-angle, degrees));
    y += (1.0 * floatcos(-angle, degrees));
    
    // Crear metadata del cargador con balas restantes (0 si se consumieron todas)
    new remainingBullets = bulletsInMag - ammosToAdd;
    new dropMetadata[64];
    format(dropMetadata, sizeof(dropMetadata), "{\"bullets\":%d}", remainingBullets);
    
    // Crear dropped item (siempre se dropea, incluso si tiene 0 balas)
    new dropSlot = CreateDroppedItemAt(magItemID, 1, dropMetadata, x, y, z, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));
    
    if(dropSlot != -1)
    {
        printf("[Weapon] Cargador dropeado al suelo con %d balas restantes (slot %d)", remainingBullets, dropSlot);
    }
    
    // Remover cargador del inventario
    RemovePlayerItem(playerid, magSlot);
    
    // Soltar objeto de mano izquierda
    if(PlayerHeldObjectLeft[playerid] != INVALID_OBJECT_ID)
    {
        DestroyPlayerObject(playerid, PlayerHeldObjectLeft[playerid]);
        PlayerHeldObjectLeft[playerid] = INVALID_OBJECT_ID;
    }
    PlayerHeldItemSlotLeft[playerid] = -1;
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Recargado: {FFFFFF}%s {00FF00}+%d balas (%d/%d)", WeaponData[weaponIdx][wName], ammosToAdd, PlayerCurrentAmmo[playerid], WeaponData[weaponIdx][wAmmoCapacity]);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    ApplyAnimation(playerid, "COLT45", "colt45_reload", 4.1, false, false, false, false, 0);
    
    return 1;
}

// Obtener nombre del arma por índice
stock GetWeaponNameByIndex(idx, name[], len = sizeof(name))
{
    if(idx < 0 || idx >= TotalWeapons)
    {
        format(name, len, "Arma desconocida");
        return 0;
    }
    
    format(name, len, "%s", WeaponData[idx][wName]);
    return 1;
}

// ========== CALLBACKS ==========

// Bloquear cambio de armas cuando tiene un arma equipada
public OnPlayerUpdate(playerid)
{
    if(PlayerCurrentWeapon[playerid] != -1)
    {
        new weaponIdx = PlayerCurrentWeapon[playerid];
        new currentWeapon = GetPlayerWeapon(playerid);
        
        // Si el jugador tiene munición, asegurarse que tenga el arma correcta
        if(PlayerCurrentAmmo[playerid] > 0)
        {
            // Si no tiene el arma correcta, dársela
            if(currentWeapon != WeaponData[weaponIdx][wSAMPID])
            {
                SetPlayerArmedWeapon(playerid, WEAPON:WeaponData[weaponIdx][wSAMPID]);
            }
        }
        else
        {
            // Si no tiene munición, quitar todas las armas para que solo vea el objeto
            if(currentWeapon != _:WEAPON_FIST)
            {
                SetPlayerArmedWeapon(playerid, WEAPON_FIST);
            }
        }
    }
    
    #if defined Weapon_OnPlayerUpdate
        return Weapon_OnPlayerUpdate(playerid);
    #else
        return 1;
    #endif
}

#if defined _ALS_OnPlayerUpdate
    #undef OnPlayerUpdate
#else
    #define _ALS_OnPlayerUpdate
#endif
#define OnPlayerUpdate Weapon_OnPlayerUpdate
#if defined Weapon_OnPlayerUpdate
    forward Weapon_OnPlayerUpdate(playerid);
#endif

// Reducir munición al disparar
#if !defined FILTERSCRIPT
public OnPlayerWeaponShot(playerid, WEAPON:weaponid, BULLET_HIT_TYPE:hittype, hitid, Float:fX, Float:fY, Float:fZ)
{
    // Reducir munición si tiene un arma equipada del sistema
    if(PlayerCurrentWeapon[playerid] != -1)
    {
        new weaponIdx = PlayerCurrentWeapon[playerid];
        
        if(WeaponData[weaponIdx][wSAMPID] == _:weaponid)
        {
            if(PlayerCurrentAmmo[playerid] > 0)
            {
                PlayerCurrentAmmo[playerid]--;
                
                // Si se queda sin munición
                if(PlayerCurrentAmmo[playerid] <= 0)
                {
                    SetPlayerAmmo(playerid, weaponid, 0);
                    SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Te has quedado sin munición. Equipa un cargador en la mano izquierda y usa /usar");
                    GameTextForPlayer(playerid, "~r~SIN MUNICION", 2000, 3);
                }
            }
        }
    }
    
    return 1;
}
#endif

// Limpiar al desconectar
stock WeaponInventory_OnDisconnect(playerid)
{
    PlayerCurrentWeapon[playerid] = -1;
    PlayerCurrentAmmo[playerid] = 0;
    PlayerWeaponSlot[playerid] = -1;
    
    return 1;
}

// ========== COMANDOS DE ADMINISTRACIÓN ==========

// Dar arma a un jugador (admin)
CMD:dararma(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes permisos.");
    
    new targetid, weaponid, quantity;
    if(sscanf(params, "udd", targetid, weaponid, quantity))
    {
        SendClientMessage(playerid, COLOR_INFO, "{FFFF00}Uso: {FFFFFF}/dararma [playerid] [weaponid] [cantidad]");
        SendClientMessage(playerid, COLOR_INFO, "{FFFFFF}Lista de armas disponibles:");
        
        // Mostrar todas las armas disponibles
        for(new i = 0; i < TotalWeapons; i++)
        {
            new string[128];
            format(string, sizeof(string), "{AAAAAA}ID %d: {FFFFFF}%s {AAAAAA}(SAMP:%d)", 
                WeaponData[i][wID], WeaponData[i][wName], WeaponData[i][wSAMPID]);
            SendClientMessage(playerid, COLOR_INFO, string);
        }
        return 1;
    }
    
    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Jugador no conectado.");
    
    // Buscar arma por ID
    new weaponIdx = -1;
    for(new i = 0; i < TotalWeapons; i++)
    {
        if(WeaponData[i][wID] == weaponid)
        {
            weaponIdx = i;
            break;
        }
    }
    
    if(weaponIdx == -1)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}ID de arma inválido.");
    
    if(WeaponData[weaponIdx][wItemID] == 0)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Esta arma no tiene item_id configurado en la base de datos.");
    
    // Dar el item del arma al jugador
    if(GivePlayerItem(targetid, WeaponData[weaponIdx][wItemID], quantity, ""))
    {
        new string[144];
        format(string, sizeof(string), "{00FF00}Le has dado {FFFFFF}%d x %s {00FF00}a {FFFFFF}%s",
            quantity, WeaponData[weaponIdx][wName], CharacterData[targetid][cName]);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        
        format(string, sizeof(string), "{00FF00}Has recibido {FFFFFF}%d x %s {00FF00}de un administrador",
            quantity, WeaponData[weaponIdx][wName]);
        SendClientMessage(targetid, COLOR_SUCCESS, string);
        
        printf("[Admin] %s dio %d x %s a %s", 
            CharacterData[playerid][cName], quantity, WeaponData[weaponIdx][wName], CharacterData[targetid][cName]);
    }
    else
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error al dar el arma. Inventario lleno?");
    }
    
    return 1;
}

// Dar cargador a un jugador (admin)
CMD:darcargador(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes permisos.");
    
    new targetid, magazineid, quantity;
    if(sscanf(params, "udd", targetid, magazineid, quantity))
    {
        SendClientMessage(playerid, COLOR_INFO, "{FFFF00}Uso: {FFFFFF}/darcargador [playerid] [magazineid] [cantidad]");
        SendClientMessage(playerid, COLOR_INFO, "{FFFFFF}Lista de cargadores disponibles:");
        
        // Mostrar primeros 10 cargadores como ejemplo
        for(new i = 0; i < 10 && i < TotalMagazines; i++)
        {
            new string[144];
            format(string, sizeof(string), "{AAAAAA}ID %d: {FFFFFF}%s {AAAAAA}(para arma ID %d)", 
                MagazineData[i][mID], MagazineData[i][mName], MagazineData[i][mWeaponID]);
            SendClientMessage(playerid, COLOR_INFO, string);
        }
        SendClientMessage(playerid, COLOR_INFO, "{AAAAAA}Usa /listcargadores para ver todos");
        return 1;
    }
    
    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Jugador no conectado.");
    
    // Buscar cargador por ID
    new magIdx = -1;
    for(new i = 0; i < TotalMagazines; i++)
    {
        if(MagazineData[i][mID] == magazineid)
        {
            magIdx = i;
            break;
        }
    }
    
    if(magIdx == -1)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}ID de cargador inválido.");
    
    if(MagazineData[magIdx][mItemID] == 0)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Este cargador no tiene item_id configurado en la base de datos.");
    
    // Dar el item del cargador al jugador
    if(GivePlayerItem(targetid, MagazineData[magIdx][mItemID], quantity, ""))
    {
        new string[144];
        format(string, sizeof(string), "{00FF00}Le has dado {FFFFFF}%d x %s {00FF00}a {FFFFFF}%s",
            quantity, MagazineData[magIdx][mName], CharacterData[targetid][cName]);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        
        format(string, sizeof(string), "{00FF00}Has recibido {FFFFFF}%d x %s {00FF00}de un administrador",
            quantity, MagazineData[magIdx][mName]);
        SendClientMessage(targetid, COLOR_SUCCESS, string);
        
        printf("[Admin] %s dio %d x %s a %s", 
            CharacterData[playerid][cName], quantity, MagazineData[magIdx][mName], CharacterData[targetid][cName]);
    }
    else
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error al dar el cargador. Inventario lleno?");
    }
    
    return 1;
}

// Listar todas las armas disponibles
CMD:listarmas(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes permisos.");
    
    new header[128];
    format(header, sizeof(header), "{FFD700}========== LISTA DE ARMAS (Total: %d) ==========", TotalWeapons);
    SendClientMessage(playerid, COLOR_INFO, header);
    
    if(TotalWeapons == 0)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No hay armas cargadas. Verifica que la tabla weapons exista y tenga datos.");
        return 1;
    }
    
    for(new i = 0; i < TotalWeapons; i++)
    {
        new string[144];
        format(string, sizeof(string), "{AAAAAA}ID %d: {FFFFFF}%s {AAAAAA}(SAMP ID: %d, Capacidad: %d, Item: %d)", 
            WeaponData[i][wID], WeaponData[i][wName], WeaponData[i][wSAMPID], 
            WeaponData[i][wAmmoCapacity], WeaponData[i][wItemID]);
        SendClientMessage(playerid, COLOR_INFO, string);
    }
    
    return 1;
}

// Listar todos los cargadores disponibles
CMD:listcargadores(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes permisos.");
    
    new header[128];
    format(header, sizeof(header), "{FFD700}========== LISTA DE CARGADORES (Total: %d) ==========", TotalMagazines);
    SendClientMessage(playerid, COLOR_INFO, header);
    
    if(TotalMagazines == 0)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No hay cargadores cargados. Verifica que la tabla weapon_magazines exista y tenga datos.");
        return 1;
    }
    
    for(new i = 0; i < TotalMagazines; i++)
    {
        new string[160];
        format(string, sizeof(string), "{AAAAAA}ID %d: {FFFFFF}%s {AAAAAA}(Arma ID: %d, Balas: %d, Item: %d)", 
            MagazineData[i][mID], MagazineData[i][mName], MagazineData[i][mWeaponID],
            MagazineData[i][mCapacity], MagazineData[i][mItemID]);
        SendClientMessage(playerid, COLOR_INFO, string);
    }
    
    return 1;
}

// Comando para editar la posición del arma attachada (índice 0)
cmd_editattachedobject(playerid, const params[])
{
    if(!CharacterData[playerid][cSelected])
        return SendClientMessage(playerid, COLOR_ERROR, "Debes tener un personaje seleccionado.");
    
    // Verificar si tiene un parámetro (0 para mano derecha, 1 para mano izquierda, 3 para espalda)
    new slotIndex = -1;
    
    if(strlen(params) > 0)
    {
        slotIndex = strval(params);
        if(slotIndex != 0 && slotIndex != 1 && slotIndex != 3)
        {
            SendClientMessage(playerid, COLOR_ERROR, "Slot inválido. Usa: /editattachedobject [0=mano derecha | 1=mano izquierda | 3=espalda]");
            return 1;
        }
    }
    
    // Si no se especificó slot, detectar automáticamente
    if(slotIndex == -1)
    {
        // Prioridad: espalda primero, luego mano derecha, luego mano izquierda
        if(IsPlayerAttachedObjectSlotUsed(playerid, 3))
        {
            slotIndex = 3;
        }
        else if(PlayerCurrentWeapon[playerid] != -1 || IsPlayerAttachedObjectSlotUsed(playerid, 0))
        {
            slotIndex = 0;
        }
        else if(IsPlayerAttachedObjectSlotUsed(playerid, 1))
        {
            slotIndex = 1;
        }
        else
        {
            SendClientMessage(playerid, COLOR_ERROR, "No tienes ningún objeto equipado.");
            SendClientMessage(playerid, COLOR_INFO, "Uso: /editattachedobject [0=mano derecha | 1=mano izquierda | 3=espalda]");
            return 1;
        }
    }
    
    // Verificar que el slot esté en uso
    if(!IsPlayerAttachedObjectSlotUsed(playerid, slotIndex))
    {
        if(slotIndex == 0)
            SendClientMessage(playerid, COLOR_ERROR, "No tienes ningún objeto en la mano derecha.");
        else if(slotIndex == 1)
            SendClientMessage(playerid, COLOR_ERROR, "No tienes ningún objeto en la mano izquierda.");
        else
            SendClientMessage(playerid, COLOR_ERROR, "No tienes ningún item en la espalda.");
        return 1;
    }
    
    // Abrir editor
    EditAttachedObject(playerid, slotIndex);
    
    new string[128];
    if(slotIndex == 0)
        format(string, sizeof(string), "{33CCFF}Editando objeto en {FFFFFF}MANO DERECHA");
    else if(slotIndex == 1)
        format(string, sizeof(string), "{33CCFF}Editando objeto en {FFFFFF}MANO IZQUIERDA");
    else
        format(string, sizeof(string), "{33CCFF}Editando objeto en {FFFFFF}ESPALDA");
    
    SendClientMessage(playerid, COLOR_INFO, string);
    SendClientMessage(playerid, COLOR_INFO, "Ajusta la posición del objeto. Los valores se guardarán automáticamente.");
    
    return 1;
}

// Callback cuando el jugador termina de editar el objeto attachado
public OnPlayerEditAttachedObject(playerid, EDIT_RESPONSE:response, index, modelid, boneid, Float:fOffsetX, Float:fOffsetY, Float:fOffsetZ, Float:fRotX, Float:fRotY, Float:fRotZ, Float:fScaleX, Float:fScaleY, Float:fScaleZ)
{
    if(response != EDIT_RESPONSE_FINAL)
    {
        if(response == EDIT_RESPONSE_CANCEL)
            SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Edición cancelada.");
        return 1;
    }
    
    // Slot 0 = Mano derecha (arma o item)
    if(index == 0)
    {
        new itemid = 0;
        
        // Obtener item ID actual
        if(PlayerCurrentWeapon[playerid] != -1)
        {
            // Es un arma
            itemid = WeaponData[PlayerCurrentWeapon[playerid]][wItemID];
        }
        else if(PlayerHeldItemIDRight[playerid] != 0)
        {
            // Es un item normal
            itemid = PlayerHeldItemIDRight[playerid];
        }
        
        if(itemid != 0)
        {
            // Guardar posición personalizada
            SaveAttachedObjectPosition(playerid, itemid, 0, fOffsetX, fOffsetY, fOffsetZ, fRotX, fRotY, fRotZ, fScaleX, fScaleY, fScaleZ);
            
            new itemName[64];
            GetItemName(itemid, itemName);
            
            new string[256];
            format(string, sizeof(string), "{00FF00}Posición de {FFFFFF}%s {00FF00}guardada.", itemName);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
            
            printf("[EditAttached] %s guardó posición para item %d (mano): X=%.2f Y=%.2f Z=%.2f RX=%.2f RY=%.2f RZ=%.2f",
                CharacterData[playerid][cName], itemid, fOffsetX, fOffsetY, fOffsetZ, fRotX, fRotY, fRotZ);
        }
    }
    // Slot 1 = Mano izquierda
    else if(index == 1 && PlayerHeldObjectLeft[playerid] != INVALID_OBJECT_ID)
    {
        new itemid = PlayerHeldItemIDLeft[playerid];
        
        if(itemid != 0)
        {
            // Guardar posición personalizada
            SaveAttachedObjectPosition(playerid, itemid, 1, fOffsetX, fOffsetY, fOffsetZ, fRotX, fRotY, fRotZ, fScaleX, fScaleY, fScaleZ);
            
            new itemName[64];
            GetItemName(itemid, itemName);
            
            new string[256];
            format(string, sizeof(string), "{00FF00}Posición de {FFFFFF}%s {00FF00}en mano izquierda guardada.", itemName);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
            
            printf("[EditAttached] %s guardó posición para item %d (mano izquierda): X=%.2f Y=%.2f Z=%.2f RX=%.2f RY=%.2f RZ=%.2f",
                CharacterData[playerid][cName], itemid, fOffsetX, fOffsetY, fOffsetZ, fRotX, fRotY, fRotZ);
        }
    }
    // Slot 3 = Espalda (items grandes)
    else if(index == 3 && PlayerBackSlot[playerid] != -1)
    {
        new slot = PlayerBackSlot[playerid];
        new itemid = PlayerInventory[playerid][slot][invItemID];
        
        if(itemid != 0)
        {
            // Guardar posición personalizada
            SaveAttachedObjectPosition(playerid, itemid, 3, fOffsetX, fOffsetY, fOffsetZ, fRotX, fRotY, fRotZ, fScaleX, fScaleY, fScaleZ);
            
            new itemName[64];
            GetItemName(itemid, itemName);
            
            new string[256];
            format(string, sizeof(string), "{00FF00}Posición de {FFFFFF}%s {00FF00}en espalda guardada.", itemName);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
            
            printf("[EditAttached] %s guardó posición para item %d (espalda): X=%.2f Y=%.2f Z=%.2f RX=%.2f RY=%.2f RZ=%.2f",
                CharacterData[playerid][cName], itemid, fOffsetX, fOffsetY, fOffsetZ, fRotX, fRotY, fRotZ);
        }
    }
    
    return 1;
}
