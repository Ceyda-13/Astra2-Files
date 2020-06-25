#include "stdafx.h"

#include "questlua.h"
#include "questmanager.h"
#include "desc_client.h"
#include "char.h"
#include "char_manager.h"
#include "utils.h"
#include "guild.h"
#include "guild_manager.h"

#undef sys_err
#ifndef _WIN32
#define sys_err(fmt, args...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, ##args)
#else
#define sys_err(fmt, ...) quest::CQuestManager::instance().QuestError(__FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#endif

namespace quest
{
	int32_t guild_around_ranking_string(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch->GetGuild())
			lua_pushstring(L,"");
		else
		{
			char szBuf[4096+1];
			CGuildManager::instance().GetAroundRankString(ch->GetGuild()->GetID(), szBuf, sizeof(szBuf));
			lua_pushstring(L, szBuf);
		}
		return 1;
	}

	int32_t guild_high_ranking_string(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		uint32_t dwMyGuild = 0;
		if (ch->GetGuild())
			dwMyGuild = ch->GetGuild()->GetID();

		char szBuf[4096+1];
		CGuildManager::instance().GetHighRankString(dwMyGuild, szBuf, sizeof(szBuf));
		lua_pushstring(L, szBuf);
		return 1;
	}

	int32_t guild_get_ladder_point(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();
		if (!ch->GetGuild())
		{
			lua_pushnumber(L, -1);
		}
		else
		{
			lua_pushnumber(L, ch->GetGuild()->GetLadderPoint());
		}
		return 1;
	}

	int32_t guild_get_rank(lua_State* L)
	{
		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (!ch->GetGuild())
		{
			lua_pushnumber(L, -1);
		}
		else
		{
			lua_pushnumber(L, CGuildManager::instance().GetRank(ch->GetGuild()));
		}
		return 1;
	}

	int32_t guild_is_war(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch->GetGuild() && ch->GetGuild()->UnderWar((uint32_t) lua_tonumber(L, 1)))
			lua_pushboolean(L, true);
		else
			lua_pushboolean(L, false);

		return 1;
	}

	int32_t guild_name(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		CGuild * pkGuild = CGuildManager::instance().FindGuild((uint32_t) lua_tonumber(L, 1));

		if (pkGuild)
			lua_pushstring(L, pkGuild->GetName());
		else
			lua_pushstring(L, "");

		return 1;
	}

	int32_t guild_level(lua_State* L)
	{
		luaL_checknumber(L, 1);

		CGuild * pkGuild = CGuildManager::instance().FindGuild((uint32_t) lua_tonumber(L, 1));

		if (pkGuild)
			lua_pushnumber(L, pkGuild->GetLevel());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t guild_war_enter(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			return 0;
		}

		CQuestManager& q = CQuestManager::instance();
		LPCHARACTER ch = q.GetCurrentCharacterPtr();

		if (ch->GetGuild())
			ch->GetGuild()->GuildWarEntryAccept((uint32_t) lua_tonumber(L, 1), ch);

		return 0;
	}

	int32_t guild_get_any_war(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if (ch->GetGuild())
			lua_pushnumber(L, ch->GetGuild()->UnderAnyWar());
		else
			lua_pushnumber(L, 0);

		return 1;
	}

	int32_t guild_get_name(lua_State * L)
	{
		if (!lua_isnumber(L, 1))
		{
			lua_pushstring(L,  "");
			return 1;
		}

		CGuild * pkGuild = CGuildManager::instance().FindGuild((uint32_t) lua_tonumber(L, 1));

		if (pkGuild)
			lua_pushstring(L, pkGuild->GetName());
		else
			lua_pushstring(L, "");

		return 1;
	}

	int32_t guild_war_bet(lua_State * L)
	{
		if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
		{
			sys_err("invalid argument");
			return 0;
		}

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		TPacketGDGuildWarBet p;

		p.dwWarID = (uint32_t) lua_tonumber(L, 1);
		strlcpy(p.szLogin, ch->GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
		p.dwGuild = (uint32_t) lua_tonumber(L, 2);
		p.dwGold = (uint32_t) lua_tonumber(L, 3);

		sys_log(0, "GUILD_WAR_BET: %s login %s war_id %u guild %u gold %u",
				ch->GetName(), p.szLogin, p.dwWarID, p.dwGuild, p.dwGold);

		db_clientdesc->DBPacket(HEADER_GD_GUILD_WAR_BET, 0, &p, sizeof(p));
		return 0;
	}

	int32_t guild_is_bet(lua_State * L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("invalid argument");
			lua_pushboolean(L, true);
			return 1;
		}

		bool bBet = CGuildManager::instance().IsBet((uint32_t) lua_tonumber(L, 1),
				CQuestManager::instance().GetCurrentCharacterPtr()->GetDesc()->GetAccountTable().login);

		lua_pushboolean(L, bBet);
		return 1;
	}

	int32_t guild_get_warp_war_list(lua_State* L)
	{
		FBuildLuaGuildWarList f(L);
		CGuildManager::instance().for_each_war(f);
		return 1;
	}

	int32_t guild_get_reserve_war_table(lua_State * L)
	{
		std::vector<CGuildWarReserveForGame *> & con = CGuildManager::instance().GetReserveWarRef();

		int32_t i = 0;
		std::vector<CGuildWarReserveForGame *>::iterator it = con.begin();

		sys_log(0, "con.size(): %d", con.size());

		lua_newtable(L);

		while (it != con.end())
		{
			TGuildWarReserve * p = &(*(it++))->data;

			if (p->bType != GUILD_WAR_TYPE_BATTLE)
				continue;

			lua_newtable(L);

			sys_log(0, "con.size(): %u %u %u handi %d", p->dwID, p->dwGuildFrom, p->dwGuildTo, p->lHandicap);

			lua_pushnumber(L, p->dwID);
			lua_rawseti(L, -2, 1);

			if (p->lPowerFrom > p->lPowerTo)
				lua_pushnumber(L, p->dwGuildFrom);
			else
				lua_pushnumber(L, p->dwGuildTo);
			lua_rawseti(L, -2, 2);

			if (p->lPowerFrom > p->lPowerTo)
				lua_pushnumber(L, p->dwGuildTo);
			else
				lua_pushnumber(L, p->dwGuildFrom);
			lua_rawseti(L, -2, 3);

			lua_pushnumber(L, p->lHandicap);
			lua_rawseti(L, -2, 4);

			lua_rawseti(L, -2, ++i);
		}

		return 1;
	}

	int32_t guild_get_member_count(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		if ( ch == nullptr )
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		CGuild* pGuild = ch->GetGuild();

		if ( pGuild == nullptr )
		{
			lua_pushnumber(L, 0);
			return 1;
		}

		lua_pushnumber(L, pGuild->GetMemberCount());

		return 1;
	}

	int32_t guild_change_master(lua_State* L)
	{

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		if ( pGuild != nullptr )
		{
			if ( pGuild->GetMasterPID() == ch->GetPlayerID() )
			{
				if ( lua_isstring(L, 1) == false )
				{
					lua_pushnumber(L, 0);
				}
				else
				{
					bool ret = pGuild->ChangeMasterTo(pGuild->GetMemberPID(lua_tostring(L, 1)));

					lua_pushnumber(L, ret == false ? 2 : 3 );
				}
			}
			else
			{
				lua_pushnumber(L, 1);
			}
		}
		else
		{
			lua_pushnumber(L, 4);
		}

		return 1;
	}

	int32_t guild_change_master_with_limit(lua_State* L)
	{

		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		if ( pGuild != nullptr )
		{
			if ( pGuild->GetMasterPID() == ch->GetPlayerID() )
			{
				if ( lua_isstring(L, 1) == false )
				{
					lua_pushnumber(L, 0);
				}
				else
				{
					LPCHARACTER pNewMaster = CHARACTER_MANAGER::instance().FindPC( lua_tostring(L,1) );

					if ( pNewMaster != nullptr )
					{
						if ( pNewMaster->GetLevel() < lua_tonumber(L, 2) )
						{
							lua_pushnumber(L, 6);
						}
						else
						{
							int32_t nBeOtherLeader = pNewMaster->GetQuestFlag("change_guild_master.be_other_leader");
							CQuestManager::instance().GetPC( ch->GetPlayerID() );

							if ( lua_toboolean(L, 6) == true ) nBeOtherLeader = 0;

							if ( nBeOtherLeader > get_global_time() )
							{
								lua_pushnumber(L, 7);
							}
							else
							{
								bool ret = pGuild->ChangeMasterTo(pGuild->GetMemberPID(lua_tostring(L, 1)));

								if ( ret == false )
								{
									lua_pushnumber(L, 2);
								}
								else
								{
									lua_pushnumber(L, 3);

									pNewMaster->SetQuestFlag("change_guild_master.be_other_leader", 0);
									pNewMaster->SetQuestFlag("change_guild_master.be_other_member", 0);
									pNewMaster->SetQuestFlag("change_guild_master.resign_limit", (int32_t)lua_tonumber(L, 3));

									ch->SetQuestFlag("change_guild_master.be_other_leader", (int32_t)lua_tonumber(L, 4));
									ch->SetQuestFlag("change_guild_master.be_other_member", (int32_t)lua_tonumber(L, 5));
									ch->SetQuestFlag("change_guild_master.resign_limit", 0);
								}
							}
						}
					}
					else
					{
						lua_pushnumber(L, 5);
					}
				}
			}
			else
			{
				lua_pushnumber(L, 1);
			}
		}
		else
		{
			lua_pushnumber(L, 4);
		}

		return 1;
	}

