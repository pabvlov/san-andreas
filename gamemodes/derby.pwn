/*
    San Andreas Roleplay by pabvlov 2024
*/
#include <open.mp>
#include <a_samp>
#include <a_mysql>
#include <sscanf2>
#include <streamer>

// Include ZCMD antes de otros includes que usen CMD:
#include "../include/zcmd.inc"

// MySQL Configuration  
#define MYSQL_HOST      "143.198.232.23"
#define MYSQL_USER      "root"
#define MYSQL_PASSWORD  "sitehefalladotepidoperdon"
#define MYSQL_DATABASE  "san-andreas"
#define MYSQL_PORT      3307

// Colors
#define COLOR_WHITE     (0xFFFFFFFF)
#define COLOR_GRAY      (0xAAAAAAFF)
#define COLOR_GREY      (0xAAAAAAFF)
#define COLOR_RED       (0xFF0000FF)
#define COLOR_GREEN     (0x00FF00FF)
#define COLOR_YELLOW    (0xFFFF00FF)
#define COLOR_INFO      (0x33CCFFFF)
#define COLOR_SUCCESS   (0x00FF00FF)
#define COLOR_ERROR     (0xFF0000FF)
#define COLOR_ADMIN     (0xFF6347FF)

// Dialogs
#define DIALOG_AUTO             1000
#define DIALOG_REGISTER         1001
#define DIALOG_LOGIN            1002
#define DIALOG_CHARACTER_LIST   1003
#define DIALOG_CHARACTER_CREATE 1004
#define DIALOG_CHARACTER_DELETE 1005
#define DIALOG_CHARACTER_STATS  1006
#define DIALOG_EDIT_AGE         1007
#define DIALOG_EDIT_SKIN        1008
#define DIALOG_EDIT_GENDER      1009
#define DIALOG_DEALERSHIP_CATALOG 1010
#define DIALOG_INVENTORY        1011
#define DIALOG_ADMIN_VEHICLE_LIST 1012
#define DIALOG_ADMIN_VEHICLE_ACTIONS 1013
#define DIALOG_ADMIN_VEHICLE_DELETE_CONFIRM 1014
#define DIALOG_PLAYER_FIND_VEHICLE 1015
#define DIALOG_PLAYER_STATS 1016
#define DIALOG_INTERIORS_LIST 1017
#define DIALOG_BUSINESS_SHOP 1018
#define DIALOG_BUSINESS_CONFIG 1019

#define MAX_CHARACTERS_PER_USER 3

// MySQL Handle
new MySQL:g_MySQL;

// User Data (Cuenta principal)
enum E_USER_DATA
{
    uID,
    uNickname[MAX_PLAYER_NAME],
    uPassword[65],
    uSalt[32],
    uAdminLevel,
    bool:uLogged
}

new UserData[MAX_PLAYERS][E_USER_DATA];

// Character Data (Personaje activo)
enum E_CHARACTER_DATA
{
    cID,
    cUserID,
    cName[MAX_PLAYER_NAME],
    cAge,
    cGender,
    cMoney,
    cBank,
    Float:cPosX,
    Float:cPosY,
    Float:cPosZ,
    Float:cPosA,
    cInterior,
    cVirtualWorld,
    Float:cHealth,
    Float:cArmour,
    cSkin,
    bool:cSelected
}

new CharacterData[MAX_PLAYERS][E_CHARACTER_DATA];

