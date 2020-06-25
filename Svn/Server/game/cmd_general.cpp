#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "motion.h"
#include "packet.h"
#include "affect.h"
#include "pvp.h"
#include "start_position.h"
#include "party.h"
#include "guild_manager.h"
#include "p2p.h"
#include "dungeon.h"
#include "messenger_manager.h"
#include "war_map.h"
#include "questmanager.h"
#include "item_manager.h"
#include "monarch.h"
#include "mob_manager.h"
#include "dev_log.h"
#include "item.h"
#include "arena.h"
#include "buffer_manager.h"
#include "unique_item.h"
#include "threeway_war.h"
#include "log.h"
#include "../common/VnumHelper.h"
#ifdef __AUCTION__
#include "auction_manager.h"
#endif
#ifdef __OFFLINE_PRIVATE_SHOP__
#include "shop_manager.h"
#include "offline_privateshop.h"
#endif
#ifdef __MOUNT__
#include "mountsystem.h"
#endif

#ifdef __SUPPORT__
#include "supportsystem.h"
#endif

#ifdef __GROWTH_PET__
#include "new_petsystem.h"
#endif
#ifdef __BOSS_TRACKING_SYSTEM__
#include "boss_tracking.h"
#endif

#ifdef __SORT_INVENTORY__
#include "safebox.h"
#endif
#include <md5.h>

ACMD(do_user_horse_ride)
{
	if (ch->IsObserverMode())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->IsHorseRiding() == false)
	{
		if (ch->GetMountVnum())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì Å»°ÍÀ» ÀÌ¿ëÁßÀÔ´Ï´Ù."));
			return;
		}

		if (ch->GetHorse() == nullptr)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
			return;
		}

		ch->StartRiding();
	}
	else
	{
		ch->StopRiding();
	}
}

ACMD(do_user_horse_back)
{
	if (ch->GetHorse() != nullptr)
	{
		ch->HorseSummon(false);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» µ¹·Áº¸³Â½À´Ï´Ù."));
	}
	else if (ch->IsHorseRiding() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»¿¡¼­ ¸ÕÀú ³»·Á¾ß ÇÕ´Ï´Ù."));
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
	}
}

ACMD(do_user_horse_feed)
{
	if (ch->GetMyShop())
		return;

	if (ch->GetHorse() == nullptr)
	{
		if (ch->IsHorseRiding() == false)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
		else
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» Åº »óÅÂ¿¡¼­´Â ¸ÔÀÌ¸¦ ÁÙ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	uint32_t dwFood = ch->GetHorseGrade() + 50054 - 1;

	if (ch->CountSpecifyItem(dwFood) > 0)
	{
		ch->RemoveSpecifyItem(dwFood, 1);
		ch->FeedHorse();
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»¿¡°Ô %s%s ÁÖ¾ú½À´Ï´Ù."),
				ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName,
				"");
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ¾ÆÀÌÅÛÀÌ ÇÊ¿äÇÕ´Ï´Ù"), ITEM_MANAGER::instance().GetTable(dwFood)->szLocaleName);
	}
}

#define MAX_REASON_LEN		128

EVENTINFO(TimedEventInfo)
{
	DynamicCharacterPtr ch;
	int32_t		subcmd;
	int32_t         	left_second;
	char		szReason[MAX_REASON_LEN];

	TimedEventInfo()
	: ch()
	, subcmd( 0 )
	, left_second( 0 )
	{
		::memset( szReason, 0, MAX_REASON_LEN );
	}
};

struct SendDisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetCharacter())
		{
			if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
				d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit Shutdown(SendDisconnectFunc)");
		}
	}
};

struct DisconnectFunc
{
	void operator () (LPDESC d)
	{
		if (d->GetType() == DESC_TYPE_CONNECTOR)
			return;

		if (d->IsPhase(PHASE_P2P))
			return;

		if (d->GetCharacter())
			d->GetCharacter()->Disconnect("Shutdown(DisconnectFunc)");

		d->SetPhase(PHASE_CLOSE);
	}
};

EVENTINFO(shutdown_event_data)
{
	int32_t seconds;

	shutdown_event_data()
	: seconds( 0 )
	{
	}
};

EVENTFUNC(shutdown_event)
{
	shutdown_event_data* info = dynamic_cast<shutdown_event_data*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "shutdown_event> <Factor> Null pointer" );
		return 0;
	}

	int32_t * pSec = & (info->seconds);

	if (*pSec < 0)
	{
		sys_log(0, "shutdown_event sec %d", *pSec);

		if (--*pSec == -10)
		{
			const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), DisconnectFunc());
			return passes_per_sec;
		}
		else if (*pSec < -10)
			return 0;

		return passes_per_sec;
	}
	else if (*pSec == 0)
	{
		const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::instance().GetClientSet();
		std::for_each(c_set_desc.begin(), c_set_desc.end(), SendDisconnectFunc());
		g_bNoMoreClient = true;
		--*pSec;
		return passes_per_sec;
	}
	else
	{
		char buf[64];
		snprintf(buf, sizeof(buf), LC_TEXT("¼Ë´Ù¿îÀÌ %dÃÊ ³²¾Ò½À´Ï´Ù."), *pSec);
		SendNotice(buf);

		--*pSec;
		return passes_per_sec;
	}
}

void Shutdown(int32_t iSec)
{
	if (g_bNoMoreClient)
	{
		thecore_shutdown();
		return;
	}

	CWarMapManager::instance().OnShutdown();

	char buf[64];
	snprintf(buf, sizeof(buf), LC_TEXT("%dÃÊ ÈÄ °ÔÀÓÀÌ ¼Ë´Ù¿î µË´Ï´Ù."), iSec);

	SendNotice(buf);

	shutdown_event_data* info = AllocEventInfo<shutdown_event_data>();
	info->seconds = iSec;

	event_create(shutdown_event, info, 1);
}

ACMD(do_shutdown)
{
	//@correction013 BEGIN
	if (!ch)
		return;

	sys_err("Accept shutdown command from %s.", ch->GetName());
	//@correction013 END
	TPacketGGShutdown p;
	p.bHeader = HEADER_GG_SHUTDOWN;
	P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGShutdown));

	Shutdown(10);
}

EVENTFUNC(timed_event)
{
	TimedEventInfo * info = dynamic_cast<TimedEventInfo *>( event->info );

	if ( info == nullptr )
	{
		sys_err( "timed_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == nullptr) {
		return 0;
	}
	LPDESC d = ch->GetDesc();

	if (info->left_second <= 0)
	{
		ch->m_pkTimedEvent = nullptr;
		// @correction005
		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
			case SCMD_QUIT:
			case SCMD_PHASE_SELECT:
				{
					TPacketNeedLoginLogInfo acc_info;
					acc_info.dwPlayerID = ch->GetDesc()->GetAccountTable().id;

					db_clientdesc->DBPacket( HEADER_GD_VALID_LOGOUT, 0, &acc_info, sizeof(acc_info) );

					LogManager::instance().DetailLoginLog( false, ch );
				}
				break;
		}

		switch (info->subcmd)
		{
			case SCMD_LOGOUT:
				if (d)
					d->SetPhase(PHASE_CLOSE);
				break;

			case SCMD_QUIT:
				ch->ChatPacket(CHAT_TYPE_COMMAND, "quit");
				break;

			case SCMD_PHASE_SELECT:
				{
					ch->Disconnect("timed_event - SCMD_PHASE_SELECT");

					if (d)
					{
						d->SetPhase(PHASE_SELECT);
					}
				}
				break;
		}

		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÃÊ ³²¾Ò½À´Ï´Ù."), info->left_second);
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

ACMD(do_cmd)
{
	if (ch->m_pkTimedEvent)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ãë¼Ò µÇ¾ú½À´Ï´Ù."));
		event_cancel(&ch->m_pkTimedEvent);
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("·Î±×ÀÎ È­¸éÀ¸·Î µ¹¾Æ °©´Ï´Ù. Àá½Ã¸¸ ±â´Ù¸®¼¼¿ä."));
			break;

		case SCMD_QUIT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°ÔÀÓÀ» Á¾·á ÇÕ´Ï´Ù. Àá½Ã¸¸ ±â´Ù¸®¼¼¿ä."));
			break;

		case SCMD_PHASE_SELECT:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ä³¸¯ÅÍ¸¦ ÀüÈ¯ ÇÕ´Ï´Ù. Àá½Ã¸¸ ±â´Ù¸®¼¼¿ä."));
			break;
	}

	int32_t nExitLimitTime = 10;

	if (ch->IsHack(false, true, nExitLimitTime) &&
		false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()) &&
	   	(!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		return;
	}

	switch (subcmd)
	{
		case SCMD_LOGOUT:
		case SCMD_QUIT:
		case SCMD_PHASE_SELECT:
			{
				TimedEventInfo* info = AllocEventInfo<TimedEventInfo>();

				{
					if (ch->IsPosition(POS_FIGHTING))
						info->left_second = 10;
					else
						info->left_second = 3;
				}

				info->ch		= ch;
				info->subcmd		= subcmd;
				strlcpy(info->szReason, argument, sizeof(info->szReason));

				ch->m_pkTimedEvent	= event_create(timed_event, info, 1);
			}
			break;
	}
}

ACMD(do_mount)
{
}

ACMD(do_fishing)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	ch->SetRotation(atof(arg1));
	ch->fishing();
}

ACMD(do_console)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");
}

ACMD(do_restart)
{
	if (false == ch->IsDead())
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
		ch->StartRecoveryEvent();
		return;
	}

	if (nullptr == ch->m_pkDeadEvent)
		return;

	int32_t iTimeToDead = (event_time(ch->m_pkDeadEvent) / passes_per_sec);

	if (subcmd != SCMD_RESTART_TOWN && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
	{
		if (!test_server)
		{
			if (ch->IsHack())
			{
				if (false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), iTimeToDead - (180 - g_nPortalLimitTime));
					return;
				}
			}
#define eFRS_HERESEC	170
			if (iTimeToDead > eFRS_HERESEC)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), iTimeToDead - eFRS_HERESEC);
				return;
			}
		}
	}

	if (subcmd == SCMD_RESTART_TOWN)
	{
		if (ch->IsHack())
		{
			if ((!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG) ||
				false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%dÃÊ ³²À½)"), iTimeToDead - (180 - g_nPortalLimitTime));
				return;
			}
		}

#define eFRS_TOWNSEC	173
		if (iTimeToDead > eFRS_TOWNSEC)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ ¸¶À»¿¡¼­ Àç½ÃÀÛ ÇÒ ¼ö ¾ø½À´Ï´Ù. (%d ÃÊ ³²À½)"), iTimeToDead - eFRS_TOWNSEC);
			return;
		}
	}

#ifdef __12ZI__
	if (!(ch->GetMapIndex() >= 3550000 && ch->GetMapIndex() < 3560000))
#endif
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

		ch->GetDesc()->SetPhase(PHASE_GAME);
		ch->SetPosition(POS_STANDING);
		ch->StartRecoveryEvent();
	}

	if (1 == quest::CQuestManager::instance().GetEventFlag("threeway_war"))
	{
		if (subcmd == SCMD_RESTART_TOWN || subcmd == SCMD_RESTART_HERE)
		{
			if (true == CThreeWayWar::instance().IsThreeWayWarMapIndex(ch->GetMapIndex()) &&
					false == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));

				ch->ReviveInvisible(5);
				ch->CheckWears();
				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());

				return;
			}

			if (true == CThreeWayWar::instance().IsSungZiMapIndex(ch->GetMapIndex()))
			{
				if (CThreeWayWar::instance().GetReviveTokenForPlayer(ch->GetPlayerID()) <= 0)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ºÁö¿¡¼­ ºÎÈ° ±âÈ¸¸¦ ¸ðµÎ ÀÒ¾ú½À´Ï´Ù! ¸¶À»·Î ÀÌµ¿ÇÕ´Ï´Ù!"));
					ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
				}
				else
				{
					ch->Show(ch->GetMapIndex(), GetSungziStartX(ch->GetEmpire()), GetSungziStartY(ch->GetEmpire()));
				}

				ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
				ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
				ch->ReviveInvisible(5);
				ch->CheckWears();

				return;
			}
		}
	}

	if (ch->GetDungeon())
		ch->GetDungeon()->UseRevive(ch);

	if (ch->GetWarMap() && !ch->IsObserverMode())
	{
		CWarMap * pMap = ch->GetWarMap();
		uint32_t dwGuildOpponent = pMap ? pMap->GetGuildOpponent(ch) : 0;

		if (dwGuildOpponent)
		{
			switch (subcmd)
			{
				case SCMD_RESTART_TOWN:
					sys_log(0, "do_restart: restart town");
					PIXEL_POSITION pos;

					if (CWarMapManager::instance().GetStartPosition(ch->GetMapIndex(), ch->GetGuild()->GetID() < dwGuildOpponent ? 0 : 1, pos))
						ch->Show(ch->GetMapIndex(), pos.x, pos.y);
					else
						ch->ExitToSavedLocation();

					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
					ch->ReviveInvisible(5);
					ch->CheckWears();
					break;

				case SCMD_RESTART_HERE:
					sys_log(0, "do_restart: restart here");
					ch->RestartAtSamePos();
					ch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
					ch->PointChange(POINT_SP, ch->GetMaxSP() - ch->GetSP());
					ch->ReviveInvisible(5);
					ch->CheckWears();
					break;
			}

			return;
		}
	}
	switch (subcmd)
	{
		case SCMD_RESTART_TOWN:
			sys_log(0, "do_restart: restart town");
			PIXEL_POSITION pos;

			if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
				ch->WarpSet(pos.x, pos.y);
			else
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
			ch->PointChange(POINT_HP, 50 - ch->GetHP());
			ch->DeathPenalty(1);
			break;

		case SCMD_RESTART_HERE:
#ifdef __12ZI__
			if (ch->GetMapIndex() >= 3550000 && ch->GetMapIndex() < 3560000)
			{
				int32_t REVIVE_PRICE = ch->GetQuestFlag("zodiac.PrismOfRevival") * 2;
				int32_t vid = (int32_t)ch->GetVID();
				ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenReviveDialog %d %d", vid, REVIVE_PRICE);
				return;
			}
			else
#endif
			{
				sys_log(0, "do_restart: restart here");
				ch->RestartAtSamePos();
				ch->PointChange(POINT_HP, 50 - ch->GetHP());
				ch->DeathPenalty(0);
				ch->ReviveInvisible(5);
				ch->CheckWears();
				break;
			}
	}
}

#define MAX_STAT g_iStatusPointSetMaxValue // @correction218

ACMD(do_stat_reset)
{
	ch->PointChange(POINT_STAT_RESET_COUNT, 12 - ch->GetPoint(POINT_STAT_RESET_COUNT));
}

ACMD(do_stat_minus)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µÐ°© Áß¿¡´Â ´É·ÂÀ» ¿Ã¸± ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetPoint(POINT_STAT_RESET_COUNT) <= 0)
		return;

	if (!strcmp(arg1, "st"))
	{
		if (ch->GetRealPoint(POINT_ST) <= JobInitialPoints[ch->GetJob()].st)
			return;

		ch->SetRealPoint(POINT_ST, ch->GetRealPoint(POINT_ST) - 1);
		ch->SetPoint(POINT_ST, ch->GetPoint(POINT_ST) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_ST, 0);
	}
	else if (!strcmp(arg1, "dx"))
	{
		if (ch->GetRealPoint(POINT_DX) <= JobInitialPoints[ch->GetJob()].dx)
			return;

		ch->SetRealPoint(POINT_DX, ch->GetRealPoint(POINT_DX) - 1);
		ch->SetPoint(POINT_DX, ch->GetPoint(POINT_DX) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_DX, 0);
	}
	else if (!strcmp(arg1, "ht"))
	{
		if (ch->GetRealPoint(POINT_HT) <= JobInitialPoints[ch->GetJob()].ht)
			return;

		ch->SetRealPoint(POINT_HT, ch->GetRealPoint(POINT_HT) - 1);
		ch->SetPoint(POINT_HT, ch->GetPoint(POINT_HT) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_HT, 0);
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (!strcmp(arg1, "iq"))
	{
		if (ch->GetRealPoint(POINT_IQ) <= JobInitialPoints[ch->GetJob()].iq)
			return;

		ch->SetRealPoint(POINT_IQ, ch->GetRealPoint(POINT_IQ) - 1);
		ch->SetPoint(POINT_IQ, ch->GetPoint(POINT_IQ) - 1);
		ch->ComputePoints();
		ch->PointChange(POINT_IQ, 0);
		ch->PointChange(POINT_MAX_SP, 0);
	}
	else
		return;

	ch->PointChange(POINT_STAT, +1);
	ch->PointChange(POINT_STAT_RESET_COUNT, -1);
	ch->ComputePoints();
}

ACMD(do_stat)
{
	char arg1[256];
#ifdef ENABLE_FAST_STAT // @correction195
	char arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;
#else
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;
#endif

	if (ch->IsPolymorphed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µÐ°© Áß¿¡´Â ´É·ÂÀ» ¿Ã¸± ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	uint8_t idx = 0;
	uint8_t count = 0;
#ifdef ENABLE_FAST_STAT // @correction195
	str_to_number(count, arg2);
#endif
	if (!strcmp(arg1, "st"))
		idx = POINT_ST;
	else if (!strcmp(arg1, "dx"))
		idx = POINT_DX;
	else if (!strcmp(arg1, "ht"))
		idx = POINT_HT;
	else if (!strcmp(arg1, "iq"))
		idx = POINT_IQ;
	else
		return;

	if (ch->GetPoint(POINT_STAT) < count)
		count = ch->GetPoint(POINT_STAT);

	if (ch->GetRealPoint(idx) + count > MAX_STAT)
		count = MAX_STAT - ch->GetRealPoint(idx);

	if (!count)
		return;

	ch->SetRealPoint(idx, ch->GetRealPoint(idx) + count);
	ch->SetPoint(idx, ch->GetPoint(idx) + count);
	ch->ComputePoints();
	ch->PointChange(idx, 0);

	if (idx == POINT_IQ)
	{
		ch->PointChange(POINT_MAX_HP, 0);
	}
	else if (idx == POINT_HT)
	{
		ch->PointChange(POINT_MAX_SP, 0);
	}

	ch->PointChange(POINT_STAT, -count);
	ch->ComputePoints();
}

ACMD(do_pvp)
{
	if (ch->GetArena() != nullptr || CArenaManager::instance().IsArenaMap(ch->GetMapIndex()) == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(vid);

	if (!pkVictim)
		return;

	if (pkVictim->IsNPC())
		return;

	if (pkVictim->GetArena() != nullptr)
	{
		pkVictim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ó´ë¹æÀÌ ´ë·ÃÁßÀÔ´Ï´Ù."));
		return;
	}

	CPVPManager::instance().Insert(ch, pkVictim);
}

ACMD(do_guildskillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	if (!ch->GetGuild())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå¿¡ ¼ÓÇØÀÖÁö ¾Ê½À´Ï´Ù."));
		return;
	}

	CGuild* g = ch->GetGuild();
	TGuildMember* gm = g->GetMember(ch->GetPlayerID());
	if (gm->grade == GUILD_LEADER_GRADE)
	{
		uint32_t vnum = 0;
		str_to_number(vnum, arg1);
		g->SkillLevelUp(vnum);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµå ½ºÅ³ ·¹º§À» º¯°æÇÒ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
	}
}

ACMD(do_skillup)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vnum = 0;
	str_to_number(vnum, arg1);

	if (true == ch->CanUseSkill(vnum))
	{
		ch->SkillLevelUp(vnum);
	}
	else
	{
		switch(vnum)
		{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:

			case SKILL_7_A_ANTI_TANHWAN:
			case SKILL_7_B_ANTI_AMSEOP:
			case SKILL_7_C_ANTI_SWAERYUNG:
			case SKILL_7_D_ANTI_YONGBI:

			case SKILL_8_A_ANTI_GIGONGCHAM:
			case SKILL_8_B_ANTI_YEONSA:
			case SKILL_8_C_ANTI_MAHWAN:
			case SKILL_8_D_ANTI_BYEURAK:

			case SKILL_ADD_HP:
			case SKILL_RESIST_PENETRATE:
#ifdef __678TH_SKILL__
			case SKILL_ANTI_PALBANG:
			case SKILL_ANTI_AMSEOP:
			case SKILL_ANTI_SWAERYUNG:
			case SKILL_ANTI_YONGBI:
			case SKILL_ANTI_GIGONGCHAM:
			case SKILL_ANTI_HWAJO:
			case SKILL_ANTI_MARYUNG:
			case SKILL_ANTI_BYEURAK:
#ifdef ENABLE_WOLFMAN_CHARACTER
			case SKILL_ANTI_SALPOONG:
#endif
			case SKILL_HELP_PALBANG:
			case SKILL_HELP_AMSEOP:
			case SKILL_HELP_SWAERYUNG:
			case SKILL_HELP_YONGBI:
			case SKILL_HELP_GIGONGCHAM:
			case SKILL_HELP_HWAJO:
			case SKILL_HELP_MARYUNG:
			case SKILL_HELP_BYEURAK:
#ifdef ENABLE_WOLFMAN_CHARACTER
			case SKILL_HELP_SALPOONG:
#endif
#endif
				ch->SkillLevelUp(vnum);
				break;
		}
	}
}

