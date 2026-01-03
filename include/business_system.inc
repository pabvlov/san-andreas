/*
    Sistema de Negocios (24/7, Tiendas, etc)
    
    - Crear negocios como admin
    - Asignar puntos de venta
    - Usuarios compran con /comprar
    - Sistema de stock
*/

#if defined _business_system_included
    #endinput
#endif
#define _business_system_included

#define MAX_BUSINESSES 100
#define BUSINESS_PICKUP_RANGE 2.0

// Tipos de negocio
enum E_BUSINESS_TYPE
{
    BUSINESS_24_7 = 1,
    BUSINESS_GUNSHOP = 2,
    BUSINESS_CLOTHES = 3,
    BUSINESS_RESTAURANT = 4
}

// Estructura de negocio
enum E_BUSINESS_DATA
{
    bID,
    bType,
    bName[64],
    Float:bPosX,
    Float:bPosY,
    Float:bPosZ,
    Float:bInteriorX,
    Float:bInteriorY,
    Float:bInteriorZ,
    bInterior,
    bVirtualWorld,
    bPickupIcon,          // ID del icono del pickup
    bPickup,
    Text3D:bLabel,
    bInteriorPickup,      // Pickup del punto de venta interior
    Text3D:bInteriorLabel // Label del punto de venta interior
}

new BusinessData[MAX_BUSINESSES][E_BUSINESS_DATA];
new BusinessCount = 0;
new PlayerInBusiness[MAX_PLAYERS] = {-1, ...}; // Trackear en qué tienda está el jugador

// Inicializar sistema
stock InitBusinessSystem()
{
    LoadAllBusinesses();
    printf("[Business System] Sistema iniciado - %d negocios cargados", BusinessCount);
}

// Cargar todos los negocios
stock LoadAllBusinesses()
{
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT id, shop_category, name, pos_x, pos_y, pos_z, interior_x, interior_y, interior_z, interior, virtual_world, COALESCE(pickup_icon, 1274) as pickup_icon \
        FROM shops WHERE pos_x IS NOT NULL ORDER BY id ASC LIMIT %d",
        MAX_BUSINESSES);
    
    mysql_tquery(g_MySQL, query, "OnLoadBusinesses", "");
}

forward OnLoadBusinesses();
public OnLoadBusinesses()
{
    BusinessCount = 0;
    new rows = cache_num_rows();
    
    printf("[DEBUG LoadAllBusinesses] Cargando %d negocios", rows);
    
    for(new i = 0; i < rows && i < MAX_BUSINESSES; i++)
    {
        cache_get_value_name_int(i, "id", BusinessData[i][bID]);
        
        // Convertir shop_category a tipo
        new category[32];
        cache_get_value_name(i, "shop_category", category, 32);
        if(strcmp(category, "general_store", true) == 0) BusinessData[i][bType] = 1;
        else if(strcmp(category, "ammunation", true) == 0) BusinessData[i][bType] = 2;
        else if(strcmp(category, "clothing", true) == 0) BusinessData[i][bType] = 3;
        else if(strcmp(category, "hardware", true) == 0) BusinessData[i][bType] = 4;
        else BusinessData[i][bType] = 1;
        
        cache_get_value_name(i, "name", BusinessData[i][bName], 64);
        cache_get_value_name_float(i, "pos_x", BusinessData[i][bPosX]);
        cache_get_value_name_float(i, "pos_y", BusinessData[i][bPosY]);
        cache_get_value_name_float(i, "pos_z", BusinessData[i][bPosZ]);
        cache_get_value_name_float(i, "interior_x", BusinessData[i][bInteriorX]);
        cache_get_value_name_float(i, "interior_y", BusinessData[i][bInteriorY]);
        cache_get_value_name_float(i, "interior_z", BusinessData[i][bInteriorZ]);
        cache_get_value_name_int(i, "interior", BusinessData[i][bInterior]);
        cache_get_value_name_int(i, "virtual_world", BusinessData[i][bVirtualWorld]);
        cache_get_value_name_int(i, "pickup_icon", BusinessData[i][bPickupIcon]);
        
        printf("[DEBUG LoadAllBusinesses] Slot %d: ID=%d, Name=%s, Interior=%d, Icon=%d, Pos=(%.2f,%.2f,%.2f)", 
            i, BusinessData[i][bID], BusinessData[i][bName], BusinessData[i][bInterior], BusinessData[i][bPickupIcon],
            BusinessData[i][bPosX], BusinessData[i][bPosY], BusinessData[i][bPosZ]);
        
        // Inicializar pickups
        BusinessData[i][bPickup] = -1;
        BusinessData[i][bInteriorPickup] = -1;
        
        // Crear pickup y label
        CreateBusinessPickup(i);
        BusinessCount++;
    }
    
    return 1;
}

