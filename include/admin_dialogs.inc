/*
    Handlers de diálogos para el sistema de administración
*/

#if defined _admin_dialogs_included
    #endinput
#endif
#define _admin_dialogs_included

// Variables globales para búsqueda de vehículos
new AdminVehicleSearchResults[MAX_PLAYERS][50]; // Guardar índices de vehículos encontrados
new AdminVehiclePage[MAX_PLAYERS]; // Página actual del listado de vehículos

#define VEHICLES_PER_PAGE 20 // Vehículos por página para evitar stack overflow

// Función para mostrar una página del listado de vehículos
stock ShowAdminVehicleListPage(playerid)
{
    new page = AdminVehiclePage[playerid];
    new totalVehicles = sizeof(VehicleModelNames);
    new totalPages = (totalVehicles / VEHICLES_PER_PAGE) + ((totalVehicles % VEHICLES_PER_PAGE) ? 1 : 0);
    
    // Validar página
    if(page < 0) page = 0;
    if(page >= totalPages) page = totalPages - 1;
    AdminVehiclePage[playerid] = page;
    
    printf("[DEBUG ShowVehiclePage] Player:%d Pagina:%d/%d TotalVehicles:%d", playerid, page + 1, totalPages, totalVehicles);
    
    new vehiclelist[1024];
    new startIdx = page * VEHICLES_PER_PAGE;
    new endIdx = startIdx + VEHICLES_PER_PAGE;
    if(endIdx > totalVehicles) endIdx = totalVehicles;
    
    // Agregar opción de búsqueda al inicio
    strcat(vehiclelist, "{FFFF00}» Buscar por nombre\n");
    
    // Agregar vehículos de esta página
    for(new i = startIdx; i < endIdx; i++)
    {
        format(vehiclelist, sizeof(vehiclelist), "%s%s\n", vehiclelist, VehicleModelNames[i]);
    }
    
    // Agregar navegación
    if(page > 0)
        strcat(vehiclelist, "{00FF00}« Página Anterior\n");
    if(page < totalPages - 1)
        strcat(vehiclelist, "{00FF00}Página Siguiente »\n");
    
    new title[64];
    format(title, sizeof(title), "Dar Vehículo - Página %d/%d", page + 1, totalPages);
    
    printf("[DEBUG ShowVehiclePage] Mostrando dialogo - Items en pagina: %d", endIdx - startIdx);
    ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR_VEHICLE, DIALOG_STYLE_LIST, title, vehiclelist, "Dar", "Cancelar");
}

// Función helper para dar un vehículo permanente a un jugador
stock AdminGiveVehicle(playerid, targetid, model)
{
    printf("[DEBUG AdminGiveVehicle] INICIO - Admin:%d Target:%d Model:%d", playerid, targetid, model);
    
    // Generar placa única
    new plate[9];
    format(plate, sizeof(plate), "ADM%d%d", random(999), CharacterData[targetid][cID]);
    printf("[DEBUG AdminGiveVehicle] Placa generada: %s", plate);
    
    // Posición del vehículo cerca del jugador objetivo
    new Float:x, Float:y, Float:z, Float:angle;
    GetPlayerPos(targetid, x, y, z);
    GetPlayerFacingAngle(targetid, angle);
    x += 3.0;
    printf("[DEBUG AdminGiveVehicle] Posicion: %.2f, %.2f, %.2f, Angulo: %.2f", x, y, z, angle);
    
    // Crear vehículo en la BD
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO `vehicles` (`model`, `owner_type`, `owner_id`, `plate`, `pos_x`, `pos_y`, `pos_z`, `pos_a`, `fiscal_price`, `is_spawned`) \
        VALUES (%d, 'character', %d, '%e', %.4f, %.4f, %.4f, %.4f, 0, 1)",
        model, CharacterData[targetid][cID], plate, x, y, z, angle);
    
    printf("[DEBUG AdminGiveVehicle] Ejecutando query MySQL: %s", query);
    
    new Cache:result = mysql_query(g_MySQL, query);
    new vehicleID = cache_insert_id();
    cache_delete(result);
    
    printf("[DEBUG AdminGiveVehicle] VehicleID insertado en BD: %d", vehicleID);
    
    if(vehicleID > 0)
    {
        printf("[DEBUG AdminGiveVehicle] VehicleID valido, procediendo a crear vehiculo");
        
        // Spawnear vehículo y guardar ID en memoria
        new ingameVehicleID = CreateVehicle(model, x, y, z, angle, -1, -1, 300000);
        printf("[DEBUG AdminGiveVehicle] CreateVehicle retorno ID: %d", ingameVehicleID);
        
        SetVehicleDBID(ingameVehicleID, vehicleID);
        printf("[DEBUG AdminGiveVehicle] SetVehicleDBID llamado: ingame=%d db=%d", ingameVehicleID, vehicleID);
        
        printf("[Admin] Vehículo spawneado - ingame ID: %d, DB ID: %d", ingameVehicleID, vehicleID);
        
        // Dar 2 llaves del vehículo: 1 maestra (ID 1) + 1 normal (ID 2)
        new metadata[128];
        format(metadata, sizeof(metadata), "vehicleid:%d|plate:%s", vehicleID, plate);
        printf("[DEBUG AdminGiveVehicle] Metadata de llaves: %s", metadata);
        
        // Llave maestra (item ID 1)
        printf("[DEBUG AdminGiveVehicle] Dando llave maestra (item ID 1)...");
        new keyMasterResult = GivePlayerItem(targetid, 1, 1, metadata);
        printf("[DEBUG AdminGiveVehicle] Resultado llave maestra: %d", keyMasterResult);
        
        // Llave normal (item ID 2)
        printf("[DEBUG AdminGiveVehicle] Dando llave normal (item ID 2)...");
        new keyNormalResult = GivePlayerItem(targetid, 2, 1, metadata);
        printf("[DEBUG AdminGiveVehicle] Resultado llave normal: %d", keyNormalResult);
        
        if(keyMasterResult && keyNormalResult)
        {
            printf("[Admin] 2 llaves dadas correctamente a %s (placa: %s)", 
                CharacterData[targetid][cName], plate);
            
            // Refrescar inventario para mostrar las llaves inmediatamente
            ShowInventoryDialog(targetid);
        }
        else
        {
            printf("[Admin] ERROR: Llaves no entregadas - Master: %d, Normal: %d", 
                keyMasterResult, keyNormalResult);
            SendClientMessage(targetid, COLOR_ERROR, "{FF0000}ADVERTENCIA: {FFFFFF}No se pudieron dar las llaves. Contacta a un admin.");
        }
        
        new string[256];
        new vehicleName[32];
        
        // Obtener nombre del vehículo
        if(model >= 400 && model <= 611)
        {
            format(vehicleName, sizeof(vehicleName), "%s", VehicleModelNames[model - 400]);
        }
        else
        {
            format(vehicleName, sizeof(vehicleName), "Vehículo ID %d", model);
        }
        
        // Mensaje al admin
        format(string, sizeof(string), "{00FF00}Has dado un {FFFFFF}%s {00FF00}(Placa: %s) a {FFFFFF}%s", 
            vehicleName, plate, CharacterData[targetid][cName]);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        
        // Mensaje al jugador
        format(string, sizeof(string), "{FFD700}Admin {FFFFFF}%s {FFD700}te ha dado un {FFFFFF}%s {FFD700}(Placa: %s)\n{FFFF00}Has recibido 1 llave maestra y 1 llave normal.", 
            CharacterData[playerid][cName], vehicleName, plate);
        SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
        
        printf("[Admin] %s dio %s (DB ID: %d, Placa: %s) a %s", 
            CharacterData[playerid][cName], vehicleName, vehicleID, plate, CharacterData[targetid][cName]);
        
        return 1;
    }
    else
    {
        printf("[DEBUG AdminGiveVehicle] ERROR - vehicleID es 0 o negativo: %d", vehicleID);
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error al crear el vehículo en la base de datos.");
        return 0;
    }
    
    printf("[DEBUG AdminGiveVehicle] FIN - Retornando 1");
}

