/*
    Sistema de Concesionarias - Compra/Venta de Vehículos
*/

#if defined _dealership_system_included
    #endinput
#endif
#define _dealership_system_included

#define MAX_VEHICLE_MODELS 212

// Información del catálogo de vehículos
new const VehicleModelNames[][] = {
    "Landstalker", "Bravura", "Buffalo", "Linerunner", "Perennial", "Sentinel", "Dumper", "Firetruck", "Trashmaster", "Stretch",
    "Manana", "Infernus", "Voodoo", "Pony", "Mule", "Cheetah", "Ambulance", "Leviathan", "Moonbeam", "Esperanto",
    "Taxi", "Washington", "Bobcat", "Mr Whoopee", "BF Injection", "Hunter", "Premier", "Enforcer", "Securicar", "Banshee",
    "Predator", "Bus", "Rhino", "Barracks", "Hotknife", "Trailer", "Previon", "Coach", "Cabbie", "Stallion",
    "Rumpo", "RC Bandit", "Romero", "Packer", "Monster", "Admiral", "Squalo", "Seasparrow", "Pizzaboy", "Tram",
    "Trailer", "Turismo", "Speeder", "Reefer", "Tropic", "Flatbed", "Yankee", "Caddy", "Solair", "Berkley's RC Van",
    "Skimmer", "PCJ-600", "Faggio", "Freeway", "RC Baron", "RC Raider", "Glendale", "Oceanic", "Sanchez", "Sparrow",
    "Patriot", "Quad", "Coastguard", "Dinghy", "Hermes", "Sabre", "Rustler", "ZR-350", "Walton", "Regina",
    "Comet", "BMX", "Burrito", "Camper", "Marquis", "Baggage", "Dozer", "Maverick", "News Chopper", "Rancher",
    "FBI Rancher", "Virgo", "Greenwood", "Jetmax", "Hotring", "Sandking", "Blista Compact", "Police Maverick", "Boxville", "Benson",
    "Mesa", "RC Goblin", "Hotring Racer A", "Hotring Racer B", "Bloodring Banger", "Rancher", "Super GT", "Elegant", "Journey", "Bike",
    "Mountain Bike", "Beagle", "Cropdust", "Stunt", "Tanker", "RoadTrain", "Nebula", "Majestic", "Buccaneer", "Shamal",
    "Hydra", "FCR-900", "NRG-500", "HPV1000", "Cement Truck", "Tow Truck", "Fortune", "Cadrona", "FBI Truck", "Willard",
    "Forklift", "Tractor", "Combine", "Feltzer", "Remington", "Slamvan", "Blade", "Freight", "Streak", "Vortex",
    "Vincent", "Bullet", "Clover", "Sadler", "Firetruck LA", "Hustler", "Intruder", "Primo", "Cargobob", "Tampa",
    "Sunrise", "Merit", "Utility", "Nevada", "Yosemite", "Windsor", "Monster A", "Monster B", "Uranus", "Jester",
    "Sultan", "Stratum", "Elegy", "Raindance", "RC Tiger", "Flash", "Tahoma", "Savanna", "Bandito", "Freight",
    "Trailer", "Kart", "Mower", "Duneride", "Sweeper", "Broadway", "Tornado", "AT-400", "DFT-30", "Huntley",
    "Stafford", "BF-400", "Newsvan", "Tug", "Trailer A", "Emperor", "Wayfarer", "Euros", "Hotdog", "Club",
    "Trailer B", "Trailer C", "Andromada", "Dodo", "RC Cam", "Launch", "Police Car (LSPD)", "Police Car (SFPD)", "Police Car (LVPD)", "Police Ranger",
    "Picador", "S.W.A.T. Van", "Alpha", "Phoenix", "Glendale", "Sadler", "Luggage Trailer A", "Luggage Trailer B", "Stair Trailer", "Boxville",
    "Farm Plow", "Utility Trailer"
};

