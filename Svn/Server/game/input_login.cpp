#include "stdafx.h"
#include "constants.h"
#include "../common/teen_packet.h"
#include "config.h"
#include "utils.h"
#include "input.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "cmd.h"
#include "buffer_manager.h"
#include "protocol.h"
#include "pvp.h"
#include "start_position.h"
#include "messenger_manager.h"
#include "guild_manager.h"
#include "party.h"
#include "dungeon.h"
#include "war_map.h"
#include "questmanager.h"
#include "building.h"
#include "wedding.h"
#include "affect.h"
#include "arena.h"
#include "OXEvent.h"
#include "priv_manager.h"
#include "block_country.h"
#include "dev_log.h"
#include "log.h"
#include "horsename_manager.h"
#include "MarkManager.h"
#include "HackShield.h"
#include "XTrapManager.h"
#ifdef __GUILD_DRAGONLAIR__
#include "meleylair.h"
#endif
#ifdef __BOSS_TRACKING_SYSTEM__
#include "boss_tracking.h"
#endif
#ifdef __MINI_GAME__
#include "minigame_manager.h"
#endif

#ifdef __SWITCHBOT__
#include "new_switchbot.h"
#endif

#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
#ifdef USE_LYCAN_CREATE_POSITION

uint32_t g_lycan_create_position[4][2] =
{
	{		0,		0 },
	{ 768000+38300, 896000+35500 },
	{ 819200+38300, 896000+35500 },
	{ 870400+38300, 896000+35500 },
};

inline uint32_t LYCAN_CREATE_START_X(uint8_t e, uint8_t job)
{
	if (1 <= e && e <= 3)
		return (job==JOB_WOLFMAN)?g_lycan_create_position[e][0]:g_create_position[e][0];
	return 0;
}

inline uint32_t LYCAN_CREATE_START_Y(uint8_t e, uint8_t job)
{
	if (1 <= e && e <= 3)
		return (job==JOB_WOLFMAN)?g_lycan_create_position[e][1]:g_create_position[e][1];
	return 0;
}

#endif


#endif

static void _send_bonus_info(LPCHARACTER ch)
{
	int32_t	item_drop_bonus = 0;
	int32_t gold_drop_bonus = 0;
	int32_t gold10_drop_bonus	= 0;
	int32_t exp_bonus		= 0;

	item_drop_bonus		= CPrivManager::instance().GetPriv(ch, PRIV_ITEM_DROP);
	gold_drop_bonus		= CPrivManager::instance().GetPriv(ch, PRIV_GOLD_DROP);
	gold10_drop_bonus	= CPrivManager::instance().GetPriv(ch, PRIV_GOLD10_DROP);
	exp_bonus			= CPrivManager::instance().GetPriv(ch, PRIV_EXP_PCT);

	if (item_drop_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE,
				LC_TEXT("아이템 드롭률  %d%% 추가 이벤트 중입니다."), item_drop_bonus);
	}
	if (gold_drop_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE,
				LC_TEXT("골드 드롭률 %d%% 추가 이벤트 중입니다."), gold_drop_bonus);
	}
	if (gold10_drop_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE,
				LC_TEXT("대박골드 드롭률 %d%% 추가 이벤트 중입니다."), gold10_drop_bonus);
	}
	if (exp_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE,
				LC_TEXT("경험치 %d%% 추가 획득 이벤트 중입니다."), exp_bonus);
	}
}

static bool FN_is_battle_zone(LPCHARACTER ch)
{
	switch (ch->GetMapIndex())
	{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 28:
			return false;
	}

	return true;
}

void CInputLogin::Login(LPDESC d, const char * data)
{
	TPacketCGLogin * pinfo = (TPacketCGLogin *) data;

	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	sys_log(0, "InputLogin::Login : %s", login);

	TPacketGCLoginFailure failurePacket;

	if (!test_server)
	{
		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "VERSION", sizeof(failurePacket.szStatus));
		d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
		return;
	}

	if (g_bNoMoreClient)
	{
		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));
		d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
		return;
	}

	if (g_iUserLimit > 0)
	{
		int32_t iTotal;
		int32_t * paiEmpireUserCount;
		int32_t iLocal;

		DESC_MANAGER::instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

		if (g_iUserLimit <= iTotal)
		{
			failurePacket.header = HEADER_GC_LOGIN_FAILURE;
			strlcpy(failurePacket.szStatus, "FULL", sizeof(failurePacket.szStatus));
			d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
			return;
		}
	}

	TLoginPacket login_packet;

	strlcpy(login_packet.login, login, sizeof(login_packet.login));
	strlcpy(login_packet.passwd, pinfo->passwd, sizeof(login_packet.passwd));

	db_clientdesc->DBPacket(HEADER_GD_LOGIN, d->GetHandle(), &login_packet, sizeof(TLoginPacket));
}