#ifdef ENABLE_NEWSTUFF //@correction232
	int32_t guild_get_id0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild!=nullptr)?pGuild->GetID():0);
		return 1;
	}

	int32_t guild_get_sp0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild!=nullptr)?pGuild->GetSP():0);
		return 1;
	}

	int32_t guild_get_maxsp0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild!=nullptr)?pGuild->GetMaxSP():0);
		return 1;
	}

	int32_t guild_get_money0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild!=nullptr)?pGuild->GetGuildMoney():0);
		return 1;
	}

	int32_t guild_get_max_member0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild!=nullptr)?pGuild->GetMaxMemberCount():0);
		return 1;
	}

	int32_t guild_get_total_member_level0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild!=nullptr)?pGuild->GetTotalLevel():0);
		return 1;
	}

	int32_t guild_has_land0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushboolean(L, (pGuild!=nullptr)?pGuild->HasLand():false);
		return 1;
	}

	int32_t guild_get_win_count0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild!=nullptr)?pGuild->GetGuildWarWinCount():0);
		return 1;
	}

	int32_t guild_get_draw_count0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild!=nullptr)?pGuild->GetGuildWarDrawCount():0);
		return 1;
	}

	int32_t guild_get_loss_count0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		lua_pushnumber(L, (pGuild!=nullptr)?pGuild->GetGuildWarLossCount():0);
		return 1;
	}

	int32_t guild_add_comment0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();
		if (pGuild)
			pGuild->AddComment(ch, std::string(lua_tostring(L, 1)));
		return 0;
	}

	int32_t guild_set_ladder_point0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();
		if (pGuild)
			pGuild->ChangeLadderPoint(lua_tonumber(L, 1));
		return 0;
	}

	int32_t guild_get_skill_level0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();
		lua_pushnumber(L, (pGuild)?pGuild->GetSkillLevel(lua_tonumber(L, 1)):0);
		return 1;
	}

	int32_t guild_set_skill_level0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();
		if (pGuild)
			pGuild->SetSkillLevel(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_isnumber(L, 3)?lua_tonumber(L, 3):0);
		return 0;
	}

	int32_t guild_get_skill_point0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();
		lua_pushnumber(L, (pGuild)?pGuild->GetSkillPoint():0);
		return 1;
	}

	int32_t guild_set_skill_point0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();
		if (pGuild)
			pGuild->SetSkillPoint(lua_tonumber(L, 1));
		return 0;
	}

	int32_t guild_get_exp_level0(lua_State* L)
	{
		lua_pushnumber(L, guild_exp_table2[MINMAX(0, lua_tonumber(L, 1) ,GUILD_MAX_LEVEL)]);
		return 1;
	}

	int32_t guild_offer_exp0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		if (!pGuild)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		uint32_t offer = lua_tonumber(L, 1);

		if (pGuild->GetLevel() >= GUILD_MAX_LEVEL)
		{
			lua_pushboolean(L, false);
		}
		else
		{
			offer /= 100;
			offer *= 100;

			if (pGuild->OfferExp(ch, offer))
			{
				lua_pushboolean(L, true);
			}
			else
			{
				lua_pushboolean(L, false);
			}
		}
		return 1;
	}

	int32_t guild_give_exp0(lua_State* L)
	{
		LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();

		CGuild* pGuild = ch->GetGuild();

		if (!pGuild)
			return 0;

		pGuild->GuildPointChange(POINT_EXP, lua_tonumber(L, 1) / 100, true);
		return 1;
	}

