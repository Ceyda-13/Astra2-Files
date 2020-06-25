#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item_manager.h"
#include "sectree_manager.h"
#include "mob_manager.h"
#include "packet.h"
#include "cmd.h"
#include "regen.h"
#include "guild.h"
#include "guild_manager.h"
#include "p2p.h"
#include "buffer_manager.h"
#include "fishing.h"
#include "mining.h"
#include "questmanager.h"
#include "vector.h"
#include "affect.h"
#include "db.h"
#include "priv_manager.h"
#include "building.h"
#include "battle.h"
#include "arena.h"
#include "start_position.h"
#include "party.h"
#include "monarch.h"
#include "castle.h"
#include "BattleArena.h"
#include "xmas_event.h"
#include "log.h"
#include "pcbang.h"
#include "threeway_war.h"
#include "unique_item.h"
#include "DragonSoul.h"

#ifdef __SUPPORT__
#include "supportsystem.h"
#endif

#ifdef __OFFLINE_PRIVATE_SHOP__
#include "shop_manager.h"
#endif

extern bool DropEvent_RefineBox_SetValue(const std::string& name, int32_t value);

enum
{
	COMMANDAFFECT_STUN,
	COMMANDAFFECT_SLOW,
};

void Command_ApplyAffect(LPCHARACTER ch, const char* argument, const char* affectName, int32_t cmdAffect)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	sys_log(0, arg1);

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: %s <name>", affectName);
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);
	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s is not in same map", arg1);
		return;
	}

	switch (cmdAffect)
	{
		case COMMANDAFFECT_STUN:
			SkillAttackAffect(tch, 1000, IMMUNE_STUN, AFFECT_STUN, POINT_NONE, 0, AFF_STUN, 30, "GM_STUN");
			break;
		case COMMANDAFFECT_SLOW:
			SkillAttackAffect(tch, 1000, IMMUNE_SLOW, AFFECT_SLOW, POINT_MOV_SPEED, -30, AFF_SLOW, 30, "GM_SLOW");
			break;
	}

	sys_log(0, "%s %s", arg1, affectName);

	ch->ChatPacket(CHAT_TYPE_INFO, "%s %s", arg1, affectName);
}

ACMD(do_pcbang_update)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint32_t PCBangID = 0;

	if (*arg1 == '\0')
		PCBangID = 0;
	else
		str_to_number(PCBangID, arg1);

	if (PCBangID == 0)
	{
		CPCBangManager::instance().RequestUpdateIPList(0);
		ch->ChatPacket(CHAT_TYPE_INFO, "PCBang Info Update For All");
	}
	else
	{
		CPCBangManager::instance().RequestUpdateIPList(PCBangID);
		ch->ChatPacket(CHAT_TYPE_INFO, "PCBang Info Update For %u", PCBangID);
	}

	TPacketPCBangUpdate packet;
	packet.bHeader = HEADER_GG_PCBANG_UPDATE;
	packet.ulPCBangID = PCBangID;

	P2P_MANAGER::instance().Send(&packet, sizeof(TPacketPCBangUpdate));

}

ACMD(do_pcbang_check)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (CPCBangManager::instance().IsPCBangIP(arg1) == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s is a PCBang IP", arg1);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s is not a PCBang IP", arg1);
	}
}

ACMD(do_stun)
{
	Command_ApplyAffect(ch, argument, "stun", COMMANDAFFECT_STUN);
}

ACMD(do_slow)
{
	Command_ApplyAffect(ch, argument, "slow", COMMANDAFFECT_SLOW);
}

ACMD(do_transfer)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: transfer <name>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);
	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Target(%s) is in %d channel (my channel %d)", arg1, pkCCI->bChannel, g_bChannel);
				return;
			}

			TPacketGGTransfer pgg;

			pgg.bHeader = HEADER_GG_TRANSFER;
			strlcpy(pgg.szName, arg1, sizeof(pgg.szName));
			pgg.lX = ch->GetX();
			pgg.lY = ch->GetY();

			P2P_MANAGER::instance().Send(&pgg, sizeof(TPacketGGTransfer));
			ch->ChatPacket(CHAT_TYPE_INFO, "Transfer requested.");
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no character(%s) by that name", arg1);

		return;
	}

	if (ch == tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Transfer me?!?");
		return;
	}

	tch->WarpSet(ch->GetX(), ch->GetY(), ch->GetMapIndex());
}

struct GotoInfo
{
	std::string 	st_name;

	uint8_t 	empire;
	int32_t 	mapIndex;
	uint32_t 	x, y;

	GotoInfo()
	{
		st_name 	= "";
		empire 		= 0;
		mapIndex 	= 0;

		x = 0;
		y = 0;
	}
	GotoInfo(const GotoInfo& c_src)
	{
		__copy__(c_src);
	}
	void operator = (const GotoInfo& c_src)
	{
		__copy__(c_src);
	}
	void __copy__(const GotoInfo& c_src)
	{
		st_name 	= c_src.st_name;
		empire 		= c_src.empire;
		mapIndex 	= c_src.mapIndex;

		x = c_src.x;
		y = c_src.y;
	}
};

static std::vector<GotoInfo> gs_vec_gotoInfo;

void CHARACTER_AddGotoInfo(const std::string& c_st_name, uint8_t empire, int32_t mapIndex, uint32_t x, uint32_t y)
{
	GotoInfo newGotoInfo;
	newGotoInfo.st_name = c_st_name;
	newGotoInfo.empire = empire;
	newGotoInfo.mapIndex = mapIndex;
	newGotoInfo.x = x;
	newGotoInfo.y = y;
	gs_vec_gotoInfo.push_back(newGotoInfo);

	sys_log(0, "AddGotoInfo(name=%s, empire=%d, mapIndex=%d, pos=(%d, %d))", c_st_name.c_str(), empire, mapIndex, x, y);
}

bool FindInString(const char * c_pszFind, const char * c_pszIn)
{
	const char * c = c_pszIn;
	const char * p;

	p = strchr(c, '|');

	if (!p)
		return (0 == strncasecmp(c_pszFind, c_pszIn, strlen(c_pszFind)));
	else
	{
		char sz[64 + 1];

		do
		{
			strlcpy(sz, c, MIN(sizeof(sz), (p - c) + 1));

			if (!strncasecmp(c_pszFind, sz, strlen(c_pszFind)))
				return true;

			c = p + 1;
		} while ((p = strchr(c, '|')));

		strlcpy(sz, c, sizeof(sz));

		if (!strncasecmp(c_pszFind, sz, strlen(c_pszFind)))
			return true;
	}

	return false;
}

bool CHARACTER_GoToName(LPCHARACTER ch, uint8_t empire, int32_t mapIndex, const char* gotoName)
{
	std::vector<GotoInfo>::iterator i;
	for (i = gs_vec_gotoInfo.begin(); i != gs_vec_gotoInfo.end(); ++i)
	{
		const GotoInfo& c_eachGotoInfo = *i;

		if (mapIndex != 0)
		{
			if (mapIndex != c_eachGotoInfo.mapIndex)
				continue;
		}
		else if (!FindInString(gotoName, c_eachGotoInfo.st_name.c_str()))
			continue;

		if (c_eachGotoInfo.empire == 0 || c_eachGotoInfo.empire == empire)
		{
			int32_t x = c_eachGotoInfo.x * 100;
			int32_t y = c_eachGotoInfo.y * 100;

			ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
			ch->WarpSet(x, y);
			ch->Stop();
			return true;
		}
	}
	return false;
}




ACMD(do_goto)
{
	char arg1[256], arg2[256];
	int32_t x = 0, y = 0, z = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 && !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: goto <x meter> <y meter>");
		return;
	}

	if (isnhdigit(*arg1) && isnhdigit(*arg2))
	{
		str_to_number(x, arg1);
		str_to_number(y, arg2);

		PIXEL_POSITION p;

		if (SECTREE_MANAGER::instance().GetMapBasePosition(ch->GetX(), ch->GetY(), p))
		{
			x += p.x / 100;
			y += p.y / 100;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "You goto ( %d, %d )", x, y);
	}
	else
	{
		int32_t mapIndex = 0;
		uint8_t empire = 0;

		if (*arg1 == '#')
			str_to_number(mapIndex,  (arg1 + 1));

		if (*arg2 && isnhdigit(*arg2))
		{
			str_to_number(empire, arg2);
			empire = MINMAX(1, empire, 3);
		}
		else
			empire = ch->GetEmpire();

		if (CHARACTER_GoToName(ch, empire, mapIndex, arg1))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find map command syntax: /goto <mapname> [empire]");
			return;
		}

		return;
	}

	x *= 100;
	y *= 100;

	ch->Show(ch->GetMapIndex(), x, y, z);
	ch->Stop();
}

ACMD(do_warp)
{
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: warp <character name> | <x meter> <y meter>");
		return;
	}

	int32_t x = 0, y = 0;

	if (isnhdigit(*arg1) && isnhdigit(*arg2))
	{
		str_to_number(x, arg1);
		str_to_number(y, arg2);
	}
	else
	{
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

		if (nullptr == tch)
		{
			const CCI* pkCCI = P2P_MANAGER::instance().Find(arg1);

			if (nullptr != pkCCI)
			{
				if (pkCCI->bChannel != g_bChannel)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Target(%s) is in %d channel (my channel %d)", arg1, pkCCI->bChannel, g_bChannel);
					return;
				}

				ch->WarpToPID( pkCCI->dwPID );
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "There is no one(%s) by that name", arg1);
			}

			return;
		}
		else
		{
			x = tch->GetX() / 100;
			y = tch->GetY() / 100;
		}
	}

	x *= 100;
	y *= 100;

	ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", x, y);
	ch->WarpSet(x, y);
	ch->Stop();
}

#ifdef ENABLE_NEWSTUFF //@correction232
ACMD(do_rewarp)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "You warp to ( %d, %d )", ch->GetX(), ch->GetY());
	ch->WarpSet(ch->GetX(), ch->GetY());
	ch->Stop();
}
#endif

ACMD(do_item)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: item <item vnum>");
		return;
	}

	int32_t iCount = 1;

	if (*arg2)
	{
		str_to_number(iCount, arg2);
		iCount = MINMAX(1, iCount, g_wItemCountLimit); // @correction002 ITEM_MAX_COUNT -> g_wItemCountLimit
	}

	uint32_t dwVnum;

	if (isnhdigit(*arg1))
		str_to_number(dwVnum, arg1);
	else
	{
		if (!ITEM_MANAGER::instance().GetVnum(arg1, dwVnum))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that vnum(%s).", dwVnum, arg1);
			return;
		}
	}

	LPITEM item = ITEM_MANAGER::instance().CreateItem(dwVnum, iCount, 0, true);

	if (item)
	{
		if (item->IsDragonSoul())
		{
			int32_t iEmptyPos = ch->GetEmptyDragonSoulInventory(item);

			if (iEmptyPos != -1)
			{
				item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
				LogManager::instance().ItemLog(ch, item, "GM", item->GetName());
			}
			else
			{
				M2_DESTROY_ITEM(item);
				if (!ch->DragonSoul_IsQualified())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "ÀÎº¥ÀÌ È°¼ºÈ­ µÇÁö ¾ÊÀ½.");
				}
				else
					ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
		else
		{
			int32_t iEmptyPos = ch->GetEmptyInventory(item);

			if (iEmptyPos != -1)
			{
				item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
				LogManager::instance().ItemLog(ch, item, "GM", item->GetName());
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "#%u item not exist by that vnum(%s).", dwVnum, arg1);
	}
}

ACMD(do_group_random)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: grrandom <group vnum>");
		return;
	}

	uint32_t dwVnum = 0;
	str_to_number(dwVnum, arg1);
	CHARACTER_MANAGER::instance().SpawnGroupGroup(dwVnum, ch->GetMapIndex(), ch->GetX() - 500, ch->GetY() - 500, ch->GetX() + 500, ch->GetY() + 500);
}

ACMD(do_group)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: group <group vnum>");
		return;
	}

	uint32_t dwVnum = 0;
	str_to_number(dwVnum, arg1);

	if (test_server)
		sys_log(0, "COMMAND GROUP SPAWN %u at %u %u %u", dwVnum, ch->GetMapIndex(), ch->GetX(), ch->GetY());

	CHARACTER_MANAGER::instance().SpawnGroup(dwVnum, ch->GetMapIndex(), ch->GetX() - 500, ch->GetY() - 500, ch->GetX() + 500, ch->GetY() + 500);
}

ACMD(do_mob_coward)
{
	char	arg1[256], arg2[256];
	uint32_t	vnum = 0;
	LPCHARACTER	tch;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mc <vnum>");
		return;
	}

	const CMob * pkMob;

	if (isdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::instance().Get(vnum)) == nullptr)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int32_t iCount = 0;

	if (*arg2)
		str_to_number(iCount, arg2);
	else
		iCount = 1;

	iCount = MIN(20, iCount);

	while (iCount--)
	{
		tch = CHARACTER_MANAGER::instance().SpawnMobRange(vnum,
				ch->GetMapIndex(),
				ch->GetX() - number(200, 750),
				ch->GetY() - number(200, 750),
				ch->GetX() + number(200, 750),
				ch->GetY() + number(200, 750),
				true,
				pkMob->m_table.bType == CHAR_TYPE_STONE);
		if (tch)
			tch->SetCoward();
	}
}

ACMD(do_mob_map)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: mm <vnum>");
		return;
	}

	uint32_t vnum = 0;
	str_to_number(vnum, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().SpawnMobRandomPosition(vnum, ch->GetMapIndex());

	if (tch)
		ch->ChatPacket(CHAT_TYPE_INFO, "%s spawned in %dx%d", tch->GetName(), tch->GetX(), tch->GetY());
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Spawn failed.");
}

ACMD(do_mob_aggresive)
{
	char	arg1[256], arg2[256];
	uint32_t	vnum = 0;
	LPCHARACTER	tch;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
		return;
	}

	const CMob * pkMob;

	if (isdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::instance().Get(vnum)) == nullptr)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int32_t iCount = 0;

	if (*arg2)
		str_to_number(iCount, arg2);
	else
		iCount = 1;

	iCount = MIN(20, iCount);

	while (iCount--)
	{
		tch = CHARACTER_MANAGER::instance().SpawnMobRange(vnum,
				ch->GetMapIndex(),
				ch->GetX() - number(200, 750),
				ch->GetY() - number(200, 750),
				ch->GetX() + number(200, 750),
				ch->GetY() + number(200, 750),
				true,
				pkMob->m_table.bType == CHAR_TYPE_STONE);
		if (tch)
			tch->SetAggressive();
	}
}

