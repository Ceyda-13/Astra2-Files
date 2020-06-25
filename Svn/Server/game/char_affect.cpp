
#include "stdafx.h"

#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "affect.h"
#include "packet.h"
#include "buffer_manager.h"
#include "desc_client.h"
#include "battle.h"
#include "guild.h"
#include "utils.h"
#include "locale_service.h"
#include "lua_incl.h"
#include "arena.h"
#include "horsename_manager.h"
#include "item.h"
#include "DragonSoul.h"

#define IS_NO_SAVE_AFFECT(type) ((type) == AFFECT_WAR_FLAG || (type) == AFFECT_REVIVE_INVISIBLE || ((type) >= AFFECT_PREMIUM_START && (type) <= AFFECT_PREMIUM_END))
#define IS_NO_CLEAR_ON_DEATH_AFFECT(type) ((type) == AFFECT_BLOCK_CHAT || ((type) >= 500 && (type) < 600))
#define IS_NO_UPDATE_PACKET_AFFECT(type) ((type) == AFFECT_MOV_SPEED || (type) == AFFECT_ATT_SPEED || (type) == AFFECT_CHINA_FIREWORK || (type) == SKILL_JEONGWI || (type) == SKILL_GEOMKYUNG || (type) == SKILL_CHUNKEON || (type) == SKILL_EUNHYUNG || (type) == SKILL_GYEONGGONG || (type) == SKILL_GWIGEOM || (type) == SKILL_TERROR || (type) == SKILL_JUMAGAP || (type) == SKILL_MANASHILED || (type) == SKILL_JEOKRANG || (type) == SKILL_CHEONGRANG)

void SendAffectRemovePacket(LPDESC d, uint32_t pid, uint32_t type, uint8_t point)
{
	TPacketGCAffectRemove ptoc;
	ptoc.bHeader	= HEADER_GC_AFFECT_REMOVE;
	ptoc.dwType		= type;
	ptoc.bApplyOn	= point;
	d->Packet(&ptoc, sizeof(TPacketGCAffectRemove));

	TPacketGDRemoveAffect ptod;
	ptod.dwPID		= pid;
	ptod.dwType		= type;
	ptod.bApplyOn	= point;
	db_clientdesc->DBPacket(HEADER_GD_REMOVE_AFFECT, 0, &ptod, sizeof(ptod));
}

void SendAffectAddPacket(LPDESC d, CAffect * pkAff)
{
	TPacketGCAffectAdd ptoc;
	ptoc.bHeader		= HEADER_GC_AFFECT_ADD;
	ptoc.elem.dwType		= pkAff->dwType;
	ptoc.elem.bApplyOn		= pkAff->bApplyOn;
	ptoc.elem.lApplyValue	= pkAff->lApplyValue;
	ptoc.elem.dwFlag		= pkAff->dwFlag;
	ptoc.elem.lDuration		= pkAff->lDuration;
	ptoc.elem.lSPCost		= pkAff->lSPCost;
	d->Packet(&ptoc, sizeof(TPacketGCAffectAdd));
}

CAffect * CHARACTER::FindAffect(uint32_t dwType, uint8_t bApply) const
{
	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect * pkAffect = *it++;

		if (pkAffect->dwType == dwType && (bApply == APPLY_NONE || bApply == pkAffect->bApplyOn))
			return pkAffect;
	}

	return nullptr;
}

EVENTFUNC(affect_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "affect_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) {
		return 0;
	}

	if (!ch->UpdateAffect())
		return 0;
	else
		return passes_per_sec;
}