ACMD(do_safebox_close)
{
	ch->CloseSafebox();
}

ACMD(do_safebox_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	ch->ReqSafeboxLoad(arg1);
}

ACMD(do_safebox_change_password)
{
	char arg1[256];
	char arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || strlen(arg1)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}

	if (!*arg2 || strlen(arg2)>6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}
	// @correction005
	TSafeboxChangePasswordPacket p;

	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szOldPassword, arg1, sizeof(p.szOldPassword));
	strlcpy(p.szNewPassword, arg2, sizeof(p.szNewPassword));

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_CHANGE_PASSWORD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_password)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1 || strlen(arg1) > 6)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Àß¸øµÈ ¾ÏÈ£¸¦ ÀÔ·ÂÇÏ¼Ì½À´Ï´Ù."));
		return;
	}

	int32_t iPulse = thecore_pulse();

	if (ch->GetMall())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Ã¢°í°¡ ÀÌ¹Ì ¿­·ÁÀÖ½À´Ï´Ù."));
		return;
	}

	if (iPulse - ch->GetMallLoadTime() < passes_per_sec * 10)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<Ã¢°í> Ã¢°í¸¦ ´ÝÀºÁö 10ÃÊ ¾È¿¡´Â ¿­ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	ch->SetMallLoadTime(iPulse);

	TSafeboxLoadPacket p;
	p.dwID = ch->GetDesc()->GetAccountTable().id;
	strlcpy(p.szLogin, ch->GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, arg1, sizeof(p.szPassword));

	db_clientdesc->DBPacket(HEADER_GD_MALL_LOAD, ch->GetDesc()->GetHandle(), &p, sizeof(p));
}

ACMD(do_mall_close)
{
	if (ch->GetMall())
	{
		ch->SetMallLoadTime(thecore_pulse());
		ch->CloseMall();
		ch->Save();
	}
}

ACMD(do_ungroup)
{
	if (!ch->GetParty())
		return;

	if (!CPartyManager::instance().IsEnablePCParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ¼­¹ö ¹®Á¦·Î ÆÄÆ¼ °ü·Ã Ã³¸®¸¦ ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetDungeon())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ´øÀü ¾È¿¡¼­´Â ÆÄÆ¼¿¡¼­ ³ª°¥ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	LPPARTY pParty = ch->GetParty();

	if (pParty->GetMemberCount() == 2)
	{
		CPartyManager::instance().DeleteParty(pParty);
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<ÆÄÆ¼> ÆÄÆ¼¿¡¼­ ³ª°¡¼Ì½À´Ï´Ù."));
		pParty->Quit(ch->GetPlayerID());
	}
}

ACMD(do_close_shop)
{
	if (ch->IsObserverMode())
		return;

	if (ch->GetMyShop())
	{
		ch->CloseMyShop();
		return;
	}
}

ACMD(do_set_walk_mode)
{
	ch->SetNowWalking(true);
	ch->SetWalking(true);
}

ACMD(do_set_run_mode)
{
	ch->SetNowWalking(false);
	ch->SetWalking(false);
}

ACMD(do_war)
{
	CGuild * g = ch->GetGuild();

	if (!g)
		return;

	if (g->UnderAnyWar())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÀÌ¹Ì ´Ù¸¥ ÀüÀï¿¡ ÂüÀü Áß ÀÔ´Ï´Ù."));
		return;
	}

	char arg1[256], arg2[256];
	uint32_t type = GUILD_WAR_TYPE_FIELD; // @correction029 int32_t -> uint32_t
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
		return;

	if (*arg2)
	{
		str_to_number(type, arg2);

		if (type >= GUILD_WAR_TYPE_MAX_NUM)
			type = GUILD_WAR_TYPE_FIELD;
	}

	uint32_t gm_pid = g->GetMasterPID();

	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀü¿¡ ´ëÇÑ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		return;
	}

	CGuild * opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±×·± ±æµå°¡ ¾ø½À´Ï´Ù."));
		return;
	}

	switch (g->GetGuildWarState(opp_g->GetID()))
	{
		case GUILD_WAR_NONE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµå°¡ ÀÌ¹Ì ÀüÀï Áß ÀÔ´Ï´Ù."));
					return;
				}

				int32_t iWarPrice = KOR_aGuildWarInfo[type].iWarPrice;

				if (g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> Àüºñ°¡ ºÎÁ·ÇÏ¿© ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return;
				}

				if (opp_g->GetGuildMoney() < iWarPrice)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ Àüºñ°¡ ºÎÁ·ÇÏ¿© ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return;
				}
			}
			break;

		case GUILD_WAR_SEND_DECLARE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì ¼±ÀüÆ÷°í ÁßÀÎ ±æµåÀÔ´Ï´Ù."));
				return;
			}
			break;

		case GUILD_WAR_RECV_DECLARE:
			{
				if (opp_g->UnderAnyWar())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµå°¡ ÀÌ¹Ì ÀüÀï Áß ÀÔ´Ï´Ù."));
					g->RequestRefuseWar(opp_g->GetID());
					return;
				}
			}
			break;

		case GUILD_WAR_RESERVE:
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÀÌ¹Ì ÀüÀïÀÌ ¿¹¾àµÈ ±æµå ÀÔ´Ï´Ù."));
				return;
			}
			break;

		case GUILD_WAR_END:
			return;

		default:
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ÀÌ¹Ì ÀüÀï ÁßÀÎ ±æµåÀÔ´Ï´Ù."));
			g->RequestRefuseWar(opp_g->GetID());
			return;
	}

	if (!g->CanStartWar(type))
	{
		if (g->GetLadderPoint() == 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ·¹´õ Á¡¼ö°¡ ¸ðÀÚ¶ó¼­ ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			sys_log(0, "GuildWar.StartError.NEED_LADDER_POINT");
		}
		else if (g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀüÀ» ÇÏ±â À§ÇØ¼± ÃÖ¼ÒÇÑ %d¸íÀÌ ÀÖ¾î¾ß ÇÕ´Ï´Ù."), GUILD_WAR_MIN_MEMBER_COUNT);
			sys_log(0, "GuildWar.StartError.NEED_MINIMUM_MEMBER[%d]", GUILD_WAR_MIN_MEMBER_COUNT);
		}
		else
		{
			sys_log(0, "GuildWar.StartError.UNKNOWN_ERROR");
		}
		return;
	}

	if (!opp_g->CanStartWar(GUILD_WAR_TYPE_FIELD))
	{
		if (opp_g->GetLadderPoint() == 0)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ·¹´õ Á¡¼ö°¡ ¸ðÀÚ¶ó¼­ ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		else if (opp_g->GetMemberCount() < GUILD_WAR_MIN_MEMBER_COUNT)
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ±æµå¿ø ¼ö°¡ ºÎÁ·ÇÏ¿© ±æµåÀüÀ» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	do
	{
		if (g->GetMasterCharacter() != nullptr)
			break;

		CCI *pCCI = P2P_MANAGER::instance().FindByPID(g->GetMasterPID());

		if (pCCI != nullptr)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ±æµåÀåÀÌ Á¢¼ÓÁßÀÌ ¾Æ´Õ´Ï´Ù."));
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	do
	{
		if (opp_g->GetMasterCharacter() != nullptr)
			break;

		CCI *pCCI = P2P_MANAGER::instance().FindByPID(opp_g->GetMasterPID());

		if (pCCI != nullptr)
			break;

		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> »ó´ë¹æ ±æµåÀÇ ±æµåÀåÀÌ Á¢¼ÓÁßÀÌ ¾Æ´Õ´Ï´Ù."));
		g->RequestRefuseWar(opp_g->GetID());
		return;

	} while (false);

	g->RequestDeclareWar(opp_g->GetID(), type);
}

ACMD(do_nowar)
{
	CGuild* g = ch->GetGuild();
	if (!g)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t gm_pid = g->GetMasterPID();

	if (gm_pid != ch->GetPlayerID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀü¿¡ ´ëÇÑ ±ÇÇÑÀÌ ¾ø½À´Ï´Ù."));
		return;
	}

	CGuild* opp_g = CGuildManager::instance().FindGuildByName(arg1);

	if (!opp_g)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±×·± ±æµå°¡ ¾ø½À´Ï´Ù."));
		return;
	}

	g->RequestRefuseWar(opp_g->GetID());
}

ACMD(do_detaillog)
{
	ch->DetailLog();
}

ACMD(do_monsterlog)
{
	ch->ToggleMonsterLog();
}

ACMD(do_pkmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint8_t mode = 0;
	str_to_number(mode, arg1);

	if (mode == PK_MODE_PROTECT)
		return;

	if (ch->GetLevel() < PK_PROTECT_LEVEL && mode != 0)
		return;

	ch->SetPKMode(mode);
}

ACMD(do_messenger_auth)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	char answer = LOWER(*arg1);
	// @correction051 AuthToAdd void -> bool
	bool bIsDenied = answer != 'y';
	bool bIsAdded = MessengerManager::instance().AuthToAdd(ch->GetName(), arg2, bIsDenied);
	if (bIsAdded && bIsDenied)
	{
		LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg2);

		if (tch)
			tch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ´ÔÀ¸·Î ºÎÅÍ Ä£±¸ µî·ÏÀ» °ÅºÎ ´çÇß½À´Ï´Ù."), ch->GetName());
	}

}

ACMD(do_setblockmode)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		uint8_t flag = 0;
		str_to_number(flag, arg1);
		ch->SetBlockMode(flag);
	}
}

ACMD(do_unmount)
{
#ifdef __COSTUME_MOUNT__
#ifndef __MOUNT__
	LPITEM item = ch->GetWear(WEAR_COSTUME_MOUNT);
	if (item && item->IsRideItem())
		ch->UnequipItem(item);
#endif
#else
	LPITEM item = ch->GetWear(WEAR_UNIQUE1);
	LPITEM item2 = ch->GetWear(WEAR_UNIQUE2);

	if (item && item->IsRideItem())
		ch->UnequipItem(item);

	if (item2 && item2->IsRideItem())
		ch->UnequipItem(item2);
#endif
	if (true == ch->UnEquipSpecialRideUniqueItem())
	{
		ch->RemoveAffect(AFFECT_MOUNT);
		ch->RemoveAffect(AFFECT_MOUNT_BONUS);

		if (ch->IsHorseRiding())
		{
			ch->StopRiding();
		}
	}
	else
	{
		ch->ChatPacket( CHAT_TYPE_INFO, LC_TEXT("ÀÎº¥Åä¸®°¡ ²Ë Â÷¼­ ³»¸± ¼ö ¾ø½À´Ï´Ù."));
	}

}

ACMD(do_observer_exit)
{
	if (ch->IsObserverMode())
	{
		if (ch->GetWarMap())
			ch->SetWarMap(nullptr);

		if (ch->GetArena() != nullptr || ch->GetArenaObserverMode() == true)
		{
			ch->SetArenaObserverMode(false);

			if (ch->GetArena() != nullptr)
				ch->GetArena()->RemoveObserver(ch->GetPlayerID());

			ch->SetArena(nullptr);
			ch->WarpSet(ARENA_RETURN_POINT_X(ch->GetEmpire()), ARENA_RETURN_POINT_Y(ch->GetEmpire()));
		}
		else
		{
			ch->ExitToSavedLocation();
		}
		ch->SetObserverMode(false);
	}
}

ACMD(do_view_equip)
{
	// if (ch->GetGMLevel() <= GM_PLAYER)
		// return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (*arg1)
	{
		uint32_t vid = 0;
		str_to_number(vid, arg1);
		LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

		if (!tch)
			return;

		if (!tch->IsPC())
			return;
		if (tch->IsBlockMode(BLOCK_VIEW_EQUIPMENT))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ekipman"));
			return;
		}

		tch->SendEquipment(ch);
	}
}

ACMD(do_party_request)
{
	if (ch->GetArena())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->GetParty())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì ÆÄÆ¼¿¡ ¼ÓÇØ ÀÖÀ¸¹Ç·Î °¡ÀÔ½ÅÃ»À» ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		if (!ch->RequestToParty(tch))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

ACMD(do_party_request_accept)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->AcceptToParty(tch);
}

ACMD(do_party_request_deny)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vid = 0;
	str_to_number(vid, arg1);
	LPCHARACTER tch = CHARACTER_MANAGER::instance().Find(vid);

	if (tch)
		ch->DenyToParty(tch);
}

ACMD(do_monarch_warpto)
{
	// @correction005
	if (!CMonarch::instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ë °¡´ÉÇÑ ±â´ÉÀÔ´Ï´Ù"));
		return;
	}

	if (!ch->IsMCOK(CHARACTER::MI_WARP))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d ÃÊ°£ ÄðÅ¸ÀÓÀÌ Àû¿ëÁßÀÔ´Ï´Ù."), ch->GetMCLTime(CHARACTER::MI_WARP));
		return;
	}

	const int32_t WarpPrice = 10000;

	if (!CMonarch::instance().IsMoneyOk(WarpPrice, ch->GetEmpire()))
	{
		int32_t NationMoney = CMonarch::instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±¹°í¿¡ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù. ÇöÀç : %u ÇÊ¿ä±Ý¾× : %u"), NationMoney, WarpPrice);
		return;
	}

	int32_t x = 0, y = 0;
	char arg1[256];

	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ë¹ý: warpto <character name>"));
		return;
	}

	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bEmpire != ch->GetEmpire())
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("Å¸Á¦±¹ À¯Àú¿¡°Ô´Â ÀÌµ¿ÇÒ¼ö ¾ø½À´Ï´Ù"));
				return;
			}

			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç À¯Àú´Â %d Ã¤³Î¿¡ ÀÖ½À´Ï´Ù. (ÇöÀç Ã¤³Î %d)"), pkCCI->bChannel, g_bChannel);
				return;
			}
			if (!IsMonarchWarpZone(pkCCI->lMapIndex))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}

			PIXEL_POSITION pos;

			if (!SECTREE_MANAGER::instance().GetCenterPositionOfMap(pkCCI->lMapIndex, pos))
				ch->ChatPacket(CHAT_TYPE_INFO, "Cannot find map (index %d)", pkCCI->lMapIndex);
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ¿¡°Ô·Î ÀÌµ¿ÇÕ´Ï´Ù"), arg1);
				ch->WarpSet(pos.x, pos.y);

				CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

				ch->SetMC(CHARACTER::MI_WARP);
			}
		}
		else if (nullptr == CHARACTER_MANAGER::instance().FindPC(arg1))
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "There is no one by that name");
		}

		return;
	}
	else
	{
		if (tch->GetEmpire() != ch->GetEmpire())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Å¸Á¦±¹ À¯Àú¿¡°Ô´Â ÀÌµ¿ÇÒ¼ö ¾ø½À´Ï´Ù"));
			return;
		}
		if (!IsMonarchWarpZone(tch->GetMapIndex()))
		{
			ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return;
		}
		x = tch->GetX();
		y = tch->GetY();
	}

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ¿¡°Ô·Î ÀÌµ¿ÇÕ´Ï´Ù"), arg1);
	ch->WarpSet(x, y);
	ch->Stop();

	CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);

	ch->SetMC(CHARACTER::MI_WARP);
}

ACMD(do_monarch_transfer)
{
	// @correction005
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ë¹ý: transfer <name>"));
		return;
	}

	if (!CMonarch::instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ë °¡´ÉÇÑ ±â´ÉÀÔ´Ï´Ù"));
		return;
	}

	if (!ch->IsMCOK(CHARACTER::MI_TRANSFER))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d ÃÊ°£ ÄðÅ¸ÀÓÀÌ Àû¿ëÁßÀÔ´Ï´Ù."), ch->GetMCLTime(CHARACTER::MI_TRANSFER));
		return;
	}

	const int32_t WarpPrice = 10000;

	if (!CMonarch::instance().IsMoneyOk(WarpPrice, ch->GetEmpire()))
	{
		int32_t NationMoney = CMonarch::instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±¹°í¿¡ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù. ÇöÀç : %u ÇÊ¿ä±Ý¾× : %u"), NationMoney, WarpPrice);
		return;
	}


	LPCHARACTER tch = CHARACTER_MANAGER::instance().FindPC(arg1);

	if (!tch)
	{
		CCI * pkCCI = P2P_MANAGER::instance().Find(arg1);

		if (pkCCI)
		{
			if (pkCCI->bEmpire != ch->GetEmpire())
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´Ù¸¥ Á¦±¹ À¯Àú´Â ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}
			if (pkCCI->bChannel != g_bChannel)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ´ÔÀº %d Ã¤³Î¿¡ Á¢¼Ó Áß ÀÔ´Ï´Ù. (ÇöÀç Ã¤³Î: %d)"), arg1, pkCCI->bChannel, g_bChannel);
				return;
			}
			if (!IsMonarchWarpZone(pkCCI->lMapIndex))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}
			if (!IsMonarchWarpZone(ch->GetMapIndex()))
			{
				ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
				return;
			}

			TPacketGGTransfer pgg;

			pgg.bHeader = HEADER_GG_TRANSFER;
			strlcpy(pgg.szName, arg1, sizeof(pgg.szName));
			pgg.lX = ch->GetX();
			pgg.lY = ch->GetY();

			P2P_MANAGER::instance().Send(&pgg, sizeof(TPacketGGTransfer));
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ´ÔÀ» ¼ÒÈ¯ÇÏ¿´½À´Ï´Ù."), arg1);

			CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);
			ch->SetMC(CHARACTER::MI_TRANSFER);
		}
		else
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÔ·ÂÇÏ½Å ÀÌ¸§À» °¡Áø »ç¿ëÀÚ°¡ ¾ø½À´Ï´Ù."));
		}

		return;
	}


	if (ch == tch)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÚ½ÅÀ» ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (tch->GetEmpire() != ch->GetEmpire())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´Ù¸¥ Á¦±¹ À¯Àú´Â ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}
	if (!IsMonarchWarpZone(tch->GetMapIndex()))
	{
		ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ÀÌµ¿ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}
	if (!IsMonarchWarpZone(ch->GetMapIndex()))
	{
		ch->ChatPacket (CHAT_TYPE_INFO, LC_TEXT("ÇØ´ç Áö¿ªÀ¸·Î ¼ÒÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	tch->WarpSet(ch->GetX(), ch->GetY(), ch->GetMapIndex());

	CMonarch::instance().SendtoDBDecMoney(WarpPrice, ch->GetEmpire(), ch);
	ch->SetMC(CHARACTER::MI_TRANSFER);
}

ACMD(do_monarch_info)
{
	if (CMonarch::instance().IsMonarch(ch->GetPlayerID(), ch->GetEmpire()))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³ªÀÇ ±ºÁÖ Á¤º¸"));
		TMonarchInfo * p = CMonarch::instance().GetMonarch();
		for (int32_t n = 1; n < 4; ++n)
		{
			if (n == ch->GetEmpire())
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[%s±ºÁÖ] : %s  º¸À¯±Ý¾× %lld "), EMPIRE_NAME(n), p->name[n], p->money[n]);
			else
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[%s±ºÁÖ] : %s  "), EMPIRE_NAME(n), p->name[n]);

		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ Á¤º¸"));
		TMonarchInfo * p = CMonarch::instance().GetMonarch();
		for (int32_t n = 1; n < 4; ++n)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[%s±ºÁÖ] : %s  "), EMPIRE_NAME(n), p->name[n]);

		}
	}

}

ACMD(do_elect)
{
	db_clientdesc->DBPacketHeader(HEADER_GD_COME_TO_VOTE, ch->GetDesc()->GetHandle(), 0);
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

ACMD(do_monarch_tax)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: monarch_tax <1-50>");
		return;
	}

	if (!ch->IsMonarch())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ëÇÒ¼ö ÀÖ´Â ±â´ÉÀÔ´Ï´Ù"));
		return;
	}

	int32_t tax = 0;
	str_to_number(tax,  arg1);

	if (tax < 1 || tax > 50)
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("1-50 »çÀÌÀÇ ¼öÄ¡¸¦ ¼±ÅÃÇØÁÖ¼¼¿ä"));

	quest::CQuestManager::instance().SetEventFlag("trade_tax", tax);

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼¼±ÝÀÌ %d %·Î ¼³Á¤µÇ¾ú½À´Ï´Ù"));

	char szMsg[1024];

	snprintf(szMsg, sizeof(szMsg), "±ºÁÖÀÇ ¸íÀ¸·Î ¼¼±ÝÀÌ %d %% ·Î º¯°æµÇ¾ú½À´Ï´Ù", tax);
	BroadcastNotice(szMsg);

	snprintf(szMsg, sizeof(szMsg), "¾ÕÀ¸·Î´Â °Å·¡ ±Ý¾×ÀÇ %d %% °¡ ±¹°í·Î µé¾î°¡°ÔµË´Ï´Ù.", tax);
	BroadcastNotice(szMsg);

	ch->SetMC(CHARACTER::MI_TAX);
}

