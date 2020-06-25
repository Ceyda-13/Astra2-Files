#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "desc.h"
#include "desc_manager.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "mob_manager.h"
#include "battle.h"
#include "pvp.h"
#include "skill.h"
#include "start_position.h"
#include "profiler.h"
#include "cmd.h"
#include "dungeon.h"
#include "log.h"
#include "unique_item.h"
#include "priv_manager.h"
#include "db.h"
#include "vector.h"
#include "marriage.h"
#include "arena.h"
#include "regen.h"
#include "monarch.h"
#include "exchange.h"
#include "shop_manager.h"
#include "castle.h"
#include "dev_log.h"
#include "ani.h"
#include "BattleArena.h"
#include "packet.h"
#include "party.h"
#include "affect.h"
#include "guild.h"
#include "guild_manager.h"
#include "questmanager.h"
#include "questlua.h"
#include "threeway_war.h"
#include "BlueDragon.h"
#include "DragonLair.h"
#include <random>
#include <algorithm>
#ifdef ENABLE_GUILDWAR_KILL_COUNTER // @correction200
#include "war_map.h"
#endif
#ifdef __MOUNT__
#include "mountsystem.h"
#endif
#ifdef __GUILD_DRAGONLAIR__
#include "meleylair.h"
#endif
#ifdef __TEMPLE_OCHAO__
#include "templeochao.h"
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

// @correction001 BEGIN
static uint32_t __GetPartyExpNP(const uint32_t level)
{
	if (!level || level > PLAYER_EXP_TABLE_MAX)
		return 14000;
	return party_exp_distribute_table[level];
}

static int32_t __GetExpLossPerc(const uint32_t level)
{
	if (!level || level > PLAYER_EXP_TABLE_MAX)
		return 1;
	return aiExpLossPercents[level];
}
// @correction001 END

uint32_t AdjustExpByLevel(const LPCHARACTER ch, const uint32_t exp)
{
	if (PLAYER_MAX_LEVEL_CONST < ch->GetLevel()) // @correction001
	{
		double ret = 0.95;
		double factor = 0.1;

		for (ssize_t i=0 ; i < ch->GetLevel()-100 ; ++i)
		{
			if ( (i%10) == 0)
				factor /= 2.0;

			ret *= 1.0 - factor;
		}

		ret = ret * static_cast<double>(exp);

		if (ret < 1.0)
			return 1;

		return static_cast<uint32_t>(ret);
	}

	return exp;
}

bool CHARACTER::CanBeginFight() const
{
	if (!CanMove())
		return false;

	return m_pointsInstant.position == POS_STANDING && !IsDead() && !IsStun();
}

void CHARACTER::BeginFight(LPCHARACTER pkVictim)
{
	SetVictim(pkVictim);
	SetPosition(POS_FIGHTING);
	SetNextStatePulse(1);
}

bool CHARACTER::CanFight() const
{
	return m_pointsInstant.position >= POS_FIGHTING ? true : false;
}

void CHARACTER::CreateFly(uint8_t bType, LPCHARACTER pkVictim)
{
	TPacketGCCreateFly packFly;

	packFly.bHeader         = HEADER_GC_CREATE_FLY;
	packFly.bType           = bType;
	packFly.dwStartVID      = GetVID();
	packFly.dwEndVID        = pkVictim->GetVID();

	PacketAround(&packFly, sizeof(TPacketGCCreateFly));
}

void CHARACTER::DistributeSP(LPCHARACTER pkKiller, int32_t iMethod)
{
	if (pkKiller->GetSP() >= pkKiller->GetMaxSP())
		return;

	bool bAttacking = (get_dword_time() - GetLastAttackTime()) < 3000;
	bool bMoving = (get_dword_time() - GetLastMoveTime()) < 3000;

	if (iMethod == 1)
	{
		int32_t num = number(0, 3);

		if (!num)
		{
			int32_t iLvDelta = GetLevel() - pkKiller->GetLevel();
			int32_t iAmount = 0;

			if (iLvDelta >= 5)
				iAmount = 10;
			else if (iLvDelta >= 0)
				iAmount = 6;
			else if (iLvDelta >= -3)
				iAmount = 2;

			if (iAmount != 0)
			{
				iAmount += (iAmount * pkKiller->GetPoint(POINT_SP_REGEN)) / 100;

				if (iAmount >= 11)
					CreateFly(FLY_SP_BIG, pkKiller);
				else if (iAmount >= 7)
					CreateFly(FLY_SP_MEDIUM, pkKiller);
				else
					CreateFly(FLY_SP_SMALL, pkKiller);

				pkKiller->PointChange(POINT_SP, iAmount);
			}
		}
	}
	else
	{
		if (pkKiller->GetJob() == JOB_SHAMAN || (pkKiller->GetJob() == JOB_SURA && pkKiller->GetSkillGroup() == 2))
		{
			int32_t iAmount;

			if (bAttacking)
				iAmount = 2 + GetMaxSP() / 100;
			else if (bMoving)
				iAmount = 3 + GetMaxSP() * 2 / 100;
			else
				iAmount = 10 + GetMaxSP() * 3 / 100;

			iAmount += (iAmount * pkKiller->GetPoint(POINT_SP_REGEN)) / 100;
			pkKiller->PointChange(POINT_SP, iAmount);
		}
		else
		{
			int32_t iAmount;

			if (bAttacking)
				iAmount = 2 + pkKiller->GetMaxSP() / 200;
			else if (bMoving)
				iAmount = 2 + pkKiller->GetMaxSP() / 100;
			else
			{
				if (pkKiller->GetHP() < pkKiller->GetMaxHP())
					iAmount = 2 + (pkKiller->GetMaxSP() / 100);
				else
					iAmount = 9 + (pkKiller->GetMaxSP() / 100);
			}

			iAmount += (iAmount * pkKiller->GetPoint(POINT_SP_REGEN)) / 100;
			pkKiller->PointChange(POINT_SP, iAmount);
		}
	}
}


bool CHARACTER::Attack(LPCHARACTER pkVictim, uint8_t bType)
{
#ifdef WJ_SECURITY_SYSTEM
    if (bIsActivate())
    {
		SendSecurityGui();
		return false;
    }
#endif

#ifdef __12ZI__
	if (pkVictim && pkVictim->IsPC() && IsPC() && GetMapIndex() >= 3550000 && GetMapIndex() < 3560000 && GetPolymorphVnum() != 101 && IsPolymorphed())
	{
		RemoveAffect(AFFECT_POLYMORPH);
		SetPolymorph(0);
	}
#endif

#ifdef __SUPPORT__
	if (pkVictim && pkVictim->IsSupport())
	{
		ChatPacket(CHAT_TYPE_INFO, "Afrodite saldiramazsiniz!");
		return false;
	}
#endif

	if (test_server)
		sys_log(0, "[TEST_SERVER] Attack : %s type %d, MobBattleType %d", GetName(), bType, !GetMobBattleType() ? 0 : GetMobAttackRange());
	if (!CanMove())
		return false;
	if (test_server)
		sys_log(0, "[TEST_SERVER] Attack1");

	if (IS_CASTLE_MAP(GetMapIndex()) && false == castle_can_attack(this, pkVictim))
		return false;
	if (test_server)
		sys_log(0, "[TEST_SERVER] Attack2");

	// @correction052
	if (!battle_is_attackable(this, pkVictim))
		return false;
	if (test_server)
		sys_log(0, "[TEST_SERVER] Attack3");

	uint32_t dwCurrentTime = get_dword_time();

	if (IsPC())
	{
		if (IS_SPEED_HACK(this, pkVictim, dwCurrentTime))
			return false;

		if (bType == 0 && dwCurrentTime < GetSkipComboAttackByTime())
			return false;
	}
	else
	{
		MonsterChat(MONSTER_CHAT_ATTACK);
	}
	if (test_server)
		sys_log(0, "[TEST_SERVER] Attack4");

	pkVictim->SetSyncOwner(this);

	if (pkVictim->CanBeginFight())
		pkVictim->BeginFight(this);

#ifdef __TEMPLE_OCHAO__
	if ((pkVictim->IsMonster()) && (pkVictim->GetMobTable().dwVnum == TEMPLE_OCHAO_GUARDIAN) && (pkVictim->GetMapIndex() == TEMPLE_OCHAO_MAP_INDEX))
		TempleOchao::CMgr::instance().GuardianAttacked();
#endif
	if (test_server)
		sys_log(0, "[TEST_SERVER] Attack5");

	int32_t iRet;

	if (bType == 0)
	{
		switch (GetMobBattleType())
		{
			case BATTLE_TYPE_MELEE:
			case BATTLE_TYPE_POWER:
			case BATTLE_TYPE_TANKER:
			case BATTLE_TYPE_SUPER_POWER:
			case BATTLE_TYPE_SUPER_TANKER:
				iRet = battle_melee_attack(this, pkVictim);
				break;

			case BATTLE_TYPE_RANGE:
				FlyTarget(pkVictim->GetVID(), pkVictim->GetX(), pkVictim->GetY(), HEADER_CG_FLY_TARGETING);
				iRet = Shoot(0) ? BATTLE_DAMAGE : BATTLE_NONE;
				break;

			case BATTLE_TYPE_MAGIC:
				FlyTarget(pkVictim->GetVID(), pkVictim->GetX(), pkVictim->GetY(), HEADER_CG_FLY_TARGETING);
				iRet = Shoot(1) ? BATTLE_DAMAGE : BATTLE_NONE;
				break;

			default:
				sys_err("Unhandled battle type %d", GetMobBattleType());
				iRet = BATTLE_NONE;
				break;
		}
	}
	else
	{
		if (IsPC() == true)
		{
			if (dwCurrentTime - m_dwLastSkillTime > 1500)
			{
				sys_log(1, "HACK: Too int32_t skill using term. Name(%s) PID(%u) delta(%u)",
						GetName(), GetPlayerID(), (dwCurrentTime - m_dwLastSkillTime));
				return false;
			}
		}

		sys_log(1, "Attack call ComputeSkill %d %s", bType, pkVictim?pkVictim->GetName():"");
		iRet = ComputeSkill(bType, pkVictim);
	}
	if (test_server)
		sys_log(0, "[TEST_SERVER] Attack6");

	if (iRet == BATTLE_DAMAGE || iRet == BATTLE_DEAD)
	{
		OnMove(true);
		pkVictim->OnMove();

		if (BATTLE_DEAD == iRet && IsPC())
			SetVictim(nullptr);

		return true;
	}
	if (test_server)
		sys_log(0, "[TEST_SERVER] Attack7");

	return false;
}

void CHARACTER::DeathPenalty(uint8_t bTown)
{
	sys_log(1, "DEATH_PERNALY_CHECK(%s) town(%d)", GetName(), bTown);

	Cube_close(this);
#ifdef __ACCE_SYSTEM__ // @correction191
	CloseAcce();
#endif
#ifdef __CHANGE_LOOK__ // @correction192
	ChangeLookWindow(false, true);
#endif
	if (CBattleArena::instance().IsBattleArenaMap(GetMapIndex()) == true)
	{
		return;
	}

	if (GetLevel() < 10)
	{
		sys_log(0, "NO_DEATH_PENALTY_LESS_LV10(%s)", GetName());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ë½ÅÀÇ °¡È£·Î °æÇèÄ¡°¡ ¶³¾îÁöÁö ¾Ê¾Ò½À´Ï´Ù."));
		return;
	}

   	if (number(0, 2))
	{
		sys_log(0, "NO_DEATH_PENALTY_LUCK(%s)", GetName());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ë½ÅÀÇ °¡È£·Î °æÇèÄ¡°¡ ¶³¾îÁöÁö ¾Ê¾Ò½À´Ï´Ù."));
		return;
	}

	if (IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY))
	{
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);

		// @correction005
		if (!bTown)
		{
			if (FindAffect(AFFECT_NO_DEATH_PENALTY))
			{
				sys_log(0, "NO_DEATH_PENALTY_AFFECT(%s)", GetName());
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ë½ÅÀÇ °¡È£·Î °æÇèÄ¡°¡ ¶³¾îÁöÁö ¾Ê¾Ò½À´Ï´Ù."));
				RemoveAffect(AFFECT_NO_DEATH_PENALTY);
				return;
			}
		}

		int32_t iLoss = ((GetNextExp() * __GetExpLossPerc(GetLevel())) / 100); // @correction001

		// @correction005 BEGIN
		iLoss = MIN(800000, iLoss);

		if (bTown)
			iLoss = 0;
		// @correction005 END

		if (IsEquipUniqueItem(UNIQUE_ITEM_TEARDROP_OF_GODNESS))
			iLoss /= 2;

		sys_log(0, "DEATH_PENALTY(%s) EXP_LOSS: %d percent %d%%", GetName(), iLoss, __GetExpLossPerc(GetLevel())); // @correction001

		PointChange(POINT_EXP, -iLoss, true);
	}
}

bool CHARACTER::IsStun() const
{
	if (IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN))
		return true;

	return false;
}

EVENTFUNC(StunEvent)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "StunEvent> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) {
		return 0;
	}
	ch->m_pkStunEvent = nullptr;
	ch->Dead();
	return 0;
}

void CHARACTER::Stun()
{
	if (IsStun())
		return;

	if (IsDead())
		return;

	if (!IsPC() && m_pkParty)
	{
		m_pkParty->SendMessage(this, PM_ATTACKED_BY, 0, 0);
	}

	sys_log(1, "%s: Stun %p", GetName(), this);

	PointChange(POINT_HP_RECOVERY, -GetPoint(POINT_HP_RECOVERY));
	PointChange(POINT_SP_RECOVERY, -GetPoint(POINT_SP_RECOVERY));

	CloseMyShop();

	event_cancel(&m_pkRecoveryEvent);

	TPacketGCStun pack;
	pack.header	= HEADER_GC_STUN;
	pack.vid	= m_vid;
	PacketAround(&pack, sizeof(pack));

	SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN);

	if (m_pkStunEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	m_pkStunEvent = event_create(StunEvent, info, PASSES_PER_SEC(0));
}

EVENTINFO(SCharDeadEventInfo)
{
	bool isPC;
	uint32_t dwID;

	SCharDeadEventInfo()
	: isPC(0)
	, dwID(0)
	{
	}
};

