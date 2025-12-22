/*
    Sistema de Administración
    Niveles: 0=Jugador, 1=Ayudante, 2=Moderador, 3=Administrador, 4=Dueño
*/

#if defined _admin_system_included
    #endinput
#endif
#define _admin_system_included

// Dependencias
#if !defined MAX_VEHICLES_ENGINE
    #define MAX_VEHICLES_ENGINE 2000
#endif

// Colores para mensajes admin
#define COLOR_ADMIN_MSG (0xFF6347FF)

// Forward declarations
forward bool:IsHelper(playerid);
forward bool:IsModerator(playerid);
forward bool:IsAdministrator(playerid);
forward bool:IsOwner(playerid);

// Verificar permisos
stock bool:IsHelper(playerid)
{
    return (UserData[playerid][uAdminLevel] >= 1);
}

stock bool:IsModerator(playerid)
{
    return (UserData[playerid][uAdminLevel] >= 2);
}

stock bool:IsAdministrator(playerid)
{
    return (UserData[playerid][uAdminLevel] >= 3);
}

stock bool:IsOwner(playerid)
{
    return (UserData[playerid][uAdminLevel] >= 4);
}

// Comandos de administración
CMD:acmds(playerid, params[])
{
    if(!IsHelper(playerid))
    {
        new string[128];
        format(string, sizeof(string), 
            "{FF0000}Error: {FFFFFF}No tienes permisos (Admin nivel: %d, Requerido: 1+)", 
            UserData[playerid][uAdminLevel]);
        return SendClientMessage(playerid, COLOR_ERROR, string);
    }
    
    new string[2048];
    strcat(string, "{FFFFFF}=== {FF6347}Comandos de Administración{FFFFFF} ===\n\n");
    
    if(IsHelper(playerid))
    {
        strcat(string, "{FFD700}[Nivel 1 - Ayudante]{FFFFFF}\n");
        strcat(string, "/kick, /mute, /unmute, /warn, /ajail\n");
        strcat(string, "/interior (/int) - Teleport a interiores\n");
        strcat(string, "/setint [id] - Cambiar interior ID\n");
        strcat(string, "/setvw [id] - Cambiar virtual world\n");
        strcat(string, "/checkint [id] - Ver propiedades en interior\n");
        strcat(string, "/checkvw [id] - Ver propiedades en VW\n\n");
    }
    
    if(IsModerator(playerid))
    {
        strcat(string, "{FFD700}[Nivel 2 - Moderador]{FFFFFF}\n");
        strcat(string, "/ban, /sban, /freeze, /unfreeze, /goto, /gethere\n");
        strcat(string, "/vehiculos [playerid] - Gestionar vehículos\n\n");
    }
    
    if(IsAdministrator(playerid))
    {
        strcat(string, "{FFD700}[Nivel 3 - Administrador]{FFFFFF}\n");
        strcat(string, "/createhouse, /deletehouse, /edithouse\n");
        strcat(string, "/createshop, /deleteshop, /editshop\n");
        strcat(string, "/createbusiness, /deletebusiness, /editbusiness\n");
        strcat(string, "/setmoney, /setadmin, /giveweapon, /jetpack\n");
        strcat(string, "/darmarma [id] [arma] [balas] - Dar arma al inventario\n");
        strcat(string, "/darcargador [id] [tipo] [cant] - Dar munición\n\n");
    }
    
    if(IsOwner(playerid))
    {
        strcat(string, "{FFD700}[Nivel 4 - Dueño]{FFFFFF}\n");
        strcat(string, "/gmx, /loadfs, /unloadfs, /setweather, /settime\n");
    }
    
    ShowPlayerDialog(playerid, 0, DIALOG_STYLE_MSGBOX, "Comandos de Administración", string, "Cerrar", "");
    return 1;
}

