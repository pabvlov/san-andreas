/*
    Sistema de Heridas y Agonía Realista
    - Cada impacto de bala se registra por bodypart
    - Daño varía según arma y parte del cuerpo
    - Las heridas se acumulan y se muestran en label 3D
    - Al llegar a 10hp, el jugador entra en agonía (animación en el piso)
    - Puede pedir morir o esperar ayuda médica
    - Al morir completamente, reaparece en hospital después de 60s

    Comandos:
    - /heridas (id)        -> Muestra las heridas del jugador
    - /pedirmorir          -> Solicita morir cuando estás en agonía
    - /aceptarmorir <id>   -> Acepta la solicitud de morir de un jugador cercano
    - /reaparecer          -> Reaparecer en hospital si el tiempo de espera terminó
*/

#if defined _AGONIZE_INC
    #endinput
#endif
#define _AGONIZE_INC

#define AGONY_MIN_HP 10.0
#define AGONY_PICKUP_DISTANCE 5.0
#define AGONY_RESPAWN_WAIT 60
#define MAX_WOUNDS_DISPLAY 50

// Bodyparts según SA-MP
#define BODYPART_TORSO 3
#define BODYPART_GROIN 4
#define BODYPART_LEFT_ARM 5
#define BODYPART_RIGHT_ARM 6
#define BODYPART_LEFT_LEG 7
#define BODYPART_RIGHT_LEG 8
#define BODYPART_HEAD 9

enum E_AGONY_STATE
{
    AGONY_STATE_NORMAL,
    AGONY_STATE_CRITICAL,  // En agonía (10hp, en el piso)
    AGONY_STATE_DEAD       // Muerto (esperando respawn)
}

enum E_WOUND_DATA
{
    wBodypart,
    wWeaponId,
    Float:wDamage
}

new E_AGONY_STATE:PlayerAgonyState[MAX_PLAYERS];
new PlayerWounds[MAX_PLAYERS][MAX_WOUNDS_DISPLAY][E_WOUND_DATA];
new PlayerWoundCount[MAX_PLAYERS];
new bool:PlayerDeathRequested[MAX_PLAYERS];
new PlayerRespawnAvailableAt[MAX_PLAYERS];
new Float:PlayerDeathX[MAX_PLAYERS];
new Float:PlayerDeathY[MAX_PLAYERS];
new Float:PlayerDeathZ[MAX_PLAYERS];
new Text3D:WoundLabelId[MAX_PLAYERS];

new Float:AGONY_HOSPITAL_X = 1838.0;
new Float:AGONY_HOSPITAL_Y = 3688.0;
new Float:AGONY_HOSPITAL_Z = 14.0;

stock InitAgonySystem()
{
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        PlayerAgonyState[i] = AGONY_STATE_NORMAL;
        PlayerWoundCount[i] = 0;
        PlayerDeathRequested[i] = false;
        PlayerRespawnAvailableAt[i] = 0;
        WoundLabelId[i] = Text3D:INVALID_3DTEXT_ID;
    }
    SetTimer("Agony_Tick", 1000, true);
    printf("[Agony] Sistema de heridas inicializado");
    return 1;
}

forward Agony_Tick();
public Agony_Tick()
{
    for(new i = 0; i < MAX_PLAYERS; i++)
    {
        if(!IsPlayerConnected(i)) continue;
        if(!CharacterData[i][cSelected]) continue;

        new Float:hp;
        GetPlayerHealth(i, hp);

        // Si el jugador está normal y llega a 10hp, entra en agonía
        if(PlayerAgonyState[i] == AGONY_STATE_NORMAL && hp <= AGONY_MIN_HP)
        {
            EnterCriticalState(i);
        }

        // Si está muerto, mantener animación y 1hp
        if(PlayerAgonyState[i] == AGONY_STATE_DEAD)
        {
            SetPlayerHealth(i, 1.0);
            #pragma warning push
            #pragma warning disable 214
            ApplyAnimation(i, "CRACK", "crckdeth2", 4.1, false, true, true, true, 0, true);
            #pragma warning pop
        }

        // Actualizar label si tiene heridas
        if(PlayerWoundCount[i] > 0)
        {
            Wounds_UpdateLabel(i);
        }
    }
    return 1;
}