bool CHARACTER::UpdateAffect()
{
	// @correction082 BEGIN
	if (!GetWear(WEAR_WEAPON) && IsAffectFlag(AFF_GEOMGYEONG))
		RemoveAffect(SKILL_GEOMKYUNG);

	if (!GetWear(WEAR_WEAPON) && IsAffectFlag(AFF_GWIGUM))
		RemoveAffect(SKILL_GWIGEOM);
	// @correction082 END

	if (GetPoint(POINT_HP_RECOVERY) > 0)
	{
		if (GetMaxHP() <= GetHP())
		{
			PointChange(POINT_HP_RECOVERY, -GetPoint(POINT_HP_RECOVERY));
		}
		else
		{
			// @correction005
			int32_t iVal = MIN(GetPoint(POINT_HP_RECOVERY), GetMaxHP() * 7 / 100);

			PointChange(POINT_HP, iVal);
			PointChange(POINT_HP_RECOVERY, -iVal);
		}
	}

	if (GetPoint(POINT_SP_RECOVERY) > 0)
	{
		if (GetMaxSP() <= GetSP())
			PointChange(POINT_SP_RECOVERY, -GetPoint(POINT_SP_RECOVERY));
		else
		{
			int32_t iVal = MIN(GetPoint(POINT_SP_RECOVERY), GetMaxSP() * 7 / 100);

			PointChange(POINT_SP, iVal);
			PointChange(POINT_SP_RECOVERY, -iVal);
		}
	}

	if (GetPoint(POINT_HP_RECOVER_CONTINUE) > 0)
	{
		PointChange(POINT_HP, GetPoint(POINT_HP_RECOVER_CONTINUE));
	}

	if (GetPoint(POINT_SP_RECOVER_CONTINUE) > 0)
	{
		PointChange(POINT_SP, GetPoint(POINT_SP_RECOVER_CONTINUE));
	}

	AutoRecoveryItemProcess( AFFECT_AUTO_HP_RECOVERY );
	AutoRecoveryItemProcess( AFFECT_AUTO_SP_RECOVERY );

	if (GetMaxStamina() > GetStamina())
	{
		int32_t iSec = (get_dword_time() - GetStopTime()) / 3000;
		if (iSec)
			PointChange(POINT_STAMINA, GetMaxStamina()/1);
	}


	if (ProcessAffect())
		if (GetPoint(POINT_HP_RECOVERY) == 0 && GetPoint(POINT_SP_RECOVERY) == 0 && GetStamina() == GetMaxStamina())
		{
			m_pkAffectEvent = nullptr;
			return false;
		}

	return true;
}

void CHARACTER::StartAffectEvent()
{
	if (m_pkAffectEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();
	info->ch = this;
	m_pkAffectEvent = event_create(affect_event, info, passes_per_sec);
	sys_log(1, "StartAffectEvent %s %p %p", GetName(), this, get_pointer(m_pkAffectEvent));
}

void CHARACTER::ClearAffect(bool bSave
#ifdef __NO_CLEAR_BUFF_WHEN_MONSTER_KILL__
, bool letBuffs
#endif
)
{
	TAffectFlag afOld/* = m_afAffectFlag*/;
	memcpy(static_cast<void*>(&afOld), &m_afAffectFlag, sizeof(afOld));
	uint16_t	wMovSpd = GetPoint(POINT_MOV_SPEED);
	uint16_t	wAttSpd = GetPoint(POINT_ATT_SPEED);

	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect * pkAff = *it;

		if (bSave)
		{
			if ( IS_NO_CLEAR_ON_DEATH_AFFECT(pkAff->dwType) || IS_NO_SAVE_AFFECT(pkAff->dwType) )
			{
				++it;
				continue;
			}

			if (IS_NO_UPDATE_PACKET_AFFECT(pkAff->dwType))
			{
				++it;
				continue;
			}

#ifdef __NO_CLEAR_BUFF_WHEN_MONSTER_KILL__
			if ((pkAff->dwType == SKILL_KWAESOK || pkAff->dwType == SKILL_JEUNGRYEOK || pkAff->dwType == SKILL_HOSIN || pkAff->dwType == SKILL_REFLECT || pkAff->dwType == SKILL_GICHEON))
			{
				++it;
				continue;
			}
#endif
			if (IsPC())
			{
				SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->bApplyOn);
			}
		}

		ComputeAffect(pkAff, false);

		it = m_list_pkAffect.erase(it);
		CAffect::Release(pkAff);
	}

	if (afOld != m_afAffectFlag ||
			wMovSpd != GetPoint(POINT_MOV_SPEED) ||
			wAttSpd != GetPoint(POINT_ATT_SPEED))
		UpdatePacket();

	CheckMaximumPoints();

	if (m_list_pkAffect.empty())
		event_cancel(&m_pkAffectEvent);
}