EVENTFUNC(dead_event)
{
	const SCharDeadEventInfo* info = dynamic_cast<SCharDeadEventInfo*>(event->info);

	if ( info == nullptr )
	{
		sys_err( "dead_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = nullptr;

	if (true == info->isPC)
	{
		ch = CHARACTER_MANAGER::instance().FindByPID( info->dwID );
	}
	else
	{
		ch = CHARACTER_MANAGER::instance().Find( info->dwID );
	}

	if (nullptr == ch)
	{
		sys_err("DEAD_EVENT: cannot find char pointer with %s id(%d)", info->isPC ? "PC" : "MOB", info->dwID );
		return 0;
	}

	ch->m_pkDeadEvent = nullptr;

	if (ch->GetDesc())
	{
		ch->GetDesc()->SetPhase(PHASE_GAME);

		ch->SetPosition(POS_STANDING);

		PIXEL_POSITION pos;

		if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
			ch->WarpSet(pos.x, pos.y);
		else
		{
			sys_err("cannot find spawn position (name %s)", ch->GetName());
			ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}

		ch->PointChange(POINT_HP, (ch->GetMaxHP() / 2) - ch->GetHP(), true);

		ch->DeathPenalty(0);

		ch->StartRecoveryEvent();

		ch->ChatPacket(CHAT_TYPE_COMMAND, "CloseRestartWindow");
	}
	else
	{
		if (ch->IsMonster() == true)
		{
			if (ch->IsRevive() == false && ch->HasReviverInParty() == true)
			{
				ch->SetPosition(POS_STANDING);
				ch->SetHP(ch->GetMaxHP());

				ch->ViewReencode();

				ch->SetAggressive();
				ch->SetRevive(true);

				return 0;
			}
		}

		M2_DESTROY_CHARACTER(ch);
	}

	return 0;
}

bool CHARACTER::IsDead() const
{
	if (m_pointsInstant.position == POS_DEAD)
		return true;

	return false;
}

#define GetGoldMultipler() (distribution_test_server ? 3 : 1)

void CHARACTER::RewardGold(LPCHARACTER pkAttacker)
{
	// bool isAutoLoot =
		// (pkAttacker->GetPremiumRemainSeconds(PREMIUM_AUTOLOOT) > 0 ||
		 // pkAttacker->IsEquipUniqueGroup(UNIQUE_GROUP_AUTOLOOT))
		// ? true : false;

	// PIXEL_POSITION pos;

	// if (!isAutoLoot)
		// if (!SECTREE_MANAGER::instance().GetMovablePosition(GetMapIndex(), GetX(), GetY(), pos))
			// return;

	int32_t iTotalGold = 0;
	int32_t iGoldPercent = MobRankStats[GetMobRank()].iGoldPercent;

	if (pkAttacker->IsPC())
		iGoldPercent = iGoldPercent * (100 + CPrivManager::instance().GetPriv(pkAttacker, PRIV_GOLD_DROP)) / 100;

	if (pkAttacker->GetPoint(POINT_MALL_GOLDBONUS))
		iGoldPercent += (iGoldPercent * pkAttacker->GetPoint(POINT_MALL_GOLDBONUS) / 100);

	iGoldPercent = iGoldPercent * CHARACTER_MANAGER::instance().GetMobGoldDropRate(pkAttacker) / 100;

	if (pkAttacker->GetPremiumRemainSeconds(PREMIUM_GOLD) > 0 ||
			pkAttacker->IsEquipUniqueGroup(UNIQUE_GROUP_LUCKY_GOLD))
		iGoldPercent += iGoldPercent;

	if (iGoldPercent > 100)
		iGoldPercent = 100;

	int32_t iPercent;

	if (GetMobRank() >= MOB_RANK_BOSS)
		iPercent = ((iGoldPercent * PERCENT_LVDELTA_BOSS(pkAttacker->GetLevel(), GetLevel())) / 100);
	else
		iPercent = ((iGoldPercent * PERCENT_LVDELTA(pkAttacker->GetLevel(), GetLevel())) / 100);

	if (number(1, 100) > iPercent)
		return;

	int32_t iGoldMultipler = GetGoldMultipler();

	if (1 == number(1, 50000))
		iGoldMultipler *= 10;
	else if (1 == number(1, 10000))
		iGoldMultipler *= 5;

	if (pkAttacker->GetPoint(POINT_GOLD_DOUBLE_BONUS))
		if (number(1, 100) <= pkAttacker->GetPoint(POINT_GOLD_DOUBLE_BONUS))
			iGoldMultipler *= 2;

	if (test_server)
		pkAttacker->ChatPacket(CHAT_TYPE_PARTY, "gold_mul %d rate %d", iGoldMultipler, CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker));

	// LPITEM item;

	int32_t iGold10DropPct = 100;
	iGold10DropPct = (iGold10DropPct * 100) / (100 + CPrivManager::instance().GetPriv(pkAttacker, PRIV_GOLD10_DROP));

	if (GetMobRank() >= MOB_RANK_BOSS && !IsStone() && GetMobTable().dwGoldMax != 0)
	{
		if (1 == number(1, iGold10DropPct))
			iGoldMultipler *= 10;

		int32_t iSplitCount = number(25, 35);

		for (int32_t i = 0; i < iSplitCount; ++i)
		{
			int32_t iGold = number(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax) / iSplitCount;
			if (test_server)
				sys_log(0, "iGold %d", iGold);
			iGold = iGold * CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker) / 100;
			iGold *= iGoldMultipler;

			if (iGold == 0)
			{
				continue ;
			}

			if (test_server)
			{
				sys_log(0, "Drop Moeny MobGoldAmountRate %d %d", CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker), iGoldMultipler);
				sys_log(0, "Drop Money gold %d GoldMin %d GoldMax %d", iGold, GetMobTable().dwGoldMax, GetMobTable().dwGoldMax);
			}

			if (pkAttacker)
				pkAttacker->GiveGold(iGold);
			// if ((item = ITEM_MANAGER::instance().CreateItem(1, iGold)))
			// {
				// pos.x = GetX() + ((number(-14, 14) + number(-14, 14)) * 23);
				// pos.y = GetY() + ((number(-14, 14) + number(-14, 14)) * 23);

				// item->AddToGround(GetMapIndex(), pos);
				// item->StartDestroyEvent();

				// iTotalGold += iGold;
			// }
		}
	}
	else if (1 == number(1, iGold10DropPct))
	{
		for (int32_t i = 0; i < 10; ++i)
		{
			int32_t iGold = number(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax);
			iGold = iGold * CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker) / 100;
			iGold *= iGoldMultipler;

			if (iGold == 0)
			{
				continue;
			}

			if (pkAttacker)
				pkAttacker->GiveGold(iGold);
			// if ((item = ITEM_MANAGER::instance().CreateItem(1, iGold)))
			// {
				// pos.x = GetX() + (number(-7, 7) * 20);
				// pos.y = GetY() + (number(-7, 7) * 20);

				// item->AddToGround(GetMapIndex(), pos);
				// item->StartDestroyEvent();

				// iTotalGold += iGold;
			// }
		}
	}
	else
	{
		int32_t iGold = number(GetMobTable().dwGoldMin, GetMobTable().dwGoldMax);
		iGold = iGold * CHARACTER_MANAGER::instance().GetMobGoldAmountRate(pkAttacker) / 100;
		iGold *= iGoldMultipler;

		int32_t iSplitCount;

		// @correction005
		if (iGold >= 3)
			iSplitCount = number(1, 3);
		else if (GetMobRank() >= MOB_RANK_BOSS)
		{
			iSplitCount = number(3, 10);

			if ((iGold / iSplitCount) == 0)
				iSplitCount = 1;
		}
		else
			iSplitCount = 1;

		if (iGold != 0)
		{
			iTotalGold += iGold;

			for (int32_t i = 0; i < iSplitCount; ++i)
			{
				// if (isAutoLoot)
				// {
					if (pkAttacker)
						pkAttacker->GiveGold(iGold / iSplitCount);
				// }
				// else if ((item = ITEM_MANAGER::instance().CreateItem(1, iGold / iSplitCount)))
				// {
					// pos.x = GetX() + (number(-7, 7) * 20);
					// pos.y = GetY() + (number(-7, 7) * 20);

					// item->AddToGround(GetMapIndex(), pos);
					// item->StartDestroyEvent();
				// }
			}
		}
	}

	DBManager::instance().SendMoneyLog(MONEY_LOG_MONSTER, GetRaceNum(), iTotalGold);
}

void CHARACTER::Reward(bool bItemDrop)
{
   	LPCHARACTER pkAttacker = DistributeExp();

	if (!pkAttacker)
		return;

	if (pkAttacker->IsPC())
	{
		if ((GetLevel() - pkAttacker->GetLevel()) >= -10)
		{
			if (pkAttacker->GetRealAlignment() < 0)
			{
				if (pkAttacker->IsEquipUniqueItem(UNIQUE_ITEM_FASTER_ALIGNMENT_UP_BY_KILL))
					pkAttacker->UpdateAlignment(14);
				else
					pkAttacker->UpdateAlignment(7);
			}
			else
				pkAttacker->UpdateAlignment(2);
		}

#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
		if (GetRaceNum() >= 6415 && GetRaceNum() <= 6419)
			RewardAttendance();
#endif

		pkAttacker->SetQuestNPCID(GetVID());
		quest::CQuestManager::instance().Kill(pkAttacker->GetPlayerID(), GetRaceNum());
		CHARACTER_MANAGER::instance().KillLog(GetRaceNum());

		if (!number(0, 9))
		{
			if (pkAttacker->GetPoint(POINT_KILL_HP_RECOVERY))
			{
				int32_t iHP = pkAttacker->GetMaxHP() * pkAttacker->GetPoint(POINT_KILL_HP_RECOVERY) / 100;
				pkAttacker->PointChange(POINT_HP, iHP);
				CreateFly(FLY_HP_SMALL, pkAttacker);
			}

			if (pkAttacker->GetPoint(POINT_KILL_SP_RECOVER))
			{
				int32_t iSP = pkAttacker->GetMaxSP() * pkAttacker->GetPoint(POINT_KILL_SP_RECOVER) / 100;
				pkAttacker->PointChange(POINT_SP, iSP);
				CreateFly(FLY_SP_SMALL, pkAttacker);
			}
		}
	}

	if (!bItemDrop)
		return;

	PIXEL_POSITION pos = GetXYZ();

	if (!SECTREE_MANAGER::instance().GetMovablePosition(GetMapIndex(), pos.x, pos.y, pos))
		return;

	if (test_server)
		sys_log(0, "Drop money : Attacker %s", pkAttacker->GetName());
	RewardGold(pkAttacker);

	LPITEM item;

	static std::vector<LPITEM> s_vec_item;
	s_vec_item.clear();

	if (ITEM_MANAGER::instance().CreateDropItem(this, pkAttacker, s_vec_item))
	{
		if (s_vec_item.size() == 0);
		else if (s_vec_item.size() == 1)
		{
			item = s_vec_item[0];
			item->AddToGround(GetMapIndex(), pos);

			if (CBattleArena::instance().IsBattleArenaMap(pkAttacker->GetMapIndex()) == false)
			{
				item->SetOwnership(pkAttacker);
			}

			item->StartDestroyEvent();

			pos.x = number(-7, 7) * 20;
			pos.y = number(-7, 7) * 20;
			pos.x += GetX();
			pos.y += GetY();

			sys_log(0, "DROP_ITEM: %s %d %d from %s", item->GetName(), pos.x, pos.y, GetName());
		}
		else
		{
			int32_t iItemIdx = s_vec_item.size() - 1;

			std::priority_queue<std::pair<int32_t, LPCHARACTER> > pq;

			int32_t total_dam = 0;

			for (TDamageMap::iterator it = m_map_kDamage.begin(); it != m_map_kDamage.end(); ++it)
			{
				int32_t iDamage = it->second.iTotalDamage;
				if (iDamage > 0)
				{
					LPCHARACTER ch = CHARACTER_MANAGER::instance().Find(it->first);

					if (ch)
					{
						pq.push(std::make_pair(iDamage, ch));
						total_dam += iDamage;
					}
				}
			}

			std::vector<LPCHARACTER> v;

			while (!pq.empty() && pq.top().first * 10 >= total_dam)
			{
				v.push_back(pq.top().second);
				pq.pop();
			}

			if (v.empty())
			{
				while (iItemIdx >= 0)
				{
					item = s_vec_item[iItemIdx--];

					if (!item)
					{
						sys_err("item null in vector idx %d", iItemIdx + 1);
						continue;
					}

					item->AddToGround(GetMapIndex(), pos);
					item->StartDestroyEvent();

					pos.x = number(-7, 7) * 20;
					pos.y = number(-7, 7) * 20;
					pos.x += GetX();
					pos.y += GetY();

					sys_log(0, "DROP_ITEM: %s %d %d by %s", item->GetName(), pos.x, pos.y, GetName());
				}
			}
			else
			{
				std::vector<LPCHARACTER>::iterator it = v.begin();

				while (iItemIdx >= 0)
				{
					item = s_vec_item[iItemIdx--];

					if (!item)
					{
						sys_err("item null in vector idx %d", iItemIdx + 1);
						continue;
					}

					item->AddToGround(GetMapIndex(), pos);

					LPCHARACTER ch = *it;

					if (ch->GetParty())
						ch = ch->GetParty()->GetNextOwnership(ch, GetX(), GetY());

					++it;

					if (it == v.end())
						it = v.begin();

					if (CBattleArena::instance().IsBattleArenaMap(ch->GetMapIndex()) == false)
					{
						item->SetOwnership(ch);
					}

					item->StartDestroyEvent();

					pos.x = number(-7, 7) * 20;
					pos.y = number(-7, 7) * 20;
					pos.x += GetX();
					pos.y += GetY();

					sys_log(0, "DROP_ITEM: %s %d %d by %s", item->GetName(), pos.x, pos.y, GetName());
				}
			}
		}
	}

	m_map_kDamage.clear();

#ifdef __AUTO_PICKUP__
	if (pkAttacker->GetAutoPickup())
	{
		pkAttacker->PickupItem(0);
		pkAttacker->PickupItem(0);
	}
#endif
}

struct TItemDropPenalty
{
	int32_t iInventoryPct;
	int32_t iInventoryQty;
	int32_t iEquipmentPct;
	int32_t iEquipmentQty;
};

TItemDropPenalty aItemDropPenalty_kor[13] =
{
	{   0,   0,  0,  0 },
	{   0,   0,  0,  0 },
	{   0,   0,  0,  0 },
	{   0,   0,  0,  0 },
	{   0,   0,  0,  0 },
	{   0,   0,  0,  0 },
	{   0,   0,  0,  0 },
	{   0,   0,  0,  0 },
	{   0,   0,  0,  0 },
	{  25,   1,  5,  1 },
	{  50,   2, 10,  1 },
	{  75,   4, 15,  1 },
	{ 100,   8, 20,  1 },
};