stock Wounds_UpdateLabel(playerid)
{
    if(!IsPlayerConnected(playerid)) return 0;

    // Si no tiene heridas, eliminar label
    if(PlayerWoundCount[playerid] == 0)
    {
        if(WoundLabelId[playerid] != Text3D:INVALID_3DTEXT_ID)
        {
            Delete3DTextLabel(WoundLabelId[playerid]);
            WoundLabelId[playerid] = Text3D:INVALID_3DTEXT_ID;
        }
        return 0;
    }

    // Contar heridas por bodypart
    new headWounds = 0, torsoWounds = 0, groinWounds = 0;
    new armWounds = 0, legWounds = 0;

    for(new i = 0; i < PlayerWoundCount[playerid]; i++)
    {
        switch(PlayerWounds[playerid][i][wBodypart])
        {
            case BODYPART_HEAD: headWounds++;
            case BODYPART_TORSO: torsoWounds++;
            case BODYPART_GROIN: groinWounds++;
            case BODYPART_LEFT_ARM, BODYPART_RIGHT_ARM: armWounds++;
            case BODYPART_LEFT_LEG, BODYPART_RIGHT_LEG: legWounds++;
        }
    }

    new labelText[128];
    format(labelText, sizeof(labelText), "{FF0000}Heridas: ");
    
    if(headWounds > 0) format(labelText, sizeof(labelText), "%sCabeza:%d ", labelText, headWounds);
    if(torsoWounds > 0) format(labelText, sizeof(labelText), "%sTorso:%d ", labelText, torsoWounds);
    if(groinWounds > 0) format(labelText, sizeof(labelText), "%sIngle:%d ", labelText, groinWounds);
    if(armWounds > 0) format(labelText, sizeof(labelText), "%sBrazos:%d ", labelText, armWounds);
    if(legWounds > 0) format(labelText, sizeof(labelText), "%sPiernas:%d", labelText, legWounds);

    // Eliminar label anterior
    if(WoundLabelId[playerid] != Text3D:INVALID_3DTEXT_ID)
    {
        Delete3DTextLabel(WoundLabelId[playerid]);
        WoundLabelId[playerid] = Text3D:INVALID_3DTEXT_ID;
    }

    // Crear nuevo label adjunto al jugador
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    WoundLabelId[playerid] = Create3DTextLabel(labelText, 0xFF0000FF, x, y, z + 0.5, 20.0, 0, true);
    Attach3DTextLabelToPlayer(WoundLabelId[playerid], playerid, 0.0, 0.0, 0.5);

    return 1;
}

stock RegisterWound(playerid, bodypart, weaponid, Float:damage)
{
    if(PlayerWoundCount[playerid] >= MAX_WOUNDS_DISPLAY) return 0;

    new idx = PlayerWoundCount[playerid];
    PlayerWounds[playerid][idx][wBodypart] = bodypart;
    PlayerWounds[playerid][idx][wWeaponId] = weaponid;
    PlayerWounds[playerid][idx][wDamage] = damage;
    PlayerWoundCount[playerid]++;

    // Guardar en base de datos
    if(CharacterData[playerid][cSelected])
    {
        new query[256];
        mysql_format(g_MySQL, query, sizeof(query),
            "INSERT INTO character_wounds (character_id, bodypart, weapon_id, damage) VALUES (%d, %d, %d, %.2f)",
            CharacterData[playerid][cID], bodypart, weaponid, damage);
        mysql_tquery(g_MySQL, query);
    }

    Wounds_UpdateLabel(playerid);
    printf("[Agony] Jugador %d herida registrada: Bodypart %d, Arma %d, Daño %.2f", playerid, bodypart, weaponid, damage);
    return 1;
}

stock ClearWounds(playerid)
{
    PlayerWoundCount[playerid] = 0;
    
    // Limpiar de base de datos
    if(CharacterData[playerid][cSelected])
    {
        new query[128];
        mysql_format(g_MySQL, query, sizeof(query),
            "UPDATE character_wounds SET healed = 1 WHERE character_id = %d AND healed = 0",
            CharacterData[playerid][cID]);
        mysql_tquery(g_MySQL, query);
    }

    Wounds_UpdateLabel(playerid);
    return 1;
}