// Crear pickup y label del negocio
stock CreateBusinessPickup(idx)
{
    // Destruir pickup anterior si existe
    if(BusinessData[idx][bPickup] != -1)
        DestroyDynamicPickup(BusinessData[idx][bPickup]);
    
    if(IsValidDynamic3DTextLabel(BusinessData[idx][bLabel]))
        DestroyDynamic3DTextLabel(BusinessData[idx][bLabel]);
    
    // Usar icono personalizado o por defecto según tipo
    new modelid = BusinessData[idx][bPickupIcon];
    if(modelid == 0) // Si no tiene icono configurado, usar por defecto según tipo
    {
        switch(BusinessData[idx][bType])
        {
            case BUSINESS_24_7: modelid = 1239; // Icono de shopping
            case BUSINESS_GUNSHOP: modelid = 1242; // Icono de arma
            case BUSINESS_CLOTHES: modelid = 1275; // Icono de ropa
            case BUSINESS_RESTAURANT: modelid = 1272; // Icono de comida
            default: modelid = 1274;
        }
    }
    
    BusinessData[idx][bPickup] = CreateDynamicPickup(modelid, 1, 
        BusinessData[idx][bPosX], BusinessData[idx][bPosY], BusinessData[idx][bPosZ],
        0, 0); // Siempre en VW 0, Interior 0 para entrada exterior
    
    // Crear label
    new labelText[256];
    format(labelText, sizeof(labelText), 
        "{00FF00}%s\n{FFFFFF}ID: %d\n{FFFF00}/enter {FFFFFF}o {FFFF00}H {FFFFFF}para entrar",
        BusinessData[idx][bName], BusinessData[idx][bID]);
    
    BusinessData[idx][bLabel] = CreateDynamic3DTextLabel(labelText, 0xFFFFFFFF,
        BusinessData[idx][bPosX], BusinessData[idx][bPosY], BusinessData[idx][bPosZ] + 0.5,
        10.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 1,
        BusinessData[idx][bVirtualWorld], BusinessData[idx][bInterior]);
    
    // Crear pickup y label interior si está configurado
    if(BusinessData[idx][bInteriorX] != 0.0 || BusinessData[idx][bInteriorY] != 0.0)
    {
        // Destruir pickup interior anterior si existe
        if(BusinessData[idx][bInteriorPickup] != -1)
            DestroyDynamicPickup(BusinessData[idx][bInteriorPickup]);
        
        if(IsValidDynamic3DTextLabel(BusinessData[idx][bInteriorLabel]))
            DestroyDynamic3DTextLabel(BusinessData[idx][bInteriorLabel]);
        
        // Crear pickup interior (mismo icono que exterior)
        BusinessData[idx][bInteriorPickup] = CreateDynamicPickup(modelid, 1,
            BusinessData[idx][bInteriorX], BusinessData[idx][bInteriorY], BusinessData[idx][bInteriorZ],
            BusinessData[idx][bVirtualWorld], BusinessData[idx][bInterior]);
        
        // Crear label interior
        format(labelText, sizeof(labelText),
            "{00FF00}Punto de Venta\\n{FFFFFF}%s\\n{FFFF00}/comprar {FFFFFF}para ver productos",
            BusinessData[idx][bName]);
        
        BusinessData[idx][bInteriorLabel] = CreateDynamic3DTextLabel(labelText, 0xFFFFFFFF,
            BusinessData[idx][bInteriorX], BusinessData[idx][bInteriorY], BusinessData[idx][bInteriorZ] + 0.5,
            10.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 1,
            BusinessData[idx][bVirtualWorld], BusinessData[idx][bInterior]);
    }
    
    return 1;
}

// Obtener negocio cercano
stock GetNearbyBusiness(playerid, Float:range = BUSINESS_PICKUP_RANGE)
{
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    new interior = GetPlayerInterior(playerid);
    new vw = GetPlayerVirtualWorld(playerid);
    
    printf("[DEBUG GetNearbyBusiness] Jugador pos: %.2f, %.2f, %.2f | Int: %d | VW: %d", x, y, z, interior, vw);
    printf("[DEBUG GetNearbyBusiness] BusinessCount: %d", BusinessCount);
    
    for(new i = 0; i < BusinessCount; i++)
    {
        if(BusinessData[i][bID] == 0) continue;
        
        printf("[DEBUG GetNearbyBusiness] Checking slot %d (ID: %d)", i, BusinessData[i][bID]);
        printf("[DEBUG GetNearbyBusiness] - Pos entrada: %.2f, %.2f, %.2f", 
            BusinessData[i][bPosX], BusinessData[i][bPosY], BusinessData[i][bPosZ]);
        
        // Verificar punto de entrada exterior (siempre en int 0, vw 0)
        if(IsPlayerInRangeOfPoint(playerid, range, 
            BusinessData[i][bPosX], BusinessData[i][bPosY], BusinessData[i][bPosZ]))
        {
            printf("[DEBUG GetNearbyBusiness] - En rango de entrada exterior!");
            printf("[DEBUG GetNearbyBusiness] - Player int/vw: %d/%d, Business int/vw exterior: 0/0", interior, vw);
            
            // La entrada siempre está en interior 0, virtual world 0
            if(interior == 0 && vw == 0)
            {
                printf("[DEBUG GetNearbyBusiness] - MATCH! Retornando slot %d", i);
                return i;
            }
        }
        
        // Verificar punto de venta interior (si está configurado)
        if(BusinessData[i][bInteriorX] != 0.0 || BusinessData[i][bInteriorY] != 0.0)
        {
            printf("[DEBUG GetNearbyBusiness] - Pos interior: %.2f, %.2f, %.2f (Int: %d)", 
                BusinessData[i][bInteriorX], BusinessData[i][bInteriorY], BusinessData[i][bInteriorZ],
                BusinessData[i][bInterior]);
            
            if(IsPlayerInRangeOfPoint(playerid, range,
                BusinessData[i][bInteriorX], BusinessData[i][bInteriorY], BusinessData[i][bInteriorZ]))
            {
                printf("[DEBUG GetNearbyBusiness] - En rango de punto interior!");
                if(interior == BusinessData[i][bInterior] && vw == BusinessData[i][bVirtualWorld])
                {
                    printf("[DEBUG GetNearbyBusiness] - MATCH interior! Retornando slot %d", i);
                    return i;
                }
            }
        }
    }
    
    printf("[DEBUG GetNearbyBusiness] - No se encontro negocio cercano");
    return -1;
}

