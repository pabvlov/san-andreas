/*
    Handlers de diálogos para el sistema de administración
*/

#if defined _admin_dialogs_included
    #endinput
#endif
#define _admin_dialogs_included

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
            new vehiclelist[2048];
            strcat(vehiclelist, "Buscar por nombre\n");
            for(new i = 0; i < sizeof(VehicleNames); i++)
            {
                format(vehiclelist, sizeof(vehiclelist), "%s%s\n", vehiclelist, VehicleNames[i]);
            }
            ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR_VEHICLE, DIALOG_STYLE_LIST,
                "Dar Vehículo",
                vehiclelist,
                "Dar", "Cancelar");
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
    if(!response) return 1;
    
    new targetid = AdminDarTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    // Si selecciona "Buscar por nombre"
    if(listitem == 0)
    {
        ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR_VEHICLE_SEARCH, DIALOG_STYLE_INPUT,
            "Buscar Vehículo",
            "{FFFFFF}Ingresa el nombre del vehículo a buscar:",
            "Buscar", "Cancelar");
        return 1;
    }
    
    // Ajustar índice (el primer item es "Buscar por nombre")
    new vehicleIndex = listitem - 1;
    new vehicleModelId = 400 + vehicleIndex;
    
    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(targetid, x, y, z);
    GetPlayerFacingAngle(targetid, a);
    
    new vehicleid = CreateVehicle(vehicleModelId, x + 3.0, y, z, a, -1, -1, 60000);
    
    new string[256];
    format(string, sizeof(string), "{00FF00}Has dado un {FFFFFF}%s {00FF00}a {FFFFFF}%s", 
        VehicleNames[vehicleIndex], CharacterData[targetid][cName]);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    format(string, sizeof(string), "{FFD700}Admin {FFFFFF}%s {FFD700}te ha dado un {FFFFFF}%s", 
        CharacterData[playerid][cName], VehicleNames[vehicleIndex]);
    SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
    
    printf("[Admin] %s dio %s (ID: %d) a %s", 
        CharacterData[playerid][cName], VehicleNames[vehicleIndex], vehicleid, CharacterData[targetid][cName]);
    
    return 1;
}

// Handler para búsqueda de vehículos por nombre
stock OnDialogAdminDarVehicleSearch(playerid, response, inputtext[])
{
    if(!response) return 1;
    
    new targetid = AdminDarTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    if(strlen(inputtext) < 2)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Ingresa al menos 2 caracteres para buscar.");
        return 1;
    }
    
    // Buscar vehículos que coincidan
    new searchResults[2048], resultsCount = 0;
    new searchLower[64], vehicleNameLower[64];
    
    // Convertir búsqueda a minúsculas
    format(searchLower, sizeof(searchLower), "%s", inputtext);
    for(new i = 0; searchLower[i] != 0; i++)
    {
        if(searchLower[i] >= 'A' && searchLower[i] <= 'Z')
            searchLower[i] += 32;
    }
    
    // Buscar coincidencias
    for(new i = 0; i < sizeof(VehicleNames); i++)
    {
        format(vehicleNameLower, sizeof(vehicleNameLower), "%s", VehicleNames[i]);
        
        // Convertir nombre del vehículo a minúsculas
        for(new j = 0; vehicleNameLower[j] != 0; j++)
        {
            if(vehicleNameLower[j] >= 'A' && vehicleNameLower[j] <= 'Z')
                vehicleNameLower[j] += 32;
        }
        
        // Verificar si contiene la búsqueda
        if(strfind(vehicleNameLower, searchLower, true) != -1)
        {
            format(searchResults, sizeof(searchResults), "%s%s\n", searchResults, VehicleNames[i]);
            resultsCount++;
            
            if(resultsCount >= 50) break; // Limitar resultados
        }
    }
    
    if(resultsCount == 0)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}No se encontraron vehículos con ese nombre.");
        return 1;
    }
    
    ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR_VEHICLE_RESULT, DIALOG_STYLE_LIST,
        "Resultados de Búsqueda",
        searchResults,
        "Dar", "Cancelar");
    
    return 1;
}

// Handler para selección de vehículo desde búsqueda
stock OnDialogAdminDarVehSearch(playerid, response, listitem, const inputtext[])
{
    #pragma unused listitem, inputtext
    if(!response) return 1;
    
    new targetid = AdminDarTarget[playerid];
    if(!IsPlayerConnected(targetid))
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}El jugador ya no está conectado.");
        return 1;
    }
    
    // Buscar el índice del vehículo por nombre
    new vehicleIndex = -1;
    for(new i = 0; i < sizeof(VehicleNames); i++)
    {
        if(strcmp(inputtext, VehicleNames[i], true) == 0)
        {
            vehicleIndex = i;
            break;
        }
    }
    
    if(vehicleIndex == -1)
    {
        SendClientMessage(playerid, COLOR_ERROR, "{FF0000}Error al seleccionar el vehículo.");
        return 1;
    }
    
    new vehicleModelId = 400 + vehicleIndex;
    
    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(targetid, x, y, z);
    GetPlayerFacingAngle(targetid, a);
    
    new vehicleid = CreateVehicle(vehicleModelId, x + 3.0, y, z, a, -1, -1, 60000);
    
    new string[256];
    format(string, sizeof(string), "{00FF00}Has dado un {FFFFFF}%s {00FF00}a {FFFFFF}%s", 
        VehicleNames[vehicleIndex], CharacterData[targetid][cName]);
    SendClientMessage(playerid, COLOR_SUCCESS, string);
    
    format(string, sizeof(string), "{FFD700}Admin {FFFFFF}%s {FFD700}te ha dado un {FFFFFF}%s", 
        CharacterData[playerid][cName], VehicleNames[vehicleIndex]);
    SendClientMessage(targetid, COLOR_ADMIN_MSG, string);
    
    printf("[Admin] %s dio %s (ID: %d) a %s via busqueda", 
        CharacterData[playerid][cName], VehicleNames[vehicleIndex], vehicleid, CharacterData[targetid][cName]);
    
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
    
    // Guardar arma seleccionada
    AdminSelectedWeapon[playerid] = listitem;
    
    // Preguntar cantidad
    new dialog[256];
    format(dialog, sizeof(dialog),
        "{FFFFFF}Has seleccionado: {FFFF00}%s\n\n\
        {FFFFFF}Ingresa la cantidad a dar (1-100):",
        WeaponData[listitem][wName]);
    
    ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR_WEAPON_QUANTITY, DIALOG_STYLE_INPUT,
        "Cantidad de Armas", dialog, "Dar", "Cancelar");
    
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
    
    // Guardar cargador seleccionado
    AdminSelectedMagazine[playerid] = listitem;
    
    // Preguntar cantidad
    new dialog[256];
    format(dialog, sizeof(dialog),
        "{FFFFFF}Has seleccionado: {FFFF00}%s\n\
        {AAAAAA}Para arma ID: %d (%d balas)\n\n\
        {FFFFFF}Ingresa la cantidad a dar (1-100):",
        MagazineData[listitem][mName],
        MagazineData[listitem][mWeaponID],
        MagazineData[listitem][mCapacity]);
    
    ShowPlayerDialog(playerid, DIALOG_ADMIN_DAR_MAGAZINE_QUANTITY, DIALOG_STYLE_INPUT,
        "Cantidad de Cargadores", dialog, "Dar", "Cancelar");
    
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