int32_t CHARACTER::ProcessAffect()
{
	bool	bDiff	= false;
	CAffect	*pkAff	= nullptr;

	for (int32_t i = 0; i <= PREMIUM_MAX_NUM; ++i)
	{
		int32_t aff_idx = i + AFFECT_PREMIUM_START;

		pkAff = FindAffect(aff_idx);

		if (!pkAff)
			continue;

		int32_t remain = GetPremiumRemainSeconds(i);

		if (remain < 0)
		{
			RemoveAffect(aff_idx);
			bDiff = true;
		}
		else
			pkAff->lDuration = remain + 1;
	}

	pkAff = FindAffect(AFFECT_HAIR);
	if (pkAff)
	{
		if ( this->GetQuestFlag("hair.limit_time") < get_global_time())
		{
			this->SetPart(PART_HAIR, 0);
			RemoveAffect(AFFECT_HAIR);
		}
		else
		{
			++(pkAff->lDuration);
		}
	}

	CHorseNameManager::instance().Validate(this);

	TAffectFlag afOld/* = m_afAffectFlag*/;
	memcpy(static_cast<void*>(&afOld), &m_afAffectFlag, sizeof(afOld));
	int32_t lMovSpd = GetPoint(POINT_MOV_SPEED);
	int32_t lAttSpd = GetPoint(POINT_ATT_SPEED);

	itertype(m_list_pkAffect) it;

	it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		pkAff = *it;

		bool bEnd = false;

		if (pkAff->dwType >= GUILD_SKILL_START && pkAff->dwType <= GUILD_SKILL_END)
		{
			if (!GetGuild() || !GetGuild()->UnderAnyWar())
				bEnd = true;
		}

		if (pkAff->lSPCost > 0)
		{
			if (GetSP() < pkAff->lSPCost)
				bEnd = true;
			else
				PointChange(POINT_SP, -pkAff->lSPCost);
		}

		if ( --pkAff->lDuration <= 0 )
		{
			bEnd = true;
		}

		if (bEnd)
		{
			it = m_list_pkAffect.erase(it);
			ComputeAffect(pkAff, false);
			bDiff = true;
			if (IsPC())
			{
				SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->bApplyOn);
			}

			CAffect::Release(pkAff);

			continue;
		}

		++it;
	}

	if (bDiff)
	{
		if (afOld != m_afAffectFlag ||
				lMovSpd != GetPoint(POINT_MOV_SPEED) ||
				lAttSpd != GetPoint(POINT_ATT_SPEED))
		{
			UpdatePacket();
		}

		CheckMaximumPoints();
	}

	if (m_list_pkAffect.empty())
		return true;

	return false;
}

void CHARACTER::SaveAffect()
{
	TPacketGDAddAffect p;

	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect * pkAff = *it++;

		if (IS_NO_SAVE_AFFECT(pkAff->dwType))
			continue;

		sys_log(1, "AFFECT_SAVE: %u %u %d %d", pkAff->dwType, pkAff->bApplyOn, pkAff->lApplyValue, pkAff->lDuration);

		p.dwPID			= GetPlayerID();
		p.elem.dwType		= pkAff->dwType;
		p.elem.bApplyOn		= pkAff->bApplyOn;
		p.elem.lApplyValue	= pkAff->lApplyValue;
		p.elem.dwFlag		= pkAff->dwFlag;
		p.elem.lDuration	= pkAff->lDuration;
		p.elem.lSPCost		= pkAff->lSPCost;
		db_clientdesc->DBPacket(HEADER_GD_ADD_AFFECT, 0, &p, sizeof(p));
	}
}

EVENTINFO(load_affect_login_event_info)
{
	uint32_t pid;
	uint32_t count;
	char* data;

	load_affect_login_event_info()
	: pid( 0 )
	, count( 0 )
	, data( 0 )
	{
	}
};