// Comando de ayuda general con controles
CMD:controles(playerid, params[])
{
    new string[900];
    strcat(string, "{FFFFFF}=== {00FF00}Controles del Servidor{FFFFFF} ===\n\n");
    strcat(string, "{FFD700}[Teclas de Juego]\n");
    strcat(string, "{FFFFFF}Y - Abrir inventario\n");
    strcat(string, "{FFFFFF}N - Encender/Apagar motor del vehículo\n");
    strcat(string, "{FFFFFF}H - Entrar/Salir de propiedades\n\n");
    strcat(string, "{FFD700}[Comandos Básicos]\n");
    strcat(string, "{FFFFFF}/inventario - Ver tu inventario\n");
    strcat(string, "{FFFFFF}/motor - Alternar motor (igual que tecla N)\n");
    strcat(string, "{FFFFFF}/entrar o /salir - Entrar/salir (igual que H)\n");
    strcat(string, "{FFFFFF}/buscarvehiculo - Encontrar tus vehículos\n");
    strcat(string, "{FFFFFF}/vehiculos - Ver lista de vehículos\n\n");
    strcat(string, "{FFD700}[Sistema de Armas]\n");
    strcat(string, "{FFFFFF}/equipar [slot] - Equipar arma del inventario\n");
    strcat(string, "{FFFFFF}/recargar - Recargar arma equipada\n\n");
    strcat(string, "{FFD700}[Información]\n");
    strcat(string, "{FFFFFF}/controles o /ayuda - Ver esta ayuda\n");
    strcat(string, "{AAAAAA}Usa {FFFF00}/acmds{AAAAAA} si eres administrador.\n");
    
    ShowPlayerDialog(playerid, 0, DIALOG_STYLE_MSGBOX, "Controles y Ayuda", string, "Cerrar", "");
    return 1;
}

CMD:ayuda(playerid, params[])
{
    return cmd_controles(playerid, params);
}

CMD:help(playerid, params[])
{
    return cmd_controles(playerid, params);
}

// Formato de mensajes admin
stock SendAdminMessage(color, const message[])
{
    new string[144];
    format(string, sizeof(string), "[ADMIN] %s", message);
    
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        if(IsPlayerConnected(i) && IsHelper(i))
        {
            SendClientMessage(i, color, string);
        }
    }
    return 1;
}

stock SendAdminMessageEx(color, const fmat[], {Float,_}:...)
{
    static const STATIC_ARGS = 2;
    new n = (numargs() - STATIC_ARGS) * BYTES_PER_CELL;
    if(n)
    {
        new message[144], arg_start, arg_end;
        #emit CONST.alt        fmat
        #emit LCTRL           5
        #emit ADD
        #emit STOR.S.pri      arg_start
        
        #emit LOAD.S.pri      n
        #emit ADD
        #emit STOR.S.pri      arg_end
        
        do
        {
            #emit LOAD.I
            #emit PUSH.pri
            arg_end -= BYTES_PER_CELL;
            #emit LOAD.S.pri    arg_end
        }
        while(arg_end > arg_start);
        
        #emit PUSH.S          fmat
        #emit PUSH.C          144
        #emit PUSH.C          message
        #emit PUSH.S          n
        #emit SYSREQ.C        format
        
        n += BYTES_PER_CELL * 3;
        #emit LCTRL           4
        #emit LOAD.S.alt      n
        #emit ADD
        #emit SCTRL           4
        
        return SendAdminMessage(color, message);
    }
    return SendAdminMessage(color, fmat);
}

// Comando: /jetpack - Dar jetpack
CMD:jetpack(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Necesitas ser Admin nivel 3.");
    
    if(!CharacterData[playerid][cID])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}No has seleccionado un personaje.");
    
    new targetid;
    if(sscanf(params, "u", targetid))
    {
        // Si no se especifica jugador, darse jetpack a sí mismo
        targetid = playerid;
    }
    else if(!IsPlayerConnected(targetid))
    {
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Jugador no conectado.");
    }
    
    SetPlayerSpecialAction(targetid, SPECIAL_ACTION_USEJETPACK);
    
    new string[128];
    if(targetid == playerid)
    {
        SendClientMessage(playerid, COLOR_SUCCESS, "{00FF00}Jetpack activado.");
    }
    else
    {
        format(string, sizeof(string), "{00FF00}Has dado jetpack a {FFFFFF}%s", CharacterData[targetid][cName]);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        
        format(string, sizeof(string), "{FFD700}Admin {FFFFFF}%s {FFD700}te ha dado un jetpack.", CharacterData[playerid][cName]);
        SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
    }
    
    printf("[Admin] %s dio jetpack a %s", CharacterData[playerid][cName], CharacterData[targetid][cName]);
    return 1;
}

// Variables temporales para gestión de vehículos
new AdminVehicleTarget[MAX_PLAYERS] = {INVALID_PLAYER_ID, ...};
new AdminVehicleSelected[MAX_PLAYERS] = {-1, ...};