static const uint32_t cs_dwMonarchMobVnums[] =
{
	191,
	192,
	193,
	194,
	391,
	392,
	393,
	394,
	491,
	492,
	493,
	494,
	591,
	691,
	791,
	1304,
	1901,
	2091,
	2191,
	2206,
	0,
};

ACMD(do_monarch_mob)
{
	char arg1[256];
	LPCHARACTER	tch;

	one_argument(argument, arg1, sizeof(arg1));

	if (!ch->IsMonarch())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖ¸¸ÀÌ »ç¿ëÇÒ¼ö ÀÖ´Â ±â´ÉÀÔ´Ï´Ù"));
		return;
	}

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: mmob <mob name>");
		return;
	}

#ifdef ENABLE_MONARCH_MOB_CMD_MAP_CHECK // @correction005
	uint8_t pcEmpire = ch->GetEmpire();
	uint8_t mapEmpire = SECTREE_MANAGER::instance().GetEmpireFromMapIndex(ch->GetMapIndex());
	if (mapEmpire != pcEmpire && mapEmpire != 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÚ±¹ ¿µÅä¿¡¼­¸¸ »ç¿ëÇÒ ¼ö ÀÖ´Â ±â´ÉÀÔ´Ï´Ù"));
		return;
	}
#endif

	const int32_t SummonPrice = 5000000;

	if (!ch->IsMCOK(CHARACTER::MI_SUMMON))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d ÃÊ°£ ÄðÅ¸ÀÓÀÌ Àû¿ëÁßÀÔ´Ï´Ù."), ch->GetMCLTime(CHARACTER::MI_SUMMON));
		return;
	}

	if (!CMonarch::instance().IsMoneyOk(SummonPrice, ch->GetEmpire()))
	{
		int32_t NationMoney = CMonarch::instance().GetMoney(ch->GetEmpire());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±¹°í¿¡ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù. ÇöÀç : %u ÇÊ¿ä±Ý¾× : %u"), NationMoney, SummonPrice);
		return;
	}

	const CMob * pkMob;
	uint32_t vnum = 0;

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

	uint32_t count;

	for (count = 0; cs_dwMonarchMobVnums[count] != 0; ++count)
		if (cs_dwMonarchMobVnums[count] == vnum)
			break;

	if (0 == cs_dwMonarchMobVnums[count])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÈ¯ÇÒ¼ö ¾ø´Â ¸ó½ºÅÍ ÀÔ´Ï´Ù. ¼ÒÈ¯°¡´ÉÇÑ ¸ó½ºÅÍ´Â È¨ÆäÀÌÁö¸¦ ÂüÁ¶ÇÏ¼¼¿ä"));
		return;
	}

	tch = CHARACTER_MANAGER::instance().SpawnMobRange(vnum,
			ch->GetMapIndex(),
			ch->GetX() - number(200, 750),
			ch->GetY() - number(200, 750),
			ch->GetX() + number(200, 750),
			ch->GetY() + number(200, 750),
			true,
			pkMob->m_table.bType == CHAR_TYPE_STONE,
			true);

	if (tch)
	{
		CMonarch::instance().SendtoDBDecMoney(SummonPrice, ch->GetEmpire(), ch);

		ch->SetMC(CHARACTER::MI_SUMMON);
	}
}

static const char* FN_point_string(int32_t apply_number)
{
	switch (apply_number)
	{
		case POINT_MAX_HP:	return LC_TEXT("ÃÖ´ë »ý¸í·Â +%d");
		case POINT_MAX_SP:	return LC_TEXT("ÃÖ´ë Á¤½Å·Â +%d");
		case POINT_HT:		return LC_TEXT("Ã¼·Â +%d");
		case POINT_IQ:		return LC_TEXT("Áö´É +%d");
		case POINT_ST:		return LC_TEXT("±Ù·Â +%d");
		case POINT_DX:		return LC_TEXT("¹ÎÃ¸ +%d");
		case POINT_ATT_SPEED:	return LC_TEXT("°ø°Ý¼Óµµ +%d");
		case POINT_MOV_SPEED:	return LC_TEXT("ÀÌµ¿¼Óµµ %d");
		case POINT_CASTING_SPEED:	return LC_TEXT("ÄðÅ¸ÀÓ -%d");
		case POINT_HP_REGEN:	return LC_TEXT("»ý¸í·Â È¸º¹ +%d");
		case POINT_SP_REGEN:	return LC_TEXT("Á¤½Å·Â È¸º¹ +%d");
		case POINT_POISON_PCT:	return LC_TEXT("µ¶°ø°Ý %d");
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case POINT_BLEEDING_PCT:	return LC_TEXT("µ¶°ø°Ý %d");
#endif
		case POINT_STUN_PCT:	return LC_TEXT("½ºÅÏ +%d");
		case POINT_SLOW_PCT:	return LC_TEXT("½½·Î¿ì +%d");
		case POINT_CRITICAL_PCT:	return LC_TEXT("%d%% È®·ü·Î Ä¡¸íÅ¸ °ø°Ý");
		case POINT_RESIST_CRITICAL:	return LC_TEXT("»ó´ëÀÇ Ä¡¸íÅ¸ È®·ü %d%% °¨¼Ò");
		case POINT_PENETRATE_PCT:	return LC_TEXT("%d%% È®·ü·Î °üÅë °ø°Ý");
		case POINT_RESIST_PENETRATE: return LC_TEXT("»ó´ëÀÇ °üÅë °ø°Ý È®·ü %d%% °¨¼Ò");
		case POINT_ATTBONUS_HUMAN:	return LC_TEXT("ÀÎ°£·ù ¸ó½ºÅÍ Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_ANIMAL:	return LC_TEXT("µ¿¹°·ù ¸ó½ºÅÍ Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_ORC:	return LC_TEXT("¿õ±ÍÁ· Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_MILGYO:	return LC_TEXT("¹Ð±³·ù Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_UNDEAD:	return LC_TEXT("½ÃÃ¼·ù Å¸°ÝÄ¡ +%d%%");
		case POINT_ATTBONUS_DEVIL:	return LC_TEXT("¾Ç¸¶·ù Å¸°ÝÄ¡ +%d%%");
		case POINT_STEAL_HP:		return LC_TEXT("Å¸°ÝÄ¡ %d%% ¸¦ »ý¸í·ÂÀ¸·Î Èí¼ö");
		case POINT_STEAL_SP:		return LC_TEXT("Å¸·ÂÄ¡ %d%% ¸¦ Á¤½Å·ÂÀ¸·Î Èí¼ö");
		case POINT_MANA_BURN_PCT:	return LC_TEXT("%d%% È®·ü·Î Å¸°Ý½Ã »ó´ë Àü½Å·Â ¼Ò¸ð");
		case POINT_DAMAGE_SP_RECOVER:	return LC_TEXT("%d%% È®·ü·Î ÇÇÇØ½Ã Á¤½Å·Â È¸º¹");
		case POINT_BLOCK:			return LC_TEXT("¹°¸®Å¸°Ý½Ã ºí·° È®·ü %d%%");
		case POINT_DODGE:			return LC_TEXT("È° °ø°Ý È¸ÇÇ È®·ü %d%%");
		case POINT_RESIST_SWORD:	return LC_TEXT("ÇÑ¼Õ°Ë ¹æ¾î %d%%");
		case POINT_RESIST_TWOHAND:	return LC_TEXT("¾ç¼Õ°Ë ¹æ¾î %d%%");
		case POINT_RESIST_DAGGER:	return LC_TEXT("µÎ¼Õ°Ë ¹æ¾î %d%%");
		case POINT_RESIST_BELL:		return LC_TEXT("¹æ¿ï ¹æ¾î %d%%");
		case POINT_RESIST_FAN:		return LC_TEXT("ºÎÃ¤ ¹æ¾î %d%%");
		case POINT_RESIST_BOW:		return LC_TEXT("È°°ø°Ý ÀúÇ× %d%%");
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case POINT_RESIST_CLAW:		return LC_TEXT("µÎ¼Õ°Ë ¹æ¾î %d%%");
#endif
		case POINT_RESIST_FIRE:		return LC_TEXT("È­¿° ÀúÇ× %d%%");
		case POINT_RESIST_ELEC:		return LC_TEXT("Àü±â ÀúÇ× %d%%");
		case POINT_RESIST_MAGIC:	return LC_TEXT("¸¶¹ý ÀúÇ× %d%%");
		case POINT_RESIST_WIND:		return LC_TEXT("¹Ù¶÷ ÀúÇ× %d%%");
		case POINT_RESIST_ICE:		return LC_TEXT("³Ã±â ÀúÇ× %d%%");
		case POINT_RESIST_EARTH:	return LC_TEXT("´ëÁö ÀúÇ× %d%%");
		case POINT_RESIST_DARK:		return LC_TEXT("¾îµÒ ÀúÇ× %d%%");
		case POINT_REFLECT_MELEE:	return LC_TEXT("Á÷Á¢ Å¸°ÝÄ¡ ¹Ý»ç È®·ü : %d%%");
		case POINT_REFLECT_CURSE:	return LC_TEXT("ÀúÁÖ µÇµ¹¸®±â È®·ü %d%%");
		case POINT_POISON_REDUCE:	return LC_TEXT("µ¶ ÀúÇ× %d%%");
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case POINT_BLEEDING_REDUCE:	return LC_TEXT("µ¶ ÀúÇ× %d%%");
#endif
		case POINT_KILL_SP_RECOVER:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã Á¤½Å·Â È¸º¹");
		case POINT_EXP_DOUBLE_BONUS:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã °æÇèÄ¡ Ãß°¡ »ó½Â");
		case POINT_GOLD_DOUBLE_BONUS:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã µ· 2¹è µå·Ó");
		case POINT_ITEM_DROP_BONUS:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã ¾ÆÀÌÅÛ 2¹è µå·Ó");
		case POINT_POTION_BONUS:	return LC_TEXT("¹°¾à »ç¿ë½Ã %d%% ¼º´É Áõ°¡");
		case POINT_KILL_HP_RECOVERY:	return LC_TEXT("%d%% È®·ü·Î ÀûÅðÄ¡½Ã »ý¸í·Â È¸º¹");
		case POINT_ATT_GRADE_BONUS:	return LC_TEXT("°ø°Ý·Â +%d");
		case POINT_DEF_GRADE_BONUS:	return LC_TEXT("¹æ¾î·Â +%d");
		case POINT_MAGIC_ATT_GRADE:	return LC_TEXT("¸¶¹ý °ø°Ý·Â +%d");
		case POINT_MAGIC_DEF_GRADE:	return LC_TEXT("¸¶¹ý ¹æ¾î·Â +%d");
		case POINT_MAX_STAMINA:	return LC_TEXT("ÃÖ´ë Áö±¸·Â +%d");
		case POINT_ATTBONUS_WARRIOR:	return LC_TEXT("¹«»ç¿¡°Ô °­ÇÔ +%d%%");
		case POINT_ATTBONUS_ASSASSIN:	return LC_TEXT("ÀÚ°´¿¡°Ô °­ÇÔ +%d%%");
		case POINT_ATTBONUS_SURA:		return LC_TEXT("¼ö¶ó¿¡°Ô °­ÇÔ +%d%%");
		case POINT_ATTBONUS_SHAMAN:		return LC_TEXT("¹«´ç¿¡°Ô °­ÇÔ +%d%%");
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case POINT_ATTBONUS_WOLFMAN:	return LC_TEXT("¹«´ç¿¡°Ô °­ÇÔ +%d%%");
#endif
		case POINT_ATTBONUS_MONSTER:	return LC_TEXT("¸ó½ºÅÍ¿¡°Ô °­ÇÔ +%d%%");
		case POINT_MALL_ATTBONUS:		return LC_TEXT("°ø°Ý·Â +%d%%");
		case POINT_MALL_DEFBONUS:		return LC_TEXT("¹æ¾î·Â +%d%%");
		case POINT_MALL_EXPBONUS:		return LC_TEXT("°æÇèÄ¡ %d%%");
		case POINT_MALL_ITEMBONUS:		return LC_TEXT("¾ÆÀÌÅÛ µå·ÓÀ² %.1f¹è");
		case POINT_MALL_GOLDBONUS:		return LC_TEXT("µ· µå·ÓÀ² %.1f¹è");
		case POINT_MAX_HP_PCT:			return LC_TEXT("ÃÖ´ë »ý¸í·Â +%d%%");
		case POINT_MAX_SP_PCT:			return LC_TEXT("ÃÖ´ë Á¤½Å·Â +%d%%");
		case POINT_SKILL_DAMAGE_BONUS:	return LC_TEXT("½ºÅ³ µ¥¹ÌÁö %d%%");
		case POINT_NORMAL_HIT_DAMAGE_BONUS:	return LC_TEXT("ÆòÅ¸ µ¥¹ÌÁö %d%%");
		case POINT_SKILL_DEFEND_BONUS:		return LC_TEXT("½ºÅ³ µ¥¹ÌÁö ÀúÇ× %d%%");
		case POINT_NORMAL_HIT_DEFEND_BONUS:	return LC_TEXT("ÆòÅ¸ µ¥¹ÌÁö ÀúÇ× %d%%");
		case POINT_RESIST_WARRIOR:	return LC_TEXT("¹«»ç°ø°Ý¿¡ %d%% ÀúÇ×");
		case POINT_RESIST_ASSASSIN:	return LC_TEXT("ÀÚ°´°ø°Ý¿¡ %d%% ÀúÇ×");
		case POINT_RESIST_SURA:		return LC_TEXT("¼ö¶ó°ø°Ý¿¡ %d%% ÀúÇ×");
		case POINT_RESIST_SHAMAN:	return LC_TEXT("¹«´ç°ø°Ý¿¡ %d%% ÀúÇ×");
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case POINT_RESIST_WOLFMAN:	return LC_TEXT("¹«´ç°ø°Ý¿¡ %d%% ÀúÇ×");
#endif
		default:					return nullptr;
	}
}

static bool FN_hair_affect_string(LPCHARACTER ch, char *buf, size_t bufsiz)
{
	if (nullptr == ch || nullptr == buf)
		return false;

	CAffect* aff = nullptr;
	time_t expire = 0;
	struct tm ltm;
	int32_t	year, mon, day;
	int32_t	offset = 0;

	aff = ch->FindAffect(AFFECT_HAIR);

	if (nullptr == aff)
		return false;

	expire = ch->GetQuestFlag("hair.limit_time");

	if (expire < get_global_time())
		return false;

	const char * pAttrName = FN_point_string(aff->bApplyOn);
	if (!pAttrName)
		return false;

	offset = snprintf(buf, bufsiz, pAttrName, aff->lApplyValue);

	if (offset < 0 || offset >= (int32_t) bufsiz)
		offset = bufsiz - 1;

	localtime_r(&expire, &ltm);

	year	= ltm.tm_year + 1900;
	mon		= ltm.tm_mon + 1;
	day		= ltm.tm_mday;

	snprintf(buf + offset, bufsiz - offset, LC_TEXT(" (¸¸·áÀÏ : %d³â %d¿ù %dÀÏ)"), year, mon, day);

	return true;
}

ACMD(do_costume)
{
#ifdef __ACCE_SYSTEM__ // @correction191
	char buf[768];
#else
	char buf[512];
#endif
	const size_t bufferSize = sizeof(buf);

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	CItem* pBody = ch->GetWear(WEAR_COSTUME_BODY);
	CItem* pHair = ch->GetWear(WEAR_COSTUME_HAIR);
#ifdef __ACCE_SYSTEM__ // @correction191
	CItem * pAcce = ch->GetWear(WEAR_COSTUME_ACCE);
#endif
	ch->ChatPacket(CHAT_TYPE_INFO, "COSTUME status:");

	if (pHair)
	{
		const char* itemName = pHair->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  HAIR : %s", itemName);

		for (int32_t i = 0; i < pHair->GetAttributeCount(); ++i)
		{
			const TPlayerItemAttribute& attr = pHair->GetAttribute(i);
			if (0 < attr.bType)
			{
				const char * pAttrName = FN_point_string(attr.bType);
				if (!pAttrName)
					continue;

				snprintf(buf, bufferSize, pAttrName, attr.sValue);
				ch->ChatPacket(CHAT_TYPE_INFO, "     %s", buf);
			}
		}

		if (pHair->IsEquipped() && arg1[0] == 'h')
			ch->UnequipItem(pHair);
	}

	if (pBody)
	{
		const char* itemName = pBody->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  BODY : %s", itemName);

		if (pBody->IsEquipped() && arg1[0] == 'b')
			ch->UnequipItem(pBody);
	}

#ifdef __ACCE_SYSTEM__ // @correction191
	if (pAcce)
	{
		const char * itemName = pAcce->GetName();
		ch->ChatPacket(CHAT_TYPE_INFO, "  ACCE: %s", itemName);
		for (int32_t i = 0; i < pAcce->GetAttributeCount(); ++i)
		{
			const TPlayerItemAttribute& attr = pAcce->GetAttribute(i);
			if (attr.bType > 0)
			{
				const char * pAttrName = FN_point_string(attr.bType);
				if (!pAttrName)
					continue;
				
				snprintf(buf, sizeof(buf), pAttrName, attr.sValue);
				ch->ChatPacket(CHAT_TYPE_INFO, "     %s", buf);
			}
		}

		if (pAcce->IsEquipped() && arg1[0] == 'a')
			ch->UnequipItem(pAcce);
	}
#endif
}

ACMD(do_hair)
{
	char buf[256];

	if (false == FN_hair_affect_string(ch, buf, sizeof(buf)))
		return;

	ch->ChatPacket(CHAT_TYPE_INFO, buf);
}

ACMD(do_inventory)
{
	int32_t	index = 0;
	int32_t	count		= 1;

	char arg1[256];
	char arg2[256];

	LPITEM	item;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: inventory <start_index> <count>");
		return;
	}

	if (!*arg2)
	{
		index = 0;
		str_to_number(count, arg1);
	}
	else
	{
		str_to_number(index, arg1); index = MIN(index, INVENTORY_MAX_NUM);
		str_to_number(count, arg2); count = MIN(count, INVENTORY_MAX_NUM);
	}

	for (int32_t i = 0; i < count; ++i)
	{
		if (index >= INVENTORY_MAX_NUM)
			break;

		item = ch->GetInventoryItem(index);

		ch->ChatPacket(CHAT_TYPE_INFO, "inventory [%d] = %s",
						index, item ? item->GetName() : "<NONE>");
		++index;
	}
}

ACMD(do_gift)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "gift");
}

ACMD(do_cube)
{
	if (!ch->CanDoCube())
		return;

	dev_log(LOG_DEB0, "CUBE COMMAND <%s>: %s", ch->GetName(), argument);
	int32_t cube_index = 0, inven_index = 0;
	const char *line;

	char arg1[256], arg2[256], arg3[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Usage: cube open");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube close");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube add <inveltory_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube delete <cube_index>");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube list");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube cancel");
		ch->ChatPacket(CHAT_TYPE_INFO, "       cube make [all]");
		return;
	}

	const std::string& strArg1 = std::string(arg1);

	if (strArg1 == "r_info")
	{
		if (0 == arg2[0])
			Cube_request_result_list(ch);
		else
		{
			if (isdigit(*arg2))
			{
				int32_t listIndex = 0, requestCount = 1;
				str_to_number(listIndex, arg2);

				if (0 != arg3[0] && isdigit(*arg3))
					str_to_number(requestCount, arg3);

				Cube_request_material_info(ch, listIndex, requestCount);
			}
		}

		return;
	}

	switch (LOWER(arg1[0]))
	{
		case 'o':
			Cube_open(ch);
			break;

		case 'c':
			Cube_close(ch);
			break;

		case 'l':
			Cube_show_list(ch);
			break;

		case 'a':
			{
				if (0 == arg2[0] || !isdigit(*arg2) ||
					0 == arg3[0] || !isdigit(*arg3))
					return;

				str_to_number(cube_index, arg2);
				str_to_number(inven_index, arg3);
				Cube_add_item (ch, cube_index, inven_index);
			}
			break;

		case 'd':
			{
				if (0 == arg2[0] || !isdigit(*arg2))
					return;

				str_to_number(cube_index, arg2);
				Cube_delete_item (ch, cube_index);
			}
			break;

		case 'm':
			if (0 != arg2[0])
			{
				while (true == Cube_make(ch))
					dev_log (LOG_DEB0, "cube make success");
			}
			else
				Cube_make(ch);
			break;

		default:
			return;
	}
}