void CHARACTER::ItemDropPenalty(LPCHARACTER pkKiller)
{
	if (GetMyShop())
		return;

	// @correction005
	if (GetLevel() < 50)
		return;

	if (CBattleArena::instance().IsBattleArenaMap(GetMapIndex()) == true)
	{
		return;
	}

	struct TItemDropPenalty * table = &aItemDropPenalty_kor[0];

	if (GetLevel() < 10)
		return;

	int32_t iAlignIndex;
#ifdef __ALIGN_RENEWAL__
	if (GetRealAlignment() >= 300000)
		iAlignIndex = 0;
	else if (GetRealAlignment() >= 250000)
		iAlignIndex = 1;
	else if (GetRealAlignment() >= 200000)
		iAlignIndex = 2;
	else if (GetRealAlignment() >= 180000)
		iAlignIndex = 3;
	else if (GetRealAlignment() >= 120000)
		iAlignIndex = 4;
	else if (GetRealAlignment() >= 80000)
		iAlignIndex = 5;
	else if (GetRealAlignment() >= 40000)
		iAlignIndex = 6;
	else if (GetRealAlignment() >= 10000)
		iAlignIndex = 7;
	else if (GetRealAlignment() >= 0)
		iAlignIndex = 8;
	else if (GetRealAlignment() > -40000)
		iAlignIndex = 9;
	else if (GetRealAlignment() > -80000)
		iAlignIndex = 10;
	else if (GetRealAlignment() > -120000)
		iAlignIndex = 11;
	else
		iAlignIndex = 12;
#else
	if (GetRealAlignment() >= 120000)
		iAlignIndex = 0;
	else if (GetRealAlignment() >= 80000)
		iAlignIndex = 1;
	else if (GetRealAlignment() >= 40000)
		iAlignIndex = 2;
	else if (GetRealAlignment() >= 10000)
		iAlignIndex = 3;
	else if (GetRealAlignment() >= 0)
		iAlignIndex = 4;
	else if (GetRealAlignment() > -40000)
		iAlignIndex = 5;
	else if (GetRealAlignment() > -80000)
		iAlignIndex = 6;
	else if (GetRealAlignment() > -120000)
		iAlignIndex = 7;
	else
		iAlignIndex = 8;
#endif

	std::vector<std::pair<LPITEM, int32_t> > vec_item;
	LPITEM pkItem;
	int32_t	i;
	bool isDropAllEquipments = false;

	TItemDropPenalty & r = table[iAlignIndex];
	sys_log(0, "%s align %d inven_pct %d equip_pct %d", GetName(), iAlignIndex, r.iInventoryPct, r.iEquipmentPct);

	bool bDropInventory = r.iInventoryPct >= number(1, 1000);
	bool bDropEquipment = r.iEquipmentPct >= number(1, 100);
	bool bDropAntiDropUniqueItem = false;

	if ((bDropInventory || bDropEquipment) && IsEquipUniqueItem(UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY))
	{
		bDropInventory = false;
		bDropEquipment = false;
		bDropAntiDropUniqueItem = true;
	}

	std::random_device rd;
	std::mt19937 g(rd());
	if (bDropInventory)
	{
		std::vector<uint8_t> vec_bSlots;

		for (i = 0; i < INVENTORY_MAX_NUM; ++i)
			if (GetInventoryItem(i))
				vec_bSlots.push_back(i);

		if (!vec_bSlots.empty())
		{
			std::shuffle(vec_bSlots.begin(), vec_bSlots.end(), g);

			int32_t iQty = MIN(vec_bSlots.size(), r.iInventoryQty);

			if (iQty)
				iQty = number(1, iQty);

			for (i = 0; i < iQty; ++i)
			{
				pkItem = GetInventoryItem(vec_bSlots[i]);

				if (!pkItem)
					continue;

				if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_PKDROP))
					continue;

				SyncQuickslot(QUICKSLOT_TYPE_ITEM, vec_bSlots[i], 255);
				vec_item.push_back(std::make_pair(pkItem->RemoveFromCharacter(), INVENTORY));
			}
		}
		else if (iAlignIndex == 8)
			isDropAllEquipments = true;
	}

	if (bDropEquipment)
	{
		std::vector<uint8_t> vec_bSlots;

		for (i = 0; i < WEAR_MAX_NUM; ++i)
			if (GetWear(i))
				vec_bSlots.push_back(i);

		if (!vec_bSlots.empty())
		{
			std::shuffle(vec_bSlots.begin(), vec_bSlots.end(), g);
			int32_t iQty;

			if (isDropAllEquipments)
				iQty = vec_bSlots.size();
			else
				iQty = MIN(vec_bSlots.size(), number(1, r.iEquipmentQty));

			if (iQty)
				iQty = number(1, iQty);

			for (i = 0; i < iQty; ++i)
			{
				pkItem = GetWear(vec_bSlots[i]);

				if (IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_PKDROP))
					continue;

				SyncQuickslot(QUICKSLOT_TYPE_ITEM, vec_bSlots[i], 255);
				vec_item.push_back(std::make_pair(pkItem->RemoveFromCharacter(), EQUIPMENT));
			}
		}
	}

	if (bDropAntiDropUniqueItem)
	{
		LPITEM pkItem;

		pkItem = GetWear(WEAR_UNIQUE1);

		if (pkItem && pkItem->GetVnum() == UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY)
		{
			SyncQuickslot(QUICKSLOT_TYPE_ITEM, WEAR_UNIQUE1, 255);
			vec_item.push_back(std::make_pair(pkItem->RemoveFromCharacter(), EQUIPMENT));
		}

		pkItem = GetWear(WEAR_UNIQUE2);

		if (pkItem && pkItem->GetVnum() == UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY)
		{
			SyncQuickslot(QUICKSLOT_TYPE_ITEM, WEAR_UNIQUE2, 255);
			vec_item.push_back(std::make_pair(pkItem->RemoveFromCharacter(), EQUIPMENT));
		}

#ifdef __COSTUME_MOUNT__
		pkItem = GetWear(WEAR_COSTUME_MOUNT);

		if (pkItem && pkItem->GetVnum() == UNIQUE_ITEM_SKIP_ITEM_DROP_PENALTY)
		{
			SyncQuickslot(QUICKSLOT_TYPE_ITEM, WEAR_COSTUME_MOUNT, 255);
			vec_item.push_back(std::make_pair(pkItem->RemoveFromCharacter(), EQUIPMENT));
		}
#endif
	}

	{
		PIXEL_POSITION pos;
		pos.x = GetX();
		pos.y = GetY();

		uint32_t i;

		for (i = 0; i < vec_item.size(); ++i)
		{
			LPITEM item = vec_item[i].first;
			int32_t window = vec_item[i].second;

			item->AddToGround(GetMapIndex(), pos);
			item->StartDestroyEvent();

			sys_log(0, "DROP_ITEM_PK: %s %d %d from %s", item->GetName(), pos.x, pos.y, GetName());
			LogManager::instance().ItemLog(this, item, "DEAD_DROP", (window == INVENTORY) ? "INVENTORY" : ((window == EQUIPMENT) ? "EQUIPMENT" : ""));

			pos.x = GetX() + number(-7, 7) * 20;
			pos.y = GetY() + number(-7, 7) * 20;
		}
	}
}

class FPartyAlignmentCompute
{
	public:
		FPartyAlignmentCompute(int32_t iAmount, int32_t x, int32_t y)
		{
			m_iAmount = iAmount;
			m_iCount = 0;
			m_iStep = 0;
			m_iKillerX = x;
			m_iKillerY = y;
		}

		void operator () (LPCHARACTER pkChr)
		{
			if (DISTANCE_APPROX(pkChr->GetX() - m_iKillerX, pkChr->GetY() - m_iKillerY) < PARTY_DEFAULT_RANGE)
			{
				if (m_iStep == 0)
				{
					++m_iCount;
				}
				else
				{
					pkChr->UpdateAlignment(m_iAmount / m_iCount);
				}
			}
		}

		int32_t m_iAmount;
		int32_t m_iCount;
		int32_t m_iStep;

		int32_t m_iKillerX;
		int32_t m_iKillerY;
};



void CHARACTER::Dead(LPCHARACTER pkKiller, bool bImmediateDead)
{
	if (IsDead())
		return;

	{
		if (IsHorseRiding())
		{
			StopRiding();
		}
		else if (GetMountVnum())
		{
			RemoveAffect(AFFECT_MOUNT_BONUS);
#ifdef __COSTUME_MOUNT__
#ifndef __MOUNT__
			LPITEM item = GetWear(WEAR_COSTUME_MOUNT);

			if (item && item->IsRideItem())
			{
				CMountSystem* MountSystem = GetMountSystem();
#ifdef __CHANGE_LOOK__
				uint32_t dwVnum = item->GetChangeLook() ? item->GetChangeLook() : item->GetValue(4);
#else
				uint32_t dwVnum = item->GetValue(4);
#endif
				if (MountSystem)
					MountSystem->Unsummon(dwVnum);

				UnequipItem(item);
			}
#endif
#else
			LPITEM item = GetWear(WEAR_UNIQUE1);
			LPITEM item2 = GetWear(WEAR_UNIQUE2);

			if (item && item->IsRideItem())
				UnequipItem(item);

			if (item2 && item2->IsRideItem())
				UnequipItem(item2);
#endif
			m_dwMountVnum = 0;
			UnEquipSpecialRideUniqueItem();

			UpdatePacket();
		}
	}

	if (!pkKiller && m_dwKillerPID)
		pkKiller = CHARACTER_MANAGER::instance().FindByPID(m_dwKillerPID);

	m_dwKillerPID = 0;

	bool isAgreedPVP = false;
	bool isUnderGuildWar = false;
	bool isDuel = false;
	bool isForked = false;

	if (pkKiller && pkKiller->IsPC())
	{
		if (pkKiller->m_pkChrTarget == this)
			pkKiller->SetTarget(nullptr);

		if (!IsPC() && pkKiller->GetDungeon())
			pkKiller->GetDungeon()->IncKillCount(pkKiller, this);

		isAgreedPVP = CPVPManager::instance().Dead(this, pkKiller->GetPlayerID());
		isDuel = CArenaManager::instance().OnDead(pkKiller, this);

		if (IsPC())
		{
			CGuild * g1 = GetGuild();
			CGuild * g2 = pkKiller->GetGuild();

			if (g1 && g2)
			{
				if (g1->UnderWar(g2->GetID()))
				{
					isUnderGuildWar = true;

#ifdef ENABLE_GUILDWAR_KILL_COUNTER // @correction200
					CWarMap * pMap = pkKiller->GetWarMap();
					if (pMap)
						pMap->SendKillNotice(pkKiller->GetName(), GetName(), pkKiller->GetRaceNum(), GetRaceNum(), pkKiller->GetWear(WEAR_WEAPON) ? pkKiller->GetWear(WEAR_WEAPON)->GetVnum() : 0);
#endif
				}
			}

			pkKiller->SetQuestNPCID(GetVID());
			quest::CQuestManager::instance().Kill(pkKiller->GetPlayerID(), quest::QUEST_NO_NPC);
			CGuildManager::instance().Kill(pkKiller, this);
		}
	}

#ifdef ENABLE_QUEST_DIE_EVENT // @correction146
	if (IsPC())
	{
		if (pkKiller)
			SetQuestNPCID(pkKiller->GetVID());

		quest::CQuestManager::instance().Die(GetPlayerID(), (pkKiller) ? pkKiller->GetRaceNum() : (uint32_t)quest::QUEST_NO_NPC);
	}
#endif

	if (IsPC())
	{
		if (CThreeWayWar::instance().IsThreeWayWarMapIndex(GetMapIndex()))
			isForked = true;
	}

#ifdef __KILL_NOTICE__
	if (IsBoss() && !IsPC() && !IsStone() && pkKiller && pkKiller->IsPC())
	{
		auto* pkMob = CMobManager::instance().Get(GetRaceNum());
		if (pkMob)
		{
			char szKillNotice[256];
			snprintf(szKillNotice, sizeof(szKillNotice), "|<Boss>| CH%d: %s, %s tarafýndan katledildi!", g_bChannel, pkMob->m_table.szLocaleName, pkKiller->GetName());
			BroadcastNotice(szKillNotice);
		}
	}
#endif

	if (pkKiller &&
			!isAgreedPVP &&
			!isUnderGuildWar &&
			IsPC() &&
			!isDuel &&
			!isForked &&
			!IS_CASTLE_MAP(GetMapIndex()))
	{
		if (GetGMLevel() == GM_PLAYER || test_server)
		{
			ItemDropPenalty(pkKiller);
		}
	}

	if (IS_CASTLE_MAP(GetMapIndex()))
	{
		if (CASTLE_FROG_VNUM == GetRaceNum())
			castle_frog_die(this, pkKiller);
		else if (castle_is_guard_vnum(GetRaceNum()))
			castle_guard_die(this, pkKiller);
		else if (castle_is_tower_vnum(GetRaceNum()))
			castle_tower_die(this, pkKiller);
	}

	if (true == isForked)
	{
		CThreeWayWar::instance().onDead( this, pkKiller );
	}

	SetPosition(POS_DEAD);
#ifdef __NO_CLEAR_BUFF_WHEN_MONSTER_KILL__
	if (pkKiller && IsPC() && !pkKiller->IsPC())
		ClearAffect(true, true);
	else
		ClearAffect(true);
#else
	ClearAffect(true);
#endif

	if (pkKiller && IsPC())
	{
		if (!pkKiller->IsPC())
		{
			if (!isForked)
			{
				sys_log(1, "DEAD: %s %p WITH PENALTY", GetName(), this);
				SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);
				LogManager::instance().CharLog(this, pkKiller->GetRaceNum(), "DEAD_BY_NPC", pkKiller->GetName());
			}
		}
		else
		{
			sys_log(1, "DEAD_BY_PC: %s %p KILLER %s %p", GetName(), this, pkKiller->GetName(), get_pointer(pkKiller));
			REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);

			if (GetEmpire() != pkKiller->GetEmpire())
			{
				int32_t iEP = MIN(GetPoint(POINT_EMPIRE_POINT), pkKiller->GetPoint(POINT_EMPIRE_POINT));

				PointChange(POINT_EMPIRE_POINT, -(iEP / 10));
				pkKiller->PointChange(POINT_EMPIRE_POINT, iEP / 5);

				if (GetPoint(POINT_EMPIRE_POINT) < 10)
				{
				}

				char buf[256];
				snprintf(buf, sizeof(buf),
						"%d %d %d %s %d %d %d %s",
						GetEmpire(), GetAlignment(), GetPKMode(), GetName(),
						pkKiller->GetEmpire(), pkKiller->GetAlignment(), pkKiller->GetPKMode(), pkKiller->GetName());

				LogManager::instance().CharLog(this, pkKiller->GetPlayerID(), "DEAD_BY_PC", buf);
			}
			else
			{
				if (!isAgreedPVP && !isUnderGuildWar && !IsKillerMode() && GetAlignment() >= 0 && !isDuel && !isForked)
				{
					int32_t iNoPenaltyProb = 0;

					if (pkKiller->GetAlignment() >= 0)
						iNoPenaltyProb = 33;
					else
						iNoPenaltyProb = 20;

					if (number(1, 100) < iNoPenaltyProb)
						pkKiller->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ë½ÅÀÇ º¸È£·Î ¾ÆÀÌÅÛÀÌ ¶³¾îÁöÁö ¾Ê¾Ò½À´Ï´Ù."));
					else
					{
						if (pkKiller->GetParty())
						{
							FPartyAlignmentCompute f(-20000, pkKiller->GetX(), pkKiller->GetY());
							pkKiller->GetParty()->ForEachOnlineMember(f);

							if (f.m_iCount == 0)
								pkKiller->UpdateAlignment(-20000);
							else
							{
								sys_log(0, "ALIGNMENT PARTY count %d amount %d", f.m_iCount, f.m_iAmount);

								f.m_iStep = 1;
								pkKiller->GetParty()->ForEachOnlineMember(f);
							}
						}
						else
							pkKiller->UpdateAlignment(-20000);
					}
				}

				char buf[256];
				snprintf(buf, sizeof(buf),
						"%d %d %d %s %d %d %d %s",
						GetEmpire(), GetAlignment(), GetPKMode(), GetName(),
						pkKiller->GetEmpire(), pkKiller->GetAlignment(), pkKiller->GetPKMode(), pkKiller->GetName());

				LogManager::instance().CharLog(this, pkKiller->GetPlayerID(), "DEAD_BY_PC", buf);
			}
		}
	}
	else
	{
		sys_log(1, "DEAD: %s %p", GetName(), this);
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_DEATH_PENALTY);
	}

	ClearSync();

	event_cancel(&m_pkStunEvent);

	if (IsPC())
	{
		m_dwLastDeadTime = get_dword_time();
		SetKillerMode(false);
		GetDesc()->SetPhase(PHASE_DEAD);
	}
	else
	{
		if (!IS_SET(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD))
		{
			if (!(pkKiller && pkKiller->IsPC() && pkKiller->GetGuild() && pkKiller->GetGuild()->UnderAnyWar(GUILD_WAR_TYPE_FIELD)))
			{
				if (GetMobTable().dwResurrectionVnum)
				{
					LPCHARACTER chResurrect = CHARACTER_MANAGER::instance().SpawnMob(GetMobTable().dwResurrectionVnum, GetMapIndex(), GetX(), GetY(), GetZ(), true, (int32_t) GetRotation());
					if (GetDungeon() && chResurrect)
					{
						chResurrect->SetDungeon(GetDungeon());
					}

					Reward(false);
				}
				else if (IsRevive() == true)
				{
					Reward(false);
				}
				else
				{
					Reward(true);
				}
			}
			else
			{
				if (pkKiller->m_dwUnderGuildWarInfoMessageTime < get_dword_time())
				{
					pkKiller->m_dwUnderGuildWarInfoMessageTime = get_dword_time() + 60000;
					pkKiller->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<±æµå> ±æµåÀüÁß¿¡´Â »ç³É¿¡ µû¸¥ ÀÌÀÍÀÌ ¾ø½À´Ï´Ù."));
				}
			}
		}
	}

	if (GetMobRank() >= MOB_RANK_BOSS && pkKiller && pkKiller->IsPC())
	{
		char buf[51];
		snprintf(buf, sizeof(buf), "%u %d", g_bChannel, pkKiller->GetMapIndex());
		if (IsStone())
			LogManager::instance().CharLog(pkKiller, GetRaceNum(), "STONE_KILL", buf);
		else
			LogManager::instance().CharLog(pkKiller, GetRaceNum(), "BOSS_KILL", buf);
	}

	TPacketGCDead pack;
	pack.header	= HEADER_GC_DEAD;
	pack.vid	= m_vid;
	PacketAround(&pack, sizeof(pack));

	REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN);

	if (GetDesc() != nullptr) {
		itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

		while (it != m_list_pkAffect.end())
			SendAffectAddPacket(GetDesc(), *it++);
	}

	if (isDuel == false)
	{
		if (m_pkDeadEvent)
		{
			sys_log(1, "DEAD_EVENT_CANCEL: %s %p %p", GetName(), this, get_pointer(m_pkDeadEvent));
			event_cancel(&m_pkDeadEvent);
		}

		if (IsStone())
			ClearStone();

		if (GetDungeon())
		{
			GetDungeon()->DeadCharacter(this);
		}

		SCharDeadEventInfo* pEventInfo = AllocEventInfo<SCharDeadEventInfo>();

		if (IsPC())
		{
			pEventInfo->isPC = true;
			pEventInfo->dwID = this->GetPlayerID();

			m_pkDeadEvent = event_create(dead_event, pEventInfo, PASSES_PER_SEC(180));
		}
		else
		{
			pEventInfo->isPC = false;
			pEventInfo->dwID = this->GetVID();

			if (IsRevive() == false && HasReviverInParty() == true)
			{
				m_pkDeadEvent = event_create(dead_event, pEventInfo, bImmediateDead ? 1 : PASSES_PER_SEC(1));
			}
			else
			{
#ifdef __DEFENSE_WAVE__
				if (GetRaceNum() >= 3950 && GetRaceNum() <= 3964)
					m_pkDeadEvent = event_create(dead_event, pEventInfo, bImmediateDead ? 1 : PASSES_PER_SEC(1));
				else
					m_pkDeadEvent = event_create(dead_event, pEventInfo, bImmediateDead ? 1 : PASSES_PER_SEC(1));
#else
				m_pkDeadEvent = event_create(dead_event, pEventInfo, bImmediateDead ? 1 : PASSES_PER_SEC(1));
#endif
			}
		}

		sys_log(1, "DEAD_EVENT_CREATE: %s %p %p", GetName(), this, get_pointer(m_pkDeadEvent));
	}

	if (m_pkExchange != nullptr)
	{
		m_pkExchange->Cancel();
	}

	if (IsCubeOpen() == true)
		Cube_close(this);