// Comando: /vehiculos - Ver vehículos de un jugador
CMD:vehiculos(playerid, params[])
{
    if(!IsModerator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Necesitas ser Moderador nivel 2.");
    
    new targetid;
    if(sscanf(params, "u", targetid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Uso: {FFFFFF}/vehiculos [playerid]");
    
    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Jugador no conectado.");
    
    if(!CharacterData[targetid][cID])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}El jugador no tiene personaje seleccionado.");
    
    // Guardar target
    AdminVehicleTarget[playerid] = targetid;
    
    // Consultar vehículos
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT id, model, plate, fiscal_price, pos_x, pos_y, pos_z, is_spawned \
        FROM vehicles \
        WHERE owner_type = 'character' AND owner_id = %d \
        ORDER BY id ASC",
        CharacterData[targetid][cID]);
    
    mysql_tquery(g_MySQL, query, "OnAdminLoadPlayerVehicles", "d", playerid);
    return 1;
}

forward OnAdminLoadPlayerVehicles(playerid);
public OnAdminLoadPlayerVehicles(playerid)
{
    new rows = cache_num_rows();
    
    if(rows == 0)
    {
        new targetid = AdminVehicleTarget[playerid];
        new string[128];
        format(string, sizeof(string), "{FF0000}%s no tiene vehículos registrados.", CharacterData[targetid][cName]);
        SendClientMessage(playerid, COLOR_ERROR, string);
        return 1;
    }
    
    new dialog[2048], line[128];
    new id, model, plate[16], price, Float:x, Float:y, Float:z, spawned;
    
    strcat(dialog, "ID\tModelo\tPlaca\tPrecio\tEstado\n");
    
    for(new i = 0; i < rows; i++)
    {
        cache_get_value_name_int(i, "id", id);
        cache_get_value_name_int(i, "model", model);
        cache_get_value_name(i, "plate", plate, 16);
        cache_get_value_name_int(i, "fiscal_price", price);
        cache_get_value_name_float(i, "pos_x", x);
        cache_get_value_name_float(i, "pos_y", y);
        cache_get_value_name_float(i, "pos_z", z);
        cache_get_value_name_int(i, "is_spawned", spawned);
        
        format(line, sizeof(line), "%d\t%s\t%s\t$%d\t%s\n",
            id,
            VehicleModelNames[model - 400],
            plate,
            price,
            spawned ? "{00FF00}Spawneado" : "{FF0000}Guardado");
        
        strcat(dialog, line);
    }
    
    new targetid = AdminVehicleTarget[playerid];
    new title[128];
    format(title, sizeof(title), "Vehículos de %s (%d vehículos)", CharacterData[targetid][cName], rows);
    
    ShowPlayerDialog(playerid, DIALOG_ADMIN_VEHICLE_LIST, DIALOG_STYLE_TABLIST_HEADERS, title, dialog, "Seleccionar", "Cerrar");
    return 1;
}

// Handler del diálogo de lista de vehículos
stock OnDialogAdminVehicleList(playerid, response, listitem)
{
    if(!response) return 1;
    
    // Recargar la consulta para obtener el ID del vehículo seleccionado
    new targetid = AdminVehicleTarget[playerid];
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT id, model, plate \
        FROM vehicles \
        WHERE owner_type = 'character' AND owner_id = %d \
        ORDER BY id ASC LIMIT %d, 1",
        CharacterData[targetid][cID], listitem);
    
    mysql_tquery(g_MySQL, query, "OnAdminSelectVehicle", "d", playerid);
    return 1;
}

forward OnAdminSelectVehicle(playerid);
public OnAdminSelectVehicle(playerid)
{
    if(cache_num_rows() == 0) return 1;
    
    new vehicleDBID, model, plate[16];
    cache_get_value_name_int(0, "id", vehicleDBID);
    cache_get_value_name_int(0, "model", model);
    cache_get_value_name(0, "plate", plate, 16);
    
    AdminVehicleSelected[playerid] = vehicleDBID;
    
    new dialog[256], title[128];
    format(title, sizeof(title), "%s - Placa: %s", VehicleModelNames[model - 400], plate);
    format(dialog, sizeof(dialog), 
        "Reparar vehiculo\nTeleportarse al vehiculo\nEliminar vehiculo");
    
    ShowPlayerDialog(playerid, DIALOG_ADMIN_VEHICLE_ACTIONS, DIALOG_STYLE_LIST, title, dialog, "Seleccionar", "Cancelar");
    return 1;
}