ACMD(do_mob)
{
	char	arg1[256], arg2[256];
	uint32_t	vnum = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
		return;
	}

	const CMob* pkMob = nullptr;

	if (isnhdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::instance().Get(vnum)) == nullptr)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int32_t iCount = 0;

	if (*arg2)
		str_to_number(iCount, arg2);
	else
		iCount = 1;

	if (test_server)
		iCount = MIN(40, iCount);
	else
		iCount = MIN(20, iCount);

	while (iCount--)
	{
		CHARACTER_MANAGER::instance().SpawnMobRange(vnum,
				ch->GetMapIndex(),
				ch->GetX() - number(200, 750),
				ch->GetY() - number(200, 750),
				ch->GetX() + number(200, 750),
				ch->GetY() + number(200, 750),
				true,
				pkMob->m_table.bType == CHAR_TYPE_STONE);
	}
}

ACMD(do_mob_ld)
{
	char	arg1[256], arg2[256], arg3[256], arg4[256];
	uint32_t	vnum = 0;

	two_arguments(two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3), arg4, sizeof(arg4));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mob <mob vnum>");
		return;
	}

	const CMob* pkMob = nullptr;

	if (isnhdigit(*arg1))
	{
		str_to_number(vnum, arg1);

		if ((pkMob = CMobManager::instance().Get(vnum)) == nullptr)
			vnum = 0;
	}
	else
	{
		pkMob = CMobManager::Instance().Get(arg1, true);

		if (pkMob)
			vnum = pkMob->m_table.dwVnum;
	}

	if (vnum == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such mob(%s) by that vnum", arg1);
		return;
	}

	int32_t dir = 1;
	int32_t x=0,y=0;

	if (*arg2)
		str_to_number(x, arg2);
	if (*arg3)
		str_to_number(y, arg3);
	if (*arg4)
		str_to_number(dir, arg4);


	CHARACTER_MANAGER::instance().SpawnMob(vnum,
		ch->GetMapIndex(),
		x*100,
		y*100,
		ch->GetZ(),
		pkMob->m_table.bType == CHAR_TYPE_STONE,
		dir);
}

struct FuncPurge
{
	LPCHARACTER m_pkGM;
	bool	m_bAll;

	FuncPurge(LPCHARACTER ch) : m_pkGM(ch), m_bAll(false)
	{
	}

	void operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
			return;

		LPCHARACTER pkChr = (LPCHARACTER) ent;

		int32_t iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkGM->GetX(), pkChr->GetY() - m_pkGM->GetY());

		if (!m_bAll && iDist >= 1000)
			return;

		sys_log(0, "PURGE: %s %d", pkChr->GetName(), iDist);

		if (pkChr->IsNPC() && !pkChr->IsPet() && pkChr->GetRider() == nullptr
#ifdef __MOUNT__
&& !pkChr->IsMountSystem()
#endif
#ifdef __GROWTH_PET__
&& !pkChr->IsNewPet()
#endif
)
		{
			M2_DESTROY_CHARACTER(pkChr);
		}
	}
};

ACMD(do_purge)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	FuncPurge func(ch);

	if (*arg1 && !strcmp(arg1, "all"))
		func.m_bAll = true;

	LPSECTREE sectree = ch->GetSectree();
	if (sectree)
		sectree->ForEachAround(func);
	else
		sys_err("PURGE_ERROR.NULL_SECTREE(mapIndex=%d, pos=(%d, %d)", ch->GetMapIndex(), ch->GetX(), ch->GetY());
}

ACMD(do_item_purge)
{
#ifdef __SUPPORT__
	if (ch->GetSupportSystem()->IsActiveSupport())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu komutu kullanabilmek i?n yard?c?þaman?g?dermen gerekiyor.");
		return;
	}
#endif
#ifdef ENABLE_CMD_IPURGE_EX // @correction160
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: ipurge <window>");
		ch->ChatPacket(CHAT_TYPE_INFO, "List of the available windows:");
		ch->ChatPacket(CHAT_TYPE_INFO, " all");
		ch->ChatPacket(CHAT_TYPE_INFO, " inventory or inv");
		ch->ChatPacket(CHAT_TYPE_INFO, " equipment or equip");
		ch->ChatPacket(CHAT_TYPE_INFO, " dragonsoul or ds");
		ch->ChatPacket(CHAT_TYPE_INFO, " belt");
		return;
	}

	int32_t         i;
	LPITEM      item;

	std::string strArg(arg1);
	if (!strArg.compare(0, 3, "all"))
	{
		for (i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; ++i)
		{
			if ((item = ch->GetInventoryItem(i)))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
			}
		}
		for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i ))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
		}
	}
	else if (!strArg.compare(0, 3, "inv"))
	{
		for (i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetInventoryItem(i)))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
			}
		}
	}
	else if (!strArg.compare(0, 5, "equip"))
	{
		for (i = 0; i < WEAR_MAX_NUM; ++i)
		{
			if ((item = ch->GetInventoryItem(INVENTORY_MAX_NUM + i)))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, INVENTORY_MAX_NUM + i, 255);
			}
		}
	}
	else if (!strArg.compare(0, 6, "dragon") || !strArg.compare(0, 2, "ds"))
	{
		for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
		{
			if ((item = ch->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i ))))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			}
		}
	}
	else if (!strArg.compare(0, 4, "belt"))
	{
		for (i = 0; i < BELT_INVENTORY_SLOT_COUNT; ++i)
		{
			if ((item = ch->GetInventoryItem(BELT_INVENTORY_SLOT_START + i)))
			{
				ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
				ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, BELT_INVENTORY_SLOT_START + i, 255);
			}
		}
	}
#else
	int32_t         i;
	LPITEM      item;

	for (i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; ++i)
	{
		if ((item = ch->GetInventoryItem(i)))
		{
			ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
			ch->SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
		}
	}
	for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = ch->GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i ))))
		{
			ITEM_MANAGER::instance().RemoveItem(item, "PURGE");
		}
	}
#endif
}

ACMD(do_state)
{
	char arg1[256];
	LPCHARACTER tch;

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		if (arg1[0] == '#')
		{
			tch = CHARACTER_MANAGER::instance().Find(strtoul(arg1+1, nullptr, 10));
		}
		else
		{
			LPDESC d = DESC_MANAGER::instance().FindByCharacterName(arg1);

			if (!d)
				tch = nullptr;
			else
				tch = d->GetCharacter();
		}
	}
	else
		tch = ch;

	if (!tch)
		return;

	char buf[256];

	snprintf(buf, sizeof(buf), "%s's State: ", tch->GetName());

	if (tch->IsPosition(POS_FIGHTING))
		strlcat(buf, "Battle", sizeof(buf));
	else if (tch->IsPosition(POS_DEAD))
		strlcat(buf, "Dead", sizeof(buf));
	else
		strlcat(buf, "Standing", sizeof(buf));

	if (ch->GetShop())
		strlcat(buf, ", Shop", sizeof(buf));

	if (ch->GetExchange())
		strlcat(buf, ", Exchange", sizeof(buf));

	ch->ChatPacket(CHAT_TYPE_INFO, "%s", buf);

	int32_t len;
	len = snprintf(buf, sizeof(buf), "Coordinate %dx%d (%dx%d)",
			tch->GetX(), tch->GetY(), tch->GetX() / 100, tch->GetY() / 100);

	if (len < 0 || len >= (int32_t) sizeof(buf))
		len = sizeof(buf) - 1;

	LPSECTREE pSec = SECTREE_MANAGER::instance().Get(tch->GetMapIndex(), tch->GetX(), tch->GetY());

	if (pSec)
	{
		TMapSetting& map_setting = SECTREE_MANAGER::instance().GetMap(tch->GetMapIndex())->m_setting;
		snprintf(buf + len, sizeof(buf) - len, " MapIndex %d Attribute %08X Local Position (%d x %d)",
			tch->GetMapIndex(), pSec->GetAttribute(tch->GetX(), tch->GetY()), (tch->GetX() - map_setting.iBaseX)/100, (tch->GetY() - map_setting.iBaseY)/100);
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "%s", buf);

	ch->ChatPacket(CHAT_TYPE_INFO, "LEV %d", tch->GetLevel());
	ch->ChatPacket(CHAT_TYPE_INFO, "HP %d/%d", tch->GetHP(), tch->GetMaxHP());
	ch->ChatPacket(CHAT_TYPE_INFO, "SP %d/%d", tch->GetSP(), tch->GetMaxSP());
	ch->ChatPacket(CHAT_TYPE_INFO, "ATT %d MAGIC_ATT %d SPD %d CRIT %d%% PENE %d%% ATT_BONUS %d%%",
			tch->GetPoint(POINT_ATT_GRADE),
			tch->GetPoint(POINT_MAGIC_ATT_GRADE),
			tch->GetPoint(POINT_ATT_SPEED),
			tch->GetPoint(POINT_CRITICAL_PCT),
			tch->GetPoint(POINT_PENETRATE_PCT),
			tch->GetPoint(POINT_ATT_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "DEF %d MAGIC_DEF %d BLOCK %d%% DODGE %d%% DEF_BONUS %d%%",
			tch->GetPoint(POINT_DEF_GRADE),
			tch->GetPoint(POINT_MAGIC_DEF_GRADE),
			tch->GetPoint(POINT_BLOCK),
			tch->GetPoint(POINT_DODGE),
			tch->GetPoint(POINT_DEF_BONUS));
	ch->ChatPacket(CHAT_TYPE_INFO, "RESISTANCES:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   WARR:%3d%% ASAS:%3d%% SURA:%3d%% SHAM:%3d%%"
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
			" WOLF:%3d%%"
#endif
			,
			tch->GetPoint(POINT_RESIST_WARRIOR),
			tch->GetPoint(POINT_RESIST_ASSASSIN),
			tch->GetPoint(POINT_RESIST_SURA),
			tch->GetPoint(POINT_RESIST_SHAMAN)
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
			,tch->GetPoint(POINT_RESIST_WOLFMAN)
#endif
	);
	ch->ChatPacket(CHAT_TYPE_INFO, "   SWORD:%3d%% THSWORD:%3d%% DAGGER:%3d%% BELL:%3d%% FAN:%3d%% BOW:%3d%%"
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
			" CLAW:%3d%%"
#endif
			,
			tch->GetPoint(POINT_RESIST_SWORD),
			tch->GetPoint(POINT_RESIST_TWOHAND),
			tch->GetPoint(POINT_RESIST_DAGGER),
			tch->GetPoint(POINT_RESIST_BELL),
			tch->GetPoint(POINT_RESIST_FAN),
			tch->GetPoint(POINT_RESIST_BOW)
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
			,tch->GetPoint(POINT_RESIST_CLAW)
#endif
	);
	ch->ChatPacket(CHAT_TYPE_INFO, "   FIRE:%3d%% ELEC:%3d%% MAGIC:%3d%% WIND:%3d%% CRIT:%3d%% PENE:%3d%%",
			tch->GetPoint(POINT_RESIST_FIRE),
			tch->GetPoint(POINT_RESIST_ELEC),
			tch->GetPoint(POINT_RESIST_MAGIC),
			tch->GetPoint(POINT_RESIST_WIND),
			tch->GetPoint(POINT_RESIST_CRITICAL),
			tch->GetPoint(POINT_RESIST_PENETRATE));
	ch->ChatPacket(CHAT_TYPE_INFO, "   ICE:%3d%% EARTH:%3d%% DARK:%3d%%",
			tch->GetPoint(POINT_RESIST_ICE),
			tch->GetPoint(POINT_RESIST_EARTH),
			tch->GetPoint(POINT_RESIST_DARK));

	ch->ChatPacket(CHAT_TYPE_INFO, "MALL:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   ATT:%3d%% DEF:%3d%% EXP:%3d%% ITEMx%d GOLDx%d",
			tch->GetPoint(POINT_MALL_ATTBONUS),
			tch->GetPoint(POINT_MALL_DEFBONUS),
			tch->GetPoint(POINT_MALL_EXPBONUS),
			tch->GetPoint(POINT_MALL_ITEMBONUS) / 10,
			tch->GetPoint(POINT_MALL_GOLDBONUS) / 10);

	ch->ChatPacket(CHAT_TYPE_INFO, "BONUS:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   SKILL:%3d%% NORMAL:%3d%% SKILL_DEF:%3d%% NORMAL_DEF:%3d%%",
			tch->GetPoint(POINT_SKILL_DAMAGE_BONUS),
			tch->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS),
			tch->GetPoint(POINT_SKILL_DEFEND_BONUS),
			tch->GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS));

	ch->ChatPacket(CHAT_TYPE_INFO, "   HUMAN:%3d%% ANIMAL:%3d%% ORC:%3d%% MILGYO:%3d%% UNDEAD:%3d%%",
			tch->GetPoint(POINT_ATTBONUS_HUMAN),
			tch->GetPoint(POINT_ATTBONUS_ANIMAL),
			tch->GetPoint(POINT_ATTBONUS_ORC),
			tch->GetPoint(POINT_ATTBONUS_MILGYO),
			tch->GetPoint(POINT_ATTBONUS_UNDEAD));

	ch->ChatPacket(CHAT_TYPE_INFO, "   DEVIL:%3d%% INSECT:%3d%% FIRE:%3d%% ICE:%3d%% DESERT:%3d%%",
			tch->GetPoint(POINT_ATTBONUS_DEVIL),
			tch->GetPoint(POINT_ATTBONUS_INSECT),
			tch->GetPoint(POINT_ATTBONUS_FIRE),
			tch->GetPoint(POINT_ATTBONUS_ICE),
			tch->GetPoint(POINT_ATTBONUS_DESERT));

	ch->ChatPacket(CHAT_TYPE_INFO, "   TREE:%3d%% MONSTER:%3d%%",
			tch->GetPoint(POINT_ATTBONUS_TREE),
			tch->GetPoint(POINT_ATTBONUS_MONSTER));

	ch->ChatPacket(CHAT_TYPE_INFO, "   WARR:%3d%% ASAS:%3d%% SURA:%3d%% SHAM:%3d%%"
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
			" WOLF:%3d%%"
#endif
			,
			tch->GetPoint(POINT_ATTBONUS_WARRIOR),
			tch->GetPoint(POINT_ATTBONUS_ASSASSIN),
			tch->GetPoint(POINT_ATTBONUS_SURA),
			tch->GetPoint(POINT_ATTBONUS_SHAMAN)
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
			,tch->GetPoint(POINT_ATTBONUS_WOLFMAN)
#endif
	);
	ch->ChatPacket(CHAT_TYPE_INFO, "IMMUNE:");
	ch->ChatPacket(CHAT_TYPE_INFO, "   STUN:%d SLOW:%d FALL:%d",
		tch->GetPoint(POINT_IMMUNE_STUN),
		tch->GetPoint(POINT_IMMUNE_SLOW),
		tch->GetPoint(POINT_IMMUNE_FALL));

	for (int32_t i = 0; i < MAX_PRIV_NUM; ++i)
		if (CPrivManager::instance().GetPriv(tch, i))
		{
			int32_t iByEmpire = CPrivManager::instance().GetPrivByEmpire(tch->GetEmpire(), i);
			int32_t iByGuild = 0;

			if (tch->GetGuild())
				iByGuild = CPrivManager::instance().GetPrivByGuild(tch->GetGuild()->GetID(), i);

			int32_t iByPlayer = CPrivManager::instance().GetPrivByCharacter(tch->GetPlayerID(), i);

			if (iByEmpire)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for empire : %d", LC_TEXT(c_apszPrivNames[i]), iByEmpire);

			if (iByGuild)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for guild : %d", LC_TEXT(c_apszPrivNames[i]), iByGuild);

			if (iByPlayer)
				ch->ChatPacket(CHAT_TYPE_INFO, "%s for player : %d", LC_TEXT(c_apszPrivNames[i]), iByPlayer);
		}
}