// Handler del diálogo principal del panel de administración
stock OnDialogAdminPanel(playerid, response, listitem)
{
    if(!response) return 1;
    
    new targetid = AdminPanelTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    new string[256];
    
    switch(listitem)
    {
        case 0: // Ir (Goto)
        {
            new Float:x, Float:y, Float:z;
            GetPlayerPos(targetid, x, y, z);
            SetPlayerPos(playerid, x + 1.0, y + 1.0, z);
            SetPlayerInterior(playerid, GetPlayerInterior(targetid));
            SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(targetid));
            
            format(string, sizeof(string), "{00FF00}Te has teletransportado a {FFFFFF}%s", CharacterData[targetid][cName]);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
            
            printf("[Admin] %s se teleporto a %s via panel", CharacterData[playerid][cName], CharacterData[targetid][cName]);
        }
        case 1: // Traer (Gethere)
        {
            new Float:x, Float:y, Float:z;
            GetPlayerPos(playerid, x, y, z);
            SetPlayerPos(targetid, x + 1.0, y + 1.0, z);
            SetPlayerInterior(targetid, GetPlayerInterior(playerid));
            SetPlayerVirtualWorld(targetid, GetPlayerVirtualWorld(playerid));
            
            format(string, sizeof(string), "{00FF00}Has traído a {FFFFFF}%s", CharacterData[targetid][cName]);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
            
            format(string, sizeof(string), "{FFD700}Admin {FFFFFF}%s {FFD700}te ha teletransportado.", CharacterData[playerid][cName]);
            SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
            
            printf("[Admin] %s trajo a %s via panel", CharacterData[playerid][cName], CharacterData[targetid][cName]);
        }
        case 2: // Mutear
        {
            // TODO: Implementar cuando /b esté programado
            SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Sistema de chat /b en desarrollo.");
        }
        case 3: // Freeze (Toggle)
        {
            if(PlayerFrozen[targetid])
            {
                // Descongelar
                TogglePlayerControllable(targetid, true);
                PlayerFrozen[targetid] = false;
                
                format(string, sizeof(string), "{00FF00}Has descongelado a {FFFFFF}%s", CharacterData[targetid][cName]);
                SendClientMessage(playerid, COLOR_SUCCESS, string);
                
                format(string, sizeof(string), "{FFD700}Has sido descongelado por el admin {FFFFFF}%s", CharacterData[playerid][cName]);
                SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
                
                printf("[Admin] %s descongelo a %s via panel", CharacterData[playerid][cName], CharacterData[targetid][cName]);
            }
            else
            {
                // Congelar
                TogglePlayerControllable(targetid, false);
                PlayerFrozen[targetid] = true;
                
                format(string, sizeof(string), "{00FF00}Has congelado a {FFFFFF}%s", CharacterData[targetid][cName]);
                SendClientMessage(playerid, COLOR_SUCCESS, string);
                
                format(string, sizeof(string), "{FFD700}Has sido congelado por el admin {FFFFFF}%s", CharacterData[playerid][cName]);
                SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
                
                printf("[Admin] %s congelo a %s via panel", CharacterData[playerid][cName], CharacterData[targetid][cName]);
            }
        }
        case 4: // Advertencia
        {
            new warnText[256];
            format(warnText, sizeof(warnText),
                "{FFFFFF}Ingresa la razón de la advertencia:\n\n\
                {AAAAAA}El jugador tiene actualmente {FF0000}%d/3 {AAAAAA}advertencias.\n\
                {FF0000}Al llegar a 3/3 será baneado automáticamente.",
                PlayerWarnings[targetid]);
            
            ShowPlayerDialog(playerid, DIALOG_ADMIN_PANEL_WARN, DIALOG_STYLE_INPUT,
                "Advertencia - Razón",
                warnText,
                "Advertir", "Cancelar");
        }
        case 5: // Encarcelar
        {
            // TODO: Implementar sistema de cárcel OOC
            // Cuando se implemente, agregar registro en admin_sanctions con:
            // sanction_type = 'JAIL', reason = razón, duration = minutos
            SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Sistema de cárcel OOC en desarrollo.");
        }
        case 6: // Expulsar (Kick)
        {
            ShowPlayerDialog(playerid, DIALOG_ADMIN_PANEL_KICK, DIALOG_STYLE_INPUT,
                "Expulsar Jugador",
                "{FFFFFF}Ingresa la razón de la expulsión:",
                "Expulsar", "Cancelar");
        }
        case 7: // Banear
        {
            ShowPlayerDialog(playerid, DIALOG_ADMIN_PANEL_BAN, DIALOG_STYLE_INPUT,
                "Banear Jugador - Razón",
                "{FFFFFF}Ingresa la razón del baneo:",
                "Siguiente", "Cancelar");
        }
        case 8: // Espectar (Toggle)
        {
            // Verificar si ya está espectando a alguien
            if(PlayerInFreecam[playerid])
            {
                // Dejar de espectar
                TogglePlayerSpectating(playerid, false);
                PlayerInFreecam[playerid] = false;
                SpawnPlayer(playerid);
                
                SendClientMessage(playerid, COLOR_SUCCESS, "{00FF00}Has dejado de espectar.");
                printf("[Admin] %s dejo de espectar", CharacterData[playerid][cName]);
            }
            else
            {
                // Comenzar a espectar
                TogglePlayerSpectating(playerid, true);
                PlayerSpectatePlayer(playerid, targetid);
                PlayerInFreecam[playerid] = true;
                
                format(string, sizeof(string), "{00FF00}Estás espectando a {FFFFFF}%s{00FF00}. Usa {FFFFFF}/afk {00FF00}para salir.", 
                    CharacterData[targetid][cName]);
                SendClientMessage(playerid, COLOR_SUCCESS, string);
                
                printf("[Admin] %s esta espectando a %s", CharacterData[playerid][cName], CharacterData[targetid][cName]);
            }
        }
        case 9: // Estadísticas
        {
            new statsText[768];
            format(statsText, sizeof(statsText), 
                "{FFFFFF}=== {00FF00}Estadísticas de %s{FFFFFF} ===\n\n\
                {FFD700}[Usuario]\n\
                {FFFFFF}Nickname: {FFFF00}%s {AAAAAA}(ID: %d)\n\
                {FFFFFF}Nivel Admin: {FF0000}%d\n\n\
                {FFD700}[Personaje]\n\
                {FFFFFF}ID: {AAAAAA}%d\n\
                {FFFFFF}Nombre: {AAAAAA}%s\n\
                {FFFFFF}Edad: {AAAAAA}%d años\n\
                {FFFFFF}Género: {AAAAAA}%s\n\
                {FFFFFF}Skin: {AAAAAA}%d\n\n\
                {FFD700}[Estado]\n\
                {FFFFFF}Dinero: {00FF00}$%d\n\
                {FFFFFF}Banco: {00FF00}$%d\n\
                {FFFFFF}Salud: {FF0000}%.1f%%\n\
                {FFFFFF}Armadura: {33CCFF}%.1f%%\n\
                {FFFFFF}Advertencias: {FF0000}%d/3",
                CharacterData[targetid][cName],
                UserData[targetid][uNickname], targetid,
                UserData[targetid][uAdminLevel],
                CharacterData[targetid][cID],
                CharacterData[targetid][cName],
                CharacterData[targetid][cAge],
                CharacterData[targetid][cGender] == 1 ? "Masculino" : "Femenino",
                CharacterData[targetid][cSkin],
                CharacterData[targetid][cMoney],
                CharacterData[targetid][cBank],
                CharacterData[targetid][cHealth],
                CharacterData[targetid][cArmour],
                PlayerWarnings[targetid]
            );
            
            ShowPlayerDialog(playerid, DIALOG_ADMIN_PANEL_STATS, DIALOG_STYLE_MSGBOX, 
                "Estadísticas del Jugador", statsText, "Cerrar", "");
        }
        case 10: // Inventario
        {
            // TODO: Implementar vista de inventario completa
            SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Vista de inventario en desarrollo.");
        }
        case 11: // Curar
        {
            // Resetear salud a 100
            SetPlayerHealth(targetid, 100.0);
            
            // Limpiar todas las heridas
            ClearWounds(targetid);
            
            // Salir de estado de agonía si está en él
            if(PlayerAgonyState[targetid] != AGONY_STATE_NORMAL)
            {
                ExitAgony(targetid);
            }
            
            format(string, sizeof(string), "{00FF00}Has curado completamente a {FFFFFF}%s{00FF00}. Todas sus heridas han sido eliminadas.", 
                CharacterData[targetid][cName]);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
            
            format(string, sizeof(string), "{00FF00}Has sido curado completamente por el admin {FFFFFF}%s", 
                CharacterData[playerid][cName]);
            SendClientMessage(targetid, COLOR_SUCCESS, string);
            
            printf("[Admin] %s curo completamente a %s via panel (heridas reseteadas)", 
                CharacterData[playerid][cName], CharacterData[targetid][cName]);
        }
    }
    
    return 1;
}

