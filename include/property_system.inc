/*
    Sistema de Propiedades - Casas, Empresas, Tiendas
*/

#if defined _property_system_included
    #endinput
#endif
#define _property_system_included

#define MAX_PROPERTIES 500

// Dialogs
#define DIALOG_HOUSE_TYPE       2000
#define DIALOG_HOUSE_INTERIOR   2001
#define DIALOG_HOUSE_NAME       2002

// Enumeraciones
enum E_PROPERTY_DATA
{
    pID,
    pTypeID,
    pName[100],
    pOwnerID,
    pFiscalPrice,
    pMarketPrice,
    Float:pPosX,
    Float:pPosY,
    Float:pPosZ,
    pInterior,
    pInteriorID,  // ID del interior específico (0-2)
    pVirtualWorld,
    bool:pLocked,
    pBankBalance,
    pPickup,
    Text3D:pLabel,
    bool:pExists
}

new PropertyData[MAX_PROPERTIES][E_PROPERTY_DATA];
new PropertyCount = 0;

// Variable para trackear jugadores dentro de propiedades
new PlayerInProperty[MAX_PLAYERS] = {-1, ...};

// Interiores disponibles por tipo de casa
enum E_INTERIOR_INFO
{
    iInteriorID,
    iName[64],
    Float:iSpawnX,
    Float:iSpawnY,
    Float:iSpawnZ,
    Float:iSpawnA
}

// Casa Pequeña - 3 opciones
new const SmallHouseInteriors[][E_INTERIOR_INFO] = {
    {2, "Departamento moderno", 2196.85, -1204.36, 1049.02, 270.0},
    {1, "Casa suburbana", 223.11, 1287.08, 1082.14, 0.0},
    {5, "Apartamento compacto", 2233.80, -1115.55, 1050.88, 0.0}
};

// Casa Mediana - 3 opciones
new const MediumHouseInteriors[][E_INTERIOR_INFO] = {
    {5, "Casa familiar", 2237.53, -1081.31, 1049.02, 0.0},
    {6, "Residencia moderna", 2324.41, -1149.54, 1050.71, 180.0},
    {15, "Casa de dos pisos", 295.14, 1472.23, 1080.26, 0.0}
};

// Casa Grande - 3 opciones
new const LargeHouseInteriors[][E_INTERIOR_INFO] = {
    {10, "Mansión urbana", 2260.38, -1210.45, 1049.02, 90.0},
    {8, "Villa de lujo", 2365.42, -1131.85, 1050.88, 270.0},
    {7, "Casa de playa", 225.68, 1240.00, 1082.14, 0.0}
};

// Mansión - 3 opciones  
new const MansionInteriors[][E_INTERIOR_INFO] = {
    {5, "Mansión Madd Dogg", 1299.14, -794.77, 1084.01, 270.0},
    {3, "Mansión clásica", 2496.65, -1692.08, 1014.74, 180.0},
    {6, "Mansión presidencial", 2807.63, -1174.38, 1025.57, 0.0}
};

// Tipos de propiedad (sincronizar con database_economy.sql)
#define HOUSE_SMALL 1        // Casa Pequeña
#define HOUSE_MEDIUM 2       // Casa Mediana
#define HOUSE_LARGE 3        // Casa Grande
#define MANSION 4            // Mansión
#define MECHANIC 5           // Taller Mecánico
#define DEALERSHIP 6         // Concesionaria
#define DRIVING_SCHOOL 7     // Escuela de Conducción
#define TAXI_COMPANY 8       // Empresa de Taxis
#define TRUCKING_COMPANY 9   // Empresa de Camiones
#define STORE_247 10         // 24/7 Market
#define AMMUNATION 11        // Ammunation
#define CLOTHING_STORE 12    // Tienda de Ropa
#define HARDWARE_STORE 13    // Ferretería

// Crear casa (solo admins nivel 3+)
CMD:createhouse(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "Necesitas ser Administrador (nivel 3) para usar este comando.");
    
    ShowPlayerDialog(playerid, DIALOG_HOUSE_TYPE, DIALOG_STYLE_LIST, 
        "Crear Casa - Tipo",
        "Casa Pequeña ($50,000)\n\
        Casa Mediana ($150,000)\n\
        Casa Grande ($300,000)\n\
        Mansión ($1,000,000)",
        "Siguiente", "Cancelar");
    
    return 1;
}