ACMD(do_in_game_mall)
{
	// @correction005
	char buf[512+1];
	char sas[33];
	MD5_CTX ctx;
	const char sas_key[] = "GF9001";

	snprintf(buf, sizeof(buf), "%u%u%s", ch->GetPlayerID(), ch->GetAID(), sas_key);

	MD5Init(&ctx);
	MD5Update(&ctx, (const unsigned char *) buf, strlen(buf));
// #ifdef __FreeBSD__
	MD5End(&ctx, sas);
// #else
	// static const char hex[] = "0123456789abcdef";
	// unsigned char digest[16];
	// MD5Final(digest, &ctx);
	// int32_t i;
	// for (i = 0; i < 16; ++i) {
		// sas[i+i] = hex[digest[i] >> 4];
		// sas[i+i+1] = hex[digest[i] & 0x0f];
	// }
	// sas[i+i] = '\0';
// #endif

	snprintf(buf, sizeof(buf), "mall http://%s/ishop?pid=%u&c=tr&sid=0&sas=%s", g_strWebMallURL.c_str(), ch->GetPlayerID(), sas);
	ch->ChatPacket(CHAT_TYPE_COMMAND, buf);
}

ACMD(do_dice)
{
	char arg1[256], arg2[256];
	int32_t start = 1, end = 100;

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (*arg1 && *arg2)
	{
		start = atoi(arg1);
		end = atoi(arg2);
	}
	else if (*arg1 && !*arg2)
	{
		start = 1;
		end = atoi(arg1);
	}

	end = MAX(start, end);
	start = MIN(start, end);

	int32_t n = number(start, end);

	if (ch->GetParty())
		ch->GetParty()->ChatPacketToAllMember(CHAT_TYPE_INFO, LC_TEXT("%s´ÔÀÌ ÁÖ»çÀ§¸¦ ±¼·Á %d°¡ ³ª¿Ô½À´Ï´Ù. (%d-%d)"), ch->GetName(), n, start, end);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ç½ÅÀÌ ÁÖ»çÀ§¸¦ ±¼·Á %d°¡ ³ª¿Ô½À´Ï´Ù. (%d-%d)"), n, start, end);
}

#ifdef ENABLE_NEWSTUFF //@correction232
// @correction003 BEGIN
ACMD(do_click_safebox)
{
	if (ch->GetDungeon() || ch->GetWarMap())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot open the safebox in dungeon or at war."));
		return;
	}

	ch->SetSafeboxOpenPosition();
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeSafeboxPassword");
}
// @correction003 END

ACMD(do_force_logout)
{
	LPDESC pDesc=DESC_MANAGER::instance().FindByCharacterName(ch->GetName());
	if (!pDesc)
		return;
	pDesc->DelayedDisconnect(0);
}
#endif

ACMD(do_click_mall)
{
	ch->ChatPacket(CHAT_TYPE_COMMAND, "ShowMeMallPassword");
}

ACMD(do_ride)
{
    dev_log(LOG_DEB0, "[DO_RIDE] start");
    if (ch->IsDead() || ch->IsStun())
	return;

#ifdef __MOUNT__
	if (ch->GetMountingVnumM() && ch->GetWear(WEAR_COSTUME_MOUNT))
	{
		CMountSystem* MountSystem = ch->GetMountSystem();
		if (MountSystem)
		{
			bool MState = MountSystem->IsMounting(ch->GetMountingVnumM());
			if (MState)
			{
				MountSystem->Unmount(ch->GetMountingVnumM());
				return;
			}
			else
			{
				do_unmount(ch, nullptr, 0, 0);
				if (ch->IsHorseRiding())
					ch->StopRiding();
				MountSystem->Mount(ch->GetMountingVnumM());
				return;
			}
		}
	}
#endif

    {
	if (ch->IsHorseRiding())
	{
	    dev_log(LOG_DEB0, "[DO_RIDE] stop riding");
	    ch->StopRiding();
	    return;
	}

	if (ch->GetMountVnum())
	{
	    dev_log(LOG_DEB0, "[DO_RIDE] unmount");
	    do_unmount(ch, nullptr, 0, 0);
	    return;
	}
    }

    {
	if (ch->GetHorse() != nullptr)
	{
	    dev_log(LOG_DEB0, "[DO_RIDE] start riding");
	    ch->StartRiding();
	    return;
	}

	for (uint16_t i=0; i<INVENTORY_MAX_NUM; ++i)
	{
	    LPITEM item = ch->GetInventoryItem(i);
	    if (!item)
			continue;

		if (item->IsRideItem())
		{
			if (nullptr==ch->GetWear(WEAR_UNIQUE1) || nullptr==ch->GetWear(WEAR_UNIQUE2)
#if defined(__COSTUME_MOUNT__) && !defined(__MOUNT__)
|| nullptr == ch->GetWear(WEAR_COSTUME_MOUNT)
#endif
				)
			{
				dev_log(LOG_DEB0, "[DO_RIDE] USE UNIQUE ITEM");
				ch->UseItem(TItemPos (INVENTORY, i));
				return;
			}
		}

	    switch (item->GetVnum())
	    {
		case 71114:
		case 71116:
		case 71118:
		case 71120:
		    dev_log(LOG_DEB0, "[DO_RIDE] USE QUEST ITEM");
		    ch->UseItem(TItemPos (INVENTORY, i));
		    return;
	    }

		if( (item->GetVnum() > 52000) && (item->GetVnum() < 52091) )	{
			dev_log(LOG_DEB0, "[DO_RIDE] USE QUEST ITEM");
			ch->UseItem(TItemPos (INVENTORY, i));
		    return;
		}
	}
    }


    ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» ¸ÕÀú ¼ÒÈ¯ÇØÁÖ¼¼¿ä."));
}

#ifdef __AUCTION__
ACMD(do_get_item_id_list)
{
	for (int32_t i = 0; i < INVENTORY_MAX_NUM; i++)
	{
		LPITEM item = ch->GetInventoryItem(i);
		if (item != nullptr)
			ch->ChatPacket(CHAT_TYPE_INFO, "name : %s id : %d", item->GetProto()->szName, item->GetID());
	}
}


ACMD(do_enroll_auction)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	char arg4[256];
	two_arguments (two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3), arg4, sizeof(arg4));

	uint32_t item_id = strtoul(arg1, nullptr, 10);
	uint8_t empire = strtoul(arg2, nullptr, 10);
	int32_t bidPrice = strtol(arg3, nullptr, 10);
	int32_t immidiatePurchasePrice = strtol(arg4, nullptr, 10);

	LPITEM item = ITEM_MANAGER::instance().Find(item_id);
	if (item == nullptr)
		return;

	AuctionManager::instance().enroll_auction(ch, item, empire, bidPrice, immidiatePurchasePrice);
}

ACMD(do_enroll_wish)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	one_argument (two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));

	uint32_t item_num = strtoul(arg1, nullptr, 10);
	uint8_t empire = strtoul(arg2, nullptr, 10);
	int32_t wishPrice = strtol(arg3, nullptr, 10);

	AuctionManager::instance().enroll_wish(ch, item_num, empire, wishPrice);
}

ACMD(do_enroll_sale)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	one_argument (two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2)), arg3, sizeof(arg3));

	uint32_t item_id = strtoul(arg1, nullptr, 10);
	uint32_t wisher_id = strtoul(arg2, nullptr, 10);
	int32_t salePrice = strtol(arg3, nullptr, 10);

	LPITEM item = ITEM_MANAGER::instance().Find(item_id);
	if (item == nullptr)
		return;

	AuctionManager::instance().enroll_sale(ch, item, wisher_id, salePrice);
}

ACMD(do_get_auction_list)
{
	char arg1[256];
	char arg2[256];
	char arg3[256];
	two_arguments (one_argument (argument, arg1, sizeof(arg1)), arg2, sizeof(arg2), arg3, sizeof(arg3));

	AuctionManager::instance().get_auction_list (ch, strtoul(arg1, nullptr, 10), strtoul(arg2, nullptr, 10), strtoul(arg3, nullptr, 10));
}
ACMD (do_get_my_auction_list)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().get_my_auction_list (ch, strtoul(arg1, nullptr, 10), strtoul(arg2, nullptr, 10));
}

ACMD (do_get_my_purchase_list)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().get_my_purchase_list (ch, strtoul(arg1, nullptr, 10), strtoul(arg2, nullptr, 10));
}

ACMD (do_auction_bid)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().bid (ch, strtoul(arg1, nullptr, 10), strtoul(arg2, nullptr, 10));
}

ACMD (do_auction_impur)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().immediate_purchase (ch, strtoul(arg1, nullptr, 10));
}

ACMD (do_get_auctioned_item)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().get_auctioned_item (ch, strtoul(arg1, nullptr, 10), strtoul(arg2, nullptr, 10));
}

ACMD (do_buy_sold_item)
{
	char arg1[256];
	char arg2[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().get_auctioned_item (ch, strtoul(arg1, nullptr, 10), strtoul(arg2, nullptr, 10));
}

ACMD (do_cancel_auction)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().cancel_auction (ch, strtoul(arg1, nullptr, 10));
}

ACMD (do_cancel_wish)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().cancel_wish (ch, strtoul(arg1, nullptr, 10));
}

ACMD (do_cancel_sale)
{
	char arg1[256];
	one_argument (argument, arg1, sizeof(arg1));

	AuctionManager::instance().cancel_sale (ch, strtoul(arg1, nullptr, 10));
}

ACMD (do_rebid)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().rebid (ch, strtoul(arg1, nullptr, 10), strtoul(arg2, nullptr, 10));
}

ACMD (do_bid_cancel)
{
	char arg1[256];
	char arg2[256];
	two_arguments (argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	AuctionManager::instance().bid_cancel (ch, strtoul(arg1, nullptr, 10));
}
#endif

#ifdef WJ_ELDER_ATTRIBUTE_SYSTEM // @correction193
ACMD(do_add_new_attribute)
{
	if (quest::CQuestManager::instance().GetEventFlag("WJ_ELDER_ATTRIBUTE_SYSTEM") == 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
		return;
	}

	std::vector<std::string> vecArgs;
	split_argument(argument, vecArgs);
	
	if (vecArgs.size() < 7)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_UNKNOWN_ARGUMENT"));
		return;
	}
	
	int32_t sourceItemPos = 0, targetItemPos = 0, attrtype0 = 0, attrtype1 = 0, attrtype2 = 0, attrtype3 = 0, attrtype4 = 0;
		
	str_to_number(sourceItemPos, vecArgs[1].c_str());
	str_to_number(targetItemPos, vecArgs[2].c_str());
	str_to_number(attrtype0, vecArgs[3].c_str());
	str_to_number(attrtype1, vecArgs[4].c_str());
	str_to_number(attrtype2, vecArgs[5].c_str());
	str_to_number(attrtype3, vecArgs[6].c_str());
	str_to_number(attrtype4, vecArgs[7].c_str());
	
	LPITEM item = ch->GetInventoryItem(targetItemPos);
	
	if (!item)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_CANNOT_FIND_ITEM"));
		return;
	}
	
	if (item->GetType() != ITEM_WEAPON && item->GetType() != ITEM_ARMOR)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_COSTUME"));
		return;
	}
	
	if (item->IsEquipped())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_EQUIPPED"));
		return;
	}
	
	if (item->IsExchanging())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_EXCHANGING"));
		return;
	}

	if (ch->CountSpecifyItem(71051) < 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_ENOUGH_NEW_ATTRIBUTE"));
		return;
	}
	
	TItemTable const * pProto = item->GetProto();
	
	bool addontype = false;
	if (pProto && pProto->sAddonType)
		addontype = true;
	
	int32_t attrtype0prob = number(5,5), attrtype1prob = number(5,5), attrtype2prob = number(5,5), attrtype3prob = number(5,5), attrtype4prob = number(5,5);
	
	bool error = false;
	if (addontype == false)
	{
		if (attrtype0 == attrtype1 || attrtype0 == attrtype2 || attrtype0 == attrtype3 || attrtype0 == attrtype4)
			error = true;
		if (attrtype1 == attrtype2 || attrtype1 == attrtype3 || attrtype1 == attrtype4)
			error = true;
		if (attrtype2 == attrtype3 || attrtype2 == attrtype4)
			error = true;
		if (attrtype3 == attrtype4)
			error = true;
	}
	else
	{
		if (attrtype0 == attrtype1 || attrtype0 == attrtype4)
			error = true;
		if (attrtype1 == attrtype4)
			error = true;
	}

	if (error)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_ERROR"));
		return;
	}
	
	if (addontype == false)
	{
		item->RemoveAttributeAt(0);
		item->RemoveAttributeAt(1);
		item->RemoveAttributeAt(2);
		item->RemoveAttributeAt(3);	
		item->RemoveAttributeAt(4);	
		item->AddNewAttr(0, attrtype0, attrtype0prob);
		item->AddNewAttr(1, attrtype1, attrtype1prob);
		item->AddNewAttr(2, attrtype2, attrtype2prob);
		item->AddNewAttr(3, attrtype3, attrtype3prob);
		item->AddNewAttr(4, attrtype4, attrtype4prob);
	}
	else
	{
		item->RemoveAttributeAt(2);
		item->RemoveAttributeAt(3);	
		item->RemoveAttributeAt(4);	
		item->AddNewAttr(2, attrtype0, attrtype0prob);
		item->AddNewAttr(3, attrtype1, attrtype1prob);
		item->AddNewAttr(4, attrtype2, attrtype2prob);				
	}
	
	ch->RemoveSpecifyItem(71051, 1);
	for (int32_t i = 0; i < 5; ++i)
	{
		for (int32_t a = 0; a < 5; ++a)
		{
			if (item->GetAttributeType(a) == item->GetAttributeType(i) && a != i)
			{
				ITEM_MANAGER::instance().RemoveItem(item);
				ch->ChatPacket(CHAT_TYPE_INFO, "Bug Yapma Kocm");
				return;
			}
		}
	}
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_SUCCESS_ATTR"));
	
}
#endif

#ifdef ENABLE_FAST_NPC // @correction194
#include "shop.h"
#include "shop_manager.h"
ACMD(do_npc_open)
{
	if (ch->IsDead() || ch->GetExchange() || ch->GetMyShop() || ch->IsOpenSafebox() || ch->IsCubeOpen() || ch->isChangeLookOpened())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´Ù¸¥ °Å·¡Ã¢ÀÌ ¿­¸°»óÅÂ¿¡¼­´Â »óÁ¡°Å·¡¸¦ ÇÒ¼ö °¡ ¾ø½À´Ï´Ù."));
		return;
	}

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint16_t vnum;
	str_to_number(vnum, arg1);
	
	LPSHOP shop = CShopManager::instance().Get(vnum);
	if (!shop)
		return;

	ch->SetShopOwner(ch);
	shop->AddGuest(ch, 0, false);
}
#endif

