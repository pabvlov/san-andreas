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

// Sistema de dudas
#define MAX_DUDAS 50
#define DIALOG_DUDA_LIST 2100
#define DUDA_TIMEOUT 180 // 3 minutos en segundos

enum E_DUDA_DATA
{
    dudaPlayerID,
    dudaAdminID,
    dudaTexto[128],
    dudaActiva,
    dudaUltimaInteraccion,
    dudaTimer
}

new DudaData[MAX_DUDAS][E_DUDA_DATA];
new PlayerDudaID[MAX_PLAYERS] = {-1, ...};
new AdminDudaID[MAX_PLAYERS] = {-1, ...};
new bool:PlayerInDudaChat[MAX_PLAYERS] = {false, ...}; // Bloquear chat normal durante duda

// Variables AFK
new bool:PlayerInFreecam[MAX_PLAYERS];
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
        strcat(string, "/interior (/int) - Teleport a interiores\n");
        strcat(string, "/setint [id] - Cambiar interior ID\n");
        strcat(string, "/setvw [id] - Cambiar virtual world\n");
        strcat(string, "/checkint [id] - Ver propiedades en interior\n");
        strcat(string, "/checkvw [id] - Ver propiedades en VW\n");
        strcat(string, "/verdudas - Ver panel de dudas pendientes\n");
        strcat(string, "/cerrarduda - Cerrar sesión de duda activa\n\n");
    }
    
    if(IsModerator(playerid))
    {
        strcat(string, "{FFD700}[Nivel 2 - Moderador]{FFFFFF}\n");
        strcat(string, "/panel [playerid] - Gestión completa de jugadores\n");
        strcat(string, "{AAAAAA}  (Ir, Traer, Mutear, Freeze, Warn, Jail, Kick, Ban, Espectar)\n");
        strcat(string, "/vehiculos [playerid] - Gestionar vehiculos\n");
        strcat(string, "/dar [playerid] - Dar items/beneficios (Vida, Armadura, Dinero, Jetpack, Skins, Autos)\n");
        strcat(string, "/pm [ID] [mensaje] - Mensaje privado (solo admins)\n\n");
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
        strcat(string, "/setadmin [playerid] [nivel] - Otorgar rango de admin\n");
    }
    
    ShowPlayerDialog(playerid, 0, DIALOG_STYLE_MSGBOX, "Comandos de Administración", string, "Cerrar", "");
    return 1;
}