// Crear concesionaria (admin nivel 3+)
CMD:createdealership(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "No tienes permisos para usar este comando.");
    
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
    new name[100], price;
    if(sscanf(params, "s[100]d", name, price))
        return SendClientMessage(playerid, COLOR_ERROR, "Uso: /createdealership [nombre] [precio]");
    
    if(price < 50000)
        return SendClientMessage(playerid, COLOR_ERROR, "El precio mínimo es $50,000");
    
    // Crear la propiedad
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO `properties` (`type_id`, `name`, `owner_type`, `owner_id`, `fiscal_price`, `pos_x`, `pos_y`, `pos_z`, `interior`) \
        VALUES (%d, '%e', 'state', NULL, %d, %.4f, %.4f, %.4f, 0)",
        DEALERSHIP, name, price, x, y, z);
    
    printf("[DEBUG] Creando concesionaria: %s", query);
    
    new Cache:result = mysql_query(g_MySQL, query);
    new insertid = cache_insert_id();
    cache_delete(result);
    
    if(insertid > 0)
    {
        // Crear entrada en businesses
        mysql_format(g_MySQL, query, sizeof(query),
            "INSERT INTO `businesses` (`property_id`, `service_type`) VALUES (%d, 'dealership')",
            insertid);
        mysql_query(g_MySQL, query);
        
        // Cargar la propiedad
        LoadProperty(insertid);
        
        new string[144];
        format(string, sizeof(string), 
            "{00FF00}Concesionaria creada! ID: %d - Usa /addvehiclestock [id] [modelo] [cantidad] [precio]", 
            insertid);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
    }
    else
    {
        SendClientMessage(playerid, COLOR_ERROR, "Error al crear la concesionaria.");
    }
    
    return 1;
}

// Agregar stock de vehículos (admin nivel 3+)
CMD:addvehiclestock(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "No tienes permisos para usar este comando.");
    
    new dealershipID, model, quantity, vehiclePrice;
    if(sscanf(params, "dddd", dealershipID, model, quantity, vehiclePrice))
        return SendClientMessage(playerid, COLOR_ERROR, "Uso: /addvehiclestock [dealership_id] [modelo] [cantidad] [precio]");
    
    if(model < 400 || model > 611)
        return SendClientMessage(playerid, COLOR_ERROR, "Modelo inválido (400-611)");
    
    if(quantity < 1 || quantity > 100)
        return SendClientMessage(playerid, COLOR_ERROR, "Cantidad inválida (1-100)");
    
    if(vehiclePrice < 1000)
        return SendClientMessage(playerid, COLOR_ERROR, "Precio mínimo: $1,000");
    
    // Verificar que sea una concesionaria
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT b.property_id FROM businesses b \
        INNER JOIN properties p ON b.property_id = p.id \
        WHERE b.property_id = %d AND b.service_type = 'dealership'",
        dealershipID);
    
    new Cache:result = mysql_query(g_MySQL, query);
    
    if(cache_num_rows() == 0)
    {
        cache_delete(result);
        return SendClientMessage(playerid, COLOR_ERROR, "Esa propiedad no es una concesionaria.");
    }
    cache_delete(result);
    
    // Agregar o actualizar stock
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO `dealership_inventory` (`property_id`, `vehicle_model`, `stock`, `sale_price`) \
        VALUES (%d, %d, %d, %d) ON DUPLICATE KEY UPDATE stock = stock + %d, sale_price = %d",
        dealershipID, model, quantity, vehiclePrice, quantity, vehiclePrice);
    
    mysql_query(g_MySQL, query);
    
    new string[144];
    format(string, sizeof(string), 
        "{00FF00}Stock agregado: %d unidades de %s ($%d c/u)", 
        quantity, VehicleModelNames[model - 400], vehiclePrice);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    return 1;
}