// Callback para dialogs
forward OnPropertyDialogResponse(playerid, dialogid, response, listitem, inputtext[]);
public OnPropertyDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    switch(dialogid)
    {
        case DIALOG_HOUSE_TYPE:
        {
            if(!response) return 1;
            
            new typeID, fiscalPrice, typeName[50];
            
            switch(listitem)
            {
                case 0: { typeID = HOUSE_SMALL; fiscalPrice = 50000; typeName = "Casa Pequeña"; }
                case 1: { typeID = HOUSE_MEDIUM; fiscalPrice = 150000; typeName = "Casa Mediana"; }
                case 2: { typeID = HOUSE_LARGE; fiscalPrice = 300000; typeName = "Casa Grande"; }
                case 3: { typeID = MANSION; fiscalPrice = 1000000; typeName = "Mansión"; }
            }
            
            // Guardar temporalmente en PVar
            SetPVarInt(playerid, "TempHouseType", typeID);
            SetPVarInt(playerid, "TempHouseFiscal", fiscalPrice);
            
            // Mostrar opciones de interiores
            new dialogText[512];
            switch(typeID)
            {
                case HOUSE_SMALL:
                {
                    format(dialogText, sizeof(dialogText),
                        "%s\n%s\n%s",
                        SmallHouseInteriors[0][iName],
                        SmallHouseInteriors[1][iName],
                        SmallHouseInteriors[2][iName]);
                }
                case HOUSE_MEDIUM:
                {
                    format(dialogText, sizeof(dialogText),
                        "%s\n%s\n%s",
                        MediumHouseInteriors[0][iName],
                        MediumHouseInteriors[1][iName],
                        MediumHouseInteriors[2][iName]);
                }
                case HOUSE_LARGE:
                {
                    format(dialogText, sizeof(dialogText),
                        "%s\n%s\n%s",
                        LargeHouseInteriors[0][iName],
                        LargeHouseInteriors[1][iName],
                        LargeHouseInteriors[2][iName]);
                }
                case MANSION:
                {
                    format(dialogText, sizeof(dialogText),
                        "%s\n%s\n%s",
                        MansionInteriors[0][iName],
                        MansionInteriors[1][iName],
                        MansionInteriors[2][iName]);
                }
            }
            
            ShowPlayerDialog(playerid, DIALOG_HOUSE_INTERIOR, DIALOG_STYLE_LIST,
                "Crear Casa - Elige el Interior", dialogText, "Siguiente", "Atrás");
        }
        
        case DIALOG_HOUSE_INTERIOR:
        {
            if(!response)
            {
                // Volver al menú de tipos
                ShowPlayerDialog(playerid, DIALOG_HOUSE_TYPE, DIALOG_STYLE_LIST, 
                    "Crear Casa - Tipo",
                    "Casa Pequeña ($50,000)\n\
                    Casa Mediana ($150,000)\n\
                    Casa Grande ($300,000)\n\
                    Mansión ($1,000,000)",
                    "Siguiente", "Cancelar");
                return 1;
            }
            
            new typeID = GetPVarInt(playerid, "TempHouseType");
            new fiscalPrice = GetPVarInt(playerid, "TempHouseFiscal");
            
            // Guardar el interior elegido
            SetPVarInt(playerid, "TempHouseInteriorSlot", listitem);
            
            new dialogText[256], interiorName[64];
            
            // Obtener nombre del interior
            switch(typeID)
            {
                case HOUSE_SMALL: format(interiorName, sizeof(interiorName), "%s", SmallHouseInteriors[listitem][iName]);
                case HOUSE_MEDIUM: format(interiorName, sizeof(interiorName), "%s", MediumHouseInteriors[listitem][iName]);
                case HOUSE_LARGE: format(interiorName, sizeof(interiorName), "%s", LargeHouseInteriors[listitem][iName]);
                case MANSION: format(interiorName, sizeof(interiorName), "%s", MansionInteriors[listitem][iName]);
            }
            
            format(dialogText, sizeof(dialogText), 
                "Interior: %s\n\
                Precio fiscal: $%s\n\n\
                Ingresa el nombre de la propiedad:",
                interiorName, FormatNumber(fiscalPrice));
            
            ShowPlayerDialog(playerid, DIALOG_HOUSE_NAME, DIALOG_STYLE_INPUT,
                "Crear Casa - Nombre", dialogText, "Crear", "Atrás");
        }
        
        case DIALOG_HOUSE_NAME:
        {
            if(!response)
            {
                DeletePVar(playerid, "TempHouseType");
                DeletePVar(playerid, "TempHouseFiscal");
                DeletePVar(playerid, "TempHouseInteriorSlot");
                return 1;
            }
            
            if(strlen(inputtext) < 3 || strlen(inputtext) > 50)
            {
                SendClientMessage(playerid, COLOR_ERROR, "El nombre debe tener entre 3 y 50 caracteres.");
                return 1;
            }
            
            new typeID = GetPVarInt(playerid, "TempHouseType");
            new fiscalPrice = GetPVarInt(playerid, "TempHouseFiscal");
            new interiorSlot = GetPVarInt(playerid, "TempHouseInteriorSlot");
            new Float:x, Float:y, Float:z;
            GetPlayerPos(playerid, x, y, z);
            
            new interiorWorld;
            switch(typeID)
            {
                case HOUSE_SMALL: interiorWorld = SmallHouseInteriors[interiorSlot][iInteriorID];
                case HOUSE_MEDIUM: interiorWorld = MediumHouseInteriors[interiorSlot][iInteriorID];
                case HOUSE_LARGE: interiorWorld = LargeHouseInteriors[interiorSlot][iInteriorID];
                case MANSION: interiorWorld = MansionInteriors[interiorSlot][iInteriorID];
            }
            
            // Crear en la base de datos
            new query[512];
            mysql_format(g_MySQL, query, sizeof(query),
                "INSERT INTO `properties` (`type_id`, `name`, `owner_id`, `fiscal_price`, `market_price`, `pos_x`, `pos_y`, `pos_z`, `interior`, `interior_id`) \
                VALUES (%d, '%e', NULL, %d, NULL, %.4f, %.4f, %.4f, %d, %d)",
                typeID, inputtext, fiscalPrice, x, y, z, interiorWorld, interiorSlot);
            
            printf("[DEBUG] Query INSERT casa: %s", query);
            printf("[DEBUG] MySQL Handle: %d", _:g_MySQL);
            printf("[DEBUG] MySQL errno antes del INSERT: %d", mysql_errno(g_MySQL));
            printf("[DEBUG] Ejecutando mysql_tquery para OnHouseCreated...");
            
            // Ejecutar con threading (debe ser threaded para usar callback)
            new Cache:result = mysql_query(g_MySQL, query);
            new insertid = cache_insert_id();
            cache_delete(result);
            
            printf("[DEBUG] Query ejecutada. Insert ID: %d", insertid);
            printf("[DEBUG] MySQL errno después del INSERT: %d", mysql_errno(g_MySQL));
            
            if(insertid > 0)
            {
                // Llamar manualmente la función de carga
                LoadProperty(insertid);
                
                new string[144];
                format(string, sizeof(string), 
                    "{00FF00}Casa creada exitosamente! ID: %d, Precio: $%d", 
                    insertid, fiscalPrice);
                SendClientMessage(playerid, COLOR_SUCCESS, string);
            }
            else
            {
                SendClientMessage(playerid, COLOR_ERROR, "Error al crear la casa. Verifica la consola.");
                printf("[DEBUG] ERROR: No se pudo insertar la casa. errno: %d", mysql_errno(g_MySQL));
            }
            
            DeletePVar(playerid, "TempHouseType");
            DeletePVar(playerid, "TempHouseFiscal");
            DeletePVar(playerid, "TempHouseInteriorSlot");
            
            new string[128];
            format(string, sizeof(string), "{FF6347}[ADMIN]{FFFFFF} Creando casa '%s'...", inputtext);
            SendClientMessage(playerid, COLOR_INFO, string);
        }
    }
    return 1;
}