stock EnterCriticalState(playerid)
{
    if(!IsPlayerConnected(playerid)) return 0;
    PlayerAgonyState[playerid] = AGONY_STATE_CRITICAL;
    PlayerDeathRequested[playerid] = false;

    SetPlayerHealth(playerid, AGONY_MIN_HP);
    
    // Guardar posición actual para mantener al jugador en el lugar
    GetPlayerPos(playerid, PlayerDeathX[playerid], PlayerDeathY[playerid], PlayerDeathZ[playerid]);
    
    // Animación aleatoria de agonía en el piso (sincronizada para todos)
    ClearAnimations(playerid);
    #pragma warning push
    #pragma warning disable 214
    if(random(2) == 0)
        ApplyAnimation(playerid, "CRACK", "crckidle4", 4.1, true, false, false, false, 0, true);
    else
        ApplyAnimation(playerid, "SWAT", "gnstwall_injurd", 4.1, true, false, false, false, 0, true);
    #pragma warning pop

    SendClientMessage(playerid, 0xFF6600FF, "Estás en agonía. No puedes moverte. Usa /pedirmorir o espera ayuda.");
    
    printf("[Agony] Jugador %d entró en estado AGONÍA", playerid);
    return 1;
}

stock ExitAgony(playerid)
{
    if(!IsPlayerConnected(playerid)) return 0;
    PlayerAgonyState[playerid] = AGONY_STATE_NORMAL;
    PlayerDeathRequested[playerid] = false;

    if(WoundLabelId[playerid] != Text3D:INVALID_3DTEXT_ID)
    {
        Delete3DTextLabel(WoundLabelId[playerid]);
        WoundLabelId[playerid] = Text3D:INVALID_3DTEXT_ID;
    }

    // Solo quitar animación y parar al jugador en su posición actual
    ClearAnimations(playerid);
    SetPlayerVelocity(playerid, 0.0, 0.0, 0.0);

    printf("[Agony] Jugador %d salió del estado de agonía", playerid);
    return 1;
}

stock CompleteKill(playerid)
{
    if(!IsPlayerConnected(playerid)) return 0;

    GetPlayerPos(playerid, PlayerDeathX[playerid], PlayerDeathY[playerid], PlayerDeathZ[playerid]);
    PlayerAgonyState[playerid] = AGONY_STATE_DEAD;
    PlayerRespawnAvailableAt[playerid] = gettime() + AGONY_RESPAWN_WAIT;
    PlayerDeathRequested[playerid] = false;

    // Dejar al jugador en el piso con 1hp (no despawnear)
    SetPlayerHealth(playerid, 1.0);
    
    // Aplicar animación de muerto en el piso
    ClearAnimations(playerid);
    #pragma warning push
    #pragma warning disable 214
    ApplyAnimation(playerid, "CRACK", "crckdeth2", 4.1, false, true, true, true, 0, true);
    #pragma warning pop
    
    SendClientMessage(playerid, 0xFF0000FF, "Has muerto. Podrás reaparecer en %d segundos con /reaparecer", AGONY_RESPAWN_WAIT);
    printf("[Agony] Jugador %d murió completamente", playerid);
    return 1;
}

stock GetBodypartName(bodypart, output[], size = sizeof(output))
{
    switch(bodypart)
    {
        case BODYPART_TORSO: format(output, size, "Torso");
        case BODYPART_GROIN: format(output, size, "Ingle");
        case BODYPART_LEFT_ARM: format(output, size, "Brazo Izquierdo");
        case BODYPART_RIGHT_ARM: format(output, size, "Brazo Derecho");
        case BODYPART_LEFT_LEG: format(output, size, "Pierna Izquierda");
        case BODYPART_RIGHT_LEG: format(output, size, "Pierna Derecha");
        case BODYPART_HEAD: format(output, size, "Cabeza");
        default: format(output, size, "Desconocido");
    }
    return 1;
}

// ========== COMANDOS ==========

