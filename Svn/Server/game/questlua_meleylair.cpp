#include "stdafx.h"
#undef sys_err
#ifndef _WIN32
	#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
	#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

#ifdef __GUILD_DRAGONLAIR__
#include "questlua.h"
#include "questmanager.h"
#include "meleylair.h"
#include "char.h"
#include "guild.h"

namespace quest
{
	int32_t MeleyLair_GetPartecipantsLimit(lua_State* L)
	{
		lua_pushnumber(L, (int32_t)(PARTECIPANTS_LIMIT));
		return 1;
	}
	
	int32_t MeleyLair_GetSubMapIndex(lua_State* L)
	{
		lua_pushnumber(L, (int32_t)(SUBMAP_INDEX));
		return 1;
	}

	int32_t MeleyLair_Enter(lua_State* L)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (pkChar)
		{
			bool bTry = CMgr::instance().Enter(pkChar);
			lua_pushboolean(L, bTry);
		}
		else
		{
			lua_pushboolean(L, false);
			lua_pushnumber(L, 1);
		}
		
		return 2;
	}
	
	int32_t MeleyLair_IsMeleyMap(lua_State* L)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		bool bTry = pkChar != nullptr ? CMgr::instance().IsMeleyMap(pkChar->GetMapIndex()) : false;
		lua_pushboolean(L, bTry);
		
		return 1;
	}
	
	int32_t MeleyLair_Check(lua_State* L)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (pkChar)
			CMgr::instance().Check(pkChar);
		
		return 0;
	}
	
	int32_t MeleyLair_Leave(lua_State* L)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (pkChar)
			CMgr::instance().LeaveRequest(pkChar);
		
		return 0;
	}
	
	int32_t MeleyLair_CanGetReward(lua_State* L)
	{
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		bool bTry = pkChar != nullptr ? CMgr::instance().CanGetReward(pkChar) : false;
		lua_pushboolean(L, bTry);
		
		return 1;
	}
	
	int32_t MeleyLair_Reward(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
			return 0;
		
		uint8_t bReward = (uint8_t)lua_tonumber(L, 1);
		LPCHARACTER pkChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (pkChar)
			CMgr::instance().Reward(pkChar, bReward);
		
		return 0;
	}
	
	int32_t MeleyLair_OpenRanking(lua_State* L)
	{
		CMgr::instance().OpenRanking(CQuestManager::instance().GetCurrentCharacterPtr());
		return 0;
	}
	
	void RegisterMeleyLairFunctionTable()
	{
		luaL_reg functions[] =
		{
			{"GetPartecipantsLimit", MeleyLair_GetPartecipantsLimit},
			{"GetSubMapIndex", MeleyLair_GetSubMapIndex},
			{"Enter", MeleyLair_Enter},
			{"IsMeleyMap", MeleyLair_IsMeleyMap},
			{"Check", MeleyLair_Check},
			{"Leave", MeleyLair_Leave},
			{"CanGetReward", MeleyLair_CanGetReward},
			{"Reward", MeleyLair_Reward},
			{"OpenRanking", MeleyLair_OpenRanking},
			{nullptr, nullptr}
		};
		
		CQuestManager::instance().AddLuaFunctionTable("MeleyLair", functions);
	}
}
#endif