EVENTFUNC(load_affect_login_event)
{
	load_affect_login_event_info* info = dynamic_cast<load_affect_login_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "load_affect_login_event_info> <Factor> Null pointer" );
		return 0;
	}

	uint32_t dwPID = info->pid;
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);

	if (!ch)
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}

	LPDESC d = ch->GetDesc();

	if (!d)
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}

	if (d->IsPhase(PHASE_HANDSHAKE) ||
			d->IsPhase(PHASE_LOGIN) ||
			d->IsPhase(PHASE_SELECT) ||
			d->IsPhase(PHASE_DEAD) ||
			d->IsPhase(PHASE_LOADING))
	{
		return PASSES_PER_SEC(1);
	}
	else if (d->IsPhase(PHASE_CLOSE))
	{
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
	else if (d->IsPhase(PHASE_GAME))
	{
		sys_log(1, "Affect Load by Event");
		ch->LoadAffect(info->count, (TPacketAffectElement*)info->data);
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
	else
	{
		sys_err("input_db.cpp:quest_login_event INVALID PHASE pid %d", ch->GetPlayerID());
		M2_DELETE_ARRAY(info->data);
		return 0;
	}
}

void CHARACTER::LoadAffect(uint32_t dwCount, TPacketAffectElement * pElements)
{
	m_bIsLoadedAffect = false;

	if (!GetDesc()->IsPhase(PHASE_GAME))
	{
		if (test_server)
			sys_log(0, "LOAD_AFFECT: Creating Event", GetName(), dwCount);

		load_affect_login_event_info* info = AllocEventInfo<load_affect_login_event_info>();

		info->pid = GetPlayerID();
		info->count = dwCount;
		info->data = M2_NEW char[sizeof(TPacketAffectElement) * dwCount];
		memcpy(info->data, pElements, sizeof(TPacketAffectElement) * dwCount);

		event_create(load_affect_login_event, info, PASSES_PER_SEC(1));

		return;
	}

	ClearAffect(true);

	if (test_server)
		sys_log(0, "LOAD_AFFECT: %s count %d", GetName(), dwCount);

	TAffectFlag afOld/* = m_afAffectFlag*/;
	memcpy(static_cast<void*>(&afOld), &m_afAffectFlag, sizeof(afOld));

	int32_t lMovSpd = GetPoint(POINT_MOV_SPEED);
	int32_t lAttSpd = GetPoint(POINT_ATT_SPEED);

	for (uint32_t i = 0; i < dwCount; ++i, ++pElements)
	{
		if (pElements->dwType == SKILL_MUYEONG)
			continue;

		if (AFFECT_AUTO_HP_RECOVERY == pElements->dwType || AFFECT_AUTO_SP_RECOVERY == pElements->dwType)
		{
			LPITEM item = FindItemByID( pElements->dwFlag );

			if (nullptr == item)
				continue;

			item->Lock(true);
		}

		if (pElements->bApplyOn >= POINT_MAX_NUM)
		{
			sys_err("invalid affect data %s ApplyOn %u ApplyValue %d",
					GetName(), pElements->bApplyOn, pElements->lApplyValue);
			continue;
		}

		if (test_server)
		{
			sys_log(0, "Load Affect : Affect %s %d %d", GetName(), pElements->dwType, pElements->bApplyOn );
		}

		CAffect* pkAff = CAffect::Acquire();
		m_list_pkAffect.push_back(pkAff);

		pkAff->dwType		= pElements->dwType;
		pkAff->bApplyOn		= pElements->bApplyOn;
		pkAff->lApplyValue	= pElements->lApplyValue;
		pkAff->dwFlag		= pElements->dwFlag;
		pkAff->lDuration	= pElements->lDuration;
		pkAff->lSPCost		= pElements->lSPCost;

		SendAffectAddPacket(GetDesc(), pkAff);

		ComputeAffect(pkAff, true);


	}

	if ( CArenaManager::instance().IsArenaMap(GetMapIndex()) == true )
	{
		RemoveGoodAffect();
	}

	if (afOld != m_afAffectFlag || lMovSpd != GetPoint(POINT_MOV_SPEED) || lAttSpd != GetPoint(POINT_ATT_SPEED))
	{
		UpdatePacket();
	}

	StartAffectEvent();

	m_bIsLoadedAffect = true;

	DragonSoul_Initialize();

	// @fixme118 (regain affect hp/mp)
	if (!IsDead())
	{
		PointChange(POINT_HP, GetMaxHP() - GetHP());
		PointChange(POINT_SP, GetMaxSP() - GetSP());
	}
}

bool CHARACTER::AddAffect(uint32_t dwType, uint8_t bApplyOn, int32_t lApplyValue, uint32_t dwFlag, int32_t lDuration, int32_t lSPCost, bool bOverride, bool IsCube )
{
	if (dwType == AFFECT_BLOCK_CHAT && lDuration > 1)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("운영자 제제로 채팅이 금지 되었습니다."));
	}

	if (lDuration == 0)
	{
		sys_err("Character::AddAffect lDuration == 0 type %d", lDuration, dwType);
		lDuration = 1;
	}

	CAffect * pkAff = nullptr;

	if (IsCube)
		pkAff = FindAffect(dwType,bApplyOn);
	else
		pkAff = FindAffect(dwType);

	if (dwFlag == AFF_STUN)
	{
		if (m_posDest.x != GetX() || m_posDest.y != GetY())
		{
			m_posDest.x = m_posStart.x = GetX();
			m_posDest.y = m_posStart.y = GetY();
			battle_end(this);

			SyncPacket();
		}
	}

	if (pkAff && bOverride)
	{
		ComputeAffect(pkAff, false);

		if (GetDesc())
			SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->bApplyOn);
	}
	else
	{
		pkAff = CAffect::Acquire();
		m_list_pkAffect.push_back(pkAff);

	}

	sys_log(1, "AddAffect %s type %d apply %d %d flag %u duration %d", GetName(), dwType, bApplyOn, lApplyValue, dwFlag, lDuration);
	sys_log(0, "AddAffect %s type %d apply %d %d flag %u duration %d", GetName(), dwType, bApplyOn, lApplyValue, dwFlag, lDuration);

	pkAff->dwType	= dwType;
	pkAff->bApplyOn	= bApplyOn;
	pkAff->lApplyValue	= lApplyValue;
	pkAff->dwFlag	= dwFlag;
	pkAff->lDuration	= lDuration;
	pkAff->lSPCost	= lSPCost;

	uint16_t wMovSpd = GetPoint(POINT_MOV_SPEED);
	uint16_t wAttSpd = GetPoint(POINT_ATT_SPEED);

	ComputeAffect(pkAff, true);

	if (pkAff->dwFlag || wMovSpd != GetPoint(POINT_MOV_SPEED) || wAttSpd != GetPoint(POINT_ATT_SPEED))
		UpdatePacket();

	StartAffectEvent();

	if (IsPC())
	{
		SendAffectAddPacket(GetDesc(), pkAff);

		if (IS_NO_SAVE_AFFECT(pkAff->dwType))
			return true;

		TPacketGDAddAffect p;
		p.dwPID			= GetPlayerID();
		p.elem.dwType		= pkAff->dwType;
		p.elem.bApplyOn		= pkAff->bApplyOn;
		p.elem.lApplyValue	= pkAff->lApplyValue;
		p.elem.dwFlag		= pkAff->dwFlag;
		p.elem.lDuration	= pkAff->lDuration;
		p.elem.lSPCost		= pkAff->lSPCost;
		db_clientdesc->DBPacket(HEADER_GD_ADD_AFFECT, 0, &p, sizeof(p));
	}

	return true;
}