CMD:heridas(playerid, params[])
{
    if(!CharacterData[playerid][cSelected])
        return SendClientMessage(playerid, 0xFF0000FF, "Debes estar logueado.");

    new target = playerid;
    if(sscanf(params, "d", target)) target = playerid;

    if(!IsPlayerConnected(target))
        return SendClientMessage(playerid, 0xFF0000FF, "Jugador inválido.");

    if(PlayerWoundCount[target] == 0)
    {
        SendClientMessage(playerid, 0xFFFFFFFF, "El jugador no tiene heridas.");
        return 1;
    }

    new targetName[MAX_PLAYER_NAME];
    GetPlayerName(target, targetName, sizeof(targetName));

    // Obtener estado
    new estadoStr[32];
    switch(PlayerAgonyState[target])
    {
        case AGONY_STATE_NORMAL: estadoStr = "Normal";
        case AGONY_STATE_CRITICAL: estadoStr = "En Agonía";
        case AGONY_STATE_DEAD: estadoStr = "Muerto";
    }

    // Crear tabla con heridas detalladas
    new dialogStr[2048];
    format(dialogStr, sizeof(dialogStr), "Parte del Cuerpo\tArma\tDaño\n");

    new string[128], weaponName[32], bodypartName[32];
    for(new i = 0; i < PlayerWoundCount[target]; i++)
    {
        GetBodypartName(PlayerWounds[target][i][wBodypart], bodypartName, sizeof(bodypartName));
        #pragma warning push
        #pragma warning disable 214
        GetWeaponName(PlayerWounds[target][i][wWeaponId], weaponName, sizeof(weaponName));
        #pragma warning pop
        
        format(string, sizeof(string), "%s\t%s\t%.1f HP\n", 
            bodypartName, 
            weaponName, 
            PlayerWounds[target][i][wDamage]);
        strcat(dialogStr, string);
    }

    new dialogTitle[128];
    format(dialogTitle, sizeof(dialogTitle), "Heridas de %s - Estado: %s", targetName, estadoStr);
    
    ShowPlayerDialog(playerid, 9999, DIALOG_STYLE_TABLIST_HEADERS, dialogTitle, dialogStr, "Cerrar", "");
    return 1;
}

CMD:pedirmorir(playerid, params[])
{
    if(!CharacterData[playerid][cSelected])
        return SendClientMessage(playerid, 0xFF0000FF, "Debes estar logueado.");

    if(PlayerAgonyState[playerid] != AGONY_STATE_CRITICAL)
        return SendClientMessage(playerid, 0xFF0000FF, "Solo puedes pedir morir en estado de agonía.");

    PlayerDeathRequested[playerid] = true;

    SendClientMessage(playerid, 0xFFFF00FF, "Has solicitado morir. Un jugador cercano puede usar /aceptarmorir %d", playerid);
    SendClientMessageToAll(0xAAAAAAFF, "{FFFF00}Solicitud de morir: {FFFFFF}Jugador %d - usa /aceptarmorir <id> si estás cerca", playerid);
    return 1;
}

CMD:aceptarmorir(playerid, params[])
{
    if(!CharacterData[playerid][cSelected])
        return SendClientMessage(playerid, 0xFF0000FF, "Debes estar logueado.");

    new target;
    if(sscanf(params, "d", target))
        return SendClientMessage(playerid, 0xFF0000FF, "Uso: /aceptarmorir <id>");

    if(!IsPlayerConnected(target))
        return SendClientMessage(playerid, 0xFF0000FF, "Jugador inválido.");

    if(!PlayerDeathRequested[target])
        return SendClientMessage(playerid, 0xFF0000FF, "El jugador no ha solicitado morir.");

    new Float:px, Float:py, Float:pz, Float:tx, Float:ty, Float:tz;
    GetPlayerPos(playerid, px, py, pz);
    GetPlayerPos(target, tx, ty, tz);
    new Float:dist = floatsqroot(((px - tx)*(px - tx)) + ((py - ty)*(py - ty)) + ((pz - tz)*(pz - tz)));

    if(dist > AGONY_PICKUP_DISTANCE)
        return SendClientMessage(playerid, 0xFF0000FF, "Estás muy lejos del jugador para aceptar.");

    CompleteKill(target);
    SendClientMessage(playerid, 0x00FF00FF, "Has aceptado la solicitud de morir de %d", target);
    return 1;
}

CMD:reaparecer(playerid, params[])
{
    if(!CharacterData[playerid][cSelected])
        return SendClientMessage(playerid, 0xFF0000FF, "Debes estar logueado.");

    if(PlayerAgonyState[playerid] != AGONY_STATE_DEAD)
        return SendClientMessage(playerid, 0xFF0000FF, "No estás muerto.");

    if(PlayerRespawnAvailableAt[playerid] > gettime())
        return SendClientMessage(playerid, 0xFF0000FF, "Aún no puedes reaparecer. Espera %d segundos.", PlayerRespawnAvailableAt[playerid] - gettime());

    SetPlayerPos(playerid, AGONY_HOSPITAL_X, AGONY_HOSPITAL_Y, AGONY_HOSPITAL_Z);
    SetPlayerHealth(playerid, 100.0);
    
    ExitAgony(playerid);
    ClearWounds(playerid);

    SendClientMessage(playerid, 0x00FF00FF, "Has reaparecido en el hospital.");
    printf("[Agony] Jugador %d reapareció en hospital", playerid);
    return 1;
}