struct notice_packet_func
{
	const char * m_str;
#ifdef ENABLE_FULL_NOTICE // @correction188
	bool m_bBigFont;
	notice_packet_func(const char * str, bool bBigFont=false) : m_str(str), m_bBigFont(bBigFont)
#else
	notice_packet_func(const char * str) : m_str(str)
#endif
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;
#ifdef ENABLE_FULL_NOTICE // @correction188
		d->GetCharacter()->ChatPacket((m_bBigFont)?CHAT_TYPE_BIG_NOTICE:CHAT_TYPE_NOTICE, "%s", m_str);
#else
		d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE, "%s", m_str);
#endif
	}
};

struct monarch_notice_packet_func
{
	const char * m_str;
	uint8_t m_bEmpire;

	monarch_notice_packet_func(uint8_t bEmpire, const char * str) : m_str(str), m_bEmpire(bEmpire)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		if (m_bEmpire == d->GetCharacter()->GetEmpire())
		{
			d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE, "%s", m_str);
		}
	}
};


#ifdef ENABLE_FULL_NOTICE // @correction188
void SendNotice(const char * c_pszBuf, bool bBigFont)
#else
void SendNotice(const char * c_pszBuf)
#endif
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::instance().GetClientSet();
#ifdef ENABLE_FULL_NOTICE // @correction188
	std::for_each(c_ref_set.begin(), c_ref_set.end(), notice_packet_func(c_pszBuf, bBigFont));
#else
	std::for_each(c_ref_set.begin(), c_ref_set.end(), notice_packet_func(c_pszBuf));
#endif
}

void SendMonarchNotice(uint8_t bEmpire, const char* c_pszBuf)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), monarch_notice_packet_func(bEmpire, c_pszBuf));
}

struct notice_map_packet_func
{
	const char* m_str;
	int32_t m_mapIndex;
	bool m_bBigFont;

	notice_map_packet_func(const char* str, int32_t idx, bool bBigFont) : m_str(str), m_mapIndex(idx), m_bBigFont(bBigFont)
	{
	}

	void operator() (LPDESC d)
	{
		if (d->GetCharacter() == nullptr) return;
		if (d->GetCharacter()->GetMapIndex() != m_mapIndex) return;

		d->GetCharacter()->ChatPacket(m_bBigFont == true ? CHAT_TYPE_BIG_NOTICE : CHAT_TYPE_NOTICE, "%s", m_str);
	}
};

void SendNoticeMap(const char* c_pszBuf, int32_t nMapIndex, bool bBigFont)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), notice_map_packet_func(c_pszBuf, nMapIndex, bBigFont));
}

struct log_packet_func
{
	const char * m_str;

	log_packet_func(const char * str) : m_str(str)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter())
			return;

		if (d->GetCharacter()->GetGMLevel() > GM_PLAYER)
			d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE, "%s", m_str);
	}
};


void SendLog(const char * c_pszBuf)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), log_packet_func(c_pszBuf));
}

#ifdef ENABLE_FULL_NOTICE // @correction188
void BroadcastNotice(const char * c_pszBuf, bool bBigFont)
#else
void BroadcastNotice(const char * c_pszBuf)
#endif
{
	TPacketGGNotice p;
#ifdef ENABLE_FULL_NOTICE // @correction188
	p.bHeader = (bBigFont)?HEADER_GG_BIG_NOTICE:HEADER_GG_NOTICE;
#else
	p.bHeader = HEADER_GG_NOTICE;
#endif
	p.lSize = strlen(c_pszBuf) + 1;

	TEMP_BUFFER buf;
	buf.write(&p, sizeof(p));
	buf.write(c_pszBuf, p.lSize);

	P2P_MANAGER::instance().Send(buf.read_peek(), buf.size());

#ifdef ENABLE_FULL_NOTICE // @correction188
	SendNotice(c_pszBuf, bBigFont);
#else
	SendNotice(c_pszBuf);
#endif
}

void BroadcastMonarchNotice(uint8_t bEmpire, const char * c_pszBuf)
{
	TPacketGGMonarchNotice p;
	p.bHeader = HEADER_GG_MONARCH_NOTICE;
	p.bEmpire = bEmpire;
	p.lSize = strlen(c_pszBuf) + 1;

	TEMP_BUFFER buf;
	buf.write(&p, sizeof(p));
	buf.write(c_pszBuf, p.lSize);

	P2P_MANAGER::instance().Send(buf.read_peek(), buf.size());

	SendMonarchNotice(bEmpire, c_pszBuf);
}

ACMD(do_notice)
{
	BroadcastNotice(argument);
}

ACMD(do_map_notice)
{
	SendNoticeMap(argument, ch->GetMapIndex(), false);
}

ACMD(do_big_notice)
{
#ifdef ENABLE_FULL_NOTICE // @correction188
	BroadcastNotice(argument, true);
#else
	ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "%s", argument);
#endif
}

#ifdef ENABLE_FULL_NOTICE // @correction188
ACMD(do_map_big_notice)
{
	SendNoticeMap(argument, ch->GetMapIndex(), true);
}

ACMD(do_notice_test)
{
	ch->ChatPacket(CHAT_TYPE_NOTICE, "%s", argument);
}

ACMD(do_big_notice_test)
{
	ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "%s", argument);
}
#endif

ACMD(do_monarch_notice)
{
	if (ch->IsMonarch() == true)
	{
		BroadcastMonarchNotice(ch->GetEmpire(), argument);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ë °¡´ÉÇÑ ±â´ÉÀÔ´Ï´Ù"));
	}
}

ACMD(do_who)
{
	int32_t iTotal;
	int32_t * paiEmpireUserCount;
	int32_t iLocal;

	DESC_MANAGER::instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

	ch->ChatPacket(CHAT_TYPE_INFO, "Total [%d] %d / %d / %d (this server %d)",
			iTotal, paiEmpireUserCount[1], paiEmpireUserCount[2], paiEmpireUserCount[3], iLocal);
}

class user_func
{
	public:
		LPCHARACTER	m_ch;
		static int32_t count;
		static char str[128];
		static int32_t str_len;

		user_func()
			: m_ch(nullptr)
		{}

		void initialize(LPCHARACTER ch)
		{
			m_ch = ch;
			str_len = 0;
			count = 0;
			str[0] = '\0';
		}

		void operator () (LPDESC d)
		{
			if (!d->GetCharacter())
				return;

			int32_t len = snprintf(str + str_len, sizeof(str) - str_len, "%-16s ", d->GetCharacter()->GetName());

			if (len < 0 || len >= (int32_t) sizeof(str) - str_len)
				len = (sizeof(str) - str_len) - 1;

			str_len += len;
			++count;

			if (!(count % 4))
			{
				m_ch->ChatPacket(CHAT_TYPE_INFO, str);

				str[0] = '\0';
				str_len = 0;
			}
		}
};

int32_t	user_func::count = 0;
char user_func::str[128] = { 0, };
int32_t	user_func::str_len = 0;

ACMD(do_user)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::instance().GetClientSet();
	user_func func;

	func.initialize(ch);
	std::for_each(c_ref_set.begin(), c_ref_set.end(), func);

	if (func.count % 4)
		ch->ChatPacket(CHAT_TYPE_INFO, func.str);

	ch->ChatPacket(CHAT_TYPE_INFO, "Total %d", func.count);
}

ACMD(do_disconnect)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /dc <player name>");
		return;
	}

	LPDESC d = DESC_MANAGER::instance().FindByCharacterName(arg1);
	LPCHARACTER	tch = d ? d->GetCharacter() : nullptr;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player.", arg1);
		return;
	}

	if (tch == ch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "cannot disconnect myself");
		return;
	}

	DESC_MANAGER::instance().DestroyDesc(d);
}

ACMD(do_kill)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /kill <player name>");
		return;
	}

	LPDESC	d = DESC_MANAGER::instance().FindByCharacterName(arg1);
	LPCHARACTER tch = d ? d->GetCharacter() : nullptr;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
		return;
	}

	tch->Dead();
}

#ifdef ENABLE_NEWSTUFF //@correction232
ACMD(do_poison)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /poison <player name>");
		return;
	}

	LPDESC	d = DESC_MANAGER::instance().FindByCharacterName(arg1);
	LPCHARACTER tch = d ? d->GetCharacter() : nullptr;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
		return;
	}

	tch->AttackedByPoison(nullptr);
}
#endif
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
ACMD(do_bleeding)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "ex) /bleeding <player name>");
		return;
	}

	LPDESC	d = DESC_MANAGER::instance().FindByCharacterName(arg1);
	LPCHARACTER tch = d ? d->GetCharacter() : nullptr;

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s: no such a player", arg1);
		return;
	}

	tch->AttackedByBleeding(nullptr);
}
#endif

#define MISC    0
#define BINARY  1
#define NUMBER  2

namespace DoSetTypes{
typedef enum do_set_types_s {GOLD, RACE, SEX, JOB, EXP, MAX_HP, MAX_SP, SKILL, ALIGNMENT, ALIGN} do_set_types_t;
}

const struct set_struct
{
	const char *cmd;
	const char type;
	const char * help;
} set_fields[] = {
	{ "gold",		NUMBER,	nullptr	},
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	{ "race",		NUMBER,	"0. Warrior, 1. Ninja, 2. Sura, 3. Shaman, 4. Lycan"		},
#else
	{ "race",		NUMBER,	"0. Warrior, 1. Ninja, 2. Sura, 3. Shaman"		},
#endif
	{ "sex",		NUMBER,	"0. Male, 1. Female"	},
	{ "job",		NUMBER,	"0. None, 1. First, 2. Second"	},
	{ "exp",		NUMBER,	nullptr	},
	{ "max_hp",		NUMBER,	nullptr	},
	{ "max_sp",		NUMBER,	nullptr	},
	{ "skill",		NUMBER,	nullptr	},
	{ "alignment",	NUMBER,	nullptr	},
	{ "align",		NUMBER,	nullptr	},
	{ "\n",			MISC,	nullptr	}
};

ACMD(do_set)
{
	char arg1[256], arg2[256], arg3[256];

	LPCHARACTER tch = nullptr;

	int32_t i, len;
	const char* line;

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (!*arg1 || !*arg2 || !*arg3)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: set <name> <field> <value>");
#ifdef ENABLE_NEWSTUFF //@correction232
		ch->ChatPacket(CHAT_TYPE_INFO, "List of the fields available:");
		for (i = 0; *(set_fields[i].cmd) != '\n'; i++)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, " %d. %s", i+1, set_fields[i].cmd);
			if (set_fields[i].help != nullptr)
				ch->ChatPacket(CHAT_TYPE_INFO, "  Help: %s", set_fields[i].help);
		}
#endif
		return;
	}

	tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s not exist", arg1);
		return;
	}

	len = strlen(arg2);

	for (i = 0; *(set_fields[i].cmd) != '\n'; i++)
		if (!strncmp(arg2, set_fields[i].cmd, len))
			break;

	switch (i)
	{
		case DoSetTypes::GOLD:
			{
				uint64_t gold = 0;
				str_to_number(gold, arg3);
				DBManager::instance().SendMoneyLog(MONEY_LOG_MISC, 3, gold);
				tch->ChangeGold(true, gold, true);
			}
			break;

		case DoSetTypes::RACE:
#ifdef ENABLE_NEWSTUFF //@correction232
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				amount = MINMAX(0, amount, JOB_MAX_NUM);
				ESex mySex = GET_SEX(tch);
				uint32_t dwRace = MAIN_RACE_WARRIOR_M;
				switch (amount)
				{
					case JOB_WARRIOR:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_WARRIOR_M:MAIN_RACE_WARRIOR_W;
						break;
					case JOB_ASSASSIN:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_ASSASSIN_M:MAIN_RACE_ASSASSIN_W;
						break;
					case JOB_SURA:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_SURA_M:MAIN_RACE_SURA_W;
						break;
					case JOB_SHAMAN:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_SHAMAN_M:MAIN_RACE_SHAMAN_W;
						break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
					case JOB_WOLFMAN:
						dwRace = (mySex==SEX_MALE)?MAIN_RACE_WOLFMAN_M:MAIN_RACE_WOLFMAN_M;
						break;
#endif
				}
				if (dwRace!=tch->GetRaceNum())
				{
					tch->SetRace(dwRace);
					tch->ClearSkill();
					tch->SetSkillGroup(0);
					tch->SetPolymorph(101);
					tch->SetPolymorph(0);
				}
			}
#endif
			break;

		case DoSetTypes::SEX:
#ifdef ENABLE_NEWSTUFF //@correction232
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				amount = MINMAX(SEX_MALE, amount, SEX_FEMALE);
				if (amount != GET_SEX(tch))
				{
					tch->ChangeSex();
					tch->SetPolymorph(101);
					tch->SetPolymorph(0);
				}
			}
#endif
			break;

		case DoSetTypes::JOB:
#ifdef ENABLE_NEWSTUFF //@correction232
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				amount = MINMAX(0, amount, 2);
				if (amount != tch->GetSkillGroup())
				{
					tch->ClearSkill();
					tch->SetSkillGroup(amount);
				}
			}
