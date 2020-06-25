#include "stdafx.h"

#include "questlua.h"
#include "questmanager.h"

#undef sys_err
#ifndef _WIN32
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	int32_t quest_start(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		q.GetCurrentPC()->SetCurrentQuestStartFlag();
		return 0;
	}

	int32_t quest_done(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		q.GetCurrentPC()->SetCurrentQuestDoneFlag();
		return 0;
	}

	int32_t quest_set_title(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		if (lua_isstring(L,-1))
			q.GetCurrentPC()->SetCurrentQuestTitle(lua_tostring(L,-1));

		return 0;
	}

	int32_t quest_set_another_title(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		if (lua_isstring(L,1) && lua_isstring(L,2))
			q.GetCurrentPC()->SetQuestTitle(lua_tostring(L,1),lua_tostring(L,2));

		return 0;
	}

	int32_t quest_set_clock_name(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		if (lua_isstring(L,-1))
			q.GetCurrentPC()->SetCurrentQuestClockName(lua_tostring(L,-1));

		return 0;
	}

	int32_t quest_set_clock_value(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		if (lua_isnumber(L,-1))
			q.GetCurrentPC()->SetCurrentQuestClockValue((int32_t)rint(lua_tonumber(L,-1)));

		return 0;
	}

	int32_t quest_set_counter_name(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		if (lua_isstring(L,-1))
			q.GetCurrentPC()->SetCurrentQuestCounterName(lua_tostring(L,-1));

		return 0;
	}

	int32_t quest_set_counter_value(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		if (lua_isnumber(L,-1))
			q.GetCurrentPC()->SetCurrentQuestCounterValue((int32_t)rint(lua_tonumber(L,-1)));

		return 0;
	}

	int32_t quest_set_icon_file(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		if (lua_isstring(L,-1))
			q.GetCurrentPC()->SetCurrentQuestIconFile(lua_tostring(L,-1));

		return 0;
	}

	int32_t quest_setstate(lua_State* L)
	{
		if (lua_tostring(L, -1)==nullptr)
		{
			sys_err("state name is empty");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		QuestState * pqs = q.GetCurrentState();
		PC* pPC = q.GetCurrentPC();

		if (L!=pqs->co)
		{
			luaL_error(L, "running thread != current thread???");
			if ( test_server )
				sys_log(0 ,"running thread != current thread???");
			return 0;
		}

		if (pPC)
		{
			std::string stCurrentState = lua_tostring(L,-1);
			if ( test_server )
				sys_log ( 0 ,"questlua->setstate( %s, %s )", pPC->GetCurrentQuestName().c_str(), stCurrentState.c_str() );
			pqs->st = q.GetQuestStateIndex(pPC->GetCurrentQuestName(), stCurrentState);
			pPC->SetCurrentQuestStateName(stCurrentState );
		}
		return 0;
	}

	int32_t quest_coroutine_yield(lua_State * L)
	{
		CQuestManager& q = CQuestManager::instance();
		if (q.IsInOtherPCBlock())
		{
			sys_err("FATAL ERROR! Yield occur in other_pc_block.");
			PC* pPC = q.GetOtherPCBlockRootPC();
			if (nullptr == pPC)
			{
				sys_err("	... FFFAAATTTAAALLL Error. RootPC is nullptr");
				return 0;
			}
			QuestState* pQS = pPC->GetRunningQuestState();
			if (nullptr == pQS || nullptr == q.GetQuestStateName(pPC->GetCurrentQuestName(), pQS->st))
			{
				sys_err("	... WHO AM I? WHERE AM I? I only know QuestName(%s)...", pPC->GetCurrentQuestName().c_str());
			}
			else
			{
				sys_err("	Current Quest(%s). State(%s)", pPC->GetCurrentQuestName().c_str(), q.GetQuestStateName(pPC->GetCurrentQuestName(), pQS->st));
			}
			return 0;
		}
		return lua_yield(L, lua_gettop(L));
	}

	int32_t quest_no_send(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		q.SetNoSend();
		return 0;
	}

	int32_t quest_get_current_quest_index(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		PC* pPC = q.GetCurrentPC();

		int32_t idx = q.GetQuestIndexByName(pPC->GetCurrentQuestName());
		lua_pushnumber(L, idx);
		return 1;
	}

#ifdef ENABLE_NEWSTUFF //@correction232
	int32_t quest_get_current_quest_name(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		PC* pPC = q.GetCurrentPC();

		lua_pushstring(L, pPC->GetCurrentQuestName().c_str());
		return 1;
	}
#endif

	int32_t quest_begin_other_pc_block(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		uint32_t pid = lua_tonumber(L, -1);
		q.BeginOtherPCBlock(pid);
		return 0;
	}

	int32_t quest_end_other_pc_block(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		q.EndOtherPCBlock();
		return 0;
	}

	void RegisterQuestFunctionTable()
	{
		luaL_reg quest_functions[] =
		{
			{ "setstate",				quest_setstate				},
			{ "set_state",				quest_setstate				},
			{ "yield",					quest_coroutine_yield		},
			{ "set_title",				quest_set_title				},
			{ "set_title2",				quest_set_another_title		},
			{ "set_clock_name",			quest_set_clock_name		},
			{ "set_clock_value",		quest_set_clock_value		},
			{ "set_counter_name",		quest_set_counter_name		},
			{ "set_counter_value",		quest_set_counter_value		},
			{ "set_icon",				quest_set_icon_file			},
			{ "start",					quest_start					},
			{ "done",					quest_done					},
			{ "getcurrentquestindex",	quest_get_current_quest_index	},
#ifdef ENABLE_NEWSTUFF //@correction232
			{ "getcurrentquestname",	quest_get_current_quest_name	},
#endif
			{ "no_send",				quest_no_send				},
			{ "begin_other_pc_block",	quest_begin_other_pc_block	},
			{ "end_other_pc_block",		quest_end_other_pc_block	},
			{ nullptr,						nullptr						}
		};

		CQuestManager::instance().AddLuaFunctionTable("q", quest_functions);
	}
}