#ifdef __ACCE_SYSTEM__ // @correction191
	if (IsPC())
		CloseAcce();
#endif
#ifdef __CHANGE_LOOK__ // @correction192
	if (IsPC())
		ChangeLookWindow(false, true);
#endif
	CShopManager::instance().StopShopping(this);
	CloseMyShop();
	CloseSafebox();

	if (true == IsMonster() && 2493 == GetMobTable().dwVnum)
	{
		if (nullptr != pkKiller && nullptr != pkKiller->GetGuild())
		{
			CDragonLairManager::instance().OnDragonDead( this, pkKiller->GetGuild()->GetID() );
		}
		else
		{
			sys_err("DragonLair: Dragon killed by nobody");
		}
	}

#ifdef __GUILD_DRAGONLAIR__
	if ((IsStone()) || (IsMonster()))
	{
		if (pkKiller)
		{
			if (((GetMobTable().dwVnum == (uint32_t)(MOBVNUM_RESPAWN_STONE_STEP2)) || (GetMobTable().dwVnum == (uint32_t)(MOBVNUM_RESPAWN_BOSS_STEP3))) && (CMgr::instance().IsMeleyMap(pkKiller->GetMapIndex())))
				CMgr::instance().OnKill(GetMobTable().dwVnum, pkKiller);
			else if (CMgr::instance().IsMeleyMap(pkKiller->GetMapIndex()))
				CMgr::instance().OnKillCommon(this, pkKiller);
		}
	}
#endif
#ifdef __TEMPLE_OCHAO__
	if (IsMonster())
		if ((GetMobTable().dwVnum == TEMPLE_OCHAO_GUARDIAN) && (pkKiller && pkKiller->GetMapIndex() == TEMPLE_OCHAO_MAP_INDEX))
			TempleOchao::CMgr::instance().OnGuardianKilled(pkKiller->GetX(), pkKiller->GetY(), pkKiller->GetZ());
#endif
#ifdef __BOSS_TRACKING_SYSTEM__
	if (IsMonster())
	{
		if (!IsPC() && pkKiller && pkKiller->IsPC())
		{
			//if (IsMonster() == true && quest::CQuestManager::instance().GetEventFlag("enable_boss_tracking") == 0)
			{
				if (CBossTracking::instance().IsTrackingMob(GetMobTable().dwVnum) == true)
				{
					LPREGEN pkRegen = GetRegen();
					if (pkRegen && pkRegen->time)
					{
						CBossTracking::instance().SetDeadTime(g_bChannel, GetMobTable().dwVnum, get_global_time(), pkKiller->GetMapIndex());
						CBossTracking::instance().SetRegenTime(g_bChannel, GetMobTable().dwVnum, /*CBossTracking::instance().RegenTimeGetNew(GetMobTable().dwVnum)*/ pkRegen->time, pkKiller->GetMapIndex());
						CBossTracking::instance().SendP2PPacket(GetMobTable().dwVnum, g_bChannel);
					}
				}
			}
		}
	}
#endif
}

struct FuncSetLastAttacked
{
	FuncSetLastAttacked(uint32_t dwTime) : m_dwTime(dwTime)
	{
	}

	void operator () (LPCHARACTER ch)
	{
		ch->SetLastAttacked(m_dwTime);
	}

	uint32_t m_dwTime;
};

void CHARACTER::SetLastAttacked(uint32_t dwTime)
{
	assert(m_pkMobInst != nullptr);

	m_pkMobInst->m_dwLastAttackedTime = dwTime;
	m_pkMobInst->m_posLastAttacked = GetXYZ();
}

void CHARACTER::SendDamagePacket(LPCHARACTER pAttacker, int32_t Damage, uint8_t DamageFlag)
{
	if (IsPC() == true || (pAttacker->IsPC() == true && pAttacker->GetTarget() == this))
	{
		TPacketGCDamageInfo damageInfo;
		memset(&damageInfo, 0, sizeof(TPacketGCDamageInfo));

		damageInfo.header = HEADER_GC_DAMAGE_INFO;
		damageInfo.dwVID = (uint32_t)GetVID();
		damageInfo.flag = DamageFlag;
		damageInfo.damage = Damage;

		if (GetDesc() != nullptr)
		{
			GetDesc()->Packet(&damageInfo, sizeof(TPacketGCDamageInfo));
		}

		if (pAttacker->GetDesc() != nullptr)
		{
			pAttacker->GetDesc()->Packet(&damageInfo, sizeof(TPacketGCDamageInfo));
		}
	}
}