#endif

	void RegisterGuildFunctionTable()
	{
		luaL_reg guild_functions[] =
		{
			{ "get_rank",				guild_get_rank				},
			{ "get_ladder_point",		guild_get_ladder_point		},
			{ "high_ranking_string",	guild_high_ranking_string	},
			{ "around_ranking_string",	guild_around_ranking_string	},
			{ "name",					guild_name					},
			{ "level",					guild_level					},
			{ "is_war",					guild_is_war				},
			{ "war_enter",				guild_war_enter				},
			{ "get_any_war",			guild_get_any_war			},
			{ "get_reserve_war_table",	guild_get_reserve_war_table	},
			{ "get_name",				guild_get_name				},
			{ "war_bet",				guild_war_bet				},
			{ "is_bet",					guild_is_bet				},
			{ "get_warp_war_list",		guild_get_warp_war_list		},
			{ "get_member_count",		guild_get_member_count		},
			{ "change_master",			guild_change_master			},
			{ "change_master_with_limit",			guild_change_master_with_limit			},
#ifdef ENABLE_NEWSTUFF //@correction232
			{ "get_id0",				guild_get_id0				},
			{ "get_sp0",				guild_get_sp0				},
			{ "get_maxsp0",				guild_get_maxsp0			},
			{ "get_money0",				guild_get_money0			},
			{ "get_max_member0",		guild_get_max_member0		},
			{ "get_total_member_level0",	guild_get_total_member_level0	},
			{ "has_land0",				guild_has_land0				},
			{ "get_win_count0",			guild_get_win_count0		},
			{ "get_draw_count0",		guild_get_draw_count0		},
			{ "get_loss_count0",		guild_get_loss_count0		},
			{ "add_comment0",			guild_add_comment0			},
			{ "set_ladder_point0",		guild_set_ladder_point0		},
			{ "get_skill_level0",		guild_get_skill_level0		},
			{ "set_skill_level0",		guild_set_skill_level0		},
			{ "get_skill_point0",		guild_get_skill_point0		},
			{ "set_skill_point0",		guild_set_skill_point0		},
			{ "get_exp_level0",			guild_get_exp_level0		},
			{ "offer_exp0",				guild_offer_exp0			},
			{ "give_exp0",				guild_give_exp0				},
#endif
			{ nullptr,						nullptr						}
		};

		CQuestManager::instance().AddLuaFunctionTable("guild", guild_functions);
	}
}