// Handler para expulsar con razón
stock OnDialogAdminPanelKick(playerid, response, inputtext[])
{
    if(!response) return 1;
    
    new targetid = AdminPanelTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    if(strlen(inputtext) < 3)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}La razón debe tener al menos 3 caracteres.");
        return 1;
    }
    
    new string[200];
    format(string, sizeof(string), "{FF0000}%s ha sido expulsado por %s. Razón: %s", 
        CharacterData[targetid][cName], CharacterData[playerid][cName], inputtext);
    SendClientMessageToAll(COLOR_ERROR, string);
    
    printf("[Admin] %s expulso a %s via panel - Razon: %s", 
        CharacterData[playerid][cName], CharacterData[targetid][cName], inputtext);
    
    Kick(targetid);
    
    return 1;
}

// Handler para baneo - Paso 1: Razón
new AdminBanReason[MAX_PLAYERS][256]; // Variable temporal para guardar la razón

stock OnDialogAdminPanelBan(playerid, response, inputtext[])
{
    if(!response) return 1;
    
    if(strlen(inputtext) < 3)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}La razón debe tener al menos 3 caracteres.");
        return 1;
    }
    
    // Guardar razón temporalmente
    format(AdminBanReason[playerid], 256, "%s", inputtext);
    
    ShowPlayerDialog(playerid, DIALOG_ADMIN_PANEL_BAN_TIME, DIALOG_STYLE_INPUT,
        "Banear Jugador - Tiempo",
        "{FFFFFF}Ingresa el tiempo del baneo en días:\n\n\
        {AAAAAA}Ingresa {FF0000}0 {AAAAAA}para baneo permanente\n\
        {AAAAAA}Ejemplos: 1, 7, 30, 0",
        "Banear", "Cancelar");
    
    return 1;
}