// Handler del dialogo de acciones
stock OnDialogAdminVehicleActions(playerid, response, listitem)
{
    if(!response) return 1;
    
    new vehicleDBID = AdminVehicleSelected[playerid];
    if(vehicleDBID == -1) return 1;
    
    switch(listitem)
    {
        case 0: // Reparar
        {
            new bool:found = false;
            for(new v = 1; v < MAX_VEHICLES_ENGINE; v++)
            {
                if(GetVehicleDBID(v) == vehicleDBID)
                {
                    SetVehicleHealth(v, 1000.0);
                    RepairVehicle(v);
                    found = true;
                    
                    new string[128];
                    format(string, sizeof(string), "{00FF00}Vehiculo ID %d reparado correctamente.", vehicleDBID);
                    SendClientMessage(playerid, COLOR_SUCCESS, string);
                    
                    printf("[Admin] %s reparo el vehiculo DB ID %d", CharacterData[playerid][cName], vehicleDBID);
                    break;
                }
            }
            
            if(!found)
            {
                SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El vehiculo no esta spawneado actualmente.");
            }
        }
        case 1: // Teleportarse
        {
            new bool:found = false;
            for(new v = 1; v < MAX_VEHICLES_ENGINE; v++)
            {
                if(GetVehicleDBID(v) == vehicleDBID)
                {
                    new Float:x, Float:y, Float:z;
                    GetVehiclePos(v, x, y, z);
                    SetPlayerPos(playerid, x, y + 2.0, z);
                    SetPlayerInterior(playerid, 0);
                    SetPlayerVirtualWorld(playerid, 0);
                    found = true;
                    
                    SendClientMessage(playerid, COLOR_SUCCESS, "{00FF00}Te has teletransportado al vehiculo.");
                    printf("[Admin] %s se teleporto al vehiculo DB ID %d", CharacterData[playerid][cName], vehicleDBID);
                    break;
                }
            }
            
            if(!found)
            {
                SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El vehiculo no esta spawneado actualmente.");
            }
        }
        case 2: // Eliminar
        {
            new dialog[256];
            format(dialog, sizeof(dialog),
                "{FF0000}ESTAS SEGURO?\n\n\
                {FFFFFF}Esta accion eliminara el vehiculo:\n\
                {FFD700}- ID de base de datos: {FFFFFF}%d\n\
                {FFD700}- Del servidor {FFFFFF}(si esta spawneado)\n\
                {FFD700}- De la base de datos {FFFFFF}(permanente)\n\n\
                {FF0000}Esta accion NO SE PUEDE DESHACER.",
                vehicleDBID);
            
            ShowPlayerDialog(playerid, DIALOG_ADMIN_VEHICLE_DELETE_CONFIRM, DIALOG_STYLE_MSGBOX, 
                "{FF0000}CONFIRMAR ELIMINACION", dialog, "SI, ELIMINAR", "Cancelar");
        }
    }
    
    return 1;
}

// Handler del diálogo de confirmación de eliminación
stock OnAdminVehicleDelConfirm(playerid, response)
{
    if(!response) 
    {
        SendClientMessage(playerid, COLOR_INFO, "Eliminación cancelada.");
        return 1;
    }
    
    new vehicleDBID = AdminVehicleSelected[playerid];
    if(vehicleDBID == -1) return 1;
    
    // Buscar y destruir el vehículo del servidor
    new bool:wasSpawned = false;
    for(new v = 1; v < MAX_VEHICLES_ENGINE; v++)
    {
        if(GetVehicleDBID(v) == vehicleDBID)
        {
            DestroyVehicle(v);
            SetVehicleDBID(v, 0);
            wasSpawned = true;
            break;
        }
    }
    
    // Eliminar llaves del inventario
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "DELETE FROM character_inventory \
        WHERE item_id IN (1, 2) AND metadata LIKE '%%vehicleid:%d%%'",
        vehicleDBID);
    mysql_tquery(g_MySQL, query);
    
    // Eliminar de la base de datos
    mysql_format(g_MySQL, query, sizeof(query),
        "DELETE FROM vehicles WHERE id = %d",
        vehicleDBID);
    mysql_tquery(g_MySQL, query);
    
    new string[128];
    format(string, sizeof(string), 
        "{00FF00}Vehiculo ID %d eliminado correctamente.\n{FFFFFF}Spawneado: %s",
        vehicleDBID, wasSpawned ? "{00FF00}Si" : "{FF0000}No");
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    printf("[Admin] %s elimino el vehiculo DB ID %d (spawned: %d)", 
        CharacterData[playerid][cName], vehicleDBID, wasSpawned);
    
    AdminVehicleSelected[playerid] = -1;
    AdminVehicleTarget[playerid] = INVALID_PLAYER_ID;
    
    return 1;
}