// Entrar a una tienda
stock EnterBusiness(playerid, businessSlot)
{
    if(businessSlot < 0 || businessSlot >= BusinessCount)
        return 0;
    
    if(BusinessData[businessSlot][bID] == 0)
        return 0;
    
    // Debug: Mostrar todos los datos del negocio
    printf("[Business DEBUG] ===== DATOS DEL NEGOCIO =====");
    printf("[Business DEBUG] Slot: %d, ID: %d, Nombre: %s", businessSlot, BusinessData[businessSlot][bID], BusinessData[businessSlot][bName]);
    printf("[Business DEBUG] Tipo: %d", BusinessData[businessSlot][bType]);
    printf("[Business DEBUG] Pos Entrada: %.2f, %.2f, %.2f", BusinessData[businessSlot][bPosX], BusinessData[businessSlot][bPosY], BusinessData[businessSlot][bPosZ]);
    printf("[Business DEBUG] Pos Interior: %.2f, %.2f, %.2f", BusinessData[businessSlot][bInteriorX], BusinessData[businessSlot][bInteriorY], BusinessData[businessSlot][bInteriorZ]);
    printf("[Business DEBUG] Interior ID: %d", BusinessData[businessSlot][bInterior]);
    printf("[Business DEBUG] Virtual World: %d", BusinessData[businessSlot][bVirtualWorld]);
    printf("[Business DEBUG] ==============================");
    
    // Verificar si tiene interior configurado
    if(BusinessData[businessSlot][bInterior] == 0)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Esta tienda no tiene interior configurado.");
        SendClientMessage(playerid, COLOR_INFO, "{FFFF00}Admin debe configurar: /configurarbusiness");
        return 0;
    }
    
    // Si no tiene posición interior configurada, usar coordenadas por defecto del interior
    new Float:intX = BusinessData[businessSlot][bInteriorX];
    new Float:intY = BusinessData[businessSlot][bInteriorY];
    new Float:intZ = BusinessData[businessSlot][bInteriorZ];
    
    // Si no hay posición configurada, usar coordenadas de spawn del interior
    if(intX == 0.0 && intY == 0.0 && intZ == 0.0)
    {
        // Coordenadas por defecto según el interior configurado
        switch(BusinessData[businessSlot][bInterior])
        {
            case 17: { intX = -25.884498; intY = -185.868988; intZ = 1003.546875; } // 24/7 Shop 1
            case 10: { intX = 6.091179; intY = -29.271898; intZ = 1003.549438; } // 24/7 Shop 2
            case 18: { intX = -30.946699; intY = -89.609596; intZ = 1003.546875; } // 24/7 Shop 3
            case 1: { intX = 286.148986; intY = -40.644397; intZ = 1001.515625; } // Ammunation 1
            case 4: { intX = 286.800994; intY = -82.547599; intZ = 1001.515625; } // Ammunation 2
            case 6: { intX = 296.919982; intY = -108.071998; intZ = 1001.515625; } // Ammunation 3
            default: { intX = -25.884498; intY = -185.868988; intZ = 1003.546875; } // 24/7 por defecto
        }
    }
    
    // Guardar posición actual
    GetPlayerPos(playerid, CharacterData[playerid][cPosX], 
                          CharacterData[playerid][cPosY], 
                          CharacterData[playerid][cPosZ]);
    GetPlayerFacingAngle(playerid, CharacterData[playerid][cPosA]);
    CharacterData[playerid][cInterior] = GetPlayerInterior(playerid);
    CharacterData[playerid][cVirtualWorld] = GetPlayerVirtualWorld(playerid);
    
    // Teleportar al interior
    SetPlayerPos(playerid, intX, intY, intZ);
    SetPlayerInterior(playerid, BusinessData[businessSlot][bInterior]);
    SetPlayerVirtualWorld(playerid, BusinessData[businessSlot][bVirtualWorld]);
    
    PlayerInBusiness[playerid] = businessSlot;
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Has entrado a {FFFF00}%s", BusinessData[businessSlot][bName]);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    SendClientMessage(playerid, COLOR_INFO, "{FFFF00}/comprar {FFFFFF}para ver productos | {FFFF00}H {FFFFFF}o {FFFF00}/salir {FFFFFF}para salir");
    
    return 1;
}