// Handler para baneo - Paso 2: Tiempo
stock OnDialogAdminPanelBanTime(playerid, response, const inputtext[])
{
    if(!response) return 1;
    
    new targetid = AdminPanelTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    new days = strval(inputtext);
    if(days < 0)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El tiempo debe ser 0 o mayor.");
        return 1;
    }
    
    // Registrar en historial de sanciones
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO admin_sanctions (user_id, admin_id, admin_name, sanction_type, reason, duration) \
        VALUES (%d, %d, '%e', 'BAN', '%e', %d)",
        UserData[targetid][uID], UserData[playerid][uID], CharacterData[playerid][cName], 
        AdminBanReason[playerid], days);
    mysql_tquery(g_MySQL, query);
    
    // TODO: Implementar sistema de baneo en base de datos
    new string[200];
    if(days == 0)
    {
        format(string, sizeof(string), "{FF0000}%s ha sido baneado PERMANENTEMENTE por %s.", 
            CharacterData[targetid][cName], CharacterData[playerid][cName]);
    }
    else
    {
        format(string, sizeof(string), "{FF0000}%s ha sido baneado por %d días por %s.", 
            CharacterData[targetid][cName], days, CharacterData[playerid][cName]);
    }
    SendClientMessageToAll(COLOR_ERROR, string);
    
    printf("[Admin] %s baneo a %s via panel - Tiempo: %d dias", 
        CharacterData[playerid][cName], CharacterData[targetid][cName], days);
    
    Ban(targetid);
    
    return 1;
}

// Handler para advertencias
stock OnDialogAdminPanelWarn(playerid, response, inputtext[])
{
    if(!response) return 1;
    
    new targetid = AdminPanelTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    if(strlen(inputtext) < 3)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}La razón debe tener al menos 3 caracteres.");
        return 1;
    }
    
    // Incrementar advertencias
    PlayerWarnings[targetid]++;
    
    // Guardar en base de datos
    new query[512];
    mysql_format(g_MySQL, query, sizeof(query),
        "UPDATE users SET warnings = %d WHERE id = %d",
        PlayerWarnings[targetid], UserData[targetid][uID]);
    mysql_tquery(g_MySQL, query);
    
    // Registrar en historial de sanciones
    mysql_format(g_MySQL, query, sizeof(query),
        "INSERT INTO admin_sanctions (user_id, admin_id, admin_name, sanction_type, reason) \
        VALUES (%d, %d, '%e', 'WARN', '%e')",
        UserData[targetid][uID], UserData[playerid][uID], CharacterData[playerid][cName], inputtext);
    mysql_tquery(g_MySQL, query);
    
    new string[256];
    format(string, sizeof(string), 
        "{FF0000}Has recibido una advertencia [{FFFFFF}%d/3{FF0000}] por el admin {FFFFFF}%s\n\
        {FF0000}Razón: {FFFFFF}%s", 
        PlayerWarnings[targetid], CharacterData[playerid][cName], inputtext);
    SendClientMessage(targetid, COLOR_ERROR, string);
    
    format(string, sizeof(string), 
        "{00FF00}Has advertido a {FFFFFF}%s {00FF00}[{FFFFFF}%d/3{00FF00}]", 
        CharacterData[targetid][cName], PlayerWarnings[targetid]);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    // Si llega a 3 advertencias, banear automáticamente
    if(PlayerWarnings[targetid] >= 3)
    {
        format(string, sizeof(string), 
            "{FF0000}%s ha sido baneado automáticamente por acumular 3 advertencias.", 
            CharacterData[targetid][cName]);
        SendClientMessageToAll(COLOR_ERROR, string);
        
        printf("[Admin] %s fue baneado automaticamente por 3 advertencias", CharacterData[targetid][cName]);
        
        Ban(targetid);
    }
    
    printf("[Admin] %s advirtio a %s (%d/3) - Razon: %s", 
        CharacterData[playerid][cName], CharacterData[targetid][cName], PlayerWarnings[targetid], inputtext);
    
    return 1;
}

// ==================== HANDLERS DEL COMANDO /DAR ====================