#endif
			break;

		case DoSetTypes::EXP:
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_EXP, amount, true);
			}
			break;

		case DoSetTypes::MAX_HP:
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_MAX_HP, amount, true);
			}
			break;

		case DoSetTypes::MAX_SP:
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_MAX_SP, amount, true);
			}
			break;

		case DoSetTypes::SKILL:
			{
				int32_t amount = 0;
				str_to_number(amount, arg3);
				tch->PointChange(POINT_SKILL, amount, true);
			}
			break;

		case DoSetTypes::ALIGN:
		case DoSetTypes::ALIGNMENT:
			{
				int32_t	amount = 0;
				str_to_number(amount, arg3);
				tch->UpdateAlignment(amount - ch->GetRealAlignment());
			}
			break;
	}

	if (set_fields[i].type == NUMBER)
	{
		uint64_t	amount = 0;
		str_to_number(amount, arg3);
		ch->ChatPacket(CHAT_TYPE_INFO, "%s's %s set to [%llu]", tch->GetName(), set_fields[i].cmd, amount);
	}
}

ACMD(do_reset)
{
	ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
	ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
	ch->Save();
}

ACMD(do_advance)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: advance <name> <level>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "%s not exist", arg1);
		return;
	}

	int32_t level = 0;
	str_to_number(level, arg2);

	tch->ResetPoint(MINMAX(0, level, gPlayerMaxLevel)); // @correction001 PLAYER_MAX_LEVEL_CONST -> gPlayerMaxLevel
}

ACMD(do_respawn)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1 && !strcasecmp(arg1, "all"))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Respaw everywhere");
		regen_reset(0, 0);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Respaw around");
		regen_reset(ch->GetX(), ch->GetY());
	}
}

ACMD(do_safebox_size)
{

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int32_t size = 0;

	if (*arg1)
		str_to_number(size, arg1);

	if (size > 3 || size < 0)
		size = 0;

	ch->ChatPacket(CHAT_TYPE_INFO, "Safebox size set to %d", size);
	ch->ChangeSafeboxSize(size);
}

ACMD(do_makeguild)
{
	if (ch->GetGuild())
		return;

	CGuildManager& gm = CGuildManager::instance();

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	TGuildCreateParameter cp;
	memset(&cp, 0, sizeof(cp));

	cp.master = ch;
	strlcpy(cp.name, arg1, sizeof(cp.name));

	if (!check_name(cp.name))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀûÇÕÇÏÁö ¾ÊÀº ±æµå ÀÌ¸§ ÀÔ´Ï´Ù."));
		return;
	}

	gm.CreateGuild(cp);
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("(%s) ±æµå°¡ »ý¼ºµÇ¾ú½À´Ï´Ù. [ÀÓ½Ã]"), cp.name);
}

ACMD(do_deleteguild)
{
	if (ch->GetGuild())
		ch->GetGuild()->RequestDisband(ch->GetPlayerID());
}

ACMD(do_greset)
{
	if (ch->GetGuild())
		ch->GetGuild()->Reset();
}

ACMD(do_refine_rod)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint8_t cell = 0;
	str_to_number(cell, arg1);
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
		fishing::RealRefineRod(ch, item);
}

ACMD(do_refine_pick)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint8_t cell = 0;
	str_to_number(cell, arg1);
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
	{
		mining::CHEAT_MAX_PICK(ch, item);
		mining::RealRefinePick(ch, item);
	}
}

ACMD(do_max_pick)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint8_t cell = 0;
	str_to_number(cell, arg1);
	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
	{
		mining::CHEAT_MAX_PICK(ch, item);
	}
}

ACMD(do_fishing_simul)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	argument = one_argument(argument, arg1, sizeof(arg1));
	two_arguments(argument, arg2, sizeof(arg2), arg3, sizeof(arg3));

	int32_t count = 1000;
	int32_t prob_idx = 0;
	int32_t level = 100;

	ch->ChatPacket(CHAT_TYPE_INFO, "Usage: fishing_simul <level> <prob index> <count>");

	if (*arg1)
		str_to_number(level, arg1);

	if (*arg2)
		str_to_number(prob_idx, arg2);

	if (*arg3)
		str_to_number(count, arg3);

	fishing::Simulation(level, count, prob_idx, ch);
}

ACMD(do_invisibility)
{
	if (ch->IsAffectFlag(AFF_INVISIBILITY))
	{
		ch->RemoveAffect(AFFECT_INVISIBILITY);
	}
	else
	{
		ch->AddAffect(AFFECT_INVISIBILITY, POINT_NONE, 0, AFF_INVISIBILITY, INFINITE_AFFECT_DURATION, 0, true);
	}
}

ACMD(do_event_flag)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!(*arg1) || !(*arg2))
		return;

	int32_t value = 0;
	str_to_number(value, arg2);

	if (!strcmp(arg1, "mob_item") ||
			!strcmp(arg1, "mob_exp") ||
			!strcmp(arg1, "mob_gold") ||
			!strcmp(arg1, "mob_dam") ||
			!strcmp(arg1, "mob_gold_pct") ||
			!strcmp(arg1, "mob_item_buyer") ||
			!strcmp(arg1, "mob_exp_buyer") ||
			!strcmp(arg1, "mob_gold_buyer") ||
			!strcmp(arg1, "mob_gold_pct_buyer")
	   )
		value = MINMAX(0, value, 1000);

	quest::CQuestManager::instance().RequestSetEventFlag(arg1, value);
	ch->ChatPacket(CHAT_TYPE_INFO, "RequestSetEventFlag %s %d", arg1, value);
	sys_log(0, "RequestSetEventFlag %s %d", arg1, value);
}

ACMD(do_get_event_flag)
{
	quest::CQuestManager::instance().SendEventFlagList(ch);
}

ACMD(do_private)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: private <map index>");
		return;
	}

	int32_t lMapIndex;
	int32_t map_index = 0;
	str_to_number(map_index, arg1);
	if ((lMapIndex = SECTREE_MANAGER::instance().CreatePrivateMap(map_index)))
	{
		ch->SaveExitLocation();

		LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(lMapIndex);
		ch->WarpSet(pkSectreeMap->m_setting.posSpawn.x, pkSectreeMap->m_setting.posSpawn.y, lMapIndex);
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Can't find map by index %d", map_index);
}

ACMD(do_qf)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	quest::PC* pPC = quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
	std::string questname = pPC->GetCurrentQuestName();

	if (!questname.empty())
	{
		int32_t value = quest::CQuestManager::Instance().GetQuestStateIndex(questname, arg1);

		pPC->SetFlag(questname + ".__status", value);
		pPC->ClearTimer();

		quest::PC::QuestInfoIterator it = pPC->quest_begin();
		uint32_t questindex = quest::CQuestManager::instance().GetQuestIndexByName(questname);

		while (it!= pPC->quest_end())
		{
			if (it->first == questindex)
			{
				it->second.st = value;
				break;
			}

			++it;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag %s %s %d", questname.c_str(), arg1, value);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag failed");
	}
}

LPCHARACTER chHori, chForge, chLib, chTemple, chTraining, chTree, chPortal, chBall;

ACMD(do_b1)
{
	chHori = CHARACTER_MANAGER::instance().SpawnMobRange(14017, ch->GetMapIndex(), 304222, 742858, 304222, 742858, true, false);
	chHori->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_BUILDING_CONSTRUCTION_SMALL, 65535, 0, true);
	chHori->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);

	for (int32_t i = 0; i < 30; ++i)
	{
		int32_t rot = number(0, 359);
		float fx, fy;
		GetDeltaByDegree(rot, 800, &fx, &fy);

		LPCHARACTER tch = CHARACTER_MANAGER::instance().SpawnMobRange(number(701, 706),
				ch->GetMapIndex(),
				304222 + (int32_t)fx,
				742858 + (int32_t)fy,
				304222 + (int32_t)fx,
				742858 + (int32_t)fy,
				true,
				false);
		tch->SetAggressive();
	}

	for (int32_t i = 0; i < 5; ++i)
	{
		int32_t rot = number(0, 359);
		float fx, fy;
		GetDeltaByDegree(rot, 800, &fx, &fy);

		LPCHARACTER tch = CHARACTER_MANAGER::instance().SpawnMobRange(8009,
				ch->GetMapIndex(),
				304222 + (int32_t)fx,
				742858 + (int32_t)fy,
				304222 + (int32_t)fx,
				742858 + (int32_t)fy,
				true,
				false);
		tch->SetAggressive();
	}
}

ACMD(do_b2)
{
	chHori->RemoveAffect(AFFECT_DUNGEON_UNIQUE);
}

ACMD(do_b3)
{
	chForge = CHARACTER_MANAGER::instance().SpawnMobRange(14003, ch->GetMapIndex(), 307500, 746300, 307500, 746300, true, false);
	chForge->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	chLib = CHARACTER_MANAGER::instance().SpawnMobRange(14007, ch->GetMapIndex(), 307900, 744500, 307900, 744500, true, false);
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	chTemple = CHARACTER_MANAGER::instance().SpawnMobRange(14004, ch->GetMapIndex(), 307700, 741600, 307700, 741600, true, false);
	chTemple->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	chTraining= CHARACTER_MANAGER::instance().SpawnMobRange(14010, ch->GetMapIndex(), 307100, 739500, 307100, 739500, true, false);
	chTraining->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	chTree= CHARACTER_MANAGER::instance().SpawnMobRange(14013, ch->GetMapIndex(), 300800, 741600, 300800, 741600, true, false);
	chTree->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	chPortal= CHARACTER_MANAGER::instance().SpawnMobRange(14001, ch->GetMapIndex(), 300900, 744500, 300900, 744500, true, false);
	chPortal->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
	chBall = CHARACTER_MANAGER::instance().SpawnMobRange(14012, ch->GetMapIndex(), 302500, 746600, 302500, 746600, true, false);
	chBall->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
}

ACMD(do_b4)
{
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_BUILDING_UPGRADE, 65535, 0, true);

	for (int32_t i = 0; i < 30; ++i)
	{
		int32_t rot = number(0, 359);
		float fx, fy;
		GetDeltaByDegree(rot, 1200, &fx, &fy);

		LPCHARACTER tch = CHARACTER_MANAGER::instance().SpawnMobRange(number(701, 706),
				ch->GetMapIndex(),
				307900 + (int32_t)fx,
				744500 + (int32_t)fy,
				307900 + (int32_t)fx,
				744500 + (int32_t)fy,
				true,
				false);
		tch->SetAggressive();
	}

	for (int32_t i = 0; i < 5; ++i)
	{
		int32_t rot = number(0, 359);
		float fx, fy;
		GetDeltaByDegree(rot, 1200, &fx, &fy);

		LPCHARACTER tch = CHARACTER_MANAGER::instance().SpawnMobRange(8009,
				ch->GetMapIndex(),
				307900 + (int32_t)fx,
				744500 + (int32_t)fy,
				307900 + (int32_t)fx,
				744500 + (int32_t)fy,
				true,
				false);
		tch->SetAggressive();
	}

}

ACMD(do_b5)
{
	M2_DESTROY_CHARACTER(chLib);
	chLib = CHARACTER_MANAGER::instance().SpawnMobRange(14008, ch->GetMapIndex(), 307900, 744500, 307900, 744500, true, false);
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
}

ACMD(do_b6)
{
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_BUILDING_UPGRADE, 65535, 0, true);
}
ACMD(do_b7)
{
	M2_DESTROY_CHARACTER(chLib);
	chLib = CHARACTER_MANAGER::instance().SpawnMobRange(14009, ch->GetMapIndex(), 307900, 744500, 307900, 744500, true, false);
	chLib->AddAffect(AFFECT_DUNGEON_UNIQUE, POINT_NONE, 0, AFF_DUNGEON_UNIQUE, 65535, 0, true);
}

ACMD(do_book)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	CSkillProto * pkProto;

	if (isnhdigit(*arg1))
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg1);
		pkProto = CSkillManager::instance().Get(vnum);
	}
	else
		pkProto = CSkillManager::instance().Get(arg1);

	if (!pkProto)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such a skill.");
		return;
	}

	LPITEM item = ch->AutoGiveItem(50300);
	item->SetSocket(0, pkProto->dwVnum);
}

ACMD(do_setskillother)
{
	char arg1[256], arg2[256], arg3[256];
	argument = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(argument, arg3, sizeof(arg3));

	if (!*arg1 || !*arg2 || !*arg3 || !isdigit(*arg3))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setskillother <target> <skillname> <lev>");
		return;
	}

	LPCHARACTER tch;

	tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
		return;
	}

	CSkillProto * pk;

	if (isdigit(*arg2))
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg2);
		pk = CSkillManager::instance().Get(vnum);
	}
	else
		pk = CSkillManager::instance().Get(arg2);

	if (!pk)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such a skill by that name.");
		return;
	}

	uint8_t level = 0;
	str_to_number(level, arg3);
	tch->SetSkillLevel(pk->dwVnum, level);
	tch->ComputePoints();
	tch->SkillLevelPacket();
}

ACMD(do_setskill)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2 || !isdigit(*arg2))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setskill <name> <lev>");
		return;
	}

	CSkillProto * pk;

	if (isdigit(*arg1))
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg1);
		pk = CSkillManager::instance().Get(vnum);
	}

	else
		pk = CSkillManager::instance().Get(arg1);

	if (!pk)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "No such a skill by that name.");
		return;
	}

	uint8_t level = 0;
	str_to_number(level, arg2);
	ch->SetSkillLevel(pk->dwVnum, level);
	ch->ComputePoints();
	ch->SkillLevelPacket();
}

ACMD(do_set_skill_point)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int32_t skill_point = 0;
	if (*arg1)
		str_to_number(skill_point, arg1);

	ch->SetRealPoint(POINT_SKILL, skill_point);
	ch->SetPoint(POINT_SKILL, ch->GetRealPoint(POINT_SKILL));
	ch->PointChange(POINT_SKILL, 0);
}

ACMD(do_set_skill_group)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int32_t skill_group = 0;
	if (*arg1)
		str_to_number(skill_group, arg1);

	ch->SetSkillGroup(skill_group);

	ch->ClearSkill();
	ch->ChatPacket(CHAT_TYPE_INFO, "skill group to %d.", skill_group);
}