// Comando de ayuda general con controles
CMD:controles(playerid, params[])
{
    new string[1500];
    strcat(string, "{FFFFFF}=== {00FF00}Controles del Servidor{FFFFFF} ===\n\n");
    strcat(string, "{FFD700}[Teclas de Juego]\n");
    strcat(string, "{FFFFFF}Y - Abrir inventario\n");
    strcat(string, "{FFFFFF}N - Encender/Apagar motor del vehículo\n");
    strcat(string, "{FFFFFF}H - Entrar/Salir de propiedades\n\n");
    strcat(string, "{FFD700}[Comandos de Chat Roleplay]\n");
    strcat(string, "{C2A2DA}/me [acción]{FFFFFF} - Acción en primera persona\n");
    strcat(string, "{FFFFE0}/do [descripción]{FFFFFF} - Descripción del entorno\n");
    strcat(string, "{FFFFFF}/intentar [acción]{FFFFFF} - Acción con éxito/fracaso aleatorio\n");
    strcat(string, "{999999}/b [mensaje]{FFFFFF} - Chat Out Of Character (OOC)\n");
    strcat(string, "{FFFFFF}/g [mensaje]{FFFFFF} - Gritar (40m de alcance)\n");
    strcat(string, "{FFFFFF}/s [mensaje]{FFFFFF} - Susurrar (5m de alcance)\n\n");
    strcat(string, "{FFD700}[Comandos Básicos]\n");
    strcat(string, "{FFFFFF}/inventario - Ver tu inventario\n");
    strcat(string, "{FFFFFF}/motor - Alternar motor (igual que tecla N)\n");
    strcat(string, "{FFFFFF}/entrar o /salir - Entrar/salir (igual que H)\n");
    strcat(string, "{FFFFFF}/buscarvehiculo - Encontrar tus vehiculos\n");
    strcat(string, "{FFFFFF}/vehiculos - Ver lista de vehiculos\n");
    strcat(string, "{FFFFFF}/stats - Ver tus estadísticas\n");
    strcat(string, "{FFFFFF}/afk - Activar/desactivar modo AFK\n");
    strcat(string, "{FFFFFF}/duda [mensaje] - Enviar duda a administración\n\n");
    strcat(string, "{FFD700}[Información]\n");
    strcat(string, "{FFFFFF}/controles o /ayuda - Ver esta ayuda\n");
    strcat(string, "{AAAAAA}Usa {FFFF00}/acmds{AAAAAA} si eres administrador.\n");
    strcat(string, "{AAAAAA}El chat tiene {FFFF00}20m de alcance{AAAAAA} con gradiente de color.\n");
    
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

// Comando: /setadmin - Otorgar rango de administrador
CMD:setadmin(playerid, params[])
{
    if(!IsOwner(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Necesitas ser Dueño nivel 4.");
    
    if(!UserData[playerid][uLogged])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Debes estar logueado.");
    
    new targetid, adminlevel;
    
    // Verificar si se proporcionaron parámetros
    if(sscanf(params, "ud", targetid, adminlevel))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Uso: {FFFFFF}/setadmin [playerid] [nivel]");
        SendClientMessage(playerid, COLOR_INFO, "{33CCFF}Niveles: {FFFFFF}0=Jugador, 1=Ayudante, 2=Moderador, 3=Admin, 4=Dueño");
        SendClientMessage(playerid, COLOR_INFO, "{33CCFF}Ejemplo: {FFFFFF}/setadmin 0 2 (otorgar moderador)");
        return 1;
    }
    
    // Verificar si el jugador objetivo está conectado
    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Jugador no conectado.");
    
    // Verificar que el jugador objetivo esté logueado
    if(!UserData[targetid][uLogged])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}El jugador no está logueado.");
    
    // Verificar que el nivel de admin sea válido (0-4)
    if(adminlevel < 0 || adminlevel > 4)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Nivel de admin inválido. Rango permitido: 0-4");
        return 1;
    }
    
    // No permitir que se den el mismo rango a sí mismos si ya lo tienen
    if(targetid == playerid && adminlevel == UserData[playerid][uAdminLevel])
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Ya tienes ese nivel de administrador.");
        return 1;
    }
    
    // Guardar el nivel anterior
    new oldLevel = UserData[targetid][uAdminLevel];
    
    // Cambiar el nivel de admin
    UserData[targetid][uAdminLevel] = adminlevel;
    
    // Guardar en base de datos
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "UPDATE users SET admin_level = %d WHERE id = %d",
        adminlevel, UserData[targetid][uID]);
    mysql_tquery(g_MySQL, query);
    
    // Nombres de los niveles para los mensajes
    new oldLevelName[32], newLevelName[32];
    switch(oldLevel)
    {
        case 0: oldLevelName = "Jugador";
        case 1: oldLevelName = "Ayudante";
        case 2: oldLevelName = "Moderador";
        case 3: oldLevelName = "Administrador";
        case 4: oldLevelName = "Dueño";
        default: oldLevelName = "Desconocido";
    }
    
    switch(adminlevel)
    {
        case 0: newLevelName = "Jugador";
        case 1: newLevelName = "Ayudante";
        case 2: newLevelName = "Moderador";
        case 3: newLevelName = "Administrador";
        case 4: newLevelName = "Dueño";
        default: newLevelName = "Desconocido";
    }
    
    // Mensajes de confirmación
    new string[200];
    if(targetid == playerid)
    {
        format(string, sizeof(string), 
            "{00FF00}Has cambiado tu nivel de admin de {FFFFFF}%s (%d) {00FF00}a {FFFFFF}%s (%d)", 
            oldLevelName, oldLevel, newLevelName, adminlevel);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
    }
    else
    {
        format(string, sizeof(string), 
            "{00FF00}Has cambiado el nivel de admin de {FFFFFF}%s {00FF00}de {FFFFFF}%s (%d) {00FF00}a {FFFFFF}%s (%d)", 
            UserData[targetid][uNickname], oldLevelName, oldLevel, newLevelName, adminlevel);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        
        format(string, sizeof(string), 
            "{FFD700}Admin {FFFFFF}%s {FFD700}ha cambiado tu nivel de admin de {FFFFFF}%s (%d) {FFD700}a {FFFFFF}%s (%d)", 
            UserData[playerid][uNickname], oldLevelName, oldLevel, newLevelName, adminlevel);
        SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
        
        // Notificar a todos los admins
        format(string, sizeof(string), 
            "{FFD700}%s ha cambiado el nivel de admin de %s a %s (%d)",
            UserData[playerid][uNickname], UserData[targetid][uNickname], newLevelName, adminlevel);
        SendAdminMessage(COLOR_ADMIN_MSG, string);
    }
    
    // Log en consola
    printf("[Admin] %s cambio el nivel de admin de %s de %d (%s) a %d (%s)", 
        UserData[playerid][uNickname], UserData[targetid][uNickname], 
        oldLevel, oldLevelName, adminlevel, newLevelName);
    
    return 1;
}