forward OnHouseCreated(playerid, typeID, fiscalPrice, Float:x, Float:y, Float:z);
public OnHouseCreated(playerid, typeID, fiscalPrice, Float:x, Float:y, Float:z)
{
    printf("[DEBUG] OnHouseCreated llamada para playerid: %d", playerid);
    
    if(!IsPlayerConnected(playerid))
    {
        printf("[DEBUG] ERROR: Jugador no conectado!");
        return 0;
    }
    
    new propertyID = cache_insert_id();
    printf("[DEBUG] cache_insert_id() = %d", propertyID);
    
    if(propertyID == 0)
    {
        printf("[DEBUG] ERROR: INSERT falló, insert_id = 0");
        printf("[DEBUG] MySQL errno: %d", mysql_errno(g_MySQL));
        SendClientMessage(playerid, COLOR_ERROR, "Error al crear la casa en la base de datos.");
        return 0;
    }
    
    printf("[DEBUG] Casa insertada con ID: %d, cargando en memoria...", propertyID);
    
    // Cargar la propiedad en memoria
    LoadProperty(propertyID);
    
    new string[144];
    format(string, sizeof(string), 
        "{00FF00}Casa creada exitosamente! ID: %d, Precio: $%d", 
        propertyID, fiscalPrice);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    printf("[DEBUG] Casa creada exitosamente (ID: %d)", propertyID);
    
    return 1;
}

// Cargar propiedad específica
stock LoadProperty(propertyID)
{
    printf("[DEBUG] LoadProperty llamada para ID: %d", propertyID);
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT * FROM `properties` WHERE `id` = %d LIMIT 1", propertyID);
    printf("[DEBUG] Query SELECT: %s", query);
    mysql_tquery(g_MySQL, query, "OnPropertyLoad", "d", propertyID);
}