ACMD(do_reload)
{
	char arg1[256];
	argument = one_argument(argument, arg1, sizeof(arg1));

// @correction235 BEGIN
	char arg2[256];
	one_argument(argument, arg2, sizeof(arg2));
	if (!strcmp(arg2, ""))
	{
		TPacketGGReloadCommand p2p_packet;
		p2p_packet.header = HEADER_GG_RELOAD_COMMAND;
		std::string s(arg1);
		s += " y";
		strlcpy(p2p_packet.argument, s.c_str(), sizeof(p2p_packet.argument));
		P2P_MANAGER::instance().Send(&p2p_packet, sizeof(p2p_packet));
		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Reloading other cores / channels.");
	}
// @correction235 END

	sys_err("Reload: %s, %s", arg1, arg2);
	if (*arg1)
	{
		switch (LOWER(*arg1))
		{
			case 'u':
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading state_user_count.");
				LoadStateUserCount();
				break;

			case 'p':
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading prototype tables,");
				db_clientdesc->DBPacket(HEADER_GD_RELOAD_PROTO, 0, nullptr, 0);
				break;

			case 's':
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading notice string.");
				DBManager::instance().LoadDBString();
				break;

			case 'q':
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading quest.");
				quest::CQuestManager::instance().Reload();
				break;

			case 'f':
				fishing::Initialize();
				break;

			case 'a':
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading Admin infomation.");
				db_clientdesc->DBPacket(HEADER_GD_RELOAD_ADMIN, 0, nullptr, 0);
				sys_log(0, "Reloading admin infomation.");
				break;
			case 'c':
				Cube_init();
				break;
// @correction234 BEGIN
			case 'g':
				extern bool __LoadGeneralConfigFile(const char* configName);
				__LoadGeneralConfigFile("/usr/game/share/conf/CONFIG");
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading general config.");
				break;
// @correction234 END
#ifdef __OFFLINE_PRIVATE_SHOP__
			case 'x':
				CShopManager::instance().ReadPriceCheckTable("/usr/game/share/locale/turkey/price_check.txt");
				if (ch)
					ch->ChatPacket(CHAT_TYPE_INFO, "Reloading general config.");
				break;
#endif
		}
	}
	else
	{
		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Reloading state_user_count.");
		LoadStateUserCount();

		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Reloading prototype tables,");
		db_clientdesc->DBPacket(HEADER_GD_RELOAD_PROTO, 0, nullptr, 0);

		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Reloading notice string.");
		DBManager::instance().LoadDBString();
	}
}

ACMD(do_cooltime)
{
	ch->DisableCooltime();
}

ACMD(do_level)
{
	char arg2[256];
	one_argument(argument, arg2, sizeof(arg2));

	if (!*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: level <level>");
		return;
	}

	int32_t	level = 0;
	str_to_number(level, arg2);

	ch->ResetPoint(MINMAX(1, level, gPlayerMaxLevel)); // @correction001 PLAYER_MAX_LEVEL_CONST -> gPlayerMaxLevel

	// ch->ClearSkill();
	// ch->ClearSubSkill();
}

ACMD(do_gwlist)
{
	ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("ÇöÀç ÀüÀïÁßÀÎ ±æµå ÀÔ´Ï´Ù"));
	CGuildManager::instance().ShowGuildWarList(ch);
}

ACMD(do_stop_guild_war)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	int32_t id1 = 0, id2 = 0;

	str_to_number(id1, arg1);
	str_to_number(id2, arg2);

	if (!id1 || !id2)
		return;

	if (id1 > id2)
	{
		std::swap(id1, id2);
	}

	ch->ChatPacket(CHAT_TYPE_TALKING, "%d %d", id1, id2);
	CGuildManager::instance().RequestEndWar(id1, id2);
}

ACMD(do_cancel_guild_war)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	int32_t id1 = 0, id2 = 0;
	str_to_number(id1, arg1);
	str_to_number(id2, arg2);

	if (id1 > id2)
		std::swap(id1, id2);

	CGuildManager::instance().RequestCancelWar(id1, id2);
}

ACMD(do_guild_state)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	CGuild* pGuild = CGuildManager::instance().FindGuildByName(arg1);
	if (pGuild != nullptr)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "GuildID: %d", pGuild->GetID());
		ch->ChatPacket(CHAT_TYPE_INFO, "GuildMasterPID: %d", pGuild->GetMasterPID());
		ch->ChatPacket(CHAT_TYPE_INFO, "IsInWar: %d", pGuild->UnderAnyWar());
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s: Á¸ÀçÇÏÁö ¾Ê´Â ±æµå ÀÔ´Ï´Ù."), arg1);
	}
}

struct FuncWeaken
{
	LPCHARACTER m_pkGM;
	bool	m_bAll;

	FuncWeaken(LPCHARACTER ch) : m_pkGM(ch), m_bAll(false)
	{
	}

	void operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
			return;

		LPCHARACTER pkChr = (LPCHARACTER) ent;

		int32_t iDist = DISTANCE_APPROX(pkChr->GetX() - m_pkGM->GetX(), pkChr->GetY() - m_pkGM->GetY());

		if (!m_bAll && iDist >= 1000)
			return;

		if (pkChr->IsNPC())
			pkChr->PointChange(POINT_HP, (10 - pkChr->GetHP()));
	}
};

ACMD(do_weaken)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	FuncWeaken func(ch);

	if (*arg1 && !strcmp(arg1, "all"))
		func.m_bAll = true;

	ch->GetSectree()->ForEachAround(func);
}

ACMD(do_getqf)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	LPCHARACTER tch;

	if (!*arg1)
		tch = ch;
	else
	{
		tch = CHARACTER_MANAGER::instance().FindPC(arg1);

		if (!tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
			return;
		}
	}

	quest::PC* pPC = quest::CQuestManager::instance().GetPC(tch->GetPlayerID());

	if (pPC)
		pPC->SendFlagList(ch);
}

ACMD(do_set_state)
{
	char arg1[256];
	char arg2[256];

	argument = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO,
			"Syntax: set_state <questname> <statename>"
#ifdef ENABLE_SET_STATE_WITH_TARGET // @correction159
			" [<character name>]"
#endif
		);
		return;
	}

#ifdef ENABLE_SET_STATE_WITH_TARGET // @correction159
	LPCHARACTER tch = ch;
	char arg3[256];
	argument = one_argument(argument, arg3, sizeof(arg3));
	if (*arg3)
	{
		tch = CHARACTER_MANAGER::instance().FindPC(arg3);
		if (!tch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
			return;
		}
	}
	quest::PC* pPC = quest::CQuestManager::instance().GetPCForce(tch->GetPlayerID());
#else
	quest::PC* pPC = quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
#endif
	std::string questname = arg1;
	std::string statename = arg2;

	if (!questname.empty())
	{
		int32_t value = quest::CQuestManager::Instance().GetQuestStateIndex(questname, statename);

		pPC->SetFlag(questname + ".__status", value);
		pPC->ClearTimer();

		quest::PC::QuestInfoIterator it = pPC->quest_begin();
		uint32_t questindex = quest::CQuestManager::instance().GetQuestIndexByName(questname);

		while (it!= pPC->quest_end())
		{
			if (it->first == questindex)
			{
				it->second.st = value;
				break;
			}

			++it;
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag %s %s %d", questname.c_str(), arg1, value);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "setting quest state flag failed");
	}
}

ACMD(do_setqf)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];

	one_argument(two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: setqf <flagname> <value> [<character name>]");
		return;
	}

	LPCHARACTER tch = ch;

	if (*arg3)
		tch = CHARACTER_MANAGER::instance().FindPC(arg3);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
		return;
	}

	quest::PC* pPC = quest::CQuestManager::instance().GetPC(tch->GetPlayerID());

	if (pPC)
	{
		int32_t value = 0;
		str_to_number(value, arg2);
		pPC->SetFlag(arg1, value);
		ch->ChatPacket(CHAT_TYPE_INFO, "Quest flag set: %s %d", arg1, value);
	}
}

ACMD(do_delqf)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: delqf <flagname> [<character name>]");
		return;
	}

	LPCHARACTER tch = ch;

	if (*arg2)
		tch = CHARACTER_MANAGER::instance().FindPC(arg2);

	if (!tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "There is no such character.");
		return;
	}

	quest::PC* pPC = quest::CQuestManager::instance().GetPC(tch->GetPlayerID());

	if (pPC)
	{
		if (pPC->DeleteFlag(arg1))
			ch->ChatPacket(CHAT_TYPE_INFO, "Delete success.");
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Delete failed. Quest flag does not exist.");
	}
}

ACMD(do_forgetme)
{
	ch->ForgetMyAttacker();
}

ACMD(do_aggregate)
{
	ch->AggregateMonster();
}

ACMD(do_attract_ranger)
{
	ch->AttractRanger();
}

ACMD(do_pull_monster)
{
	ch->PullMonster();
}

ACMD(do_polymorph)
{
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (*arg1)
	{
		uint32_t dwVnum = 0;
		str_to_number(dwVnum, arg1);
		bool bMaintainStat = false;
		if (*arg2)
		{
			int32_t value = 0;
			str_to_number(value, arg2);
			bMaintainStat = (value>0);
		}

		ch->SetPolymorph(dwVnum, bMaintainStat);
	}
}

ACMD(do_polymorph_item)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		uint32_t dwVnum = 0;
		str_to_number(dwVnum, arg1);

		LPITEM item = ITEM_MANAGER::instance().CreateItem(70104, 1, 0, true);
		if (item)
		{
			item->SetSocket(0, dwVnum);
			int32_t iEmptyPos = ch->GetEmptyInventory(item);

			if (iEmptyPos != -1)
			{
				item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
				LogManager::instance().ItemLog(ch, item, "GM", item->GetName());
			}
			else
			{
				M2_DESTROY_ITEM(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Not enough inventory space.");
			}
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "#%d item not exist by that vnum.", 70103);
		}
	}
}

ACMD(do_priv_empire)
{
	char arg1[256] = {0};
	char arg2[256] = {0};
	char arg3[256] = {0};
	char arg4[256] = {0};
	int32_t empire = 0;
	int32_t type = 0;
	int32_t value = 0;
	int32_t duration = 0;

	const char* line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		goto USAGE;

	if (!line)
		goto USAGE;

	two_arguments(line, arg3, sizeof(arg3), arg4, sizeof(arg4));

	if (!*arg3 || !*arg4)
		goto USAGE;

	str_to_number(empire, arg1);
	str_to_number(type,	arg2);
	str_to_number(value,	arg3);
	value = MINMAX(0, value, 1000);
	str_to_number(duration, arg4);

	if (empire < 0 || 3 < empire)
		goto USAGE;

	if (type < 1 || 4 < type)
		goto USAGE;

	if (value < 0)
		goto USAGE;

	if (duration < 0)
		goto USAGE;

	duration = duration * (60*60);

	sys_log(0, "_give_empire_privileage(empire=%d, type=%d, value=%d, duration=%d) by command",
			empire, type, value, duration);
	CPrivManager::instance().RequestGiveEmpirePriv(empire, type, value, duration);
	return;

USAGE:
	ch->ChatPacket(CHAT_TYPE_INFO, "usage : priv_empire <empire> <type> <value> <duration>");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <empire>    0 - 3 (0==all)");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <type>      1:item_drop, 2:gold_drop, 3:gold10_drop, 4:exp");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <value>     percent");
	ch->ChatPacket(CHAT_TYPE_INFO, "  <duration>  hour");
}

ACMD(do_priv_guild)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		CGuild * g = CGuildManager::instance().FindGuildByName(arg1);

		if (!g)
		{
			uint32_t guild_id = 0;
			str_to_number(guild_id, arg1);
			g = CGuildManager::instance().FindGuild(guild_id);
		}

		if (!g)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±×·± ÀÌ¸§ ¶Ç´Â ¹øÈ£ÀÇ ±æµå°¡ ¾ø½À´Ï´Ù."));
		else
		{
			char buf[1024+1];
			snprintf(buf, sizeof(buf), "%u", g->GetID());

			using namespace quest;
			PC * pc = CQuestManager::instance().GetPC(ch->GetPlayerID());
			QuestState qs = CQuestManager::instance().OpenState("ADMIN_QUEST", QUEST_FISH_REFINE_STATE_INDEX);
			luaL_loadbuffer(qs.co, buf, strlen(buf), "ADMIN_QUEST");
			pc->SetQuest("ADMIN_QUEST", qs);

			QuestState & rqs = *pc->GetRunningQuestState();

			if (!CQuestManager::instance().RunState(rqs))
			{
				CQuestManager::instance().CloseState(rqs);
				pc->EndRunning();
				return;
			}
		}
	}
}

ACMD(do_mount_test)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg1);
		ch->MountVnum(vnum);
	}
}

ACMD(do_observer)
{
	ch->SetObserverMode(!ch->IsObserverMode());
}

ACMD(do_socket_item)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1)
	{
		uint32_t dwVnum = 0;
		str_to_number(dwVnum, arg1);

		int32_t iSocketCount = 0;
		str_to_number(iSocketCount, arg2);

		if (!iSocketCount || iSocketCount >= ITEM_SOCKET_MAX_NUM)
			iSocketCount = 3;

		if (!dwVnum)
		{
			if (!ITEM_MANAGER::instance().GetVnum(arg1, dwVnum))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "#%d item not exist by that vnum.", dwVnum);
				return;
			}
		}

		LPITEM item = ch->AutoGiveItem(dwVnum);

		if (item)
		{
			for (int32_t i = 0; i < iSocketCount; ++i)
				item->SetSocket(i, 1);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "#%d cannot create item.", dwVnum);
		}
	}
}

ACMD(do_xmas)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	int32_t flag = 0;

	if (*arg1)
		str_to_number(flag, arg1);

	switch (subcmd)
	{
		case SCMD_XMAS_SNOW:
			quest::CQuestManager::instance().RequestSetEventFlag("xmas_snow", flag);
			break;

		case SCMD_XMAS_BOOM:
			quest::CQuestManager::instance().RequestSetEventFlag("xmas_boom", flag);
			break;

		case SCMD_XMAS_SANTA:
			quest::CQuestManager::instance().RequestSetEventFlag("xmas_santa", flag);
			break;
	}
}


ACMD(do_block_chat_list)
{
	if (!ch || (ch->GetGMLevel() < GM_HIGH_WIZARD && ch->GetQuestFlag("chat_privilege.block") <= 0))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±×·± ¸í·É¾î´Â ¾ø½À´Ï´Ù"));
		return;
	}

	DBManager::instance().ReturnQuery(QID_BLOCK_CHAT_LIST, ch->GetPlayerID(), nullptr,
			"SELECT p.name, a.lDuration FROM affect%s as a, player%s as p WHERE a.bType = %d AND a.dwPID = p.id",
			get_table_postfix(), get_table_postfix(), AFFECT_BLOCK_CHAT);
}

ACMD(do_vote_block_chat)
{
	return;

	char arg1[256];
	argument = one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: vote_block_chat <name>");
		return;
	}

	const char* name = arg1;
	int32_t lBlockDuration = 10;
	sys_log(0, "vote_block_chat %s %d", name, lBlockDuration);

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(name);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(name);

		if (pkCCI)
		{
			TPacketGGBlockChat p;

			p.bHeader = HEADER_GG_BLOCK_CHAT;
			strlcpy(p.szName, name, sizeof(p.szName));
			p.lBlockDuration = lBlockDuration;
			P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGBlockChat));
		}
		else
		{
			TPacketBlockChat p;

			strlcpy(p.szName, name, sizeof(p.szName));
			p.lDuration = lBlockDuration;
			db_clientdesc->DBPacket(HEADER_GD_BLOCK_CHAT, ch ? ch->GetDesc()->GetHandle() : 0, &p, sizeof(p));

		}

		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Chat block requested.");

		return;
	}

	if (tch && ch != tch)
		tch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, AFF_NONE, lBlockDuration, 0, true);
}