// Vehicle list for /auto command
new const VehicleNames[][] = {
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

#define DIALOG_AUTO 1000

// Sistema de protección contra explosión de vehículos
forward CheckVehicleHealth();
public CheckVehicleHealth()
{
    new Float:health;
    for(new i = 1; i < MAX_VEHICLES; i++)
    {
        if(GetVehicleModel(i) == 0) continue; // Vehículo no válido
        
        GetVehicleHealth(i, health);
        
        // Si la salud está muy baja (por debajo de 250), mantenerla en 251
        // 250 es aproximadamente cuando empieza el fuego y la explosión
        if(health < 251.0 && health > 0.0)
        {
            SetVehicleHealth(i, 251.0);
            RepairVehicle(i); // Reparar daños visuales para que no explote
        }
    }
    return 1;
}

// Callback cuando un vehículo recibe daño
public OnVehicleDamageStatusUpdate(vehicleid, playerid)
{
    new Float:health;
    GetVehicleHealth(vehicleid, health);
    
    // Si está muy dañado, mantener vida mínima
    if(health < 251.0 && health > 0.0)
    {
        SetVehicleHealth(vehicleid, 251.0);
        RepairVehicle(vehicleid);
    }
    
    return 1;
}

// Includes de sistemas
#include "../include/character_system.inc"
#include "../include/inventory_system.inc"
#include "../include/weapon_inventory.inc"
#include "../include/dropped_items.inc"
#include "../include/business_system.inc"
#include "../include/property_system.inc"
#include "../include/vehicle_engine_system.inc"
#include "../include/vehicle_persistence.inc"
#include "../include/dealership_system.inc"
#include "../include/admin_system.inc"
#include "../include/zones/unity_station.inc"

main()
{
    print("=====================================");
    print("  San Andreas Roleplay - pabvlov");
    print("=====================================");
}

public OnGameModeInit()
{
    SetGameModeText("San Andreas Roleplay");
    EnableStuntBonusForAll(false);
    
    SetWeather(10);
    SetWorldTime(12);
    
    // Conectar a MySQL
    MySQL_Connect();
    
    // Cargar datos de armas y cargadores
    LoadWeaponData();
    LoadMagazineData();
    
    // Cargar items tirados en el suelo
    LoadDroppedItems();
    
    // Inicializar sistemas
    InitVehiclePersistence();
    
    // Unity Station spawn
    AddPlayerClass(0, 1759.0189, -1898.1260, 13.5622, 266.4503, WEAPON_FIST, 0, WEAPON_FIST, 0, WEAPON_FIST, 0);
    
    // Cargar decoración de zonas
    LoadUnityStation();
    
    // Cargar todas las propiedades
    LoadAllProperties();
    // LoadAllDealerships(); // TODO: Implementar si es necesario
    InitBusinessSystem();
    InitVehiclePersistence();
    
    // Remover pickups originales de tiendas del SA-MP
    RemoveOriginalSAMPPickups();
    
    // Iniciar timer de proteccion de vehiculos
    SetTimer("CheckVehicleHealth", 1000, true);
    
    return true;
}

public OnGameModeExit()
{
    // Cerrar sistema de vehículos
    CloseVehiclePersistence();
    
    // Cerrar conexión MySQL
    if(g_MySQL != MYSQL_INVALID_HANDLE)
    {
        mysql_close(g_MySQL);
        print("[MySQL] Conexión cerrada correctamente.");
    }
    return true;
}

public OnPlayerConnect(playerid)
{
    // Resetear datos
    UserData[playerid][uID] = 0;
    UserData[playerid][uLogged] = false;
    UserData[playerid][uAdminLevel] = 0;
    
    CharacterData[playerid][cID] = 0;
    CharacterData[playerid][cSelected] = false;
    
    PlayerInBusiness[playerid] = -1;
    
    GetPlayerName(playerid, UserData[playerid][uNickname], MAX_PLAYER_NAME);
    
    // Verificar si el usuario está registrado
    new query[256];
    mysql_format(g_MySQL, query, sizeof(query), "SELECT * FROM `users` WHERE `nickname` = '%e' LIMIT 1", UserData[playerid][uNickname]);
    mysql_tquery(g_MySQL, query, "OnUserCheckAccount", "d", playerid);
    
    return true;
}

public OnPlayerDisconnect(playerid, reason)
{
    // Guardar vehículos del jugador
    SavePlayerVehiclesOnDisconnect(playerid);
    
    if(CharacterData[playerid][cSelected])
    {
        // Guardar arma equipada antes de guardar personaje
        if(PlayerCurrentWeapon[playerid] != -1)
        {
            UnequipWeapon(playerid);
        }
        
        SaveCharacterData(playerid);
    }
    
    // Limpiar sistema de armas
    WeaponInventory_OnDisconnect(playerid);
    
    // Resetear estado de tienda
    PlayerInBusiness[playerid] = -1;
    
    new string[128];
    
    new reasonText[32];
    switch(reason)
    {
        case 0: reasonText = "Timeout/Crash";
        case 1: reasonText = "Salió";
        case 2: reasonText = "Kick/Ban";
    }
    
    if(CharacterData[playerid][cSelected])
    {
        format(string, sizeof(string), "%s se ha desconectado. [%s]", CharacterData[playerid][cName], reasonText);
    }
    else
    {
        format(string, sizeof(string), "%s se ha desconectado. [%s]", UserData[playerid][uNickname], reasonText);
    }
    SendClientMessageToAll(COLOR_GRAY, string);
    return true;
}

public OnPlayerExitVehicle(playerid, vehicleid)
{
    // Guardar posicion del vehiculo al salir
    new dbid = GetVehicleDBID(vehicleid);
    if(dbid > 0)
    {
        SaveVehiclePosition(vehicleid);
        printf("[Vehicle Persistence] Guardado vehiculo %d al salir %s", dbid, CharacterData[playerid][cName]);
    }
    return 1;
}

public OnPlayerSpawn(playerid)
{
    if(!CharacterData[playerid][cSelected])
    {
        Kick(playerid);
        return true;
    }
    
    SetPlayerPos(playerid, CharacterData[playerid][cPosX], CharacterData[playerid][cPosY], CharacterData[playerid][cPosZ]);
    SetPlayerFacingAngle(playerid, CharacterData[playerid][cPosA]);
    SetPlayerInterior(playerid, CharacterData[playerid][cInterior]);
    SetPlayerVirtualWorld(playerid, CharacterData[playerid][cVirtualWorld]);
    SetPlayerHealth(playerid, CharacterData[playerid][cHealth]);
    SetPlayerArmour(playerid, CharacterData[playerid][cArmour]);
    SetPlayerSkin(playerid, CharacterData[playerid][cSkin]);
    ResetPlayerMoney(playerid);
    GivePlayerMoney(playerid, CharacterData[playerid][cMoney]);
    SetCameraBehindPlayer(playerid);
    
    // Cargar inventario
    LoadPlayerInventory(playerid);
    
    // Cargar vehículos del personaje
    LoadCharacterVehicles(playerid);
    
    SendClientMessage(playerid, COLOR_GREEN, "Has spawneado correctamente!");
    SendClientMessage(playerid, COLOR_WHITE, "{AAAAAA}» {FFFF00}Y{AAAAAA}=Inventario | {FFFF00}N{AAAAAA}=Motor | {FFFF00}H{AAAAAA}=Entrar/Salir | {FFFF00}/ayuda{AAAAAA} para más.");
    return true;
}

public OnPlayerKeyStateChange(playerid, KEY:newkeys, KEY:oldkeys)
{
    // Tecla Y - Abrir inventario (KEY_YES)
    if((newkeys & KEY_YES) && !(oldkeys & KEY_YES))
    {
        if(CharacterData[playerid][cSelected])
        {
            new params[1];
            cmd_inventario(playerid, params);
        }
        return 1;
    }
    
    // Tecla N - Encender/Apagar motor (KEY_NO)
    if((newkeys & KEY_NO) && !(oldkeys & KEY_NO))
    {
        if(CharacterData[playerid][cSelected])
        {
            new vehicleid = GetPlayerVehicleID(playerid);
            if(vehicleid != 0 && GetPlayerVehicleSeat(playerid) == 0)
            {
                ToggleVehicleEngine(playerid, vehicleid);
            }
        }
        return 1;
    }
    
    // Tecla H - Entrar/Salir de propiedades y tiendas (KEY_CTRL_BACK)
    if((newkeys & KEY_CTRL_BACK) && !(oldkeys & KEY_CTRL_BACK))
    {
        printf("[DEBUG H] Tecla H presionada por %s", CharacterData[playerid][cName]);
        
        if(CharacterData[playerid][cSelected])
        {
            printf("[DEBUG H] Personaje seleccionado OK");
            
            // Si está dentro de una propiedad, salir
            if(PlayerInProperty[playerid] != -1)
            {
                printf("[DEBUG H] PlayerInProperty: %d - Ejecutando ExitProperty", PlayerInProperty[playerid]);
                ExitProperty(playerid);
            }
            // Si está dentro de una tienda, salir
            else if(PlayerInBusiness[playerid] != -1)
            {
                printf("[DEBUG H] PlayerInBusiness: %d - Ejecutando ExitBusiness", PlayerInBusiness[playerid]);
                ExitBusiness(playerid);
            }
            else
            {
                printf("[DEBUG H] Jugador fuera, buscando entrada cercana");
                
                // Primero intentar entrar a una tienda
                new businessSlot = GetNearbyBusiness(playerid, 3.0);
                printf("[DEBUG H] BusinessSlot encontrado: %d", businessSlot);
                
                if(businessSlot != -1)
                {
                    printf("[DEBUG H] Intentando entrar a tienda slot %d", businessSlot);
                    EnterBusiness(playerid, businessSlot);
                }
                else
                {
                    // Si no hay tienda, intentar entrar a propiedad
                    new propertySlot = GetNearestProperty(playerid);
                    printf("[DEBUG H] PropertySlot encontrado: %d", propertySlot);
                    
                    if(propertySlot != -1)
                    {
                        printf("[DEBUG H] Intentando entrar a propiedad slot %d", propertySlot);
                        EnterProperty(playerid, propertySlot);
                    }
                    else
                    {
                        printf("[DEBUG H] No hay propiedad ni tienda cercana");
                        SendClientMessage(playerid, COLOR_ERROR, "No hay ninguna entrada cercana.");
                    }
                }
            }
        }
        else
        {
            printf("[DEBUG H] Personaje NO seleccionado");
        }
        return 1;
    }
    
    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    if(!CharacterData[playerid][cSelected])
    {
        SendClientMessage(playerid, COLOR_RED, "Debes seleccionar un personaje primero!");
        return 1;
    }
    
    if(strcmp("/auto", cmdtext, true, 5) == 0)
    {
        new dialogString[3000];
        for(new i = 0; i < sizeof(VehicleNames); i++)
        {
            format(dialogString, sizeof(dialogString), "%s%d. %s\n", dialogString, 400 + i, VehicleNames[i]);
        }
        ShowPlayerDialog(playerid, DIALOG_AUTO, DIALOG_STYLE_LIST, "Selecciona un vehículo", dialogString, "Seleccionar", "Cancelar");
        return 1;
    }
    
    if(strcmp("/stats", cmdtext, true) == 0)
    {
        printf("[DEBUG] Comando /stats ejecutado por %s (ID: %d)", CharacterData[playerid][cName], CharacterData[playerid][cID]);
        
        // Consultar vehiculos, propiedades y facciones
        new query[512];
        mysql_format(g_MySQL, query, sizeof(query),
            "SELECT \
                (SELECT COUNT(*) FROM vehicles WHERE owner_type='character' AND owner_id=%d) as total_vehicles, \
                (SELECT COUNT(*) FROM properties WHERE owner_type='character' AND owner_id=%d) as total_properties, \
                (SELECT f.name FROM faction_members fm JOIN factions f ON fm.faction_id=f.id WHERE fm.character_id=%d LIMIT 1) as faction_name",
            CharacterData[playerid][cID], CharacterData[playerid][cID], CharacterData[playerid][cID]);
        
        printf("[DEBUG] Query: %s", query);
        mysql_tquery(g_MySQL, query, "OnPlayerStatsLoaded", "d", playerid);
        return 1;
    }
    
    if(strcmp("/personajes", cmdtext, true) == 0 || strcmp("/chars", cmdtext, true) == 0)
    {
        ShowCharacterList(playerid);
        return 1;
    }
    
    if(strcmp("/acmds", cmdtext, true) == 0)
    {
        new params[1];
        return cmd_acmds(playerid, params);
    }
    
    // Procesar comandos ZCMD
    new cmd[128], idx;
    cmd = strtok(cmdtext, idx);
    
    if(strcmp(cmd, "/createhouse", true) == 0) return cmd_createhouse(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/enter", true) == 0) return cmd_enter(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/exitproperty", true) == 0) return cmd_exitproperty(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/buyproperty", true) == 0) return cmd_buyproperty(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/sellproperty", true) == 0) return cmd_sellproperty(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/lock", true) == 0) return cmd_lock(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/createdealership", true) == 0) return cmd_createdealership(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/addvehiclestock", true) == 0) return cmd_addvehiclestock(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/buycar", true) == 0) return cmd_buycar(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/inventario", true) == 0 || strcmp(cmd, "/inv", true) == 0) return cmd_inventario(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/usar", true) == 0) return cmd_usar(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/guardar", true) == 0) return cmd_guardar(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/giveitem", true) == 0) return cmd_giveitem(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/motor", true) == 0) return cmd_motor(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/engine", true) == 0) return cmd_engine(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/m", true) == 0) return cmd_m(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/jetpack", true) == 0) return cmd_jetpack(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/vehiculos", true) == 0) return cmd_vehiculos(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/buscarvehiculo", true) == 0) return cmd_buscarvehiculo(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/buscarcarro", true) == 0) return cmd_buscarcarro(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/buscarauto", true) == 0) return cmd_buscarauto(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/creartienda", true) == 0) return cmd_creartienda(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/configurarbusiness", true) == 0) return cmd_configurarbusiness(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/setinteriorpos", true) == 0) return cmd_setinteriorpos(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/agregarstock", true) == 0) return cmd_agregarstock(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/comprar", true) == 0) return cmd_comprar(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/interiores", true) == 0 || strcmp(cmd, "/int", true) == 0) return cmd_interiores(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/dararma", true) == 0) return cmd_dararma(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/darcargador", true) == 0) return cmd_darcargador(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/listarmas", true) == 0) return cmd_listarmas(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/listcargadores", true) == 0) return cmd_listcargadores(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/tirar", true) == 0) return cmd_tirar(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/recoger", true) == 0) return cmd_recoger(playerid, cmdtext[idx]);
    if(strcmp(cmd, "/editattachedobject", true) == 0) return cmd_editattachedobject(playerid, cmdtext[idx]);
    
    return 0; // Comando no encontrado
}