void CInputLogin::LoginByKey(LPDESC d, const char * data)
{
	TPacketCGLogin2 * pinfo = (TPacketCGLogin2 *) data;

	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	{
		dev_log(LOG_DEB0, "check_blocked_country_start");

		if (!is_block_exception(login) && is_blocked_country_ip(d->GetHostName()))
		{
			sys_log(0, "BLOCK_COUNTRY_IP (%s)", d->GetHostName());
			d->SetPhase(PHASE_CLOSE);
			return;
		}

		dev_log(LOG_DEB0, "check_blocked_country_end");
	}

	if (g_bNoMoreClient)
	{
		TPacketGCLoginFailure failurePacket;

		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));
		d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
		return;
	}

	if (g_iUserLimit > 0)
	{
		int32_t iTotal;
		int32_t * paiEmpireUserCount;
		int32_t iLocal;

		DESC_MANAGER::instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

		if (g_iUserLimit <= iTotal)
		{
			TPacketGCLoginFailure failurePacket;

			failurePacket.header = HEADER_GC_LOGIN_FAILURE;
			strlcpy(failurePacket.szStatus, "FULL", sizeof(failurePacket.szStatus));

			d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
			return;
		}
	}

	sys_log(0, "LOGIN_BY_KEY: %s key %u", login, pinfo->dwLoginKey);

	d->SetLoginKey(pinfo->dwLoginKey);
#ifndef _IMPROVED_PACKET_ENCRYPTION_
	d->SetSecurityKey(pinfo->adwClientKey);
#endif

	TPacketGDLoginByKey ptod;

	strlcpy(ptod.szLogin, login, sizeof(ptod.szLogin));
	ptod.dwLoginKey = pinfo->dwLoginKey;
	memcpy(ptod.adwClientKey, pinfo->adwClientKey, sizeof(uint32_t) * 4);
	strlcpy(ptod.szIP, d->GetHostName(), sizeof(ptod.szIP));

	db_clientdesc->DBPacket(HEADER_GD_LOGIN_BY_KEY, d->GetHandle(), &ptod, sizeof(TPacketGDLoginByKey));
}

void CInputLogin::ChangeName(LPDESC d, const char * data)
{
	TPacketCGChangeName * p = (TPacketCGChangeName *) data;
	const TAccountTable & c_r = d->GetAccountTable();

	if (!c_r.id)
	{
		sys_err("no account table");
		return;
	}

	if (!c_r.players[p->index].bChangeName)
		return;

	if (!check_name(p->name))
	{
		TPacketGCCreateFailure pack;
		pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
		pack.bType = 0;
		d->Packet(&pack, sizeof(pack));
		return;
	}

	TPacketGDChangeName pdb;

	pdb.pid = c_r.players[p->index].dwID;
	strlcpy(pdb.name, p->name, sizeof(pdb.name));
	db_clientdesc->DBPacket(HEADER_GD_CHANGE_NAME, d->GetHandle(), &pdb, sizeof(TPacketGDChangeName));
}

void CInputLogin::CharacterSelect(LPDESC d, const char * data)
{
	struct command_player_select * pinfo = (struct command_player_select *) data;
#ifdef __MULTI_LANGUAGE__
	TAccountTable& c_r = d->GetAccountTable();
#else
	const TAccountTable& c_r = d->GetAccountTable();
#endif

	sys_log(0, "player_select: login: %s index: %d", c_r.login, pinfo->index);

	if (!c_r.id)
	{
		sys_err("no account table");
		return;
	}

	if (pinfo->index >= PLAYER_PER_ACCOUNT)
	{
		sys_err("index overflow %d, login: %s", pinfo->index, c_r.login);
		return;
	}

	if (c_r.players[pinfo->index].bChangeName)
	{
		sys_err("name must be changed idx %d, login %s, name %s",
				pinfo->index, c_r.login, c_r.players[pinfo->index].szName);
		return;
	}
#ifdef __MULTI_LANGUAGE__
	sprintf(c_r.szLanguage, "%s", pinfo->szLanguage);
#endif

	TPlayerLoadPacket player_load_packet;

	player_load_packet.account_id	= c_r.id;
	player_load_packet.player_id	= c_r.players[pinfo->index].dwID;
	player_load_packet.account_index	= pinfo->index;

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_LOAD, d->GetHandle(), &player_load_packet, sizeof(TPlayerLoadPacket));
}

bool NewPlayerTable(TPlayerTable * table,
		const char * name,
		uint8_t job,
		uint8_t shape,
		uint8_t bEmpire,
		uint8_t bCon,
		uint8_t bInt,
		uint8_t bStr,
		uint8_t bDex)
{
	if (job >= JOB_MAX_NUM)
		return false;

	memset(table, 0, sizeof(TPlayerTable));

	strlcpy(table->name, name, sizeof(table->name));

	table->level = g_dwStartLevel;
	table->job = job;
	table->voice = 0;
	table->part_base = shape;

	table->st = JobInitialPoints[job].st;
	table->dx = JobInitialPoints[job].dx;
	table->ht = JobInitialPoints[job].ht;
	table->iq = JobInitialPoints[job].iq;

	table->hp = JobInitialPoints[job].max_hp + table->ht * JobInitialPoints[job].hp_per_ht;
	table->sp = JobInitialPoints[job].max_sp + table->iq * JobInitialPoints[job].sp_per_iq;
	table->stamina = JobInitialPoints[job].max_stamina;

#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_LYCAN_CREATE_POSITION) // @correction190
	table->x 	= LYCAN_CREATE_START_X(bEmpire, job) + number(-300, 300);
	table->y 	= LYCAN_CREATE_START_Y(bEmpire, job) + number(-300, 300);
#else
	table->x 	= CREATE_START_X(bEmpire) + number(-300, 300);
	table->y 	= CREATE_START_Y(bEmpire) + number(-300, 300);
#endif
	table->z	= 0;
	table->dir	= 0;
	table->playtime = 0;
	table->gold 	= 0;
	table->skill_group = 0;
#ifdef __12ZI__
	table->bBead = 0;
#endif

	if (china_event_server)
	{
		table->level = 35;

		for (int32_t i = 1; i < 35; ++i)
		{
			int32_t iHP = number(JobInitialPoints[job].hp_per_lv_begin, JobInitialPoints[job].hp_per_lv_end);
			int32_t iSP = number(JobInitialPoints[job].sp_per_lv_begin, JobInitialPoints[job].sp_per_lv_end);
			table->sRandomHP += iHP;
			table->sRandomSP += iSP;
			table->stat_point += 3;
		}

		table->hp += table->sRandomHP;
		table->sp += table->sRandomSP;

		table->gold = 1000000;
	}

	return true;
}