// Salir de una tienda
stock ExitBusiness(playerid)
{
    if(PlayerInBusiness[playerid] == -1)
        return 0;
    
    // Teleportar a la posición guardada
    SetPlayerPos(playerid, CharacterData[playerid][cPosX],
                          CharacterData[playerid][cPosY],
                          CharacterData[playerid][cPosZ]);
    SetPlayerFacingAngle(playerid, CharacterData[playerid][cPosA]);
    SetPlayerInterior(playerid, CharacterData[playerid][cInterior]);
    SetPlayerVirtualWorld(playerid, CharacterData[playerid][cVirtualWorld]);
    
    PlayerInBusiness[playerid] = -1;
    
    SendClientMessage(playerid, COLOR_INFO, "{00FF00}Has salido de la tienda.");
    return 1;
}

// Comando: /creartienda - Admin
CMD:creartienda(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}No tienes permisos (Admin nivel 3+)");
    
    new type, name[64];
    if(sscanf(params, "ds[64]", type, name))
    {
        SendClientMessage(playerid, COLOR_INFO, "{FFFF00}Uso: {FFFFFF}/creartienda [tipo] [nombre]");
        SendClientMessage(playerid, COLOR_INFO, "{FFFFFF}Tipos: 1=24/7, 2=Armeria, 3=Ropa, 4=Restaurante");
        return 1;
    }
    
    if(type < 1 || type > 4)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Tipo invalido (1-4)");
    
    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);
    
    new interior = GetPlayerInterior(playerid);
    new vw = GetPlayerVirtualWorld(playerid);
    
    // Mapear tipo a property type_id y shop_category
    new query[512], category[32], propertyTypeId;
    
    switch(type)
    {
        case 1: { category = "general_store"; propertyTypeId = 10; } // 24/7 Market
        case 2: { category = "ammunation"; propertyTypeId = 11; }    // Ammunation
        case 3: { category = "clothing"; propertyTypeId = 12; }      // Tienda de Ropa
        case 4: { category = "hardware"; propertyTypeId = 13; }      // Ferretería
        default: { category = "general_store"; propertyTypeId = 10; }
    }
    
    // Guardar datos temporalmente para el callback
    SetPVarInt(playerid, "ShopType", type);
    SetPVarFloat(playerid, "ShopX", x);
    SetPVarFloat(playerid, "ShopY", y);
    SetPVarFloat(playerid, "ShopZ", z);
    SetPVarFloat(playerid, "ShopA", a);
    SetPVarInt(playerid, "ShopInterior", interior);
    SetPVarInt(playerid, "ShopVW", vw);
    
    new pvString[128];
    format(pvString, sizeof(pvString), "%s", name);
    SetPVarString(playerid, "ShopName", pvString);
    format(pvString, sizeof(pvString), "%s", category);
    SetPVarString(playerid, "ShopCategory", pvString);
    
    // Crear property primero
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO properties (type_id, name, owner_type, fiscal_price, pos_x, pos_y, pos_z, interior, virtual_world) \
        VALUES (%d, '%e', 'state', 0, %.4f, %.4f, %.4f, %d, %d)",
        propertyTypeId, name, x, y, z, interior, vw);
    
    mysql_tquery(g_MySQL, query, "OnBusinessPropertyCreated", "d", playerid);
    
    SendClientMessage(playerid, COLOR_SUCCESS, "{00FF00}Creando tienda...");
    return 1;
}

forward OnBusinessPropertyCreated(playerid);
public OnBusinessPropertyCreated(playerid)
{
    new propertyid = cache_insert_id();
    
    if(propertyid == 0)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error al crear property. Verifica la consola del servidor.");
        DeletePVar(playerid, "ShopType");
        DeletePVar(playerid, "ShopX");
        DeletePVar(playerid, "ShopY");
        DeletePVar(playerid, "ShopZ");
        DeletePVar(playerid, "ShopA");
        DeletePVar(playerid, "ShopInterior");
        DeletePVar(playerid, "ShopVW");
        DeletePVar(playerid, "ShopName");
        DeletePVar(playerid, "ShopCategory");
        return 1;
    }
    
    // Recuperar datos guardados
    new Float:x = GetPVarFloat(playerid, "ShopX");
    new Float:y = GetPVarFloat(playerid, "ShopY");
    new Float:z = GetPVarFloat(playerid, "ShopZ");
    new interior = GetPVarInt(playerid, "ShopInterior");
    new vw = GetPVarInt(playerid, "ShopVW");
    
    new shopName[64], category[32];
    GetPVarString(playerid, "ShopName", shopName, sizeof(shopName));
    GetPVarString(playerid, "ShopCategory", category, sizeof(category));
    
    // Crear shop vinculada a la property
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO shops (property_id, shop_category, name, pos_x, pos_y, pos_z, interior, virtual_world) \
        VALUES (%d, '%s', '%e', %.4f, %.4f, %.4f, %d, %d)",
        propertyid, category, shopName, x, y, z, interior, vw);
    
    mysql_tquery(g_MySQL, query, "OnBusinessCreated", "dd", playerid, propertyid);
    
    return 1;
}