// Variables temporales para gestión de vehículos
new AdminVehicleTarget[MAX_PLAYERS] = {INVALID_PLAYER_ID, ...};
new AdminVehicleSelected[MAX_PLAYERS] = {-1, ...};

// Variables para sistema de administración
new AdminPanelTarget[MAX_PLAYERS] = {INVALID_PLAYER_ID, ...};
new AdminDarTarget[MAX_PLAYERS] = {INVALID_PLAYER_ID, ...};
new PlayerWarnings[MAX_PLAYERS] = {0, ...}; // Sistema de advertencias
new bool:PlayerFrozen[MAX_PLAYERS] = {false, ...}; // Estado de freeze

// Comando: /panel - Panel de gestión de jugadores
CMD:panel(playerid, params[])
{
    if(!IsModerator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Necesitas ser Moderador nivel 2.");
    
    new targetid;
    if(sscanf(params, "u", targetid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Uso: {FFFFFF}/panel [playerid]");
    
    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Jugador no conectado.");
    
    if(!CharacterData[targetid][cID])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}El jugador no tiene personaje seleccionado.");
    
    // Guardar target
    AdminPanelTarget[playerid] = targetid;
    
    // Mostrar panel
    new dialog[512], title[128];
    format(title, sizeof(title), "Panel de Administración - %s", CharacterData[targetid][cName]);
    format(dialog, sizeof(dialog),
        "Ir\n\
        Traer\n\
        Mutear\n\
        Freeze\n\
        Advertencia\n\
        Encarcelar\n\
        Expulsar\n\
        Banear\n\
        Espectar\n\
        Estadísticas\n\
        Inventario");
    
    ShowPlayerDialog(playerid, DIALOG_ADMIN_PANEL, DIALOG_STYLE_LIST, title, dialog, "Seleccionar", "Cancelar");
    return 1;
}

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
        format(string, sizeof(string), "{FF0000}%s no tiene vehiculos registrados.", CharacterData[targetid][cName]);
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
    format(title, sizeof(title), "Vehiculos de %s (%d vehiculos)", CharacterData[targetid][cName], rows);
    
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
        "Reparar vehiculo\n\
        Teleportarse al vehiculo\n\
        Respawn vehiculo\n\
        Rellenar gasolina\n\
        Eliminar vehiculo");
    
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
        case 2: // Respawn
        {
            new bool:found = false;
            for(new v = 1; v < MAX_VEHICLES_ENGINE; v++)
            {
                if(GetVehicleDBID(v) == vehicleDBID)
                {
                    SetVehicleToRespawn(v);
                    found = true;
                    
                    new string[128];
                    format(string, sizeof(string), "{00FF00}Vehiculo ID %d respawneado correctamente.", vehicleDBID);
                    SendClientMessage(playerid, COLOR_SUCCESS, string);
                    
                    printf("[Admin] %s respawneo el vehiculo DB ID %d", CharacterData[playerid][cName], vehicleDBID);
                    break;
                }
            }
            
            if(!found)
            {
                SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El vehiculo no esta spawneado actualmente.");
            }
        }
        case 3: // Rellenar gasolina
        {
            // TODO: Implementar cuando el sistema de gasolina esté listo
            SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Sistema de gasolina en desarrollo.");
            printf("[Admin] %s intento rellenar gasolina del vehiculo DB ID %d (sistema no disponible)", 
                CharacterData[playerid][cName], vehicleDBID);
        }
        case 4: // Eliminar
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

// ==================== NUEVOS COMANDOS DE ADMINISTRACIÓN ====================

// Comando: /dar - Dar items/beneficios a un jugador
CMD:dar(playerid, params[])
{
    if(!IsModerator(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Necesitas ser Moderador nivel 2.");
    
    new targetid;
    if(sscanf(params, "u", targetid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Uso: {FFFFFF}/dar [playerid]");
    
    if(!IsPlayerConnected(targetid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Jugador no conectado.");
    
    if(!CharacterData[targetid][cID])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}El jugador no tiene personaje seleccionado.");
    
    // Guardar target
    AdminDarTarget[playerid] = targetid;
    
    // Mostrar menú
    new dialog[512], title[128];
    format(title, sizeof(title), "Dar Items - %s", CharacterData[targetid][cName]);
    format(dialog, sizeof(dialog),
        "Vida\n\
        Armadura\n\
        Dinero\n\
        Jetpack\n\
        Skins\n\
        Autos\n\
        Armas\n\
        Cargadores\n\
        Necesidades");
    
    ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR, DIALOG_STYLE_LIST, title, dialog, "Seleccionar", "Cancelar");
    return 1;
}

// Comando: /afk - Activar/desactivar modo AFK (cámara libre)
CMD:afk(playerid, params[])
{
    if(!CharacterData[playerid][cID])
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}No has seleccionado un personaje.");
    
    if(!PlayerInFreecam[playerid])
    {
        // Activar AFK
        TogglePlayerSpectating(playerid, true);
        PlayerInFreecam[playerid] = true;
        
        SendClientMessage(playerid, COLOR_SUCCESS, "{00FF00}Modo AFK activado.");
        SendClientMessage(playerid, COLOR_INFO, "{33CCFF}Usa {FFFFFF}/afk {33CCFF}nuevamente para desactivar.");
        
        new string[128];
        format(string, sizeof(string), "* %s está ausente (AFK)", CharacterData[playerid][cName]);
        SendClientMessageToAll(COLOR_GRAY, string);
    }
    else
    {
        // Desactivar AFK
        TogglePlayerSpectating(playerid, false);
        PlayerInFreecam[playerid] = false;
        
        SpawnPlayer(playerid);
        
        SendClientMessage(playerid, COLOR_SUCCESS, "{00FF00}Modo AFK desactivado.");
        
        new string[128];
        format(string, sizeof(string), "* %s ha regresado", CharacterData[playerid][cName]);
        SendClientMessageToAll(COLOR_GRAY, string);
    }
    
    return 1;
}

// ============================================================================
// SISTEMA DE DUDAS
// ============================================================================

// Función para obtener un slot de duda libre
stock GetFreeDudaSlot()
{
    for(new i = 0; i < MAX_DUDAS; i++)
    {
        if(!DudaData[i][dudaActiva])
            return i;
    }
    return -1;
}

// Timer callback para cerrar dudas inactivas
forward CheckDudaTimeout(dudaid);
public CheckDudaTimeout(dudaid)
{
    if(dudaid < 0 || dudaid >= MAX_DUDAS) return;
    if(!DudaData[dudaid][dudaActiva]) return;
    
    new currentTime = gettime();
    new timeDiff = currentTime - DudaData[dudaid][dudaUltimaInteraccion];
    
    // Si han pasado 3 minutos (180 segundos) sin interacción
    if(timeDiff >= DUDA_TIMEOUT)
    {
        new playerid = DudaData[dudaid][dudaPlayerID];
        new adminid = DudaData[dudaid][dudaAdminID];
        
        if(IsPlayerConnected(playerid))
        {
            SendClientMessage(playerid, COLOR_INFO, "{FF6347}[STAFF] {FFFFFF}La sesión de ayuda ha finalizado por inactividad (3 minutos).");
            PlayerDudaID[playerid] = -1;
            PlayerInDudaChat[playerid] = false;
        }
        
        if(adminid != -1 && IsPlayerConnected(adminid))
        {
            SendClientMessage(adminid, COLOR_ADMIN_MSG, "{FF6347}[STAFF] {FFFFFF}La sesión de ayuda ha finalizado por inactividad (3 minutos).");
            AdminDudaID[adminid] = -1;
            PlayerInDudaChat[adminid] = false;
        }
        
        // Limpiar datos
        DudaData[dudaid][dudaActiva] = 0;
        DudaData[dudaid][dudaPlayerID] = -1;
        DudaData[dudaid][dudaAdminID] = -1;
        DudaData[dudaid][dudaTexto][0] = 0;
        
        // Matar el timer
        KillTimer(DudaData[dudaid][dudaTimer]);
        DudaData[dudaid][dudaTimer] = 0;
    }
}

// Comando /duda
CMD:duda(playerid, params[])
{
    if(!CharacterData[playerid][cSelected])
        return SendClientMessage(playerid, COLOR_RED, "Debes seleccionar un personaje primero!");
    
    if(isnull(params))
        return SendClientMessage(playerid, COLOR_GRAY, "Uso: /duda [tu pregunta o duda]");
    
    // Verificar si ya tiene una duda activa
    if(PlayerDudaID[playerid] != -1)
    {
        new dudaid = PlayerDudaID[playerid];
        if(DudaData[dudaid][dudaAdminID] != -1)
        {
            // Ya está en comunicación con un admin
            return SendClientMessage(playerid, COLOR_INFO, "{FF6347}[STAFF] {FFFFFF}Ya tienes una sesión activa con un administrador.");
        }
        else
        {
            return SendClientMessage(playerid, COLOR_INFO, "{FF6347}[STAFF] {FFFFFF}Tu duda ya fue enviada. Espera a que un administrador la acepte.");
        }
    }
    
    // Crear nueva duda
    new dudaid = GetFreeDudaSlot();
    if(dudaid == -1)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}El sistema de dudas está lleno. Intenta más tarde.");
    
    // Registrar duda
    DudaData[dudaid][dudaPlayerID] = playerid;
    DudaData[dudaid][dudaAdminID] = -1;
    DudaData[dudaid][dudaActiva] = 1;
    DudaData[dudaid][dudaUltimaInteraccion] = gettime();
    format(DudaData[dudaid][dudaTexto], 128, "%s", params);
    
    PlayerDudaID[playerid] = dudaid;
    
    // Notificar al jugador
    SendClientMessage(playerid, COLOR_SUCCESS, "{00FF00}[STAFF] {FFFFFF}Tu duda ha sido enviada al staff.");
    SendClientMessage(playerid, COLOR_INFO, "{33CCFF}Espera a que un administrador la acepte en el panel de dudas.");
    
    // Notificar a todos los administradores
    new string[128];
    format(string, sizeof(string), 
        "{FF6347}[NUEVA DUDA] {FFFFFF}%s ha enviado una duda. Usa {FFFF00}/verdudas {FFFFFF}para verla.",
        CharacterData[playerid][cName]);
    
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        if(IsPlayerConnected(i) && IsHelper(i))
        {
            SendClientMessage(i, COLOR_ADMIN_MSG, string);
        }
    }
    
    return 1;
}

// Comando /verdudas - Panel de dudas para administradores
CMD:verdudas(playerid, params[])
{
    if(!IsHelper(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}No tienes permisos para usar este comando.");
    
    // Verificar si el admin ya tiene una duda activa
    if(AdminDudaID[playerid] != -1)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}Ya estás atendiendo una duda. Ciérrala primero con /cerrarduda.");
    
    // Construir lista de dudas pendientes
    new string[2048], dudaCount = 0;
    strcat(string, "{FFFFFF}ID\t{FFFFFF}Jugador\t{FFFFFF}Duda\n");
    
    for(new i = 0; i < MAX_DUDAS; i++)
    {
        if(DudaData[i][dudaActiva] && DudaData[i][dudaAdminID] == -1)
        {
            new targetid = DudaData[i][dudaPlayerID];
            if(IsPlayerConnected(targetid))
            {
                new tempStr[256];
                format(tempStr, sizeof(tempStr), "{FFFF00}%d\t{FFFFFF}%s (ID:%d)\t{AAAAAA}%s\n", 
                    i, CharacterData[targetid][cName], targetid, DudaData[i][dudaTexto]);
                strcat(string, tempStr);
                dudaCount++;
            }
            else
            {
                // Limpiar duda si el jugador se desconectó
                DudaData[i][dudaActiva] = 0;
                PlayerDudaID[targetid] = -1;
            }
        }
    }
    
    if(dudaCount == 0)
    {
        SendClientMessage(playerid, COLOR_INFO, "{33CCFF}[STAFF] {FFFFFF}No hay dudas pendientes en este momento.");
        return 1;
    }
    
    ShowPlayerDialog(playerid, DIALOG_DUDA_LIST, DIALOG_STYLE_TABLIST_HEADERS, 
        "{FF6347}Panel de Dudas - Selecciona una para responder", string, "Responder", "Cerrar");
    
    return 1;
}

// Callback para el diálogo de dudas
stock OnDialogDudaList(playerid, response, listitem)
{
    if(!response) return 1;
    
    // Buscar la duda seleccionada
    new currentItem = 0;
    for(new i = 0; i < MAX_DUDAS; i++)
    {
        if(DudaData[i][dudaActiva] && DudaData[i][dudaAdminID] == -1)
        {
            new targetid = DudaData[i][dudaPlayerID];
            if(IsPlayerConnected(targetid))
            {
                if(currentItem == listitem)
                {
                    // Asignar admin a la duda
                    DudaData[i][dudaAdminID] = playerid;
                    DudaData[i][dudaUltimaInteraccion] = gettime();
                    AdminDudaID[playerid] = i;
                    PlayerInDudaChat[playerid] = true;
                    PlayerInDudaChat[targetid] = true;
                    
                    // Iniciar timer de 3 minutos
                    DudaData[i][dudaTimer] = SetTimerEx("CheckDudaTimeout", 60000, true, "d", i);
                    
                    // Notificar al jugador
                    new string[256];
                    format(string, sizeof(string), 
                        "{00FF00}[STAFF] {FFFFFF}El administrador {FFFF00}%s {FFFFFF}está atendiendo tu duda.",
                        CharacterData[playerid][cName]);
                    SendClientMessage(targetid, COLOR_SUCCESS, string);
                    SendClientMessage(targetid, COLOR_INFO, "{33CCFF}Escribe en el chat para comunicarte. No podrás usar otros chats hasta que se cierre la sesión.");
                    
                    // Notificar al admin
                    format(string, sizeof(string),
                        "{00FF00}[STAFF] {FFFFFF}Has aceptado la duda de {FFFF00}%s",
                        CharacterData[targetid][cName]);
                    SendClientMessage(playerid, COLOR_SUCCESS, string);
                    SendClientMessage(playerid, COLOR_ADMIN_MSG, "{FF6347}[DUDA] {FFFFFF}Usa {FFFF00}/cerrarduda {FFFFFF}para finalizar la sesión cuando termines.");
                    
                    // Mostrar la duda original
                    format(string, sizeof(string),
                        "{FF6347}[DUDA ORIGINAL] {FFFFFF}%s", DudaData[i][dudaTexto]);
                    SendClientMessage(playerid, COLOR_ADMIN_MSG, string);
                    SendClientMessage(playerid, COLOR_INFO, "{33CCFF}Escribe en el chat para responder. No podrás usar otros chats hasta cerrar la sesión.");
                    
                    return 1;
                }
                currentItem++;
            }
        }
    }
    
    return 1;
}

// Comando /cerrarduda
CMD:cerrarduda(playerid, params[])
{
    if(!IsHelper(playerid))
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}No tienes permisos para usar este comando.");
    
    if(AdminDudaID[playerid] == -1)
        return SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error: {FFFFFF}No estás atendiendo ninguna duda actualmente.");
    
    new dudaid = AdminDudaID[playerid];
    new targetid = DudaData[dudaid][dudaPlayerID];
    
    // Notificar al jugador
    if(IsPlayerConnected(targetid))
    {
        SendClientMessage(targetid, COLOR_SUCCESS, "{00FF00}[STAFF] {FFFFFF}La sesión de ayuda ha finalizado. Gracias por tu consulta.");
        PlayerDudaID[targetid] = -1;
        PlayerInDudaChat[targetid] = false;
        
        // Mensaje visible en el chat
        new string[128];
        format(string, sizeof(string), "* El administrador %s ha cerrado la sesión de ayuda con %s", 
            CharacterData[playerid][cName], CharacterData[targetid][cName]);
        SendClientMessageToAll(COLOR_GRAY, string);
    }
    
    // Limpiar datos
    DudaData[dudaid][dudaActiva] = 0;
    DudaData[dudaid][dudaAdminID] = -1;
    AdminDudaID[playerid] = -1;
    PlayerInDudaChat[playerid] = false;
    KillTimer(DudaData[dudaid][dudaTimer]);
    
    // Notificar al jugador con mensaje visible
    if(IsPlayerConnected(targetid))
    {
        new string[144];
        format(string, sizeof(string), "* El administrador %s ha cerrado la sesión de ayuda.", CharacterData[playerid][cName]);
        SendClientMessage(targetid, COLOR_SUCCESS, string);
        SendClientMessage(targetid, COLOR_INFO, "{33CCFF}Gracias por tu consulta. Puedes enviar otra duda con /duda cuando lo necesites.");
        PlayerDudaID[targetid] = -1;
        PlayerInDudaChat[targetid] = false;
    }
    
    // Mensaje visible para el admin
    new msgString[144];
    format(msgString, sizeof(msgString), "* %s ha cerrado la sesión de ayuda con %s.", CharacterData[playerid][cName], CharacterData[targetid][cName]);
    SendClientMessage(playerid, COLOR_SUCCESS, msgString);
    
    // Limpiar datos
    DudaData[dudaid][dudaActiva] = 0;
    DudaData[dudaid][dudaAdminID] = -1;
    AdminDudaID[playerid] = -1;
    PlayerInDudaChat[playerid] = false;
    
    return 1;
}
