#include "stdafx.h"
#include "questlua.h"
#include "questmanager.h"
#include "desc_client.h"
#include "char.h"
#include "item_manager.h"
#include "item.h"
#include "cmd.h"
#include "packet.h"
#ifdef  __MINI_GAME_CATCH_KING__
#include "db.h"
#endif

#undef sys_err
#ifndef _WIN32
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

extern ACMD(do_in_game_mall);

namespace quest
{
	int32_t game_set_event_flag(lua_State* L)
	{
		CQuestManager & q = CQuestManager::instance();

		if (lua_isstring(L,1) && lua_isnumber(L, 2))
			q.RequestSetEventFlag(lua_tostring(L,1), (int32_t)lua_tonumber(L,2));

		return 0;
	}

	int32_t game_get_event_flag(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		if (lua_isstring(L,1))
			lua_pushnumber(L, q.GetEventFlag(lua_tostring(L,1)));
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t game_request_make_guild(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPDESC d = q.GetCurrentCharacterPtr()->GetDesc();
		if (d)
		{
			uint8_t header = HEADER_GC_REQUEST_MAKE_GUILD;
			d->Packet(&header, 1);
		}
		return 0;
	}

	int32_t game_get_safebox_level(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		lua_pushnumber(L, q.GetCurrentCharacterPtr()->GetSafeboxSize()/SAFEBOX_PAGE_SIZE);
		return 1;
	}

	int32_t game_set_safebox_level(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();

		TSafeboxChangeSizePacket p;
		p.dwID = q.GetCurrentCharacterPtr()->GetDesc()->GetAccountTable().id;
		p.bSize = (int32_t)lua_tonumber(L,-1);
		db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_SIZE,  q.GetCurrentCharacterPtr()->GetDesc()->GetHandle(), &p, sizeof(p));

		q.GetCurrentCharacterPtr()->SetSafeboxSize(SAFEBOX_PAGE_SIZE * (int32_t)lua_tonumber(L,-1));
		return 0;
	}

	int32_t game_open_safebox(lua_State*)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		ch->SetSafeboxOpenPosition();
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSafeboxPassword");
		return 0;
	}

	int32_t game_open_mall(lua_State*)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		ch->SetSafeboxOpenPosition();
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
		return 0;
	}

	int32_t game_drop_item(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		uint32_t item_vnum = (uint32_t) lua_tonumber(L, 1);
		int32_t count = (int32_t) lua_tonumber(L, 2);
		int32_t x = ch->GetX();
		int32_t y = ch->GetY();

		LPITEM item = ITEM_MANAGER::instance().CreateItem(item_vnum, count);

		if (!item)
		{
			sys_err("cannot create item vnum %d count %d", item_vnum, count);
			return 0;
		}

		PIXEL_POSITION pos;
		pos.x = x + number(-200, 200);
		pos.y = y + number(-200, 200);

		item->AddToGround(ch->GetMapIndex(), pos);
		item->StartDestroyEvent();

		return 0;
	}

	int32_t game_drop_item_with_ownership(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		LPITEM item = nullptr;
		switch (lua_gettop(L))
		{
		case 1:
			item = ITEM_MANAGER::instance().CreateItem((uint32_t) lua_tonumber(L, 1));
			break;
		case 2:
		case 3:
			item = ITEM_MANAGER::instance().CreateItem((uint32_t) lua_tonumber(L, 1), (int32_t) lua_tonumber(L, 2));
			break;
		default:
			return 0;
		}

		if ( item == nullptr )
		{
			return 0;
		}

		if (lua_isnumber(L, 3))
		{
			int32_t sec = (int32_t) lua_tonumber(L, 3);
			if (sec <= 0)
			{
				item->SetOwnership( ch );
			}
			else
			{
				item->SetOwnership( ch, sec );
			}
		}
		else
			item->SetOwnership( ch );

		PIXEL_POSITION pos;
		pos.x = ch->GetX() + number(-200, 200);
		pos.y = ch->GetY() + number(-200, 200);

		item->AddToGround(ch->GetMapIndex(), pos);
		item->StartDestroyEvent();

		return 0;
	}

	int32_t game_web_mall(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if ( ch != nullptr )
		{
			do_in_game_mall(ch, const_cast<char*>(""), 0, 0);
		}
		return 0;
	}