stock OnDialogAdminDar(playerid, response, listitem)
{
    if(!response) return 1;
    
    new targetid = AdminDarTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    new string[256];
    
    switch(listitem)
    {
        case 0: // Vida
        {
            SetPlayerHealth(targetid, 100.0);
            CharacterData[targetid][cHealth] = 100.0;
            
            format(string, sizeof(string), "{00FF00}Has dado vida completa a {FFFFFF}%s", CharacterData[targetid][cName]);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
            
            format(string, sizeof(string), "{FFD700}Admin {FFFFFF}%s {FFD700}te ha dado vida completa.", CharacterData[playerid][cName]);
            SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
            
            printf("[Admin] %s dio vida a %s", CharacterData[playerid][cName], CharacterData[targetid][cName]);
        }
        case 1: // Armadura
        {
            SetPlayerArmour(targetid, 100.0);
            CharacterData[targetid][cArmour] = 100.0;
            
            format(string, sizeof(string), "{00FF00}Has dado armadura completa a {FFFFFF}%s", CharacterData[targetid][cName]);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
            
            format(string, sizeof(string), "{FFD700}Admin {FFFFFF}%s {FFD700}te ha dado armadura completa.", CharacterData[playerid][cName]);
            SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
            
            printf("[Admin] %s dio armadura a %s", CharacterData[playerid][cName], CharacterData[targetid][cName]);
        }
        case 2: // Dinero
        {
            ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR_MONEY, DIALOG_STYLE_LIST,
                "Dar Dinero",
                "$1,000\n$10,000\n$100,000\n$1,000,000\n$10,000,000",
                "Dar", "Cancelar");
        }
        case 3: // Jetpack
        {
            SetPlayerSpecialAction(targetid, SPECIAL_ACTION_USEJETPACK);
            
            format(string, sizeof(string), "{00FF00}Has dado jetpack a {FFFFFF}%s", CharacterData[targetid][cName]);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
            
            format(string, sizeof(string), "{FFD700}Admin {FFFFFF}%s {FFD700}te ha dado un jetpack.", CharacterData[playerid][cName]);
            SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
            
            printf("[Admin] %s dio jetpack a %s via /dar", CharacterData[playerid][cName], CharacterData[targetid][cName]);
        }
        case 4: // Skins
        {
            ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR_SKIN, DIALOG_STYLE_INPUT,
                "Dar Skin",
                "{FFFFFF}Ingresa el ID del skin (0-311):",
                "Dar", "Cancelar");
        }
        case 5: // Autos
        {
            printf("[DEBUG /dar] Jugador %s (ID:%d) selecciono opcion AUTOS", CharacterData[playerid][cName], playerid);
            printf("[DEBUG /dar] Target: %s (ID:%d)", CharacterData[targetid][cName], targetid);
            
            // Resetear página a 0 y mostrar primera página
            AdminVehiclePage[playerid] = 0;
            ShowAdminVehicleListPage(playerid);
        }
        case 6: // Armas
        {
            new weaponlist[2048];
            for(new i = 0; i < TotalWeapons; i++)
            {
                new line[64];
                format(line, sizeof(line), "%s (ID:%d)\n", WeaponData[i][wName], WeaponData[i][wID]);
                strcat(weaponlist, line);
            }
            ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR_WEAPON, DIALOG_STYLE_LIST,
                "Dar Arma",
                weaponlist,
                "Seleccionar", "Cancelar");
        }
        case 7: // Cargadores
        {
            new maglist[2048];
            for(new i = 0; i < TotalMagazines && i < 20; i++) // Limitar a 20 para no overflow
            {
                new line[80];
                format(line, sizeof(line), "%s (ID:%d - Arma:%d)\n", 
                    MagazineData[i][mName], MagazineData[i][mID], MagazineData[i][mWeaponID]);
                strcat(maglist, line);
            }
            ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR_MAGAZINE, DIALOG_STYLE_LIST,
                "Dar Cargador",
                maglist,
                "Seleccionar", "Cancelar");
        }
        case 8: // Necesidades
        {
            // TODO: Implementar cuando el sistema de necesidades esté listo
            SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Sistema de necesidades en desarrollo.");
        }
    }
    
    return 1;
}

stock OnDialogAdminDarMoney(playerid, response, listitem)
{
    if(!response) return 1;
    
    new targetid = AdminDarTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    new amounts[] = {1000, 10000, 100000, 1000000, 10000000};
    new amount = amounts[listitem];
    
    GivePlayerMoney(targetid, amount);
    CharacterData[targetid][cMoney] += amount;
    
    // Guardar en base de datos
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "UPDATE characters SET money = %d WHERE id = %d",
        CharacterData[targetid][cMoney], CharacterData[targetid][cID]);
    mysql_tquery(g_MySQL, query);
    
    new string[256];
    format(string, sizeof(string), "{00FF00}Has dado {FFFFFF}$%d {00FF00}a {FFFFFF}%s", 
        amount, CharacterData[targetid][cName]);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    format(string, sizeof(string), "{FFD700}Admin {FFFFFF}%s {FFD700}te ha dado {FFFFFF}$%d", 
        CharacterData[playerid][cName], amount);
    SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
    
    printf("[Admin] %s dio $%d a %s", CharacterData[playerid][cName], amount, CharacterData[targetid][cName]);
    
    return 1;
}