// Comando: /buscarvehiculo - Marcar vehiculo en el mapa (usuarios normales)
CMD:buscarvehiculo(playerid, params[])
{
    if(!CharacterData[playerid][cID])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}No has seleccionado un personaje.");
    
    // Consultar vehiculos del jugador
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT id, model, plate, pos_x, pos_y, pos_z, is_spawned \
        FROM vehicles \
        WHERE owner_type = 'character' AND owner_id = %d AND is_spawned = 1 \
        ORDER BY id ASC",
        CharacterData[playerid][cID]);
    
    mysql_tquery(g_MySQL, query, "OnPlayerFindVehicle", "d", playerid);
    return 1;
}

CMD:buscarcarro(playerid, params[])
{
    return cmd_buscarvehiculo(playerid, params);
}

CMD:buscarauto(playerid, params[])
{
    return cmd_buscarvehiculo(playerid, params);
}

forward OnPlayerFindVehicle(playerid);
public OnPlayerFindVehicle(playerid)
{
    new rows = cache_num_rows();
    
    if(rows == 0)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No tienes vehiculos spawneados actualmente.");
        return 1;
    }
    
    new dialog[2048], line[128];
    new id, model, plate[16], Float:x, Float:y, Float:z;
    
    for(new i = 0; i < rows; i++)
    {
        cache_get_value_name_int(i, "id", id);
        cache_get_value_name_int(i, "model", model);
        cache_get_value_name(i, "plate", plate, 16);
        cache_get_value_name_float(i, "pos_x", x);
        cache_get_value_name_float(i, "pos_y", y);
        cache_get_value_name_float(i, "pos_z", z);
        
        format(line, sizeof(line), "%s - Placa: %s\n",
            VehicleModelNames[model - 400],
            plate);
        
        strcat(dialog, line);
    }
    
    new title[128];
    format(title, sizeof(title), "Mis Vehiculos (%d)", rows);
    
    // Guardar para el callback
    AdminVehicleTarget[playerid] = playerid;
    
    ShowPlayerDialog(playerid, DIALOG_PLAYER_FIND_VEHICLE, DIALOG_STYLE_LIST, title, dialog, "Marcar", "Cancelar");
    return 1;
}

forward OnPlayerMarkVehicle(playerid);
public OnPlayerMarkVehicle(playerid)
{
    if(cache_num_rows() == 0) return 1;
    
    new Float:x, Float:y, Float:z, model, plate[16];
    cache_get_value_name_float(0, "pos_x", x);
    cache_get_value_name_float(0, "pos_y", y);
    cache_get_value_name_float(0, "pos_z", z);
    cache_get_value_name_int(0, "model", model);
    cache_get_value_name(0, "plate", plate, 16);
    
    // Marcar en el mapa
    SetPlayerCheckpoint(playerid, x, y, z, 3.0);
    
    new string[128];
    format(string, sizeof(string), 
        "{00FF00}Tu %s (Placa: %s) ha sido marcado en el mapa.",
        VehicleModelNames[model - 400], plate);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    return 1;
}

// Comando para teleportarse a interiores
CMD:interiores(playerid, params[])
{
    printf("[DEBUG /interior] Comando ejecutado por %s (Admin: %d)", CharacterData[playerid][cName], UserData[playerid][uAdminLevel]);
    
    if(!IsHelper(playerid))
    {
        printf("[DEBUG /interior] Sin permisos - AdminLevel: %d", UserData[playerid][uAdminLevel]);
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}[ERROR]{FFFFFF} No tienes permisos para usar este comando.");
        return 1;
    }
    
    printf("[DEBUG /interior] Permisos OK - Mostrando dialog");
    
    new string[2048];
    strcat(string, "24/7 Shop 1\n24/7 Shop 2\n24/7 Shop 3\n24/7 Shop 4\n24/7 Shop 5\n24/7 Shop 6\n");
    strcat(string, "Airport Ticket Desk\nAirport Baggage\nShamal Interior\nAndromada Interior\n");
    strcat(string, "Ammunation 1\nAmmunation 2\nAmmunation 3\nAmmunation 4\nAmmunation 5\n");
    strcat(string, "Blastin Fools Records\nBurger Shot\nCluckin Bell\nWell Stacked Pizza\n");
    strcat(string, "Gym (LS)\nGym (SF)\nGym (LV)\nTattoo (LS)\nTattoo (SF)\nTattoo (LV)\n");
    strcat(string, "Victim\nZip\nBinco\nDidier Sachs\nProlaps\nSuburban\n");
    strcat(string, "Planning Dept\nLSPD\nSFPD\nLVPD\nFBI HQ\nArea 51\n");
    strcat(string, "Caligula's Casino\n4 Dragons Casino\nMad Dogg Mansion\nMadd Dogg Mansion 2\n");
    strcat(string, "Crack Palace\nBig Smoke's Crack Palace\nJeff's House\nRyders House\nSweet's House\n");
    strcat(string, "OG Loc's House\nCJ's House\nDenise's Bedroom\nKatie's Bedroom\nHelena's Bedroom\n");
    strcat(string, "Barbara's Bedroom\nMichelle's Bedroom\nMillie's Bedroom\n");
    
    ShowPlayerDialog(playerid, DIALOG_INTERIORS_LIST, DIALOG_STYLE_LIST, "Teleport a Interiores", string, "Ir", "Cancelar");
    printf("[DEBUG /interior] Dialog mostrado - ID: %d", DIALOG_INTERIORS_LIST);
    return 1;
}