stock strtok(const string[], &index)
{
    new length = strlen(string);
    while((index < length) && (string[index] <= ' '))
        index++;
    
    new offset = index;
    new result[128];
    while((index < length) && (string[index] > ' ') && ((index - offset) < (sizeof(result) - 1)))
    {
        result[index - offset] = string[index];
        index++;
    }
    result[index - offset] = EOS;
    return result;
}

// Función para remover pickups originales de SA-MP
stock RemoveOriginalSAMPPickups()
{
    // Remover pickups de tiendas 24/7, Ammunation, ropa, etc.
    RemoveBuildingForPlayer(0, 1239, 0.0, 0.0, 0.0, 6000.0); // 24/7
    RemoveBuildingForPlayer(0, 1242, 0.0, 0.0, 0.0, 6000.0); // Ammunation
    RemoveBuildingForPlayer(0, 1275, 0.0, 0.0, 0.0, 6000.0); // Ropa
    RemoveBuildingForPlayer(0, 1272, 0.0, 0.0, 0.0, 6000.0); // Comida
    RemoveBuildingForPlayer(0, 1274, 0.0, 0.0, 0.0, 6000.0); // Generico
    
    print("[Server] Pickups originales de SA-MP removidos");
    return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    // Dialogs del sistema de propiedades
    if(dialogid >= 2000 && dialogid < 3000)
        return OnPropertyDialogResponse(playerid, dialogid, response, listitem, inputtext);
    
    // Dialogs del sistema de concesionarias
    if(dialogid == DIALOG_DEALERSHIP_CATALOG)
        return OnDealershipPurchase(playerid, dialogid, response, listitem);
    
    // Dialogs de administración de vehículos
    if(dialogid == DIALOG_ADMIN_VEHICLE_LIST)
        return OnDialogAdminVehicleList(playerid, response, listitem);
    if(dialogid == DIALOG_ADMIN_VEHICLE_ACTIONS)
        return OnDialogAdminVehicleActions(playerid, response, listitem);
    if(dialogid == DIALOG_ADMIN_VEHICLE_DELETE_CONFIRM)
        return OnAdminVehicleDelConfirm(playerid, response);
    
    // Dialog de inventario
    if(dialogid == DIALOG_INVENTORY)
        return OnDialogInventory(playerid, response, listitem);
    
    // Dialog de negocios
    if(dialogid == DIALOG_BUSINESS_SHOP)
        return OnDialogBusinessShop(playerid, response, listitem);
    
    if(dialogid == DIALOG_BUSINESS_CONFIG)
        return OnDialogBusinessConfig(playerid, response, listitem);
    
    if(dialogid == DIALOG_BUSINESS_CONFIG + 1)
        return OnDialogBusinessCfgInteriorID(playerid, response, inputtext);
    
    if(dialogid == DIALOG_BUSINESS_CONFIG + 2)
        return 1;
    
    if(dialogid == DIALOG_BUSINESS_CONFIG + 3)
        return OnDialogBusinessCfgIcon(playerid, response, inputtext);
    
    if(dialogid == DIALOG_PLAYER_FIND_VEHICLE)
    {
        if(!response) return 1;
        
        // Recargar consulta para obtener el vehiculo seleccionado
        new query[512];
        mysql_format(g_MySQL, query, sizeof(query),
            "SELECT id, model, plate, pos_x, pos_y, pos_z \
            FROM vehicles \
            WHERE owner_type = 'character' AND owner_id = %d AND is_spawned = 1 \
            ORDER BY id ASC LIMIT %d, 1",
            CharacterData[playerid][cID], listitem);
        
        mysql_tquery(g_MySQL, query, "OnPlayerMarkVehicle", "d", playerid);
        return 1;
    }
    
    switch(dialogid)
    {
        case DIALOG_REGISTER:
        {
            if(!response) return Kick(playerid);
            
            if(strlen(inputtext) < 6 || strlen(inputtext) > 32)
            {
                SendClientMessage(playerid, COLOR_RED, "La contraseña debe tener entre 6 y 32 caracteres.");
                ShowRegisterDialog(playerid);
                return true;
            }
            
            // Generar salt aleatorio único
            new salt[32];
            format(salt, sizeof(salt), "%d%d%d", random(99999), gettime(), playerid);
            format(UserData[playerid][uSalt], 32, "%s", salt);
            
            // Hashear con SHA256
            new query[512], hash[65];
            SHA256_PassHash(inputtext, UserData[playerid][uSalt], hash, sizeof(hash));
            
            mysql_format(g_MySQL, query, sizeof(query), 
                "INSERT INTO `users` (`nickname`, `password`, `salt`) VALUES ('%e', '%e', '%e')",
                UserData[playerid][uNickname], hash, UserData[playerid][uSalt]
            );
            mysql_tquery(g_MySQL, query, "OnUserRegister", "d", playerid);
            
            SendClientMessage(playerid, COLOR_GREEN, "Cuenta creada exitosamente!");
            return true;
        }
        
        case DIALOG_LOGIN:
        {
            if(!response) return Kick(playerid);
            
            // Hashear con SHA256
            new hash[65];
            SHA256_PassHash(inputtext, UserData[playerid][uSalt], hash, sizeof(hash));
            
            if(strcmp(hash, UserData[playerid][uPassword], false) != 0)
            {
                SendClientMessage(playerid, COLOR_RED, "Contraseña incorrecta!");
                ShowLoginDialog(playerid);
                return true;
            }
            
            new query[256];
            mysql_format(g_MySQL, query, sizeof(query), "SELECT * FROM `users` WHERE `nickname` = '%e' LIMIT 1", UserData[playerid][uNickname]);
            mysql_tquery(g_MySQL, query, "OnUserLogin", "d", playerid);
            return true;
        }
        
        case DIALOG_CHARACTER_LIST:
        {
            if(!response) return true;
            
            new query[256];
            mysql_format(g_MySQL, query, sizeof(query), "SELECT * FROM `characters` WHERE `user_id` = %d ORDER BY `last_played` DESC", UserData[playerid][uID]);
            new Cache:result = mysql_query(g_MySQL, query);
            new rows = cache_num_rows();
            
            if(listitem >= rows)
            {
                // Crear nuevo personaje
                cache_delete(result);
                ShowCharacterCreate(playerid);
            }
            else
            {
                // Seleccionar personaje existente
                new charID;
                cache_get_value_name_int(listitem, "id", charID);
                cache_delete(result);
                
                mysql_format(g_MySQL, query, sizeof(query), "SELECT * FROM `characters` WHERE `id` = %d LIMIT 1", charID);
                mysql_tquery(g_MySQL, query, "OnCharacterSelect", "d", playerid);
            }
            
            return true;
        }
        
        case DIALOG_CHARACTER_CREATE:
        {
            if(!response)
            {
                ShowCharacterList(playerid);
                return true;
            }
            
            // Validar nombre RP
            if(!ValidateRPName(inputtext))
            {
                SendClientMessage(playerid, COLOR_RED, "Nombre inválido! Debe ser Nombre_Apellido con formato RP");
                SendClientMessage(playerid, COLOR_YELLOW, "Ejemplos: Pablo Prieto, Maria Garcia, John Smith");
                ShowCharacterCreate(playerid);
                return true;
            }
            
            // Guardar nombre temporalmente
            format(CharacterData[playerid][cName], MAX_PLAYER_NAME, "%s", inputtext);
            
            // Mostrar diálogo de configuración inicial
            ShowCharacterStatsDialog(playerid);
            return true;
        }
        
        case DIALOG_CHARACTER_STATS:
        {
            if(!response)
            {
                ShowCharacterCreate(playerid);
                return true;
            }
            
            switch(listitem)
            {
                case 0: // Edad
                {
                    ShowPlayerDialog(playerid, DIALOG_EDIT_AGE, DIALOG_STYLE_INPUT,
                        "Configurar Personaje - Edad",
                        "{FFFFFF}Ingresa la edad de tu personaje:\n\n\
                        {AAAAAA}Rango permitido: 18 a 80 a\xF1os",
                        "Confirmar", "Atras");
                }
                case 1: // Genero
                {
                    ShowPlayerDialog(playerid, DIALOG_EDIT_GENDER, DIALOG_STYLE_LIST,
                        "Configurar Personaje - Genero",
                        "Masculino\nFemenino",
                        "Seleccionar", "Atras");
                }
                case 2: // Skin
                {
                    ShowPlayerDialog(playerid, DIALOG_EDIT_SKIN, DIALOG_STYLE_INPUT,
                        "Configurar Personaje - Skin",
                        "{FFFFFF}Ingresa el ID del skin (0-311):\n\n\
                        {AAAAAA}Puedes elegir cualquier skin de SA-MP\n\
                        {AAAAAA}Ejemplos: 0 (CJ), 60 (Policia), 294 (Triad)",
                        "Confirmar", "Atras");
                }
                case 3: // Finalizar
                {
                    // Validar que todos los datos estén completos
                    if(CharacterData[playerid][cAge] == 0)
                    {
                        SendClientMessage(playerid, COLOR_ERROR, "Debes configurar la edad primero.");
                        ShowCharacterStatsDialog(playerid);
                        return true;
                    }
                    
                    if(CharacterData[playerid][cGender] == 0)
                    {
                        SendClientMessage(playerid, COLOR_ERROR, "Debes configurar el género primero.");
                        ShowCharacterStatsDialog(playerid);
                        return true;
                    }
                    
                    // Debug: Mostrar datos antes de insertar
                    printf("[DEBUG] Iniciando creacion de personaje:");
                    printf("[DEBUG] - user_id: %d", UserData[playerid][uID]);
                    printf("[DEBUG] - name: %s", CharacterData[playerid][cName]);
                    printf("[DEBUG] - age: %d", CharacterData[playerid][cAge]);
                    printf("[DEBUG] - gender: %d", CharacterData[playerid][cGender]);
                    printf("[DEBUG] - skin: %d", CharacterData[playerid][cSkin]);
                    
                    // Crear personaje en la base de datos
                    new query[512];
                    mysql_format(g_MySQL, query, sizeof(query), 
                        "INSERT INTO `characters` (`user_id`, `name`, `age`, `gender`, `skin`) VALUES (%d, '%e', %d, %d, %d)",
                        UserData[playerid][uID], CharacterData[playerid][cName], 
                        CharacterData[playerid][cAge], CharacterData[playerid][cGender], CharacterData[playerid][cSkin]
                    );
                    printf("[DEBUG] Query generada: %s", query);
                    mysql_tquery(g_MySQL, query, "OnCharacterCreate", "d", playerid);
                    printf("[DEBUG] mysql_tquery ejecutada, esperando callback...");
                    
                    SendClientMessage(playerid, COLOR_SUCCESS, "Creando personaje...");
                }
            }
            return 1;
        }
        
        case DIALOG_EDIT_AGE:
        {
            if(!response)
            {
                ShowCharacterStatsDialog(playerid);
                return true;
            }
            
            new age = strval(inputtext);
            if(age < 18 || age > 80)
            {
                SendClientMessage(playerid, COLOR_ERROR, "La edad debe estar entre 18 y 80 a\xF1os.");
                ShowPlayerDialog(playerid, DIALOG_EDIT_AGE, DIALOG_STYLE_INPUT,
                    "Configurar Personaje - Edad",
                    "{FFFFFF}Ingresa la edad de tu personaje:\n\n\
                    {AAAAAA}Rango permitido: 18 a 80 a\xF1os\n\
                    {FF0000}Error: Edad invalida",
                    "Confirmar", "Atras");
                return true;
            }
            
            CharacterData[playerid][cAge] = age;
            SendClientMessage(playerid, COLOR_SUCCESS, "Edad configurada correctamente.");
            ShowCharacterStatsDialog(playerid);
            return 1;
        }
        
        case DIALOG_EDIT_GENDER:
        {
            if(!response)
            {
                ShowCharacterStatsDialog(playerid);
                return true;
            }
            
            CharacterData[playerid][cGender] = listitem + 1; // 1=Masculino, 2=Femenino
            SendClientMessage(playerid, COLOR_SUCCESS, listitem == 0 ? ("Genero: Masculino") : ("Genero: Femenino"));
            ShowCharacterStatsDialog(playerid);
            return 1;
        }
        
        case DIALOG_EDIT_SKIN:
        {
            if(!response)
            {
                ShowCharacterStatsDialog(playerid);
                return true;
            }
            
            new skinid = strval(inputtext);
            if(skinid < 0 || skinid > 311 || skinid == 74) // 74 es el skin del CJ sin cabeza
            {
                SendClientMessage(playerid, COLOR_ERROR, "ID de skin invalido. Debe estar entre 0 y 311.");
                ShowPlayerDialog(playerid, DIALOG_EDIT_SKIN, DIALOG_STYLE_INPUT,
                    "Configurar Personaje - Skin",
                    "{FFFFFF}Ingresa el ID del skin (0-311):\n\n\
                    {AAAAAA}Puedes elegir cualquier skin de SA-MP\n\
                    {FF0000}Error: ID invalido",
                    "Confirmar", "Atras");
                return true;
            }
            
            CharacterData[playerid][cSkin] = skinid;
            SetPlayerSkin(playerid, skinid);
            SendClientMessage(playerid, COLOR_SUCCESS, "Skin configurado correctamente.");
            ShowCharacterStatsDialog(playerid);
            return 1;
        }
        
        case DIALOG_AUTO:
        {
            if(response)
            {
                new Float:x, Float:y, Float:z, Float:angle;
                GetPlayerPos(playerid, x, y, z);
                GetPlayerFacingAngle(playerid, angle);
                
                new vehicleid = CreateVehicle(400 + listitem, x + 3.0, y, z, angle, -1, -1, 60000);
                
                new message[128];
                format(message, sizeof(message), "Has spawneado un %s (ID: %d)", VehicleNames[listitem], 400 + listitem);
                SendClientMessage(playerid, COLOR_WHITE, message);
                
                PutPlayerInVehicle(playerid, vehicleid, 0);
            }
            return true;
        }
        
        case DIALOG_INTERIORS_LIST:
        {
            if(!response) return 1;
            
            new Float:x, Float:y, Float:z, interior, vw = 0;
            
            // Datos de interiores basados en la documentación de open.mp
            switch(listitem)
            {
                case 0: { x = -25.884498; y = -185.868988; z = 1003.546875; interior = 17; } // 24/7 Shop 1
                case 1: { x = 6.091179; y = -29.271898; z = 1003.549438; interior = 10; } // 24/7 Shop 2
                case 2: { x = -30.946699; y = -89.609596; z = 1003.546875; interior = 18; } // 24/7 Shop 3
                case 3: { x = -25.132598; y = -139.066986; z = 1003.546875; interior = 16; } // 24/7 Shop 4
                case 4: { x = -27.312299; y = -29.277599; z = 1003.557250; interior = 4; } // 24/7 Shop 5
                case 5: { x = -26.691598; y = -55.714897; z = 1003.546875; interior = 6; } // 24/7 Shop 6
                case 6: { x = -1827.147338; y = 7.207417; z = 1061.143554; interior = 3; } // Airport Ticket Desk
                case 7: { x = -1855.568725; y = 41.263156; z = 1061.143554; interior = 14; } // Airport Baggage
                case 8: { x = 1.808619; y = 32.384357; z = 1199.593750; interior = 1; } // Shamal Interior
                case 9: { x = 315.745086; y = 984.969299; z = 1958.919067; interior = 1; } // Andromada Interior
                case 10: { x = 286.148986; y = -40.644397; z = 1001.515625; interior = 1; } // Ammunation 1
                case 11: { x = 286.800994; y = -82.547599; z = 1001.515625; interior = 4; } // Ammunation 2
                case 12: { x = 296.919982; y = -108.071998; z = 1001.515625; interior = 6; } // Ammunation 3
                case 13: { x = 314.820983; y = -141.431991; z = 999.601562; interior = 7; } // Ammunation 4
                case 14: { x = 316.524993; y = -167.706985; z = 999.593750; interior = 6; } // Ammunation 5
                case 15: { x = 1038.531372; y = 0.111030; z = 1001.284484; interior = 3; } // Blastin Fools Records
                case 16: { x = 363.412994; y = -74.705497; z = 1001.507812; interior = 10; } // Burger Shot
                case 17: { x = 365.674987; y = -9.984399; z = 1001.851562; interior = 9; } // Cluckin Bell
                case 18: { x = 373.825653; y = -117.270904; z = 1001.499511; interior = 5; } // Well Stacked Pizza
                case 19: { x = 772.111999; y = -3.898649; z = 1000.728820; interior = 5; } // Gym (LS)
                case 20: { x = 774.213989; y = -48.924297; z = 1000.585937; interior = 6; } // Gym (SF)
                case 21: { x = 773.579956; y = -77.096694; z = 1000.655029; interior = 7; } // Gym (LV)
                case 22: { x = -204.439987; y = -26.453998; z = 1002.273437; interior = 16; } // Tattoo (LS)
                case 23: { x = -204.439987; y = -8.469599; z = 1002.273437; interior = 17; } // Tattoo (SF)
                case 24: { x = -204.439987; y = -43.652496; z = 1002.273437; interior = 3; } // Tattoo (LV)
                case 25: { x = 207.054992; y = -138.804992; z = 1003.507812; interior = 3; } // Victim
                case 26: { x = 161.391006; y = -93.159156; z = 1001.804687; interior = 18; } // Zip
                case 27: { x = 207.737991; y = -109.019996; z = 1005.132812; interior = 15; } // Binco
                case 28: { x = 204.332992; y = -166.694992; z = 1000.523437; interior = 14; } // Didier Sachs
                case 29: { x = -100.403900; y = -24.071399; z = 1000.718750; interior = 3; } // Prolaps
                case 30: { x = 203.777999; y = -48.492397; z = 1001.804687; interior = 1; } // Suburban
                case 31: { x = 384.808624; y = 173.804992; z = 1008.382812; interior = 3; } // Planning Dept
                case 32: { x = 246.783996; y = 63.900199; z = 1003.640625; interior = 6; } // LSPD
                case 33: { x = 246.375991; y = 107.603996; z = 1003.218750; interior = 10; } // SFPD
                case 34: { x = 288.745971; y = 169.350997; z = 1007.171875; interior = 3; } // LVPD
                case 35: { x = 246.046279; y = 65.803421; z = 1003.640625; interior = 6; } // FBI HQ
                case 36: { x = 318.564971; y = 302.497985; z = 999.148437; interior = 0; } // Area 51
                case 37: { x = 2233.893310; y = 1711.976196; z = 1011.629638; interior = 1; } // Caligula's Casino
                case 38: { x = 2019.073486; y = 1017.827819; z = 996.875000; interior = 10; } // 4 Dragons Casino
                case 39: { x = 1260.636474; y = -785.323974; z = 1091.906250; interior = 5; } // Mad Dogg Mansion
                case 40: { x = 1299.144165; y = -794.796936; z = 1084.007812; interior = 5; } // Madd Dogg Mansion 2
                case 41: { x = 318.564971; y = 1118.209960; z = 1083.882812; interior = 5; } // Crack Palace
                case 42: { x = 2543.462646; y = -1304.110596; z = 1025.070312; interior = 2; } // Big Smoke's Crack Palace
                case 43: { x = 2496.049804; y = -1692.065307; z = 1014.742187; interior = 3; } // Jeff's House
                case 44: { x = 2454.717041; y = -1700.871582; z = 1013.515197; interior = 2; } // Ryders House
                case 45: { x = 2527.654052; y = -1679.388305; z = 1015.498596; interior = 1; } // Sweet's House
                case 46: { x = 513.882507; y = -11.269994; z = 1001.565307; interior = 1; } // OG Loc's House
                case 47: { x = 2495.935302; y = -1690.689453; z = 1014.742187; interior = 3; } // CJ's House
                case 48: { x = 244.411987; y = 305.032989; z = 999.148437; interior = 1; } // Denise's Bedroom
                case 49: { x = 271.884979; y = 306.631988; z = 999.148437; interior = 2; } // Katie's Bedroom
                case 50: { x = 291.282989; y = 310.031982; z = 999.148437; interior = 3; } // Helena's Bedroom
                case 51: { x = 322.197998; y = 302.497985; z = 999.148437; interior = 4; } // Barbara's Bedroom
                case 52: { x = 346.870025; y = 309.259033; z = 999.155700; interior = 4; } // Michelle's Bedroom
                case 53: { x = 346.339996; y = 309.310028; z = 999.155700; interior = 6; } // Millie's Bedroom
            }
            
            SetPlayerPos(playerid, x, y, z);
            SetPlayerInterior(playerid, interior);
            SetPlayerVirtualWorld(playerid, vw);
            
            new string[128];
            format(string, sizeof(string), "{00FF00}[TP]{FFFFFF} Teleportado al interior. Int: %d | VW: %d", interior, vw);
            SendClientMessage(playerid, COLOR_SUCCESS, string);
            return 1;
        }
    }
    return false;
}