void CHARACTER::RefreshAffect()
{
	itertype(m_list_pkAffect) it = m_list_pkAffect.begin();

	while (it != m_list_pkAffect.end())
	{
		CAffect * pkAff = *it++;
		ComputeAffect(pkAff, true);
	}
}

void CHARACTER::ComputeAffect(CAffect * pkAff, bool bAdd)
{
	if (bAdd && pkAff->dwType >= GUILD_SKILL_START && pkAff->dwType <= GUILD_SKILL_END)
	{
		if (!GetGuild())
			return;

		if (!GetGuild()->UnderAnyWar())
			return;
	}

	if (pkAff->dwFlag)
	{
		if (!bAdd)
			m_afAffectFlag.Reset(pkAff->dwFlag);
		else
			m_afAffectFlag.Set(pkAff->dwFlag);
	}

	if (bAdd)
		PointChange(pkAff->bApplyOn, pkAff->lApplyValue);
	else
		PointChange(pkAff->bApplyOn, -pkAff->lApplyValue);

	if (pkAff->dwType == SKILL_MUYEONG)
	{
		if (bAdd)
			StartMuyeongEvent();
		else
			StopMuyeongEvent();
	}
}

bool CHARACTER::RemoveAffect(CAffect * pkAff, bool bIsCompute) // @correction033
{
	if (!pkAff)
		return false;

	m_list_pkAffect.remove(pkAff);

	ComputeAffect(pkAff, false);

	if (bIsCompute && AFFECT_REVIVE_INVISIBLE != pkAff->dwType && AFFECT_MOUNT != pkAff->dwType) // @correction033
		ComputePoints();
	else  // @correction037
		UpdatePacket();

	CheckMaximumPoints();

	if (test_server)
		sys_log(0, "AFFECT_REMOVE: %s (flag %u apply: %u)", GetName(), pkAff->dwFlag, pkAff->bApplyOn);

	if (IsPC())
	{
		SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->bApplyOn);
	}

	CAffect::Release(pkAff);
	return true;
}