bool CHARACTER::Damage(LPCHARACTER pAttacker, int32_t _dam, EDamageType type)
{
	int64_t dam = _dam;
	// sys_err("bas: %d", dam);
	if (IsPet())
		return false;

#ifdef __MOUNT__
	if (IsMountSystem())
		return false;
#endif

#ifdef __SUPPORT__
	if (IsSupport())
		return false;
#endif

#ifdef __GROWTH_PET__
	if (IsImmortal())
		return false;
#endif

#ifdef __GUILD_DRAGONLAIR__
	if (pAttacker)
	{
		if ((GetRaceNum() == (uint16_t)(STATUE_VNUM)) && (CMgr::instance().IsMeleyMap(pAttacker->GetMapIndex())) && (!CMgr::instance().Damage(this, pAttacker)))
		{
			SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
			return false;
		}
		else if ((GetRaceNum() == (uint16_t)(BOSS_VNUM)) && (CMgr::instance().IsMeleyMap(pAttacker->GetMapIndex())))
		{
			SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
			return false;
		}
		else if ((GetRaceNum() == (uint16_t)(STATUE_VNUM)))
		{
			if (GetHP() - dam <= 0)
			{
				SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
				return false;
			}
		}
	}
#endif

#ifdef ENABLE_NEWSTUFF //@correction232
	if (pAttacker && IsStone() && pAttacker->IsPC())
	{
		if (GetEmpire() && GetEmpire() == pAttacker->GetEmpire())
		{
			SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
			return false;
		}
	}
#endif
	if (DAMAGE_TYPE_MAGIC == type)
	{
		dam = (int64_t)((float)dam * (100 + (pAttacker->GetPoint(POINT_MAGIC_ATT_BONUS_PER) + pAttacker->GetPoint(POINT_MELEE_MAGIC_ATT_BONUS_PER))) / 100.f + 0.5f);
		// sys_err("-4: %d", dam);
	}
	if (GetRaceNum() == 5001)
	{
		bool bDropMoney = false;

// @correction056 BEGIN
		int32_t iPercent = 0;
		if (GetMaxHP() >= 0)
			iPercent = (GetHP() * 100) / GetMaxHP();
// @correction056 END

		if (iPercent <= 10 && GetMaxSP() < 5)
		{
			SetMaxSP(5);
			bDropMoney = true;
		}
		else if (iPercent <= 20 && GetMaxSP() < 4)
		{
			SetMaxSP(4);
			bDropMoney = true;
		}
		else if (iPercent <= 40 && GetMaxSP() < 3)
		{
			SetMaxSP(3);
			bDropMoney = true;
		}
		else if (iPercent <= 60 && GetMaxSP() < 2)
		{
			SetMaxSP(2);
			bDropMoney = true;
		}
		else if (iPercent <= 80 && GetMaxSP() < 1)
		{
			SetMaxSP(1);
			bDropMoney = true;
		}

		if (bDropMoney)
		{
			uint32_t dwGold = 1000;
			int32_t iSplitCount = number(10, 13);

			sys_log(0, "WAEGU DropGoldOnHit %d times", GetMaxSP());

			// for (int32_t i = 1; i <= iSplitCount; ++i)
			// {
				if (pAttacker)
					pAttacker->GiveGold(dwGold);
				// PIXEL_POSITION pos;
				// LPITEM item;

				// if ((item = ITEM_MANAGER::instance().CreateItem(1, dwGold / iSplitCount)))
				// {
					// if (i != 0)
					// {
						// pos.x = (number(-14, 14) + number(-14, 14)) * 20;
						// pos.y = (number(-14, 14) + number(-14, 14)) * 20;

						// pos.x += GetX();
						// pos.y += GetY();
					// }

					// item->AddToGround(GetMapIndex(), pos);
					// item->StartDestroyEvent();
				// }
			// }
		}
	}

	if (type != DAMAGE_TYPE_NORMAL && type != DAMAGE_TYPE_NORMAL_RANGE)
	{
		if (IsAffectFlag(AFF_TERROR))
		{
			int32_t pct = GetSkillPower(SKILL_TERROR) / 400;

			if (number(1, 100) <= pct)
				return false;
		}
	}

	int32_t iCurHP = GetHP();
	int32_t iCurSP = GetSP();

	bool IsCritical = false;
	bool IsPenetrate = false;
	bool IsDeathBlow = false;


	if (type == DAMAGE_TYPE_MELEE || type == DAMAGE_TYPE_RANGE || type == DAMAGE_TYPE_MAGIC)
	{
		if (pAttacker)
		{
			int32_t iCriticalPct = pAttacker->GetPoint(POINT_CRITICAL_PCT);

			if (!IsPC())
				iCriticalPct += pAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_CRITICAL_BONUS);

			if (iCriticalPct)
			{
				if (iCriticalPct >= 10)
					iCriticalPct = 5 + (iCriticalPct - 10) / 4;
				else
					iCriticalPct /= 2;

				iCriticalPct -= GetPoint(POINT_RESIST_CRITICAL);

				if (number(1, 100) <= iCriticalPct)
				{
					IsCritical = true;
					dam *= 2;
					// sys_err("-3: %d", dam);
					EffectPacket(SE_CRITICAL);

					if (IsAffectFlag(AFF_MANASHIELD))
					{
						RemoveAffect(AFF_MANASHIELD);
					}
				}
			}

			int32_t iPenetratePct = pAttacker->GetPoint(POINT_PENETRATE_PCT);

			if (!IsPC())
				iPenetratePct += pAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_PENETRATE_BONUS);


			if (iPenetratePct)
			{
				{
					CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_RESIST_PENETRATE);

					if (nullptr != pkSk)
					{
						pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_RESIST_PENETRATE) / 100.0f);

						iPenetratePct -= static_cast<int32_t>(pkSk->kPointPoly.Eval());
					}
				}

				if (iPenetratePct >= 10)
				{
					iPenetratePct = 5 + (iPenetratePct - 10) / 4;
				}
				else
				{
					iPenetratePct /= 2;
				}

				iPenetratePct -= GetPoint(POINT_RESIST_PENETRATE);

				if (number(1, 100) <= iPenetratePct)
				{
					IsPenetrate = true;

					if (test_server)
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°üÅë Ãß°¡ µ¥¹ÌÁö %d"), GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100);

					dam += GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100;
					// sys_err("bas: %-33", dam);

					if (IsAffectFlag(AFF_MANASHIELD))
					{
						RemoveAffect(AFF_MANASHIELD);
					}
#ifdef ENABLE_EFFECT_PENETRATE // @correction169
					EffectPacket(SE_PENETRATE);
#endif
				}
			}
		}
	}
	else if (type == DAMAGE_TYPE_NORMAL || type == DAMAGE_TYPE_NORMAL_RANGE)
	{
		if (type == DAMAGE_TYPE_NORMAL)
		{
			if (GetPoint(POINT_BLOCK) && number(1, 100) <= GetPoint(POINT_BLOCK))
			{
				if (test_server)
				{
					pAttacker->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ºí·°! (%d%%)"), GetName(), GetPoint(POINT_BLOCK));
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s ºí·°! (%d%%)"), GetName(), GetPoint(POINT_BLOCK));
				}

				SendDamagePacket(pAttacker, 0, DAMAGE_BLOCK);
				return false;
			}
		}
		else if (type == DAMAGE_TYPE_NORMAL_RANGE)
		{
			if (GetPoint(POINT_DODGE) && number(1, 100) <= GetPoint(POINT_DODGE))
			{
				if (test_server)
				{
					pAttacker->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s È¸ÇÇ! (%d%%)"), GetName(), GetPoint(POINT_DODGE));
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s È¸ÇÇ! (%d%%)"), GetName(), GetPoint(POINT_DODGE));
				}

				SendDamagePacket(pAttacker, 0, DAMAGE_DODGE);
				return false;
			}
		}

		if (IsAffectFlag(AFF_JEONGWIHON))
			dam = (int64_t) (dam * (100 + GetSkillPower(SKILL_JEONGWI) * 25 / 100) / 100);

		if (IsAffectFlag(AFF_TERROR))
			dam = (int64_t) (dam * (95 - GetSkillPower(SKILL_TERROR) / 5) / 100);

		if (IsAffectFlag(AFF_HOSIN))
			dam = dam * (100 - GetPoint(POINT_RESIST_NORMAL_DAMAGE)) / 100;

		// sys_err("-2: %d", dam);
		if (pAttacker)
		{
			if (type == DAMAGE_TYPE_NORMAL)
			{
				if (GetPoint(POINT_REFLECT_MELEE))
				{
					int64_t reflectDamage = dam * GetPoint(POINT_REFLECT_MELEE) / 100;

					if (pAttacker->IsImmune(IMMUNE_REFLECT))
						reflectDamage = int64_t(reflectDamage / 3.0f + 0.5f);

					pAttacker->Damage(this, reflectDamage, DAMAGE_TYPE_SPECIAL);
				}
			}

			int32_t iCriticalPct = pAttacker->GetPoint(POINT_CRITICAL_PCT);

			if (!IsPC())
				iCriticalPct += pAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_CRITICAL_BONUS);

			if (iCriticalPct)
			{
				iCriticalPct -= GetPoint(POINT_RESIST_CRITICAL);

				if (number(1, 100) <= iCriticalPct)
				{
					IsCritical = true;
					dam *= 2;
					// sys_err("-1: %d", dam);
					EffectPacket(SE_CRITICAL);
				}
			}

			int32_t iPenetratePct = pAttacker->GetPoint(POINT_PENETRATE_PCT);

			if (!IsPC())
				iPenetratePct += pAttacker->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_PENETRATE_BONUS);

			{
				CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_RESIST_PENETRATE);

				if (nullptr != pkSk)
				{
					pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_RESIST_PENETRATE) / 100.0f);

					iPenetratePct -= static_cast<int32_t>(pkSk->kPointPoly.Eval());
				}
			}


			if (iPenetratePct)
			{

				iPenetratePct -= GetPoint(POINT_RESIST_PENETRATE);

				if (number(1, 100) <= iPenetratePct)
				{
					IsPenetrate = true;

					if (test_server)
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°üÅë Ãß°¡ µ¥¹ÌÁö %d"), GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100);
					dam += GetPoint(POINT_DEF_GRADE) * (100 + GetPoint(POINT_DEF_BONUS)) / 100;
					// sys_err("-11: %d", dam);
#ifdef ENABLE_EFFECT_PENETRATE // @correction169
					EffectPacket(SE_PENETRATE);
#endif
				}
			}

			if (pAttacker->GetPoint(POINT_STEAL_HP))
			{
				int32_t pct = 1;

				if (number(1, 10) <= pct)
				{
					int32_t iHP = MIN(dam, MAX(0, iCurHP)) * pAttacker->GetPoint(POINT_STEAL_HP) / 100;

					if (iHP > 0 && GetHP() >= iHP)
					{
						CreateFly(FLY_HP_SMALL, pAttacker);
						pAttacker->PointChange(POINT_HP, iHP);
						PointChange(POINT_HP, -iHP);
					}
				}
			}

			if (pAttacker->GetPoint(POINT_STEAL_SP))
			{
				int32_t pct = 1;

				if (number(1, 10) <= pct)
				{
					int32_t iCur;

					if (IsPC())
						iCur = iCurSP;
					else
						iCur = iCurHP;

					int32_t iSP = MIN(dam, MAX(0, iCur)) * pAttacker->GetPoint(POINT_STEAL_SP) / 100;

					if (iSP > 0 && iCur >= iSP)
					{
						CreateFly(FLY_SP_SMALL, pAttacker);
						pAttacker->PointChange(POINT_SP, iSP);

						if (IsPC())
							PointChange(POINT_SP, -iSP);
					}
				}
			}

			if (pAttacker->GetPoint(POINT_STEAL_GOLD))
			{
				if (number(1, 100) <= pAttacker->GetPoint(POINT_STEAL_GOLD))
				{
					int32_t iAmount = number(1, GetLevel());
					pAttacker->ChangeGold(true, iAmount);
					DBManager::instance().SendMoneyLog(MONEY_LOG_MISC, 1, iAmount);
				}
			}

			if (pAttacker->GetPoint(POINT_HIT_HP_RECOVERY) && number(0, 4) > 0)
			{
				int32_t i = ((iCurHP >= 0) ? MIN(dam, iCurHP) : dam) * pAttacker->GetPoint(POINT_HIT_HP_RECOVERY) / 100; // @correction034

				if (i)
				{
					CreateFly(FLY_HP_SMALL, pAttacker);
					pAttacker->PointChange(POINT_HP, i);
				}
			}

			if (pAttacker->GetPoint(POINT_HIT_SP_RECOVERY) && number(0, 4) > 0)
			{
				int32_t i = ((iCurHP >= 0) ? MIN(dam, iCurHP) : dam) * pAttacker->GetPoint(POINT_HIT_SP_RECOVERY) / 100; // @correction034

				if (i)
				{
					CreateFly(FLY_SP_SMALL, pAttacker);
					pAttacker->PointChange(POINT_SP, i);
				}
			}

			if (pAttacker->GetPoint(POINT_MANA_BURN_PCT))
			{
				if (number(1, 100) <= pAttacker->GetPoint(POINT_MANA_BURN_PCT))
					PointChange(POINT_SP, -50);
			}
		}
	}

	switch (type)
	{
		case DAMAGE_TYPE_NORMAL:
		case DAMAGE_TYPE_NORMAL_RANGE:
			// if (pAttacker)
				// sys_err("0: %d %d %d", dam, dam * (100 + pAttacker->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS)) / 100, dam * ((100 + pAttacker->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS)) / 100));
			if (pAttacker)
				if (pAttacker->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS))
					dam = dam * (100 + pAttacker->GetPoint(POINT_NORMAL_HIT_DAMAGE_BONUS)) / 100;

			// if (pAttacker)
				// sys_err("1: %d %d %d", dam, dam * (100 - MIN(99, GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS))) / 100, dam * ((100 - MIN(99, GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS))) / 100));
			dam = dam * (100 - MIN(99, GetPoint(POINT_NORMAL_HIT_DEFEND_BONUS))) / 100;
			// if (pAttacker)
				// sys_err("2: %d", dam);
			break;

		case DAMAGE_TYPE_MELEE:
		case DAMAGE_TYPE_RANGE:
		case DAMAGE_TYPE_FIRE:
		case DAMAGE_TYPE_ICE:
		case DAMAGE_TYPE_ELEC:
		case DAMAGE_TYPE_MAGIC:
			// sys_err("skll: %d", dam);
			if (pAttacker)
				if (pAttacker->GetPoint(POINT_SKILL_DAMAGE_BONUS))
					dam = dam * (100 + pAttacker->GetPoint(POINT_SKILL_DAMAGE_BONUS)) / 100;

			// sys_err("sklll: %d", dam);
			dam = dam * (100 - MIN(99, GetPoint(POINT_SKILL_DEFEND_BONUS))) / 100;
			// sys_err("sklll: %d", dam);
			break;

		default:
			break;
	}

	if (IsAffectFlag(AFF_MANASHIELD))
	{
		int64_t iDamageSPPart = dam / 3;
		int64_t iDamageToSP = iDamageSPPart * GetPoint(POINT_MANASHIELD) / 100;
		int64_t iSP = GetSP();

		if (iDamageToSP <= iSP)
		{
			PointChange(POINT_SP, -iDamageToSP);
			dam -= iDamageSPPart;
		}
		else
		{
			PointChange(POINT_SP, -GetSP());
			dam -= iSP * 100 / MAX(GetPoint(POINT_MANASHIELD), 1);
		}
	}

	if (GetPoint(POINT_MALL_DEFBONUS) > 0)
	{
		int64_t dec_dam = MIN(200, dam * GetPoint(POINT_MALL_DEFBONUS) / 100);
		dam -= dec_dam;
		// sys_err("3: %d", dam);
	}

	if (pAttacker)
	{
		if (pAttacker->GetPoint(POINT_MALL_ATTBONUS) > 0)
		{
			// sys_err("4: %d", dam);
			int64_t add_dam = MIN(300, dam * pAttacker->GetLimitPoint(POINT_MALL_ATTBONUS) / 100);
			dam += add_dam;
			// sys_err("44: %d", dam);
		}

		if (pAttacker->IsPC())
		{
			int32_t iEmpire = pAttacker->GetEmpire();
			int32_t lMapIndex = pAttacker->GetMapIndex();
			int32_t iMapEmpire = SECTREE_MANAGER::instance().GetEmpireFromMapIndex(lMapIndex);

			// @correction005
			if (iEmpire && iMapEmpire && iEmpire != iMapEmpire)
			{
				dam = dam * 9 / 10;
				// sys_err("5: %d", dam);
			}

			if (!IsPC() && GetMonsterDrainSPPoint())
			{
				int32_t iDrain = GetMonsterDrainSPPoint();

				if (iDrain <= pAttacker->GetSP())
					pAttacker->PointChange(POINT_SP, -iDrain);
				else
				{
					int32_t iSP = pAttacker->GetSP();
					pAttacker->PointChange(POINT_SP, -iSP);
				}
			}

		}
		else if (pAttacker->IsGuardNPC())
		{
			SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_NO_REWARD);
			Stun();
			return true;
		}

		if (pAttacker->IsPC() && CMonarch::instance().IsPowerUp(pAttacker->GetEmpire()))
		{
			dam += dam / 10;
		}

		if (IsPC() && CMonarch::instance().IsDefenceUp(GetEmpire()))
		{
			dam -= dam / 10;
		}
	}

	if (!GetSectree() || GetSectree()->IsAttr(GetX(), GetY(), ATTR_BANPK))
		return false;

	if (!IsPC())
	{
		if (m_pkParty && m_pkParty->GetLeader())
			m_pkParty->GetLeader()->SetLastAttacked(get_dword_time());
		else
			SetLastAttacked(get_dword_time());

		MonsterChat(MONSTER_CHAT_ATTACKED);
	}

	if (IsStun())
	{
		Dead(pAttacker);
		return true;
	}

	if (IsDead())
		return true;

	if (type == DAMAGE_TYPE_POISON)
	{
		if (GetHP() - dam <= 0)
		{
			dam = GetHP() - 1;
		}
	}
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	else if (type == DAMAGE_TYPE_BLEEDING)
	{
		if (GetHP() - dam <= 0)
		{
			dam = GetHP();
		}
	}