forward OnPropertyLoad(propertyID);
public OnPropertyLoad(propertyID)
{
    printf("[DEBUG] OnPropertyLoad ejecutada para ID: %d", propertyID);
    printf("[DEBUG] cache_num_rows() = %d", cache_num_rows());
    
    if(!cache_num_rows())
    {
        printf("[DEBUG] ERROR: No se encontró la propiedad ID %d en la BD!", propertyID);
        return 0;
    }
    
    // Buscar slot libre
    new slot = -1;
    for(new i = 0; i < MAX_PROPERTIES; i++)
    {
        if(!PropertyData[i][pExists])
        {
            slot = i;
            break;
        }
    }
    
    if(slot == -1)
    {
        print("[ERROR] No hay slots disponibles para propiedades!");
        return 0;
    }
    
    PropertyData[slot][pID] = propertyID;
    cache_get_value_name_int(0, "type_id", PropertyData[slot][pTypeID]);
    cache_get_value_name(0, "name", PropertyData[slot][pName], 100);
    cache_get_value_name_int(0, "owner_id", PropertyData[slot][pOwnerID]);
    cache_get_value_name_int(0, "fiscal_price", PropertyData[slot][pFiscalPrice]);
    cache_get_value_name_int(0, "market_price", PropertyData[slot][pMarketPrice]);
    cache_get_value_name_float(0, "pos_x", PropertyData[slot][pPosX]);
    cache_get_value_name_float(0, "pos_y", PropertyData[slot][pPosY]);
    cache_get_value_name_float(0, "pos_z", PropertyData[slot][pPosZ]);
    
    new interiorWorld;
    cache_get_value_name_int(0, "interior", interiorWorld);
    PropertyData[slot][pInterior] = interiorWorld;
    cache_get_value_name_int(0, "interior_id", PropertyData[slot][pInteriorID]);
    cache_get_value_name_int(0, "virtual_world", PropertyData[slot][pVirtualWorld]);
    
    new locked;
    cache_get_value_name_int(0, "is_locked", locked);
    PropertyData[slot][pLocked] = (locked == 1);
    
    cache_get_value_name_int(0, "bank_balance", PropertyData[slot][pBankBalance]);
    PropertyData[slot][pExists] = true;
    
    // Crear pickup y label
    PropertyData[slot][pPickup] = CreateDynamicPickup(1273, 1, 
        PropertyData[slot][pPosX], PropertyData[slot][pPosY], PropertyData[slot][pPosZ]);
    
    new labelText[256];
    if(PropertyData[slot][pOwnerID] == 0) // Estado
    {
        format(labelText, sizeof(labelText),
            "{FFFFFF}%s\n\
            {FFD700}Precio: {00FF00}$%s\n\
            {FFFFFF}Presiona {FFD700}Y {FFFFFF}para más información",
            PropertyData[slot][pName],
            FormatNumber(PropertyData[slot][pFiscalPrice]));
    }
    else // Tiene dueño
    {
        if(PropertyData[slot][pMarketPrice] > 0)
        {
            format(labelText, sizeof(labelText),
                "{FFFFFF}%s\n\
                {FF0000}EN VENTA\n\
                {FFD700}Precio: {00FF00}$%s\n\
                {FFFFFF}Presiona {FFD700}Y {FFFFFF}para comprar",
                PropertyData[slot][pName],
                FormatNumber(PropertyData[slot][pMarketPrice]));
        }
        else
        {
            format(labelText, sizeof(labelText),
                "{FFFFFF}%s\n\
                {AAAAAA}Propiedad Privada",
                PropertyData[slot][pName]);
        }
    }
    
    PropertyData[slot][pLabel] = CreateDynamic3DTextLabel(labelText, 0xFFFFFFFF,
        PropertyData[slot][pPosX], PropertyData[slot][pPosY], PropertyData[slot][pPosZ] + 0.5,
        15.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0);
    
    PropertyCount++;
    
    printf("[Properties] Casa cargada: %s (ID: %d) en slot %d", PropertyData[slot][pName], propertyID, slot);
    printf("[DEBUG] Pickup ID: %d, Label ID: %d", PropertyData[slot][pPickup], _:PropertyData[slot][pLabel]);
    printf("[DEBUG] Posicion: %.2f, %.2f, %.2f", PropertyData[slot][pPosX], PropertyData[slot][pPosY], PropertyData[slot][pPosZ]);
    return 1;
}

// Cargar todas las propiedades al iniciar
stock LoadAllProperties()
{
    mysql_tquery(g_MySQL, "SELECT `id` FROM `properties`", "OnLoadAllProperties");
}