bool CHARACTER::RemoveAffect(uint32_t dwType, bool bIsCompute) // @correction033
{
	if (dwType == AFFECT_BLOCK_CHAT)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("채팅 금지가 풀렸습니다."));
	}

	bool flag = false;

	CAffect * pkAff;

	while ((pkAff = FindAffect(dwType)))
	{
		RemoveAffect(pkAff, bIsCompute); // @correction033
		flag = true;
	}

	return flag;
}

bool CHARACTER::IsAffectFlag(uint32_t dwAff) const
{
	return m_afAffectFlag.IsSet(dwAff);
}

void CHARACTER::RemoveGoodAffect(bool bIsSkills) // @correction098
{
// @correction098 BEGIN
	if (!bIsSkills)
	{
		RemoveAffect(AFFECT_MOV_SPEED, false);
		RemoveAffect(AFFECT_ATT_SPEED, false);
		RemoveAffect(AFFECT_STR, false);
		RemoveAffect(AFFECT_DEX, false);
		RemoveAffect(AFFECT_INT, false);
		RemoveAffect(AFFECT_CON, false);
		RemoveAffect(AFFECT_CHINA_FIREWORK, false);
	}
// @correction098 END

	RemoveAffect(SKILL_JEONGWI, false);
	RemoveAffect(SKILL_GEOMKYUNG, false);
	RemoveAffect(SKILL_CHUNKEON, false);
	RemoveAffect(SKILL_EUNHYUNG, false);
	RemoveAffect(SKILL_GYEONGGONG, false);
	RemoveAffect(SKILL_GWIGEOM, false);
	RemoveAffect(SKILL_TERROR, false);
	RemoveAffect(SKILL_JUMAGAP, false);
	RemoveAffect(SKILL_MANASHILED, false);
	RemoveAffect(SKILL_HOSIN, false);
	RemoveAffect(SKILL_REFLECT, false);
	RemoveAffect(SKILL_KWAESOK, false);
	RemoveAffect(SKILL_JEUNGRYEOK, false);
	RemoveAffect(SKILL_GICHEON, false);
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	RemoveAffect(SKILL_JEOKRANG, false);
	RemoveAffect(SKILL_CHEONGRANG, false);
#endif
}

bool CHARACTER::IsGoodAffect(uint8_t bAffectType) const
{
	switch (bAffectType)
	{
		case (AFFECT_MOV_SPEED):
		case (AFFECT_ATT_SPEED):
		case (AFFECT_STR):
		case (AFFECT_DEX):
		case (AFFECT_INT):
		case (AFFECT_CON):
		case (AFFECT_CHINA_FIREWORK):

		case (SKILL_JEONGWI):
		case (SKILL_GEOMKYUNG):
		case (SKILL_CHUNKEON):
		case (SKILL_EUNHYUNG):
		case (SKILL_GYEONGGONG):
		case (SKILL_GWIGEOM):
		case (SKILL_TERROR):
		case (SKILL_JUMAGAP):
		case (SKILL_MANASHILED):
		case (SKILL_HOSIN):
		case (SKILL_REFLECT):
		case (SKILL_KWAESOK):
		case (SKILL_JEUNGRYEOK):
		case (SKILL_GICHEON):
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case (SKILL_JEOKRANG):
		case (SKILL_CHEONGRANG):
#endif
			return true;
	}
	return false;
}

void CHARACTER::RemoveBadAffect()
{
	sys_log(0, "RemoveBadAffect %s", GetName());
	RemovePoison();
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	RemoveBleeding();
#endif
	RemoveFire();

	RemoveAffect(AFFECT_STUN);

	RemoveAffect(AFFECT_SLOW);

	RemoveAffect(SKILL_TUSOK);
}