// Comando para cambiar interior ID
CMD:setint(playerid, params[])
{
    if(!IsHelper(playerid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}[ERROR]{FFFFFF} No tienes permisos para usar este comando.");
        return 1;
    }
    
    new interiorid;
    if(sscanf(params, "d", interiorid))
    {
        SendClientMessage(playerid, COLOR_WHITE, "{FFFF00}[USO]{FFFFFF} /setint [interior id]");
        return 1;
    }
    
    SetPlayerInterior(playerid, interiorid);
    
    new string[256];
    format(string, sizeof(string), "{00FF00}[INFO]{FFFFFF} Interior ID cambiado a: %d", interiorid);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    // Consultar si el interior está en uso
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT id, name, type_id, virtual_world FROM properties WHERE interior = %d LIMIT 5",
        interiorid);
    mysql_tquery(g_MySQL, query, "OnCheckInteriorUsage", "dd", playerid, interiorid);
    
    return 1;
}

// Comando para cambiar Virtual World
CMD:setvw(playerid, params[])
{
    if(!IsHelper(playerid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}[ERROR]{FFFFFF} No tienes permisos para usar este comando.");
        return 1;
    }
    
    new virtualworld;
    if(sscanf(params, "d", virtualworld))
    {
        SendClientMessage(playerid, COLOR_WHITE, "{FFFF00}[USO]{FFFFFF} /setvw [virtual world]");
        return 1;
    }
    
    SetPlayerVirtualWorld(playerid, virtualworld);
    
    new string[256];
    format(string, sizeof(string), "{00FF00}[INFO]{FFFFFF} Virtual World cambiado a: %d", virtualworld);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    // Consultar si el virtual world está en uso
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT id, name, type_id, interior FROM properties WHERE virtual_world = %d LIMIT 5",
        virtualworld);
    mysql_tquery(g_MySQL, query, "OnCheckVirtualWorldUsage", "dd", playerid, virtualworld);
    
    return 1;
}

// Comando alias
CMD:setinterior(playerid, params[]) return cmd_setint(playerid, params);
CMD:setworld(playerid, params[]) return cmd_setvw(playerid, params);
CMD:int(playerid, params[]) return cmd_interiores(playerid, params);

// Comando para buscar qué propiedades usan un interior o virtual world
CMD:checkint(playerid, params[])
{
    if(!IsHelper(playerid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}[ERROR]{FFFFFF} No tienes permisos para usar este comando.");
        return 1;
    }
    
    new interiorid;
    if(sscanf(params, "d", interiorid))
    {
        SendClientMessage(playerid, COLOR_WHITE, "{FFFF00}[USO]{FFFFFF} /checkint [interior id] - Verifica qué propiedades usan este interior");
        return 1;
    }
    
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT id, name, type_id, virtual_world FROM properties WHERE interior = %d",
        interiorid);
    mysql_tquery(g_MySQL, query, "OnCheckInteriorUsage", "dd", playerid, interiorid);
    
    return 1;
}

CMD:checkvw(playerid, params[])
{
    if(!IsHelper(playerid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}[ERROR]{FFFFFF} No tienes permisos para usar este comando.");
        return 1;
    }
    
    new virtualworld;
    if(sscanf(params, "d", virtualworld))
    {
        SendClientMessage(playerid, COLOR_WHITE, "{FFFF00}[USO]{FFFFFF} /checkvw [virtual world] - Verifica qué propiedades usan este VW");
        return 1;
    }
    
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "SELECT id, name, type_id, interior FROM properties WHERE virtual_world = %d",
        virtualworld);
    mysql_tquery(g_MySQL, query, "OnCheckVirtualWorldUsage", "dd", playerid, virtualworld);
    
    return 1;
}