bool RaceToJob(uint32_t race, uint32_t* ret_job)
{
	*ret_job = 0;

	if (race >= MAIN_RACE_MAX_NUM)
		return false;

	switch (race)
	{
		case MAIN_RACE_WARRIOR_M:
			*ret_job = JOB_WARRIOR;
			break;

		case MAIN_RACE_WARRIOR_W:
			*ret_job = JOB_WARRIOR;
			break;

		case MAIN_RACE_ASSASSIN_M:
			*ret_job = JOB_ASSASSIN;
			break;

		case MAIN_RACE_ASSASSIN_W:
			*ret_job = JOB_ASSASSIN;
			break;

		case MAIN_RACE_SURA_M:
			*ret_job = JOB_SURA;
			break;

		case MAIN_RACE_SURA_W:
			*ret_job = JOB_SURA;
			break;

		case MAIN_RACE_SHAMAN_M:
			*ret_job = JOB_SHAMAN;
			break;

		case MAIN_RACE_SHAMAN_W:
			*ret_job = JOB_SHAMAN;
			break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case MAIN_RACE_WOLFMAN_M:
			*ret_job = JOB_WOLFMAN;
			break;
#endif
		default:
			return false;
			break;
	}
	return true;
}

bool NewPlayerTable2(TPlayerTable * table, const char * name, uint8_t race, uint8_t shape, uint8_t bEmpire)
{
	if (race >= MAIN_RACE_MAX_NUM)
	{
		sys_err("NewPlayerTable2.OUT_OF_RACE_RANGE(%d >= max(%d))\n", race, MAIN_RACE_MAX_NUM);
		return false;
	}

	uint32_t job;

	if (!RaceToJob(race, &job))
	{
		sys_err("NewPlayerTable2.RACE_TO_JOB_ERROR(%d)\n", race);
		return false;
	}

	sys_log(0, "NewPlayerTable2(name=%s, race=%d, job=%d)", name, race, job);

	memset(table, 0, sizeof(TPlayerTable));

	strlcpy(table->name, name, sizeof(table->name));

	table->level		= g_dwStartLevel;
	table->job			= race;
	table->voice		= 0;
	table->part_base	= shape;

	table->st		= JobInitialPoints[job].st;
	table->dx		= JobInitialPoints[job].dx;
	table->ht		= JobInitialPoints[job].ht;
	table->iq		= JobInitialPoints[job].iq;

	table->hp		= JobInitialPoints[job].max_hp + table->ht * JobInitialPoints[job].hp_per_ht;
	table->sp		= JobInitialPoints[job].max_sp + table->iq * JobInitialPoints[job].sp_per_iq;
	table->stamina	= JobInitialPoints[job].max_stamina;

#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_LYCAN_CREATE_POSITION) // @correction190
	table->x 		= LYCAN_CREATE_START_X(bEmpire, job) + number(-300, 300);
	table->y 		= LYCAN_CREATE_START_Y(bEmpire, job) + number(-300, 300);
#else
	table->x		= CREATE_START_X(bEmpire) + number(-300, 300);
	table->y		= CREATE_START_Y(bEmpire) + number(-300, 300);
#endif
	table->z		= 0;
	table->dir		= 0;
	table->playtime = 0;
	table->gold 	= 0;

#ifdef __12ZI__
	table->bBead = 0;
#endif
	table->skill_group = 0;

	return true;
}