forward OnLoadAllProperties();
public OnLoadAllProperties()
{
    new rows = cache_num_rows();
    
    if(!rows)
    {
        print("[Properties] No hay propiedades en la base de datos.");
        return 1;
    }
    
    for(new i = 0; i < rows; i++)
    {
        new propertyID;
        cache_get_value_name_int(i, "id", propertyID);
        LoadProperty(propertyID);
    }
    
    printf("[Properties] %d propiedades cargadas.", PropertyCount);
    return 1;
}

// Formatear números (simplificado)
stock FormatNumber(number)
{
    new string[32];
    format(string, sizeof(string), "%d", number);
    return string;
}

// ============================================================================
// COMANDOS DE JUGADORES - PROPIEDADES
// ============================================================================

// Entrar a una propiedad
CMD:enter(playerid, params[])
{
    new propertySlot = GetNearestProperty(playerid);
    
    if(propertySlot == -1)
        return SendClientMessage(playerid, COLOR_ERROR, "No estás cerca de ninguna propiedad.");
    
    // Verificar si está cerrada
    if(PropertyData[propertySlot][pLocked])
    {
        // Si es dueño puede entrar
        if(PropertyData[propertySlot][pOwnerID] != CharacterData[playerid][cID] && PropertyData[propertySlot][pOwnerID] != 0)
            return SendClientMessage(playerid, COLOR_ERROR, "Esta propiedad está cerrada.");
    }
    
    // Obtener coordenadas del interior según tipo y slot
    new Float:x, Float:y, Float:z, Float:a;
    new interiorID;
    
    switch(PropertyData[propertySlot][pTypeID])
    {
        case HOUSE_SMALL:
        {
            new idx = PropertyData[propertySlot][pInteriorID];
            x = SmallHouseInteriors[idx][iSpawnX];
            y = SmallHouseInteriors[idx][iSpawnY];
            z = SmallHouseInteriors[idx][iSpawnZ];
            a = SmallHouseInteriors[idx][iSpawnA];
            interiorID = SmallHouseInteriors[idx][iInteriorID];
        }
        case HOUSE_MEDIUM:
        {
            new idx = PropertyData[propertySlot][pInteriorID];
            x = MediumHouseInteriors[idx][iSpawnX];
            y = MediumHouseInteriors[idx][iSpawnY];
            z = MediumHouseInteriors[idx][iSpawnZ];
            a = MediumHouseInteriors[idx][iSpawnA];
            interiorID = MediumHouseInteriors[idx][iInteriorID];
        }
        case HOUSE_LARGE:
        {
            new idx = PropertyData[propertySlot][pInteriorID];
            x = LargeHouseInteriors[idx][iSpawnX];
            y = LargeHouseInteriors[idx][iSpawnY];
            z = LargeHouseInteriors[idx][iSpawnZ];
            a = LargeHouseInteriors[idx][iSpawnA];
            interiorID = LargeHouseInteriors[idx][iInteriorID];
        }
        case MANSION:
        {
            new idx = PropertyData[propertySlot][pInteriorID];
            x = MansionInteriors[idx][iSpawnX];
            y = MansionInteriors[idx][iSpawnY];
            z = MansionInteriors[idx][iSpawnZ];
            a = MansionInteriors[idx][iSpawnA];
            interiorID = MansionInteriors[idx][iInteriorID];
        }
        default:
        {
            return SendClientMessage(playerid, COLOR_ERROR, "Esta propiedad aún no tiene interior configurado.");
        }
    }
    
    // Teleportar al jugador
    SetPlayerInterior(playerid, interiorID);
    SetPlayerVirtualWorld(playerid, PropertyData[propertySlot][pID] + 1000); // VW único por propiedad
    SetPlayerPos(playerid, x, y, z);
    SetPlayerFacingAngle(playerid, a);
    
    PlayerInProperty[playerid] = propertySlot;
    
    new string[128];
    format(string, sizeof(string), "Entraste a: {FFD700}%s", PropertyData[propertySlot][pName]);
    SendClientMessage(playerid, COLOR_INFO, string);
    SendClientMessage(playerid, COLOR_GREY, "Usa {FFFFFF}/exitproperty {AAAAAA}para salir.");
    
    return 1;
}

// Salir de una propiedad
CMD:exitproperty(playerid, params[])
{
    if(PlayerInProperty[playerid] == -1)
        return SendClientMessage(playerid, COLOR_ERROR, "No estás dentro de una propiedad.");
    
    new propertySlot = PlayerInProperty[playerid];
    
    // Teleportar afuera
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);
    SetPlayerPos(playerid, 
        PropertyData[propertySlot][pPosX], 
        PropertyData[propertySlot][pPosY], 
        PropertyData[propertySlot][pPosZ]);
    
    PlayerInProperty[playerid] = -1;
    
    SendClientMessage(playerid, COLOR_INFO, "Saliste de la propiedad.");
    return 1;
}