#endif
	// @correction005
	if (pAttacker && pAttacker->IsPC())
	{
		int64_t iDmgPct = CHARACTER_MANAGER::instance().GetUserDamageRate(pAttacker);
		dam = dam * iDmgPct / 100;
		// sys_err("6: %d", dam);
	}

	if (IsMonster() && IsStoneSkinner())
	{
		if (GetHPPct() < GetMobTable().bStoneSkinPoint)
			dam /= 2;
	}

	if (pAttacker)
	{
		if (pAttacker->IsMonster() && pAttacker->IsDeathBlower())
		{
			if (pAttacker->IsDeathBlow())
			{
				if (number(1, 4) == GetJob())
				{
					IsDeathBlow = true;
					dam = dam * 4;
					// sys_err("7: %d", dam);
				}
			}
		}

		dam = BlueDragon_Damage(this, pAttacker, dam);
		// sys_err("8: %d", dam);

		uint8_t damageFlag = 0;

		if (type == DAMAGE_TYPE_POISON)
			damageFlag = DAMAGE_POISON;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
#ifndef USE_MOB_BLEEDING_AS_POISON
		else if (type == DAMAGE_TYPE_BLEEDING)
			damageFlag = DAMAGE_BLEEDING;
#else
		else if (type == DAMAGE_TYPE_BLEEDING)
			damageFlag = DAMAGE_POISON;
#endif
#endif
		else
			damageFlag = DAMAGE_NORMAL;

		if (IsCritical == true)
			damageFlag |= DAMAGE_CRITICAL;

		if (IsPenetrate == true)
			damageFlag |= DAMAGE_PENETRATE;


		float damMul = this->GetDamMul();
		double tempDam = dam;
		dam = tempDam * damMul + 0.5f;
		// sys_err("9: %d", dam);
#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
		if (GetRaceNum() >= 6415 && GetRaceNum() <= 6419)
			dam = 1;
#endif
#ifdef __12ZI__
		if ((GetRaceNum() >= 20452 && GetRaceNum() <= 20464) || GetRaceNum() == 2905 || GetRaceNum() == 2906)
			dam = 1;
#endif

		if (pAttacker)
			SendDamagePacket(pAttacker, (int32_t)dam, damageFlag);

		if (test_server)
		{
// @correction056 BEGIN
			int32_t iTmpPercent = 0;
			if (GetMaxHP() >= 0)
				iTmpPercent = (GetHP() * 100) / GetMaxHP();
// @correction056 END

			if(pAttacker)
			{
				pAttacker->ChatPacket(CHAT_TYPE_INFO, "-> %s, DAM %d HP %d(%d%%) %s%s",
						GetName(),
						dam,
						GetHP(),
						iTmpPercent, // @correction056
						IsCritical ? "crit " : "",
						IsPenetrate ? "pene " : "",
						IsDeathBlow ? "deathblow " : "");
			}

			ChatPacket(CHAT_TYPE_PARTY, "<- %s, DAM %d HP %d(%d%%) %s%s",
					pAttacker ? pAttacker->GetName() : 0,
					dam,
					GetHP(),
					iTmpPercent, // @correction056
					IsCritical ? "crit " : "",
					IsPenetrate ? "pene " : "",
					IsDeathBlow ? "deathblow " : "");
		}

		if (m_bDetailLog)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s[%d]°¡ °ø°Ý À§Ä¡: %d %d"), pAttacker->GetName(), (uint32_t) pAttacker->GetVID(), pAttacker->GetX(), pAttacker->GetY());
		}
	}

	if (test_server && pAttacker)
		pAttacker->ChatPacket(CHAT_TYPE_INFO, "%d %d", dam, GetHP());
	if (!cannot_dead)
	{
// @correction057 BEGIN
		if (GetHP() - dam <= 0)
		{
			dam = GetHP();
#ifdef __GUILD_DRAGONLAIR__
			if ((GetRaceNum() == (uint16_t)(STATUE_VNUM)) && (CMgr::instance().IsMeleyMap(pAttacker->GetMapIndex())))
				dam = GetHP() - 1;
#endif
		}
// @correction057 END
		PointChange(POINT_HP, -dam, false);
		if (test_server && pAttacker)
			pAttacker->ChatPacket(CHAT_TYPE_INFO, "%d %d", dam, GetHP());
	}

	if (test_server && pAttacker)
		pAttacker->ChatPacket(CHAT_TYPE_INFO, "%d %d", dam, GetHP());

	if (pAttacker && dam > 0 && IsNPC())
	{
		TDamageMap::iterator it = m_map_kDamage.find(pAttacker->GetVID());

		if (it == m_map_kDamage.end())
		{
			m_map_kDamage.insert(TDamageMap::value_type(pAttacker->GetVID(), TBattleInfo(dam, 0)));
			it = m_map_kDamage.find(pAttacker->GetVID());
		}
		else
		{
			it->second.iTotalDamage += dam;
		}
#ifdef __DEFENSE_WAVE__
		if (GetRaceNum() != 20434)
#endif
#ifdef __12ZI__
		if (!(GetRaceNum() >= 20452 && GetRaceNum() <= 20464) && GetRaceNum() != 2905 && GetRaceNum() != 2906)
#endif
		{
			StartRecoveryEvent();
			UpdateAggrPointEx(pAttacker, type, dam, it->second);
		}
	}
	if (test_server && pAttacker)
		pAttacker->ChatPacket(CHAT_TYPE_INFO, "%d %d", dam, GetHP());

	if (GetHP() <= 0)
	{
		Stun();

		if (pAttacker && !pAttacker->IsNPC())
			m_dwKillerPID = pAttacker->GetPlayerID();
		else
			m_dwKillerPID = 0;
	}

#ifdef __DEFENSE_WAVE__
	if (GetRaceNum() == 20434)
		if (GetDungeon())
			GetDungeon()->UpdateMastHP();
#endif

	return false;
}

void CHARACTER::DistributeHP(LPCHARACTER pkKiller)
{
	if (pkKiller->GetDungeon())
		return;
}

#ifdef ENABLE_NEWEXP_CALCULATION // @correction168
#define NEW_GET_LVDELTA(me, victim) aiPercentByDeltaLev[MINMAX(0, (victim + 15) - me, MAX_EXP_DELTA_OF_LEV - 1)]
typedef long double rate_t;
static void GiveExp(LPCHARACTER from, LPCHARACTER to, int32_t iExp)
{
	if (test_server && iExp < 0)
	{
		to->ChatPacket(CHAT_TYPE_INFO, "exp(%d) overflow", iExp);
		return;
	}
	rate_t lvFactor = static_cast<rate_t>(NEW_GET_LVDELTA(to->GetLevel(), from->GetLevel())) / 100.0L;
	iExp *= lvFactor;
	int32_t iBaseExp = iExp;
	rate_t rateFactor = 100;
	if (distribution_test_server)
		rateFactor *= 3;

	rateFactor += CPrivManager::instance().GetPriv(to, PRIV_EXP_PCT);
	if (to->IsEquipUniqueItem(UNIQUE_ITEM_LARBOR_MEDAL))
		rateFactor += 20;
	if (to->GetMapIndex() >= 660000 && to->GetMapIndex() < 670000)
		rateFactor += 20;
	if (to->GetPoint(POINT_EXP_DOUBLE_BONUS))
		if (number(1, 100) <= to->GetPoint(POINT_EXP_DOUBLE_BONUS))
			rateFactor += 30;
	if (to->IsEquipUniqueItem(UNIQUE_ITEM_DOUBLE_EXP))
		rateFactor += 50;

	switch (to->GetMountVnum())
	{
		case 20110:
		case 20111:
		case 20112:
		case 20113:
			if (to->IsEquipUniqueItem(71115) || to->IsEquipUniqueItem(71117) || to->IsEquipUniqueItem(71119) ||
					to->IsEquipUniqueItem(71121) )
			{
				rateFactor += 10;
			}
			break;

		case 20114:
		case 20120:
		case 20121:
		case 20122:
		case 20123:
		case 20124:
		case 20125:
			rateFactor += 30;
			break;
	}

	if (to->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
		rateFactor += 50;
	if (to->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_EXP))
		rateFactor += 50;
	if (to->GetPoint(POINT_PC_BANG_EXP_BONUS) > 0)
	{
		if (to->IsPCBang())
			rateFactor += to->GetPoint(POINT_PC_BANG_EXP_BONUS);
	}
	rateFactor += to->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_EXP_BONUS);
	rateFactor += to->GetPoint(POINT_RAMADAN_CANDY_BONUS_EXP);
	rateFactor += to->GetPoint(POINT_MALL_EXPBONUS);
	rateFactor += to->GetPoint(POINT_EXP);
	rateFactor = rateFactor * static_cast<rate_t>(CHARACTER_MANAGER::instance().GetMobExpRate(to))/100.0L;
	// fix underflow formula
	iExp = std::max<int32_t>(0, iExp);
	rateFactor = std::max<rate_t>(100.0L, rateFactor);
	// apply calculated rate bonus
	iExp *= (rateFactor/100.0L);
	if (test_server)
		to->ChatPacket(CHAT_TYPE_INFO, "base_exp(%d) * rate(%Lf) = exp(%d)", iBaseExp, rateFactor/100.0L, iExp);
	iExp = MIN(to->GetNextExp() / 10, iExp);
	iExp = AdjustExpByLevel(to, iExp);
	if (test_server)
		to->ChatPacket(CHAT_TYPE_INFO, "exp+minGNE+adjust(%d)", iExp);

#ifdef __SUPPORT__
	if (to->GetSupportSystem())
	{
		if (to->GetSupportSystem()->IsActiveSupport() && to->GetSupportSystem()->GetLevel() < gPlayerMaxLevel)
		{
			int32_t tmpexp = iExp * 9 / 20;
			iExp = iExp - tmpexp;
			to->GetSupportSystem()->SetExp(tmpexp);
		}
	}
#endif
#ifdef __GROWTH_PET__
	if (to->GetNewPetSystem())
	{
		if (to->GetNewPetSystem()->IsActivePet() && to->GetNewPetSystem()->GetLevelStep() < 4)
		{
			int32_t tmpexp = iExp * 9 / 20;
			iExp = iExp - tmpexp;
			to->GetNewPetSystem()->SetExp(tmpexp, 0);
		}
	}
#endif

	to->PointChange(POINT_EXP, iExp, true);
	from->CreateFly(FLY_EXP, to);
	{
		LPCHARACTER you = to->GetMarryPartner();
		if (you)
		{
			uint32_t dwUpdatePoint = (2000.0L/to->GetLevel()/to->GetLevel()/3)*iExp;

			if (to->GetPremiumRemainSeconds(PREMIUM_MARRIAGE_FAST) > 0 ||
					you->GetPremiumRemainSeconds(PREMIUM_MARRIAGE_FAST) > 0)
				dwUpdatePoint *= 3;

			marriage::TMarriage* pMarriage = marriage::CManager::instance().Get(to->GetPlayerID());

			if (pMarriage && pMarriage->IsNear())
				pMarriage->Update(dwUpdatePoint);
		}
	}
}
#else
static void GiveExp(LPCHARACTER from, LPCHARACTER to, int32_t iExp)
{
	iExp = CALCULATE_VALUE_LVDELTA(to->GetLevel(), from->GetLevel(), iExp);

	if (distribution_test_server)
		iExp *= 3;

	int32_t iBaseExp = iExp;

	iExp = iExp * (100 + CPrivManager::instance().GetPriv(to, PRIV_EXP_PCT)) / 100;

	{
		if (to->IsEquipUniqueItem(UNIQUE_ITEM_LARBOR_MEDAL))
			iExp += iExp * 20 /100;

		if (to->GetMapIndex() >= 660000 && to->GetMapIndex() < 670000)
			iExp += iExp * 20 / 100;

		if (to->GetPoint(POINT_EXP_DOUBLE_BONUS))
			if (number(1, 100) <= to->GetPoint(POINT_EXP_DOUBLE_BONUS))
				iExp += iExp * 30 / 100;

		if (to->IsEquipUniqueItem(UNIQUE_ITEM_DOUBLE_EXP))
			iExp += iExp * 50 / 100;

		switch (to->GetMountVnum())
		{
			case 20110:
			case 20111:
			case 20112:
			case 20113:
				if (to->IsEquipUniqueItem(71115) || to->IsEquipUniqueItem(71117) || to->IsEquipUniqueItem(71119) ||
						to->IsEquipUniqueItem(71121) )
				{
					iExp += iExp * 10 / 100;
				}
				break;

			case 20114:
			case 20120:
			case 20121:
			case 20122:
			case 20123:
			case 20124:
			case 20125:
				iExp += iExp * 30 / 100;
				break;
		}
	}

	// @correction005
	{
		if (to->GetPremiumRemainSeconds(PREMIUM_EXP) > 0)
		{
			iExp += (iExp * 50 / 100);
		}

		if (to->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_EXP) == true)
		{
			iExp += (iExp * 50 / 100);
		}

		if (to->GetPoint(POINT_PC_BANG_EXP_BONUS) > 0)
		{
			if (to->IsPCBang() == true)
				iExp += (iExp * to->GetPoint(POINT_PC_BANG_EXP_BONUS)/100);
		}

		iExp += iExp * to->GetMarriageBonus(UNIQUE_ITEM_MARRIAGE_EXP_BONUS) / 100;
	}

	iExp += (iExp * to->GetPoint(POINT_RAMADAN_CANDY_BONUS_EXP)/100);
	iExp += (iExp * to->GetPoint(POINT_MALL_EXPBONUS)/100);
	iExp += (iExp * to->GetPoint(POINT_EXP)/100);

	if (test_server)
	{
		sys_log(0, "Bonus Exp : Ramadan Candy: %d MallExp: %d PointExp: %d",
				to->GetPoint(POINT_RAMADAN_CANDY_BONUS_EXP),
				to->GetPoint(POINT_MALL_EXPBONUS),
				to->GetPoint(POINT_EXP)
			   );
	}

	iExp = iExp * CHARACTER_MANAGER::instance().GetMobExpRate(to) / 100;

	iExp = MIN(to->GetNextExp() / 10, iExp);

	if (test_server)
	{
		if (quest::CQuestManager::instance().GetEventFlag("exp_bonus_log") && iBaseExp>0)
			to->ChatPacket(CHAT_TYPE_INFO, "exp bonus %d%%", (iExp-iBaseExp)*100/iBaseExp);
		to->ChatPacket(CHAT_TYPE_INFO, "exp(%d) base_exp(%d)", iExp, iBaseExp);
	}

	iExp = AdjustExpByLevel(to, iExp);

#ifdef __SUPPORT__
	if (to->GetSupportSystem())
	{
		if (to->GetSupportSystem()->IsActiveSupport() && to->GetSupportSystem()->GetLevel() < gPlayerMaxLevel)
		{
			int32_t tmpexp = iExp * 9 / 20;
			iExp = iExp - tmpexp;
			to->GetSupportSystem()->SetExp(tmpexp);
		}
	}
#endif
#ifdef __GROWTH_PET__
	if (to->GetNewPetSystem())
	{
		if (to->GetNewPetSystem()->IsActivePet() && to->GetNewPetSystem()->GetLevelStep() < 4)
		{
			int32_t tmpexp = iExp * 9 / 20;
			iExp = iExp - tmpexp;
			to->GetNewPetSystem()->SetExp(tmpexp, 0);
		}
	}
#endif

	to->PointChange(POINT_EXP, iExp, true);
	from->CreateFly(FLY_EXP, to);

	{
		LPCHARACTER you = to->GetMarryPartner();
		if (you)
		{
			uint32_t dwUpdatePoint = 2000*iExp/to->GetLevel()/to->GetLevel()/3;

			if (to->GetPremiumRemainSeconds(PREMIUM_MARRIAGE_FAST) > 0 ||
					you->GetPremiumRemainSeconds(PREMIUM_MARRIAGE_FAST) > 0)
				dwUpdatePoint = (uint32_t)(dwUpdatePoint * 3);

			marriage::TMarriage* pMarriage = marriage::CManager::instance().Get(to->GetPlayerID());

			if (pMarriage && pMarriage->IsNear())
				pMarriage->Update(dwUpdatePoint);
		}
	}
}
#endif

namespace NPartyExpDistribute
{
	struct FPartyTotaler
	{
		int32_t		total;
		int32_t		member_count;
		int32_t		x, y;

		FPartyTotaler(LPCHARACTER center)
			: total(0), member_count(0), x(center->GetX()), y(center->GetY())
		{};

		void operator () (LPCHARACTER ch)
		{
			if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
			{
				// @correction005
				total += __GetPartyExpNP(ch->GetLevel()); // @correction001
				++member_count;
			}
		}
	};

	struct FPartyDistributor
	{
		int32_t		total;
		LPCHARACTER	c;
		int32_t		x, y;
		uint32_t		_iExp;
		int32_t		m_iMode;
		int32_t		m_iMemberCount;

		FPartyDistributor(LPCHARACTER center, int32_t member_count, int32_t total, uint32_t iExp, int32_t iMode)
			: total(total), c(center), x(center->GetX()), y(center->GetY()), _iExp(iExp), m_iMode(iMode), m_iMemberCount(member_count)
			{
				if (m_iMemberCount == 0)
					m_iMemberCount = 1;
			};

		void operator () (LPCHARACTER ch)
		{
			if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
			{
				uint32_t iExp2 = 0;

				switch (m_iMode)
				{
					case PARTY_EXP_DISTRIBUTION_NON_PARITY:
						// @correction005
						iExp2 = (uint32_t) (_iExp * (float) __GetPartyExpNP(ch->GetLevel()) / total); // @correction001
						break;

					case PARTY_EXP_DISTRIBUTION_PARITY:
						iExp2 = _iExp / m_iMemberCount;
						break;

					default:
						sys_err("Unknown party exp distribution mode %d", m_iMode);
						return;
				}

				GiveExp(c, ch, iExp2);
			}
		}
	};
}