// ========== FUNCIONES DE CALLBACK (deben ser llamadas desde derby.pwn) ==========

stock Agony_OnPlayerTakeDamage(playerid, issuerid, Float:amount, weaponid, bodypart)
{
    printf("[Agony Debug] OnPlayerTakeDamage: playerid=%d, issuerid=%d, amount=%.2f, weapon=%d, bodypart=%d", 
        playerid, issuerid, amount, weaponid, bodypart);

    // Si está muerto, es completamente inmortal - rechazar daño inmediatamente
    if(PlayerAgonyState[playerid] == AGONY_STATE_DEAD)
    {
        SetPlayerHealth(playerid, 1.0);
        printf("[Agony Debug] Jugador muerto - daño rechazado (inmortal)");
        return 0;
    }

    // Obtener vida y armadura actual
    new Float:health, Float:armour;
    GetPlayerHealth(playerid, health);
    GetPlayerArmour(playerid, armour);

    // Calcular cuánto daño va al chaleco y cuánto a la vida
    new Float:damageToArmour = 0.0;
    new Float:damageToHealth = 0.0;

    if(armour > 0.0)
    {
        // El chaleco absorbe el daño primero
        if(amount >= armour)
        {
            // El daño excede el chaleco
            damageToArmour = armour;
            damageToHealth = amount - armour;
        }
        else
        {
            // El chaleco absorbe todo el daño
            damageToArmour = amount;
            damageToHealth = 0.0;
        }
    }
    else
    {
        // Sin chaleco, todo el daño va a la vida
        damageToHealth = amount;
    }

    new Float:newHealth = health - damageToHealth;

    printf("[Agony Debug] Vida: %.2f, Armadura: %.2f, Daño total: %.2f", health, armour, amount);
    printf("[Agony Debug] Daño a armadura: %.2f, Daño a vida: %.2f, Nueva vida: %.2f", damageToArmour, damageToHealth, newHealth);

    // Detectar armas de alto calibre
    new bool:highCaliberWeapon = false;
    switch(weaponid)
    {
        case 24: highCaliberWeapon = true; // Desert Eagle
        case 30: highCaliberWeapon = true; // AK-47
        case 31: highCaliberWeapon = true; // M4
        case 33: highCaliberWeapon = true; // Rifle de Caza
        case 34: highCaliberWeapon = true; // Rifle de Francotirador
        case 38: highCaliberWeapon = true; // Minigun
    }

    // Pistola silenciada: headshot crítico solo a menos de 10 metros
    if(weaponid == 23 && bodypart == BODYPART_HEAD && issuerid != INVALID_PLAYER_ID)
    {
        new Float:px, Float:py, Float:pz, Float:tx, Float:ty, Float:tz;
        GetPlayerPos(issuerid, px, py, pz);
        GetPlayerPos(playerid, tx, ty, tz);
        new Float:distance = floatsqroot(((px - tx)*(px - tx)) + ((py - ty)*(py - ty)) + ((pz - tz)*(pz - tz)));
        
        if(distance <= 10.0)
        {
            highCaliberWeapon = true;
            printf("[Agony Debug] Pistola silenciada headshot a %.2f metros - crítico activado", distance);
        }
        else
        {
            printf("[Agony Debug] Pistola silenciada headshot a %.2f metros - fuera de rango crítico", distance);
        }
    }

    // Registrar herida solo si es un disparo válido
    if(bodypart >= BODYPART_TORSO && bodypart <= BODYPART_HEAD)
    {
        printf("[Agony Debug] Registrando herida - bodypart válido: %d", bodypart);
        RegisterWound(playerid, bodypart, weaponid, amount);
    }
    else
    {
        printf("[Agony Debug] Bodypart inválido o no es disparo: %d", bodypart);
    }

    // Manejo de estados
    switch(PlayerAgonyState[playerid])
    {
        case AGONY_STATE_CRITICAL:
        {
            // Si está en agonía y recibe daño, muere (remate)
            SetPlayerHealth(playerid, 10.0); // Mantener en 10hp durante agonía
            CompleteKill(playerid);
            printf("[Agony Debug] Jugador rematado - cambiando a estado DEAD");
            return 0; // Cancelar daño original
        }
        case AGONY_STATE_NORMAL:
        {
            // HEADSHOT INSTAKILL: Armas de alto calibre en la cabeza = agonía instantánea (10hp)
            if(bodypart == BODYPART_HEAD)
            {
                new bool:isHeadshot = false;
                
                // Armas de alto calibre siempre hacen headshot
                if(highCaliberWeapon)
                {
                    isHeadshot = true;
                }
                // Pistola silenciada solo si está a menos de 10 metros
                else if(weaponid == 23 && issuerid != INVALID_PLAYER_ID)
                {
                    new Float:x1, Float:y1, Float:z1, Float:x2, Float:y2, Float:z2;
                    GetPlayerPos(playerid, x1, y1, z1);
                    GetPlayerPos(issuerid, x2, y2, z2);
                    new Float:distance = floatsqroot(
                        ((x1 - x2) * (x1 - x2)) +
                        ((y1 - y2) * (y1 - y2)) +
                        ((z1 - z2) * (z1 - z2))
                    );
                    if(distance <= 10.0)
                    {
                        isHeadshot = true;
                        printf("[Agony Debug] HEADSHOT SIGILOSO - Pistola silenciada a %.2f metros", distance);
                    }
                }
                
                if(isHeadshot)
                {
                    SetPlayerHealth(playerid, 10.0);
                    SetPlayerArmour(playerid, 0.0);
                    EnterCriticalState(playerid);
                    printf("[Agony Debug] HEADSHOT CRÍTICO - Arma %d en cabeza - entrando en agonía", weaponid);
                    return 0; // Cancelar daño original
                }
            }
            
            // Solo entrar en agonía si el daño afecta la vida (no solo el chaleco)
            if(damageToHealth > 0.0 && newHealth <= 10.0)
            {
                SetPlayerHealth(playerid, 10.0);
                SetPlayerArmour(playerid, 0.0); // Quitar chaleco restante
                EnterCriticalState(playerid);
                printf("[Agony Debug] Jugador bloqueado en 10hp - entrando en estado crítico");
                return 0; // Cancelar daño original
            }
        }
    }

    return 1; // Permitir daño normal
}