// Comprar propiedad
CMD:buyproperty(playerid, params[])
{
    new propertySlot = GetNearestProperty(playerid);
    
    if(propertySlot == -1)
        return SendClientMessage(playerid, COLOR_ERROR, "No estás cerca de ninguna propiedad.");
    
    // Verificar si es del Estado
    if(PropertyData[propertySlot][pOwnerID] != 0)
    {
        // Verificar si está en venta
        if(PropertyData[propertySlot][pMarketPrice] == 0)
            return SendClientMessage(playerid, COLOR_ERROR, "Esta propiedad no está en venta.");
        
        // Compra entre jugadores
        new price = PropertyData[propertySlot][pMarketPrice];
        
        if(CharacterData[playerid][cMoney] < price)
            return SendClientMessage(playerid, COLOR_ERROR, "No tienes suficiente dinero.");
        
        // Transferir dinero
        CharacterData[playerid][cMoney] -= price;
        
        // Actualizar dueño en base de datos
        new query[256];
        mysql_format(g_MySQL, query, sizeof(query),
            "UPDATE `properties` SET `owner_id` = %d, `market_price` = NULL WHERE `id` = %d",
            CharacterData[playerid][cID], PropertyData[propertySlot][pID]);
        mysql_tquery(g_MySQL, query);
        
        // Registrar transacción
        mysql_format(g_MySQL, query, sizeof(query),
            "INSERT INTO `transactions` (`type`, `from_type`, `from_id`, `to_type`, `to_id`, `amount`, `description`) \
            VALUES ('property_purchase', 'character', %d, 'property', %d, %d, 'Compra de propiedad')",
            CharacterData[playerid][cID], PropertyData[propertySlot][pID], price);
        mysql_tquery(g_MySQL, query);
        
        PropertyData[propertySlot][pOwnerID] = CharacterData[playerid][cID];
        PropertyData[propertySlot][pMarketPrice] = 0;
        
        UpdatePropertyLabel(propertySlot);
        
        new string[144];
        format(string, sizeof(string), 
            "{00FF00}¡Felicidades! Compraste {FFFFFF}%s {00FF00}por {FFFFFF}$%s", 
            PropertyData[propertySlot][pName], FormatNumber(price));
        SendClientMessage(playerid, COLOR_SUCCESS, string);
    }
    else // Compra del Estado
    {
        new price = PropertyData[propertySlot][pFiscalPrice];
        
        if(CharacterData[playerid][cMoney] < price)
            return SendClientMessage(playerid, COLOR_ERROR, "No tienes suficiente dinero.");
        
        // Verificar cuántas propiedades tiene
        new ownedProperties = CountPlayerProperties(playerid);
        
        if(ownedProperties >= 3) // Límite de 3 propiedades
            return SendClientMessage(playerid, COLOR_ERROR, "Ya tienes el máximo de propiedades permitidas (3).");
        
        CharacterData[playerid][cMoney] -= price;
        
        new query[256];
        mysql_format(g_MySQL, query, sizeof(query),
            "UPDATE `properties` SET `owner_id` = %d WHERE `id` = %d",
            CharacterData[playerid][cID], PropertyData[propertySlot][pID]);
        mysql_tquery(g_MySQL, query);
        
        // Registrar transacción
        mysql_format(g_MySQL, query, sizeof(query),
            "INSERT INTO `transactions` (`type`, `from_type`, `from_id`, `to_type`, `to_id`, `amount`, `description`) \
            VALUES ('property_purchase', 'character', %d, 'system', NULL, %d, 'Compra de propiedad al Estado')",
            CharacterData[playerid][cID], price);
        mysql_tquery(g_MySQL, query);
        
        PropertyData[propertySlot][pOwnerID] = CharacterData[playerid][cID];
        
        UpdatePropertyLabel(propertySlot);
        
        new string[144];
        format(string, sizeof(string), 
            "{00FF00}¡Felicidades! Compraste {FFFFFF}%s {00FF00}por {FFFFFF}$%s", 
            PropertyData[propertySlot][pName], FormatNumber(price));
        SendClientMessage(playerid, COLOR_SUCCESS, string);
    }
    
    return 1;
}