// Callbacks para verificar uso de interiores y virtual worlds

forward OnCheckInteriorUsage(playerid, interiorid);
public OnCheckInteriorUsage(playerid, interiorid)
{
    if(!IsPlayerConnected(playerid)) return 0;
    
    new rows = cache_num_rows();
    
    if(rows == 0)
    {
        new string[128];
        format(string, sizeof(string), "{00FF00}[INFO]{FFFFFF} El interior %d no está en uso por ninguna propiedad.", interiorid);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        return 1;
    }
    
    new string[256];
    format(string, sizeof(string), "{FFAA00}[ADVERTENCIA]{FFFFFF} El interior %d está en uso por %d propiedad(es):", interiorid, rows);
    SendClientMessage(playerid, COLOR_YELLOW, string);
    
    for(new i = 0; i < rows; i++)
    {
        new propid, propname[100], typeid, vw;
        cache_get_value_name_int(i, "id", propid);
        cache_get_value_name(i, "name", propname, 100);
        cache_get_value_name_int(i, "type_id", typeid);
        cache_get_value_name_int(i, "virtual_world", vw);
        
        new typename[32];
        switch(typeid)
        {
            case 1: typename = "Casa Pequeña";
            case 2: typename = "Casa Mediana";
            case 3: typename = "Casa Grande";
            case 4: typename = "Mansión";
            case 5: typename = "Tienda";
            case 6: typename = "Empresa";
            default: typename = "Desconocido";
        }
        
        format(string, sizeof(string), "  {AAAAAA}#%d: {FFFFFF}%s {AAAAAA}(%s) - VW: %d", propid, propname, typename, vw);
        SendClientMessage(playerid, COLOR_WHITE, string);
    }
    
    return 1;
}

forward OnCheckVirtualWorldUsage(playerid, virtualworld);
public OnCheckVirtualWorldUsage(playerid, virtualworld)
{
    if(!IsPlayerConnected(playerid)) return 0;
    
    new rows = cache_num_rows();
    
    if(rows == 0)
    {
        new string[128];
        format(string, sizeof(string), "{00FF00}[INFO]{FFFFFF} El virtual world %d no está en uso por ninguna propiedad.", virtualworld);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        return 1;
    }
    
    new string[256];
    format(string, sizeof(string), "{FFAA00}[ADVERTENCIA]{FFFFFF} El virtual world %d está en uso por %d propiedad(es):", virtualworld, rows);
    SendClientMessage(playerid, COLOR_YELLOW, string);
    
    for(new i = 0; i < rows; i++)
    {
        new propid, propname[100], typeid, interior;
        cache_get_value_name_int(i, "id", propid);
        cache_get_value_name(i, "name", propname, 100);
        cache_get_value_name_int(i, "type_id", typeid);
        cache_get_value_name_int(i, "interior", interior);
        
        new typename[32];
        switch(typeid)
        {
            case 1: typename = "Casa Pequeña";
            case 2: typename = "Casa Mediana";
            case 3: typename = "Casa Grande";
            case 4: typename = "Mansión";
            case 5: typename = "Tienda";
            case 6: typename = "Empresa";
            default: typename = "Desconocido";
        }
        
        format(string, sizeof(string), "  {AAAAAA}#%d: {FFFFFF}%s {AAAAAA}(%s) - Int: %d", propid, propname, typename, interior);
        SendClientMessage(playerid, COLOR_WHITE, string);
    }
    
    return 1;
}
// ============================================================================
// COMANDOS DE ARMAS
// ============================================================================