stock Agony_OnPlayerDeath(playerid, killerid, reason)
{
    #pragma unused killerid
    #pragma unused reason
    
    // Guardar posición de muerte si no está ya muerto
    if(PlayerAgonyState[playerid] != AGONY_STATE_DEAD)
    {
        GetPlayerPos(playerid, PlayerDeathX[playerid], PlayerDeathY[playerid], PlayerDeathZ[playerid]);
        CompleteKill(playerid);
    }

    return 1;
}

stock Agony_OnPlayerUpdate(playerid)
{
    // Si está en agonía o muerto, mantenerlo en su posición pero permitir rotación de cámara
    if(PlayerAgonyState[playerid] == AGONY_STATE_CRITICAL || PlayerAgonyState[playerid] == AGONY_STATE_DEAD)
    {
        new Float:x, Float:y, Float:z;
        GetPlayerPos(playerid, x, y, z);
        
        // Si se movió más de 0.5 unidades, devolverlo a su posición
        new Float:distance = floatsqroot(
            ((x - PlayerDeathX[playerid]) * (x - PlayerDeathX[playerid])) +
            ((y - PlayerDeathY[playerid]) * (y - PlayerDeathY[playerid]))
        );
        
        if(distance > 0.5)
        {
            SetPlayerPos(playerid, PlayerDeathX[playerid], PlayerDeathY[playerid], PlayerDeathZ[playerid]);
            SetPlayerVelocity(playerid, 0.0, 0.0, 0.0);
        }
    }
    return 1;
}

stock Agony_OnPlayerSpawn(playerid)
{
    // Si está muerto, spawnearlo en la posición de muerte con animación
    if(PlayerAgonyState[playerid] == AGONY_STATE_DEAD)
    {
        SetPlayerPos(playerid, PlayerDeathX[playerid], PlayerDeathY[playerid], PlayerDeathZ[playerid]);
        SetPlayerHealth(playerid, 1.0);
        ClearAnimations(playerid);
        #pragma warning push
        #pragma warning disable 214
        ApplyAnimation(playerid, "CRACK", "crckdeth2", 4.1, false, true, true, true, 0, true);
        #pragma warning pop
    }
    return 1;
}