typedef struct SDamageInfo
{
	int32_t iDam;
	LPCHARACTER pAttacker;
	LPPARTY pParty;

	void Clear()
	{
		pAttacker = nullptr;
		pParty = nullptr;
	}

	inline void Distribute(LPCHARACTER ch, int32_t iExp)
	{
		if (pAttacker)
			GiveExp(ch, pAttacker, iExp);
		else if (pParty)
		{
			NPartyExpDistribute::FPartyTotaler f(ch);
			pParty->ForEachOnlineMember(f);

			if (pParty->IsPositionNearLeader(ch))
				iExp = iExp * (100 + pParty->GetExpBonusPercent()) / 100;

			if (test_server)
			{
				if (quest::CQuestManager::instance().GetEventFlag("exp_bonus_log") && pParty->GetExpBonusPercent())
					pParty->ChatPacketToAllMember(CHAT_TYPE_INFO, "exp party bonus %d%%", pParty->GetExpBonusPercent());
			}

			if (pParty->GetExpCentralizeCharacter())
			{
				LPCHARACTER tch = pParty->GetExpCentralizeCharacter();

				if (DISTANCE_APPROX(ch->GetX() - tch->GetX(), ch->GetY() - tch->GetY()) <= PARTY_DEFAULT_RANGE)
				{
					int32_t iExpCenteralize = (int32_t) (iExp * 0.05f);
					iExp -= iExpCenteralize;

					GiveExp(ch, pParty->GetExpCentralizeCharacter(), iExpCenteralize);
				}
			}

			NPartyExpDistribute::FPartyDistributor fDist(ch, f.member_count, f.total, iExp, pParty->GetExpDistributionMode());
			pParty->ForEachOnlineMember(fDist);
		}
	}
} TDamageInfo;

LPCHARACTER CHARACTER::DistributeExp()
{
	int32_t iExpToDistribute = GetExp();

	if (iExpToDistribute <= 0)
		return nullptr;

	int32_t	iTotalDam = 0;
	LPCHARACTER pkChrMostAttacked = nullptr;
	int32_t iMostDam = 0;

	typedef std::vector<TDamageInfo> TDamageInfoTable;
	TDamageInfoTable damage_info_table;
	std::map<LPPARTY, TDamageInfo> map_party_damage;

	damage_info_table.reserve(m_map_kDamage.size());

	TDamageMap::iterator it = m_map_kDamage.begin();

	while (it != m_map_kDamage.end())
	{
		const VID & c_VID = it->first;
		int32_t iDam = it->second.iTotalDamage;

		++it;

		LPCHARACTER pAttacker = CHARACTER_MANAGER::instance().Find(c_VID);

		if (!pAttacker || pAttacker->IsNPC() || DISTANCE_APPROX(GetX()-pAttacker->GetX(), GetY()-pAttacker->GetY())>5000)
			continue;

		iTotalDam += iDam;
		if (!pkChrMostAttacked || iDam > iMostDam)
		{
			pkChrMostAttacked = pAttacker;
			iMostDam = iDam;
		}

		if (pAttacker->GetParty())
		{
			std::map<LPPARTY, TDamageInfo>::iterator it = map_party_damage.find(pAttacker->GetParty());
			if (it == map_party_damage.end())
			{
				TDamageInfo di;
				di.iDam = iDam;
				di.pAttacker = nullptr;
				di.pParty = pAttacker->GetParty();
				map_party_damage.insert(std::make_pair(di.pParty, di));
			}
			else
			{
				it->second.iDam += iDam;
			}
		}
		else
		{
			TDamageInfo di;

			di.iDam = iDam;
			di.pAttacker = pAttacker;
			di.pParty = nullptr;

			damage_info_table.push_back(di);
		}
	}

	for (std::map<LPPARTY, TDamageInfo>::iterator it = map_party_damage.begin(); it != map_party_damage.end(); ++it)
	{
		damage_info_table.push_back(it->second);
	}

	SetExp(0);

	if (iTotalDam == 0)
		return nullptr;

	if (m_pkChrStone)
	{
		int32_t iExp = iExpToDistribute >> 1;
		m_pkChrStone->SetExp(m_pkChrStone->GetExp() + iExp);
		iExpToDistribute -= iExp;
	}

	sys_log(1, "%s total exp: %d, damage_info_table.size() == %d, TotalDam %d",
			GetName(), iExpToDistribute, damage_info_table.size(), iTotalDam);

	if (damage_info_table.empty())
		return nullptr;

	DistributeHP(pkChrMostAttacked);

	{
		TDamageInfoTable::iterator di = damage_info_table.begin();
		{
			TDamageInfoTable::iterator it;

			for (it = damage_info_table.begin(); it != damage_info_table.end();++it)
			{
				if (it->iDam > di->iDam)
					di = it;
			}
		}

		int32_t	iExp = iExpToDistribute / 5;
		iExpToDistribute -= iExp;

		float fPercent = (float) di->iDam / iTotalDam;

		if (fPercent > 1.0f)
		{
			sys_err("DistributeExp percent over 1.0 (fPercent %f name %s)", fPercent, di->pAttacker->GetName());
			fPercent = 1.0f;
		}

		iExp += (int32_t) (iExpToDistribute * fPercent);


		di->Distribute(this, iExp);

		if (fPercent == 1.0f)
			return pkChrMostAttacked;

		di->Clear();
	}

	{
		TDamageInfoTable::iterator it;

		for (it = damage_info_table.begin(); it != damage_info_table.end(); ++it)
		{
			TDamageInfo & di = *it;

			float fPercent = (float) di.iDam / iTotalDam;

			if (fPercent > 1.0f)
			{
				sys_err("DistributeExp percent over 1.0 (fPercent %f name %s)", fPercent, di.pAttacker->GetName());
				fPercent = 1.0f;
			}

			di.Distribute(this, (int32_t) (iExpToDistribute * fPercent));
		}
	}

	return pkChrMostAttacked;
}

int32_t CHARACTER::GetArrowAndBow(LPITEM * ppkBow, LPITEM * ppkArrow, int32_t iArrowCount)
{
	LPITEM pkBow;

	if (!(pkBow = GetWear(WEAR_WEAPON)) || pkBow->GetProto()->bSubType != WEAPON_BOW)
	{
		return 0;
	}

	LPITEM pkArrow;

#ifdef __QUIVER__ // @correction203
	if (!(pkArrow = GetWear(WEAR_ARROW)) || pkArrow->GetType() != ITEM_WEAPON || (pkArrow->GetProto()->bSubType != WEAPON_ARROW && pkArrow->GetProto()->bSubType != WEAPON_QUIVER))
#else
	if (!(pkArrow = GetWear(WEAR_ARROW)) || pkArrow->GetType() != ITEM_WEAPON || pkArrow->GetProto()->bSubType != WEAPON_ARROW)
#endif
	{
		return 0;
	}

#ifdef __QUIVER__ // @correction203
	if (pkArrow->GetProto()->bSubType == WEAPON_QUIVER)
	{
		iArrowCount = MIN(iArrowCount, 200);
	}
	else
	{
		iArrowCount = MIN(iArrowCount, pkArrow->GetCount());
	}
#else
	iArrowCount = MIN(iArrowCount, pkArrow->GetCount());
#endif

	*ppkBow = pkBow;
	*ppkArrow = pkArrow;

	return iArrowCount;
}

void CHARACTER::UseArrow(LPITEM pkArrow, uint32_t dwArrowCount)
{
#ifdef __QUIVER__ // @correction203
	if (pkArrow->GetSubType() != WEAPON_QUIVER)
	{
		int32_t iCount = pkArrow->GetCount();
		uint32_t dwVnum = pkArrow->GetVnum();
		iCount = iCount - MIN(iCount, dwArrowCount);
		pkArrow->SetCount(iCount);
		if (iCount == 0)
		{
			LPITEM pkNewArrow = FindSpecifyItem(dwVnum);
			sys_log(0, "UseArrow : FindSpecifyItem %u %p", dwVnum, get_pointer(pkNewArrow));
			if (pkNewArrow)
				EquipItem(pkNewArrow);
		}
	}
#else
	int32_t iCount = pkArrow->GetCount();
	uint32_t dwVnum = pkArrow->GetVnum();
	iCount = iCount - MIN(iCount, dwArrowCount);
	pkArrow->SetCount(iCount);
	if (iCount == 0)
	{
		LPITEM pkNewArrow = FindSpecifyItem(dwVnum);
		sys_log(0, "UseArrow : FindSpecifyItem %u %p", dwVnum, get_pointer(pkNewArrow));
		if (pkNewArrow)
			EquipItem(pkNewArrow);
	}
#endif
}

class CFuncShoot
{
	public:
		LPCHARACTER	m_me;
		uint8_t		m_bType;
		bool		m_bSucceed;

		CFuncShoot(LPCHARACTER ch, uint8_t bType) : m_me(ch), m_bType(bType), m_bSucceed(false)
		{
		}

		void operator () (uint32_t dwTargetVID)
		{
			if (m_bType > 1)
			{
				if (g_bSkillDisable)
					return;

				m_me->m_SkillUseInfo[m_bType].SetMainTargetVID(dwTargetVID);
			}

			LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(dwTargetVID);

			if (!pkVictim)
				return;

			if (!battle_is_attackable(m_me, pkVictim))
				return;

			if (m_me->IsNPC())
			{
				if (DISTANCE_APPROX(m_me->GetX() - pkVictim->GetX(), m_me->GetY() - pkVictim->GetY()) > 5000)
					return;
			}

			LPITEM pkBow, pkArrow;

			switch (m_bType)
			{
				case 0:
					{
						int32_t iDam = 0;

						if (m_me->IsPC())
						{
							if (m_me->GetJob() != JOB_ASSASSIN)
								return;

							if (0 == m_me->GetArrowAndBow(&pkBow, &pkArrow))
								return;

							if (m_me->GetSkillGroup() != 0)
								if (!m_me->IsNPC() && m_me->GetSkillGroup() != 2)
								{
									if (m_me->GetSP() < 5)
										return;

									m_me->PointChange(POINT_SP, -5);
								}

							iDam = CalcArrowDamage(m_me, pkVictim, pkBow, pkArrow);
							m_me->UseArrow(pkArrow, 1);

							uint32_t	dwCurrentTime	= get_dword_time();
							if (IS_SPEED_HACK(m_me, pkVictim, dwCurrentTime))
								iDam	= 0;
						}
						else
							iDam = CalcMeleeDamage(m_me, pkVictim);

						NormalAttackAffect(m_me, pkVictim);
						int32_t reduce_resist_magic = pkVictim->GetPoint(POINT_RESIST_MAGIC);
#ifdef __ANTI_RESIST_MAGIC_REDUCTION__
						if (m_me->GetPoint(POINT_RESIST_MAGIC_REDUCTION) > 0)
						{
							// Fix Magic Resistance : If player have more than 100% magic resistance, will be modified to 100%;
							int32_t fix_magic_resistance = (pkVictim->GetPoint(POINT_RESIST_MAGIC) > 100) ? (int32_t)(100) : (int32_t)(pkVictim->GetPoint(POINT_RESIST_MAGIC));
							// End Fix;

							reduce_resist_magic = fix_magic_resistance - m_me->GetPoint(POINT_RESIST_MAGIC_REDUCTION);
							if (reduce_resist_magic < 1)
								reduce_resist_magic = 0;
						}
#endif
						iDam = iDam * (100 - reduce_resist_magic) / 100;


						m_me->OnMove(true);
						pkVictim->OnMove();

						if (pkVictim->CanBeginFight())
							pkVictim->BeginFight(m_me);

						pkVictim->Damage(m_me, iDam, DAMAGE_TYPE_NORMAL_RANGE);
					}
					break;

				case 1:
					{
						int32_t iDam;

						if (m_me->IsPC())
							return;

						iDam = CalcMagicDamage(m_me, pkVictim);

						NormalAttackAffect(m_me, pkVictim);

						iDam = iDam * (100 - pkVictim->GetPoint(POINT_RESIST_MAGIC)) / 100;


						m_me->OnMove(true);
						pkVictim->OnMove();

						if (pkVictim->CanBeginFight())
							pkVictim->BeginFight(m_me);

						pkVictim->Damage(m_me, iDam, DAMAGE_TYPE_MAGIC);
					}
					break;

				case SKILL_YEONSA:
					{
						int32_t iUseArrow = 1;

						{
							if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
							{
								m_me->OnMove(true);
								pkVictim->OnMove();

								if (pkVictim->CanBeginFight())
									pkVictim->BeginFight(m_me);

								m_me->ComputeSkill(m_bType, pkVictim);
								m_me->UseArrow(pkArrow, iUseArrow);

								if (pkVictim->IsDead())
									break;

							}
							else
								break;
						}
					}
					break;


				case SKILL_KWANKYEOK:
					{
						int32_t iUseArrow = 1;

						if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
						{
							m_me->OnMove(true);
							pkVictim->OnMove();

							if (pkVictim->CanBeginFight())
								pkVictim->BeginFight(m_me);

							sys_log(0, "%s kwankeyok %s", m_me->GetName(), pkVictim->GetName());
							m_me->ComputeSkill(m_bType, pkVictim);
							m_me->UseArrow(pkArrow, iUseArrow);
						}
					}
					break;

				case SKILL_GIGUNG:
					{
						int32_t iUseArrow = 1;
						if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
						{
							m_me->OnMove(true);
							pkVictim->OnMove();

							if (pkVictim->CanBeginFight())
								pkVictim->BeginFight(m_me);

							sys_log(0, "%s gigung %s", m_me->GetName(), pkVictim->GetName());
							m_me->ComputeSkill(m_bType, pkVictim);
							m_me->UseArrow(pkArrow, iUseArrow);
						}
					}

					break;
				case SKILL_HWAJO:
					{
						int32_t iUseArrow = 1;
						if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
						{
							m_me->OnMove(true);
							pkVictim->OnMove();

							if (pkVictim->CanBeginFight())
								pkVictim->BeginFight(m_me);

							sys_log(0, "%s hwajo %s", m_me->GetName(), pkVictim->GetName());
							m_me->ComputeSkill(m_bType, pkVictim);
							m_me->UseArrow(pkArrow, iUseArrow);
						}
					}

					break;

				case SKILL_HORSE_WILDATTACK_RANGE:
					{
						int32_t iUseArrow = 1;
						if (iUseArrow == m_me->GetArrowAndBow(&pkBow, &pkArrow, iUseArrow))
						{
							m_me->OnMove(true);
							pkVictim->OnMove();

							if (pkVictim->CanBeginFight())
								pkVictim->BeginFight(m_me);

							sys_log(0, "%s horse_wildattack %s", m_me->GetName(), pkVictim->GetName());
							m_me->ComputeSkill(m_bType, pkVictim);
							m_me->UseArrow(pkArrow, iUseArrow);
						}
					}

					break;

				case SKILL_MARYUNG:
				case SKILL_TUSOK:
				case SKILL_BIPABU:
				case SKILL_NOEJEON:
				case SKILL_GEOMPUNG:
				case SKILL_SANGONG:
				case SKILL_MAHWAN:
				case SKILL_PABEOB:
					{
						m_me->OnMove(true);
						pkVictim->OnMove();

						if (pkVictim->CanBeginFight())
							pkVictim->BeginFight(m_me);

						sys_log(0, "%s - Skill %d -> %s", m_me->GetName(), m_bType, pkVictim->GetName());
						m_me->ComputeSkill(m_bType, pkVictim);
					}
					break;

				case SKILL_CHAIN:
					{
						m_me->OnMove(true);
						pkVictim->OnMove();

						if (pkVictim->CanBeginFight())
							pkVictim->BeginFight(m_me);

						sys_log(0, "%s - Skill %d -> %s", m_me->GetName(), m_bType, pkVictim->GetName());
						m_me->ComputeSkill(m_bType, pkVictim);

					}
					break;

				case SKILL_YONGBI:
					{
						m_me->OnMove(true);
					}
					break;


				default:
					sys_err("CFuncShoot: I don't know this type [%d] of range attack.", (int32_t) m_bType);
					break;
			}

			m_bSucceed = true;
		}
};