// Comando: /darmarma [playerid] [weapon_id o nombre] [municion]
CMD:darmarma(playerid, params[])
{
    if(!IsAdministrator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}No tienes permisos (Nivel 3 requerido)");
    
    new targetid, weapon_input[64], ammo;
    if(sscanf(params, "us[64]D(0)", targetid, weapon_input, ammo))
    {
        SendClientMessage(playerid, COLOR_INFO, "{FFFF00}Uso: {FFFFFF}/darmarma [playerid] [weapon_id o nombre] [municion]");
        SendClientMessage(playerid, COLOR_INFO, "{FFFFFF}Ejemplos:");
        SendClientMessage(playerid, COLOR_INFO, "  /darmarma 0 Desert Eagle 50");
        SendClientMessage(playerid, COLOR_INFO, "  /darmarma 0 24 50 {AAAAAA}(ID 24 = Desert Eagle)");
        SendClientMessage(playerid, COLOR_INFO, "  /darmarma 0 AK-47 100");
        return 1;
    }
    
    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Jugador no conectado");
    
    if(!CharacterData[targetid][cID])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}El jugador no tiene un personaje seleccionado");
    
    // Buscar arma por ID o nombre
    new query[512];
    if(strval(weapon_input) > 0) // Si es un número
    {
        mysql_format(g_MySQL, query, sizeof(query),
            "SELECT id, name, weapon_id, weapon_ammo_capacity \
            FROM items \
            WHERE weapon_id = %d AND is_weapon = 1 \
            LIMIT 1",
            strval(weapon_input));
    }
    else // Si es un nombre
    {
        mysql_format(g_MySQL, query, sizeof(query),
            "SELECT id, name, weapon_id, weapon_ammo_capacity \
            FROM items \
            WHERE name LIKE '%%%s%%' AND is_weapon = 1 \
            LIMIT 1",
            weapon_input);
    }
    
    new tmpData[128];
    format(tmpData, sizeof(tmpData), "%d|%d", targetid, ammo);
    SetPVarString(playerid, "GiveWeaponData", tmpData);
    
    mysql_tquery(g_MySQL, query, "OnAdminGiveWeapon", "d", playerid);
    return 1;
}

forward OnAdminGiveWeapon(adminid);
public OnAdminGiveWeapon(adminid)
{
    if(cache_num_rows() == 0)
    {
        SendClientMessage(adminid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Arma no encontrada en la base de datos");
        SendClientMessage(adminid, COLOR_INFO, "{FFFFFF}Lista de algunas armas:");
        SendClientMessage(adminid, COLOR_INFO, "  Colt 45 (ID:22), Silenced (23), Desert Eagle (24)");
        SendClientMessage(adminid, COLOR_INFO, "  Shotgun (25), Combat Shotgun (27)");
        SendClientMessage(adminid, COLOR_INFO, "  Micro SMG (28), MP5 (29), AK-47 (30), M4 (31)");
        SendClientMessage(adminid, COLOR_INFO, "  Sniper Rifle (34), RPG (35)");
        DeletePVar(adminid, "GiveWeaponData");
        return 1;
    }
    
    new tmpData[128];
    GetPVarString(adminid, "GiveWeaponData", tmpData, sizeof(tmpData));
    DeletePVar(adminid, "GiveWeaponData");
    
    new targetid, ammo_given;
    sscanf(tmpData, "p<|>dd", targetid, ammo_given);
    
    new itemid, weaponid, capacity;
    new weaponName[64];
    
    cache_get_value_name_int(0, "id", itemid);
    cache_get_value_name_int(0, "weapon_id", weaponid);
    cache_get_value_name_int(0, "weapon_ammo_capacity", capacity);
    cache_get_value_name(0, "name", weaponName, 64);
    
    // Limitar munición a la capacidad máxima si no se especificó
    if(ammo_given == 0) ammo_given = capacity;
    if(ammo_given > capacity) ammo_given = capacity;
    
    // Dar el arma al inventario con munición
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO character_inventory (character_id, item_id, quantity, current_ammo) \
        VALUES (%d, %d, 1, %d) \
        ON DUPLICATE KEY UPDATE quantity = quantity + 1",
        CharacterData[targetid][cID], itemid, ammo_given);
    mysql_tquery(g_MySQL, query);
    
    // Recargar inventario del jugador
    LoadPlayerInventory(targetid);
    
    // Mensajes
    new string[256];
    format(string, sizeof(string), "{00FF00}Has dado {FFFFFF}%s {AAAAAA}(%d balas) {00FF00}a {FFFFFF}%s", 
        weaponName, ammo_given, CharacterData[targetid][cName]);
    SendClientMessage(adminid, COLOR_SUCCESS, string);
    
    format(string, sizeof(string), "{00FF00}El admin {FFFFFF}%s {00FF00}te ha dado un arma: {FFFFFF}%s {AAAAAA}(%d balas)", 
        CharacterData[adminid][cName], weaponName, ammo_given);
    SendClientMessage(targetid, COLOR_SUCCESS, string);
    SendClientMessage(targetid, COLOR_INFO, "{FFFFFF}Usa {FFFF00}/inventario {FFFFFF}para verla y {FFFF00}/equipar [slot] {FFFFFF}para usarla");
    
    printf("[Admin Weapon] %s dio %s (%d balas) a %s", 
        CharacterData[adminid][cName], weaponName, ammo_given, CharacterData[targetid][cName]);
    
    return 1;
}