// Ver catálogo de la concesionaria
CMD:buycar(playerid, params[])
{
    // Buscar concesionaria cercana
    new propertySlot = GetNearestProperty(playerid);
    
    if(propertySlot == -1)
        return SendClientMessage(playerid, COLOR_ERROR, "No estás cerca de ninguna concesionaria.");
    
    // Verificar que sea una concesionaria
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT b.property_id FROM businesses b \
        WHERE b.property_id = %d AND b.service_type = 'dealership'",
        PropertyData[propertySlot][pID]);
    
    new Cache:result = mysql_query(g_MySQL, query);
    
    if(cache_num_rows() == 0)
    {
        cache_delete(result);
        return SendClientMessage(playerid, COLOR_ERROR, "Esta propiedad no es una concesionaria.");
    }
    cache_delete(result);
    
    // Obtener inventario
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT vehicle_model, stock, sale_price FROM dealership_inventory \
        WHERE property_id = %d AND stock > 0 ORDER BY sale_price",
        PropertyData[propertySlot][pID]);
    
    result = mysql_query(g_MySQL, query);
    new rows = cache_num_rows();
    
    if(rows == 0)
    {
        cache_delete(result);
        return SendClientMessage(playerid, COLOR_ERROR, "Esta concesionaria no tiene vehículos en stock.");
    }
    
    new dialogStr[2048], model, quantity, vehiclePrice;
    strcat(dialogStr, "Modelo\tStock\tPrecio\n");
    
    for(new i = 0; i < rows; i++)
    {
        cache_get_value_name_int(i, "vehicle_model", model);
        cache_get_value_name_int(i, "stock", quantity);
        cache_get_value_name_int(i, "sale_price", vehiclePrice);
        
        format(dialogStr, sizeof(dialogStr), "%s%s (%d)\t%d\t$%d\n",
            dialogStr, VehicleModelNames[model - 400], model, quantity, vehiclePrice);
    }
    
    cache_delete(result);
    
    SetPVarInt(playerid, "BuyingFromDealership", PropertyData[propertySlot][pID]);
    ShowPlayerDialog(playerid, DIALOG_DEALERSHIP_CATALOG, DIALOG_STYLE_TABLIST_HEADERS,
        "Catálogo de Vehículos", dialogStr, "Comprar", "Cancelar");
    
    return 1;
}