forward OnBusinessCreated(playerid, propertyid);
public OnBusinessCreated(playerid, propertyid)
{
    new shopid = cache_insert_id();
    
    // Recargar negocios y propiedades
    LoadAllBusinesses();
    LoadAllProperties();
    
    // Limpiar PVars
    DeletePVar(playerid, "ShopType");
    DeletePVar(playerid, "ShopX");
    DeletePVar(playerid, "ShopY");
    DeletePVar(playerid, "ShopZ");
    DeletePVar(playerid, "ShopA");
    DeletePVar(playerid, "ShopInterior");
    DeletePVar(playerid, "ShopVW");
    DeletePVar(playerid, "ShopName");
    DeletePVar(playerid, "ShopCategory");
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Tienda creada - Property ID: %d, Shop ID: %d", propertyid, shopid);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    SendClientMessage(playerid, COLOR_INFO, "{FFFF00}/configurarbusiness %d {FFFFFF}para configurar interior", shopid);
    
    printf("[Business] %s creo una tienda (Property: %d, Shop: %d)", CharacterData[playerid][cName], propertyid, shopid);
    return 1;
}

// Comando: /configurarbusiness - Admin
CMD:configurarbusiness(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}No tienes permisos (Admin nivel 3+)");
    
    new businessid;
    if(sscanf(params, "d", businessid))
    {
        SendClientMessage(playerid, COLOR_INFO, "{FFFF00}Uso: {FFFFFF}/configurarbusiness [business_id]");
        return 1;
    }
    
    // Validar que el negocio existe
    new bool:found = false;
    for(new i = 0; i < BusinessCount; i++)
    {
        if(BusinessData[i][bID] == businessid)
        {
            found = true;
            break;
        }
    }
    
    if(!found)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Negocio no encontrado");
    
    // Guardar ID en variable temporal
    SetPVarInt(playerid, "ConfigBusinessID", businessid);
    
    // Mostrar dialog de configuración
    new dialog[512];
    format(dialog, sizeof(dialog),
        "Setear Interior ID\n\
        Setear Posicion Interior\n\
        Setear Posicion Entrada\n\
        Cambiar Icono Pickup\n\
        Abrir/Cerrar Tienda\n\
        Ver Informacion");
    
    ShowPlayerDialog(playerid, DIALOG_BUSINESS_CONFIG, DIALOG_STYLE_LIST,
        "Configurar Business", dialog, "Seleccionar", "Cancelar");
    
    return 1;
}

// Handler del dialog de configuración
stock OnDialogBusinessConfig(playerid, response, listitem)
{
    if(!response)
    {
        DeletePVar(playerid, "ConfigBusinessID");
        return 1;
    }
    
    new businessid = GetPVarInt(playerid, "ConfigBusinessID");
    
    switch(listitem)
    {
        case 0: // Setear Interior ID
        {
            new string[256];
            format(string, sizeof(string),
                "{FFFFFF}Ingresa el ID de interior para el negocio %d:\n\n\
                {FFFF00}Ejemplos:\n\
                {FFFFFF}0 = Exterior\n\
                6 = Interior 24/7\n\
                1 = Interior Ammunation",
                businessid);
            
            ShowPlayerDialog(playerid, DIALOG_BUSINESS_CONFIG + 1, DIALOG_STYLE_INPUT,
                "Setear Interior ID", string, "Confirmar", "Volver");
        }
        case 1: // Setear Posicion Interior
        {
            new Float:x, Float:y, Float:z;
            GetPlayerPos(playerid, x, y, z);
            
            new query[384];
            mysql_format(g_MySQL, query, sizeof(query),
                "UPDATE shops SET interior_x = %.4f, interior_y = %.4f, interior_z = %.4f WHERE id = %d",
                x, y, z, businessid);
            
            mysql_tquery(g_MySQL, query);
            LoadAllBusinesses();
            
            new string[256];
            format(string, sizeof(string), "{00FF00}Posicion interior actualizada a: {FFFFFF}%.1f, %.1f, %.1f", x, y, z);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
            
            DeletePVar(playerid, "ConfigBusinessID");
        }
        case 2: // Setear Posicion Entrada
        {
            new Float:x, Float:y, Float:z;
            GetPlayerPos(playerid, x, y, z);
            
            new query[384];
            mysql_format(g_MySQL, query, sizeof(query),
                "UPDATE shops SET pos_x = %.4f, pos_y = %.4f, pos_z = %.4f WHERE id = %d",
                x, y, z, businessid);
            
            mysql_tquery(g_MySQL, query);
            LoadAllBusinesses();
            
            new string[256];
            format(string, sizeof(string), "{00FF00}Posicion entrada actualizada a: {FFFFFF}%.1f, %.1f, %.1f", x, y, z);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
            
            DeletePVar(playerid, "ConfigBusinessID");
        }
        case 3: // Cambiar Icono Pickup
        {
            new iconstring[512];
            format(iconstring, sizeof(iconstring),
                "{FFFFFF}Selecciona el icono para el negocio:\n\n\
                {FFFF00}Iconos comunes:\n\
                {FFFFFF}1274 - Icono generico tienda\n\
                1239 - 24/7 (tienda)\n\
                1242 - Ammunation\n\
                1275 - Ropa\n\
                1559 - Ferreteria\n\n\
                {AAAAAA}Ingresa el ID del icono (1000-1999):");
            
            ShowPlayerDialog(playerid, DIALOG_BUSINESS_CONFIG + 3, DIALOG_STYLE_INPUT,
                "Cambiar Icono Pickup", iconstring, "Confirmar", "Volver");
        }
        case 4: // Abrir/Cerrar Tienda
        {
            // Buscar el property_id de esta shop
            new query[256];
            mysql_format(g_MySQL, query, sizeof(query),
                "SELECT property_id FROM shops WHERE id = %d",
                businessid);
            
            mysql_tquery(g_MySQL, query, "OnToggleBusinessLock", "dd", playerid, businessid);
            
            SendClientMessage(playerid, COLOR_INFO, "{FFFF00}Cambiando estado de la tienda...");
            DeletePVar(playerid, "ConfigBusinessID");
        }
        case 5: // Ver Informacion
        {
            new idx = -1;
            for(new i = 0; i < BusinessCount; i++)
            {
                if(BusinessData[i][bID] == businessid)
                {
                    idx = i;
                    break;
                }
            }
            
            if(idx != -1)
            {
                new info[512];
                format(info, sizeof(info),
                    "{FFFF00}Business ID: {FFFFFF}%d\n\
                    {FFFF00}Nombre: {FFFFFF}%s\n\
                    {FFFF00}Tipo: {FFFFFF}%d\n\
                    {FFFF00}Entrada: {FFFFFF}%.1f, %.1f, %.1f\n\
                    {FFFF00}Interior Pos: {FFFFFF}%.1f, %.1f, %.1f\n\
                    {FFFF00}Interior ID: {FFFFFF}%d\n\
                    {FFFF00}Virtual World: {FFFFFF}%d",
                    BusinessData[idx][bID], BusinessData[idx][bName], BusinessData[idx][bType],
                    BusinessData[idx][bPosX], BusinessData[idx][bPosY], BusinessData[idx][bPosZ],
                    BusinessData[idx][bInteriorX], BusinessData[idx][bInteriorY], BusinessData[idx][bInteriorZ],
                    BusinessData[idx][bInterior], BusinessData[idx][bVirtualWorld]);
                
                ShowPlayerDialog(playerid, DIALOG_BUSINESS_CONFIG + 2, DIALOG_STYLE_MSGBOX,
                    "Informacion del Business", info, "Cerrar", "");
            }
            
            DeletePVar(playerid, "ConfigBusinessID");
        }
    }
    
    return 1;
}