void CInputLogin::CharacterCreate(LPDESC d, const char * data)
{
	struct command_player_create * pinfo = (struct command_player_create *) data;
	TPlayerCreatePacket player_create_packet;

	sys_log(0, "PlayerCreate: name %s pos %d job %d shape %d",
			pinfo->name,
			pinfo->index,
			pinfo->job,
			pinfo->shape);

	TPacketGCLoginFailure packFailure;
	memset(&packFailure, 0, sizeof(packFailure));
	packFailure.header = HEADER_GC_CHARACTER_CREATE_FAILURE;

	if (true == g_BlockCharCreation)
	{
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	// @correction091 BEGIN
	if (strlen(pinfo->name) > 12)
	{
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}
	// @correction091 END

	if (!check_name(pinfo->name) || pinfo->shape > 1)
	{
		// @correction005
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	// @correction005
	const TAccountTable & c_rAccountTable = d->GetAccountTable();

	if (0 == strcmp(c_rAccountTable.login, pinfo->name))
	{
		TPacketGCCreateFailure pack;
		pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
		pack.bType = 1;

		d->Packet(&pack, sizeof(pack));
		return;
	}

	memset(&player_create_packet, 0, sizeof(TPlayerCreatePacket));

	if (!NewPlayerTable2(&player_create_packet.player_table, pinfo->name, pinfo->job, pinfo->shape, d->GetEmpire()))
	{
		sys_err("player_prototype error: job %d face %d ", pinfo->job);
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	trim_and_lower(c_rAccountTable.login, player_create_packet.login, sizeof(player_create_packet.login));
	strlcpy(player_create_packet.passwd, c_rAccountTable.passwd, sizeof(player_create_packet.passwd));

	player_create_packet.account_id	= c_rAccountTable.id;
	player_create_packet.account_index	= pinfo->index;

	sys_log(0, "PlayerCreate: name %s account_id %d, TPlayerCreatePacketSize(%d), Packet->Gold %d",
			pinfo->name,
			pinfo->index,
			sizeof(TPlayerCreatePacket),
			player_create_packet.player_table.gold);

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_CREATE, d->GetHandle(), &player_create_packet, sizeof(TPlayerCreatePacket));
}

void CInputLogin::CharacterDelete(LPDESC d, const char * data)
{
	struct command_player_delete * pinfo = (struct command_player_delete *) data;
	const TAccountTable & c_rAccountTable = d->GetAccountTable();

	if (!c_rAccountTable.id)
	{
		sys_err("PlayerDelete: no login data");
		return;
	}

	sys_log(0, "PlayerDelete: login: %s index: %d, social_id %s", c_rAccountTable.login, pinfo->index, pinfo->private_code);

	if (pinfo->index >= PLAYER_PER_ACCOUNT)
	{
		sys_err("PlayerDelete: index overflow %d, login: %s", pinfo->index, c_rAccountTable.login);
		return;
	}

	if (!c_rAccountTable.players[pinfo->index].dwID)
	{
		sys_err("PlayerDelete: Wrong Social ID index %d, login: %s", pinfo->index, c_rAccountTable.login);
		d->Packet(encode_byte(HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID), 1);
		return;
	}

	TPlayerDeletePacket	player_delete_packet;

	trim_and_lower(c_rAccountTable.login, player_delete_packet.login, sizeof(player_delete_packet.login));
	player_delete_packet.player_id	= c_rAccountTable.players[pinfo->index].dwID;
	player_delete_packet.account_index	= pinfo->index;
	strlcpy(player_delete_packet.private_code, pinfo->private_code, sizeof(player_delete_packet.private_code));

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_DELETE, d->GetHandle(), &player_delete_packet, sizeof(TPlayerDeletePacket));
}

#pragma pack(1)
typedef struct SPacketGTLogin
{
	uint8_t header;
	uint16_t empty;
	uint32_t id;
} TPacketGTLogin;
#pragma pack()

void CInputLogin::Entergame(LPDESC d, const char * data)
{
	LPCHARACTER ch;

	if (!(ch = d->GetCharacter()))
	{
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	PIXEL_POSITION pos = ch->GetXYZ();

	if (!SECTREE_MANAGER::instance().GetMovablePosition(ch->GetMapIndex(), pos.x, pos.y, pos))
	{
		PIXEL_POSITION pos2;
		SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos2);

		sys_err("!GetMovablePosition (name %s %dx%d map %d changed to %dx%d)",
				ch->GetName(),
				pos.x, pos.y,
				ch->GetMapIndex(),
				pos2.x, pos2.y);
		pos = pos2;
	}

	CGuildManager::instance().LoginMember(ch);

	ch->Show(ch->GetMapIndex(), pos.x, pos.y, pos.z);

	SECTREE_MANAGER::instance().SendNPCPosition(ch);
	ch->CheckWears();
	ch->ReviveInvisible(5);

	d->SetPhase(PHASE_GAME);

	if(ch->GetItemAward_cmd())
		quest::CQuestManager::instance().ItemInformer(ch->GetPlayerID(),ch->GetItemAward_vnum());

	sys_log(0, "ENTERGAME: %s %dx%dx%d %s map_index %d",
			ch->GetName(), ch->GetX(), ch->GetY(), ch->GetZ(), d->GetHostName(), ch->GetMapIndex());

	if (ch->GetHorseLevel() > 0)
	{
		ch->EnterHorse();
	}

	ch->ResetPlayTime();
	ch->SendGold(true);
	ch->StartSaveEvent();
	ch->StartRecoveryEvent();
	ch->StartCheckSpeedHackEvent();

#ifdef __BOSS_TRACKING_SYSTEM__
	ch->StartBossTrackingDataEvent();
#endif

	CPVPManager::instance().Connect(ch);
	CPVPManager::instance().SendList(d);

	MessengerManager::instance().Login(ch->GetName());

	CPartyManager::instance().SetParty(ch);
	CGuildManager::instance().SendGuildWar(ch);

	building::CManager::instance().SendLandList(d, ch->GetMapIndex());

	marriage::CManager::instance().Login(ch);

#ifdef __MINI_GAME__
	CMiniGameManager::instance().SetCharecterEvents(ch);
#endif

	TPacketGCTime p;
	p.bHeader = HEADER_GC_TIME;
	p.time = get_global_time();
	d->Packet(&p, sizeof(p));

	TPacketGCChannel p2;
	p2.header = HEADER_GC_CHANNEL;
	p2.channel = g_bChannel;
	d->Packet(&p2, sizeof(p2));

#ifdef __12ZI__
	for (int32_t i = 0; i < 24; ++i)
	{
		std::string s("zodiac.Insignia");
		s += std::to_string(i + 1);
		ch->SetQuestFlag(s.c_str(), 0);
	}
	ch->SetQuestFlag("zodiac.PrismOfRevival", 0);

	if (ch->GetBead() >= 36)
		ch->SetBeadTime(0, true);
	else
	{
		uint32_t dwTime = time(nullptr) - ch->GetLastPlay();

		if (!ch->GetBead() && !ch->GetBeadTime())
			ch->SetBeadTime(3600, true);
		else if (ch->GetBeadTime() && dwTime < ch->GetBeadTime())
			ch->SetBeadTime(ch->GetBeadTime() - dwTime, true);
		else if (dwTime >= 129600 || (dwTime >= 3600 && ch->GetBead() + (dwTime / 3600) >= 36))
		{
			ch->SetBeadTime(0, true);
			ch->SetBead(36, true);
		}
		else if (dwTime >= 3600)
		{
			ch->SetBead(ch->GetBead() + dwTime / 3600, true);
			ch->SetBeadTime(dwTime % 3600, true);
		}
		else
			ch->SetBeadTime(dwTime, true);

		if (ch->GetBeadTime())
			ch->StartBeadEvent();
	}

	ch->ChatPacket(CHAT_TYPE_COMMAND, "NextBeadUpdateTime %d", ch->GetBeadTime());
	ch->ChatPacket(CHAT_TYPE_COMMAND, "SetBeadCount %d", ch->GetBead());
	//ch->ChatPacket(CHAT_TYPE_INFO, "IN: %d %d %d %d", ch->GetBeadTime(), ch->GetBead(), ch->GetLastPlay(), time(nullptr) - ch->GetLastPlay());
#endif


#ifdef ENABLE_DISCORD_RPC // @correction201
	TGuildDiscordInfo p3{};
	p3.header = HEADER_GC_GUILD_DCINFO;

	auto pGuild = ch->GetGuild();
    if (pGuild)
    {
        p3.dwMemberCount = pGuild->GetMemberCount();
        p3.dwMaxMemberCount = pGuild->GetMaxMemberCount();
        strcpy(p3.szName, pGuild->GetName());
    }
    d->Packet(&p3, sizeof(p3));
#endif

#ifdef __SWITCHBOT__
	CSwitchbotManager::Instance().EnterGame(ch);
#endif

#ifdef __AFAR_BIOLOG__
	int32_t biodurum = ch->GetQuestFlag("bio.durum");
	if (biodurum == 0 && ch->GetLevel() >= 30)
	{
		ch->SetQuestFlag("bio.durum", 1);
		ch->SetQuestFlag("bio.verilen", 0);
		ch->SetQuestFlag("bio.ruhtasi", 0);
		ch->SetQuestFlag("bio.kalan", get_global_time());

		int32_t bioverilen = ch->GetQuestFlag("bio.verilen");
		int32_t biokalan = ch->GetQuestFlag("bio.kalan");
		int32_t bioruhtasi = ch->GetQuestFlag("bio.ruhtasi");
		biodurum = ch->GetQuestFlag("bio.durum");
		ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
	}
	else
	{
		int32_t bioverilen = ch->GetQuestFlag("bio.verilen");
		int32_t biokalan = ch->GetQuestFlag("bio.kalan");
		int32_t bioruhtasi = ch->GetQuestFlag("bio.ruhtasi");

		if (biodurum == 31 || biodurum == 41 || biodurum == 51 || biodurum == 61 || biodurum == 71 || biodurum == 81 || biodurum == 91 || biodurum == 93 || biodurum == 11)
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
		}
		else if (bioverilen >= BiyologSistemi[biodurum][1] && bioruhtasi == 1)
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 1 0", BiyologSistemi[biodurum][4]);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
		}
	}