// Callback para mostrar estadisticas
forward OnPlayerStatsLoaded(playerid);
public OnPlayerStatsLoaded(playerid)
{
    if(!IsPlayerConnected(playerid)) return 0;
    
    new total_vehicles, total_properties;
    new faction_name[64];
    
    if(cache_num_rows() > 0)
    {
        cache_get_value_name_int(0, "total_vehicles", total_vehicles);
        cache_get_value_name_int(0, "total_properties", total_properties);
        cache_get_value_name(0, "faction_name", faction_name, 64);
    }
    
    new string[1024];
    format(string, sizeof(string), 
        "{FFFFFF}=== {00FF00}Estadisticas de %s{FFFFFF} ===\n\n\
        {FFD700}[Cuenta]\n\
        {FFFFFF}Usuario: {FFFF00}%s {AAAAAA}(ID: %d)\n\
        {FFFFFF}Nivel Admin: {FF0000}%d\n\n\
        {FFD700}[Personaje]\n\
        {FFFFFF}ID: {AAAAAA}%d\n\
        {FFFFFF}Edad: {AAAAAA}%d anos\n\
        {FFFFFF}Genero: {AAAAAA}%s\n\
        {FFFFFF}Skin: {AAAAAA}%d\n\n\
        {FFD700}[Economia]\n\
        {FFFFFF}Dinero en mano: {00FF00}$%d\n\
        {FFFFFF}Banco: {00FF00}$%d\n\n\
        {FFD700}[Posesiones]\n\
        {FFFFFF}Vehiculos: {FFFF00}%d\n\
        {FFFFFF}Propiedades: {FFFF00}%d\n\
        {FFFFFF}Faccion: {FFFF00}%s",
        CharacterData[playerid][cName],
        UserData[playerid][uNickname], UserData[playerid][uID],
        UserData[playerid][uAdminLevel],
        CharacterData[playerid][cID],
        CharacterData[playerid][cAge],
        CharacterData[playerid][cGender] == 1 ? "Masculino" : "Femenino",
        CharacterData[playerid][cSkin],
        CharacterData[playerid][cMoney],
        CharacterData[playerid][cBank],
        total_vehicles,
        total_properties,
        strlen(faction_name) > 0 ? faction_name : "Ninguna"
    );
    
    ShowPlayerDialog(playerid, DIALOG_PLAYER_STATS, DIALOG_STYLE_MSGBOX, "Estadisticas", string, "Cerrar", "");
    
    printf("[Stats] %s consulto sus stats (Vehiculos: %d, Propiedades: %d)", 
        CharacterData[playerid][cName], total_vehicles, total_properties);
    
    return 1;
}