// Handler para input de interior ID
stock OnDialogBusinessCfgInteriorID(playerid, response, const inputtext[])
{
    new businessid = GetPVarInt(playerid, "ConfigBusinessID");
    
    if(!response)
    {
        // Volver al menu principal
        SetPVarInt(playerid, "ConfigBusinessID", businessid);
        
        new dialog[512];
        format(dialog, sizeof(dialog),
            "Setear Interior ID\n\
            Setear Posicion Interior\n\
            Setear Posicion Entrada\n\
            Ver Informacion");
        
        ShowPlayerDialog(playerid, DIALOG_BUSINESS_CONFIG, DIALOG_STYLE_LIST,
            "Configurar Business", dialog, "Seleccionar", "Cancelar");
        return 1;
    }
    
    new interiorid = strval(inputtext);
    
    if(interiorid < 0 || interiorid > 18)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Interior ID invalido (0-18)");
        DeletePVar(playerid, "ConfigBusinessID");
        return 1;
    }
    
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "UPDATE shops SET interior = %d WHERE id = %d",
        interiorid, businessid);
    
    mysql_tquery(g_MySQL, query, "OnBusinessInteriorUpdated", "ddd", playerid, businessid, interiorid);
    
    SendClientMessage(playerid, COLOR_INFO, "{FFFF00}Actualizando interior...");
    return 1;
}

forward OnBusinessInteriorUpdated(playerid, businessid, interiorid);
public OnBusinessInteriorUpdated(playerid, businessid, interiorid)
{
    DeletePVar(playerid, "ConfigBusinessID");
    
    // Recargar negocios
    LoadAllBusinesses();
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Interior ID actualizado a: {FFFFFF}%d - Negocios recargados", interiorid);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    SendClientMessage(playerid, COLOR_INFO, "{FFFFFF}Ya puedes usar {FFFF00}/enter {FFFFFF}para entrar a la tienda");
    
    printf("[Business] Interior actualizado: Business %d -> Interior %d", businessid, interiorid);
    return 1;
}

// Handler para cambiar icono del pickup
stock OnDialogBusinessCfgIcon(playerid, response, const inputtext[])
{
    new businessid = GetPVarInt(playerid, "ConfigBusinessID");
    
    if(!response)
    {
        DeletePVar(playerid, "ConfigBusinessID");
        return 1;
    }
    
    new iconid = strval(inputtext);
    
    if(iconid < 1000 || iconid > 1999)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}ID de icono invalido (1000-1999)");
        DeletePVar(playerid, "ConfigBusinessID");
        return 1;
    }
    
    // Actualizar icon_id en la base de datos
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "UPDATE shops SET pickup_icon = %d WHERE id = %d",
        iconid, businessid);
    
    mysql_tquery(g_MySQL, query);
    LoadAllBusinesses();
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Icono actualizado a ID: {FFFFFF}%d", iconid);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    DeletePVar(playerid, "ConfigBusinessID");
    return 1;
}