#ifdef __OFFLINE_PRIVATE_SHOP__
ACMD(do_click_offline_private_shop)
{
	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen() || ch->isChangeLookOpened() || ch->isAcceOpened(true) || ch->isAcceOpened(false))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´Ù¸¥ °Å·¡Áß(Ã¢°í,±³È¯,»óÁ¡)¿¡´Â °³ÀÎ»óÁ¡À» »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (!ch)
		return;

	char arg1[256];
	argument = one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint8_t bNumber = 0;
	str_to_number(bNumber, arg1);

	switch(bNumber)
	{
		case 0:
		{
			if (quest::CQuestManager::instance().GetEventFlag("DISABLE_OPS0") == 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
				return;
			}

			uint32_t dwPID = ch->GetPlayerID();
			const uint8_t bCount = CShopManager::instance().GetOfflinePrivateShopCount(dwPID);

			if (bCount)
			{
				LPOPSHOP pkOfflinePrivateShops[bCount];
				CShopManager::instance().SetOfflinePrivateShops(pkOfflinePrivateShops, bCount, dwPID);
				TOfflinePrivateShopInfo pkTOfflinePrivateShopInfo[bCount];
				// memset(&pkTOfflinePrivateShopInfo, 0, sizeof(pkTOfflinePrivateShopInfo));
				for (uint8_t i = 0; i < bCount; ++i)
				{
					pkOfflinePrivateShops[i]->GetInfo(&pkTOfflinePrivateShopInfo[i]);
					TPacketGCOfflinePrivateShopInfo packet;
					packet.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
					packet.bSubHeader = OFFLINE_PRIVATE_SHOP_INFO;
					packet.wSize = sizeof(packet) + sizeof(TOfflinePrivateShopInfo);

					ch->GetDesc()->BufferedPacket(&packet, sizeof(packet));
					ch->GetDesc()->LargePacket(&pkTOfflinePrivateShopInfo[i], sizeof(TOfflinePrivateShopInfo));
				}
			}

			TPacketGGOfflinePrivateShopInfo p2p_packet;
			p2p_packet.bHeader = HEADER_GG_OFFLINE_PRIVATE_SHOP_INFO;
			p2p_packet.bSubHeader = OFFLINE_PRIVATE_SHOP_INFO;
			p2p_packet.dwPID = dwPID;
			P2P_MANAGER::instance().SendEmpires(&p2p_packet, sizeof(p2p_packet));
		}
		break;
		case 1:
		case 2:
		{
			if (bNumber == 1 && quest::CQuestManager::instance().GetEventFlag("DISABLE_OPS1") == 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
				return;
			}
			if (bNumber == 2 && quest::CQuestManager::instance().GetEventFlag("DISABLE_OPS2") == 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
				return;
			}
			char arg2[256], arg3[256];
			argument = two_arguments(argument, arg2, sizeof(arg2), arg3, sizeof(arg3));
			if (!*arg2 || !*arg3)
				return;
			uint32_t dwPID = ch->GetPlayerID();
			uint32_t dwShopId = 0, dwTargetPos = 0;
			str_to_number(dwShopId, arg2);
			str_to_number(dwTargetPos, arg3);
			if (dwTargetPos >= SHOP_HOST_ITEM_MAX_NUM)
				return;

			LPOPSHOP pkShop = CShopManager::instance().FindOfflinePrivateShop(dwShopId);

			if (pkShop)
			{
				LPSHOP pShop = pkShop->GetShop();
				if (!pShop)
					return;

				if (bNumber == 2)
				{
					pShop->DelOfflinePrivateShopItem(dwTargetPos, ch);
				}
				else
				{
					uint8_t bInvenType = 0;
					uint32_t dwSourcePos = 0;
					uint64_t dwPrices = 0;
					char arg4[256], arg5[256], arg6[256];
					three_arguments(argument, arg4, sizeof(arg4), arg5, sizeof(arg5), arg6, sizeof(arg6));
					str_to_number(bInvenType, arg4);
					str_to_number(dwSourcePos, arg5);
					str_to_number(dwPrices, arg6);
					TItemPos cell(INVENTORY, dwSourcePos);
					if (false == cell.IsDefaultInventoryPosition() && false == cell.IsBeltInventoryPosition())
						return;

					pShop->AddOfflinePrivateShopItem(dwTargetPos, bInvenType, dwSourcePos, dwPrices, dwShopId, ch);
				}

				TPacketGCOfflinePrivateShopInfo packet;
				packet.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
				packet.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_UPDATE;

				TOfflinePrivateShopItemUpdate update_packet;
				update_packet.dwVid = dwShopId;
				update_packet.item = pShop->GetItemVector()[dwTargetPos].item;
				update_packet.item.display_pos = dwTargetPos;
				TEMP_BUFFER buf;
				buf.write(&update_packet, sizeof(update_packet));
				packet.wSize = sizeof(packet) + buf.size();

				ch->GetDesc()->BufferedPacket(&packet, sizeof(packet));
				ch->GetDesc()->Packet(buf.read_peek(), buf.size());
			}
			else
			{
				if (bNumber == 2)
				{
					TPacketGGOfflinePrivateShopItemDel p2p_packet;
					p2p_packet.bHeader = HEADER_GG_OFFLINE_PRIVATE_SHOP_ITEM_DEL;
					p2p_packet.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_DEL;
					p2p_packet.dwPID = dwPID;
					p2p_packet.dwShopId = dwShopId;
					p2p_packet.dwTargetPos = dwTargetPos;
					P2P_MANAGER::instance().SendEmpires(&p2p_packet, sizeof(p2p_packet));
				}

				if (bNumber == 1)
				{
					TPacketGGOfflinePrivateShopItemAdd p2p_packet;
					p2p_packet.bHeader = HEADER_GG_OFFLINE_PRIVATE_SHOP_ITEM_ADD;
					p2p_packet.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_ADD;
					p2p_packet.dwPID = dwPID;
					p2p_packet.dwShopId = dwShopId;
					p2p_packet.dwTargetPos = dwTargetPos;

					char arg4[256], arg5[256], arg6[256];
					three_arguments(argument, arg4, sizeof(arg4), arg5, sizeof(arg5), arg6, sizeof(arg6));
					str_to_number(p2p_packet.bInvenType, arg4);
					str_to_number(p2p_packet.dwSourcePos, arg5);
					str_to_number(p2p_packet.dwPrices, arg6);
					TItemPos cell(INVENTORY, p2p_packet.dwSourcePos);
					if (false == cell.IsDefaultInventoryPosition() && false == cell.IsBeltInventoryPosition())
						return;

					LPITEM item = ch->GetItem(TItemPos(p2p_packet.bInvenType, p2p_packet.dwSourcePos));
					if (!item)
						return;

#ifdef __SOULBIND__
					if (item->IsSealed())
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't upgrade sealbind item."));
						return;
					}
#endif

					if (item->isLocked())
						return;

					item->Lock(true);

					TShopItemData* pData = &p2p_packet.pData;
					pData->vnum = item->GetVnum();
					pData->count = item->GetCount();
					pData->dwPrices = p2p_packet.dwPrices;
					pData->display_pos = dwTargetPos;
					memcpy(pData->alSockets, item->GetSockets(), sizeof(pData->alSockets));
					memcpy(pData->aAttr, item->GetAttributes(), sizeof(pData->aAttr));
#ifdef __CHANGE_LOOK__
					pData->changelook = item->GetChangeLook();
#endif

					p2p_packet.bSize = item->GetSize();
					P2P_MANAGER::instance().SendEmpires(&p2p_packet, sizeof(p2p_packet));
				}
			}
		}
		break;
		case 3:
		{
			if (quest::CQuestManager::instance().GetEventFlag("DISABLE_OPS3") == 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
				return;
			}
			const int32_t iPulse = thecore_pulse();
			if (iPulse - ch->GetRenamePulse()  < PASSES_PER_SEC(60))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡ ÈÄ %dÃÊ ÀÌ³»¿¡´Â ´Ù¸¥Áö¿ªÀ¸·Î ÀÌµ¿ ÇÒ ¼ö ¾ø½À´Ï´Ù."), 60);
				return;
			}

			char arg2[256], arg3[CHARACTER_NAME_MAX_LEN + 1] = {};
			argument = one_argument(argument, arg2, sizeof(arg2));
			if (!*arg2 || !argument || !*argument)
				return;

			// uint32_t dwPID = ch->GetPlayerID();
			uint32_t dwShopId = 0;
			str_to_number(dwShopId, arg2);
			memcpy(arg3, argument, CHARACTER_NAME_MAX_LEN + 1);
			LPOPSHOP pkShop = CShopManager::instance().FindOfflinePrivateShop(dwShopId);
			if (pkShop)
			{
				char queryStr[256];
				snprintf(queryStr, sizeof(queryStr), "UPDATE player.player_shop SET name = '%s' WHERE id = %u", arg3, pkShop->GetShopVid());
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(queryStr));
				pkShop->SetSign(arg3);
				pkShop->UpdateSectree();
				ch->SetRenamePulse();
			}
			else
			{
				TPacketGGOfflinePrivateShopRename p2p_packet;
				p2p_packet.bHeader = HEADER_GG_OFFLINE_PRIVATE_SHOP_RENAME;
				p2p_packet.bSubHeader = OFFLINE_PRIVATE_SHOP_RENAME;
				p2p_packet.dwShopId = dwShopId;
				strlcpy(p2p_packet.szSign, arg3, sizeof(p2p_packet.szSign));
				P2P_MANAGER::instance().SendEmpires(&p2p_packet, sizeof(p2p_packet));
				ch->SetRenamePulse();
			}
		}
		break;
		case 4:
		{
			if (quest::CQuestManager::instance().GetEventFlag("DISABLE_OPS4") == 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
				return;
			}
			quest::PC* pPC = quest::CQuestManager::instance().GetPC(ch->GetPlayerID());
			if (!pPC)
				return;

			uint32_t dwShopSearchSecCycle = 5;
			uint32_t dwNowSec = get_global_time();
			uint32_t dwLastShopSearchAttrSec = pPC->GetFlag("ShopSearch.LastShopSearchSecAttr");

			if (dwLastShopSearchAttrSec + dwShopSearchSecCycle > dwNowSec)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("PRIVATE_SHOP_SEARCH_NEED_WAIT_%d_LEFT_(%d)"), dwShopSearchSecCycle, dwShopSearchSecCycle - (dwNowSec - dwLastShopSearchAttrSec));
				return;
			}
			pPC->SetFlag("ShopSearch.LastShopSearchSecAttr", dwNowSec);

			std::vector<std::string> vecArgs;
			split_argument(argument, vecArgs);

			if (vecArgs.size() < 10)
				return;

			uint64_t dwArgs[9] = { };
			for (int32_t i = 0; i < 9; ++i)
				str_to_number(dwArgs[i], vecArgs[i].c_str());

			std::string& itemName = vecArgs[9];
			std::replace(itemName.begin(), itemName.end(), '_', ' ');
			uint32_t dwPID = ch->GetPlayerID();
			std::vector<CShop::SHOP_ITEM> sendItems = {};
			CShopManager::instance().SearchShops(sendItems, dwArgs, sizeof(dwArgs) / sizeof(dwArgs[0]), itemName.c_str(), dwPID);
			size_t size = sendItems.size();
			// if (size > 1000)
				// size = 1000;

			for (size_t i = 0; i < size; ++i)
			{
				const CShop::SHOP_ITEM * item = &sendItems[i];
				LPITEM pkItem = item->pkItem;
				
				if (pkItem)
				{	
					LPDESC d = ch->GetDesc();

					if (!d)
						return;

					TEMP_BUFFER buf;
					TPacketGCOfflinePrivateShopInfo pack;
					TOfflinePrivateShopItemSearch data;
					data.dwVid = pkItem->GetOfflinePrivateShopVid();
					strlcpy(data.szSellerName, pkItem->GetOwnerName(), sizeof(data.szSellerName));
					data.item = item->item;
					pack.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
					pack.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_SEARCH;
					pack.wSize = sizeof(pack) + sizeof(data);
					buf.write(&pack, sizeof(pack));
					buf.write(&data, sizeof(data));
					d->LargePacket(buf.read_peek(), buf.size());
				}
			}

			TPacketGGOfflinePrivateShopSearch p2p_packet;
			p2p_packet.bHeader = HEADER_GG_OFFLINE_PRIVATE_SHOP_ITEM_SEARCH;
			p2p_packet.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_SEARCH;
			p2p_packet.dwPID = dwPID;
			memcpy(p2p_packet.dwArgs, dwArgs, sizeof(p2p_packet.dwArgs));
			strlcpy(p2p_packet.szItemName, itemName.c_str(), sizeof(p2p_packet.szItemName));
			P2P_MANAGER::instance().SendEmpires(&p2p_packet, sizeof(p2p_packet));
		}
		break;
		case 5:
		{
			if (quest::CQuestManager::instance().GetEventFlag("DISABLE_OPS5") == 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
				return;
			}
			char arg2[256], arg3[256];
			two_arguments(argument, arg2, sizeof(arg2), arg3, sizeof(arg3));
			if (!*arg2 || !*arg3)
				return;

			uint32_t dwPID = ch->GetPlayerID();
			uint64_t dwPrices = ch->GetGold();
			uint32_t dwShopId = 0, dwTargetPos;
			str_to_number(dwShopId, arg2);
			str_to_number(dwTargetPos, arg3);
			LPOPSHOP pkOPShop = CShopManager::instance().FindOfflinePrivateShop(dwShopId);

			if (!pkOPShop)
			{
				TPacketGGOfflinePrivateShopBuy p2p_packet;
				p2p_packet.bHeader = HEADER_GG_OFFLINE_PRIVATE_SHOP_ITEM_BUY;
				p2p_packet.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_BUY;
				p2p_packet.dwPID = dwPID;
				p2p_packet.dwShopId = dwShopId;
				p2p_packet.dwPrices = dwPrices;
				p2p_packet.dwTargetPos = dwTargetPos;
				P2P_MANAGER::instance().SendEmpires(&p2p_packet, sizeof(p2p_packet));
				return;
			}

			LPSHOP pkShop = pkOPShop->GetShop();
			if (!pkShop)
				return;

			int32_t ret = pkShop->Buy(ch, dwTargetPos);
			if (SHOP_SUBHEADER_GC_OK == ret)
			{
				if (ch->GetDesc())
				{
					TEMP_BUFFER buf;
					TPacketGCOfflinePrivateShopInfo pack;
					pack.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
					pack.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_BUY;
					pack.wSize = sizeof(pack) + sizeof(dwShopId) + sizeof(dwTargetPos);
					buf.write(&pack, sizeof(pack));
					buf.write(&dwShopId, sizeof(dwShopId));
					buf.write(&dwTargetPos, sizeof(dwTargetPos));
					ch->GetDesc()->Packet(buf.read_peek(), buf.size());
				}
			}
			else
			{
				TPacketGCShop pack;
				pack.header	= HEADER_GC_SHOP;
				pack.subheader = ret;
				pack.size = sizeof(TPacketGCShop);
				ch->GetDesc()->Packet(&pack, sizeof(pack));
			}
		}
		break;
		case 6:
		{
			if (quest::CQuestManager::instance().GetEventFlag("DISABLE_OPS6") == 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
				return;
			}
			char arg2[256];
			one_argument(argument, arg2, sizeof(arg2));
			if (!*arg2)
				return;

			uint32_t dwShopId = 0;
			str_to_number(dwShopId, arg2);
			uint32_t dwPID = ch->GetPlayerID();

			if (!ch->CountSpecifyItem(72319))
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Yeterli Kilit Acma Bulunmamakta !");
				return;
			}

			ch->RemoveSpecifyItem(72319, 1);

			LPOPSHOP pkOPShop = CShopManager::instance().FindOfflinePrivateShop(dwShopId);
			if (pkOPShop)
			{
				uint8_t& bUnlockSlots = pkOPShop->GetShop()->GetUnlockSlots();
				bUnlockSlots += 1;
				char queryStr[256];
				snprintf(queryStr, sizeof(queryStr), "UPDATE player.player_shop SET unlock_slots = %u WHERE id = %u", bUnlockSlots, pkOPShop->GetShopVid());
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(queryStr));

				if (ch->GetDesc())
				{
					TEMP_BUFFER buf;
					TPacketGCOfflinePrivateShopInfo packet;
					packet.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
					packet.bSubHeader = OFFLINE_PRIVATE_SHOP_UNLOCK_SLOTS;
					packet.wSize = sizeof(packet) + (sizeof(dwShopId)) + (sizeof(bUnlockSlots));
					buf.write(&packet, sizeof(packet));
					buf.write(&dwShopId, sizeof(dwShopId));
					buf.write(&bUnlockSlots, sizeof(bUnlockSlots));

					ch->GetDesc()->Packet(buf.read_peek(), buf.size());
				}
			}
			else
			{
				TPacketGGOfflinePrivateShopUnlockSlots p2p_packet;
				p2p_packet.bHeader = HEADER_GG_OFFLINE_PRIVATE_SHOP_UNLOCK_SLOTS;
				p2p_packet.bSubHeader = OFFLINE_PRIVATE_SHOP_UNLOCK_SLOTS;
				p2p_packet.dwShopId = dwShopId;
				p2p_packet.dwPID = dwPID;
				P2P_MANAGER::instance().SendEmpires(&p2p_packet, sizeof(p2p_packet));
			}
		}
		break;
		case 7:
		{
			if (quest::CQuestManager::instance().GetEventFlag("DISABLE_OPS7") == 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
				return;
			}
			/*uint32_t dwPID = ch->GetPlayerID();
			uint32_t dwTime = time(nullptr) - 604800;
			std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT price, vnum, UNIX_TIMESTAMP(date) FROM player.player_shop_logs WHERE id = %u AND UNIX_TIMESTAMP(date) > %u", dwPID, dwTime));
			SQLResult* pRes = pMsg->Get();
			MYSQL_ROW row;
			if (!pMsg || !pRes || !pRes->uiNumRows)
				return;

			while ((row = mysql_fetch_row(pRes->pSQLResult)))
			{
				TOfflinePrivateShopLog tLogData;
				str_to_number(tLogData.dwPrice, row[0]);
				str_to_number(tLogData.dwVnum, row[1]);
				str_to_number(tLogData.dwDate, row[2]);
				if (ch->GetDesc())
				{
					TEMP_BUFFER buf;
					TPacketGCOfflinePrivateShopInfo packet;
					packet.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
					packet.bSubHeader = OFFLINE_PRIVATE_SHOP_LOGS;
					packet.wSize = sizeof(packet) + sizeof(tLogData);
					buf.write(&packet, sizeof(packet));
					buf.write(&tLogData, sizeof(tLogData));
					ch->GetDesc()->Packet(buf.read_peek(), buf.size());
				}
			}*/
		};
		break;
		case 8:
		{
			if (quest::CQuestManager::instance().GetEventFlag("DISABLE_OPS8") == 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
				return;
			}
			char arg2[256], arg3[256];
			two_arguments(argument, arg2, sizeof(arg2), arg3, sizeof(arg3));
			if (!*arg2 || !*arg3)
				return;

			uint32_t dwShopId = 0;
			uint64_t dwPrices = 0;
			uint64_t dwYang = 0;
			uint32_t dwPID = ch->GetPlayerID();
			str_to_number(dwShopId, arg2);
			str_to_number(dwYang, arg3);

			LPOPSHOP pkOPShop = CShopManager::instance().FindOfflinePrivateShop(dwShopId);

			if (pkOPShop)
				dwPrices = pkOPShop->GetPrices();
			else
			{
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT prices FROM player.player_shop WHERE id = %u and player_id = %u", dwShopId, dwPID));
				SQLResult * pRes = pMsg->Get();
				if (!pMsg || !pRes || !pRes->uiNumRows)
					return;

				MYSQL_ROW row = mysql_fetch_row(pRes->pSQLResult);
				str_to_number(dwPrices, row[0]);
			}

			if (!dwPrices)
				break;

			if (dwYang >= dwPrices)
			{
				dwYang = dwPrices;
				dwPrices = 0;
			}
			else
			{
				dwPrices -= dwYang;
			}

			ch->ChangeGold(true, dwYang, true);
			ch->SaveAndFlush();
			char queryStr[256];
			snprintf(queryStr, sizeof(queryStr), "UPDATE player.player_shop SET prices = %llu WHERE id = %u and player_id = %u", dwPrices, dwShopId, dwPID);
			std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(queryStr));

			if (ch->GetDesc())
			{
				TEMP_BUFFER buf;
				TPacketGCOfflinePrivateShopInfo packet;
				packet.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
				packet.bSubHeader = OFFLINE_PRIVATE_SHOP_PRICE_PULL;
				packet.wSize = sizeof(packet) + (sizeof(dwShopId)) + (sizeof(dwPrices));
				buf.write(&packet, sizeof(packet));
				buf.write(&dwShopId, sizeof(dwShopId));
				buf.write(&dwPrices, sizeof(dwPrices));

				ch->GetDesc()->Packet(buf.read_peek(), buf.size());
			}
		}
		break;
		default:
			break;
	}
}

ACMD(do_price_check)
{
	auto& vecChecks = CShopManager::instance().GetPriceChecks();
	for (auto& it : vecChecks)
		ch->ChatPacket(CHAT_TYPE_COMMAND, "PriceCheck %u %llu %llu", it.vnum, it.min, it.max);
}
#endif

#ifdef __MOVE_CHANNEL__
int32_t new_ch;

EVENTFUNC(channel_event)
{
	TimedEventInfo* info = dynamic_cast<TimedEventInfo*>(event->info);

	if (info == nullptr)
	{
		sys_err("timed_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == nullptr) { // <Factor>
		return 0;
	}

	if (info->left_second <= 0)
	{
		ch->m_pkTimedEvent = nullptr;

		TPacketNeedLoginLogInfo acc_info;
		acc_info.dwPlayerID = ch->GetDesc()->GetAccountTable().id;

		db_clientdesc->DBPacket(HEADER_GD_VALID_LOGOUT, 0, &acc_info, sizeof(acc_info));

		LogManager::instance().DetailLoginLog(false, ch);

		ch->ChannelSwitch(new_ch);

		return 0;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÃÊ ³²¾Ò½À´Ï´Ù."), info->left_second);
		--info->left_second;
	}

	return PASSES_PER_SEC(1);
}

ACMD(do_move_channel)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	str_to_number(new_ch, arg1);

	if (new_ch < 1 || new_ch > 4)
		return;

	if (new_ch == g_bChannel)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Channel_Move_ch"));
		return;
	}

	if (!ch->IsPC())
		return;

	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Channel_Move"));

	int32_t nExitLimitTime = 10;

	if (ch->IsHack(false, true, nExitLimitTime) && (!ch->GetWarMap() || ch->GetWarMap()->GetType() == GUILD_WAR_TYPE_FLAG))
		return;

	TimedEventInfo* info = AllocEventInfo<TimedEventInfo>();

	{
		if (ch->IsPosition(POS_FIGHTING))
			info->left_second = 10;
		else
			info->left_second = 3;
	}

	info->ch = ch;
	info->subcmd = subcmd;
	strlcpy(info->szReason, argument, sizeof(info->szReason));

	ch->m_pkTimedEvent = event_create(channel_event, info, 1);
}
#endif

#ifdef __SUPPORT__
ACMD(do_support_system)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	uint16_t wCell = 0;
	str_to_number(wCell, arg1);
	if (ch->GetSupportSystem()->IsActiveSupport())
	{
		LPITEM item = ch->GetInventoryItem(wCell);
		LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(ch->GetSupportSystem()->GetActiveSupport()->GetSummonItemVID());
		if (item)
		{
			if (item->GetType() == ITEM_COSTUME || item->GetType() == ITEM_ARMOR)
			{

				if (item->GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> sadece ?aman z©¥rh©¥ giydirebilirsin..");
					return;
				}

				if (item->IsExchanging())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> ticaret yaparken bu i?lemi yapamazs©¥n");
					return;
				}
				if (item->GetSubType() == COSTUME_BODY)
				{
					item->ClearAttribute();
					pSummonItem->SetSocket(4, item->GetVnum());
					ch->ChatPacket(CHAT_TYPE_COMMAND, "SupportArmor %u", item->GetVnum());
					ch->UpdatePacket();
					item->RemoveFromCharacter();
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu i?lemi yapamazs©¥n");
					return;
				}
			}
		}
		ch->GetSupportSystem()->GetActiveSupport()->RefreshCostume();
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu i?lemi kullanabilmek icin yard©¥mc©¥ ?aman muhuru gerekiyor.");
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> e?er ?aman muhuru varsa lutfen ?aman©¥ ca?©¥r©¥n.");
	}
}
ACMD(do_support_system_w)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	uint16_t wCell = 0;
	str_to_number(wCell, arg1);
	if (ch->GetSupportSystem()->IsActiveSupport())
	{
		LPITEM item = ch->GetInventoryItem(wCell);
		LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(ch->GetSupportSystem()->GetActiveSupport()->GetSummonItemVID());
		if (item)
		{
			if (item->GetType() == ITEM_WEAPON)
			{
				if (item->IsExchanging())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> ticaret yaparken bu i?lemi yapamazs©¥n");
					return;
				}
				if (item->GetSubType() == WEAPON_FAN || item->GetSubType() == WEAPON_BELL)
				{
					item->ClearAttribute();
					pSummonItem->SetSocket(5, item->GetVnum());
					ch->ChatPacket(CHAT_TYPE_COMMAND, "SupportWeapon %u", item->GetVnum());
					ch->UpdatePacket();
					item->RemoveFromCharacter();
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu i?lemi yapamazs©¥n");
					return;
				}
			}

		}
		ch->GetSupportSystem()->GetActiveSupport()->RefreshCostume();
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu i?lemi kullanabilmek icin yard©¥mc©¥ ?aman muhuru gerekiyor.");
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> e?er ?aman muhuru varsa lutfen ?aman©¥ ca?©¥r©¥n.");
	}
}
ACMD(do_support_system_d)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	uint16_t wCell = 0;
	str_to_number(wCell, arg1);
	if (ch->GetSupportSystem()->IsActiveSupport())
	{
		LPITEM item = ch->GetInventoryItem(wCell);
		LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(ch->GetSupportSystem()->GetActiveSupport()->GetSummonItemVID());
		if (item)
		{
			if (item->GetType() == ITEM_COSTUME)
			{
				if (item->IsExchanging())
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> ticaret yaparken bu i?lemi yapamazs©¥n");
					return;
				}
				if (item->GetSubType() == COSTUME_HAIR)
				{
					item->ClearAttribute();
					pSummonItem->SetForceAttribute(0, 1, item->GetValue(3));
					pSummonItem->SetForceAttribute(1, 1, item->GetVnum());
					ch->ChatPacket(CHAT_TYPE_COMMAND, "SupportHair %u", item->GetVnum());
					ch->UpdatePacket();
					item->RemoveFromCharacter();
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu i?lemi yapamazs©¥n");
					return;
				}
			}

		}
		ch->GetSupportSystem()->GetActiveSupport()->RefreshCostume();
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> Bu i?lemi kullanabilmek icin yard©¥mc©¥ ?aman muhuru gerekiyor.");
		ch->ChatPacket(CHAT_TYPE_INFO, "<Sistem> e?er ?aman muhuru varsa lutfen ?aman©¥ ca?©¥r©¥n.");
	}
}
#endif