// ==================== MySQL Functions ====================

MySQL_Connect()
{
    print("[MySQL] Iniciando conexión...");
    printf("[MySQL] Host: %s:%d", MYSQL_HOST, MYSQL_PORT);
    printf("[MySQL] User: %s", MYSQL_USER);
    printf("[MySQL] Database: %s", MYSQL_DATABASE);
    
    // Usar MySQLOpt para especificar puerto personalizado
    new MySQLOpt:options = mysql_init_options();
    mysql_set_option(options, SERVER_PORT, MYSQL_PORT);
    mysql_set_option(options, AUTO_RECONNECT, true);
    
    g_MySQL = mysql_connect(MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, options);
    
    printf("[MySQL] Handle obtenido: %d", _:g_MySQL);
    
    if(g_MySQL == MYSQL_INVALID_HANDLE || _:g_MySQL == 0)
    {
        print("====================================");
        print("[MySQL] ERROR: No se pudo conectar!");
        print("====================================");
        return false;
    }
    
    new errno = mysql_errno(g_MySQL);
    printf("[MySQL] Error code: %d", errno);
    
    if(errno == 2019)
    {
        print("[MySQL] Error 2019: Charset incompatible");
        return false;
    }
    
    if(errno != 0)
    {
        printf("[MySQL] ERROR: Código de error %d", errno);
        return false;
    }
    
    print("====================================");
    print("[MySQL] ¡Conexión exitosa a MySQL 5.7!");
    printf("[MySQL] Base de datos: %s", MYSQL_DATABASE);
    print("====================================");
    
    MySQL_CreateTables();
    return true;
}