// Manejar compra de vehículo
stock OnDealershipPurchase(playerid, dialogid, response, listitem)
{
    if(dialogid != DIALOG_DEALERSHIP_CATALOG)
        return 0;
    
    if(!response)
    {
        DeletePVar(playerid, "BuyingFromDealership");
        return 1;
    }
    
    new dealershipID = GetPVarInt(playerid, "BuyingFromDealership");
    
    // Obtener vehículo seleccionado
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT vehicle_model, stock, sale_price FROM dealership_inventory \
        WHERE property_id = %d AND stock > 0 ORDER BY sale_price LIMIT %d, 1",
        dealershipID, listitem);
    
    new Cache:result = mysql_query(g_MySQL, query);
    
    if(cache_num_rows() == 0)
    {
        cache_delete(result);
        SendClientMessage(playerid, COLOR_ERROR, "Este vehículo ya no está disponible.");
        DeletePVar(playerid, "BuyingFromDealership");
        return 1;
    }
    
    new model, quantity, vehiclePrice;
    cache_get_value_name_int(0, "vehicle_model", model);
    cache_get_value_name_int(0, "stock", quantity);
    cache_get_value_name_int(0, "sale_price", vehiclePrice);
    cache_delete(result);
    
    // Verificar dinero
    if(CharacterData[playerid][cMoney] < vehiclePrice)
    {
        SendClientMessage(playerid, COLOR_ERROR, "No tienes suficiente dinero.");
        DeletePVar(playerid, "BuyingFromDealership");
        return 1;
    }
    
    // Generar placa única
    new plate[9];
    format(plate, sizeof(plate), "SA%d%d", random(9999), CharacterData[playerid][cID]);
    
    // Posición del vehículo
    new Float:x, Float:y, Float:z, Float:angle;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, angle);
    x += 3.0;
    
    // Crear vehículo en la BD
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO `vehicles` (`model`, `owner_type`, `owner_id`, `plate`, `pos_x`, `pos_y`, `pos_z`, `pos_a`, `fiscal_price`, `is_spawned`) \
        VALUES (%d, 'character', %d, '%e', %.4f, %.4f, %.4f, %.4f, %d, 1)",
        model, CharacterData[playerid][cID], plate, x, y, z, angle, vehiclePrice);
    
    result = mysql_query(g_MySQL, query);
    new vehicleID = cache_insert_id();
    cache_delete(result);
    
    if(vehicleID > 0)
    {
        // Reducir stock
        mysql_format(g_MySQL, query, sizeof(query),
            "UPDATE dealership_inventory SET stock = stock - 1 \
            WHERE property_id = %d AND vehicle_model = %d",
            dealershipID, model);
        mysql_query(g_MySQL, query);
        
        // Transferir dinero a la empresa
        mysql_format(g_MySQL, query, sizeof(query),
            "UPDATE properties SET bank_balance = bank_balance + %d WHERE id = %d",
            vehiclePrice, dealershipID);
        mysql_query(g_MySQL, query);
        
        // Quitar dinero al jugador
        CharacterData[playerid][cMoney] -= vehiclePrice;
        ResetPlayerMoney(playerid);
        GivePlayerMoney(playerid, CharacterData[playerid][cMoney]);
        
        // Registrar transacción
        mysql_format(g_MySQL, query, sizeof(query),
            "INSERT INTO `transactions` (`type`, `from_type`, `from_id`, `to_type`, `to_id`, `amount`, `description`) \
            VALUES ('vehicle_purchase', 'character', %d, 'property', %d, %d, 'Compra de %s (Placa: %s)')",
            CharacterData[playerid][cID], dealershipID, vehiclePrice, VehicleModelNames[model - 400], plate);
        mysql_query(g_MySQL, query);
        
        // Spawnear vehículo y guardar ID en memoria
        new ingameVehicleID = CreateVehicle(model, x, y, z, angle, -1, -1, 300000);
        SetVehicleDBID(ingameVehicleID, vehicleID);
        
        printf("[Dealership] Vehículo spawneado - ingame ID: %d, DB ID: %d", ingameVehicleID, vehicleID);
        
        // Dar 2 llaves del vehículo: 1 maestra + 1 normal
        new metadata[128];
        format(metadata, sizeof(metadata), "vehicleid:%d|plate:%s", vehicleID, plate);
        
        // Llave maestra
        new keyMasterResult = GivePlayerItem(playerid, ITEM_VEHICLE_KEY_MASTER, 1, metadata);
        
        // Llave normal
        new keyNormalResult = GivePlayerItem(playerid, ITEM_VEHICLE_KEY, 1, metadata);
        
        if(keyMasterResult && keyNormalResult)
        {
            printf("[Dealership] 2 llaves dadas correctamente a %s (placa: %s)", 
                CharacterData[playerid][cName], plate);
            
            // Refrescar inventario para mostrar las llaves inmediatamente
            ShowInventoryDialog(playerid);
        }
        else
        {
            printf("[Dealership] ERROR: Llaves no entregadas - Master: %d, Normal: %d", 
                keyMasterResult, keyNormalResult);
            SendClientMessage(playerid, COLOR_ERROR, "{FF0000}ADVERTENCIA: {FFFFFF}No se pudieron dar las llaves. Contacta a un admin.");
        }
        
        new string[180];
        format(string, sizeof(string), 
            "{00FF00}Has comprado un %s por $%d (Placa: %s)\n{FFFF00}Has recibido 1 llave maestra y 1 llave normal.", 
            VehicleModelNames[model - 400], vehiclePrice, plate);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        
        printf("[Dealership] %s compró %s (ID: %d) por $%d", 
            CharacterData[playerid][cName], VehicleModelNames[model - 400], vehicleID, vehiclePrice);
    }
    else
    {
        SendClientMessage(playerid, COLOR_ERROR, "Error al procesar la compra.");
    }
    
    DeletePVar(playerid, "BuyingFromDealership");
    return 1;
}