#endif

	ch->SendGreetMessage();

	_send_bonus_info(ch);

	for (int32_t i = 0; i <= PREMIUM_MAX_NUM; ++i)
	{
		int32_t remain = ch->GetPremiumRemainSeconds(i);

		if (remain <= 0)
			continue;

		ch->AddAffect(AFFECT_PREMIUM_START + i, POINT_NONE, 0, 0, remain, 0, true);
		sys_log(0, "PREMIUM: %s type %d %dmin", ch->GetName(), i, remain);
	}

	// @correction005
	if (g_bCheckClientVersion)
	{
		sys_log(0, "VERSION CHECK %s %s", g_stClientVersion.c_str(), d->GetClientVersion());

		if (!d->GetClientVersion())
		{
			d->DelayedDisconnect(10);
		}
		else
		{
			if (0 != g_stClientVersion.compare(d->GetClientVersion())) // @correction030 (version > date)
			{
				ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("클라이언트 버전이 틀려 로그아웃 됩니다. 정상적으로 패치 후 접속하세요."));
				d->DelayedDisconnect(0); // @correction030 (10);
				LogManager::instance().HackLog("VERSION_CONFLICT", ch);

				sys_log(0, "VERSION : WRONG VERSION USER : account:%s name:%s hostName:%s server_version:%s client_version:%s",
						d->GetAccountTable().login,
						ch->GetName(),
						d->GetHostName(),
						g_stClientVersion.c_str(),
						d->GetClientVersion());
			}
		}
	}
	else
	{
		sys_log(0, "VERSION : NO CHECK");
	}

	// @correction005
	if (ch->IsGM() == true)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");