// Callback para toggle lock/unlock de tienda
forward OnToggleBusinessLock(playerid, businessid);
public OnToggleBusinessLock(playerid, businessid)
{
    if(cache_num_rows() == 0)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: No se encontro la property asociada.");
        return 1;
    }
    
    new propertyid;
    cache_get_value_name_int(0, "property_id", propertyid);
    
    // Toggle el estado is_locked en properties
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "UPDATE properties SET is_locked = NOT is_locked WHERE id = %d",
        propertyid);
    
    mysql_tquery(g_MySQL, query, "OnBusinessLockToggled", "ddd", playerid, businessid, propertyid);
    
    return 1;
}

forward OnBusinessLockToggled(playerid, businessid, propertyid);
public OnBusinessLockToggled(playerid, businessid, propertyid)
{
    // Recargar propiedades para actualizar estado
    LoadAllProperties();
    
    // Consultar estado actual
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT is_locked FROM properties WHERE id = %d",
        propertyid);
    
    new Cache:result = mysql_query(g_MySQL, query);
    
    if(cache_num_rows() > 0)
    {
        new is_locked;
        cache_get_value_name_int(0, "is_locked", is_locked);
        
        new string[128];
        if(is_locked)
        {
            format(string, sizeof(string), "{00FF00}Tienda ID %d {FFFFFF}ahora esta {FF0000}CERRADA", businessid);
        }
        else
        {
            format(string, sizeof(string), "{00FF00}Tienda ID %d {FFFFFF}ahora esta {00FF00}ABIERTA", businessid);
        }
        SendClientMessage(playerid, COLOR_SUCCESS, string);
    }
    
    cache_delete(result);
    return 1;
}

// Comando: /setinteriorpos - Admin
CMD:setinteriorpos(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}No tienes permisos (Admin nivel 3+)");
    
    new businessid;
    if(sscanf(params, "d", businessid))
    {
        SendClientMessage(playerid, COLOR_INFO, "{FFFF00}Uso: {FFFFFF}/setinteriorpos [business_id]");
        SendClientMessage(playerid, COLOR_INFO, "{FFFFFF}Establece tu posicion actual como punto de venta interior");
        return 1;
    }
    
    // Validar que el negocio existe
    new bool:found = false;
    for(new i = 0; i < BusinessCount; i++)
    {
        if(BusinessData[i][bID] == businessid)
        {
            found = true;
            break;
        }
    }
    
    if(!found)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Negocio no encontrado");
    
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    
    // Actualizar en BD
    new query[384];
    mysql_format(g_MySQL, query, sizeof(query),
        "UPDATE shops SET interior_x = %.4f, interior_y = %.4f, interior_z = %.4f WHERE id = %d",
        x, y, z, businessid);
    
    mysql_tquery(g_MySQL, query, "OnBusinessInteriorPosUpdated", "ddfff", playerid, businessid, x, y, z);
    
    SendClientMessage(playerid, COLOR_SUCCESS, "{00FF00}Actualizando posicion interior...");
    return 1;
}

forward OnBusinessInteriorPosUpdated(playerid, businessid, Float:x, Float:y, Float:z);
public OnBusinessInteriorPosUpdated(playerid, businessid, Float:x, Float:y, Float:z)
{
    // Recargar negocios
    LoadAllBusinesses();
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Posicion interior del negocio %d actualizada (%.1f, %.1f, %.1f)", businessid, x, y, z);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    printf("[Business] Admin actualizo posicion interior del negocio %d a %.1f, %.1f, %.1f", businessid, x, y, z);
    return 1;
}

// Comando: /agregarstock - Admin
CMD:agregarstock(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}No tienes permisos (Admin nivel 3+)");
    
    new businessid, itemid, quantity, price;
    if(sscanf(params, "dddd", businessid, itemid, quantity, price))
    {
        SendClientMessage(playerid, COLOR_INFO, "{FFFF00}Uso: {FFFFFF}/agregarstock [negocio_id] [item_id] [cantidad] [precio]");
        SendClientMessage(playerid, COLOR_INFO, "{FFFFFF}Items: 1=Llave Master, 2=Llave Normal, 5=Telefono, 6=Kit Reparacion, 7=Hamburguesa, 8=Agua");
        return 1;
    }
    
    // Insertar o actualizar stock
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO shop_stock (shop_id, item_id, quantity, price) \
        VALUES (%d, %d, %d, %d) \
        ON DUPLICATE KEY UPDATE quantity = quantity + %d, price = %d",
        businessid, itemid, quantity, price, quantity, price);
    
    mysql_tquery(g_MySQL, query);
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Stock agregado: Item %d x%d a $%d en negocio %d", 
        itemid, quantity, price, businessid);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    printf("[Business] %s agrego stock: Item %d x%d en negocio %d", 
        CharacterData[playerid][cName], itemid, quantity, businessid);
    
    return 1;
}