#ifdef __GROWTH_PET__
ACMD(do_CubePetAdd)
{

	int32_t pos = 0;
	int32_t invpos = 0;

	const char* line;
	char arg1[256], arg2[256], arg3[256];

	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (0 == arg1[0])
		return;
	switch (LOWER(arg1[0]))
	{
	case 'a':	// add cue_index inven_index
	{
		if (0 == arg2[0] || !isdigit(*arg2) ||
			0 == arg3[0] || !isdigit(*arg3))
			return;

		str_to_number(pos, arg2);
		str_to_number(invpos, arg3);

	}
	break;

	default:
		return;
	}

	if (ch->GetNewPetSystem()->IsActivePet())
		ch->GetNewPetSystem()->SetItemCube(pos, invpos);
	else
		return;

}

ACMD(do_PetSkill) {
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	int32_t skillslot = 0;
	str_to_number(skillslot, arg1);
	if (skillslot > 2 || skillslot < 0)
		return;

	if (ch->GetNewPetSystem()->IsActivePet())
		ch->GetNewPetSystem()->DoPetSkill(skillslot);
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Devam etmek icin Pet'i cagirmis olmaniz gerekir.");
}

ACMD(do_FeedCubePet) {
	// char arg1[256];
	// one_argument(argument, arg1, sizeof(arg1));
	// if (!*arg1)
		// return;

	// uint32_t feedtype = 0;
	// str_to_number(feedtype, arg1);
	// if (ch->GetNewPetSystem()->IsActivePet())
		// ch->GetNewPetSystem()->ItemCubeFeed(feedtype);
	// else
		// ch->ChatPacket(CHAT_TYPE_INFO, "Lutfen once Petinizi cagiriniz!");
}

ACMD(do_PetEvo) {

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen()) {
		ch->ChatPacket(CHAT_TYPE_INFO, "[Pet Gelistirme] Evcil hayvaniniz daha fazla gelistirilemez.");
		return;
	}
	if (ch->GetNewPetSystem()->IsActivePet()) {

		int32_t it[3][1] = {
						{ 55003}, //Here Modify Items to request for 1 evo
						{ 55004 }, //Here Modify Items to request for 2 evo
						{ 55005 }  //Here Modify Items to request for 3 evo
		};
		int32_t ic[3][1] = {
						{ 10 },
						{ 20 },
						{ 30 }
		};
		int32_t tmpevo = ch->GetNewPetSystem()->GetEvolution();
		
		if (tmpevo > 2)
			return;

		// if (tmpevo == 2 && ch->GetNewPetSystem()->GetAge() < 30 && ch->GetNewPetSystem()->GetLevel() == 80)
			// return;

		if ((ch->GetNewPetSystem()->GetLevel() == 40 && tmpevo == 0) ||
			(ch->GetNewPetSystem()->GetLevel() == 60 && tmpevo == 1) ||
			(ch->GetNewPetSystem()->GetLevel() == 80 && tmpevo == 2))
		{
			for (int32_t b = 0; b < 1; b++)
			{
				if (ch->CountSpecifyItem(it[tmpevo][b]) < ic[tmpevo][b])
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "[Pet Gelistirme] Gereken itemler:");
					for (int32_t c = 0; c < 1; c++)
					{
						uint32_t vnum = it[tmpevo][c];
						ch->ChatPacket(CHAT_TYPE_INFO, "%s X%d", ITEM_MANAGER::instance().GetTable(vnum)->szLocaleName, ic[tmpevo][c]);
					}
					return;
				}
			}
			for (int32_t c = 0; c < 1; c++)
				ch->RemoveSpecifyItem(it[tmpevo][c], ic[tmpevo][c]);

			ch->GetNewPetSystem()->IncreasePetEvolution();

		}
		else {
			ch->ChatPacket(CHAT_TYPE_INFO, "Senin Pet Suan icin Gelistirilemez!");
			return;
		}

	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Once Pet'i Cagir!");

}

ACMD(do_PetChangeName)
{
	if (!ch)
		return;

	if (!ch->GetDesc())
		return;
	
	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Pet Gelistirme] Pencere Acik."));
		return;
	}

	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	uint16_t wCell = 0;
	str_to_number(wCell, arg1);
	LPITEM item = ch->GetInventoryItem(wCell);
	if (!item)
		return;

	if (ch->CountSpecifyItem(55030) < 1)
		return;
	
	if (!strcmp(arg2, "%") || 
		!strcmp(arg2, "/") || 
		!strcmp(arg2, ">") || 
		!strcmp(arg2, "|") || 
		!strcmp(arg2, ";") || 
		!strcmp(arg2, ":") || 
		!strcmp(arg2, "}") || 
		!strcmp(arg2, "{") || 
		!strcmp(arg2, "[") || 
		!strcmp(arg2, "]") || 
		!strcmp(arg2, "%") || 
		!strcmp(arg2, "#") || 
		!strcmp(arg2, "@") || 
		!strcmp(arg2, "^") || 
		!strcmp(arg2, "&") ||
		!strcmp(arg2, "'")
		)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[Pet-Kulucka] Hatali isim girdiniz"));
		return;		
	}
	
	if (ch->GetNewPetSystem() && ch->GetNewPetSystem()->IsActivePet())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Once petini gonder. "));
		return ;
	}
	
	DBManager::instance().DirectQuery("UPDATE new_petsystem SET name = '%s' WHERE id = %u", arg2, item->GetID());
	ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Pet Ismi Basarili Bir Sekilde Degistirildi!"));
	ch->RemoveSpecifyItem(55030, 1);
}

ACMD(do_Determine)
{
	if (ch->GetNewPetSystem())
	{
		if (ch->GetNewPetSystem()->IsActivePet())
		{
			if (ch->CountSpecifyItem(55032) >= 1)
			{
				LPITEM pSummonItem = nullptr;
				if ((pSummonItem = ITEM_MANAGER::instance().FindByVID(ch->GetNewPetSystem()->GetSummonItemVID())))
				{
					int16_t type = number(1, 8);
					ch->GetNewPetSystem()->SetType(type);
					pSummonItem->SetForceAttribute(5, pSummonItem->GetAttributeType(5), type);
					ch->RemoveSpecifyItem(55032, 1);
					ch->ChatPacket(CHAT_TYPE_INFO, "Pet Tipi Basari ile degistirildi !");
				}
			}
			else
				ch->ChatPacket(CHAT_TYPE_INFO, "Yeterli item yok !");
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Lutfen once Petinizi cagiriniz!");
	}
}

ACMD(do_PetAttr)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint16_t wCell = 0;
	str_to_number(wCell, arg1);
	LPITEM item = ch->GetInventoryItem(wCell);
	if (!item)
		return;

	if (ch->CountSpecifyItem(55033) >= 1)
	{
		if (!item->GetAttributeValue(4))
		{
			int32_t m_dwbonuspet[3];
			int16_t m_dwPetType = item->GetAttributeValue(5);
			int32_t tmplevel = item->GetAttributeType(5);
			// item->SetForceAttribute(5, tmplevel, m_dwPetType);
			int32_t cnt = tmplevel / Pet_Types[m_dwPetType][0];
			m_dwbonuspet[0] = number(1, 30);
			m_dwbonuspet[1] = number(1, 30);
			m_dwbonuspet[2] = number(1, 30);
			for (int i = 0; i < cnt; ++i)
			{
				m_dwbonuspet[0] += number(Pet_Types[m_dwPetType][1], Pet_Types[m_dwPetType][2]);
				m_dwbonuspet[1] += number(Pet_Types[m_dwPetType][1], Pet_Types[m_dwPetType][2]);
				m_dwbonuspet[2] += number(Pet_Types[m_dwPetType][1], Pet_Types[m_dwPetType][2]);
			}

			ch->ChatPacket(CHAT_TYPE_COMMAND, "ResultAttr %d %d %d %d %d", m_dwbonuspet[0], m_dwbonuspet[1], m_dwbonuspet[2], m_dwPetType, wCell);

			for (int32_t b = 0; b < 3; b++)
				item->SetSocket(b + 3, m_dwbonuspet[b]);

			ch->RemoveSpecifyItem(55033, 1);
			ch->ChatPacket(CHAT_TYPE_INFO, "Pet Bonuslari Basari ile degistirildi !");
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Önce Peti Gönder !");
	}
	else
		ch->ChatPacket(CHAT_TYPE_INFO, "Yeterli item yok !");
}

ACMD(do_PetRevert)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint8_t bType = 0;
	str_to_number(bType, arg1);

	if (ch->GetNewPetSystem())
	{
		if (ch->GetNewPetSystem()->IsActivePet())
		{
			if (bType >= 0 && bType <= 2)
			{
				if (ch->CountSpecifyItem(55028) < 1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, "Yeterli Item Yok !");
					return;
				}

				LPITEM pSummonItem = nullptr;
				if ((pSummonItem = ITEM_MANAGER::instance().FindByVID(ch->GetNewPetSystem()->GetSummonItemVID())))
				{
					if (ch->GetNewPetSystem()->Revert(bType, pSummonItem))
					{
						ch->ChatPacket(CHAT_TYPE_INFO, "Secilen Skill Basari ile temizlendi !");
						ch->RemoveSpecifyItem(55028, 1);
						return;
					}
				}
			}
		}
		else
			ch->ChatPacket(CHAT_TYPE_INFO, "Önce Peti Cagir !");
	}
}
#endif

#ifdef __BOSS_TRACKING_SYSTEM__
ACMD(do_open_boss_tracking)
{
	if (!ch || !ch->GetDesc())
		return;

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen())
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t mob_vnum = 0;
	str_to_number(mob_vnum, arg1);

	for (int32_t i = 0; i < 7; i++)
		CBossTracking::instance().SendClientPacket(ch, i, mob_vnum);

	ch->IsBossTrackingWindow = true;
	ch->BossTrackingMobVnum = mob_vnum;
}
ACMD(do_close_boss_tracking)
{
	/***if (!ch || !ch->GetDesc())
		return;

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen())
		return;***/

	ch->StopBossTrackingDataEvent();
}
#endif

#ifdef __SORT_INVENTORY__
ACMD(do_sort_inventory_items)
{
	if (ch->IsDead() || ch->GetExchange() || ch->IsOpenSafebox() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't sort your inventory with those windows open."));
		return;
	}

	int32_t lastSortInventoryPulse = ch->GetSortInventoryPulse();
	int32_t currentPulse = thecore_pulse();

	if (lastSortInventoryPulse > currentPulse)
		return;

	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint8_t bType = 0;
	str_to_number(bType, arg1);

	for (int32_t i = INVENTORY_ONE_MAX_NUM * bType; i < ch->GetInventoryMax(bType); ++i)
	{
		LPITEM item = ch->GetInventoryItem(i);

		if (!item)
			continue;

		if (item->isLocked())
			continue;

		// if (item->GetCount() == g_wItemCountLimit)
			// continue;

		if (item->IsStackable() && item->GetCount() < g_wItemCountLimit)
		{
			for (int32_t j = i; j < INVENTORY_MAX_NUM; ++j)
			{
				LPITEM item2 = ch->GetInventoryItem(j);

				if (!item2)
					continue;

				if (item2->isLocked())
					continue;

				if (item2->GetVnum() == item->GetVnum())
				{
					bool bStopSockets = false;

					for (int32_t k = 0; k < ITEM_SOCKET_MAX_NUM; ++k)
					{
						if (item2->GetSocket(k) != item->GetSocket(k))
						{
							bStopSockets = true;
							break;
						}
					}

					if (bStopSockets)
						continue;

					uint16_t wAddCount = MIN(g_wItemCountLimit - item->GetCount(), item2->GetCount());

					item->SetCount(item->GetCount() + wAddCount);
					item2->SetCount(item2->GetCount() - wAddCount);

					continue;
				}
			}
		}

		int32_t pos = ch->GetEmptyInventory(item);
		if (pos >= 0 && pos < item->GetCell())
		{
			item->RemoveFromCharacter();
			item->AddToCharacter(ch, TItemPos(INVENTORY, pos));
		}
	}

	ch->SetNextSortInventoryPulse(thecore_pulse() + PASSES_PER_SEC(15));
}

ACMD(do_sort_safebox_items)
{
	/*if (ch->IsDead() || ch->GetExchange() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't sort your inventory with those windows open."));
		return;
	}

	int32_t lastSortInventoryPulse = ch->GetSortInventoryPulse();
	int32_t currentPulse = thecore_pulse();

	if (lastSortInventoryPulse > currentPulse)
		return;

	CSafebox* pkSafebox;

	pkSafebox = ch->GetSafebox();

	if (!pkSafebox)
		return;

	for (int32_t i = 0; i < SAFEBOX_MAX_NUM; ++i)
	{
		LPITEM item = pkSafebox->Get(i);

		if (!item)
			continue;

		if (item->isLocked())
			continue;

		// if (item->GetCount() == g_wItemCountLimit)
			// continue;

		int32_t pos = pkSafebox->GetEmptySafeBox(item->GetSize());

		if (pos >= 0 && pos < item->GetCell())
			pkSafebox->MoveItem(item->GetCell(), pos, item->GetCount());

		if (item->IsStackable() && item->GetCount() < g_wItemCountLimit)
		{
			for (int32_t j = i; j < SAFEBOX_MAX_NUM; ++j)
			{
				LPITEM item2 = pkSafebox->Get(j);

				if (!item2)
					continue;

				if (item2->isLocked())
					continue;

				if (item2->GetVnum() == item->GetVnum())
				{
					bool bStopSockets = false;

					for (int32_t k = 0; k < ITEM_SOCKET_MAX_NUM; ++k)
					{
						if (item2->GetSocket(k) != item->GetSocket(k))
						{
							bStopSockets = true;
							break;
						}
					}

					if (bStopSockets)
						continue;

					// uint8_t bAddCount = MIN(g_wItemCountLimit - item->GetCount(), item2->GetCount());

					pkSafebox->MoveItem(item2->GetCell(), item->GetCell(), item2->GetCount());
					pkSafebox->Remove(item->GetCell());
					pkSafebox->Add(item->GetCell(), item);

					// if (item && item2->GetCount() < bAddCount)
						// pkSafebox->Add(item2->GetCell(), item2);

					continue;
				}
			}
		}
	}

	ch->SetNextSortInventoryPulse(thecore_pulse() + PASSES_PER_SEC(15));*/
}
#endif

#ifdef __SPILT_ITEMS__
ACMD(do_split_items) //SPLIT ITEMS
{
	if (!ch)
		return;

	const char* line;
	char arg1[256], arg2[256], arg3[256];
	line = two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	one_argument(line, arg3, sizeof(arg3));

	if (!*arg1 || !*arg2 || !*arg3)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Wrong command use.");
		return;
	}

	if (!ch->CanWarp())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Close all windows and wait a few seconds, before using this.");
		return;
	}

	uint16_t count = 0;
	uint16_t cell = 0;
	uint16_t destCell = 0;

	str_to_number(cell, arg1);
	str_to_number(count, arg2);
	str_to_number(destCell, arg3);

	LPITEM item = ch->GetInventoryItem(cell);
	if (item != nullptr)
	{
		uint16_t itemCount = item->GetCount();
		while (itemCount > 0)
		{
			if (count > itemCount)
				count = itemCount;

			int32_t iEmptyPosition = ch->GetEmptyInventory(item, destCell);
			if (iEmptyPosition == -1)
				break;

			itemCount -= count;
			ch->MoveItem(TItemPos(INVENTORY, cell), TItemPos(INVENTORY, iEmptyPosition), count);
		}
	}
}
#endif

#ifdef __12ZI__
ACMD(do_cz_reward)
{
	char arg1[12];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	int32_t type;
	str_to_number(type, arg1);

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT * FROM player.zodiac_reward WHERE name = '%s'", ch->GetName()));

	if (pMsg->Get()->uiNumRows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		switch (type)
		{
		case 1:
		{
			DBManager::instance().DirectQuery("UPDATE player.zodiac_reward SET yellowreward = (yellowreward + 1), yellowmark = 0 WHERE name = '%s'", ch->GetName());
			ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacReward %d %d %d %d %d", 0, atoi(row[2]), atoi(row[3]) + 1, atoi(row[4]), atoi(row[5]));
			ch->AutoGiveItem(33026);
		}
		break;
		case 2:
		{
			DBManager::instance().DirectQuery("UPDATE player.zodiac_reward SET greenreward = (greenreward + 1), greenmark = 0 WHERE name = '%s'", ch->GetName());
			ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacReward %d %d %d %d %d", atoi(row[1]), 0, atoi(row[3]), atoi(row[4]) + 1, atoi(row[5]));
			ch->AutoGiveItem(33027);
		}
		break;
		case 3:
		{
			DBManager::instance().DirectQuery("UPDATE player.zodiac_reward SET yellowreward = (yellowreward - 1), greenreward = (greenreward - 1) WHERE name = '%s'", ch->GetName());
			ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacReward %d %d %d %d %d", atoi(row[1]), atoi(row[2]), atoi(row[3]) - 1, atoi(row[4]) - 1, atoi(row[5]));
			ch->AutoGiveItem(33028);
		}
		break;
		default:
			return;
		}
	}
}

ACMD(do_cz_check_box)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!*arg1 || !*arg2)
		return;

	int32_t type = 0;
	int32_t value = 0;
	str_to_number(type, arg1);
	str_to_number(value, arg2);

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT * FROM player.zodiac_reward WHERE name = '%s'", ch->GetName()));

	if (pMsg->Get()->uiNumRows > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
		int32_t yellowmark = 0;
		int32_t greenmark = 0;
		str_to_number(yellowmark, row[1]);
		str_to_number(greenmark, row[2]);

		if (ch->CountSpecifyItem(iCzTable[type][value][0]) >= 50 && ch->CountSpecifyItem(iCzTable[type][value][1]) >= 50)
		{
			if (type == 0)
			{
				if (!IS_SET(yellowmark, 1 << value))
				{
					SET_BIT(yellowmark, 1 << value);
					ch->RemoveSpecifyItem(iCzTable[type][value][0], 50);
					ch->RemoveSpecifyItem(iCzTable[type][value][1], 50);
					DBManager::instance().DirectQuery("UPDATE player.zodiac_reward SET yellowmark = '%d' WHERE name = '%s'", yellowmark, ch->GetName());
					ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacReward %d %d %d %d %d", yellowmark, atoi(row[2]), atoi(row[3]), atoi(row[4]), atoi(row[5]));
				}
				else
					return;
			}
			else if (type == 1)
			{
				if (!IS_SET(greenmark, 1 << value))
				{
					SET_BIT(greenmark, 1 << value);
					ch->RemoveSpecifyItem(iCzTable[type][value][0], 50);
					ch->RemoveSpecifyItem(iCzTable[type][value][1], 50);
					DBManager::instance().DirectQuery("UPDATE player.zodiac_reward SET greenmark = '%d' WHERE name = '%s'", greenmark, ch->GetName());
					ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenZodiacReward %d %d %d %d %d", atoi(row[1]), greenmark, atoi(row[3]), atoi(row[4]), atoi(row[5]));
				}
				else
					return;
			}
		}
	}
}

ACMD(do_revive)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vid;
	str_to_number(vid, arg1);

	LPCHARACTER tch = nullptr;
	if (vid == ch->GetVID())
		tch = ch;
	else
		tch = CHARACTER_MANAGER::instance().Find(vid);

	if (!tch)
		return;

	int32_t ZODIAC_REVIVE_ITEM = ch->CountSpecifyItem(33032);
	int32_t REVIVE_PRICE = ch->GetQuestFlag("zodiac.PrismOfRevival") * 2;
	if (ZODIAC_REVIVE_ITEM < REVIVE_PRICE)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "You dont have enough Prism of Revival. You need %d.", REVIVE_PRICE);
		return;
	}

	tch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");

	tch->GetDesc()->SetPhase(PHASE_GAME);
	tch->SetPosition(POS_STANDING);
	tch->StartRecoveryEvent();

	sys_log(0, "do_restart: restart here");
	tch->RestartAtSamePos();

	tch->PointChange(POINT_HP, ch->GetMaxHP() - ch->GetHP());
	tch->DeathPenalty(0);
	tch->ReviveInvisible(5);
	tch->CheckWears();
	if (vid != ch->GetVID())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "{{212, %s}}", tch->GetName());
		tch->ChatPacket(CHAT_TYPE_INFO, "{{215}}");
		tch->ChatPacket(CHAT_TYPE_INFO, "{{213, %s}}", tch->GetName());
	}

	if (ch->GetQuestFlag("zodiac.PrismOfRevival") == 0)
	{
		ch->RemoveSpecifyItem(33032, 1);
		ch->SetQuestFlag("zodiac.PrismOfRevival", 1);
	}
	else
	{
		ch->RemoveSpecifyItem(33032, REVIVE_PRICE);
		ch->SetQuestFlag("zodiac.PrismOfRevival", REVIVE_PRICE);
	}
}