stock OnDialogAdminDarSkin(playerid, response, const inputtext[])
{
    if(!response) return 1;
    
    new targetid = AdminDarTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    new skinid = strval(inputtext);
    if(skinid < 0 || skinid > 311)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Skin ID inválido. Rango: 0-311");
        return 1;
    }
    
    SetPlayerSkin(targetid, skinid);
    CharacterData[targetid][cSkin] = skinid;
    
    // Guardar en base de datos
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query),
        "UPDATE characters SET skin = %d WHERE id = %d",
        skinid, CharacterData[targetid][cID]);
    mysql_tquery(g_MySQL, query);
    
    new string[256];
    format(string, sizeof(string), "{00FF00}Has cambiado la skin de {FFFFFF}%s {00FF00}a {FFFFFF}%d", 
        CharacterData[targetid][cName], skinid);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    format(string, sizeof(string), "{FFD700}Admin {FFFFFF}%s {FFD700}ha cambiado tu skin a {FFFFFF}%d", 
        CharacterData[playerid][cName], skinid);
    SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
    
    printf("[Admin] %s cambio skin de %s a %d via /dar", 
        CharacterData[playerid][cName], CharacterData[targetid][cName], skinid);
    
    return 1;
}

stock OnDialogAdminDarVehicle(playerid, response, listitem)
{
    printf("[DEBUG OnDialogAdminDarVehicle] Llamado - Player:%d Response:%d Listitem:%d", playerid, response, listitem);
    
    if(!response)
    {
        printf("[DEBUG OnDialogAdminDarVehicle] Player %d cancelo el dialogo", playerid);
        return 1;
    }
    
    new targetid = AdminDarTarget[playerid];
    printf("[DEBUG OnDialogAdminDarVehicle] Target guardado: %d", targetid);
    
    if(!IsPlayerConnected(targetid))
    {
        printf("[DEBUG OnDialogAdminDarVehicle] Target %d no esta conectado", targetid);
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    printf("[DEBUG OnDialogAdminDarVehicle] Target %s (ID:%d) esta conectado", CharacterData[targetid][cName], targetid);
    
    new page = AdminVehiclePage[playerid];
    new totalVehicles = sizeof(VehicleModelNames);
    new totalPages = (totalVehicles / VEHICLES_PER_PAGE) + ((totalVehicles % VEHICLES_PER_PAGE) ? 1 : 0);
    new startIdx = page * VEHICLES_PER_PAGE;
    new itemsInPage = VEHICLES_PER_PAGE;
    if(startIdx + itemsInPage > totalVehicles) itemsInPage = totalVehicles - startIdx;
    
    // Item 0 es siempre "Buscar por nombre"
    if(listitem == 0)
    {
        printf("[DEBUG OnDialogAdminDarVehicle] Opcion 'Buscar por nombre' seleccionada");
        ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR_VEHICLE_SEARCH, DIALOG_STYLE_INPUT,
            "Buscar Vehículo",
            "{FFFFFF}Ingresa el nombre del vehículo a buscar:\n{FFFF00}Ejemplos: {FFFFFF}infernus, sultan, sanchez, bmx",
            "Buscar", "Cancelar");
        return 1;
    }
    
    // Calcular posición de botones de navegación
    new prevButtonPos = itemsInPage + 1; // Después de los vehículos
    new nextButtonPos = prevButtonPos;
    if(page > 0) nextButtonPos++; // Si hay botón anterior, el siguiente está después
    
    // Verificar si es botón de navegación "Anterior"
    if(page > 0 && listitem == prevButtonPos)
    {
        printf("[DEBUG OnDialogAdminDarVehicle] Navegacion: Pagina Anterior (de %d a %d)", page + 1, page);
        AdminVehiclePage[playerid]--;
        ShowAdminVehicleListPage(playerid);
        return 1;
    }
    
    // Verificar si es botón de navegación "Siguiente"
    if(page < totalPages - 1 && listitem == nextButtonPos)
    {
        printf("[DEBUG OnDialogAdminDarVehicle] Navegacion: Pagina Siguiente (de %d a %d)", page + 1, page + 2);
        AdminVehiclePage[playerid]++;
        ShowAdminVehicleListPage(playerid);
        return 1;
    }
    
    // Es un vehículo - calcular índice real (restando 1 por el botón de búsqueda)
    new vehicleIndex = startIdx + (listitem - 1);
    new vehicleModelId = 400 + vehicleIndex;
    
    printf("[DEBUG OnDialogAdminDarVehicle] Listitem:%d -> VehicleIndex:%d -> ModelID:%d", listitem, vehicleIndex, vehicleModelId);
    printf("[DEBUG OnDialogAdminDarVehicle] Llamando AdminGiveVehicle(%d, %d, %d)", playerid, targetid, vehicleModelId);
    
    // Usar la función helper para dar el vehículo permanentemente
    AdminGiveVehicle(playerid, targetid, vehicleModelId);
    
    printf("[DEBUG OnDialogAdminDarVehicle] AdminGiveVehicle finalizado");
    
    return 1;
}