// Comando: /comprar - Usuario
CMD:comprar(playerid, params[])
{
    if(!CharacterData[playerid][cID])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Debes seleccionar un personaje.");
    
    new businessIdx = -1;
    
    // Verificar si está dentro de una tienda
    if(PlayerInBusiness[playerid] != -1)
    {
        businessIdx = PlayerInBusiness[playerid];
    }
    else
    {
        // Si no está dentro, verificar si está cerca del pickup exterior
        businessIdx = GetNearbyBusiness(playerid);
    }
    
    if(businessIdx == -1)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No estas cerca de ninguna tienda.");
    
    // Cargar stock del negocio
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT ss.item_id, i.name, ss.quantity, ss.price \
        FROM shop_stock ss \
        JOIN items i ON ss.item_id = i.id \
        WHERE ss.shop_id = %d AND ss.quantity > 0 \
        ORDER BY ss.item_id ASC",
        BusinessData[businessIdx][bID]);
    
    mysql_tquery(g_MySQL, query, "OnShowBusinessStock", "dd", playerid, businessIdx);
    return 1;
}

forward OnShowBusinessStock(playerid, businessIdx);
public OnShowBusinessStock(playerid, businessIdx)
{
    new rows = cache_num_rows();
    
    if(rows == 0)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Esta tienda no tiene stock disponible.");
        return 1;
    }
    
    new dialog[2048], line[128];
    new itemid, quantity, price;
    new itemName[64];
    
    strcat(dialog, "Item\tPrecio\tStock\n");
    
    for(new i = 0; i < rows; i++)
    {
        cache_get_value_name_int(i, "item_id", itemid);
        cache_get_value_name(i, "name", itemName, 64);
        cache_get_value_name_int(i, "quantity", quantity);
        cache_get_value_name_int(i, "price", price);
        
        format(line, sizeof(line), "%s\t$%d\t%d\n", itemName, price, quantity);
        strcat(dialog, line);
    }
    
    new title[128];
    format(title, sizeof(title), "%s - Tienda", BusinessData[businessIdx][bName]);
    
    ShowPlayerDialog(playerid, DIALOG_BUSINESS_SHOP, DIALOG_STYLE_TABLIST_HEADERS, 
        title, dialog, "Comprar", "Cancelar");
    
    return 1;
}

// Handler del dialog de compra
stock OnDialogBusinessShop(playerid, response, listitem)
{
    if(!response) return 1;
    
    new businessIdx = -1;
    
    // Verificar si está dentro de una tienda
    if(PlayerInBusiness[playerid] != -1)
    {
        businessIdx = PlayerInBusiness[playerid];
    }
    else
    {
        // Si no, buscar cerca
        businessIdx = GetNearbyBusiness(playerid);
    }
    
    if(businessIdx == -1)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Ya no estas cerca de la tienda.");
        return 1;
    }
    
    // Recargar stock para obtener el item seleccionado
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT ss.item_id, i.name, ss.quantity, ss.price \
        FROM shop_stock ss \
        JOIN items i ON ss.item_id = i.id \
        WHERE ss.shop_id = %d AND ss.quantity > 0 \
        ORDER BY ss.item_id ASC LIMIT %d, 1",
        BusinessData[businessIdx][bID], listitem);
    
    mysql_tquery(g_MySQL, query, "OnBusinessPurchase", "d", playerid);
    return 1;
}

forward OnBusinessPurchase(playerid);
public OnBusinessPurchase(playerid)
{
    if(cache_num_rows() == 0) return 1;
    
    new itemid, quantity, price;
    new itemName[64];
    
    cache_get_value_name_int(0, "item_id", itemid);
    cache_get_value_name(0, "name", itemName, 64);
    cache_get_value_name_int(0, "quantity", quantity);
    cache_get_value_name_int(0, "price", price);
    
    // Verificar dinero
    if(CharacterData[playerid][cMoney] < price)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes suficiente dinero.");
        return 1;
    }
    
    // Verificar inventario lleno
    if(PlayerInventoryCount[playerid] >= MAX_INVENTORY_SLOTS)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Tu inventario esta lleno.");
        return 1;
    }
    
    // Buscar negocio
    new businessIdx = GetNearbyBusiness(playerid);
    if(businessIdx == -1) return 1;
    
    new businessId = BusinessData[businessIdx][bID];
    
    // Restar dinero
    GivePlayerMoney(playerid, -price);
    CharacterData[playerid][cMoney] -= price;
    
    // Dar item al inventario
    GivePlayerItem(playerid, itemid, 1, "");
    
    // Refrescar inventario para mostrar el item inmediatamente
    ShowInventoryDialog(playerid);
    
    // Teletransportar al interior si esta configurado
    if(BusinessData[businessIdx][bInteriorX] != 0.0 || BusinessData[businessIdx][bInteriorY] != 0.0)
    {
        SetPlayerPos(playerid, 
            BusinessData[businessIdx][bInteriorX], 
            BusinessData[businessIdx][bInteriorY], 
            BusinessData[businessIdx][bInteriorZ]);
        SetPlayerInterior(playerid, BusinessData[businessIdx][bInterior]);
        SetPlayerVirtualWorld(playerid, BusinessData[businessIdx][bVirtualWorld]);
    }
    
    // Reducir stock
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "UPDATE shop_stock SET quantity = quantity - 1 \
        WHERE shop_id = %d AND item_id = %d",
        businessId, itemid);
    mysql_tquery(g_MySQL, query);
    
    new string[128];
    format(string, sizeof(string), "{00FF00}Has comprado: {FFFFFF}%s {00FF00}por $%d", itemName, price);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    printf("[Business] %s compro %s por $%d en negocio %d", 
        CharacterData[playerid][cName], itemName, price, businessId);
    
    return 1;
}