ACMD(do_revivedialog)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));

	if (!*arg1)
		return;

	uint32_t vid;
	str_to_number(vid, arg1);

	int32_t REVIVE_PRICE = ch->GetQuestFlag("zodiac.PrismOfRevival") * 2;
	ch->ChatPacket(CHAT_TYPE_COMMAND, "OpenReviveDialog %d %d", vid, REVIVE_PRICE);
}
#endif

#ifdef __FAST_RUHBK__
EVENTINFO(TMainEventInfo2)
{
	LPCHARACTER	pkChar;
	int32_t skillindexx;

	TMainEventInfo2()
		: pkChar(nullptr)
		, skillindexx(0)
	{
	}

};

EVENTFUNC(bk_event)
{
	TMainEventInfo2* info = dynamic_cast<TMainEventInfo2*>(event->info);

	if (info == nullptr)
	{
		sys_err("ruh_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER	ch = info->pkChar;
	int32_t skillindex = info->skillindexx;


	if (!ch || skillindex == 0)
		return 0;

	if (!ch->GetDesc())
		return 0;

	if (ch)
	{
		if (ch->CountSpecifyItem(50300) < 1)
			return 0;

		int32_t skilllevel = ch->GetSkillLevel(skillindex);
		if (skilllevel >= 30)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_SKILL_FINISH"));
			return 0;
		}

		LPITEM item = nullptr;
		int32_t iPos = -1;
		for (uint16_t i = INVENTORY_ONE_MAX_NUM * INVENTORY_BOOK; i < ch->GetInventoryMax(INVENTORY_BOOK); ++i)
		{
			item = ch->GetInventoryItem(i);
			if (item && item->GetSocket(0) == skillindex && item->GetVnum() == 50300)
			{
				iPos = i;
				break;
			}
		}

		if (iPos == -1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_NOT_SKILL"));
			return 0;
		}

		if (!item)
			return 0;

		if (item->GetVnum() != 50300 || item->GetSocket(0) != skillindex)
			return 0;

		if (ch->CountSpecifyItem(71001) > 0)
		{
			if (!ch->IsAffectFlag(513))
			{
				ch->AddAffect(513, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
				ch->RemoveSpecifyItem(71001, 1);
			}
		}

		if (ch->CountSpecifyItem(71001) < 1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_NOT_KR_ITEM"));
			return 0;
		}

		if (ch->CountSpecifyItem(71094) >= 1)
		{
			if (!ch->IsAffectFlag(512))
			{
				ch->AddAffect(512, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
				ch->RemoveSpecifyItem(71094, 1);
			}
		}
		if (item->GetVnum() == 50300)
		{
			if (true == ch->LearnSkillByBook(skillindex))
			{
				if (item->GetVnum() == 50300 && item->GetSocket(0) == skillindex)
				{
					item->SetCount(item->GetCount() - 1);
					int32_t iReadDelay;
					iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
					ch->SetSkillNextReadTime(skillindex, get_global_time() + iReadDelay);
				}
			}
			else
			{
				if (item->GetVnum() == 50300 && item->GetSocket(0) == skillindex)
				{
					item->SetCount(item->GetCount() - 1);
					int32_t iReadDelay;
					iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
					ch->SetSkillNextReadTime(skillindex, get_global_time() + iReadDelay);
				}
			}
		}
		return 1;
	}
	return 0;
}

ACMD(do_fastbk)
{

	int32_t gelen = 0;
	int32_t skillindex = 0;
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (!*arg1 || !*arg2)
		return;

	str_to_number(gelen, arg1);
	str_to_number(skillindex, arg2);

	if (gelen < 0 || skillindex < 0)
		return;

	if (!ch)
		return;

	if (!ch->IsPC())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->IsHack())
		return;

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´Ù¸¥ °Å·¡Áß(Ã¢°í,±³È¯,»óÁ¡)¿¡´Â °³ÀÎ»óÁ¡À» »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	int32_t skillgrup = ch->GetSkillGroup();
	if (skillgrup == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_SKILL_NOT_N"));
		return;
	}

	if (ch->CountSpecifyItem(50300) < 1)
		return;

	if (ch->GetMapIndex() == 246)
		return;

	if (gelen == 1)
	{
		int32_t skilllevel = ch->GetSkillLevel(skillindex);

		if (skilllevel >= 30)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_SKILL_MAXLEVEL"));
			return;
		}

		if (ch->GetQuestFlag("bk.yenisure") > get_global_time())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_TIME_OUT_ONE_SECOND"));
			return;
		}

		LPITEM item = nullptr;
		int32_t iPos = -1;
		for (uint16_t i = INVENTORY_ONE_MAX_NUM * INVENTORY_BOOK; i < ch->GetInventoryMax(INVENTORY_BOOK); ++i)
		{
			item = ch->GetInventoryItem(i);
			if (item && item->GetSocket(0) == skillindex && item->GetVnum() == 50300)
			{
				iPos = i;
				break;
			}
		}

		if (iPos == -1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("BK_SYSTEM_NOT_SKILL"));
			return;
		}

		if (!item)
			return;

		if (item->GetVnum() != 50300 || item->GetSocket(0) != skillindex)
			return;

		if (ch->CountSpecifyItem(71001) > 0)
		{
			if (!ch->IsAffectFlag(513))
			{
				ch->AddAffect(513, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
				ch->RemoveSpecifyItem(71001, 1);
			}
		}

		if (ch->CountSpecifyItem(71094) >= 1)
		{
			if (!ch->IsAffectFlag(512))
			{
				ch->AddAffect(512, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
				ch->RemoveSpecifyItem(71094, 1);
			}
		}

		if (item->GetVnum() == 50300)
		{
			if (!ch->FindAffect(AFFECT_SKILL_NO_BOOK_DELAY) && ch->CountSpecifyItem(71001) < 1)
			{
				ch->SkillLearnWaitMoreTimeMessage(ch->GetSkillNextReadTime(skillindex) - get_global_time());
				return;
			}

			if (true == ch->LearnSkillByBook(skillindex))
			{
				if (item->GetVnum() == 50300 && item->GetSocket(0) == skillindex)
				{
					item->SetCount(item->GetCount() - 1);
					int32_t iReadDelay;
					iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
					ch->SetSkillNextReadTime(skillindex, get_global_time() + iReadDelay);
				}
			}
			else
			{
				if (item->GetVnum() == 50300 && item->GetSocket(0) == skillindex)
				{
					item->SetCount(item->GetCount() - 1);
					int32_t iReadDelay;
					iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
					ch->SetSkillNextReadTime(skillindex, get_global_time() + iReadDelay);
				}
			}
			ch->SetQuestFlag("bk.yenisure", get_global_time() + 1);
		}
	}
	else if (gelen == 0)
	{
		if (ch->m_pkFastBKEvent)
			event_cancel(&ch->m_pkFastBKEvent);

		TMainEventInfo2* info = AllocEventInfo<TMainEventInfo2>();
		info->pkChar = ch;
		info->skillindexx = skillindex;
		ch->m_pkFastBKEvent = event_create(bk_event, info, PASSES_PER_SEC(1));
	}

	return;
}

EVENTINFO(TMainEventInfo)
{
	LPCHARACTER	pkChar;
	int32_t skillindexx;

	TMainEventInfo()
		: pkChar(nullptr)
		, skillindexx(0)
	{
	}

};

EVENTFUNC(ruh_event)
{
	TMainEventInfo* info = dynamic_cast<TMainEventInfo*>(event->info);

	if (info == nullptr)
	{
		sys_err("ruh_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER	ch = info->pkChar;
	int32_t skillindex = info->skillindexx;

	if (!ch || skillindex == 0)
		return 0;

	if (!ch->GetDesc())
		return 0;

	if (ch->CountSpecifyItem(50513) < 1)
		return 0;

	int32_t skilllevel = ch->GetSkillLevel(skillindex);

	if (skilllevel >= 40)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhskillson"));
		return 0;
	}
	int32_t gerekenderece = (1000 + 500 * (skilllevel - 30));
	int32_t derecem = (ch->GetRealAlignment() / 10);
	int32_t sonuc = (-29000 + gerekenderece);

	if (derecem < 0)
	{
		gerekenderece = gerekenderece * 2;
		sonuc = (-29000 - gerekenderece);
	}

	if (derecem > sonuc)
	{
		int32_t gerekliknk = gerekenderece;
		int32_t kactane = gerekliknk / 500;
		if (kactane < 0)
			kactane = 0 - kactane;

		if (derecem < 0)
		{
			if (ch->CountSpecifyItem(70102) < kactane)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhzenbitti %d"), kactane);
				return 0;
			}

			int32_t delta = MIN(-(ch->GetAlignment()), 500);
			ch->UpdateAlignment(delta * kactane);
			ch->RemoveSpecifyItem(70102, kactane);
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhzenbastim"));
		}
	}

	if (ch->GetQuestFlag("ruh.sure") > get_global_time())
	{
		if (ch->CountSpecifyItem(71001) < 1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhsuredolmadi"));
			return 0;
		}
		else
			ch->RemoveSpecifyItem(71001, 1);
	}

	if (ch->CountSpecifyItem(71094) >= 1)
	{
		ch->AddAffect(512, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
		ch->RemoveSpecifyItem(71094, 1);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhmunzevikullandim"));
	}

	if (gerekenderece < 0)
		ch->UpdateAlignment(gerekenderece * 10);
	else
		ch->UpdateAlignment(-gerekenderece * 10);

	ch->LearnGrandMasterSkill(skillindex);
	ch->RemoveSpecifyItem(50513, 1);
	ch->SetQuestFlag("ruh.sure", get_global_time() + 60 * 60 * 24);

	return 1;
}

ACMD(do_fastruh)
{
	int32_t gelen = 0;
	int32_t skillindex = 0;
	char arg1[256], arg2[256];

	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));
	if (!*arg1 || !*arg2)
		return;

	str_to_number(gelen, arg1);
	str_to_number(skillindex, arg2);

	if (gelen < 0 || skillindex < 0)
		return;

	if (!ch)
		return;

	if (!ch->IsPC())
		return;

	if (ch->IsDead() || ch->IsStun())
		return;

	if (ch->IsHack())
		return;

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´Ù¸¥ °Å·¡Áß(Ã¢°í,±³È¯,»óÁ¡)¿¡´Â °³ÀÎ»óÁ¡À» »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	if (ch->CountSpecifyItem(50513) < 1)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhtasiyok"));
		return;
	}

	if (ch->GetMapIndex() == 246)
		return;

	LPITEM slot1 = ch->GetWear(WEAR_UNIQUE1);
	LPITEM slot2 = ch->GetWear(WEAR_UNIQUE2);

	if (nullptr != slot1)
	{
		if (slot1->GetVnum() == 70048)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("pelerin_cikar"));
			return;
		}
	}

	if (nullptr != slot2)
	{
		if (slot2->GetVnum() == 70048)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("pelerin_cikar"));
			return;
		}
	}

	int32_t skillgrup = ch->GetSkillGroup();
	if (skillgrup == 0)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhokuyamazsin"));
		return;
	}

	if (gelen == 1)
	{
		int32_t skilllevel = ch->GetSkillLevel(skillindex);
		int32_t gerekenderece = (1000 + 500 * (skilllevel - 30));
		int32_t derecem = (ch->GetRealAlignment() / 10);
		int32_t sonuc = (-29000 + gerekenderece);

		if (ch->GetQuestFlag("ruh.yenisure") > get_global_time())
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruh1sn"));
			return;
		}

		if (derecem < 0)
		{
			gerekenderece = gerekenderece * 2;
			sonuc = (-29000 - gerekenderece);
		}

		if (derecem > sonuc)
		{
			int32_t gerekliknk = gerekenderece;
			int32_t kactane = gerekliknk / 500;
			if (kactane < 0)
				kactane = 0 - kactane;

			if (derecem < 0)
			{
				if (ch->CountSpecifyItem(70102) < kactane)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhzenbitti %d"), kactane);
					return;
				}

				int32_t delta = MIN(-(ch->GetAlignment()), 500);
				ch->UpdateAlignment(delta * kactane);
				ch->RemoveSpecifyItem(70102, kactane);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhzenbastim"));
			}
		}

		if (ch->GetQuestFlag("ruh.sure") > get_global_time())
		{
			if (ch->CountSpecifyItem(71001) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhsuredolmadi"));
				return;
			}
			else
				ch->RemoveSpecifyItem(71001, 1);
		}

		if (ch->CountSpecifyItem(71094) >= 1)
		{
			ch->AddAffect(512, aApplyInfo[0].bPointType, 0, 0, 536870911, 0, false);
			ch->RemoveSpecifyItem(71094, 1);
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ruhmunzevikullandim"));
		}

		if (gerekenderece < 0)
			ch->UpdateAlignment(gerekenderece * 10);
		else
			ch->UpdateAlignment(-gerekenderece * 10);

		ch->LearnGrandMasterSkill(skillindex);
		ch->RemoveSpecifyItem(50513, 1);
		ch->SetQuestFlag("ruh.sure", get_global_time() + 60 * 60 * 24);
		ch->SetQuestFlag("ruh.yenisure", get_global_time() + 1);
	}
	else if (gelen == 0)
	{
		if (ch->m_pkFastRuhEvent)
			event_cancel(&ch->m_pkFastRuhEvent);

		TMainEventInfo* info = AllocEventInfo<TMainEventInfo>();
		info->pkChar = ch;
		info->skillindexx = skillindex;
		ch->m_pkFastRuhEvent = event_create(ruh_event, info, PASSES_PER_SEC(1));
	}
	return;
}
#endif

