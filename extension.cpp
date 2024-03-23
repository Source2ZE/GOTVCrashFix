/**
 * =============================================================================
 * GOTVCrashFix
 * Copyright (C) 2023-2024 Poggu
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "extension.h"
#include "iserver.h"
#include <entity2/entitysystem.h>
#include <cstdint>

struct GameSessionConfiguration_t {
public:
	char pad[0x64]; // 0x0
	int maxPlayers; // 0x64
};

SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t&, ISource2WorldSession*, const char*);

GOTVCrashFix g_GOTVCrashFix;
IServerGameDLL* server = NULL;
IServerGameClients* gameclients = NULL;
IVEngineServer* engine = NULL;
IGameEventManager2* gameevents = NULL;
ICvar* icvar = NULL;

CGameEntitySystem* GameEntitySystem()
{
	return nullptr;
}

// Should only be called within the active game loop (i e map should be loaded and active)
// otherwise that'll be nullptr!
CGlobalVars* GetGameGlobals()
{
	INetworkGameServer* server = g_pNetworkServerService->GetIGameServer();

	if (!server)
		return nullptr;

	return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}

PLUGIN_EXPOSE(GOTVCrashFix, g_GOTVCrashFix);
bool GOTVCrashFix::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, g_pSource2ServerConfig, ISource2ServerConfig, INTERFACEVERSION_SERVERCONFIG);
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pGameResourceServiceServer, IGameResourceServiceServer, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);

	// Currently doesn't work from within mm side, use GetGameGlobals() in the mean time instead
	// gpGlobals = ismm->GetCGlobals();

	// Required to get the IMetamodListener events
	g_SMAPI->AddListener(this, this);

	SH_ADD_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &GOTVCrashFix::Hook_StartupServer, false);

	g_pCVar = icvar;
	ConVar_Register(FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL);

	return true;
}

bool GOTVCrashFix::Unload(char* error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &GOTVCrashFix::Hook_StartupServer, false);

	return true;
}

void GOTVCrashFix::Hook_StartupServer(const GameSessionConfiguration_t& config, ISource2WorldSession* session, const char*)
{
	auto cvar = g_pCVar->GetConVar(g_pCVar->FindConVar("tv_enable"));
	if ((*(CVValue_t*)&cvar->values).m_bValue)
	{
		ConMsg("GOTV is enabled, expanding game session maxplayers (%i -> %i)\n", config.maxPlayers, config.maxPlayers + 1);
		const_cast<GameSessionConfiguration_t&>(config).maxPlayers++;
	}
}

void GOTVCrashFix::AllPluginsLoaded()
{
}

void GOTVCrashFix::OnLevelInit(char const* pMapName,
	char const* pMapEntities,
	char const* pOldLevel,
	char const* pLandmarkName,
	bool loadGame,
	bool background)
{
}

void GOTVCrashFix::OnLevelShutdown()
{
}

bool GOTVCrashFix::Pause(char* error, size_t maxlen)
{
	return true;
}

bool GOTVCrashFix::Unpause(char* error, size_t maxlen)
{
	return true;
}

const char* GOTVCrashFix::GetLicense()
{
	return "GPLv3";
}

const char* GOTVCrashFix::GetVersion()
{
	return "1.0.0";
}

const char* GOTVCrashFix::GetDate()
{
	return __DATE__;
}

const char* GOTVCrashFix::GetLogTag()
{
	return "GOTVCRASHFIX";
}

const char* GOTVCrashFix::GetAuthor()
{
	return "Poggu";
}

const char* GOTVCrashFix::GetDescription()
{
	return "Fixes crashes caused by GOTV not being accounted for inside player limit checks";
}

const char* GOTVCrashFix::GetName()
{
	return "GOTVCrashFix";
}

const char* GOTVCrashFix::GetURL()
{
	return "https://poggu.me/";
}