#ifdef __BLOCK_EXP__
	if (ch)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "ExpBlock %d", ch->GetQuestFlag("sec.exp"));
#endif

	if (ch->GetMapIndex() >= 10000)
	{
		if (CWarMapManager::instance().IsWarMap(ch->GetMapIndex()))
			ch->SetWarMap(CWarMapManager::instance().Find(ch->GetMapIndex()));
		else if (marriage::WeddingManager::instance().IsWeddingMap(ch->GetMapIndex()))
			ch->SetWeddingMap(marriage::WeddingManager::instance().Find(ch->GetMapIndex()));
#ifdef __GUILD_DRAGONLAIR__
		else if (CMgr::instance().IsMeleyMap(ch->GetMapIndex()))
			// CMgr::instance().Leave(ch, true);
			CMgr::instance().DungeonSet(ch);
#endif
		else
			ch->SetDungeon(CDungeonManager::instance().FindByMapIndex(ch->GetMapIndex()));
	}
	else if (CArenaManager::instance().IsArenaMap(ch->GetMapIndex()) == true)
	{
		int32_t memberFlag = CArenaManager::instance().IsMember(ch->GetMapIndex(), ch->GetPlayerID());
		if (memberFlag == MEMBER_OBSERVER)
		{
			ch->SetObserverMode(true);
			ch->SetArenaObserverMode(true);
			if (CArenaManager::instance().RegisterObserverPtr(ch, ch->GetMapIndex(), ch->GetX()/100, ch->GetY()/100))
			{
				sys_log(0, "ARENA : Observer add failed");
			}

			if (ch->IsHorseRiding() == true)
			{
				ch->StopRiding();
				ch->HorseSummon(false);
			}
		}
		else if (memberFlag == MEMBER_DUELIST)
		{
			TPacketGCDuelStart duelStart;
			duelStart.header = HEADER_GC_DUEL_START;
			duelStart.wSize = sizeof(TPacketGCDuelStart);

			ch->GetDesc()->Packet(&duelStart, sizeof(TPacketGCDuelStart));

			if (ch->IsHorseRiding() == true)
			{
				ch->StopRiding();
				ch->HorseSummon(false);
			}

			LPPARTY pParty = ch->GetParty();
			if (pParty != nullptr)
			{
				if (pParty->GetMemberCount() == 2)
				{
					CPartyManager::instance().DeleteParty(pParty);
				}
				else
				{
					pParty->Quit(ch->GetPlayerID());
				}
			}
		}
		else if (memberFlag == MEMBER_NO)
		{
			if (ch->GetGMLevel() == GM_PLAYER)
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}
		else
		{
		}
	}
	else if (ch->GetMapIndex() == 28)
	{
		if (COXEventManager::instance().Enter(ch) == false)
		{
			if (ch->GetGMLevel() == GM_PLAYER)
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}
	}
	else
	{
		if (CWarMapManager::instance().IsWarMap(ch->GetMapIndex()) ||
				marriage::WeddingManager::instance().IsWeddingMap(ch->GetMapIndex()))
		{
			if (!test_server)
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}
	}

	if (g_TeenDesc)
	{
		TPacketGTLogin p;

		p.header = HEADER_GT_LOGIN;
		p.empty = 0;
		p.id = d->GetAccountTable().id;

		g_TeenDesc->Packet(&p, sizeof(p));
		sys_log(0, "TEEN_SEND: (%u, %s)", d->GetAccountTable().id, ch->GetName());
	}

	if (ch->GetHorseLevel() > 0)
	{
		uint32_t pid = ch->GetPlayerID();

		if (pid != 0 && CHorseNameManager::instance().GetHorseName(pid) == nullptr)
			db_clientdesc->DBPacket(HEADER_GD_REQ_HORSE_NAME, 0, &pid, sizeof(uint32_t));

		// @correction025 BEGIN
		ch->SetHorseLevel(ch->GetHorseLevel());
		ch->SkillLevelPacket();
		// @correction025 END
	}

	if (g_noticeBattleZone)
	{
		if (FN_is_battle_zone(ch))
		{
			ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("이 맵에선 강제적인 대전이 있을수 도 있습니다."));
			ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("이 조항에 동의하지 않을시"));
			ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("본인의 주성 및 부성으로 돌아가시기 바랍니다."));
		}
	}
}