#ifdef __AFAR_BIOLOG__
ACMD(do_biyolog)
{
	char arg1[256], arg2[256];
	two_arguments(argument, arg1, sizeof(arg1), arg2, sizeof(arg2));

	if (!ch->IsPC())
		return;

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´Ù¸¥ °Å·¡Áß(Ã¢°í,±³È¯,»óÁ¡)¿¡´Â °³ÀÎ»óÁ¡À» »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return;
	}

	int32_t biodurum = ch->GetQuestFlag("bio.durum");
	uint32_t bioverilen = ch->GetQuestFlag("bio.verilen");
	int32_t biokalan = ch->GetQuestFlag("bio.kalan");
	int32_t ruhmu = ch->GetQuestFlag("bio.ruhtasi");
	int32_t gerekenitem = BiyologSistemi[biodurum][0];
	int32_t ruhtasi = BiyologSistemi[biodurum][4];
	int32_t odul = BiyologSistemi[biodurum][5];
	int32_t sans = BiyologSistemi[biodurum][3];
	int32_t bekleme = (BiyologSistemi[biodurum][2] * 60);
	int32_t affectvnum = BiyologSistemi[biodurum][6];
	int32_t affectvalue = BiyologSistemi[biodurum][7];
	int32_t affectvnum2 = BiyologSistemi[biodurum][8];
	int32_t affectvalue2 = BiyologSistemi[biodurum][9];
	int32_t affectvnum3 = BiyologSistemi[biodurum][10];
	int32_t affectvalue3 = BiyologSistemi[biodurum][11];
	/*int32_t affectvnum4 = BiyologSistemi[biodurum][12];
	int32_t affectvalue4 = BiyologSistemi[biodurum][13];*/
	uint32_t toplam = BiyologSistemi[biodurum][1];
	int32_t level = ch->GetLevel();

	if (*arg1 && *arg2)
	{
		if (ch->GetQuestFlag("bio.odulvakti") == 0)
			return;

		int32_t secim = atoi(arg2);
		int32_t seviye = atoi(arg1);
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
		if (secim == 1)
			ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60 * 60 * 24 * 365 * 60, 0, false);
		else if (secim == 2)
			ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum2].bPointType, affectvalue2, 0, 60 * 60 * 24 * 365 * 60, 0, false);
		else if (secim == 3)
			ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum3].bPointType, affectvalue3, 0, 60 * 60 * 24 * 365 * 60, 0, false);
		if (seviye == 92)
		{
			if (level >= 94)
			{
				ch->SetQuestFlag("bio.durum", 10);
				ch->SetQuestFlag("bio.verilen", 0);
				ch->SetQuestFlag("bio.kalan", 0);
				ch->SetQuestFlag("bio.ruhtasi", 0);
				ch->SetQuestFlag("bio.odulvakti", 0);
				biodurum = ch->GetQuestFlag("bio.durum");
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 %d 0", BiyologSistemi[biodurum][0], BiyologSistemi[biodurum][1]);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyologekrankapa");
				return;
			}
			else
			{
				ch->SetQuestFlag("bio.durum", 93);
				ch->SetQuestFlag("bio.verilen", 0);
				ch->SetQuestFlag("bio.kalan", 0);
				ch->SetQuestFlag("bio.ruhtasi", 0);
				ch->SetQuestFlag("bio.odulvakti", 0);
				biodurum = ch->GetQuestFlag("bio.durum");
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyologekrankapa");
				return;
			}
		}
		if (seviye == 94)
		{
			if (level >= 94)
			{
				ch->SetQuestFlag("bio.durum", 11);
				ch->SetQuestFlag("bio.verilen", 0);
				ch->SetQuestFlag("bio.kalan", 0);
				ch->SetQuestFlag("bio.ruhtasi", 0);
				ch->SetQuestFlag("bio.odulvakti", 0);
				biodurum = ch->GetQuestFlag("bio.durum");
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevlerbitti"));
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyologekrankapa");
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
				return;
			}
		}
		return;
	}

	if (level < 30)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biolevelyok"));
		return;
	}

	if (ch->GetQuestFlag("bio.odulvakti") == 1)
	{
		if (biodurum == 9)
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "biyologodul 92 %d %d %d %d %d %d", affectvnum, affectvalue, affectvnum2, affectvalue2, affectvnum3, affectvalue3);
		}
		else if (biodurum == 10)
		{
			ch->ChatPacket(CHAT_TYPE_COMMAND, "biyologodul 94 %d %d %d %d %d %d", affectvnum, affectvalue, affectvnum2, affectvalue2, affectvnum3, affectvalue3);
		}
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioodulsec"));
		return;
	}

	if (biokalan > get_global_time())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosuredolmadi"));
		return;
	}

	/////ork diþi
	if (biodurum == 1 and level >= 30)
	{

		if (bioverilen >= BiyologSistemi[biodurum][1] && ruhmu == 1)
		{
			if (ch->CountSpecifyItem(ruhtasi) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhtasiyok"));
				return;
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->RemoveSpecifyItem(ruhtasi, 1);
				ch->AutoGiveItem(odul, 1);
				ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60 * 60 * 24 * 365 * 60, 0, false);
				if (level >= 40)
				{
					ch->SetQuestFlag("bio.durum", 2);
					ch->SetQuestFlag("bio.30", 1);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 %d 0", BiyologSistemi[biodurum][0], BiyologSistemi[biodurum][1]);
					return;
				}
				else
				{
					ch->SetQuestFlag("bio.durum", 31);
					ch->SetQuestFlag("bio.30", 1);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
					return;
				}
			}
		}
		else
		{
			if (ch->CountSpecifyItem(gerekenitem) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioitemyok"));
				return;
			}
			else
			{
				int32_t prob = number(1, 100);
				if (ch->GetQuestFlag("bio.sans") == 1)
					sans = sans + 100;

				if (ch->GetQuestFlag("bio.sure") == 1)
					ch->SetQuestFlag("bio.sure", 0);
				// ch->ChatPacket(CHAT_TYPE_INFO, "%d %d",sans,prob);
				if (sans >= prob)
				{

					ch->SetQuestFlag("bio.verilen", bioverilen + 1);
					if (ch->GetQuestFlag("bio.sans") == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioozutgitti"));
						ch->SetQuestFlag("bio.sans", 0);
					}

					bioverilen = ch->GetQuestFlag("bio.verilen");
					if (bioverilen == toplam)
					{
						TItemTable* pTable = ITEM_MANAGER::instance().GetTable(ruhtasi);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biotoplamabittiruhtasibul %s"), pTable->szLocaleName);
						ch->SetQuestFlag("bio.ruhtasi", 1);
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 1 0", BiyologSistemi[biodurum][4]);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogecti %d"), (toplam - bioverilen));
						ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
						biokalan = ch->GetQuestFlag("bio.kalan");
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
					}

				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobasarisiz"));
					ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
					biokalan = ch->GetQuestFlag("bio.kalan");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				ch->RemoveSpecifyItem(gerekenitem, 1);
				return;
			}
		}
	}
	//////lanet kitabý
	if (biodurum == 2 and level >= 40)
	{

		if (bioverilen >= BiyologSistemi[biodurum][1] && ruhmu == 1)
		{
			if (ch->CountSpecifyItem(ruhtasi) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhtasiyok"));
				return;
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->RemoveSpecifyItem(ruhtasi, 1);
				ch->AutoGiveItem(odul, 1);
				ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60 * 60 * 24 * 365 * 60, 0, false);
				if (level >= 50)
				{
					ch->SetQuestFlag("bio.durum", 3);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.40", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 %d 0", BiyologSistemi[biodurum][0], BiyologSistemi[biodurum][1]);
					return;
				}
				else
				{
					ch->SetQuestFlag("bio.durum", 41);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.40", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
					return;

				}
			}
		}
		else
		{
			if (ch->CountSpecifyItem(gerekenitem) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioitemyok"));
				return;
			}
			else
			{
				int32_t prob = number(1, 100);
				if (ch->GetQuestFlag("bio.sans") == 1)
					sans = sans + 100;

				if (ch->GetQuestFlag("bio.sure") == 1)
					ch->SetQuestFlag("bio.sure", 0);

				if (sans >= prob)
				{

					ch->SetQuestFlag("bio.verilen", bioverilen + 1);
					if (ch->GetQuestFlag("bio.sans") == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioozutgitti"));
						ch->SetQuestFlag("bio.sans", 0);
					}

					bioverilen = ch->GetQuestFlag("bio.verilen");
					if (bioverilen == toplam)
					{
						TItemTable* pTable = ITEM_MANAGER::instance().GetTable(ruhtasi);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biotoplamabittiruhtasibul %s"), pTable->szLocaleName);
						ch->SetQuestFlag("bio.ruhtasi", 1);
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 1 0", BiyologSistemi[biodurum][4]);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogecti %d"), (toplam - bioverilen));
						ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
						biokalan = ch->GetQuestFlag("bio.kalan");
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
					}

				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobasarisiz"));
					ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
					biokalan = ch->GetQuestFlag("bio.kalan");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				ch->RemoveSpecifyItem(gerekenitem, 1);
				return;
			}
		}
	}
	////////þeytan hatýrasý
	if (biodurum == 3 and level >= 50)
	{

		if (bioverilen >= BiyologSistemi[biodurum][1] && ruhmu == 1)
		{
			if (ch->CountSpecifyItem(ruhtasi) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhtasiyok"));
				return;
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->RemoveSpecifyItem(ruhtasi, 1);
				ch->AutoGiveItem(odul, 1);
				ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60 * 60 * 24 * 365 * 60, 0, false);
				if (level >= 60)
				{
					ch->SetQuestFlag("bio.durum", 4);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.50", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 %d 0", BiyologSistemi[biodurum][0], BiyologSistemi[biodurum][1]);
					return;
				}
				else
				{
					ch->SetQuestFlag("bio.durum", 51);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.50", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
					return;
				}
			}
		}
		else
		{
			if (ch->CountSpecifyItem(gerekenitem) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioitemyok"));
				return;
			}
			else
			{
				int32_t prob = number(1, 100);
				if (ch->GetQuestFlag("bio.sans") == 1)
					sans = sans + 100;

				if (ch->GetQuestFlag("bio.sure") == 1)
					ch->SetQuestFlag("bio.sure", 0);

				if (sans >= prob)
				{

					ch->SetQuestFlag("bio.verilen", bioverilen + 1);
					if (ch->GetQuestFlag("bio.sans") == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioozutgitti"));
						ch->SetQuestFlag("bio.sans", 0);
					}

					bioverilen = ch->GetQuestFlag("bio.verilen");
					if (bioverilen == toplam)
					{
						TItemTable* pTable = ITEM_MANAGER::instance().GetTable(ruhtasi);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biotoplamabittiruhtasibul %s"), pTable->szLocaleName);
						ch->SetQuestFlag("bio.ruhtasi", 1);
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 1 0", BiyologSistemi[biodurum][4]);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogecti %d"), (toplam - bioverilen));
						ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
						biokalan = ch->GetQuestFlag("bio.kalan");
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
					}

				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobasarisiz"));
					ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
					biokalan = ch->GetQuestFlag("bio.kalan");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				ch->RemoveSpecifyItem(gerekenitem, 1);
				return;
			}
		}
	}
	////////buz topu
	if (biodurum == 4 and level >= 60)
	{

		if (bioverilen >= BiyologSistemi[biodurum][1] && ruhmu == 1)
		{
			if (ch->CountSpecifyItem(ruhtasi) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhtasiyok"));
				return;
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->RemoveSpecifyItem(ruhtasi, 1);
				ch->AutoGiveItem(odul, 1);
				ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60 * 60 * 24 * 365 * 60, 0, false);
				if (level >= 70)
				{
					ch->SetQuestFlag("bio.durum", 5);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.60", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 %d 0", BiyologSistemi[biodurum][0], BiyologSistemi[biodurum][1]);
					return;
				}
				else
				{
					ch->SetQuestFlag("bio.durum", 61);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.60", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
					return;
				}
			}
		}
		else
		{
			if (ch->CountSpecifyItem(gerekenitem) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioitemyok"));
				return;
			}
			else
			{
				int32_t prob = number(1, 100);
				if (ch->GetQuestFlag("bio.sans") == 1)
					sans = sans + 100;

				if (ch->GetQuestFlag("bio.sure") == 1)
					ch->SetQuestFlag("bio.sure", 0);

				if (sans >= prob)
				{

					ch->SetQuestFlag("bio.verilen", bioverilen + 1);
					if (ch->GetQuestFlag("bio.sans") == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioozutgitti"));
						ch->SetQuestFlag("bio.sans", 0);
					}

					bioverilen = ch->GetQuestFlag("bio.verilen");
					if (bioverilen == toplam)
					{
						TItemTable* pTable = ITEM_MANAGER::instance().GetTable(ruhtasi);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biotoplamabittiruhtasibul %s"), pTable->szLocaleName);
						ch->SetQuestFlag("bio.ruhtasi", 1);
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 1 0", BiyologSistemi[biodurum][4]);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogecti %d"), (toplam - bioverilen));
						ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
						biokalan = ch->GetQuestFlag("bio.kalan");
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
					}

				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobasarisiz"));
					ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
					biokalan = ch->GetQuestFlag("bio.kalan");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				ch->RemoveSpecifyItem(gerekenitem, 1);
				return;
			}
		}
	}
	////////zelkova
	if (biodurum == 5 and level >= 70)
	{

		if (bioverilen >= BiyologSistemi[biodurum][1] && ruhmu == 1)
		{
			if (ch->CountSpecifyItem(ruhtasi) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhtasiyok"));
				return;
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->RemoveSpecifyItem(ruhtasi, 1);
				ch->AutoGiveItem(odul, 1);
				ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60 * 60 * 24 * 365 * 60, 0, false);
				ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum2].bPointType, affectvalue2, 0, 60 * 60 * 24 * 365 * 60, 0, false);
				if (level >= 80)
				{
					ch->SetQuestFlag("bio.durum", 6);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.70", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 %d 0", BiyologSistemi[biodurum][0], BiyologSistemi[biodurum][1]);
					return;
				}
				else
				{
					ch->SetQuestFlag("bio.durum", 71);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.70", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
					return;
				}
			}
		}
		else
		{
			if (ch->CountSpecifyItem(gerekenitem) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioitemyok"));
				return;
			}
			else
			{
				int32_t prob = number(1, 100);
				if (ch->GetQuestFlag("bio.sans") == 1)
					sans = sans + 100;

				if (ch->GetQuestFlag("bio.sure") == 1)
					ch->SetQuestFlag("bio.sure", 0);

				if (sans >= prob)
				{

					ch->SetQuestFlag("bio.verilen", bioverilen + 1);
					if (ch->GetQuestFlag("bio.sans") == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioozutgitti"));
						ch->SetQuestFlag("bio.sans", 0);
					}

					bioverilen = ch->GetQuestFlag("bio.verilen");
					if (bioverilen == toplam)
					{
						TItemTable* pTable = ITEM_MANAGER::instance().GetTable(ruhtasi);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biotoplamabittiruhtasibul %s"), pTable->szLocaleName);
						ch->SetQuestFlag("bio.ruhtasi", 1);
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 1 0", BiyologSistemi[biodurum][4]);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogecti %d"), (toplam - bioverilen));
						ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
						biokalan = ch->GetQuestFlag("bio.kalan");
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
					}

				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobasarisiz"));
					ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
					biokalan = ch->GetQuestFlag("bio.kalan");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				ch->RemoveSpecifyItem(gerekenitem, 1);
				return;
			}
		}
	}
	////////tabela
	if (biodurum == 6 and level >= 80)
	{

		if (bioverilen >= BiyologSistemi[biodurum][1] && ruhmu == 1)
		{
			if (ch->CountSpecifyItem(ruhtasi) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhtasiyok"));
				return;
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->RemoveSpecifyItem(ruhtasi, 1);
				ch->AutoGiveItem(odul, 1);
				ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60 * 60 * 24 * 365 * 60, 0, false);
				ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum2].bPointType, affectvalue2, 0, 60 * 60 * 24 * 365 * 60, 0, false);
				if (level >= 85)
				{
					ch->SetQuestFlag("bio.durum", 7);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.80", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 %d 0", BiyologSistemi[biodurum][0], BiyologSistemi[biodurum][1]);
					return;
				}
				else
				{
					ch->SetQuestFlag("bio.durum", 81);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.80", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
					return;
				}
			}
		}
		else
		{
			if (ch->CountSpecifyItem(gerekenitem) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioitemyok"));
				return;
			}
			else
			{
				int32_t prob = number(1, 100);
				if (ch->GetQuestFlag("bio.sans") == 1)
					sans = sans + 100;

				if (ch->GetQuestFlag("bio.sure") == 1)
					ch->SetQuestFlag("bio.sure", 0);

				if (sans >= prob)
				{

					ch->SetQuestFlag("bio.verilen", bioverilen + 1);
					if (ch->GetQuestFlag("bio.sans") == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioozutgitti"));
						ch->SetQuestFlag("bio.sans", 0);
					}

					bioverilen = ch->GetQuestFlag("bio.verilen");
					if (bioverilen == toplam)
					{
						TItemTable* pTable = ITEM_MANAGER::instance().GetTable(ruhtasi);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biotoplamabittiruhtasibul %s"), pTable->szLocaleName);
						ch->SetQuestFlag("bio.ruhtasi", 1);
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 1 0", BiyologSistemi[biodurum][4]);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogecti %d"), (toplam - bioverilen));
						ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
						biokalan = ch->GetQuestFlag("bio.kalan");
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
					}

				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobasarisiz"));
					ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
					biokalan = ch->GetQuestFlag("bio.kalan");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				ch->RemoveSpecifyItem(gerekenitem, 1);
				return;
			}
		}
	}

	////////kýrýk dal
	if (biodurum == 7 and level >= 85)
	{

		if (bioverilen >= BiyologSistemi[biodurum][1] && ruhmu == 1)
		{
			if (ch->CountSpecifyItem(ruhtasi) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhtasiyok"));
				return;
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->RemoveSpecifyItem(ruhtasi, 1);
				ch->AutoGiveItem(odul, 1);
				ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60 * 60 * 24 * 365 * 60, 0, false);
				ch->AddAffect(AFFECT_COLLECT, 141, 10, 0, 60 * 60 * 24 * 365 * 60, 0, false);
				if (level >= 90)
				{
					ch->SetQuestFlag("bio.durum", 8);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.85", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 %d 0", BiyologSistemi[biodurum][0], BiyologSistemi[biodurum][1]);
					return;
				}
				else
				{
					ch->SetQuestFlag("bio.durum", 86);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.85", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
					return;
				}
			}
		}
		else
		{
			if (ch->CountSpecifyItem(gerekenitem) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioitemyok"));
				return;
			}
			else
			{
				int32_t prob = number(1, 100);
				if (ch->GetQuestFlag("bio.sans") == 1)
					sans = sans + 100;

				if (ch->GetQuestFlag("bio.sure") == 1)
					ch->SetQuestFlag("bio.sure", 0);

				if (sans >= prob)
				{

					ch->SetQuestFlag("bio.verilen", bioverilen + 1);
					if (ch->GetQuestFlag("bio.sans") == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioozutgitti"));
						ch->SetQuestFlag("bio.sans", 0);
					}

					bioverilen = ch->GetQuestFlag("bio.verilen");
					if (bioverilen == toplam)
					{
						TItemTable* pTable = ITEM_MANAGER::instance().GetTable(ruhtasi);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biotoplamabittiruhtasibul %s"), pTable->szLocaleName);
						ch->SetQuestFlag("bio.ruhtasi", 1);
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 1 0", BiyologSistemi[biodurum][4]);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogecti %d"), (toplam - bioverilen));
						ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
						biokalan = ch->GetQuestFlag("bio.kalan");
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
					}

				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobasarisiz"));
					ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
					biokalan = ch->GetQuestFlag("bio.kalan");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				ch->RemoveSpecifyItem(gerekenitem, 1);
				return;
			}
		}
	}

	////////lider not
	if (biodurum == 8 and level >= 90)
	{

		if (bioverilen >= BiyologSistemi[biodurum][1] && ruhmu == 1)
		{
			if (ch->CountSpecifyItem(ruhtasi) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhtasiyok"));
				return;
			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevtamamlandi"));
				ch->RemoveSpecifyItem(ruhtasi, 1);
				ch->AutoGiveItem(odul, 1);
				ch->AddAffect(AFFECT_COLLECT, aApplyInfo[affectvnum].bPointType, affectvalue, 0, 60 * 60 * 24 * 365 * 60, 0, false);
				ch->AddAffect(AFFECT_COLLECT, 140, 8, 0, 60 * 60 * 24 * 365 * 60, 0, false);
				if (level >= 92)
				{
					ch->SetQuestFlag("bio.durum", 9);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.90", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioyenigorev"));
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 %d 0", BiyologSistemi[biodurum][0], BiyologSistemi[biodurum][1]);
					return;
				}
				else
				{
					ch->SetQuestFlag("bio.durum", 91);
					ch->SetQuestFlag("bio.verilen", 0);
					ch->SetQuestFlag("bio.kalan", 0);
					ch->SetQuestFlag("bio.ruhtasi", 0);
					ch->SetQuestFlag("bio.90", 1);
					biodurum = ch->GetQuestFlag("bio.durum");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
					return;
				}
			}
		}
		else
		{
			if (ch->CountSpecifyItem(gerekenitem) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioitemyok"));
				return;
			}
			else
			{
				int32_t prob = number(1, 100);
				if (ch->GetQuestFlag("bio.sans") == 1)
					sans = sans + 100;

				if (ch->GetQuestFlag("bio.sure") == 1)
					ch->SetQuestFlag("bio.sure", 0);

				if (sans >= prob)
				{

					ch->SetQuestFlag("bio.verilen", bioverilen + 1);
					if (ch->GetQuestFlag("bio.sans") == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioozutgitti"));
						ch->SetQuestFlag("bio.sans", 0);
					}

					bioverilen = ch->GetQuestFlag("bio.verilen");
					if (bioverilen == toplam)
					{
						TItemTable* pTable = ITEM_MANAGER::instance().GetTable(ruhtasi);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biotoplamabittiruhtasibul %s"), pTable->szLocaleName);
						ch->SetQuestFlag("bio.ruhtasi", 1);
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 1 0", BiyologSistemi[biodurum][4]);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogecti %d"), (toplam - bioverilen));
						ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
						biokalan = ch->GetQuestFlag("bio.kalan");
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
					}

				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobasarisiz"));
					ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
					biokalan = ch->GetQuestFlag("bio.kalan");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				ch->RemoveSpecifyItem(gerekenitem, 1);
				return;
			}
		}
	}
	////////kemgöz
	if (biodurum == 9 and level >= 92)
	{
		if (ch->CountSpecifyItem(gerekenitem) < 1)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioitemyok"));
			return;
		}
		else
		{
			int32_t prob = number(1, 100);
			if (ch->GetQuestFlag("bio.sans") == 1)
				sans = sans + 100;

			if (ch->GetQuestFlag("bio.sure") == 1)
				ch->SetQuestFlag("bio.sure", 0);

			if (sans >= prob)
			{
				ch->SetQuestFlag("bio.verilen", bioverilen + 1);
				if (ch->GetQuestFlag("bio.sans") == 1)
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioozutgitti"));
					ch->SetQuestFlag("bio.sans", 0);
				}

				bioverilen = ch->GetQuestFlag("bio.verilen");
				if (bioverilen == toplam)
				{
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyologodul 92 %d %d %d %d %d %d", affectvnum, affectvalue, affectvnum2, affectvalue2, affectvnum3, affectvalue3);
					ch->SetQuestFlag("bio.odulvakti", 1);
					ch->SetQuestFlag("bio.92", 1);
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioodulsec"));
					return;
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogecti %d"), (toplam - bioverilen));
					ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
					biokalan = ch->GetQuestFlag("bio.kalan");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}

			}
			else
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobasarisiz"));
				ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
				biokalan = ch->GetQuestFlag("bio.kalan");
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
			}
			ch->RemoveSpecifyItem(gerekenitem, 1);
			return;
		}
	}

	if (biodurum == 10 and level >= 94)
	{

		if (bioverilen >= BiyologSistemi[biodurum][1] && ruhmu == 1)
		{
			if (ch->CountSpecifyItem(ruhtasi) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhtasiyok"));
				return;
			}
			else
			{
				ch->RemoveSpecifyItem(ruhtasi, 1);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				ch->ChatPacket(CHAT_TYPE_COMMAND, "biyologodul 94 %d %d %d %d %d %d", affectvnum, affectvalue, affectvnum2, affectvalue2, affectvnum3, affectvalue3);
				ch->SetQuestFlag("bio.odulvakti", 1);
				ch->SetQuestFlag("bio.94", 1);
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioodulsec"));
				return;
			}
		}
		else
		{
			if (ch->CountSpecifyItem(gerekenitem) < 1)
			{
				ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioitemyok"));
				return;
			}
			else
			{
				int32_t prob = number(1, 100);
				if (ch->GetQuestFlag("bio.sans") == 1)
					sans = sans + 100;

				if (ch->GetQuestFlag("bio.sure") == 1)
					ch->SetQuestFlag("bio.sure", 0);

				if (sans >= prob)
				{
					ch->SetQuestFlag("bio.verilen", bioverilen + 1);
					if (ch->GetQuestFlag("bio.sans") == 1)
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioozutgitti"));
						ch->SetQuestFlag("bio.sans", 0);
					}

					bioverilen = ch->GetQuestFlag("bio.verilen");
					if (bioverilen == toplam)
					{
						TItemTable* pTable = ITEM_MANAGER::instance().GetTable(ruhtasi);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biotoplamabittiruhtasibul %s"), pTable->szLocaleName);
						ch->SetQuestFlag("bio.ruhtasi", 1);
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d 0 1 0", BiyologSistemi[biodurum][4]);
					}
					else
					{
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogecti %d"), (toplam - bioverilen));
						ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
						biokalan = ch->GetQuestFlag("bio.kalan");
						ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
					}
				}
				else
				{
					ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biobasarisiz"));
					ch->SetQuestFlag("bio.kalan", get_global_time() + bekleme);
					biokalan = ch->GetQuestFlag("bio.kalan");
					ch->ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				ch->RemoveSpecifyItem(gerekenitem, 1);
				return;
			}
		}
	}
	return;
}
#endif

#ifdef __AUTO_PICKUP__
ACMD(do_autopick)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	bool bIsOpen = false;
	str_to_number(bIsOpen, arg1);
	if (ch)
		ch->SetAutoPickup(bIsOpen);
}
#endif

#ifdef __BLOCK_EXP__
ACMD(do_bolckexp)
{
	char arg1[256];
	one_argument(argument, arg1, sizeof(arg1));
	if (!*arg1)
		return;

	bool bIsOpen = false;
	str_to_number(bIsOpen, arg1);
	if (ch)
		ch->SetQuestFlag("sec.exp", bIsOpen);
}
#endif

#ifdef WJ_SECURITY_SYSTEM
ACMD(do_input_security)
{
    char arg1[256];
    int32_t gelensifre;
    one_argument(argument, arg1, sizeof(arg1));

    if (!*arg1)
        return;

    str_to_number(gelensifre, arg1);

    if (gelensifre == 0 || ch->GetCaptchaCount() >= 5)
    {

        ch->GetDesc()->DelayedDisconnect(3);
        ch->ChatPacket(CHAT_TYPE_INFO, "Cok Fazla Yanlis Girdiniz !");
        return;
    }

    int32_t captcham = ch->GetCaptcha();

    if (captcham == gelensifre)
    {
		ch->SetQuestFlag("sec.cpt", time(nullptr) + 30 * 60);
		// ch->SetLastSecurityLastTime(thecore_pulse() + PASSES_PER_SEC(10 * 60));
		ch->PrepareSecurityGui(false);
 		ch->SetCaptchaCount(0);
   }
    else
    {
        ch->ChatPacket(CHAT_TYPE_INFO, "Yanlis Girdiniz !");
		ch->PrepareSecurityGui(false);
		ch->SendSecurityGui();
		ch->SetCaptchaCount(ch->GetCaptchaCount() + 1);
    }
    return;
}
#endif

ACMD(do_down)
{
	LPITEM item = nullptr;
	item->SetCount(1);
}