MySQL_CreateTables()
{
    mysql_tquery(g_MySQL, "CREATE TABLE IF NOT EXISTS `users` (\
        `id` INT NOT NULL AUTO_INCREMENT,\
        `nickname` VARCHAR(24) NOT NULL,\
        `password` VARCHAR(64) NOT NULL,\
        `salt` VARCHAR(32) NOT NULL,\
        `email` VARCHAR(100) DEFAULT NULL,\
        `admin_level` INT DEFAULT 0,\
        `registered` DATETIME DEFAULT CURRENT_TIMESTAMP,\
        `last_login` DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
        PRIMARY KEY (`id`),\
        UNIQUE KEY `nickname` (`nickname`)\
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8;");
    
    mysql_tquery(g_MySQL, "CREATE TABLE IF NOT EXISTS `characters` (\
        `id` INT NOT NULL AUTO_INCREMENT,\
        `user_id` INT NOT NULL,\
        `name` VARCHAR(24) NOT NULL,\
        `money` INT DEFAULT 5000,\
        `bank` INT DEFAULT 10000,\
        `pos_x` FLOAT DEFAULT 1759.0189,\
        `pos_y` FLOAT DEFAULT -1898.1260,\
        `pos_z` FLOAT DEFAULT 13.5622,\
        `pos_a` FLOAT DEFAULT 266.4503,\
        `interior` INT DEFAULT 0,\
        `virtual_world` INT DEFAULT 0,\
        `health` FLOAT DEFAULT 100.0,\
        `armour` FLOAT DEFAULT 0.0,\
        `skin` INT DEFAULT 0,\
        `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP,\
        `last_played` DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\
        PRIMARY KEY (`id`),\
        UNIQUE KEY `name` (`name`)\
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8;");
    
    print("[MySQL] Tablas verificadas/creadas correctamente.");
}