// Vender propiedad
CMD:sellproperty(playerid, params[])
{
    new price;
    if(sscanf(params, "d", price))
        return SendClientMessage(playerid, COLOR_GREY, "Uso: /sellproperty [precio] (0 para cancelar venta)");
    
    new propertySlot = GetNearestProperty(playerid);
    
    if(propertySlot == -1)
        return SendClientMessage(playerid, COLOR_ERROR, "No estás cerca de ninguna propiedad.");
    
    if(PropertyData[propertySlot][pOwnerID] != CharacterData[playerid][cID])
        return SendClientMessage(playerid, COLOR_ERROR, "No eres dueño de esta propiedad.");
    
    if(price < 0)
        return SendClientMessage(playerid, COLOR_ERROR, "El precio debe ser positivo.");
    
    new query[256];
    if(price == 0)
    {
        mysql_format(g_MySQL, query, sizeof(query),
            "UPDATE `properties` SET `market_price` = NULL WHERE `id` = %d",
            PropertyData[propertySlot][pID]);
    }
    else
    {
        mysql_format(g_MySQL, query, sizeof(query),
            "UPDATE `properties` SET `market_price` = %d WHERE `id` = %d",
            price, PropertyData[propertySlot][pID]);
    }
    mysql_tquery(g_MySQL, query);
    
    PropertyData[propertySlot][pMarketPrice] = price;
    UpdatePropertyLabel(propertySlot);
    
    if(price == 0)
        SendClientMessage(playerid, COLOR_INFO, "Propiedad retirada de la venta.");
    else
    {
        new string[128];
        format(string, sizeof(string), 
            "{00FF00}Propiedad puesta en venta por {FFFFFF}$%s", FormatNumber(price));
        SendClientMessage(playerid, COLOR_SUCCESS, string);
    }
    
    return 1;
}

// Lock/Unlock propiedad
CMD:lock(playerid, params[])
{
    new propertySlot = GetNearestProperty(playerid);
    
    if(propertySlot == -1)
        return SendClientMessage(playerid, COLOR_ERROR, "No estás cerca de ninguna propiedad.");
    
    if(PropertyData[propertySlot][pOwnerID] != CharacterData[playerid][cID])
        return SendClientMessage(playerid, COLOR_ERROR, "No eres dueño de esta propiedad.");
    
    PropertyData[propertySlot][pLocked] = !PropertyData[propertySlot][pLocked];
    
    new query[128];
    mysql_format(g_MySQL, query, sizeof(query),
        "UPDATE `properties` SET `is_locked` = %d WHERE `id` = %d",
        PropertyData[propertySlot][pLocked], PropertyData[propertySlot][pID]);
    mysql_tquery(g_MySQL, query);
    
    new string[64];
    format(string, sizeof(string), "Propiedad %s.", PropertyData[propertySlot][pLocked] ? ("{FF0000}cerrada") : ("{00FF00}abierta"));
    SendClientMessage(playerid, COLOR_INFO, string);
    
    return 1;
}

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

// Obtener propiedad más cercana
stock GetNearestProperty(playerid)
{
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
    for(new i = 0; i < MAX_PROPERTIES; i++)
    {
        if(!PropertyData[i][pExists]) continue;
        
        if(IsPlayerInRangeOfPoint(playerid, 3.0, 
            PropertyData[i][pPosX], PropertyData[i][pPosY], PropertyData[i][pPosZ]))
        {
            return i;
        }
    }
    
    return -1;
}

// Contar propiedades del jugador
stock CountPlayerProperties(playerid)
{
    new count = 0;
    new characterID = CharacterData[playerid][cID];
    
    for(new i = 0; i < MAX_PROPERTIES; i++)
    {
        if(!PropertyData[i][pExists]) continue;
        if(PropertyData[i][pOwnerID] == characterID)
            count++;
    }
    
    return count;
}

// Actualizar label de propiedad
stock UpdatePropertyLabel(slot)
{
    if(IsValidDynamic3DTextLabel(PropertyData[slot][pLabel]))
        DestroyDynamic3DTextLabel(PropertyData[slot][pLabel]);
    
    new labelText[256];
    if(PropertyData[slot][pOwnerID] == 0) // Estado
    {
        format(labelText, sizeof(labelText),
            "{FFFFFF}%s\n\
            {FFD700}Precio: {00FF00}$%s\n\
            {FFFFFF}Usa {FFD700}/buyproperty {FFFFFF}para comprar",
            PropertyData[slot][pName],
            FormatNumber(PropertyData[slot][pFiscalPrice]));
    }
    else // Tiene dueño
    {
        if(PropertyData[slot][pMarketPrice] > 0)
        {
            format(labelText, sizeof(labelText),
                "{FFFFFF}%s\n\
                {FF0000}EN VENTA\n\
                {FFD700}Precio: {00FF00}$%s\n\
                {FFFFFF}Usa {FFD700}/buyproperty {FFFFFF}para comprar",
                PropertyData[slot][pName],
                FormatNumber(PropertyData[slot][pMarketPrice]));
        }
        else
        {
            format(labelText, sizeof(labelText),
                "{FFFFFF}%s\n\
                {AAAAAA}Propiedad Privada\n\
                {FFFFFF}Presiona {FFD700}H {FFFFFF}para entrar",
                PropertyData[slot][pName]);
        }
    }
    
    PropertyData[slot][pLabel] = CreateDynamic3DTextLabel(labelText, 0xFFFFFFFF,
        PropertyData[slot][pPosX], PropertyData[slot][pPosY], PropertyData[slot][pPosZ] + 0.5,
        15.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0);
}