ACMD(do_block_chat)
{
	if (ch && (ch->GetGMLevel() < GM_LOW_WIZARD && ch->GetQuestFlag("chat_privilege.block") <= 0))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±×·± ¸í·É¾î´Â ¾ø½À´Ï´Ù"));
		return;
	}

	char arg1[256];
	argument = one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Usage: block_chat <name> <time> (0 to off)");

		return;
	}

	const char* name = arg1;
	int32_t lBlockDuration = parse_time_str(argument);

	if (lBlockDuration < 0)
	{
		if (ch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Àß¸øµÈ Çü½ÄÀÇ ½Ã°£ÀÔ´Ï´Ù. h, m, s¸¦ ºÙ¿©¼­ ÁöÁ¤ÇØ ÁÖ½Ê½Ã¿À.");
			ch->ChatPacket(CHAT_TYPE_INFO, "¿¹) 10s, 10m, 1m 30s");
		}
		return;
	}

	sys_log(0, "BLOCK CHAT %s %d", name, lBlockDuration);

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(name);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(name);

		if (pkCCI)
		{
			TPacketGGBlockChat p;

			p.bHeader = HEADER_GG_BLOCK_CHAT;
			strlcpy(p.szName, name, sizeof(p.szName));
			p.lBlockDuration = lBlockDuration;
			P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGBlockChat));
		}
		else
		{
			TPacketBlockChat p;

			strlcpy(p.szName, name, sizeof(p.szName));
			p.lDuration = lBlockDuration;
			db_clientdesc->DBPacket(HEADER_GD_BLOCK_CHAT, ch ? ch->GetDesc()->GetHandle() : 0, &p, sizeof(p));
		}

		if (ch)
			ch->ChatPacket(CHAT_TYPE_INFO, "Chat block requested.");

		return;
	}

	if (tch && ch != tch)
		tch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, AFF_NONE, lBlockDuration, 0, true);
}

ACMD(do_build)
{
	using namespace building;

	char arg1[256], arg2[256], arg3[256], arg4[256];
	const char * line = one_argument(argument, arg1, sizeof(arg1));
	uint8_t GMLevel = ch->GetGMLevel();

	CLand * pkLand = CManager::instance().FindLand(ch->GetMapIndex(), ch->GetX(), ch->GetY());

	if (!pkLand)
	{
		sys_err("%s trying to build on not buildable area.", ch->GetName());
		return;
	}

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax: no command");
		return;
	}

	if (GMLevel == GM_PLAYER)
	{
		if ((!ch->GetGuild() || ch->GetGuild()->GetID() != pkLand->GetOwner()))
		{
			sys_err("%s trying to build on not owned land.", ch->GetName());
			return;
		}

		if (ch->GetGuild()->GetMasterPID() != ch->GetPlayerID())
		{
			sys_err("%s trying to build while not the guild master.", ch->GetName());
			return;
		}
	}

	switch (LOWER(*arg1))
	{
		case 'c':
			{
				char arg5[256], arg6[256];
				line = one_argument(two_arguments(line, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));
				one_argument(two_arguments(line, arg4, sizeof(arg4), arg5, sizeof(arg5)), arg6, sizeof(arg6));

				if (!*arg1 || !*arg2 || !*arg3 || !*arg4 || !*arg5 || !*arg6)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax");
					return;
				}

				uint32_t dwVnum = 0;
				str_to_number(dwVnum,  arg1);

				using namespace building;

				const TObjectProto * t = CManager::instance().GetObjectProto(dwVnum);
				if (!t)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á¸ÀçÇÏÁö ¾Ê´Â °Ç¹°ÀÔ´Ï´Ù."));
					return;
				}

				const uint32_t BUILDING_MAX_PRICE = 100000000;

				if (t->dwGroupVnum)
				{
					if (pkLand->FindObjectByGroup(t->dwGroupVnum))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°°ÀÌ ÁöÀ» ¼ö ¾ø´Â Á¾·ùÀÇ °Ç¹°ÀÌ Áö¾îÁ® ÀÖ½À´Ï´Ù."));
						return;
					}
				}

				if (t->dwDependOnGroupVnum)
				{
					{
						if (!pkLand->FindObjectByGroup(t->dwDependOnGroupVnum))
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Ç¼³¿¡ ÇÊ¿äÇÑ °Ç¹°ÀÌ Áö¾îÁ® ÀÖÁö ¾Ê½À´Ï´Ù."));
							return;
						}
					}
				}

				if (test_server || GMLevel == GM_PLAYER)
				{
					if (t->dwPrice > BUILDING_MAX_PRICE)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Ç¹° ºñ¿ë Á¤º¸ ÀÌ»óÀ¸·Î °Ç¼³ ÀÛ¾÷¿¡ ½ÇÆÐÇß½À´Ï´Ù."));
						return;
					}

					if (ch->GetGold() < t->dwPrice)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Ç¼³ ºñ¿ëÀÌ ºÎÁ·ÇÕ´Ï´Ù."));
						return;
					}


					int32_t i;
					for (i = 0; i < OBJECT_MATERIAL_MAX_NUM; ++i)
					{
						uint32_t dwItemVnum = t->kMaterials[i].dwItemVnum;
						uint32_t dwItemCount = t->kMaterials[i].dwCount;

						if (dwItemVnum == 0)
							break;

						if ((int32_t) dwItemCount > ch->CountSpecifyItem(dwItemVnum))
						{
							ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÚÀç°¡ ºÎÁ·ÇÏ¿© °Ç¼³ÇÒ ¼ö ¾ø½À´Ï´Ù."));
							return;
						}
					}
				}

				float x_rot = atof(arg4);
				float y_rot = atof(arg5);
				float z_rot = atof(arg6);

				int32_t map_x = 0;
				str_to_number(map_x, arg2);
				int32_t map_y = 0;
				str_to_number(map_y, arg3);

				bool isSuccess = pkLand->RequestCreateObject(dwVnum,
						ch->GetMapIndex(),
						map_x,
						map_y,
						x_rot,
						y_rot,
						z_rot, true);

				if (!isSuccess)
				{
					if (test_server)
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Ç¹°À» ÁöÀ» ¼ö ¾ø´Â À§Ä¡ÀÔ´Ï´Ù."));
					return;
				}

				if (test_server || GMLevel == GM_PLAYER)
				{
					ch->ChangeGold(false, t->dwPrice);

					{
						int32_t i;
						for (i = 0; i < OBJECT_MATERIAL_MAX_NUM; ++i)
						{
							uint32_t dwItemVnum = t->kMaterials[i].dwItemVnum;
							uint32_t dwItemCount = t->kMaterials[i].dwCount;

							if (dwItemVnum == 0)
								break;

							sys_log(0, "BUILD: material %d %u %u", i, dwItemVnum, dwItemCount);
							ch->RemoveSpecifyItem(dwItemVnum, dwItemCount);
						}
					}
				}
			}
			break;

		case 'd' :
			{
				one_argument(line, arg1, sizeof(arg1));

				if (!*arg1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Invalid syntax");
					return;
				}

				uint32_t vid = 0;
				str_to_number(vid, arg1);
				pkLand->RequestDeleteObjectByVID(vid);
			}
			break;


		case 'w' :
			if (GMLevel > GM_PLAYER)
			{
				int32_t mapIndex = ch->GetMapIndex();

				one_argument(line, arg1, sizeof(arg1));

				sys_log(0, "guild.wall.build map[%d] direction[%s]", mapIndex, arg1);

				switch (arg1[0])
				{
					case 's':
						pkLand->RequestCreateWall(mapIndex,   0.0f);
						break;
					case 'n':
						pkLand->RequestCreateWall(mapIndex, 180.0f);
						break;
					case 'e':
						pkLand->RequestCreateWall(mapIndex,  90.0f);
						break;
					case 'w':
						pkLand->RequestCreateWall(mapIndex, 270.0f);
						break;
					default:
						ch->ChatPacket(CHAT_TYPE_INFO, "guild.wall.build unknown_direction[%s]", arg1);
						sys_err("guild.wall.build unknown_direction[%s]", arg1);
						break;
				}

			}
			break;

		case 'e':
			if (GMLevel > GM_PLAYER)
			{
				pkLand->RequestDeleteWall();
			}
			break;

		case 'W' :

			if (GMLevel >  GM_PLAYER)
			{
				int32_t setID = 0, wallSize = 0;
				char arg5[256], arg6[256];
				line = two_arguments(line, arg1, sizeof(arg1), arg2, sizeof(arg2));
				line = two_arguments(line, arg3, sizeof(arg3), arg4, sizeof(arg4));
				two_arguments(line, arg5, sizeof(arg5), arg6, sizeof(arg6));

				str_to_number(setID, arg1);
				str_to_number(wallSize, arg2);

				if (setID != 14105 && setID != 14115 && setID != 14125)
				{
					sys_log(0, "BUILD_WALL: wrong wall set id %d", setID);
					break;
				}
				else
				{
					bool door_east = false;
					str_to_number(door_east, arg3);
					bool door_west = false;
					str_to_number(door_west, arg4);
					bool door_south = false;
					str_to_number(door_south, arg5);
					bool door_north = false;
					str_to_number(door_north, arg6);
					pkLand->RequestCreateWallBlocks(setID, ch->GetMapIndex(), wallSize, door_east, door_west, door_south, door_north);
				}
			}
			break;

		case 'E' :
			if (GMLevel > GM_PLAYER)
			{
				one_argument(line, arg1, sizeof(arg1));
				uint32_t id = 0;
				str_to_number(id, arg1);
				pkLand->RequestDeleteWallBlocks(id);
			}
			break;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, "Invalid command %s", arg1);
			break;
	}
}

ACMD(do_clear_quest)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	quest::PC* pPC = quest::CQuestManager::instance().GetPCForce(ch->GetPlayerID());
	pPC->ClearQuest(arg1);
}

ACMD(do_horse_state)
{
	ch->ChatPacket(CHAT_TYPE_INFO, "Horse Information:");
	ch->ChatPacket(CHAT_TYPE_INFO, "    Level  %d", ch->GetHorseLevel());
	ch->ChatPacket(CHAT_TYPE_INFO, "    Health %d/%d (%d%%)", ch->GetHorseHealth(), ch->GetHorseMaxHealth(), ch->GetHorseHealth() * 100 / ch->GetHorseMaxHealth());
	ch->ChatPacket(CHAT_TYPE_INFO, "    Stam   %d/%d (%d%%)", ch->GetHorseStamina(), ch->GetHorseMaxStamina(), ch->GetHorseStamina() * 100 / ch->GetHorseMaxStamina());
}

ACMD(do_horse_level)
{
	char arg1[256] = {0};
	char arg2[256] = {0};
	LPCHARACTER victim;
	int32_t	level = 0;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage : /horse_level <name> <level>");
		return;
	}

	victim = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (nullptr == victim)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á¸ÀçÇÏÁö ¾Ê´Â Ä³¸¯ÅÍ ÀÔ´Ï´Ù."));
		return;
	}

	str_to_number(level, arg2);
	level = MINMAX(0, level, HORSE_MAX_LEVEL);

	ch->ChatPacket(CHAT_TYPE_INFO, "horse level set (%s: %d)", victim->GetName(), level);

	victim->SetHorseLevel(level);
	victim->ComputePoints();
	victim->SkillLevelPacket();
	return;
}

ACMD(do_horse_ride)
{
	if (ch->IsHorseRiding())
		ch->StopRiding();
	else
		ch->StartRiding();
}

ACMD(do_horse_summon)
{
	ch->HorseSummon(true, true);
}

ACMD(do_horse_unsummon)
{
	ch->HorseSummon(false, true);
}

ACMD(do_horse_set_stat)
{
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		int32_t hp = 0;
		str_to_number(hp, arg1);
		int32_t stam = 0;
		str_to_number(stam, arg2);
		ch->UpdateHorseHealth(hp - ch->GetHorseHealth());
		ch->UpdateHorseStamina(stam - ch->GetHorseStamina());
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage : /horse_set_stat <hp> <stamina>");
	}
}

ACMD(do_save_attribute_to_image)
{
	char szFileName[256];
	char szMapIndex[256];

	two_arguments(argument, szMapIndex, sizeof(szMapIndex), szFileName, sizeof(szFileName));

	if (!*szMapIndex || !*szFileName)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /saveati <map_index> <filename>");
		return;
	}

	int32_t lMapIndex = 0;
	str_to_number(lMapIndex, szMapIndex);

	if (SECTREE_MANAGER::instance().SaveAttributeToImage(lMapIndex, szFileName))
		ch->ChatPacket(CHAT_TYPE_INFO, "Save done.");
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Save failed.");
}

ACMD(do_affect_remove)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /affect_remove <player name>");
		ch->ChatPacket(CHAT_TYPE_INFO, "Syntax: /affect_remove <type> <point>");

		LPCHARACTER tch = ch;

		if (*arg1)
			if (!(tch = CHARACTER_MANAGER::instance().FindPC(arg1)))
				tch = ch;

		ch->ChatPacket(CHAT_TYPE_INFO, "-- Affect List of %s -------------------------------", tch->GetName());
		ch->ChatPacket(CHAT_TYPE_INFO, "Type Point Modif Duration Flag");

		const std::list<CAffect *> & cont = tch->GetAffectContainer();

		itertype(cont) it = cont.begin();

		while (it != cont.end())
		{
			CAffect * pkAff = *it++;

			ch->ChatPacket(CHAT_TYPE_INFO, "%4d %5d %5d %8d %u",
					pkAff->dwType, pkAff->bApplyOn, pkAff->lApplyValue, pkAff->lDuration, pkAff->dwFlag);
		}
		return;
	}

	bool removed = false;

	CAffect * af;

	uint32_t	type = 0;
	str_to_number(type, arg1);
	uint8_t	point = 0;
	str_to_number(point, arg2);
	while ((af = ch->FindAffect(type, point)))
	{
		ch->RemoveAffect(af);
		removed = true;
	}

	if (removed)
		ch->ChatPacket(CHAT_TYPE_INFO, "Affect successfully removed.");
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Not affected by that type and point.");
}

ACMD(do_change_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->ChangeAttribute();
}

ACMD(do_add_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->AddAttribute();
}