// Handler para búsqueda de vehículos por nombre
stock OnDialogAdminDarVehicleSearch(playerid, response, inputtext[])
{
    printf("[DEBUG OnDialogAdminDarVehicleSearch] Llamado - Player:%d Response:%d Input:'%s'", playerid, response, inputtext);
    
    if(!response)
    {
        printf("[DEBUG OnDialogAdminDarVehicleSearch] Player %d cancelo la busqueda", playerid);
        return 1;
    }
    
    new targetid = AdminDarTarget[playerid];
    printf("[DEBUG OnDialogAdminDarVehicleSearch] Target: %d", targetid);
    
    if(!IsPlayerConnected(targetid))
    {
        printf("[DEBUG OnDialogAdminDarVehicleSearch] Target %d no esta conectado", targetid);
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    if(strlen(inputtext) < 2)
    {
        printf("[DEBUG OnDialogAdminDarVehicleSearch] Input muy corto: %d caracteres", strlen(inputtext));
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Ingresa al menos 2 caracteres para buscar.");
        return 1;
    }
    
    // Buscar vehículos que coincidan
    new searchResults[2048], resultsCount = 0;
    new searchLower[64], vehicleNameLower[64];
    
    // Limpiar resultados previos
    for(new i = 0; i < 50; i++)
    {
        AdminVehicleSearchResults[playerid][i] = -1;
    }
    
    // Convertir búsqueda a minúsculas
    format(searchLower, sizeof(searchLower), "%s", inputtext);
    for(new i = 0; searchLower[i] != 0; i++)
    {
        if(searchLower[i] >= 'A' && searchLower[i] <= 'Z')
            searchLower[i] += 32;
    }
    
    // Buscar coincidencias
    for(new i = 0; i < sizeof(VehicleModelNames); i++)
    {
        format(vehicleNameLower, sizeof(vehicleNameLower), "%s", VehicleModelNames[i]);
        
        // Convertir nombre del vehículo a minúsculas
        for(new j = 0; vehicleNameLower[j] != 0; j++)
        {
            if(vehicleNameLower[j] >= 'A' && vehicleNameLower[j] <= 'Z')
                vehicleNameLower[j] += 32;
        }
        
        // Verificar si contiene la búsqueda
        if(strfind(vehicleNameLower, searchLower, true) != -1)
        {
            format(searchResults, sizeof(searchResults), "%s%s\n", searchResults, VehicleModelNames[i]);
            AdminVehicleSearchResults[playerid][resultsCount] = i; // Guardar índice
            resultsCount++;
            
            if(resultsCount >= 50) break; // Limitar resultados
        }
    }
    
    if(resultsCount == 0)
    {
        printf("[DEBUG OnDialogAdminDarVehicleSearch] No se encontraron resultados para: '%s'", inputtext);
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No se encontraron vehículos con ese nombre.");
        return 1;
    }
    
    printf("[DEBUG OnDialogAdminDarVehicleSearch] Se encontraron %d resultados", resultsCount);
    printf("[DEBUG OnDialogAdminDarVehicleSearch] Mostrando dialogo DIALOG_ADMIN_DAR_VEHICLE_RESULT");
    
    ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR_VEHICLE_RESULT, DIALOG_STYLE_LIST,
        "Resultados de Búsqueda",
        searchResults,
        "Dar", "Cancelar");
    
    return 1;
}

// Handler para selección de vehículo desde búsqueda
stock OnDialogAdminDarVehSearch(playerid, response, listitem, const inputtext[])
{
    #pragma unused inputtext
    printf("[DEBUG OnDialogAdminDarVehSearch] Llamado - Player:%d Response:%d Listitem:%d", playerid, response, listitem);
    
    if(!response)
    {
        printf("[DEBUG OnDialogAdminDarVehSearch] Player %d cancelo la seleccion", playerid);
        return 1;
    }
    
    new targetid = AdminDarTarget[playerid];
    printf("[DEBUG OnDialogAdminDarVehSearch] Target: %d", targetid);
    
    if(!IsPlayerConnected(targetid))
    {
        printf("[DEBUG OnDialogAdminDarVehSearch] Target %d no esta conectado", targetid);
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    // Obtener el índice del vehículo desde los resultados guardados
    if(listitem < 0 || listitem >= 50)
    {
        printf("[DEBUG OnDialogAdminDarVehSearch] Listitem fuera de rango: %d", listitem);
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Selección inválida.");
        return 1;
    }
    
    new vehicleIndex = AdminVehicleSearchResults[playerid][listitem];
    printf("[DEBUG OnDialogAdminDarVehSearch] VehicleIndex obtenido: %d", vehicleIndex);
    
    if(vehicleIndex == -1)
    {
        printf("[DEBUG OnDialogAdminDarVehSearch] VehicleIndex es -1, error en la seleccion");
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error al seleccionar el vehículo.");
        return 1;
    }
    
    new vehicleModelId = 400 + vehicleIndex;
    printf("[DEBUG OnDialogAdminDarVehSearch] ModelID calculado: %d", vehicleModelId);
    printf("[DEBUG OnDialogAdminDarVehSearch] Llamando AdminGiveVehicle(%d, %d, %d)", playerid, targetid, vehicleModelId);
    
    // Usar la función helper para dar el vehículo permanentemente
    AdminGiveVehicle(playerid, targetid, vehicleModelId);
    
    printf("[DEBUG OnDialogAdminDarVehSearch] AdminGiveVehicle finalizado");
    
    return 1;
}

// ==================== HANDLERS DE ARMAS Y CARGADORES ====================

// Variables temporales para armas/cargadores
new AdminSelectedWeapon[MAX_PLAYERS] = {-1, ...};
new AdminSelectedMagazine[MAX_PLAYERS] = {-1, ...};

// Handler para selección de arma
stock OnDialogAdminDarWeapon(playerid, response, listitem)
{
    if(!response) return 1;
    
    new targetid = AdminDarTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    if(listitem >= TotalWeapons) return 1;
    
    // Dar arma directamente (1 unidad)
    new weaponIdx = listitem;
    
    new weaponItemID = WeaponData[weaponIdx][wItemID];
    
    // Crear metadata con 0 balas
    new metadata[64];
    format(metadata, sizeof(metadata), "{\"ammo\":0,\"equipped\":0}");
    
    if(GivePlayerItem(targetid, weaponItemID, 1, metadata))
    {
        new string[144];
        format(string, sizeof(string), "{00FF00}Le has dado {FFFFFF}%s {00FF00}a {FFFFFF}%s {00FF00}(0 balas guardadas).",
            WeaponData[weaponIdx][wName], CharacterData[targetid][cName]);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        
        format(string, sizeof(string), "{00FF00}Has recibido {FFFFFF}%s {00FF00}(0 balas). Necesitas cargadores para usar el arma.",
            WeaponData[weaponIdx][wName]);
        SendClientMessage(targetid, COLOR_SUCCESS, string);
        
        // Refrescar inventario para mostrar el item inmediatamente
        ShowInventoryDialog(targetid);
    }
    else
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El inventario del jugador está lleno.");
    }
    
    return 1;
}