void CInputLogin::Empire(LPDESC d, const char * c_pData)
{
	const TPacketCGEmpire* p = reinterpret_cast<const TPacketCGEmpire*>(c_pData);

	if (EMPIRE_MAX_NUM <= p->bEmpire)
	{
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	const TAccountTable& r = d->GetAccountTable();

	if (r.bEmpire != 0)
	{
		for (int32_t i = 0; i < PLAYER_PER_ACCOUNT; ++i)
		{
			if (0 != r.players[i].dwID)
			{
				sys_err("EmpireSelectFailed %d", r.players[i].dwID);
				return;
			}
		}
	}

	TEmpireSelectPacket pd;

	pd.dwAccountID = r.id;
	pd.bEmpire = p->bEmpire;

	db_clientdesc->DBPacket(HEADER_GD_EMPIRE_SELECT, d->GetHandle(), &pd, sizeof(pd));
}

int32_t CInputLogin::GuildSymbolUpload(LPDESC d, const char* c_pData, size_t uiBytes)
{
	if (uiBytes < sizeof(TPacketCGGuildSymbolUpload))
		return -1;

	sys_log(0, "GuildSymbolUpload uiBytes %u", uiBytes);

	TPacketCGGuildSymbolUpload* p = (TPacketCGGuildSymbolUpload*) c_pData;

	if (uiBytes < p->size)
		return -1;

	int32_t iSymbolSize = p->size - sizeof(TPacketCGGuildSymbolUpload);

	if (iSymbolSize <= 0 || iSymbolSize > 64 * 1024)
	{
		d->SetPhase(PHASE_CLOSE);
		return 0;
	}

	if (!test_server)
		if (!building::CManager::instance().FindLandByGuild(p->guild_id))
		{
			d->SetPhase(PHASE_CLOSE);
			return 0;
		}

	sys_log(0, "GuildSymbolUpload Do Upload %02X%02X%02X%02X %d", c_pData[7], c_pData[8], c_pData[9], c_pData[10], sizeof(*p));

	CGuildMarkManager::instance().UploadSymbol(p->guild_id, iSymbolSize, (const uint8_t*)(c_pData + sizeof(*p)));
	CGuildMarkManager::instance().SaveSymbol(GUILD_SYMBOL_FILENAME);
	return iSymbolSize;
}

void CInputLogin::GuildSymbolCRC(LPDESC d, const char* c_pData)
{
	const TPacketCGSymbolCRC & CGPacket = *((TPacketCGSymbolCRC *) c_pData);

	sys_log(0, "GuildSymbolCRC %u %u %u", CGPacket.guild_id, CGPacket.crc, CGPacket.size);

	const CGuildMarkManager::TGuildSymbol * pkGS = CGuildMarkManager::instance().GetGuildSymbol(CGPacket.guild_id);

	if (!pkGS)
		return;

	sys_log(0, "  Server %u %u", pkGS->crc, pkGS->raw.size());

	if (pkGS->raw.size() != CGPacket.size || pkGS->crc != CGPacket.crc)
	{
		TPacketGCGuildSymbolData GCPacket;

		GCPacket.header = HEADER_GC_SYMBOL_DATA;
		GCPacket.size = sizeof(GCPacket) + pkGS->raw.size();
		GCPacket.guild_id = CGPacket.guild_id;

		d->BufferedPacket(&GCPacket, sizeof(GCPacket));
		d->Packet(&pkGS->raw[0], pkGS->raw.size());

		sys_log(0, "SendGuildSymbolHead %02X%02X%02X%02X Size %d",
				pkGS->raw[0], pkGS->raw[1], pkGS->raw[2], pkGS->raw[3], pkGS->raw.size());
	}
}

void CInputLogin::GuildMarkUpload(LPDESC d, const char* c_pData)
{
	TPacketCGMarkUpload * p = (TPacketCGMarkUpload *) c_pData;
	CGuildManager& rkGuildMgr = CGuildManager::instance();
	CGuild * pkGuild;

	if (!(pkGuild = rkGuildMgr.FindGuild(p->gid)))
	{
		sys_err("MARK_SERVER: GuildMarkUpload: no guild. gid %u", p->gid);
		return;
	}

	if (pkGuild->GetLevel() < guild_mark_min_level)
	{
		sys_log(0, "MARK_SERVER: GuildMarkUpload: level < %u (%u)", guild_mark_min_level, pkGuild->GetLevel());
		return;
	}

	CGuildMarkManager & rkMarkMgr = CGuildMarkManager::instance();

	sys_log(0, "MARK_SERVER: GuildMarkUpload: gid %u", p->gid);

	bool isEmpty = true;

	for (uint32_t iPixel = 0; iPixel < SGuildMark::SIZE; ++iPixel)
		if (*((uint32_t *) p->image + iPixel) != 0x00000000)
			isEmpty = false;

	if (isEmpty)
		rkMarkMgr.DeleteMark(p->gid);
	else
		rkMarkMgr.SaveMark(p->gid, p->image);
}

void CInputLogin::GuildMarkIDXList(LPDESC d, const char* c_pData)
{
	CGuildMarkManager & rkMarkMgr = CGuildMarkManager::instance();

	uint32_t bufSize = sizeof(uint16_t) * 2 * rkMarkMgr.GetMarkCount();
	char * buf = nullptr;

	if (bufSize > 0)
	{
		buf = (char *) malloc(bufSize);
		rkMarkMgr.CopyMarkIdx(buf);
	}

	TPacketGCMarkIDXList p;
	p.header = HEADER_GC_MARK_IDXLIST;
	p.bufSize = sizeof(p) + bufSize;
	p.count = rkMarkMgr.GetMarkCount();

	if (buf)
	{
		d->BufferedPacket(&p, sizeof(p));
		d->LargePacket(buf, bufSize);
		free(buf);
	}
	else
		d->Packet(&p, sizeof(p));

	sys_log(0, "MARK_SERVER: GuildMarkIDXList %d bytes sent.", p.bufSize);
}

void CInputLogin::GuildMarkCRCList(LPDESC d, const char* c_pData)
{
	TPacketCGMarkCRCList * pCG = (TPacketCGMarkCRCList *) c_pData;

	std::map<uint8_t, const SGuildMarkBlock *> mapDiffBlocks;
	CGuildMarkManager::instance().GetDiffBlocks(pCG->imgIdx, pCG->crclist, mapDiffBlocks);

	uint32_t blockCount = 0;
	TEMP_BUFFER buf(1024 * 1024);

	for (itertype(mapDiffBlocks) it = mapDiffBlocks.begin(); it != mapDiffBlocks.end(); ++it)
	{
		uint8_t posBlock = it->first;
		const SGuildMarkBlock & rkBlock = *it->second;

		buf.write(&posBlock, sizeof(uint8_t));
		buf.write(&rkBlock.m_sizeCompBuf, sizeof(uint32_t));
		buf.write(rkBlock.m_abCompBuf, rkBlock.m_sizeCompBuf);

		++blockCount;
	}

	TPacketGCMarkBlock pGC;

	pGC.header = HEADER_GC_MARK_BLOCK;
	pGC.imgIdx = pCG->imgIdx;
	pGC.bufSize = buf.size() + sizeof(TPacketGCMarkBlock);
	pGC.count = blockCount;

	sys_log(0, "MARK_SERVER: Sending blocks. (imgIdx %u diff %u size %u)", pCG->imgIdx, mapDiffBlocks.size(), pGC.bufSize);

	if (buf.size() > 0)
	{
		d->BufferedPacket(&pGC, sizeof(TPacketGCMarkBlock));
		d->LargePacket(buf.read_peek(), buf.size());
	}
	else
		d->Packet(&pGC, sizeof(TPacketGCMarkBlock));
}

int32_t CInputLogin::Analyze(LPDESC d, uint8_t bHeader, const char * c_pData)
{
	int32_t iExtraLen = 0;

	switch (bHeader)
	{
		case HEADER_CG_PONG:
			Pong(d);
			break;

		case HEADER_CG_TIME_SYNC:
			Handshake(d, c_pData);
			break;

		case HEADER_CG_LOGIN:
			Login(d, c_pData);
			break;

		case HEADER_CG_LOGIN2:
			LoginByKey(d, c_pData);
			break;

		case HEADER_CG_CHARACTER_SELECT:
			CharacterSelect(d, c_pData);
			break;

		case HEADER_CG_CHARACTER_CREATE:
			CharacterCreate(d, c_pData);
			break;

		case HEADER_CG_CHARACTER_DELETE:
			CharacterDelete(d, c_pData);
			break;

		case HEADER_CG_ENTERGAME:
			Entergame(d, c_pData);
			break;

		case HEADER_CG_EMPIRE:
			Empire(d, c_pData);
			break;

		case HEADER_CG_MOVE:
			break;

		case HEADER_CG_MARK_CRCLIST:
			GuildMarkCRCList(d, c_pData);
			break;

		case HEADER_CG_MARK_IDXLIST:
			GuildMarkIDXList(d, c_pData);
			break;

		case HEADER_CG_MARK_UPLOAD:
			GuildMarkUpload(d, c_pData);
			break;

		case HEADER_CG_GUILD_SYMBOL_UPLOAD:
			if ((iExtraLen = GuildSymbolUpload(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_SYMBOL_CRC:
			GuildSymbolCRC(d, c_pData);
			break;

		case HEADER_CG_HACK:
			break;

		case HEADER_CG_CHANGE_NAME:
			ChangeName(d, c_pData);
			break;

		case HEADER_CG_CLIENT_VERSION:
			Version(d->GetCharacter(), c_pData);
			break;

		case HEADER_CG_CLIENT_VERSION2:
			Version(d->GetCharacter(), c_pData);
			break;

		case HEADER_CG_HS_ACK:
			if (isHackShieldEnable)
			{
				CHackShieldManager::instance().VerifyAck(d->GetCharacter(), c_pData);
			}
			break;

		// @correction043
		case HEADER_CG_ITEM_USE:
		case HEADER_CG_TARGET:
			break;

		case HEADER_CG_XTRAP_ACK:
			{
				TPacketXTrapCSVerify* p = reinterpret_cast<TPacketXTrapCSVerify*>((void*)c_pData);
				CXTrapManager::instance().Verify_CSStep3(d->GetCharacter(), p->bPacketData);
			}
			break;

		case HEADER_CG_CHAT:
			{
				const TPacketCGChat* pinfo = reinterpret_cast<const TPacketCGChat*>(c_pData);
				const int32_t iExtraLen = pinfo->size - sizeof(TPacketCGChat);
				char buf[CHAT_MAX_LEN - (CHARACTER_NAME_MAX_LEN + 3) + 1];
				strlcpy(buf, c_pData + sizeof(TPacketCGChat), MIN(iExtraLen + 1, sizeof(buf)));
				sys_err("ChatGeldi: %s", buf);
			}
			break;

		default:
			sys_err("login phase does not handle this packet! header %d", bHeader);
			return (0);
	}

	return (iExtraLen);
}