ACMD(do_add_socket)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->AddSocket();
}

#ifdef ENABLE_NEWSTUFF //@correction232
ACMD(do_change_rare_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->ChangeRareAttribute();
}

ACMD(do_add_rare_attr)
{
	LPITEM weapon = ch->GetWear(WEAR_WEAPON);
	if (weapon)
		weapon->AddRareAttribute();
}
#endif

ACMD(do_show_arena_list)
{
	CArenaManager::instance().SendArenaMapListTo(ch);
}

ACMD(do_end_all_duel)
{
	CArenaManager::instance().EndAllDuel();
}

ACMD(do_end_duel)
{
	char szName[256];

	one_argument(argument, szName, sizeof(szName));

	LPCHARACTER pChar = CHARACTER_MANAGER::instance().FindPC(szName);
	if (pChar == nullptr)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á¸ÀçÇÏÁö ¾Ê´Â Ä³¸¯ÅÍ ÀÔ´Ï´Ù."));
		return;
	}

	if (CArenaManager::instance().EndDuel(pChar->GetPlayerID()) == false)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã °­Á¦ Á¾·á ½ÇÆÐ"));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã °­Á¦ Á¾·á ¼º°ø"));
	}
}

ACMD(do_duel)
{
	char szName1[256];
	char szName2[256];
	char szSet[256];
	char szMinute[256];
	int32_t set = 0;
	int32_t minute = 0;

	argument = two_arguments(argument, szName1, sizeof(szName1), szName2, sizeof(szName2));
	two_arguments(argument, szSet, sizeof(szSet), szMinute, sizeof(szMinute));

	str_to_number(set, szSet);

	if (set < 0) set = 1;
	if (set > 5) set = 5;

	// @correction005 BEGIN
	if (!str_to_number(minute, szMinute))
		minute = 5;
	// @correction005 END

	if (minute < 5)
		minute = 5;

	LPCHARACTER pChar1 = CHARACTER_MANAGER::instance().FindPC(szName1);
	LPCHARACTER pChar2 = CHARACTER_MANAGER::instance().FindPC(szName2);

	if (pChar1 != nullptr && pChar2 != nullptr)
	{
		pChar1->RemoveGoodAffect();
		pChar2->RemoveGoodAffect();

		pChar1->RemoveBadAffect();
		pChar2->RemoveBadAffect();

		LPPARTY pParty = pChar1->GetParty();
		if (pParty != nullptr)
		{
			if (pParty->GetMemberCount() == 2)
			{
				CPartyManager::instance().DeleteParty(pParty);
			}
			else
			{
				pChar1->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼¿¡¼­ ³ª°¡¼Ì½À´Ï´Ù."));
				pParty->Quit(pChar1->GetPlayerID());
			}
		}

		pParty = pChar2->GetParty();
		if (pParty != nullptr)
		{
			if (pParty->GetMemberCount() == 2)
			{
				CPartyManager::instance().DeleteParty(pParty);
			}
			else
			{
				pChar2->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼¿¡¼­ ³ª°¡¼Ì½À´Ï´Ù."));
				pParty->Quit(pChar2->GetPlayerID());
			}
		}

		if (CArenaManager::instance().StartDuel(pChar1, pChar2, set, minute) == true)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀÌ ¼º°øÀûÀ¸·Î ½ÃÀÛ µÇ¾ú½À´Ï´Ù."));
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã ½ÃÀÛ¿¡ ¹®Á¦°¡ ÀÖ½À´Ï´Ù."));
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀÚ°¡ ¾ø½À´Ï´Ù."));
	}
}

ACMD(do_stat_plus_amount)
{
	char szPoint[256];

	one_argument(argument, szPoint, sizeof(szPoint));

	if (*szPoint == '\0')
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µÐ°© Áß¿¡´Â ´É·ÂÀ» ¿Ã¸± ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	int32_t nRemainPoint = ch->GetPoint(POINT_STAT);

	if (nRemainPoint <= 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³²Àº ½ºÅÈ Æ÷ÀÎÆ®°¡ ¾ø½À´Ï´Ù."));
		return;
	}

	int32_t nPoint = 0;
	str_to_number(nPoint, szPoint);

	if (nRemainPoint < nPoint)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³²Àº ½ºÅÈ Æ÷ÀÎÆ®°¡ Àû½À´Ï´Ù."));
		return;
	}

	if (nPoint < 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°ªÀ» Àß¸ø ÀÔ·ÂÇÏ¿´½À´Ï´Ù."));
		return;
	}

	switch (subcmd)
	{
		case POINT_HT :
			if (nPoint + ch->GetPoint(POINT_HT) > g_iStatusPointSetMaxValue) // @correction218
			{
				nPoint = g_iStatusPointSetMaxValue - ch->GetPoint(POINT_HT); // @correction218
			}
			break;

		case POINT_IQ :
			if (nPoint + ch->GetPoint(POINT_IQ) > g_iStatusPointSetMaxValue) // @correction218
			{
				nPoint = g_iStatusPointSetMaxValue - ch->GetPoint(POINT_IQ); // @correction218
			}
			break;

		case POINT_ST :
			if (nPoint + ch->GetPoint(POINT_ST) > g_iStatusPointSetMaxValue) // @correction218
			{
				nPoint = g_iStatusPointSetMaxValue - ch->GetPoint(POINT_ST); // @correction218
			}
			break;

		case POINT_DX :
			if (nPoint + ch->GetPoint(POINT_DX) > g_iStatusPointSetMaxValue) // @correction218
			{
				nPoint = g_iStatusPointSetMaxValue - ch->GetPoint(POINT_DX); // @correction218
			}
			break;

		default :
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸í·É¾îÀÇ ¼­ºê Ä¿¸Çµå°¡ Àß¸ø µÇ¾ú½À´Ï´Ù."));
			return;
			break;
	}

	if (nPoint != 0)
	{
		ch->SetRealPoint(subcmd, ch->GetRealPoint(subcmd) + nPoint);
		ch->SetPoint(subcmd, ch->GetPoint(subcmd) + nPoint);
		ch->ComputePoints();
		ch->PointChange(subcmd, 0);

		ch->PointChange(POINT_STAT, -nPoint);
		ch->ComputePoints();
	}
}

struct tTwoPID
{
	int32_t pid1;
	int32_t pid2;
};

ACMD(do_break_marriage)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	tTwoPID pids = { 0, 0 };

	str_to_number(pids.pid1, arg1);
	str_to_number(pids.pid2, arg2);

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÇÃ·¹ÀÌ¾î %d ¿Í ÇÃ·¹ÀÌ¾î  %d¸¦ ÆÄÈ¥½ÃÅµ´Ï´Ù.."), pids.pid1, pids.pid2);
	db_clientdesc->DBPacket(HEADER_GD_BREAK_MARRIAGE, 0, &pids, sizeof(pids));
}

ACMD(do_effect)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	int32_t	effect_type = 0;
	str_to_number(effect_type, arg1);
	ch->EffectPacket(effect_type);
}


struct FCountInMap
{
	int32_t m_Count[4];
	FCountInMap() { memset(m_Count, 0, sizeof(int32_t) * 4); }
	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;
			if (ch && ch->IsPC())
				++m_Count[ch->GetEmpire()];
		}
	}
	int32_t GetCount(uint8_t bEmpire) { return m_Count[bEmpire]; }
};

ACMD(do_threeway_war_info)
{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°¢Á¦±¹ ÁøÇà Á¤º¸"));
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼±ÅÃ ¸Ê Á¤º¸ ¼ºÁö %d Åë·Î %d %d %d"), GetSungziMapIndex(), GetPassMapIndex(1), GetPassMapIndex(2), GetPassMapIndex(3));
	ch->ChatPacket(CHAT_TYPE_INFO, "ThreewayPhase %d", CThreeWayWar::instance().GetRegenFlag());

	for (int32_t n = 1; n < 4; ++n)
	{
		LPSECTREE_MAP pSecMap = SECTREE_MANAGER::instance().GetMap(GetSungziMapIndex());

		FCountInMap c;

		if (pSecMap)
		{
			pSecMap->for_each(c);
		}

		ch->ChatPacket(CHAT_TYPE_INFO, "%s killscore %d usercount %d",
				EMPIRE_NAME(n),
			   	CThreeWayWar::instance().GetKillScore(n),
				c.GetCount(n));
	}
}

ACMD(do_threeway_war_myinfo)
{
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³ªÀÇ »ï°Å¸® ÁøÇàÁ¤º¸"));
	ch->ChatPacket(CHAT_TYPE_INFO, "Deadcount %d",
			CThreeWayWar::instance().GetReviveTokenForPlayer(ch->GetPlayerID()));
}

ACMD(do_rmcandidacy)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: rmcandidacy <name>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Target is in %d channel (my channel %d)", pkCCI->bChannel, g_bChannel);
				return;
			}
		}
	}

	db_clientdesc->DBPacket(HEADER_GD_RMCANDIDACY, 0, nullptr, 32);
	db_clientdesc->Packet(arg1, 32);
}

ACMD(do_setmonarch)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: setmonarch <name>");
		return;
	}

	db_clientdesc->DBPacket(HEADER_GD_SETMONARCH, 0, nullptr, 32);
	db_clientdesc->Packet(arg1, 32);
}

ACMD(do_rmmonarch)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: rmmonarch <name>");
		return;
	}

	db_clientdesc->DBPacket(HEADER_GD_RMMONARCH, 0, nullptr, 32);
	db_clientdesc->Packet(arg1, 32);
}

ACMD(do_check_monarch_money)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	int32_t empire = 0;
	str_to_number(empire, arg1);
	int32_t NationMoney = CMonarch::instance().GetMoney(empire);

	ch->ChatPacket(CHAT_TYPE_INFO, "±¹°í: %d ¿ø", NationMoney);
}

ACMD(do_reset_subskill)
{
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: reset_subskill <name>");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (tch == nullptr)
		return;

	tch->ClearSubSkill();
	ch->ChatPacket(CHAT_TYPE_INFO, "Subskill of [%s] was reset", tch->GetName());
}

ACMD(do_siege)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	int32_t	empire = strtol(arg1, nullptr, 10);
	int32_t tower_count = strtol(arg2, nullptr, 10);

	if (empire == 0) empire = number(1, 3);
	if (tower_count < 5 || tower_count > 10) tower_count = number(5, 10);

	TPacketGGSiege packet;
	packet.bHeader = HEADER_GG_SIEGE;
	packet.bEmpire = empire;
	packet.bTowerCount = tower_count;

	P2P_MANAGER::instance().Send(&packet, sizeof(TPacketGGSiege));

	switch (castle_siege(empire, tower_count))
	{
		case 0 :
			ch->ChatPacket(CHAT_TYPE_INFO, "SIEGE FAILED");
			break;
		case 1 :
			ch->ChatPacket(CHAT_TYPE_INFO, "SIEGE START Empire(%d) Tower(%d)", empire, tower_count);
			break;
		case 2 :
			ch->ChatPacket(CHAT_TYPE_INFO, "SIEGE END");
			break;
	}
}

ACMD(do_temp)
{
	if (false == test_server)
		return;

	char	arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (0 == arg1[0] || 0 == arg2[0])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: empire money");
		return;
	}

	int32_t	empire = 0;
	str_to_number(empire, arg1);
	int32_t	money = 0;
	str_to_number(money, arg2);

	CMonarch::instance().SendtoDBAddMoney(money, empire, ch);
}

ACMD(do_frog)
{
	char	arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (0 == arg1[0])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: empire(1, 2, 3)");
		return;
	}

	int32_t	empire = 0;
	str_to_number(empire, arg1);

	switch (empire)
	{
		case 1:
		case 2:
		case 3:
			if (IS_CASTLE_MAP(ch->GetMapIndex()))
			{
				castle_spawn_frog(empire);
				castle_save();
			}
			else
				ch->ChatPacket(CHAT_TYPE_INFO, "You must spawn frog in castle");
			break;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, "Usage: empire(1, 2, 3)");
			break;
	}

}

ACMD(do_flush)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (0 == arg1[0])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "usage : /flush player_id");
		return;
	}

	uint32_t pid = (uint32_t) strtoul(arg1, nullptr, 10);

	db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
	db_clientdesc->Packet(&pid, sizeof(uint32_t));
}

ACMD(do_eclipse)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (strtol(arg1, nullptr, 10) == 1)
	{
		quest::CQuestManager::instance().RequestSetEventFlag("eclipse", 1);
	}
	else
	{
		quest::CQuestManager::instance().RequestSetEventFlag("eclipse", 0);
	}
}

ACMD(do_weeklyevent)
{
	char arg1[256];
	int32_t empire = 0;

	if (CBattleArena::instance().IsRunning() == false)
	{
		one_argument(argument, arg1, sizeof(arg1));

		empire = strtol(arg1, nullptr, 10);

		if (empire == 1 || empire == 2 || empire == 3)
		{
			CBattleArena::instance().Start(empire);
		}
		else
		{
			CBattleArena::instance().Start(rand()%3 + 1);
		}
		ch->ChatPacket(CHAT_TYPE_INFO, "Weekly Event Start");
	}
	else
	{
		CBattleArena::instance().ForceEnd();
		ch->ChatPacket(CHAT_TYPE_INFO, "Weekly Event End");
	}
}

ACMD(do_event_helper)
{
	char arg1[256];
	int32_t mode = 0;

	one_argument(argument, arg1, sizeof(arg1));
	str_to_number(mode, arg1);

	if (mode == 1)
	{
		xmas::SpawnEventHelper(true);
		ch->ChatPacket(CHAT_TYPE_INFO, "Event Helper Spawn");
	}
	else
	{
		xmas::SpawnEventHelper(false);
		ch->ChatPacket(CHAT_TYPE_INFO, "Event Helper Delete");
	}
}

struct FMobCounter
{
	int32_t nCount;

	void operator () (LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pChar = static_cast<LPCHARACTER>(ent);

			if (pChar->IsMonster() == true || pChar->IsStone())
			{
				nCount++;
			}
		}
	}
};

ACMD(do_get_mob_count)
{
	LPSECTREE_MAP pSectree = SECTREE_MANAGER::instance().GetMap(ch->GetMapIndex());

	if (pSectree == nullptr)
		return;

	FMobCounter f;
	f.nCount = 0;

	pSectree->for_each(f);

	ch->ChatPacket(CHAT_TYPE_INFO, "MapIndex: %d MobCount %d", ch->GetMapIndex(), f.nCount);
}

ACMD(do_clear_land)
{
	const building::CLand* pLand = building::CManager::instance().FindLand(ch->GetMapIndex(), ch->GetX(), ch->GetY());

	if( nullptr == pLand )
	{
		return;
	}

	ch->ChatPacket(CHAT_TYPE_INFO, "Guild Land(%d) Cleared", pLand->GetID());

	building::CManager::instance().ClearLand(pLand->GetID());
}