// Handler para cantidad de armas
stock OnDialogAdminDarWeaponQuantity(playerid, response, const inputtext[])
{
    if(!response)
    {
        AdminSelectedWeapon[playerid] = -1;
        return 1;
    }
    
    new targetid = AdminDarTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        AdminSelectedWeapon[playerid] = -1;
        return 1;
    }
    
    new weaponIdx = AdminSelectedWeapon[playerid];
    if(weaponIdx == -1 || weaponIdx >= TotalWeapons) return 1;
    
    new quantity = strval(inputtext);
    if(quantity < 1 || quantity > 100)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Cantidad inválida (1-100).");
        AdminSelectedWeapon[playerid] = -1;
        return 1;
    }
    
    // Verificar que el arma tenga item_id
    if(WeaponData[weaponIdx][wItemID] == 0)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Esta arma no tiene item_id configurado.");
        AdminSelectedWeapon[playerid] = -1;
        return 1;
    }
    
    // Dar arma al inventario
    if(GivePlayerItem(targetid, WeaponData[weaponIdx][wItemID], quantity, ""))
    {
        new string[144];
        format(string, sizeof(string), "{00FF00}Has dado {FFFFFF}%d x %s {00FF00}a {FFFFFF}%s",
            quantity, WeaponData[weaponIdx][wName], CharacterData[targetid][cName]);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        
        format(string, sizeof(string), "{FFD700}Admin {FFFFFF}%s {FFD700}te ha dado {FFFFFF}%d x %s",
            CharacterData[playerid][cName], quantity, WeaponData[weaponIdx][wName]);
        SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
        
        // Refrescar inventario para mostrar el item inmediatamente
        ShowInventoryDialog(targetid);
        
        printf("[Admin] %s dio %d x %s a %s via /dar", 
            CharacterData[playerid][cName], quantity, WeaponData[weaponIdx][wName], CharacterData[targetid][cName]);
    }
    else
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error al dar el arma. Inventario lleno?");
    }
    
    AdminSelectedWeapon[playerid] = -1;
    return 1;
}

// Handler para selección de cargador
stock OnDialogAdminDarMagazine(playerid, response, listitem)
{
    if(!response) return 1;
    
    new targetid = AdminDarTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    if(listitem >= TotalMagazines) return 1;
    
    // Dar cargador directamente (1 unidad con capacidad completa)
    new magIdx = listitem;
    new magItemID = MagazineData[magIdx][mItemID];
    new capacity = MagazineData[magIdx][mCapacity];
    
    // Crear metadata con cantidad de balas igual a la capacidad
    new metadata[64];
    format(metadata, sizeof(metadata), "{\\\"bullets\\\":%d}", capacity);
    
    if(GivePlayerItem(targetid, magItemID, 1, metadata))
    {
        new string[144];
        format(string, sizeof(string), "{00FF00}Le has dado {FFFFFF}%s {00FF00}a {FFFFFF}%s {00FF00}(%d balas).",
            MagazineData[magIdx][mName], CharacterData[targetid][cName], capacity);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        
        format(string, sizeof(string), "{00FF00}Has recibido {FFFFFF}%s {00FF00}(%d balas).",
            MagazineData[magIdx][mName], capacity);
        SendClientMessage(targetid, COLOR_SUCCESS, string);
        
        // Refrescar inventario para mostrar el item inmediatamente
        ShowInventoryDialog(targetid);
    }
    else
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El inventario del jugador está lleno.");
    }
    
    return 1;
}

// Handler para cantidad de cargadores
stock OnDialogAdminDarMagQty(playerid, response, const inputtext[])
{
    if(!response)
    {
        AdminSelectedMagazine[playerid] = -1;
        return 1;
    }
    
    new targetid = AdminDarTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        AdminSelectedMagazine[playerid] = -1;
        return 1;
    }
    
    new magIdx = AdminSelectedMagazine[playerid];
    if(magIdx == -1 || magIdx >= TotalMagazines) return 1;
    
    new quantity = strval(inputtext);
    if(quantity < 1 || quantity > 100)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Cantidad inválida (1-100).");
        AdminSelectedMagazine[playerid] = -1;
        return 1;
    }
    
    // Verificar que el cargador tenga item_id
    if(MagazineData[magIdx][mItemID] == 0)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Este cargador no tiene item_id configurado.");
        AdminSelectedMagazine[playerid] = -1;
        return 1;
    }
    
    // Dar cargador al inventario
    if(GivePlayerItem(targetid, MagazineData[magIdx][mItemID], quantity, ""))
    {
        new string[144];
        format(string, sizeof(string), "{00FF00}Has dado {FFFFFF}%d x %s {00FF00}a {FFFFFF}%s",
            quantity, MagazineData[magIdx][mName], CharacterData[targetid][cName]);
        SendClientMessage(playerid, COLOR_SUCCESS, string);
        
        format(string, sizeof(string), "{FFD700}Admin {FFFFFF}%s {FFD700}te ha dado {FFFFFF}%d x %s",
            CharacterData[playerid][cName], quantity, MagazineData[magIdx][mName]);
        SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
        
        // Refrescar inventario para mostrar el item inmediatamente
        ShowInventoryDialog(targetid);
        
        printf("[Admin] %s dio %d x %s a %s via /dar", 
            CharacterData[playerid][cName], quantity, MagazineData[magIdx][mName], CharacterData[targetid][cName]);
    }
    else
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error al dar el cargador. Inventario lleno?");
    }
    
    AdminSelectedMagazine[playerid] = -1;
    return 1;
}