// Función para entrar a una propiedad
stock EnterProperty(playerid, propertySlot)
{
    if(propertySlot == -1) return 0;
    
    // Verificar si está cerrada y el jugador no es dueño
    if(PropertyData[propertySlot][pLocked])
    {
        new characterID = CharacterData[playerid][cID];
        if(PropertyData[propertySlot][pOwnerID] != characterID)
        {
            SendClientMessage(playerid, COLOR_ERROR, "{FF0000}[ERROR]{FFFFFF} Esta propiedad está cerrada.");
            return 0;
        }
    }
    
    // Obtener coordenadas del interior según el tipo
    new Float:x, Float:y, Float:z, Float:a, interior;
    new typeID = PropertyData[propertySlot][pTypeID];
    new interiorChoice = PropertyData[propertySlot][pInteriorID];
    
    switch(typeID)
    {
        case HOUSE_SMALL:
        {
            x = SmallHouseInteriors[interiorChoice][iSpawnX];
            y = SmallHouseInteriors[interiorChoice][iSpawnY];
            z = SmallHouseInteriors[interiorChoice][iSpawnZ];
            a = SmallHouseInteriors[interiorChoice][iSpawnA];
            interior = SmallHouseInteriors[interiorChoice][iInteriorID];
        }
        case HOUSE_MEDIUM:
        {
            x = MediumHouseInteriors[interiorChoice][iSpawnX];
            y = MediumHouseInteriors[interiorChoice][iSpawnY];
            z = MediumHouseInteriors[interiorChoice][iSpawnZ];
            a = MediumHouseInteriors[interiorChoice][iSpawnA];
            interior = MediumHouseInteriors[interiorChoice][iInteriorID];
        }
        case HOUSE_LARGE:
        {
            x = LargeHouseInteriors[interiorChoice][iSpawnX];
            y = LargeHouseInteriors[interiorChoice][iSpawnY];
            z = LargeHouseInteriors[interiorChoice][iSpawnZ];
            a = LargeHouseInteriors[interiorChoice][iSpawnA];
            interior = LargeHouseInteriors[interiorChoice][iInteriorID];
        }
        case MANSION:
        {
            x = MansionInteriors[interiorChoice][iSpawnX];
            y = MansionInteriors[interiorChoice][iSpawnY];
            z = MansionInteriors[interiorChoice][iSpawnZ];
            a = MansionInteriors[interiorChoice][iSpawnA];
            interior = MansionInteriors[interiorChoice][iInteriorID];
        }
        default: return 0;
    }
    
    // Teleportar al jugador
    SetPlayerPos(playerid, x, y, z);
    SetPlayerFacingAngle(playerid, a);
    SetPlayerInterior(playerid, interior);
    SetPlayerVirtualWorld(playerid, PropertyData[propertySlot][pVirtualWorld]);
    
    // Guardar en qué propiedad está
    PlayerInProperty[playerid] = propertySlot;
    
    new string[128];
    format(string, sizeof(string), "{00FF00}[INFO]{FFFFFF} Has entrado a: %s", PropertyData[propertySlot][pName]);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    SendClientMessage(playerid, COLOR_WHITE, "{AAAAAA}» Presiona {FFFF00}H{AAAAAA} nuevamente para salir.");
    
    return 1;
}

// Función para salir de una propiedad
stock ExitProperty(playerid)
{
    new propertySlot = PlayerInProperty[playerid];
    if(propertySlot == -1) return 0;
    
    // Teleportar a la entrada
    SetPlayerPos(playerid, 
        PropertyData[propertySlot][pPosX], 
        PropertyData[propertySlot][pPosY], 
        PropertyData[propertySlot][pPosZ]);
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);
    
    // Limpiar variable
    PlayerInProperty[playerid] = -1;
    
    new string[128];
    format(string, sizeof(string), "{00FF00}[INFO]{FFFFFF} Has salido de: %s", PropertyData[propertySlot][pName]);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    return 1;
}

// Comando manual para entrar/salir (alternativa a la tecla H)
CMD:entrar(playerid, params[])
{
    if(PlayerInProperty[playerid] != -1)
    {
        ExitProperty(playerid);
    }
    else
    {
        new propertySlot = GetNearestProperty(playerid);
        if(propertySlot == -1)
        {
            SendClientMessage(playerid, COLOR_ERROR, "{FF0000}[ERROR]{FFFFFF} No estás cerca de ninguna propiedad.");
            return 1;
        }
        EnterProperty(playerid, propertySlot);
    }
    return 1;
}

CMD:salir(playerid, params[])
{
    return cmd_entrar(playerid, params);
}