ACMD(do_special_item)
{
    ITEM_MANAGER::instance().ConvSpecialDropItemFile();
}

ACMD(do_set_stat)
{
	char szName [256];
	char szChangeAmount[256];

	two_arguments (argument, szName, sizeof (szName), szChangeAmount, sizeof(szChangeAmount));

	if (*szName == '\0' || *szChangeAmount == '\0')
	{
		ch->ChatPacket (CHAT_TYPE_INFO, "Invalid argument.");
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(szName);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(szName);

		if (pkCCI)
		{
			ch->ChatPacket (CHAT_TYPE_INFO, "Cannot find player(%s). %s is not in your game server.", szName, szName);
			return;
		}
		else
		{
			ch->ChatPacket (CHAT_TYPE_INFO, "Cannot find player(%s). Perhaps %s doesn't login or exist.", szName, szName);
			return;
		}
	}
	else
	{
		if (tch->IsPolymorphed())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µÐ°© Áß¿¡´Â ´É·ÂÀ» ¿Ã¸± ¼ö ¾ø½À´Ï´Ù."));
			return;
		}

		if (subcmd != POINT_HT && subcmd != POINT_IQ && subcmd != POINT_ST && subcmd != POINT_DX)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸í·É¾îÀÇ ¼­ºê Ä¿¸Çµå°¡ Àß¸ø µÇ¾ú½À´Ï´Ù."));
			return;
		}
		int32_t nRemainPoint = tch->GetPoint(POINT_STAT);
		int32_t nCurPoint = tch->GetRealPoint(subcmd);
		int32_t nChangeAmount = 0;
		str_to_number(nChangeAmount, szChangeAmount);
		int32_t nPoint = nCurPoint + nChangeAmount;

		int32_t n = -1;
		switch (subcmd)
		{
		case POINT_HT:
			if (nPoint < JobInitialPoints[tch->GetJob()].ht)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
				return;
			}
			n = 0;
			break;
		case POINT_IQ:
			if (nPoint < JobInitialPoints[tch->GetJob()].iq)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
				return;
			}
			n = 1;
			break;
		case POINT_ST:
			if (nPoint < JobInitialPoints[tch->GetJob()].st)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
				return;
			}
			n = 2;
			break;
		case POINT_DX:
			if (nPoint < JobInitialPoints[tch->GetJob()].dx)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot set stat under initial stat."));
				return;
			}
			n = 3;
			break;
		}

		if (nPoint > 90)
		{
			nChangeAmount -= nPoint - 90;
			nPoint = 90;
		}

		if (nRemainPoint < nChangeAmount)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³²Àº ½ºÅÈ Æ÷ÀÎÆ®°¡ Àû½À´Ï´Ù."));
			return;
		}

		tch->SetRealPoint(subcmd, nPoint);
		tch->SetPoint(subcmd, tch->GetPoint(subcmd) + nChangeAmount);
		tch->ComputePoints();
		tch->PointChange(subcmd, 0);

		tch->PointChange(POINT_STAT, -nChangeAmount);
		tch->ComputePoints();

		const char* stat_name[4] = {"con", "int32_t", "str", "dex"};
		if (-1 == n)
			return;
		ch->ChatPacket(CHAT_TYPE_INFO, "%s's %s change %d to %d", szName, stat_name[n], nCurPoint, nPoint);
	}
}

ACMD(do_get_item_id_list)
{
	for (int32_t i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; i++)
	{
		LPITEM item = ch->GetInventoryItem(i);
		if (item != nullptr)
			ch->ChatPacket(CHAT_TYPE_INFO, "cell : %u, name : %s, id : %d", item->GetCell(), item->GetName(), item->GetID());
	}
}

ACMD(do_set_socket)
{
	char arg1 [256];
	char arg2 [256];
	char arg3 [256];

	one_argument (two_arguments (argument, arg1, sizeof (arg1), arg2, sizeof(arg2)), arg3, sizeof (arg3));

	int32_t item_id, socket_num, value;
	if (!str_to_number (item_id, arg1) || !str_to_number (socket_num, arg2) || !str_to_number (value, arg3))
		return;

	LPITEM item = ITEM_MANAGER::instance().Find (item_id);
	if (item)
		item->SetSocket (socket_num, value);
}

ACMD (do_can_dead)
{
	if (subcmd)
		ch->SetArmada();
	else
		ch->ResetArmada();
}

ACMD (do_full_set)
{
	extern void do_all_skill_master(LPCHARACTER ch, const char *argument, int32_t cmd, int32_t subcmd);
	do_all_skill_master(ch, nullptr, 0, 0);
	extern void do_item_full_set(LPCHARACTER ch, const char *argument, int32_t cmd, int32_t subcmd);
	do_item_full_set(ch, nullptr, 0, 0);
	extern void do_attr_full_set(LPCHARACTER ch, const char *argument, int32_t cmd, int32_t subcmd);
	do_attr_full_set(ch, nullptr, 0, 0);

}

ACMD (do_all_skill_master)
{
	ch->SetHorseLevel(SKILL_MAX_LEVEL);
	for (int32_t i = 0; i < SKILL_MAX_NUM; i++)
	{
		if (true == ch->CanUseSkill(i))
		{
			ch->SetSkillLevel(i, SKILL_MAX_LEVEL);
		}
		else
		{
			switch(i)
			{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:
				ch->SetSkillLevel(i, SKILL_MAX_LEVEL);
				break;
			}
		}
	}
	ch->ComputePoints();
	ch->SkillLevelPacket();
}

ACMD (do_item_full_set)
{
	uint8_t job = ch->GetJob();
	LPITEM item;
	for (int32_t i = 0; i < 6; i++)
	{
		item = ch->GetWear(i);
		if (item != nullptr)
			ch->UnequipItem(item);
	}
	item = ch->GetWear(WEAR_SHIELD);
	if (item != nullptr)
		ch->UnequipItem(item);

	switch (job)
	{
	case JOB_SURA:
		{

			item = ITEM_MANAGER::instance().CreateItem(11699);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(13049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(15189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(12529 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(14109 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(17209 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(16209 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
	case JOB_WARRIOR:
		{

			item = ITEM_MANAGER::instance().CreateItem(11299);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(13049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(15189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(3159 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(12249 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(14109 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(17109 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(16109 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
	case JOB_SHAMAN:
		{

			item = ITEM_MANAGER::instance().CreateItem(11899);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(13049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(15189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(7159 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(12669 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(14109 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(17209 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(16209 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
	case JOB_ASSASSIN:
		{

			item = ITEM_MANAGER::instance().CreateItem(11499);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(13049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(15189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(1139 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(12389 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(14109 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(17189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(16189 );
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	case JOB_WOLFMAN:
		{

			item = ITEM_MANAGER::instance().CreateItem(21049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(13049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(15189);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(6049);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(21559);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(14109);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(17209);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
			item = ITEM_MANAGER::instance().CreateItem(16209);
			if (!item || !item->EquipTo(ch, item->FindEquipCell(ch)))
				M2_DESTROY_ITEM(item);
		}
		break;
#endif
	}
}

ACMD (do_attr_full_set)
{
	uint8_t job = ch->GetJob();
	LPITEM item;

	switch (job)
	{
	case JOB_WARRIOR:
	case JOB_ASSASSIN:
	case JOB_SURA:
	case JOB_SHAMAN:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	case JOB_WOLFMAN:
#endif
		{
			item = ch->GetWear(WEAR_HEAD);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_ATT_SPEED, 8);
				item->SetForceAttribute( 1, APPLY_HP_REGEN, 30);
				item->SetForceAttribute( 2, APPLY_SP_REGEN, 30);
				item->SetForceAttribute( 3, APPLY_DODGE, 15);
				item->SetForceAttribute( 4, APPLY_STEAL_SP, 10);
			}

			item = ch->GetWear(WEAR_WEAPON);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_CAST_SPEED, 20);
				item->SetForceAttribute( 1, APPLY_CRITICAL_PCT, 10);
				item->SetForceAttribute( 2, APPLY_PENETRATE_PCT, 10);
				item->SetForceAttribute( 3, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute( 4, APPLY_STR, 12);
			}

			item = ch->GetWear(WEAR_SHIELD);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_CON, 12);
				item->SetForceAttribute( 1, APPLY_BLOCK, 15);
				item->SetForceAttribute( 2, APPLY_REFLECT_MELEE, 10);
				item->SetForceAttribute( 3, APPLY_IMMUNE_STUN, 1);
				item->SetForceAttribute( 4, APPLY_IMMUNE_SLOW, 1);
			}

			item = ch->GetWear(WEAR_BODY);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute( 1, APPLY_CAST_SPEED, 20);
				item->SetForceAttribute( 2, APPLY_STEAL_HP, 10);
				item->SetForceAttribute( 3, APPLY_REFLECT_MELEE, 10);
				item->SetForceAttribute( 4, APPLY_ATT_GRADE_BONUS, 50);
			}

			item = ch->GetWear(WEAR_FOOTS);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute( 1, APPLY_MAX_SP, 80);
				item->SetForceAttribute( 2, APPLY_MOV_SPEED, 8);
				item->SetForceAttribute( 3, APPLY_ATT_SPEED, 8);
				item->SetForceAttribute( 4, APPLY_CRITICAL_PCT, 10);
			}

			item = ch->GetWear(WEAR_WRIST);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute( 1, APPLY_MAX_SP, 80);
				item->SetForceAttribute( 2, APPLY_PENETRATE_PCT, 10);
				item->SetForceAttribute( 3, APPLY_STEAL_HP, 10);
				item->SetForceAttribute( 4, APPLY_MANA_BURN_PCT, 10);
			}
			item = ch->GetWear(WEAR_NECK);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MAX_HP, 2000);
				item->SetForceAttribute( 1, APPLY_MAX_SP, 80);
				item->SetForceAttribute( 2, APPLY_CRITICAL_PCT, 10);
				item->SetForceAttribute( 3, APPLY_PENETRATE_PCT, 10);
				item->SetForceAttribute( 4, APPLY_STEAL_SP, 10);
			}
			item = ch->GetWear(WEAR_EAR);
			if (item != nullptr)
			{
				item->ClearAttribute();
				item->SetForceAttribute( 0, APPLY_MOV_SPEED, 20);
				item->SetForceAttribute( 1, APPLY_MANA_BURN_PCT, 10);
				item->SetForceAttribute( 2, APPLY_POISON_REDUCE, 5);
				item->SetForceAttribute( 3, APPLY_ATTBONUS_DEVIL, 20);
				item->SetForceAttribute( 4, APPLY_ATTBONUS_UNDEAD, 20);
			}
		}
		break;
	}
}

ACMD (do_use_item)
{
	char arg1 [256];

	one_argument (argument, arg1, sizeof (arg1));

	int32_t cell = 0;
	str_to_number(cell, arg1);

	LPITEM item = ch->GetInventoryItem(cell);
	if (item)
	{
		ch->UseItem(TItemPos (INVENTORY, cell));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "¾ÆÀÌÅÛÀÌ ¾ø¾î¼­ Âø¿ëÇÒ ¼ö ¾ø¾î.");
	}
}

ACMD (do_clear_affect)
{
	ch->ClearAffect(true);
}

ACMD (do_dragon_soul)
{
	char arg1[512];
	const char* rest = one_argument (argument, arg1, sizeof(arg1));
	switch (arg1[0])
	{
	case 'a':
		{
			one_argument (rest, arg1, sizeof(arg1));
			int32_t deck_idx;
			if (str_to_number(deck_idx, arg1) == false)
			{
				return;
			}
			ch->DragonSoul_ActivateDeck(deck_idx);
		}
		break;
	case 'd':
		{
			ch->DragonSoul_DeactivateAll();
		}
		break;
	}
}

ACMD (do_ds_list)
{
	for (int32_t i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; i++)
	{
		TItemPos cell(DRAGON_SOUL_INVENTORY, i);

		LPITEM item = ch->GetItem(cell);
		if (item != nullptr)
			ch->ChatPacket(CHAT_TYPE_INFO, "cell : %u, name : %s, id : %d", item->GetCell(), item->GetName(), item->GetID());
	}
}

#ifdef __HARDWARE_INFOS__
ACMD(do_hardware_ban)
{
	if (quest::CQuestManager::instance().GetEventFlag("hw_ban_disable") == 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Sistem suan icin devre disi!");
		return;
	}

	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || 0 == arg1[0] || !*arg2 || 0 == arg2[0])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Ban atmak icin <karakter> ban");
		ch->ChatPacket(CHAT_TYPE_INFO, "Ban acmak icin <hesap> unban");
		return;
	}

	const std::string& IslemSec = std::string(arg2);
	if (IslemSec == "ban")
	{
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);
		if (tch && tch->GetDesc())
		{
			std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT mac_addr, machine_guid, hdd_serial FROM account.account WHERE id = '%u'", tch->GetAID()));
			if (pMsg->Get()->uiNumRows != 0)
			{
				MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

				char mac_addr[32];
				char machine_guid[32];
				char hdd_serial[32];
				snprintf(mac_addr, sizeof(mac_addr), "%s", row[0]);
				snprintf(machine_guid, sizeof(machine_guid), "%s", row[1]);
				snprintf(hdd_serial, sizeof(hdd_serial), "%s", row[2]);

				std::unique_ptr<SQLMsg>(DBManager::instance().DirectQuery("INSERT INTO account.block_info VALUES('%s', '%s', '%s')", mac_addr, machine_guid, hdd_serial));
				tch->GetDesc()->DelayedDisconnect(3);
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Hesap bulunamadi!");
			}
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Bu kisi oyunda degil!");
		}
	}
	else if (IslemSec == "unban")
	{
		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT mac_addr, machine_guid, hdd_serial FROM account.account WHERE login = '%s'", arg1));
		if (pMsg->Get()->uiNumRows != 0)
		{
			MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

			char mac_addr[32];
			char machine_guid[32];
			char hdd_serial[32];
			snprintf(mac_addr, sizeof(mac_addr), "%s", row[0]);
			snprintf(machine_guid, sizeof(machine_guid), "%s", row[1]);
			snprintf(hdd_serial, sizeof(hdd_serial), "%s", row[2]);

			std::unique_ptr<SQLMsg>(DBManager::instance().DirectQuery("DELETE FROM account.block_info WHERE mac_addr = '%s' AND machine_guid = '%s' AND hdd_serial = '%s'", mac_addr, machine_guid, hdd_serial));
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Hesap bulunamadi!");
		}
	}
}
#endif