#ifdef __MINI_GAME_CATCH_KING__
	int32_t mini_game_catch_king_get_score(lua_State* L)
	{
		uint32_t dwArg = (uint32_t)lua_tonumber(L, 1);
		bool isTotal = dwArg ? true : false;

		uint32_t index = 1;
		lua_newtable(L);

		char querySelect[256];

		if (isTotal)
			snprintf(querySelect, sizeof(querySelect), "SELECT name, empire, total_score FROM catck_king_event ORDER BY total_score DESC LIMIT 10;");
		else
			snprintf(querySelect, sizeof(querySelect), "SELECT name, empire, max_score FROM catck_king_event ORDER BY max_score DESC LIMIT 10;");

		std::unique_ptr<SQLMsg> pSelectMsg(DBManager::instance().DirectQuery(querySelect));
		SQLResult* resSelect = pSelectMsg->Get();

		if (resSelect && resSelect->uiNumRows > 0)
		{
			for (uint i = 0; i < resSelect->uiNumRows; i++)
			{
				MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
				uint8_t bEmpire = 0;
				uint32_t dwScore = 0;

				str_to_number(bEmpire, row[1]);
				str_to_number(dwScore, row[2]);

				lua_newtable(L);

				lua_pushstring(L, row[0]);
				lua_rawseti(L, -2, 1);

				lua_pushnumber(L, bEmpire);
				lua_rawseti(L, -2, 2);

				lua_pushnumber(L, dwScore);
				lua_rawseti(L, -2, 3);

				lua_rawseti(L, -2, index++);
			}
		}

		return 1;
	}
#endif


#ifdef __12ZI__
	int32_t game_open_reward(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT * FROM player.zodiac_reward WHERE name = '%s' ORDER BY time ASC;", ch->GetName()));

		if (pMsg->Get()->uiNumRows > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacReward %d %d %d %d %d", atoi(row[1]), atoi(row[2]), atoi(row[3]), atoi(row[4]), atoi(row[5]));
		}
		else
			ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacReward 0 0 0 0 0");

		return 0;
	}

	int32_t game_open_zodiac_minimap(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacMinimap");
		return 0;
	}

#ifdef __RANKING__
	int32_t game_open_zodiac_ranking(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		int32_t portal = (int32_t)lua_tonumber(L, 1);

		TPacketRanking packet;
		packet.bHeader = HEADER_GC_RANKING;
		TEMP_BUFFER buf;

		TInfoMap Map = CRankingManager::instance().GetRankingInfo();
		for (int32_t i = 0; i < PARTY_RK_CATEGORY_MAX + SOLO_RK_CATEGORY_MAX; ++i)
			for (TRankingInfoMap::iterator it = Map[i].begin(); it != Map[i].end(); ++it)
				buf.write(&it->second, sizeof(it->second));

		packet.wSize = sizeof(packet) + buf.size();

		ch->GetDesc()->BufferedPacket(&packet, sizeof(packet));
		ch->GetDesc()->Packet(buf.read_peek(), buf.size());

		ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenRankingBoard 1 %d", portal + 2 + SOLO_RK_CATEGORY_MAX);
		return 0;
	}
	int32_t game_add_zodiac_ranking(lua_State* L)
	{
		TRankingInfo Info;

		Info.bCategory = (int32_t)lua_tonumber(L, 1) + 2 + SOLO_RK_CATEGORY_MAX;
		Info.iRankingIdx = CRankingManager::instance().GetSpaceLine(Info.bCategory);
		strlcpy(Info.szCharName, lua_tostring(L, 2), sizeof(Info.szCharName));
		strlcpy(Info.szMembersName, lua_tostring(L, 3), sizeof(Info.szMembersName));
		Info.iRecord0 = lua_tonumber(L, 4);
		Info.iRecord1 = lua_tonumber(L, 5);
		Info.iTime = lua_tonumber(L, 6);
		Info.bEmpire = lua_tonumber(L, 7);
		CRankingManager::instance().AddRankingInfo(Info.bCategory, Info.iRankingIdx, Info);

		return 0;
	}
#endif
#endif
	void RegisterGameFunctionTable()
	{
		luaL_reg game_functions[] =
		{
			{ "get_safebox_level",			game_get_safebox_level			},
			{ "request_make_guild",			game_request_make_guild			},
			{ "set_safebox_level",			game_set_safebox_level			},
			{ "open_safebox",				game_open_safebox				},
			{ "open_mall",					game_open_mall					},
			{ "get_event_flag",				game_get_event_flag				},
			{ "set_event_flag",				game_set_event_flag				},
			{ "drop_item",					game_drop_item					},
			{ "drop_item_with_ownership",	game_drop_item_with_ownership	},
			{ "open_web_mall",				game_web_mall					},
#ifdef __MINI_GAME_CATCH_KING__
			{ "mini_game_catch_king_get_score",	mini_game_catch_king_get_score		},
#endif
#ifdef __12ZI__
			{ "open_zodiac_reward",			game_open_reward				},
			{ "open_zodiac_minimap",		game_open_zodiac_minimap		},
#ifdef __RANKING__
			{ "open_zodiac_ranking",		game_open_zodiac_ranking		},
			{ "add_zodiac_ranking",			game_add_zodiac_ranking			},
#endif
#endif
			{ nullptr,					nullptr				}
		};

		CQuestManager::instance().AddLuaFunctionTable("game", game_functions);
	}
}