bool CHARACTER::Shoot(uint8_t bType)
{
	sys_log(1, "Shoot %s type %u flyTargets.size %zu", GetName(), bType, m_vec_dwFlyTargets.size());

	if (!CanMove())
	{
		return false;
	}

	CFuncShoot f(this, bType);

	if (m_dwFlyTargetID != 0)
	{
		f(m_dwFlyTargetID);
		m_dwFlyTargetID = 0;
	}

	f = std::for_each(m_vec_dwFlyTargets.begin(), m_vec_dwFlyTargets.end(), f);
	m_vec_dwFlyTargets.clear();

	return f.m_bSucceed;
}

void CHARACTER::FlyTarget(uint32_t dwTargetVID, int32_t x, int32_t y, uint8_t bHeader)
{
	LPCHARACTER pkVictim = CHARACTER_MANAGER::instance().Find(dwTargetVID);
	TPacketGCFlyTargeting pack;

	pack.bHeader	= (bHeader == HEADER_CG_FLY_TARGETING) ? HEADER_GC_FLY_TARGETING : HEADER_GC_ADD_FLY_TARGETING;
	pack.dwShooterVID	= GetVID();

	if (pkVictim)
	{
		pack.dwTargetVID = pkVictim->GetVID();
		pack.x = pkVictim->GetX();
		pack.y = pkVictim->GetY();

		if (bHeader == HEADER_CG_FLY_TARGETING)
			m_dwFlyTargetID = dwTargetVID;
		else
			m_vec_dwFlyTargets.push_back(dwTargetVID);
	}
	else
	{
		pack.dwTargetVID = 0;
		pack.x = x;
		pack.y = y;
	}

	sys_log(1, "FlyTarget %s vid %d x %d y %d", GetName(), pack.dwTargetVID, pack.x, pack.y);
	PacketAround(&pack, sizeof(pack), this);
}

LPCHARACTER CHARACTER::GetNearestVictim(LPCHARACTER pkChr)
{
	if (nullptr == pkChr)
		pkChr = this;

	float fMinDist = 99999.0f;
	LPCHARACTER pkVictim = nullptr;

	TDamageMap::iterator it = m_map_kDamage.begin();

	while (it != m_map_kDamage.end())
	{
		const VID & c_VID = it->first;
		++it;

		LPCHARACTER pAttacker = CHARACTER_MANAGER::instance().Find(c_VID);

		if (!pAttacker)
			continue;

		if (pAttacker->IsAffectFlag(AFF_EUNHYUNG) ||
				pAttacker->IsAffectFlag(AFF_INVISIBILITY) ||
				pAttacker->IsAffectFlag(AFF_REVIVE_INVISIBLE))
			continue;

		float fDist = DISTANCE_APPROX(pAttacker->GetX() - pkChr->GetX(), pAttacker->GetY() - pkChr->GetY());

		if (fDist < fMinDist)
		{
			pkVictim = pAttacker;
			fMinDist = fDist;
		}
	}

	return pkVictim;
}

void CHARACTER::SetVictim(LPCHARACTER pkVictim)
{
	if (!pkVictim)
	{
		if (0 != (uint32_t)m_kVIDVictim)
			MonsterLog("°ø°Ý ´ë»óÀ» ÇØÁ¦");

		m_kVIDVictim.Reset();
		battle_end(this);
	}
	else
	{
		if (m_kVIDVictim != pkVictim->GetVID())
			MonsterLog("°ø°Ý ´ë»óÀ» ¼³Á¤: %s", pkVictim->GetName());

		m_kVIDVictim = pkVictim->GetVID();
		m_dwLastVictimSetTime = get_dword_time();
	}
}

LPCHARACTER CHARACTER::GetVictim() const
{
	return CHARACTER_MANAGER::instance().Find(m_kVIDVictim);
}

LPCHARACTER CHARACTER::GetProtege() const
{
	if (m_pkChrStone)
		return m_pkChrStone;

	if (m_pkParty)
		return m_pkParty->GetLeader();

	return nullptr;
}

int32_t CHARACTER::GetAlignment() const
{
	return m_iAlignment;
}

int32_t CHARACTER::GetRealAlignment() const
{
	return m_iRealAlignment;
}

void CHARACTER::ShowAlignment(bool bShow)
{
	if (bShow)
	{
		if (m_iAlignment != m_iRealAlignment)
		{
			m_iAlignment = m_iRealAlignment;
			UpdatePacket();
		}
	}
	else
	{
		if (m_iAlignment != 0)
		{
			m_iAlignment = 0;
			UpdatePacket();
		}
	}
}

void CHARACTER::UpdateAlignment(int32_t iAmount)
{
	bool bShow = false;

	if (m_iAlignment == m_iRealAlignment)
		bShow = true;

	int32_t i = m_iAlignment / 10;

#ifdef __ALIGN_RENEWAL__
	int32_t iOldAlignment = m_iRealAlignment;
	m_iRealAlignment = MINMAX(-300000, m_iRealAlignment + iAmount, 300000);
	OnAlignUpdate(iOldAlignment, false);
#else
	m_iRealAlignment = MINMAX(-200000, m_iRealAlignment + iAmount, 200000);
#endif

	if (bShow)
	{
		m_iAlignment = m_iRealAlignment;

		if (i != m_iAlignment / 10)
			UpdatePacket();
	}
}

void CHARACTER::SetKillerMode(bool isOn)
{
	if ((isOn ? ADD_CHARACTER_STATE_KILLER : 0) == IS_SET(m_bAddChrState, ADD_CHARACTER_STATE_KILLER))
		return;

	if (isOn)
		SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_KILLER);
	else
		REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_KILLER);

	m_iKillerModePulse = thecore_pulse();
	UpdatePacket();
	sys_log(0, "SetKillerMode Update %s[%d]", GetName(), GetPlayerID());
}

bool CHARACTER::IsKillerMode() const
{
	return IS_SET(m_bAddChrState, ADD_CHARACTER_STATE_KILLER);
}

void CHARACTER::UpdateKillerMode()
{
	if (!IsKillerMode())
		return;

	// @correction005
	if (thecore_pulse() - m_iKillerModePulse >= PASSES_PER_SEC(30))
		SetKillerMode(false);
}

void CHARACTER::SetPKMode(uint8_t bPKMode)
{
	if (bPKMode >= PK_MODE_MAX_NUM)
		return;

	if (m_bPKMode == bPKMode)
		return;

	if (bPKMode == PK_MODE_GUILD && !GetGuild())
		bPKMode = PK_MODE_FREE;

	m_bPKMode = bPKMode;
	UpdatePacket();

	sys_log(0, "PK_MODE: %s %d", GetName(), m_bPKMode);
}

uint8_t CHARACTER::GetPKMode() const
{
	return m_bPKMode;
}

struct FuncForgetMyAttacker
{
	LPCHARACTER m_ch;
	FuncForgetMyAttacker(LPCHARACTER ch)
	{
		m_ch = ch;
	}
	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;
			if (ch->IsPC())
				return;
			if (ch->m_kVIDVictim == m_ch->GetVID())
				ch->SetVictim(nullptr);
		}
	}
};

struct FuncAggregateMonster
{
	LPCHARACTER m_ch;
	FuncAggregateMonster(LPCHARACTER ch)
	{
		m_ch = ch;
	}
	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;
			if (ch->IsPC())
				return;
			if (!ch->IsMonster())
				return;
			if (ch->GetVictim())
				return;

			if (number(1, 100) <= 50)
				if (DISTANCE_APPROX(ch->GetX() - m_ch->GetX(), ch->GetY() - m_ch->GetY()) < 5000)
					if (ch->CanBeginFight())
						ch->BeginFight(m_ch);
		}
	}
};

struct FuncAttractRanger
{
	LPCHARACTER m_ch;
	FuncAttractRanger(LPCHARACTER ch)
	{
		m_ch = ch;
	}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;
			if (ch->IsPC())
				return;
			if (!ch->IsMonster())
				return;
			if (ch->GetVictim() && ch->GetVictim() != m_ch)
				return;
			if (ch->GetMobAttackRange() > 150)
			{
				int32_t iNewRange = 150;
				if (iNewRange < 150)
					iNewRange = 150;

				ch->AddAffect(AFFECT_BOW_DISTANCE, POINT_BOW_DISTANCE, iNewRange - ch->GetMobAttackRange(), AFF_NONE, 3*60, 0, false);
			}
		}
	}
};

struct FuncPullMonster
{
	LPCHARACTER m_ch;
	int32_t m_iLength;
	FuncPullMonster(LPCHARACTER ch, int32_t iLength = 300)
	{
		m_ch = ch;
		m_iLength = iLength;
	}

	void operator()(LPENTITY ent)
	{
		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER ch = (LPCHARACTER) ent;
			if (ch->IsPC())
				return;
			if (!ch->IsMonster())
				return;
			float fDist = DISTANCE_APPROX(m_ch->GetX() - ch->GetX(), m_ch->GetY() - ch->GetY());
			if (fDist > 3000 || fDist < 100)
				return;

			float fNewDist = fDist - m_iLength;
			if (fNewDist < 100)
				fNewDist = 100;

			float degree = GetDegreeFromPositionXY(ch->GetX(), ch->GetY(), m_ch->GetX(), m_ch->GetY());
			float fx;
			float fy;

			GetDeltaByDegree(degree, fDist - fNewDist, &fx, &fy);
			int32_t tx = (int32_t)(ch->GetX() + fx);
			int32_t ty = (int32_t)(ch->GetY() + fy);

			ch->Sync(tx, ty);
			ch->Goto(tx, ty);
			ch->CalculateMoveDuration();

			ch->SyncPacket();
		}
	}
};

void CHARACTER::ForgetMyAttacker()
{
	LPSECTREE pSec = GetSectree();
	if (pSec)
	{
		FuncForgetMyAttacker f(this);
		pSec->ForEachAround(f);
	}
	ReviveInvisible(5);
}

void CHARACTER::AggregateMonster()
{
	LPSECTREE pSec = GetSectree();
	if (pSec)
	{
		FuncAggregateMonster f(this);
		pSec->ForEachAround(f);
	}
}

void CHARACTER::AttractRanger()
{
	LPSECTREE pSec = GetSectree();
	if (pSec)
	{
		FuncAttractRanger f(this);
		pSec->ForEachAround(f);
	}
}

void CHARACTER::PullMonster()
{
	LPSECTREE pSec = GetSectree();
	if (pSec)
	{
		FuncPullMonster f(this);
		pSec->ForEachAround(f);
	}
}

void CHARACTER::UpdateAggrPointEx(LPCHARACTER pAttacker, EDamageType type, int32_t dam, CHARACTER::TBattleInfo & info)
{
	switch (type)
	{
		case DAMAGE_TYPE_NORMAL_RANGE:
			dam = (int32_t) (dam*1.2f);
			break;

		case DAMAGE_TYPE_RANGE:
			dam = (int32_t) (dam*1.5f);
			break;

		case DAMAGE_TYPE_MAGIC:
			dam = (int32_t) (dam*1.2f);
			break;

		default:
			break;
	}

	if (pAttacker == GetVictim())
		dam = (int32_t) (dam * 1.2f);

	info.iAggro += dam;

	if (info.iAggro < 0)
		info.iAggro = 0;

	if (GetParty() && dam > 0 && type != DAMAGE_TYPE_SPECIAL)
	{
		LPPARTY pParty = GetParty();

		int32_t iPartyAggroDist = dam;

		if (pParty->GetLeaderPID() == GetVID())
			iPartyAggroDist /= 2;
		else
			iPartyAggroDist /= 3;

		pParty->SendMessage(this, PM_AGGRO_INCREASE, iPartyAggroDist, pAttacker->GetVID());
	}

	ChangeVictimByAggro(info.iAggro, pAttacker);
}

void CHARACTER::UpdateAggrPoint(LPCHARACTER pAttacker, EDamageType type, int32_t dam)
{
	if (IsDead() || IsStun())
		return;

	TDamageMap::iterator it = m_map_kDamage.find(pAttacker->GetVID());

	if (it == m_map_kDamage.end())
	{
		m_map_kDamage.insert(TDamageMap::value_type(pAttacker->GetVID(), TBattleInfo(0, dam)));
		it = m_map_kDamage.find(pAttacker->GetVID());
	}

	UpdateAggrPointEx(pAttacker, type, dam, it->second);
}

void CHARACTER::ChangeVictimByAggro(int32_t iNewAggro, LPCHARACTER pNewVictim)
{
	if (get_dword_time() - m_dwLastVictimSetTime < 3000)
		return;

	if (pNewVictim == GetVictim())
	{
		if (m_iMaxAggro < iNewAggro)
		{
			m_iMaxAggro = iNewAggro;
			return;
		}

		TDamageMap::iterator it;
		TDamageMap::iterator itFind = m_map_kDamage.end();

		for (it = m_map_kDamage.begin(); it != m_map_kDamage.end(); ++it)
		{
			if (it->second.iAggro > iNewAggro)
			{
				LPCHARACTER ch = CHARACTER_MANAGER::instance().Find(it->first);

				if (ch && !ch->IsDead() && DISTANCE_APPROX(ch->GetX() - GetX(), ch->GetY() - GetY()) < 5000)
				{
					itFind = it;
					iNewAggro = it->second.iAggro;
				}
			}
		}

		if (itFind != m_map_kDamage.end())
		{
			m_iMaxAggro = iNewAggro;
#ifdef __DEFENSE_WAVE__
			if (!IsDefanceWaweMastAttackMob(GetRaceNum()))
				SetVictim(CHARACTER_MANAGER::instance().Find(itFind->first));
#else
			SetVictim(CHARACTER_MANAGER::instance().Find(itFind->first));
#endif
			m_dwStateDuration = 1;
		}
	}
	else
	{
		if (m_iMaxAggro < iNewAggro)
		{
			m_iMaxAggro = iNewAggro;
#ifdef __DEFENSE_WAVE__
			if (!IsDefanceWaweMastAttackMob(GetRaceNum()))
				SetVictim(pNewVictim);
#else
			SetVictim(pNewVictim);
#endif
			m_dwStateDuration = 1;
		}
	}
}

