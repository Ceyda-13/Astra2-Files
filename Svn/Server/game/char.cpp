#include "stdafx.h"

#include "../common/teen_packet.h"
#include "../common/VnumHelper.h"

#include "char.h"

#include "config.h"
#include "utils.h"
#include "crc32.h"
#include "char_manager.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "buffer_manager.h"
#include "item_manager.h"
#include "motion.h"
#include "vector.h"
#include "packet.h"
#include "cmd.h"
#include "fishing.h"
#include "exchange.h"
#include "battle.h"
#include "affect.h"
#include "shop.h"
#include "shop_manager.h"
#include "safebox.h"
#include "regen.h"
#include "pvp.h"
#include "party.h"
#include "start_position.h"
#include "questmanager.h"
#include "log.h"
#include "p2p.h"
#include "guild.h"
#include "guild_manager.h"
#include "dungeon.h"
#include "messenger_manager.h"
#include "unique_item.h"
#include "priv_manager.h"
#include "war_map.h"
#include "xmas_event.h"
#include "banword.h"
#include "target.h"
#include "wedding.h"
#include "mob_manager.h"
#include "mining.h"
#include "monarch.h"
#include "castle.h"
#include "arena.h"
#include "dev_log.h"
#include "horsename_manager.h"
#include "pcbang.h"
#include "gm.h"
#include "map_location.h"
#include "BlueDragon_Binder.h"
#include "HackShield.h"
#include "skill_power.h"
#include "XTrapManager.h"
#include "buff_on_attributes.h"
#include "DragonSoul.h"

#ifdef __PET_SYSTEM__
#include "PetSystem.h"
#endif
#ifdef __MOUNT__
#include "mountsystem.h"
#endif
#ifdef __GUILD_DRAGONLAIR__
#include "meleylair.h"
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
#ifdef __GUILD_DRAGONLAIR__
#include "meleylair.h"
#endif

#ifdef __SWITCHBOT__
#include "new_switchbot.h"
#endif
extern const uint8_t g_aBuffOnAttrPoints;
extern bool RaceToJob(uint32_t race, uint32_t *ret_job);

extern bool IS_SUMMONABLE_ZONE(int32_t map_index);
bool CAN_ENTER_ZONE(const LPCHARACTER& ch, int32_t map_index);

bool CAN_ENTER_ZONE(const LPCHARACTER& ch, int32_t map_index)
{
	switch (map_index)
	{
	case 301:
	case 302:
	case 303:
	case 304:
		if (ch->GetLevel() < 90)
			return false;
	}
	return true;
}

#ifdef NEW_ICEDAMAGE_SYSTEM // @correction118
const uint32_t CHARACTER::GetNoDamageRaceFlag()
{
	return m_dwNDRFlag;
}

void CHARACTER::SetNoDamageRaceFlag(uint32_t dwRaceFlag)
{
	if (dwRaceFlag>=MAIN_RACE_MAX_NUM) return;
	if (IS_SET(m_dwNDRFlag, 1<<dwRaceFlag)) return;
	SET_BIT(m_dwNDRFlag, 1<<dwRaceFlag);
}

void CHARACTER::UnsetNoDamageRaceFlag(uint32_t dwRaceFlag)
{
	if (dwRaceFlag>=MAIN_RACE_MAX_NUM) return;
	if (!IS_SET(m_dwNDRFlag, 1<<dwRaceFlag)) return;
	REMOVE_BIT(m_dwNDRFlag, 1<<dwRaceFlag);
}

void CHARACTER::ResetNoDamageRaceFlag()
{
	m_dwNDRFlag = 0;
}

const std::set<uint32_t> & CHARACTER::GetNoDamageAffectFlag()
{
	return m_setNDAFlag;
}

void CHARACTER::SetNoDamageAffectFlag(uint32_t dwAffectFlag)
{
	m_setNDAFlag.insert(dwAffectFlag);
}

void CHARACTER::UnsetNoDamageAffectFlag(uint32_t dwAffectFlag)
{
	m_setNDAFlag.erase(dwAffectFlag);
}

void CHARACTER::ResetNoDamageAffectFlag()
{
	m_setNDAFlag.clear();
}
#endif

LPCHARACTER DynamicCharacterPtr::Get() const {
	LPCHARACTER p = nullptr;
	if (is_pc) {
		p = CHARACTER_MANAGER::instance().FindByPID(id);
	} else {
		p = CHARACTER_MANAGER::instance().Find(id);
	}
	return p;
}

DynamicCharacterPtr& DynamicCharacterPtr::operator=(LPCHARACTER character) {
	if (character == nullptr) {
		Reset();
		return *this;
	}
	if (character->IsPC()) {
		is_pc = true;
		id = character->GetPlayerID();
	} else {
		is_pc = false;
		id = character->GetVID();
	}
	return *this;
}

CHARACTER::CHARACTER()
{
	m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateIdle, &CHARACTER::EndStateEmpty);
	m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateMove, &CHARACTER::EndStateEmpty);
	m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateBattle, &CHARACTER::EndStateEmpty);

	Initialize();
}

CHARACTER::~CHARACTER()
{
	Destroy();
}

void CHARACTER::Initialize()
{
	CEntity::Initialize(ENTITY_CHARACTER);

	m_bNoOpenedShop = true;

	m_bOpeningSafebox = false;

	m_fSyncTime = get_float_time()-3;
	m_dwPlayerID = 0;
	m_dwKillerPID = 0;

	m_iMoveCount = 0;

	m_pkRegen = nullptr;
	regen_id_ = 0;
	m_posRegen.x = m_posRegen.y = m_posRegen.z = 0;
	m_posStart.x = m_posStart.y = 0;
	m_posDest.x = m_posDest.y = 0;
	m_fRegenAngle = 0.0f;

	m_pkMobData		= nullptr;
	m_pkMobInst		= nullptr;

	m_pkShop		= nullptr;
	m_pkChrShopOwner	= nullptr;
	m_pkMyShop		= nullptr;
	m_pkExchange	= nullptr;
	m_pkParty		= nullptr;
	m_pkPartyRequestEvent = nullptr;

	m_pGuild = nullptr;

	m_pkChrTarget = nullptr;

	m_pkMuyeongEvent = nullptr;

	m_pkWarpNPCEvent = nullptr;
	m_pkDeadEvent = nullptr;
	m_pkStunEvent = nullptr;
	m_pkSaveEvent = nullptr;
	m_pkRecoveryEvent = nullptr;
	m_pkTimedEvent = nullptr;
	m_pkFishingEvent = nullptr;
	m_pkWarpEvent = nullptr;

	m_pkMiningEvent = nullptr;

	m_pkPoisonEvent = nullptr;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	m_pkBleedingEvent = nullptr;
#endif
	m_pkFireEvent = nullptr;
	m_pkCheckSpeedHackEvent	= nullptr;
	m_speed_hack_count	= 0;

	m_pkAffectEvent = nullptr;
	m_afAffectFlag = TAffectFlag(0, 0);

	m_pkDestroyWhenIdleEvent = nullptr;

	m_pkChrSyncOwner = nullptr;

	memset(&m_points, 0, sizeof(m_points));
	memset(&m_pointsInstant, 0, sizeof(m_pointsInstant));
	memset(&m_quickslot, 0, sizeof(m_quickslot));

	m_bCharType = CHAR_TYPE_MONSTER;

	SetPosition(POS_STANDING);

	m_dwPlayStartTime = m_dwLastMoveTime = get_dword_time();

	GotoState(m_stateIdle);
	m_dwStateDuration = 1;

	m_dwLastAttackTime = get_dword_time() - 20000;

	m_bAddChrState = 0;

	m_pkChrStone = nullptr;

	m_pkSafebox = nullptr;
	m_iSafeboxSize = -1;
	m_iSafeboxLoadTime = 0;

	m_pkMall = nullptr;
	m_iMallLoadTime = 0;

	m_posWarp.x = m_posWarp.y = m_posWarp.z = 0;
	m_lWarpMapIndex = 0;

	m_posExit.x = m_posExit.y = m_posExit.z = 0;
	m_lExitMapIndex = 0;

	m_pSkillLevels = nullptr;

	m_dwMoveStartTime = 0;
	m_dwMoveDuration = 0;

	m_dwFlyTargetID = 0;

	m_dwNextStatePulse = 0;

	m_dwLastDeadTime = get_dword_time()-180000;

	m_bSkipSave = false;

	m_bItemLoaded = false;

	m_bHasPoisoned = false;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	m_bHasBled = false;
#endif
	m_pkDungeon = nullptr;
	m_iEventAttr = 0;

	m_kAttackLog.dwVID = 0;
	m_kAttackLog.dwTime = 0;

	m_bNowWalking = m_bWalking = false;
	ResetChangeAttackPositionTime();

	m_bDetailLog = false;
	m_bMonsterLog = false;

	m_bDisableCooltime = false;

	m_iAlignment = 0;
	m_iRealAlignment = 0;

	m_iKillerModePulse = 0;
	m_bPKMode = PK_MODE_PEACE;

	m_dwQuestNPCVID = 0;
	m_dwQuestByVnum = 0;
	m_pQuestItem = nullptr;

	m_szMobileAuth[0] = '\0';

	m_dwUnderGuildWarInfoMessageTime = get_dword_time()-60000;

	m_bUnderRefine = false;

	m_dwRefineNPCVID = 0;

	m_dwPolymorphRace = 0;

	m_bStaminaConsume = false;

	ResetChainLightningIndex();

	m_dwMountVnum = 0;
	m_chHorse = nullptr;
	m_chRider = nullptr;

	m_pWarMap = nullptr;
	m_pWeddingMap = nullptr;
	m_bChatCounter = 0;

	ResetStopTime();

	m_dwLastVictimSetTime = get_dword_time() - 3000;
	m_iMaxAggro = -100;

	m_bSendHorseLevel = 0;
	m_bSendHorseHealthGrade = 0;
	m_bSendHorseStaminaGrade = 0;

	m_dwLoginPlayTime = 0;

	m_pkChrMarried = nullptr;

	m_posSafeboxOpen.x = -1000;
	m_posSafeboxOpen.y = -1000;

	m_dwLastSkillTime = get_dword_time();

	memset(m_adwMobSkillCooltime, 0, sizeof(m_adwMobSkillCooltime));

	m_isinPCBang = false;

	m_pArena = nullptr;
	m_nPotionLimit = quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count");

	m_isOpenSafebox = 0;

	m_iRefineTime = 0;

	m_iSeedTime = 0;
	m_iExchangeTime = 0;
	m_iSafeboxLoadTime = 0;

	m_iMyShopTime = 0;

	InitMC();

	m_deposit_pulse = 0;

	SET_OVER_TIME(this, OT_NONE);

	m_strNewName = "";

	m_known_guild.clear();

	m_dwLogOffInterval = 0;

	m_bComboSequence = 0;
	m_dwLastComboTime = 0;
	m_bComboIndex = 0;
	m_iComboHackCount = 0;
	m_dwSkipComboAttackByTime = 0;

	m_dwMountTime = 0;

	m_dwLastGoldDropTime = 0;
	m_dwLastItemDropTime = 0; // @correction225
	m_dwLastBoxUseTime = 0; // @correction226
	m_dwLastBuySellTime = 0; // @correction227

	m_HackShieldCheckEvent = nullptr;
	m_HackShieldCheckMode = false;

	m_bIsLoadedAffect = false;
	cannot_dead = false;

#ifdef __PET_SYSTEM__
	m_petSystem = 0;
	m_bIsPet = false;
#endif
#ifdef NEW_ICEDAMAGE_SYSTEM // @correction118
	m_dwNDRFlag = 0;
	m_setNDAFlag.clear();
#endif

	m_fAttMul = 1.0f;
	m_fDamMul = 1.0f;

	m_pointsInstant.iDragonSoulActiveDeck = -1;
#ifdef ENABLE_ANTI_CMD_FLOOD // @correction170
	m_dwCmdAntiFloodCount = 0;
	m_dwCmdAntiFloodPulse = 0;
#endif
	memset(&m_tvLastSyncTime, 0, sizeof(m_tvLastSyncTime));
	m_iSyncHackCount = 0;
	m_ItemUseTimesMap.clear(); // @correction090
#ifdef __ACCE_SYSTEM__ // @correction191
	m_bAcceCombination = false;
	m_bAcceAbsorption = false;
#endif
#ifdef __CHANGE_LOOK__ // @correction192
	m_bChangeLook = false;
#endif
#ifdef __OFFLINE_PRIVATE_SHOP__
	m_dwOfflinePrivateShopVid = 0;
	m_iRenameTime = 0;
#endif
#ifdef __MOUNT__
	m_MountSystem = 0;
	m_bIsMountSystem = false;
#endif
#ifdef __SUPPORT__
	m_supportSystem = 0;
	m_dwSupportHair = 0;
	m_dwSupportWeapon = 0;
	m_dwSupportCostume = 0;
	m_bIsSupport = false;
	m_Owner = nullptr;
#endif
#ifdef __GROWTH_PET__
	m_stImmortalSt = 0;
	memset(m_newpetskillcd, 0, sizeof(m_newpetskillcd));
	m_newpetSystem = 0;
	m_eggvid = 0;
	m_bIsNewPet = false;
#endif
#ifdef __BOSS_TRACKING_SYSTEM__
	m_pkUpdateBossTracking = nullptr;
#endif
#ifdef __SORT_INVENTORY__
	m_sortInventoryPulse = 0;
#endif
#ifdef __GUILD_DRAGONLAIR__
	m_pMeley = nullptr;
#endif
#ifdef __MINI_GAME_RUMI__
	m_MiniGameRumiCardDeckCount = 0;
	m_MiniGameRumiPoints = 0;
	memset(&m_MiniGameRumiHandCards, 0, sizeof(m_MiniGameRumiHandCards));
	memset(&m_MiniGameRumiFieldCards, 0, sizeof(m_MiniGameRumiFieldCards));
	memset(&m_MiniGameRumiDeckCards, 0, sizeof(m_MiniGameRumiDeckCards));
#endif
#ifdef __ACCUMULATE_DAMAGE_DISPLAY__
	m_Accumulate.clear();
#endif
#ifdef __MINI_GAME_FISH__
	m_fishSlots = nullptr;
	m_dwFishUseCount = 0;
	m_bFishAttachedShape = 0;
#endif

#ifdef __MINI_GAME_CATCH_KING__
	m_vecCatchKingFieldCards.clear();
	bCatchKingHandCard = 0;
	bCatchKingHandCardLeft = 0;
	bCatchKingBetSetNumber = 0;
	dwCatchKingTotalScore = 0;
	dwCatchKingGameStatus = false;
#endif
#ifdef __12ZI__
	m_pkBeadEvent = nullptr;
#endif
#ifdef __MULTI_LANGUAGE__
	m_stLanguage = "tr";
#endif
#ifdef __FAST_RUHBK__
	m_pkFastBKEvent = nullptr;
	m_pkFastRuhEvent = nullptr;
#endif
#ifdef __AUTO_PICKUP__
	m_bIsAutoPickUp = false;
#endif

#ifdef WJ_SECURITY_SYSTEM
	// m_dwSecurityLastTime = 0;
	m_bActivateSecurityGui = false;
	m_captcha = 0;
	m_captchacnt = 0;
#endif
}

void CHARACTER::Create(const char * c_pszName, uint32_t vid, bool isPC)
{
	static int32_t s_crc = 172814;

	char crc_string[128+1];
	snprintf(crc_string, sizeof(crc_string), "%s%p%d", c_pszName, this, ++s_crc);
	m_vid = VID(vid, GetCRC32(crc_string, strlen(crc_string)));

	if (isPC)
		m_stName = c_pszName;
}

void CHARACTER::Destroy()
{
	CloseMyShop();

	if (m_pkRegen)
	{
		if (m_pkDungeon) {
			if (m_pkDungeon->IsValidRegen(m_pkRegen, regen_id_)) {
				--m_pkRegen->count;
			}
		} else {
			--m_pkRegen->count;
		}
		m_pkRegen = nullptr;
	}

	if (m_pkDungeon)
	{
		SetDungeon(nullptr);
	}

#ifdef __PET_SYSTEM__
	if (m_petSystem)
	{
		m_petSystem->Destroy();
		delete m_petSystem;

		m_petSystem = 0;
	}
#endif

#ifdef __MOUNT__
	if (m_MountSystem)
	{
		m_MountSystem->Destroy();
		delete m_MountSystem;

		m_MountSystem = 0;
	}
#endif

#ifdef __SUPPORT__
	if (m_supportSystem)
	{
		m_supportSystem->Destroy();
		delete m_supportSystem;

		m_supportSystem = 0;
	}
#endif

#ifdef __GROWTH_PET__
	if (m_newpetSystem)
	{
		m_newpetSystem->Destroy();
		delete m_newpetSystem;

		m_newpetSystem = 0;
	}
#endif

	HorseSummon(false);

	if (GetRider())
		GetRider()->ClearHorseInfo();

	if( IsPC() )
	{
		if (isHackShieldEnable)
		{
			CHackShieldManager::instance().DeleteClientHandle(GetPlayerID());
		}
	}

	if (GetDesc())
	{
		GetDesc()->BindCharacter(nullptr);
	}

	if (m_pkExchange)
		m_pkExchange->Cancel();

	SetVictim(nullptr);

	if (GetShop())
	{
		GetShop()->RemoveGuest(this);
		SetShop(nullptr);
	}

	ClearStone();
	ClearSync();
	ClearTarget();

	if (nullptr == m_pkMobData)
	{
		DragonSoul_CleanUp();
		ClearItem();
	}

	LPPARTY party = m_pkParty;
	if (party)
	{
		if (party->GetLeaderPID() == GetVID() && !IsPC())
		{
			M2_DELETE(party);
		}
		else
		{
			party->Unlink(this);

			if (!IsPC())
				party->Quit(GetVID());
		}

		SetParty(nullptr);
	}

	if (m_pkMobInst)
	{
		M2_DELETE(m_pkMobInst);
		m_pkMobInst = nullptr;
	}

	m_pkMobData = nullptr;

	if (m_pkSafebox)
	{
		M2_DELETE(m_pkSafebox);
		m_pkSafebox = nullptr;
	}

	if (m_pkMall)
	{
		M2_DELETE(m_pkMall);
		m_pkMall = nullptr;
	}

	for (TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.begin();  it != m_map_buff_on_attrs.end(); it++)
	{
		if (nullptr != it->second)
		{
			M2_DELETE(it->second);
		}
	}
	m_map_buff_on_attrs.clear();

	m_set_pkChrSpawnedBy.clear();

	StopMuyeongEvent();
	event_cancel(&m_pkWarpNPCEvent);
	event_cancel(&m_pkRecoveryEvent);
	event_cancel(&m_pkDeadEvent);
	event_cancel(&m_pkSaveEvent);
	event_cancel(&m_pkTimedEvent);
	event_cancel(&m_pkStunEvent);
	event_cancel(&m_pkFishingEvent);
	event_cancel(&m_pkPoisonEvent);
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	event_cancel(&m_pkBleedingEvent);
#endif
	event_cancel(&m_pkFireEvent);
	event_cancel(&m_pkPartyRequestEvent);
	event_cancel(&m_pkWarpEvent);
	event_cancel(&m_pkCheckSpeedHackEvent);
	event_cancel(&m_pkMiningEvent);
	StopHackShieldCheckCycle();

	for (itertype(m_mapMobSkillEvent) it = m_mapMobSkillEvent.begin(); it != m_mapMobSkillEvent.end(); ++it)
	{
		LPEVENT pkEvent = it->second;
		event_cancel(&pkEvent);
	}
	m_mapMobSkillEvent.clear();

	ClearAffect();

	event_cancel(&m_pkDestroyWhenIdleEvent);

	if (m_pSkillLevels)
	{
		M2_DELETE_ARRAY(m_pSkillLevels);
		m_pSkillLevels = nullptr;
	}

	CEntity::Destroy();

	if (GetSectree())
		GetSectree()->RemoveEntity(this);

	if (m_bMonsterLog)
		CHARACTER_MANAGER::instance().UnregisterForMonsterLog(this);

	m_ItemUseTimesMap.clear(); // @correction090

#ifdef __GUILD_DRAGONLAIR__
	m_pMeley = nullptr;
#endif
#ifdef __BOSS_TRACKING_SYSTEM__
	event_cancel(&m_pkUpdateBossTracking);
#endif
#ifdef __MINI_GAME_RUMI__
	m_MiniGameRumiCardDeckCount = 0;
	m_MiniGameRumiPoints = 0;
	memset(&m_MiniGameRumiHandCards, 0, sizeof(m_MiniGameRumiHandCards));
	memset(&m_MiniGameRumiFieldCards, 0, sizeof(m_MiniGameRumiFieldCards));
	memset(&m_MiniGameRumiDeckCards, 0, sizeof(m_MiniGameRumiDeckCards));
#endif
#ifdef __MINI_GAME_FISH__
	if (m_fishSlots)
	{
		M2_DELETE_ARRAY(m_fishSlots);
		m_fishSlots = nullptr;
	}
#endif
#ifdef __MINI_GAME_CATCH_KING__
	m_vecCatchKingFieldCards.clear();
	bCatchKingHandCard = 0;
	bCatchKingHandCardLeft = 0;
	bCatchKingBetSetNumber = 0;
	dwCatchKingTotalScore = 0;
	dwCatchKingGameStatus = false;
#endif
#ifdef __12ZI__
	event_cancel(&m_pkBeadEvent);
#endif
#ifdef __FAST_RUHBK__
	event_cancel(&m_pkFastBKEvent);
	event_cancel(&m_pkFastRuhEvent);
#endif
}

const char * CHARACTER::GetName() const
{
	return m_stName.empty() ? (m_pkMobData ? m_pkMobData->m_table.szLocaleName : "") : m_stName.c_str();
}

void CHARACTER::OpenMyShop(const char * c_pszSign, TShopItemTable * pTable, uint8_t bItemCount
#ifdef __OFFLINE_PRIVATE_SHOP__
, int32_t iTime, int32_t iTitle, int32_t iPoly
#endif
)
{
	if (!CanHandleItem()) // @correction067
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중(창고,교환,상점)에는 개인상점을 사용할 수 없습니다."));
		return;
	}

#ifndef ENABLE_OPEN_SHOP_WITH_ARMOR // @correction171
	if (GetPart(PART_MAIN) > 2)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("갑옷을 벗어야 개인 상점을 열 수 있습니다."));
		return;
	}
#endif

	if (GetMyShop())
	{
		CloseMyShop();
		return;
	}

	quest::PC * pPC = quest::CQuestManager::instance().GetPCForce(GetPlayerID());

	if (pPC->IsRunning())
		return;

	if (bItemCount == 0)
		return;

	uint64_t nTotalMoney = GetGold();

	for (int32_t n = 0; n < bItemCount; ++n)
		nTotalMoney += (pTable + n)->price;

	if (GOLD_MAX <= nTotalMoney)
	{
		sys_err("[OVERFLOW_GOLD] Overflow (GOLD_MAX) id %u name %s", GetPlayerID(), GetName());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20억 냥을 초과하여 상점을 열수가 없습니다"));
		return;
	}

	char szSign[SHOP_SIGN_MAX_LEN+1];
	strlcpy(szSign, c_pszSign, sizeof(szSign));

	// m_stShopSign = szSign;

	if (strlen(szSign) == 0)
		return;

	// @correction005
	if (CBanwordManager::instance().CheckString(szSign, strlen(szSign)))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("비속어나 은어가 포함된 상점 이름으로 상점을 열 수 없습니다."));
		return;
	}

	std::map<uint32_t, uint32_t> itemkind;

	std::set<TItemPos> cont;
	for (uint8_t i = 0; i < bItemCount; ++i)
	{
		if (cont.find((pTable + i)->pos) != cont.end())
		{
			sys_err("MYSHOP: duplicate shop item detected! (name: %s)", GetName());
			return;
		}

		LPITEM pkItem = GetItem((pTable + i)->pos);

		if (pkItem)
		{
			const TItemTable * item_table = pkItem->GetProto();

			if (item_table && (IS_SET(item_table->dwAntiFlags, ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_MYSHOP)))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("유료화 아이템은 개인상점에서 판매할 수 없습니다."));
				return;
			}

			if (pkItem->IsEquipped() == true)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("장비중인 아이템은 개인상점에서 판매할 수 없습니다."));
				return;
			}

			if (true == pkItem->isLocked())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("사용중인 아이템은 개인상점에서 판매할 수 없습니다."));
				return;
			}

#ifdef __SOULBIND__ // @correction202 
			if (pkItem->IsSealed())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't trade with sealed item."));
				return;
			}
#endif

			itemkind[pkItem->GetVnum()] = (pTable + i)->price / pkItem->GetCount();
		}

		cont.insert((pTable + i)->pos);
	}

	if (CountSpecifyItem(71049))
	{
		// @correction077 BEGIN
		TItemPriceListTable header;
		memset(&header, 0, sizeof(TItemPriceListTable));

		header.dwOwnerID = GetPlayerID();
		header.byCount = itemkind.size();

		size_t idx=0;
		for (itertype(itemkind) it = itemkind.begin(); it != itemkind.end(); ++it)
		{
			header.aPriceInfo[idx].dwVnum = it->first;
			header.aPriceInfo[idx].dwPrice = it->second;
			idx++;
		}

		db_clientdesc->DBPacket(HEADER_GD_MYSHOP_PRICELIST_UPDATE, GetDesc()->GetHandle(), &header, sizeof(TItemPriceListTable));
		// @correction077 END
	}
	// else if (CountSpecifyItem(50200))
		// RemoveSpecifyItem(50200, 1);
	// else
		// return;

	if (m_pkExchange)
		m_pkExchange->Cancel();

#ifdef __OFFLINE_PRIVATE_SHOP__
	if (!iTime)
	{
#endif
	m_stShopSign = szSign;

	TPacketGCShopSign p;

	p.bHeader = HEADER_GC_SHOP_SIGN;
	p.dwVID = GetVID();
	strlcpy(p.szSign, c_pszSign, sizeof(p.szSign));

	PacketAround(&p, sizeof(TPacketGCShopSign));

	m_pkMyShop = CShopManager::instance().CreatePCShop(this, pTable, bItemCount);

	if (IsPolymorphed() == true)
		RemoveAffect(AFFECT_POLYMORPH);

	if (GetHorse())
		HorseSummon( false, true );
	else if (GetMountVnum())
	{
		RemoveAffect(AFFECT_MOUNT);
		RemoveAffect(AFFECT_MOUNT_BONUS);
	}

	SetPolymorph(30000, true);
#ifdef __OFFLINE_PRIVATE_SHOP__
	}
	else
	{
		CShopManager::instance().CreateOfflinePrivateShop(this, szSign, iTitle, iPoly, iTime, pTable, bItemCount);
	}
#endif
}

void CHARACTER::CloseMyShop()
{
	if (GetMyShop())
	{
		m_stShopSign.clear();
		CShopManager::instance().DestroyPCShop(this);
		m_pkMyShop = nullptr;

		TPacketGCShopSign p;

		p.bHeader = HEADER_GC_SHOP_SIGN;
		p.dwVID = GetVID();
		p.szSign[0] = '\0';

		PacketAround(&p, sizeof(p));
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		SetPolymorph(m_points.job, true);
#else
		SetPolymorph(GetJob(), true);
#endif
	}
}

void EncodeMovePacket(TPacketGCMove & pack, uint32_t dwVID, uint8_t bFunc, uint8_t bArg, uint32_t x, uint32_t y, uint32_t dwDuration, uint32_t dwTime, uint8_t bRot)
{
	pack.bHeader = HEADER_GC_MOVE;
	pack.bFunc   = bFunc;
	pack.bArg    = bArg;
	pack.dwVID   = dwVID;
	pack.dwTime  = dwTime ? dwTime : get_dword_time();
	pack.bRot    = bRot;
	pack.lX		= x;
	pack.lY		= y;
	pack.dwDuration	= dwDuration;
}

void CHARACTER::RestartAtSamePos()
{
	if (m_bIsObserver)
		return;

	EncodeRemovePacket(this);
	EncodeInsertPacket(this);

	ENTITY_MAP::iterator it = m_map_view.begin();

	while (it != m_map_view.end())
	{
		LPENTITY entity = (it++)->first;

		EncodeRemovePacket(entity);
		if (!m_bIsObserver)
			EncodeInsertPacket(entity);

		if( entity->IsType(ENTITY_CHARACTER) )
		{
			LPCHARACTER lpChar = (LPCHARACTER)entity;
			if( lpChar->IsPC() || lpChar->IsNPC() || lpChar->IsMonster() )
			{
				if (!entity->IsObserverMode())
					entity->EncodeInsertPacket(this);
			}
		}
		else
		{
			if( !entity->IsObserverMode())
			{
				entity->EncodeInsertPacket(this);
			}
		}
	}
}

void CHARACTER::EncodeInsertPacket(LPENTITY entity)
{

	LPDESC d;

	if (!(d = entity->GetDesc()))
		return;

	LPCHARACTER ch = (LPCHARACTER) entity;
	ch->SendGuildName(GetGuild());

	TPacketGCCharacterAdd pack;

	pack.header		= HEADER_GC_CHARACTER_ADD;
	pack.dwVID		= m_vid;
	pack.bType		= GetCharType();
	pack.angle		= GetRotation();
	pack.x		= GetX();
	pack.y		= GetY();
	pack.z		= GetZ();
	pack.wRaceNum	= GetRaceNum();
	/*if (IsMonster() || IsStone()
#ifdef __GROWTH_PET__
		|| IsNewPet()
#endif
		)
		pack.dwLevel = GetLevel();
	else
		pack.dwLevel = 0;*/
	if (IsPet()
#ifdef __MOUNT__
|| IsMountSystem()
#endif
#ifdef __SUPPORT__
|| IsSupport()
#endif
#ifdef __GROWTH_PET__
|| IsNewPet()
#endif
)
	{
		pack.bMovingSpeed	= 150;
	}
	else
	{
		pack.bMovingSpeed	= GetLimitPoint(POINT_MOV_SPEED);
	}
	pack.bAttackSpeed	= GetLimitPoint(POINT_ATT_SPEED);
	pack.dwAffectFlag[0] = m_afAffectFlag.bits[0];
	pack.dwAffectFlag[1] = m_afAffectFlag.bits[1];

	pack.bStateFlag = m_bAddChrState;

	int32_t iDur = 0;

	if (m_posDest.x != pack.x || m_posDest.y != pack.y)
	{
		iDur = (m_dwMoveStartTime + m_dwMoveDuration) - get_dword_time();

		if (iDur <= 0)
		{
			pack.x = m_posDest.x;
			pack.y = m_posDest.y;
		}
	}

	d->Packet(&pack, sizeof(pack));

	if (IsPC() == true || m_bCharType == CHAR_TYPE_NPC || IsPet()
#ifdef __SUPPORT__
|| IsSupport()
#endif
#ifdef __GROWTH_PET__
|| IsNewPet()
#endif
)
	{
		TPacketGCCharacterAdditionalInfo addPacket;
		memset(&addPacket, 0, sizeof(TPacketGCCharacterAdditionalInfo));

		addPacket.header = HEADER_GC_CHAR_ADDITIONAL_INFO;
		addPacket.dwVID = m_vid;

		addPacket.awPart[CHR_EQUIPPART_ARMOR] = GetPart(PART_MAIN);
		addPacket.awPart[CHR_EQUIPPART_WEAPON] = GetPart(PART_WEAPON);
		addPacket.awPart[CHR_EQUIPPART_HEAD] = GetPart(PART_HEAD);
		addPacket.awPart[CHR_EQUIPPART_HAIR] = GetPart(PART_HAIR);
#ifdef __ACCE_SYSTEM__ // @correction191
		addPacket.awPart[CHR_EQUIPPART_ACCE] = GetPart(PART_ACCE);
#endif
#ifdef __QUIVER__ // @correction203
		addPacket.awPart[CHR_EQUIPPART_ARROW_TYPE] = GetPart(PART_ARROW_TYPE);
#endif
#ifdef __COSTUME_EFFECT__
		addPacket.awPart[CHR_EQUIPPART_BODY_EFFECT] = GetPart(PART_BODY_EFFECT);
		addPacket.awPart[CHR_EQUIPPART_WEAPON_RIGHT_EFFECT] = GetPart(PART_WEAPON_RIGHT_EFFECT);
		addPacket.awPart[CHR_EQUIPPART_WEAPON_LEFT_EFFECT] = GetPart(PART_WEAPON_LEFT_EFFECT);
#endif
#ifdef __ELEMENT_ADD__
		addPacket.bElement = GetElement();
#endif
#ifdef __MULTI_LANGUAGE__
		strlcpy(addPacket.szLanguage, GetLanguage(), sizeof(addPacket.szLanguage));
#endif
		addPacket.bPKMode = m_bPKMode;
		addPacket.dwMountVnum = GetMountVnum();
		addPacket.bEmpire = m_bEmpire;

#ifdef ENABLE_SHOWNPCLEVEL // @correction172
		if (1)
#else
		// @correction005
		if (IsPC()
#ifdef __SUPPORT__
|| IsSupport()
#endif
#ifdef __GROWTH_PET__
|| IsNewPet()
#endif
)
#endif
		{
			addPacket.dwLevel = GetLevel();
		}
		else
		{
			addPacket.dwLevel = 0;
		}

#ifdef __SUPPORT__
		if (IsSupport())
		{
			addPacket.awPart[CHR_EQUIPPART_ARMOR] = GetSupportArmor();
			addPacket.awPart[CHR_EQUIPPART_WEAPON] = GetSupportWeapon();
			addPacket.awPart[CHR_EQUIPPART_HAIR] = GetSupportHair();
		}
#endif

		if (false)
		{
			LPCHARACTER ch = (LPCHARACTER) entity;

			if (GetEmpire() == ch->GetEmpire() || ch->GetGMLevel() > GM_PLAYER || m_bCharType == CHAR_TYPE_NPC || IsPet()
#ifdef __SUPPORT__
|| IsSupport()
#endif
#ifdef __GROWTH_PET__
|| IsNewPet()
#endif
)
			{
				goto show_all_info;
			}
			else
			{
				memset(addPacket.name, 0, CHARACTER_NAME_MAX_LEN);
				addPacket.dwGuildID = 0;
				addPacket.sAlignment = 0;
			}
		}
		else
		{
		show_all_info:
			strlcpy(addPacket.name, GetName(), sizeof(addPacket.name));

			if (GetGuild() != nullptr)
			{
				addPacket.dwGuildID = GetGuild()->GetID();
#ifdef __GUILD_GENERAL_AND_LEADER__
				CGuild* pGuild = this->GetGuild();
				if (pGuild->GetMasterPID() == GetPlayerID())
					addPacket.bNewIsGuildName = 3;

				else if (pGuild->NewIsGuildGeneral(GetPlayerID()) == true)
					addPacket.bNewIsGuildName = 2;

				else
					addPacket.bNewIsGuildName = 1;
#endif
			}
			else
			{
				addPacket.dwGuildID = 0;
#ifdef __GUILD_GENERAL_AND_LEADER__
				addPacket.bNewIsGuildName = 0;
#endif
			}

			addPacket.sAlignment = m_iAlignment / 10;
		}

		d->Packet(&addPacket, sizeof(TPacketGCCharacterAdditionalInfo));
	}

	if (iDur)
	{
		TPacketGCMove pack;
		EncodeMovePacket(pack, GetVID(), FUNC_MOVE, 0, m_posDest.x, m_posDest.y, iDur, 0, (uint8_t) (GetRotation() / 5));
		d->Packet(&pack, sizeof(pack));

		TPacketGCWalkMode p;
		p.vid = GetVID();
		p.header = HEADER_GC_WALK_MODE;
		p.mode = m_bNowWalking ? WALKMODE_WALK : WALKMODE_RUN;

		d->Packet(&p, sizeof(p));
	}

	if (entity->IsType(ENTITY_CHARACTER) && GetDesc())
	{
		LPCHARACTER ch = (LPCHARACTER) entity;
		if (ch->IsWalking())
		{
			TPacketGCWalkMode p;
			p.vid = ch->GetVID();
			p.header = HEADER_GC_WALK_MODE;
			p.mode = ch->m_bNowWalking ? WALKMODE_WALK : WALKMODE_RUN;
			GetDesc()->Packet(&p, sizeof(p));
		}
	}

	if (GetMyShop())
	{
		TPacketGCShopSign p;

		p.bHeader = HEADER_GC_SHOP_SIGN;
		p.dwVID = GetVID();
		strlcpy(p.szSign, m_stShopSign.c_str(), sizeof(p.szSign));

		d->Packet(&p, sizeof(TPacketGCShopSign));
	}

	if (entity->IsType(ENTITY_CHARACTER))
	{
		sys_log(3, "EntityInsert %s (RaceNum %d) (%d %d) TO %s",
				GetName(), GetRaceNum(), GetX() / SECTREE_SIZE, GetY() / SECTREE_SIZE, ((LPCHARACTER)entity)->GetName());
	}
}

void CHARACTER::EncodeRemovePacket(LPENTITY entity)
{
	if (entity->GetType() != ENTITY_CHARACTER)
		return;

	LPDESC d;

	if (!(d = entity->GetDesc()))
		return;

	TPacketGCCharacterDelete pack;

	pack.header	= HEADER_GC_CHARACTER_DEL;
	pack.id	= m_vid;

	d->Packet(&pack, sizeof(TPacketGCCharacterDelete));

	if (entity->IsType(ENTITY_CHARACTER))
		sys_log(3, "EntityRemove %s(%d) FROM %s", GetName(), (uint32_t) m_vid, ((LPCHARACTER) entity)->GetName());
}

void CHARACTER::UpdatePacket()
{
	if (GetSectree() == nullptr) return;

	TPacketGCCharacterUpdate pack;
	TPacketGCCharacterUpdate pack2;

	pack.header = HEADER_GC_CHARACTER_UPDATE;
	pack.dwVID = m_vid;

	pack.awPart[CHR_EQUIPPART_ARMOR] = GetPart(PART_MAIN);
	pack.awPart[CHR_EQUIPPART_WEAPON] = GetPart(PART_WEAPON);
	pack.awPart[CHR_EQUIPPART_HEAD] = GetPart(PART_HEAD);
	pack.awPart[CHR_EQUIPPART_HAIR] = GetPart(PART_HAIR);
#ifdef __ACCE_SYSTEM__ // @correction191
	pack.awPart[CHR_EQUIPPART_ACCE] = GetPart(PART_ACCE);
#endif
#ifdef __QUIVER__ // @correction203
	pack.awPart[CHR_EQUIPPART_ARROW_TYPE] = GetPart(PART_ARROW_TYPE);
#endif
#ifdef __COSTUME_EFFECT__
	pack.awPart[CHR_EQUIPPART_BODY_EFFECT] = GetPart(PART_BODY_EFFECT);
	pack.awPart[CHR_EQUIPPART_WEAPON_RIGHT_EFFECT] = GetPart(PART_WEAPON_RIGHT_EFFECT);
	pack.awPart[CHR_EQUIPPART_WEAPON_LEFT_EFFECT] = GetPart(PART_WEAPON_LEFT_EFFECT);
#endif
#ifdef __ELEMENT_ADD__
	pack.bElement = GetElement();
#endif

#ifdef __SUPPORT__
	if (IsSupport())
	{
		pack.awPart[CHR_EQUIPPART_ARMOR] = GetSupportArmor();
		pack.awPart[CHR_EQUIPPART_WEAPON] = GetSupportWeapon();
		pack.awPart[CHR_EQUIPPART_HAIR] = GetSupportHair();
	}
#endif
	pack.bMovingSpeed	= GetLimitPoint(POINT_MOV_SPEED);
	pack.bAttackSpeed	= GetLimitPoint(POINT_ATT_SPEED);
	pack.bStateFlag	= m_bAddChrState;
	pack.dwAffectFlag[0] = m_afAffectFlag.bits[0];
	pack.dwAffectFlag[1] = m_afAffectFlag.bits[1];
	pack.dwGuildID	= 0;
	pack.sAlignment	= m_iAlignment / 10;
	pack.bPKMode	= m_bPKMode;

	if (GetGuild())
		pack.dwGuildID = GetGuild()->GetID();

	pack.dwMountVnum	= GetMountVnum();

#ifdef __GUILD_GENERAL_AND_LEADER__
	CGuild* pGuild = this->GetGuild();
	if (pGuild)
	{
		if (pGuild->GetMasterPID() == GetPlayerID())
			pack.bNewIsGuildName = 3;
		else if (pGuild->NewIsGuildGeneral(GetPlayerID()) == true)
			pack.bNewIsGuildName = 2;
		else
			pack.bNewIsGuildName = 1;
	}
	else
		pack.bNewIsGuildName = 0;
#endif

	pack2 = pack;
	pack2.dwGuildID = 0;
	pack2.sAlignment = 0;

	if (false)
	{
		if (m_bIsObserver != true)
		{
			for (ENTITY_MAP::iterator iter = m_map_view.begin(); iter != m_map_view.end(); iter++)
			{
				LPENTITY pEntity = iter->first;

				if (pEntity != nullptr)
				{
					if (pEntity->IsType(ENTITY_CHARACTER) == true)
					{
						if (pEntity->GetDesc() != nullptr)
						{
							LPCHARACTER pChar = (LPCHARACTER)pEntity;

							if (GetEmpire() == pChar->GetEmpire() || pChar->GetGMLevel() > GM_PLAYER)
							{
								pEntity->GetDesc()->Packet(&pack, sizeof(pack));
							}
							else
							{
								pEntity->GetDesc()->Packet(&pack2, sizeof(pack2));
							}
						}
					}
					else
					{
						if (pEntity->GetDesc() != nullptr)
						{
							pEntity->GetDesc()->Packet(&pack, sizeof(pack));
						}
					}
				}
			}
		}

		if (GetDesc() != nullptr)
		{
			GetDesc()->Packet(&pack, sizeof(pack));
		}
	}
	else
	{
		PacketAround(&pack, sizeof(pack));
	}
}

LPCHARACTER CHARACTER::FindCharacterInView(const char * c_pszName, bool bFindPCOnly)
{
	ENTITY_MAP::iterator it = m_map_view.begin();

	for (; it != m_map_view.end(); ++it)
	{
		if (!it->first->IsType(ENTITY_CHARACTER))
			continue;

		LPCHARACTER tch = (LPCHARACTER) it->first;

		if (bFindPCOnly && tch->IsNPC())
			continue;

		if (!strcasecmp(tch->GetName(), c_pszName))
			return (tch);
	}

	return nullptr;
}

void CHARACTER::SetPosition(int32_t pos)
{
	if (pos == POS_STANDING)
	{
		REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_DEAD);
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_STUN);

		event_cancel(&m_pkDeadEvent);
		event_cancel(&m_pkStunEvent);
	}
	else if (pos == POS_DEAD)
		SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_DEAD);

	if (!IsStone())
	{
		switch (pos)
		{
			case POS_FIGHTING:
				if (!IsState(m_stateBattle))
					MonsterLog("[BATTLE] 싸우는 상태");

				GotoState(m_stateBattle);
				break;

			default:
				if (!IsState(m_stateIdle))
					MonsterLog("[IDLE] 쉬는 상태");

				GotoState(m_stateIdle);
				break;
		}
	}

	m_pointsInstant.position = pos;
}

void CHARACTER::Save()
{
	if (!m_bSkipSave)
		CHARACTER_MANAGER::instance().DelayedSave(this);
}

void CHARACTER::CreatePlayerProto(TPlayerTable & tab)
{
	memset(&tab, 0, sizeof(TPlayerTable));

	if (GetNewName().empty())
	{
		strlcpy(tab.name, GetName(), sizeof(tab.name));
	}
	else
	{
		strlcpy(tab.name, GetNewName().c_str(), sizeof(tab.name));
	}

	strlcpy(tab.ip, GetDesc()->GetHostName(), sizeof(tab.ip));

	tab.id			= m_dwPlayerID;
	tab.voice		= GetPoint(POINT_VOICE);
	tab.level		= GetLevel();
	tab.level_step	= GetPoint(POINT_LEVEL_STEP);
	tab.exp			= GetExp();
	tab.gold		= GetGold();
	tab.job			= m_points.job;
	tab.part_base	= m_pointsInstant.bBasePart;
	tab.skill_group	= m_points.skill_group;
#ifdef __12ZI__
	tab.bBead = GetBead();
	tab.dwBeadTime = iRealBeadTime - time(nullptr) > 0 ? iRealBeadTime - time(nullptr) : 0;
#endif

	uint32_t dwPlayedTime = (get_dword_time() - m_dwPlayStartTime);

	if (dwPlayedTime > 60000)
	{
		if (GetSectree() && !GetSectree()->IsAttr(GetX(), GetY(), ATTR_BANPK))
		{
			if (GetRealAlignment() < 0)
			{
				if (IsEquipUniqueItem(UNIQUE_ITEM_FASTER_ALIGNMENT_UP_BY_TIME))
					UpdateAlignment(120 * (dwPlayedTime / 60000));
				else
					UpdateAlignment(60 * (dwPlayedTime / 60000));
			}
			else
				UpdateAlignment(5 * (dwPlayedTime / 60000));
		}

		SetRealPoint(POINT_PLAYTIME, GetRealPoint(POINT_PLAYTIME) + dwPlayedTime / 60000);
		ResetPlayTime(dwPlayedTime % 60000);
	}

	tab.playtime = GetRealPoint(POINT_PLAYTIME);
	tab.lAlignment = m_iRealAlignment;

	if (m_posWarp.x != 0 || m_posWarp.y != 0)
	{
		tab.x = m_posWarp.x;
		tab.y = m_posWarp.y;
		tab.z = 0;
		tab.lMapIndex = m_lWarpMapIndex;
	}
	else
	{
		tab.x = GetX();
		tab.y = GetY();
		tab.z = GetZ();
		tab.lMapIndex	= GetMapIndex();
	}

	if (m_lExitMapIndex == 0)
	{
		tab.lExitMapIndex	= tab.lMapIndex;
		tab.lExitX		= tab.x;
		tab.lExitY		= tab.y;
	}
	else
	{
		tab.lExitMapIndex	= m_lExitMapIndex;
		tab.lExitX		= m_posExit.x;
		tab.lExitY		= m_posExit.y;
	}

	sys_log(0, "SAVE: %s %dx%d", GetName(), tab.x, tab.y);

	tab.st = GetRealPoint(POINT_ST);
	tab.ht = GetRealPoint(POINT_HT);
	tab.dx = GetRealPoint(POINT_DX);
	tab.iq = GetRealPoint(POINT_IQ);

	tab.stat_point = GetPoint(POINT_STAT);
	tab.skill_point = GetPoint(POINT_SKILL);
	tab.sub_skill_point = GetPoint(POINT_SUB_SKILL);
	tab.horse_skill_point = GetPoint(POINT_HORSE_SKILL);

	tab.stat_reset_count = GetPoint(POINT_STAT_RESET_COUNT);

	tab.hp = GetHP();
	tab.sp = GetSP();

	tab.stamina = GetStamina();

	tab.sRandomHP = m_points.iRandomHP;
	tab.sRandomSP = m_points.iRandomSP;

	for (int32_t i = 0; i < QUICKSLOT_MAX_NUM; ++i)
		tab.quickslot[i] = m_quickslot[i];

	if (m_stMobile.length() && !*m_szMobileAuth)
		strlcpy(tab.szMobile, m_stMobile.c_str(), sizeof(tab.szMobile));

	memcpy(tab.parts, m_pointsInstant.parts, sizeof(tab.parts));

	memcpy(tab.skills, m_pSkillLevels, sizeof(TPlayerSkill) * SKILL_MAX_NUM);

#ifdef __MINI_GAME_FISH__
	memcpy(tab.fishSlots, m_fishSlots, sizeof(TPlayerFishEventSlot)* FISH_EVENT_SLOTS_NUM);
	tab.fishEventUseCount = GetFishEventUseCount();
#endif

	tab.horse = GetHorseData();
}


void CHARACTER::SaveReal()
{
	if (m_bSkipSave)
		return;

	if (!GetDesc())
	{
		sys_err("Character::Save : no descriptor when saving (name: %s)", GetName());
		return;
	}

	TPlayerTable table;
	CreatePlayerProto(table);

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_SAVE, GetDesc()->GetHandle(), &table, sizeof(TPlayerTable));

	quest::PC * pkQuestPC = quest::CQuestManager::instance().GetPCForce(GetPlayerID());

	if (!pkQuestPC)
		sys_err("CHARACTER::Save : null quest::PC pointer! (name %s)", GetName());
	else
	{
		pkQuestPC->Save();
	}

	marriage::TMarriage* pMarriage = marriage::CManager::instance().Get(GetPlayerID());
	if (pMarriage)
		pMarriage->Save();
}

void CHARACTER::FlushDelayedSaveItem()
{
	LPITEM item;

	for (int32_t i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; ++i)
		if ((item = GetInventoryItem(i)))
			ITEM_MANAGER::instance().FlushDelayedSave(item);
}

void CHARACTER::Disconnect(const char * c_pszReason)
{
	assert(GetDesc() != nullptr);

	sys_log(0, "DISCONNECT: %s (%s)", GetName(), c_pszReason ? c_pszReason : "unset" );

	if (GetShop())
	{
		GetShop()->RemoveGuest(this);
		SetShop(nullptr);
	}

	if (GetArena() != nullptr)
	{
		GetArena()->OnDisconnect(GetPlayerID());
	}

	if (GetParty() != nullptr)
	{
		GetParty()->UpdateOfflineState(GetPlayerID());
	}

	marriage::CManager::instance().Logout(this);

	TPacketGGLogout p;
	p.bHeader = HEADER_GG_LOGOUT;
	strlcpy(p.szName, GetName(), sizeof(p.szName));
	P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGLogout));
	LogManager::instance().CharLog(this, 0, "LOGOUT", "");
#ifdef __GUILD_DRAGONLAIR__
	if (CMgr::instance().IsMeleyMap(GetMapIndex()))
		CMgr::instance().Leave(this, false);
#endif
#ifdef ENABLE_PCBANG_FEATURE // @correction005
	{
		int32_t playTime = GetRealPoint(POINT_PLAYTIME) - m_dwLoginPlayTime;
		LogManager::instance().LoginLog(false, GetDesc()->GetAccountTable().id, GetPlayerID(), GetLevel(), GetJob(), playTime);

		if (0)
			CPCBangManager::instance().Log(GetDesc()->GetHostName(), GetPlayerID(), playTime);
	}
#endif

	if (m_pWarMap)
		SetWarMap(nullptr);

	if (m_pWeddingMap)
	{
		SetWeddingMap(nullptr);
	}

	if (GetGuild())
		GetGuild()->LogoutMember(this);

	quest::CQuestManager::instance().LogoutPC(this);

#ifdef __12ZI__
	DBManager::instance().DirectQuery("UPDATE player.player SET last_play = NOW() WHERE name = '%s'", GetName());
#endif

	if (GetParty())
		GetParty()->Unlink(this);

	if (IsStun() || IsDead())
	{
		DeathPenalty(0);
		PointChange(POINT_HP, 50 - GetHP());
	}


	if (!CHARACTER_MANAGER::instance().FlushDelayedSave(this))
	{
		SaveReal();
	}

	FlushDelayedSaveItem();

	SaveAffect();
	m_bIsLoadedAffect = false;

	m_bSkipSave = true;

	quest::CQuestManager::instance().DisconnectPC(this);

	CloseSafebox();

	CloseMall();

	CPVPManager::instance().Disconnect(this);

	CTargetManager::instance().Logout(GetPlayerID());

	MessengerManager::instance().Logout(GetName());

	if (g_TeenDesc)
	{
		int32_t		offset = 0;
		char	buf[245] = {0};

		buf[0] = HEADER_GT_LOGOUT;
		offset += 1;

		memset(buf+offset, 0x00, 2);
		offset += 2;

		TAccountTable	&acc_table = GetDesc()->GetAccountTable();
		memcpy(buf+offset, &acc_table.id, 4);
		offset += 4;

		g_TeenDesc->Packet(buf, offset);
	}

	if (GetDesc())
	{
		GetDesc()->BindCharacter(nullptr);
	}

	CXTrapManager::instance().DestroyClientSession(this);

	M2_DESTROY_CHARACTER(this);
}

bool CHARACTER::Show(int32_t lMapIndex, int32_t x, int32_t y, int32_t z, bool bShowSpawnMotion)
{
	LPSECTREE sectree = SECTREE_MANAGER::instance().Get(lMapIndex, x, y);

	if (!sectree)
	{
		sys_log(0, "cannot find sectree by %dx%d mapindex %d", x, y, lMapIndex);
		return false;
	}

	SetMapIndex(lMapIndex);

	bool bChangeTree = false;

	if (!GetSectree() || GetSectree() != sectree)
		bChangeTree = true;

	if (bChangeTree)
	{
		if (GetSectree())
			GetSectree()->RemoveEntity(this);

		ViewCleanup();
	}

	if (!IsNPC())
	{
		sys_log(0, "SHOW: %s %dx%dx%d", GetName(), x, y, z);
		if (GetStamina() < GetMaxStamina())
			StartAffectEvent();
	}
	else if (m_pkMobData)
	{
		m_pkMobInst->m_posLastAttacked.x = x;
		m_pkMobInst->m_posLastAttacked.y = y;
		m_pkMobInst->m_posLastAttacked.z = z;
	}

	if (bShowSpawnMotion)
	{
		SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);
		m_afAffectFlag.Set(AFF_SPAWN);
	}

	SetXYZ(x, y, z);

	m_posDest.x = x;
	m_posDest.y = y;
	m_posDest.z = z;

	m_posStart.x = x;
	m_posStart.y = y;
	m_posStart.z = z;

	if (bChangeTree)
	{
		EncodeInsertPacket(this);
		sectree->InsertEntity(this);

		UpdateSectree();
	}
	else
	{
		ViewReencode();
		sys_log(0, "      in same sectree");
	}

	REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);

	SetValidComboInterval(0);
	return true;
}

struct BGMInfo
{
	std::string	name;
	float		vol;
};

typedef std::map<uint32_t, BGMInfo> BGMInfoMap;

static BGMInfoMap 	gs_bgmInfoMap;
static bool		gs_bgmVolEnable = false;

void CHARACTER_SetBGMVolumeEnable()
{
	gs_bgmVolEnable = true;
	sys_log(0, "bgm_info.set_bgm_volume_enable");
}

void CHARACTER_AddBGMInfo(uint32_t mapIndex, const char* name, float vol)
{
	BGMInfo newInfo;
	newInfo.name = name;
	newInfo.vol = vol;

	gs_bgmInfoMap[mapIndex] = newInfo;

	sys_log(0, "bgm_info.add_info(%d, '%s', %f)", mapIndex, name, vol);
}

const BGMInfo& CHARACTER_GetBGMInfo(uint32_t mapIndex)
{
	BGMInfoMap::iterator f = gs_bgmInfoMap.find(mapIndex);
	if (gs_bgmInfoMap.end() == f)
	{
		static BGMInfo s_empty = {"", 0.0f};
		return s_empty;
	}
	return f->second;
}

bool CHARACTER_IsBGMVolumeEnable()
{
	return gs_bgmVolEnable;
}

void CHARACTER::MainCharacterPacket()
{
	const uint32_t mapIndex = GetMapIndex();
	const BGMInfo& bgmInfo = CHARACTER_GetBGMInfo(mapIndex);

	if (!bgmInfo.name.empty())
	{
		if (CHARACTER_IsBGMVolumeEnable())
		{
			sys_log(1, "bgm_info.play_bgm_vol(%d, name='%s', vol=%f)", mapIndex, bgmInfo.name.c_str(), bgmInfo.vol);
			TPacketGCMainCharacter4_BGM_VOL mainChrPacket;
			mainChrPacket.header = HEADER_GC_MAIN_CHARACTER4_BGM_VOL;
			mainChrPacket.dwVID = m_vid;
			mainChrPacket.wRaceNum = GetRaceNum();
			mainChrPacket.lx = GetX();
			mainChrPacket.ly = GetY();
			mainChrPacket.lz = GetZ();
			mainChrPacket.empire = GetDesc()->GetEmpire();
			mainChrPacket.skill_group = GetSkillGroup();
			strlcpy(mainChrPacket.szChrName, GetName(), sizeof(mainChrPacket.szChrName));

			mainChrPacket.fBGMVol = bgmInfo.vol;
			strlcpy(mainChrPacket.szBGMName, bgmInfo.name.c_str(), sizeof(mainChrPacket.szBGMName));
			GetDesc()->Packet(&mainChrPacket, sizeof(TPacketGCMainCharacter4_BGM_VOL));
		}
		else
		{
			sys_log(1, "bgm_info.play(%d, '%s')", mapIndex, bgmInfo.name.c_str());
			TPacketGCMainCharacter3_BGM mainChrPacket;
			mainChrPacket.header = HEADER_GC_MAIN_CHARACTER3_BGM;
			mainChrPacket.dwVID = m_vid;
			mainChrPacket.wRaceNum = GetRaceNum();
			mainChrPacket.lx = GetX();
			mainChrPacket.ly = GetY();
			mainChrPacket.lz = GetZ();
			mainChrPacket.empire = GetDesc()->GetEmpire();
			mainChrPacket.skill_group = GetSkillGroup();
			strlcpy(mainChrPacket.szChrName, GetName(), sizeof(mainChrPacket.szChrName));
			strlcpy(mainChrPacket.szBGMName, bgmInfo.name.c_str(), sizeof(mainChrPacket.szBGMName));
			GetDesc()->Packet(&mainChrPacket, sizeof(TPacketGCMainCharacter3_BGM));
		}
	}
	else
	{
		sys_log(0, "bgm_info.play(%d, DEFAULT_BGM_NAME)", mapIndex);

		TPacketGCMainCharacter pack;
		pack.header = HEADER_GC_MAIN_CHARACTER;
		pack.dwVID = m_vid;
		pack.wRaceNum = GetRaceNum();
		pack.lx = GetX();
		pack.ly = GetY();
		pack.lz = GetZ();
		pack.empire = GetDesc()->GetEmpire();
		pack.skill_group = GetSkillGroup();
		strlcpy(pack.szName, GetName(), sizeof(pack.szName));
		GetDesc()->Packet(&pack, sizeof(TPacketGCMainCharacter));

		if (m_stMobile.length())
			ChatPacket(CHAT_TYPE_COMMAND, "sms");
	}
}

void CHARACTER::PointsPacket()
{
	if (!GetDesc())
		return;

	TPacketGCPoints pack;

	pack.header	= HEADER_GC_CHARACTER_POINTS;

	pack.points[POINT_LEVEL]		= GetLevel();
	pack.points[POINT_EXP]		= GetExp();
	pack.points[POINT_NEXT_EXP]		= GetNextExp();
	pack.points[POINT_HP]		= GetHP();
	pack.points[POINT_MAX_HP]		= GetMaxHP();
	pack.points[POINT_SP]		= GetSP();
	pack.points[POINT_MAX_SP]		= GetMaxSP();
	pack.points[POINT_STAMINA]		= GetStamina();
	pack.points[POINT_MAX_STAMINA]	= GetMaxStamina();

	for (int32_t i = POINT_ST; i < POINT_MAX_NUM; ++i)
		pack.points[i] = GetPoint(i);

	GetDesc()->Packet(&pack, sizeof(TPacketGCPoints));
}

bool CHARACTER::ChangeSex()
{
	int32_t src_race = GetRaceNum();

	switch (src_race)
	{
		case MAIN_RACE_WARRIOR_M:
			m_points.job = MAIN_RACE_WARRIOR_W;
			break;

		case MAIN_RACE_WARRIOR_W:
			m_points.job = MAIN_RACE_WARRIOR_M;
			break;

		case MAIN_RACE_ASSASSIN_M:
			m_points.job = MAIN_RACE_ASSASSIN_W;
			break;

		case MAIN_RACE_ASSASSIN_W:
			m_points.job = MAIN_RACE_ASSASSIN_M;
			break;

		case MAIN_RACE_SURA_M:
			m_points.job = MAIN_RACE_SURA_W;
			break;

		case MAIN_RACE_SURA_W:
			m_points.job = MAIN_RACE_SURA_M;
			break;

		case MAIN_RACE_SHAMAN_M:
			m_points.job = MAIN_RACE_SHAMAN_W;
			break;

		case MAIN_RACE_SHAMAN_W:
			m_points.job = MAIN_RACE_SHAMAN_M;
			break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case MAIN_RACE_WOLFMAN_M:
			m_points.job = MAIN_RACE_WOLFMAN_M;
			break;
#endif
		default:
			sys_err("CHANGE_SEX: %s unknown race %d", GetName(), src_race);
			return false;
	}

	sys_log(0, "CHANGE_SEX: %s (%d -> %d)", GetName(), src_race, m_points.job);
	return true;
}

uint16_t CHARACTER::GetRaceNum() const
{
	if (m_dwPolymorphRace)
		return m_dwPolymorphRace;

	if (m_pkMobData)
		return m_pkMobData->m_table.dwVnum;

	return m_points.job;
}

void CHARACTER::SetRace(uint8_t race)
{
	if (race >= MAIN_RACE_MAX_NUM)
	{
		sys_err("CHARACTER::SetRace(name=%s, race=%d).OUT_OF_RACE_RANGE", GetName(), race);
		return;
	}

	m_points.job = race;
}

uint8_t CHARACTER::GetJob() const
{
	uint32_t race = m_points.job;
	uint32_t job;

	if (RaceToJob(race, &job))
		return job;

	sys_err("CHARACTER::GetJob(name=%s, race=%d).OUT_OF_RACE_RANGE", GetName(), race);
	return JOB_WARRIOR;
}

void CHARACTER::SetLevel(uint8_t level)
{
	m_points.level = level;

	if (IsPC())
	{
		if (level < PK_PROTECT_LEVEL)
			SetPKMode(PK_MODE_PROTECT);
		else if (GetGMLevel() != GM_PLAYER)
			SetPKMode(PK_MODE_PROTECT);
		else if (m_bPKMode == PK_MODE_PROTECT)
			SetPKMode(PK_MODE_PEACE);
	}
}

void CHARACTER::SetEmpire(uint8_t bEmpire)
{
	m_bEmpire = bEmpire;
}

void CHARACTER::SetPlayerProto(const TPlayerTable * t)
{
	if (!GetDesc() || !*GetDesc()->GetHostName())
		sys_err("cannot get desc or hostname");
	else
		SetGMLevel();

	m_bCharType = CHAR_TYPE_PC;

	m_dwPlayerID = t->id;

	m_iAlignment = t->lAlignment;
	m_iRealAlignment = t->lAlignment;

	m_points.voice = t->voice;

	m_points.skill_group = t->skill_group;

	m_pointsInstant.bBasePart = t->part_base;
	SetPart(PART_HAIR, t->parts[PART_HAIR]);
#ifdef __ACCE_SYSTEM__ // @correction191
	SetPart(PART_ACCE, t->parts[PART_ACCE]);
#endif
	m_points.iRandomHP = t->sRandomHP;
	m_points.iRandomSP = t->sRandomSP;

	if (m_pSkillLevels)
		M2_DELETE_ARRAY(m_pSkillLevels);

	m_pSkillLevels = M2_NEW TPlayerSkill[SKILL_MAX_NUM];
	memcpy(m_pSkillLevels, t->skills, sizeof(TPlayerSkill) * SKILL_MAX_NUM);

#ifdef __MINI_GAME_FISH__
	if (m_fishSlots)
		M2_DELETE_ARRAY(m_fishSlots);

	m_fishSlots = M2_NEW TPlayerFishEventSlot[FISH_EVENT_SLOTS_NUM];
	memcpy(m_fishSlots, t->fishSlots, sizeof(TPlayerFishEventSlot) * FISH_EVENT_SLOTS_NUM);

	m_dwFishUseCount = t->fishEventUseCount;
#endif

	// if (t->lMapIndex >= 10000)
	// {
		// m_posWarp.x = t->lExitX;
		// m_posWarp.y = t->lExitY;
		// m_lWarpMapIndex = t->lExitMapIndex;
	// }

	SetRealPoint(POINT_PLAYTIME, t->playtime);
	m_dwLoginPlayTime = t->playtime;
	SetRealPoint(POINT_ST, t->st);
	SetRealPoint(POINT_HT, t->ht);
	SetRealPoint(POINT_DX, t->dx);
	SetRealPoint(POINT_IQ, t->iq);

	SetPoint(POINT_ST, t->st);
	SetPoint(POINT_HT, t->ht);
	SetPoint(POINT_DX, t->dx);
	SetPoint(POINT_IQ, t->iq);

	SetPoint(POINT_STAT, t->stat_point);
	SetPoint(POINT_SKILL, t->skill_point);
	SetPoint(POINT_SUB_SKILL, t->sub_skill_point);
	SetPoint(POINT_HORSE_SKILL, t->horse_skill_point);

	SetPoint(POINT_STAT_RESET_COUNT, t->stat_reset_count);

	SetPoint(POINT_LEVEL_STEP, t->level_step);
	SetRealPoint(POINT_LEVEL_STEP, t->level_step);

	SetRace(t->job);

	SetLevel(t->level);
	SetExp(t->exp);
	SetGold(t->gold);

	SetMapIndex(t->lMapIndex);
	SetXYZ(t->x, t->y, t->z);

	ComputePoints();

	SetHP(t->hp);
	SetSP(t->sp);
	SetStamina(t->stamina);
#ifdef __12ZI__
	SetBead(t->bBead);
	SetBeadTime(t->dwBeadTime);
#endif

#ifndef ENABLE_GM_FLAG_IF_TEST_SERVER // @correction174
	if (!test_server)
#endif
	{
#ifdef ENABLE_GM_FLAG_FOR_LOW_WIZARD // @correction175
		if (GetGMLevel() > GM_PLAYER)
#else
		if (GetGMLevel() > GM_LOW_WIZARD)
#endif
		{
			m_afAffectFlag.Set(AFF_YMIR);
			m_bPKMode = PK_MODE_PROTECT;
		}
	}

	if (GetLevel() < PK_PROTECT_LEVEL)
		m_bPKMode = PK_MODE_PROTECT;

	m_stMobile = t->szMobile;

	SetHorseData(t->horse);

	if (GetHorseLevel() > 0)
		UpdateHorseDataByLogoff(t->logoff_interval);

	memcpy(m_aiPremiumTimes, t->aiPremiumTimes, sizeof(t->aiPremiumTimes));

	m_dwLogOffInterval = t->logoff_interval;

	sys_log(0, "PLAYER_LOAD: %s PREMIUM %d %d, LOGGOFF_INTERVAL %u PTR: %p", t->name, m_aiPremiumTimes[0], m_aiPremiumTimes[1], t->logoff_interval, this);

	if (GetGMLevel() != GM_PLAYER)
	{
		LogManager::instance().CharLog(this, GetGMLevel(), "GM_LOGIN", "");
		sys_log(0, "GM_LOGIN(gmlevel=%d, name=%s(%d), pos=(%d, %d)", GetGMLevel(), GetName(), GetPlayerID(), GetX(), GetY());
	}

#ifdef __PET_SYSTEM__
	if (m_petSystem)
	{
		m_petSystem->Destroy();
		delete m_petSystem;
	}

	m_petSystem = M2_NEW CPetSystem(this);
#endif

#ifdef __MOUNT__
	if (m_MountSystem)
	{
		m_MountSystem->Destroy();
		delete m_MountSystem;
	}

	m_MountSystem = M2_NEW CMountSystem(this);
#endif

#ifdef __SUPPORT__
	if (m_supportSystem)
	{
		m_supportSystem->Destroy();
		delete m_supportSystem;
	}

	m_supportSystem = M2_NEW CSupport(this);
#endif


#ifdef __GROWTH_PET__
	if (m_newpetSystem)
	{
		m_newpetSystem->Destroy();
		delete m_newpetSystem;
	}

	m_newpetSystem = M2_NEW CNewPetSystem(this);
#endif
}

EVENTFUNC(kill_ore_load_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );
	if ( info == nullptr )
	{
		sys_err( "kill_ore_load_even> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;
	if (ch == nullptr) {
		return 0;
	}

	ch->m_pkMiningEvent = nullptr;
	M2_DESTROY_CHARACTER(ch);
	return 0;
}

void CHARACTER::SetProto(const CMob * pkMob)
{
	if (m_pkMobInst)
		M2_DELETE(m_pkMobInst);

	m_pkMobData = pkMob;
	m_pkMobInst = M2_NEW CMobInstance;

	m_bPKMode = PK_MODE_FREE;

	const TMobTable * t = &m_pkMobData->m_table;

	m_bCharType = t->bType;

	SetLevel(t->bLevel);
	SetEmpire(t->bEmpire);

	SetExp(t->dwExp);
	SetRealPoint(POINT_ST, t->bStr);
	SetRealPoint(POINT_DX, t->bDex);
	SetRealPoint(POINT_HT, t->bCon);
	SetRealPoint(POINT_IQ, t->bInt);

	ComputePoints();

	SetHP(GetMaxHP());
	SetSP(GetMaxSP());

	m_pointsInstant.dwAIFlag = t->dwAIFlag;
	SetImmuneFlag(t->dwImmuneFlag);

	AssignTriggers(t);

	ApplyMobAttribute(t);

	if (IsStone())
	{
		DetermineDropMetinStone();
	}

	if (IsWarp() || IsGoto()
#ifdef __DEFENSE_WAVE__
|| GetRaceNum() == 3949
#endif
)
	{
		StartWarpNPCEvent();
	}

	CHARACTER_MANAGER::instance().RegisterRaceNumMap(this);

	if (GetRaceNum() == xmas::MOB_SANTA_VNUM)
	{
		SetPoint(POINT_ATT_GRADE_BONUS, 10);
		SetPoint(POINT_DEF_GRADE_BONUS, 6);

		m_dwPlayStartTime = get_dword_time() + 30 * 1000;
		if (test_server)
			m_dwPlayStartTime = get_dword_time() + 30 * 1000;
	}

	if (warmap::IsWarFlag(GetRaceNum()))
	{
		m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlag, &CHARACTER::EndStateEmpty);
		m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlag, &CHARACTER::EndStateEmpty);
		m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlag, &CHARACTER::EndStateEmpty);
	}

	if (warmap::IsWarFlagBase(GetRaceNum()))
	{
		m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlagBase, &CHARACTER::EndStateEmpty);
		m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlagBase, &CHARACTER::EndStateEmpty);
		m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateFlagBase, &CHARACTER::EndStateEmpty);
	}

	if (m_bCharType == CHAR_TYPE_HORSE ||
			GetRaceNum() == 20101 ||
			GetRaceNum() == 20102 ||
			GetRaceNum() == 20103 ||
			GetRaceNum() == 20104 ||
			GetRaceNum() == 20105 ||
			GetRaceNum() == 20106 ||
			GetRaceNum() == 20107 ||
			GetRaceNum() == 20108 ||
			GetRaceNum() == 20109
#ifdef __MOUNT__
|| IsMountSystem()
#endif
	  )
	{
		m_stateIdle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateHorse, &CHARACTER::EndStateEmpty);
		m_stateMove.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateMove, &CHARACTER::EndStateEmpty);
		m_stateBattle.Set(this, &CHARACTER::BeginStateEmpty, &CHARACTER::StateHorse, &CHARACTER::EndStateEmpty);
	}

	if (mining::IsVeinOfOre (GetRaceNum()))
	{
		char_event_info* info = AllocEventInfo<char_event_info>();

		info->ch = this;

		m_pkMiningEvent = event_create(kill_ore_load_event, info, PASSES_PER_SEC(number(7 * 60, 15 * 60)));
	}
}

const TMobTable & CHARACTER::GetMobTable() const
{
	return m_pkMobData->m_table;
}

bool CHARACTER::IsRaceFlag(uint32_t dwBit) const
{
	return m_pkMobData ? IS_SET(m_pkMobData->m_table.dwRaceFlag, dwBit) : 0;
}

uint32_t CHARACTER::GetMobDamageMin() const
{
#ifdef __DEFENSE_WAVE__
	if (IsDefanceWaweMastAttackMob(GetRaceNum()))
		return m_pkMobData ? m_pkMobData->m_table.dwDamageRange[0] : 0;
	else
		return m_pkMobData ? m_pkMobData->m_table.dwDamageRange[0] : 0;
#else
	return m_pkMobData ? m_pkMobData->m_table.dwDamageRange[0] : 0;
#endif
}

uint32_t CHARACTER::GetMobDamageMax() const
{
#ifdef __DEFENSE_WAVE__
	if (IsDefanceWaweMastAttackMob(GetRaceNum()))
		return m_pkMobData ? m_pkMobData->m_table.dwDamageRange[1] : 0;
	else
		return m_pkMobData ? m_pkMobData->m_table.dwDamageRange[1] : 0;
#else
	return m_pkMobData ? m_pkMobData->m_table.dwDamageRange[1] : 0;
#endif
}

float CHARACTER::GetMobDamageMultiply() const
{
	float fDamMultiply = GetMobTable().fDamMultiply;

	if (IsBerserk())
		fDamMultiply = fDamMultiply * 2.0f;

	return fDamMultiply;
}

uint32_t CHARACTER::GetMobDropItemVnum() const
{
	return m_pkMobData->m_table.dwDropItemVnum;
}

bool CHARACTER::IsSummonMonster() const
{
	return GetSummonVnum() != 0;
}

uint32_t CHARACTER::GetSummonVnum() const
{
	return m_pkMobData ? m_pkMobData->m_table.dwSummonVnum : 0;
}

uint32_t CHARACTER::GetPolymorphItemVnum() const
{
	return m_pkMobData ? m_pkMobData->m_table.dwPolymorphItemVnum : 0;
}

uint32_t CHARACTER::GetMonsterDrainSPPoint() const
{
	return m_pkMobData ? m_pkMobData->m_table.dwDrainSP : 0;
}

uint8_t CHARACTER::GetMobRank() const
{
	if (!m_pkMobData)
		return MOB_RANK_KNIGHT;

	return m_pkMobData->m_table.bRank;
}

uint8_t CHARACTER::GetMobSize() const
{
	if (!m_pkMobData)
		return MOBSIZE_MEDIUM;

	return m_pkMobData->m_table.bSize;
}

uint16_t CHARACTER::GetMobAttackRange() const
{
	switch (GetMobBattleType())
	{
		case BATTLE_TYPE_RANGE:
		case BATTLE_TYPE_MAGIC:
#ifdef __DEFENSE_WAVE__
			if (GetRaceNum() == 3960 || GetRaceNum() == 3961 || GetRaceNum() == 3962)
				return m_pkMobData ? m_pkMobData->m_table.wAttackRange + GetPoint(POINT_BOW_DISTANCE) + 4000 : 0;
#endif
			return m_pkMobData->m_table.wAttackRange + GetPoint(POINT_BOW_DISTANCE);
		default:
#ifdef __DEFENSE_WAVE__
			if ((GetRaceNum() <= 3955 && GetRaceNum() >= 3950 && GetRaceNum() != 3953) || (GetRaceNum() <= 3605 && GetRaceNum() >= 3601 && GetRaceNum() != 3602) || (GetRaceNum() <= 3405 && GetRaceNum() >= 3401 && GetRaceNum() != 3403))
				return m_pkMobData ? m_pkMobData->m_table.wAttackRange + 300 : 0;
#endif
#ifdef __12ZI__
			if (GetRaceNum() >= 20452 && GetRaceNum() <= 20463)
				return m_pkMobData ? m_pkMobData->m_table.wAttackRange + GetPoint(POINT_BOW_DISTANCE) + 300 : 0;
#endif
			return m_pkMobData->m_table.wAttackRange;
	}
}

uint8_t CHARACTER::GetMobBattleType() const
{
	if (!m_pkMobData)
		return BATTLE_TYPE_MELEE;

	return (m_pkMobData->m_table.bBattleType);
}

void CHARACTER::ComputeBattlePoints()
{
	if (IsPolymorphed())
	{
		uint32_t dwMobVnum = GetPolymorphVnum();
		const CMob * pMob = CMobManager::instance().Get(dwMobVnum);
		int32_t iAtt = 0;
		int32_t iDef = 0;

		if (pMob)
		{
			iAtt = GetLevel() * 2 + GetPolymorphPoint(POINT_ST) * 2;
			iDef = GetLevel() + GetPolymorphPoint(POINT_HT) + pMob->m_table.wDef;
		}

		SetPoint(POINT_ATT_GRADE, iAtt);
		SetPoint(POINT_DEF_GRADE, iDef);
		SetPoint(POINT_MAGIC_ATT_GRADE, GetPoint(POINT_ATT_GRADE));
		SetPoint(POINT_MAGIC_DEF_GRADE, GetPoint(POINT_DEF_GRADE));
	}
	else if (IsPC())
	{
		SetPoint(POINT_ATT_GRADE, 0);
		SetPoint(POINT_DEF_GRADE, 0);
		SetPoint(POINT_CLIENT_DEF_GRADE, 0);
		SetPoint(POINT_MAGIC_ATT_GRADE, GetPoint(POINT_ATT_GRADE));
		SetPoint(POINT_MAGIC_DEF_GRADE, GetPoint(POINT_DEF_GRADE));

		int32_t iAtk = GetLevel() * 2;
		int32_t iStatAtk = 0;

		switch (GetJob())
		{
			case JOB_WARRIOR:
			case JOB_SURA:
				iStatAtk = (2 * GetPoint(POINT_ST));
				break;

			case JOB_ASSASSIN:
				iStatAtk = (4 * GetPoint(POINT_ST) + 2 * GetPoint(POINT_DX)) / 3;
				break;

			case JOB_SHAMAN:
				iStatAtk = (4 * GetPoint(POINT_ST) + 2 * GetPoint(POINT_IQ)) / 3;
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
			case JOB_WOLFMAN:
				iStatAtk = (2 * GetPoint(POINT_ST));
				break;
#endif
			default:
				sys_err("invalid job %d", GetJob());
				iStatAtk = (2 * GetPoint(POINT_ST));
				break;
		}

		if (GetMountVnum() && iStatAtk < 2 * GetPoint(POINT_ST))
			iStatAtk = (2 * GetPoint(POINT_ST));

		iAtk += iStatAtk;

		if (GetMountVnum())
		{
			if (GetJob() == JOB_SURA && GetSkillGroup() == 1)
			{
				iAtk += (iAtk * GetHorseLevel()) / 60;
			}
			else
			{
				iAtk += (iAtk * GetHorseLevel()) / 30;
			}
		}

		iAtk += GetPoint(POINT_ATT_GRADE_BONUS);

		PointChange(POINT_ATT_GRADE, iAtk);

		int32_t iShowDef = GetLevel() + GetPoint(POINT_HT);
		int32_t iDef = GetLevel() + (int32_t) (GetPoint(POINT_HT) / 1.25);
		int32_t iArmor = 0;

		LPITEM pkItem;

		for (int32_t i = 0; i < WEAR_MAX_NUM; ++i)
			if ((pkItem = GetWear(i)) && pkItem->GetType() == ITEM_ARMOR)
			{
				if (pkItem->GetSubType() == ARMOR_BODY || pkItem->GetSubType() == ARMOR_HEAD || pkItem->GetSubType() == ARMOR_FOOTS || pkItem->GetSubType() == ARMOR_SHIELD)
				{
					iArmor += pkItem->GetValue(1);
					iArmor += (2 * pkItem->GetValue(5));
				}
			}

		if( true == IsHorseRiding() )
		{
			if (iArmor < GetHorseArmor())
				iArmor = GetHorseArmor();

			const char* pHorseName = CHorseNameManager::instance().GetHorseName(GetPlayerID());

			if (pHorseName != nullptr && strlen(pHorseName))
			{
				iArmor += 20;
			}
		}

		iArmor += GetPoint(POINT_DEF_GRADE_BONUS);
		iArmor += GetPoint(POINT_PARTY_DEFENDER_BONUS);

		// @correction005
		PointChange(POINT_DEF_GRADE, iDef + iArmor);
		PointChange(POINT_CLIENT_DEF_GRADE, (iShowDef + iArmor) - GetPoint(POINT_DEF_GRADE));

		PointChange(POINT_MAGIC_ATT_GRADE, GetLevel() * 2 + GetPoint(POINT_IQ) * 2 + GetPoint(POINT_MAGIC_ATT_GRADE_BONUS));
		PointChange(POINT_MAGIC_DEF_GRADE, GetLevel() + (GetPoint(POINT_IQ) * 3 + GetPoint(POINT_HT)) / 3 + iArmor / 2 + GetPoint(POINT_MAGIC_DEF_GRADE_BONUS));
	}
	else
	{
		int32_t iAtt = GetLevel() * 2 + GetPoint(POINT_ST) * 2;
		int32_t iDef = GetLevel() + GetPoint(POINT_HT) + GetMobTable().wDef;

		SetPoint(POINT_ATT_GRADE, iAtt);
		SetPoint(POINT_DEF_GRADE, iDef);
		SetPoint(POINT_MAGIC_ATT_GRADE, GetPoint(POINT_ATT_GRADE));
		SetPoint(POINT_MAGIC_DEF_GRADE, GetPoint(POINT_DEF_GRADE));
	}
}

void CHARACTER::ComputePoints()
{
	int32_t lStat = GetPoint(POINT_STAT);
	int32_t lStatResetCount = GetPoint(POINT_STAT_RESET_COUNT);
	int32_t lSkillActive = GetPoint(POINT_SKILL);
	int32_t lSkillSub = GetPoint(POINT_SUB_SKILL);
	int32_t lSkillHorse = GetPoint(POINT_HORSE_SKILL);
	int32_t lLevelStep = GetPoint(POINT_LEVEL_STEP);

	int32_t lAttackerBonus = GetPoint(POINT_PARTY_ATTACKER_BONUS);
	int32_t lTankerBonus = GetPoint(POINT_PARTY_TANKER_BONUS);
	int32_t lBufferBonus = GetPoint(POINT_PARTY_BUFFER_BONUS);
	int32_t lSkillMasterBonus = GetPoint(POINT_PARTY_SKILL_MASTER_BONUS);
	int32_t lHasteBonus = GetPoint(POINT_PARTY_HASTE_BONUS);
	int32_t lDefenderBonus = GetPoint(POINT_PARTY_DEFENDER_BONUS);

	int32_t lHPRecovery = GetPoint(POINT_HP_RECOVERY);
	int32_t lSPRecovery = GetPoint(POINT_SP_RECOVERY);

	memset(m_pointsInstant.points, 0, sizeof(m_pointsInstant.points));
	BuffOnAttr_ClearAll();
	m_SkillDamageBonus.clear();

	SetPoint(POINT_STAT, lStat);
	SetPoint(POINT_SKILL, lSkillActive);
	SetPoint(POINT_SUB_SKILL, lSkillSub);
	SetPoint(POINT_HORSE_SKILL, lSkillHorse);
	SetPoint(POINT_LEVEL_STEP, lLevelStep);
	SetPoint(POINT_STAT_RESET_COUNT, lStatResetCount);

	SetPoint(POINT_ST, GetRealPoint(POINT_ST));
	SetPoint(POINT_HT, GetRealPoint(POINT_HT));
	SetPoint(POINT_DX, GetRealPoint(POINT_DX));
	SetPoint(POINT_IQ, GetRealPoint(POINT_IQ));

	SetPart(PART_MAIN, GetOriginalPart(PART_MAIN));
	SetPart(PART_WEAPON, GetOriginalPart(PART_WEAPON));
	SetPart(PART_HEAD, GetOriginalPart(PART_HEAD));
	SetPart(PART_HAIR, GetOriginalPart(PART_HAIR));
#ifdef __ACCE_SYSTEM__ // @correction191
	SetPart(PART_ACCE, GetOriginalPart(PART_ACCE));
#endif
#ifdef __QUIVER__ // @correction203
	SetPart(PART_ARROW_TYPE, GetOriginalPart(PART_ARROW_TYPE));
#endif
#ifdef __COSTUME_EFFECT__
	SetPart(PART_BODY_EFFECT, GetOriginalPart(PART_BODY_EFFECT));
	SetPart(PART_WEAPON_RIGHT_EFFECT, GetOriginalPart(PART_WEAPON_RIGHT_EFFECT));
	SetPart(PART_WEAPON_LEFT_EFFECT, GetOriginalPart(PART_WEAPON_LEFT_EFFECT));
#endif
	SetPoint(POINT_PARTY_ATTACKER_BONUS, lAttackerBonus);
	SetPoint(POINT_PARTY_TANKER_BONUS, lTankerBonus);
	SetPoint(POINT_PARTY_BUFFER_BONUS, lBufferBonus);
	SetPoint(POINT_PARTY_SKILL_MASTER_BONUS, lSkillMasterBonus);
	SetPoint(POINT_PARTY_HASTE_BONUS, lHasteBonus);
	SetPoint(POINT_PARTY_DEFENDER_BONUS, lDefenderBonus);

	SetPoint(POINT_HP_RECOVERY, lHPRecovery);
	SetPoint(POINT_SP_RECOVERY, lSPRecovery);

	SetPoint(POINT_PC_BANG_EXP_BONUS, 0);
	SetPoint(POINT_PC_BANG_DROP_BONUS, 0);

	int32_t iMaxHP, iMaxSP;
	int32_t iMaxStamina;

	if (IsPC())
	{
		iMaxHP = JobInitialPoints[GetJob()].max_hp + m_points.iRandomHP + GetPoint(POINT_HT) * JobInitialPoints[GetJob()].hp_per_ht;
		iMaxSP = JobInitialPoints[GetJob()].max_sp + m_points.iRandomSP + GetPoint(POINT_IQ) * JobInitialPoints[GetJob()].sp_per_iq;
		iMaxStamina = JobInitialPoints[GetJob()].max_stamina + GetPoint(POINT_HT) * JobInitialPoints[GetJob()].stamina_per_con;

		{
			CSkillProto* pkSk = CSkillManager::instance().Get(SKILL_ADD_HP);

			if (nullptr != pkSk)
			{
				pkSk->SetPointVar("k", 1.0f * GetSkillPower(SKILL_ADD_HP) / 100.0f);

				iMaxHP += static_cast<int32_t>(pkSk->kPointPoly.Eval());
			}
		}

		SetPoint(POINT_MOV_SPEED,	100);
		SetPoint(POINT_ATT_SPEED,	100);
		PointChange(POINT_ATT_SPEED, GetPoint(POINT_PARTY_HASTE_BONUS));
		SetPoint(POINT_CASTING_SPEED,	100);
	}
	else
	{
		iMaxHP = m_pkMobData ? m_pkMobData->m_table.dwMaxHP : 0;
		iMaxSP = 0;
		iMaxStamina = 0;

		SetPoint(POINT_ATT_SPEED, m_pkMobData ? m_pkMobData->m_table.sAttackSpeed : 0);
		SetPoint(POINT_MOV_SPEED, m_pkMobData ? m_pkMobData->m_table.sMovingSpeed : 0);
		SetPoint(POINT_CASTING_SPEED, m_pkMobData ? m_pkMobData->m_table.sAttackSpeed : 0);
	}

	if (IsPC())
	{
		if (GetMountVnum())
		{
			if (GetHorseST() > GetPoint(POINT_ST))
				PointChange(POINT_ST, GetHorseST() - GetPoint(POINT_ST));

			if (GetHorseDX() > GetPoint(POINT_DX))
				PointChange(POINT_DX, GetHorseDX() - GetPoint(POINT_DX));

			if (GetHorseHT() > GetPoint(POINT_HT))
				PointChange(POINT_HT, GetHorseHT() - GetPoint(POINT_HT));

			if (GetHorseIQ() > GetPoint(POINT_IQ))
				PointChange(POINT_IQ, GetHorseIQ() - GetPoint(POINT_IQ));
		}

	}

	ComputeBattlePoints();

	if (iMaxHP != GetMaxHP())
	{
		SetRealPoint(POINT_MAX_HP, iMaxHP);
	}

	PointChange(POINT_MAX_HP, 0);

	if (iMaxSP != GetMaxSP())
	{
		SetRealPoint(POINT_MAX_SP, iMaxSP);
	}

	PointChange(POINT_MAX_SP, 0);

	SetMaxStamina(iMaxStamina);
	// @fixme118 part1
	int32_t iCurHP = this->GetHP();
	int32_t iCurSP = this->GetSP();

	m_pointsInstant.dwImmuneFlag = 0;

	for (int32_t i = 0 ; i < WEAR_MAX_NUM; i++)
	{
		LPITEM pItem = GetWear(i);
		if (pItem)
		{
			pItem->ModifyPoints(true);
			SET_BIT(m_pointsInstant.dwImmuneFlag, GetWear(i)->GetImmuneFlag());
		}
	}

	if (DragonSoul_IsDeckActivated())
	{
		for (int32_t i = WEAR_MAX_NUM + DS_SLOT_MAX * DragonSoul_GetActiveDeck();
			i < WEAR_MAX_NUM + DS_SLOT_MAX * (DragonSoul_GetActiveDeck() + 1); i++)
		{
			LPITEM pItem = GetWear(i);
			if (pItem)
			{
				if (DSManager::instance().IsTimeLeftDragonSoul(pItem))
					pItem->ModifyPoints(true);
			}
		}
	}

	if (GetHP() > GetMaxHP())
		PointChange(POINT_HP, GetMaxHP() - GetHP());

	if (GetSP() > GetMaxSP())
		PointChange(POINT_SP, GetMaxSP() - GetSP());

	ComputeSkillPoints();

	RefreshAffect();

	CPetSystem* pPetSystem = GetPetSystem();
	if (pPetSystem)
		pPetSystem->RefreshBuff();

	// @fixme118 part2 (after petsystem stuff)
	if (IsPC())
	{
		if (this->GetHP() != iCurHP)
			this->PointChange(POINT_HP, iCurHP-this->GetHP());
		if (this->GetSP() != iCurSP)
			this->PointChange(POINT_SP, iCurSP-this->GetSP());
	}

	UpdatePacket();
}

void CHARACTER::ResetPlayTime(uint32_t dwTimeRemain)
{
	m_dwPlayStartTime = get_dword_time() - dwTimeRemain;
}

const int32_t aiRecoveryPercents[10] = { 1, 5, 5, 5, 5, 5, 5, 5, 5, 5 };

EVENTFUNC(recovery_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );
	if ( info == nullptr )
	{
		sys_err( "recovery_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;

	if (ch == nullptr) {
		return 0;
	}

	if (!ch->IsPC())
	{
		if (ch->IsAffectFlag(AFF_POISON))
			return PASSES_PER_SEC(MAX(1, ch->GetMobTable().bRegenCycle));
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		if (ch->IsAffectFlag(AFF_BLEEDING))
			return PASSES_PER_SEC(MAX(1, ch->GetMobTable().bRegenCycle));
#endif
		if (2493 == ch->GetMobTable().dwVnum)
		{
			int32_t regenPct = BlueDragon_GetRangeFactor("hp_regen", ch->GetHPPct());
			regenPct += ch->GetMobTable().bRegenPercent;

			for (int32_t i=1 ; i <= 4 ; ++i)
			{
				if (REGEN_PECT_BONUS == BlueDragon_GetIndexFactor("DragonStone", i, "effect_type"))
				{
					uint32_t dwDragonStoneID = BlueDragon_GetIndexFactor("DragonStone", i, "vnum");
					size_t val = BlueDragon_GetIndexFactor("DragonStone", i, "val");
					size_t cnt = SECTREE_MANAGER::instance().GetMonsterCountInMap( ch->GetMapIndex(), dwDragonStoneID );

					regenPct += (val*cnt);

					break;
				}
			}

			ch->PointChange(POINT_HP, MAX(1, (ch->GetMaxHP() * regenPct) / 100));
		}
		else if (!ch->IsDoor())
		{
			ch->MonsterLog("HP_REGEN +%d", MAX(1, (ch->GetMaxHP() * ch->GetMobTable().bRegenPercent) / 100));
			ch->PointChange(POINT_HP, MAX(1, (ch->GetMaxHP() * ch->GetMobTable().bRegenPercent) / 100));
		}

		if (ch->GetHP() >= ch->GetMaxHP())
		{
			ch->m_pkRecoveryEvent = nullptr;
			return 0;
		}

		if (2493 == ch->GetMobTable().dwVnum)
		{
			for (int32_t i=1 ; i <= 4 ; ++i)
			{
				if (REGEN_TIME_BONUS == BlueDragon_GetIndexFactor("DragonStone", i, "effect_type"))
				{
					uint32_t dwDragonStoneID = BlueDragon_GetIndexFactor("DragonStone", i, "vnum");
					size_t val = BlueDragon_GetIndexFactor("DragonStone", i, "val");
					size_t cnt = SECTREE_MANAGER::instance().GetMonsterCountInMap( ch->GetMapIndex(), dwDragonStoneID );

					return PASSES_PER_SEC(MAX(1, (ch->GetMobTable().bRegenCycle - (val*cnt))));
				}
			}
		}

		return PASSES_PER_SEC(MAX(1, ch->GetMobTable().bRegenCycle));
	}
	else
	{
		ch->CheckTarget();
		ch->UpdateKillerMode();

		if (ch->IsAffectFlag(AFF_POISON) == true)
		{
			return 3;
		}
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		if (ch->IsAffectFlag(AFF_BLEEDING))
			return 3;
#endif
		int32_t iSec = (get_dword_time() - ch->GetLastMoveTime()) / 3000;

		ch->DistributeSP(ch);

		if (ch->GetMaxHP() <= ch->GetHP())
			return PASSES_PER_SEC(3);

		int32_t iPercent = 0;
		int32_t iAmount = 0;

		{
			iPercent = aiRecoveryPercents[MIN(9, iSec)];
			iAmount = 15 + (ch->GetMaxHP() * iPercent) / 100;
		}

		iAmount += (iAmount * ch->GetPoint(POINT_HP_REGEN)) / 100;

		sys_log(1, "RECOVERY_EVENT: %s %d HP_REGEN %d HP +%d", ch->GetName(), iPercent, ch->GetPoint(POINT_HP_REGEN), iAmount);

		ch->PointChange(POINT_HP, iAmount, false);
		return PASSES_PER_SEC(3);
	}
}

void CHARACTER::StartRecoveryEvent()
{
	if (m_pkRecoveryEvent)
		return;

	if (IsDead() || IsStun())
		return;

	if (IsNPC() && GetHP() >= GetMaxHP())
		return;

#ifdef __GUILD_DRAGONLAIR__
	if ((CMgr::instance().IsMeleyMap(GetMapIndex())) && ((GetRaceNum() == (uint16_t)(STATUE_VNUM)) || ((GetRaceNum() == (uint16_t)(BOSS_VNUM)))))
		return;
#endif

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	int32_t iSec = IsPC() ? 3 : (MAX(1, GetMobTable().bRegenCycle));
	m_pkRecoveryEvent = event_create(recovery_event, info, PASSES_PER_SEC(iSec));
}

void CHARACTER::Standup()
{
	struct packet_position pack_position;

	if (!IsPosition(POS_SITTING))
		return;

	SetPosition(POS_STANDING);

	sys_log(1, "STANDUP: %s", GetName());

	pack_position.header	= HEADER_GC_CHARACTER_POSITION;
	pack_position.vid		= GetVID();
	pack_position.position	= POSITION_GENERAL;

	PacketAround(&pack_position, sizeof(pack_position));
}

void CHARACTER::Sitdown(int32_t is_ground)
{
	struct packet_position pack_position;

	if (IsPosition(POS_SITTING))
		return;

	SetPosition(POS_SITTING);
	sys_log(1, "SITDOWN: %s", GetName());

	pack_position.header	= HEADER_GC_CHARACTER_POSITION;
	pack_position.vid		= GetVID();
	pack_position.position	= POSITION_SITTING_GROUND;
	PacketAround(&pack_position, sizeof(pack_position));
}

void CHARACTER::SetRotation(float fRot)
{
	m_pointsInstant.fRot = fRot;
}

void CHARACTER::SetRotationToXY(int32_t x, int32_t y)
{
	SetRotation(GetDegreeFromPositionXY(GetX(), GetY(), x, y));
}

bool CHARACTER::CannotMoveByAffect() const
{
	return (IsAffectFlag(AFF_STUN));
}

bool CHARACTER::CanMove() const
{
	if (CannotMoveByAffect())
		return false;

	if (GetMyShop())
		return false;

	return true;
}

bool CHARACTER::Sync(int32_t x, int32_t y)
{
	if (!GetSectree())
		return false;

	LPSECTREE new_tree = SECTREE_MANAGER::instance().Get(GetMapIndex(), x, y);

	if (!new_tree)
	{
		if (GetDesc())
		{
			sys_err("cannot find tree at %d %d (name: %s)", x, y, GetName());
			GetDesc()->SetPhase(PHASE_CLOSE);
		}
		else
		{
			sys_err("no tree: %s %d %d %d", GetName(), x, y, GetMapIndex());
			Dead();
		}

		return false;
	}

	SetRotationToXY(x, y);
	SetXYZ(x, y, 0);

	if (GetDungeon())
	{
		int32_t iLastEventAttr = m_iEventAttr;
		m_iEventAttr = new_tree->GetEventAttribute(x, y);

		if (m_iEventAttr != iLastEventAttr)
		{
			if (GetParty())
			{
				quest::CQuestManager::instance().AttrOut(GetParty()->GetLeaderPID(), this, iLastEventAttr);
				quest::CQuestManager::instance().AttrIn(GetParty()->GetLeaderPID(), this, m_iEventAttr);
			}
			else
			{
				quest::CQuestManager::instance().AttrOut(GetPlayerID(), this, iLastEventAttr);
				quest::CQuestManager::instance().AttrIn(GetPlayerID(), this, m_iEventAttr);
			}
		}
	}

	if (GetSectree() != new_tree)
	{
		if (!IsNPC())
		{
			SECTREEID id = new_tree->GetID();
			SECTREEID old_id = GetSectree()->GetID();
			// @correction121 BEGIN fDist added
			const float fDist = DISTANCE_SQRT(id.coord.x - old_id.coord.x, id.coord.y - old_id.coord.y);
			sys_log(0, "SECTREE DIFFER: %s %dx%d was %dx%d dist %.1fm", GetName(), id.coord.x, id.coord.y, old_id.coord.x, old_id.coord.y, fDist);
			// @correction121 END
		}

		new_tree->InsertEntity(this);
	}

	return true;
}

void CHARACTER::Stop()
{
	if (!IsState(m_stateIdle))
		MonsterLog("[IDLE] 정지");

	GotoState(m_stateIdle);

	m_posDest.x = m_posStart.x = GetX();
	m_posDest.y = m_posStart.y = GetY();
}

bool CHARACTER::Goto(int32_t x, int32_t y)
{
	if (GetX() == x && GetY() == y)
		return false;

	if (m_posDest.x == x && m_posDest.y == y)
	{
		if (!IsState(m_stateMove))
		{
			m_dwStateDuration = 4;
			GotoState(m_stateMove);
		}
		return false;
	}

	m_posDest.x = x;
	m_posDest.y = y;

	CalculateMoveDuration();

	m_dwStateDuration = 4;


	if (!IsState(m_stateMove))
	{
		MonsterLog("[MOVE] %s", GetVictim() ? "대상추적" : "그냥이동");

		if (GetVictim())
		{
			MonsterChat(MONSTER_CHAT_ATTACK);
		}
	}

	GotoState(m_stateMove);

	return true;
}


uint32_t CHARACTER::GetMotionMode() const
{
	uint32_t dwMode = MOTION_MODE_GENERAL;

	if (IsPolymorphed())
		return dwMode;

	LPITEM pkItem;

	if ((pkItem = GetWear(WEAR_WEAPON)))
	{
		switch (pkItem->GetProto()->bSubType)
		{
			case WEAPON_SWORD:
				dwMode = MOTION_MODE_ONEHAND_SWORD;
				break;

			case WEAPON_TWO_HANDED:
				dwMode = MOTION_MODE_TWOHAND_SWORD;
				break;

			case WEAPON_DAGGER:
				dwMode = MOTION_MODE_DUALHAND_SWORD;
				break;

			case WEAPON_BOW:
				dwMode = MOTION_MODE_BOW;
				break;

			case WEAPON_BELL:
				dwMode = MOTION_MODE_BELL;
				break;

			case WEAPON_FAN:
				dwMode = MOTION_MODE_FAN;
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
			case WEAPON_CLAW:
				dwMode = MOTION_MODE_CLAW;
				break;
#endif
		}
	}
	return dwMode;
}

float CHARACTER::GetMoveMotionSpeed() const
{
	uint32_t dwMode = GetMotionMode();

	const CMotion * pkMotion = nullptr;

	if (!GetMountVnum())
		pkMotion = CMotionManager::instance().GetMotion(GetRaceNum(), MAKE_MOTION_KEY(dwMode, (IsWalking() && IsPC()) ? MOTION_WALK : MOTION_RUN));
	else
	{
		pkMotion = CMotionManager::instance().GetMotion(GetMountVnum(), MAKE_MOTION_KEY(MOTION_MODE_GENERAL, (IsWalking() && IsPC()) ? MOTION_WALK : MOTION_RUN));

		if (!pkMotion)
			pkMotion = CMotionManager::instance().GetMotion(GetRaceNum(), MAKE_MOTION_KEY(MOTION_MODE_HORSE, (IsWalking() && IsPC()) ? MOTION_WALK : MOTION_RUN));
	}

	if (pkMotion)
		return -pkMotion->GetAccumVector().y / pkMotion->GetDuration();
	else
	{
		// sys_err("cannot find motion (name %s race %d mode %d)", GetName(), GetRaceNum(), dwMode);
		return 300.0f;
	}
}

float CHARACTER::GetMoveSpeed() const
{
	return GetMoveMotionSpeed() * 10000 / CalculateDuration(GetLimitPoint(POINT_MOV_SPEED), 10000);
}

void CHARACTER::CalculateMoveDuration()
{
	m_posStart.x = GetX();
	m_posStart.y = GetY();

	float fDist = DISTANCE_SQRT(m_posStart.x - m_posDest.x, m_posStart.y - m_posDest.y);

	float motionSpeed = GetMoveMotionSpeed();

	m_dwMoveDuration = CalculateDuration(GetLimitPoint(POINT_MOV_SPEED),
			(int32_t) ((fDist / motionSpeed) * 1000.0f));

	if (IsNPC())
		sys_log(1, "%s: GOTO: distance %f, spd %u, duration %u, motion speed %f pos %d %d -> %d %d",
				GetName(), fDist, GetLimitPoint(POINT_MOV_SPEED), m_dwMoveDuration, motionSpeed,
				m_posStart.x, m_posStart.y, m_posDest.x, m_posDest.y);

	m_dwMoveStartTime = get_dword_time();
}

bool CHARACTER::Move(int32_t x, int32_t y)
{
	if (GetX() == x && GetY() == y)
		return true;

	if (test_server)
		if (m_bDetailLog)
			sys_log(0, "%s position %u %u", GetName(), x, y);

	OnMove();
	return Sync(x, y);
}

void CHARACTER::SendMovePacket(uint8_t bFunc, uint8_t bArg, uint32_t x, uint32_t y, uint32_t dwDuration, uint32_t dwTime, int32_t iRot)
{
	TPacketGCMove pack;

	if (bFunc == FUNC_WAIT)
	{
		x = m_posDest.x;
		y = m_posDest.y;
		dwDuration = m_dwMoveDuration;
	}

	EncodeMovePacket(pack, GetVID(), bFunc, bArg, x, y, dwDuration, dwTime, iRot == -1 ? (int32_t) GetRotation() / 5 : iRot);
	PacketView(&pack, sizeof(TPacketGCMove), this);
}

int32_t CHARACTER::GetRealPoint(uint8_t type) const
{
	return m_points.points[type];
}

void CHARACTER::SetRealPoint(uint8_t type, int32_t val)
{
	m_points.points[type] = val;
}

int32_t CHARACTER::GetPolymorphPoint(uint8_t type) const
{
	if (IsPolymorphed() && !IsPolyMaintainStat())
	{
		uint32_t dwMobVnum = GetPolymorphVnum();
		const CMob * pMob = CMobManager::instance().Get(dwMobVnum);
		int32_t iPower = GetPolymorphPower();

		if (pMob)
		{
			switch (type)
			{
				case POINT_ST:
					if ((GetJob() == JOB_SHAMAN) || ((GetJob() == JOB_SURA) && (GetSkillGroup() == 2)))
						return pMob->m_table.bStr * iPower / 100 + GetPoint(POINT_IQ);
					return pMob->m_table.bStr * iPower / 100 + GetPoint(POINT_ST);

				case POINT_HT:
					return pMob->m_table.bCon * iPower / 100 + GetPoint(POINT_HT);

				case POINT_IQ:
					return pMob->m_table.bInt * iPower / 100 + GetPoint(POINT_IQ);

				case POINT_DX:
					return pMob->m_table.bDex * iPower / 100 + GetPoint(POINT_DX);
			}
		}
	}

	return GetPoint(type);
}

int32_t CHARACTER::GetPoint(uint8_t type) const
{
	if (type >= POINT_MAX_NUM)
	{
		sys_err("Point type overflow (type %u)", type);
		return 0;
	}

	int32_t val = m_pointsInstant.points[type];
	int32_t max_val = INT_MAX;

	switch (type)
	{
		case POINT_STEAL_HP:
		case POINT_STEAL_SP:
			max_val = 50;
			break;
	}

	if (val > max_val)
		sys_err("POINT_ERROR: %s type %d val %d (max: %d)", GetName(), val, max_val);

	return (val);
}

int32_t CHARACTER::GetLimitPoint(uint8_t type) const
{
	if (type >= POINT_MAX_NUM)
	{
		sys_err("Point type overflow (type %u)", type);
		return 0;
	}

	int32_t val = m_pointsInstant.points[type];
	int32_t max_val = INT_MAX;
	int32_t limit = INT_MAX;
	int32_t min_limit = -INT_MAX;

	switch (type)
	{
		case POINT_ATT_SPEED:
			min_limit = 0;

			if (IsPC())
				limit = 170;
			else
				limit = 250;
			break;

		case POINT_MOV_SPEED:
			min_limit = 0;

			if (IsPC())
				limit = 200;
			else
				limit = 250;
			break;

		case POINT_STEAL_HP:
		case POINT_STEAL_SP:
			limit = 50;
			max_val = 50;
			break;

		case POINT_MALL_ATTBONUS:
		case POINT_MALL_DEFBONUS:
			limit = 20;
			max_val = 50;
			break;
	}

	if (val > max_val)
		sys_err("POINT_ERROR: %s type %d val %d (max: %d)", GetName(), val, max_val);

	if (val > limit)
		val = limit;

	if (val < min_limit)
		val = min_limit;

	return (val);
}

void CHARACTER::SetPoint(uint8_t type, int32_t val)
{
	if (type >= POINT_MAX_NUM)
	{
		sys_err("Point type overflow (type %u)", type);
		return;
	}

	m_pointsInstant.points[type] = val;

	if (type == POINT_MOV_SPEED && get_dword_time() < m_dwMoveStartTime + m_dwMoveDuration)
	{
		CalculateMoveDuration();
	}
}

uint64_t CHARACTER::GetAllowedGold() const
{
	if (GetLevel() <= 10)
		return 100000;
	else if (GetLevel() <= 20)
		return 500000;
	else
		return 50000000;
}

void CHARACTER::CheckMaximumPoints()
{
	if (GetMaxHP() < GetHP())
		PointChange(POINT_HP, GetMaxHP() - GetHP());

	if (GetMaxSP() < GetSP())
		PointChange(POINT_SP, GetMaxSP() - GetSP());
}

void CHARACTER::PointChange(uint8_t type, int32_t amount, bool bAmount, bool bBroadcast)
{
	int32_t val = 0;


	switch (type)
	{
		case POINT_NONE:
			return;

		case POINT_LEVEL:
			if ((GetLevel() + amount) > gPlayerMaxLevel)
				return;

			SetLevel(GetLevel() + amount);
			val = GetLevel();

			sys_log(0, "LEVELUP: %s %d NEXT EXP %d", GetName(), GetLevel(), GetNextExp());
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
			if (GetJob() == JOB_WOLFMAN)
			{
				if ((5 <= val) && (GetSkillGroup()!=1))
				{
					ClearSkill();
					SetSkillGroup(1);
					SetRealPoint(POINT_SKILL, GetLevel()-1);
					SetPoint(POINT_SKILL, GetRealPoint(POINT_SKILL));
					PointChange(POINT_SKILL, 0);
				}
			}
#endif
			PointChange(POINT_NEXT_EXP,	GetNextExp(), false);

			if (amount)
			{
#ifdef __AFAR_BIOLOG__
				int32_t biodurum = GetQuestFlag("bio.durum");
				if (biodurum == 0 && GetLevel() >= 30)
				{
					SetQuestFlag("bio.durum", 1);
					SetQuestFlag("bio.verilen", 0);
					SetQuestFlag("bio.ruhtasi", 0);
					SetQuestFlag("bio.kalan", get_global_time());
					int32_t bioverilen = GetQuestFlag("bio.verilen");
					int32_t biokalan = GetQuestFlag("bio.kalan");
					biodurum = GetQuestFlag("bio.durum");
					ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);

				}
				else if (biodurum == 31 && GetLevel() >= 40)
				{
					SetQuestFlag("bio.durum", 2);
					SetQuestFlag("bio.verilen", 0);
					SetQuestFlag("bio.ruhtasi", 0);
					SetQuestFlag("bio.kalan", get_global_time());
					int32_t bioverilen = GetQuestFlag("bio.verilen");
					int32_t biokalan = GetQuestFlag("bio.kalan");
					biodurum = GetQuestFlag("bio.durum");
					ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				else if (biodurum == 41 && GetLevel() >= 50)
				{
					SetQuestFlag("bio.durum", 3);
					SetQuestFlag("bio.verilen", 0);
					SetQuestFlag("bio.ruhtasi", 0);
					SetQuestFlag("bio.kalan", get_global_time());
					int32_t bioverilen = GetQuestFlag("bio.verilen");
					int32_t biokalan = GetQuestFlag("bio.kalan");
					biodurum = GetQuestFlag("bio.durum");
					ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				else if (biodurum == 51 && GetLevel() >= 60)
				{
					SetQuestFlag("bio.durum", 4);
					SetQuestFlag("bio.verilen", 0);
					SetQuestFlag("bio.ruhtasi", 0);
					SetQuestFlag("bio.kalan", get_global_time());
					int32_t bioverilen = GetQuestFlag("bio.verilen");
					int32_t biokalan = GetQuestFlag("bio.kalan");
					biodurum = GetQuestFlag("bio.durum");
					ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				else if (biodurum == 61 && GetLevel() >= 70)
				{
					SetQuestFlag("bio.durum", 5);
					SetQuestFlag("bio.verilen", 0);
					SetQuestFlag("bio.ruhtasi", 0);
					SetQuestFlag("bio.kalan", get_global_time());
					int32_t bioverilen = GetQuestFlag("bio.verilen");
					int32_t biokalan = GetQuestFlag("bio.kalan");
					biodurum = GetQuestFlag("bio.durum");
					ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				else if (biodurum == 71 && GetLevel() >= 80)
				{
					SetQuestFlag("bio.durum", 6);
					SetQuestFlag("bio.verilen", 0);
					SetQuestFlag("bio.ruhtasi", 0);
					SetQuestFlag("bio.kalan", get_global_time());
					int32_t bioverilen = GetQuestFlag("bio.verilen");
					int32_t biokalan = GetQuestFlag("bio.kalan");
					biodurum = GetQuestFlag("bio.durum");
					ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				else if (biodurum == 81 && GetLevel() >= 85)
				{
					SetQuestFlag("bio.durum", 7);
					SetQuestFlag("bio.verilen", 0);
					SetQuestFlag("bio.ruhtasi", 0);
					SetQuestFlag("bio.kalan", get_global_time());
					int32_t bioverilen = GetQuestFlag("bio.verilen");
					int32_t biokalan = GetQuestFlag("bio.kalan");
					biodurum = GetQuestFlag("bio.durum");
					ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				else if (biodurum == 86 && GetLevel() >= 90)
				{
					SetQuestFlag("bio.durum", 8);
					SetQuestFlag("bio.verilen", 0);
					SetQuestFlag("bio.ruhtasi", 0);
					SetQuestFlag("bio.kalan", get_global_time());
					int32_t bioverilen = GetQuestFlag("bio.verilen");
					int32_t biokalan = GetQuestFlag("bio.kalan");
					biodurum = GetQuestFlag("bio.durum");
					ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				else if (biodurum == 91 && GetLevel() >= 92)
				{
					SetQuestFlag("bio.durum", 9);
					SetQuestFlag("bio.verilen", 0);
					SetQuestFlag("bio.ruhtasi", 0);
					SetQuestFlag("bio.kalan", get_global_time());
					int32_t bioverilen = GetQuestFlag("bio.verilen");
					int32_t biokalan = GetQuestFlag("bio.kalan");
					biodurum = GetQuestFlag("bio.durum");
					ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
				else if (biodurum == 93 && GetLevel() >= 94)
				{
					SetQuestFlag("bio.durum", 10);
					SetQuestFlag("bio.verilen", 0);
					SetQuestFlag("bio.ruhtasi", 0);
					SetQuestFlag("bio.kalan", get_global_time());
					int32_t bioverilen = GetQuestFlag("bio.verilen");
					int32_t biokalan = GetQuestFlag("bio.kalan");
					biodurum = GetQuestFlag("bio.durum");
					ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[biodurum][0], bioverilen, BiyologSistemi[biodurum][1], biokalan);
				}
#endif
				quest::CQuestManager::instance().LevelUp(GetPlayerID());

				LogManager::instance().LevelLog(this, val, GetRealPoint(POINT_PLAYTIME) + (get_dword_time() - m_dwPlayStartTime) / 60000);

				if (GetGuild())
				{
					GetGuild()->LevelChange(GetPlayerID(), GetLevel());
				}

				if (GetParty())
				{
					GetParty()->RequestSetMemberLevel(GetPlayerID(), GetLevel());
				}
			}
			break;

		case POINT_NEXT_EXP:
			val = GetNextExp();
			bAmount = false;
			break;

		case POINT_EXP:
			{
#ifdef __BLOCK_EXP__
				if (GetQuestFlag("sec.exp"))
					return;
#endif
				uint32_t exp = GetExp();
				uint32_t next_exp = GetNextExp();

				// @correction005
				if (g_bChinaIntoxicationCheck) // @correction215
				{
					if (IsOverTime(OT_NONE))
					{
						dev_log(LOG_DEB0, "<EXP_LOG> %s = NONE", GetName());
					}
					else if (IsOverTime(OT_3HOUR))
					{
						amount = (amount / 2);
						dev_log(LOG_DEB0, "<EXP_LOG> %s = 3HOUR", GetName());
					}
					else if (IsOverTime(OT_5HOUR))
					{
						amount = 0;
						dev_log(LOG_DEB0, "<EXP_LOG> %s = 5HOUR", GetName());
					}
				}

				if ((amount < 0) && (exp < (uint32_t)(-amount)))
				{
					sys_log(1, "%s AMOUNT < 0 %d, CUR EXP: %d", GetName(), -amount, exp);
					amount = -exp;

					SetExp(exp + amount);
					val = GetExp();
				}
				else
				{
					if (gPlayerMaxLevel <= GetLevel())
						return;

					if (test_server)
						ChatPacket(CHAT_TYPE_INFO, "You have gained %d exp.", amount);

					uint32_t iExpBalance = 0;

					if (exp + amount >= next_exp)
					{
						iExpBalance = (exp + amount) - next_exp;
						amount = next_exp - exp;

						SetExp(0);
						exp = next_exp;
					}
					else
					{
						SetExp(exp + amount);
						exp = GetExp();
					}

					uint32_t q = uint32_t(next_exp / 4.0f);
					int32_t iLevStep = GetRealPoint(POINT_LEVEL_STEP);

					if (iLevStep >= 4)
					{
						sys_err("%s LEVEL_STEP bigger than 4! (%d)", GetName(), iLevStep);
						iLevStep = 4;
					}

					if (exp >= next_exp && iLevStep < 4)
					{
						for (int32_t i = 0; i < 4 - iLevStep; ++i)
							PointChange(POINT_LEVEL_STEP, 1, false, true);
					}
					else if (exp >= q * 3 && iLevStep < 3)
					{
						for (int32_t i = 0; i < 3 - iLevStep; ++i)
							PointChange(POINT_LEVEL_STEP, 1, false, true);
					}
					else if (exp >= q * 2 && iLevStep < 2)
					{
						for (int32_t i = 0; i < 2 - iLevStep; ++i)
							PointChange(POINT_LEVEL_STEP, 1, false, true);
					}
					else if (exp >= q && iLevStep < 1)
						PointChange(POINT_LEVEL_STEP, 1);

					if (iExpBalance)
					{
						PointChange(POINT_EXP, iExpBalance);
					}

					val = GetExp();
				}
			}
			break;

		case POINT_LEVEL_STEP:
			if (amount > 0)
			{
				val = GetPoint(POINT_LEVEL_STEP) + amount;

				switch (val)
				{
					case 1:
					case 2:
					case 3:
						if ((GetLevel() <= g_iStatusPointGetLevelLimit) && // @correction217
							(GetLevel() <= gPlayerMaxLevel) ) // @correction031
							PointChange(POINT_STAT, 1);
						break;

					case 4:
						{
							int32_t iHP = number(JobInitialPoints[GetJob()].hp_per_lv_begin, JobInitialPoints[GetJob()].hp_per_lv_end);
							int32_t iSP = number(JobInitialPoints[GetJob()].sp_per_lv_begin, JobInitialPoints[GetJob()].sp_per_lv_end);

							m_points.iRandomHP += iHP;
							m_points.iRandomSP += iSP;

							if (GetSkillGroup())
							{
								if (GetLevel() >= 5)
									PointChange(POINT_SKILL, 1);

								if (GetLevel() >= 9)
									PointChange(POINT_SUB_SKILL, 1);
							}

							PointChange(POINT_MAX_HP, iHP);
							PointChange(POINT_MAX_SP, iSP);
							PointChange(POINT_LEVEL, 1, false, true);

							val = 0;
						}
						break;
				}

				PointChange(POINT_HP, GetMaxHP() - GetHP());
				PointChange(POINT_SP, GetMaxSP() - GetSP());
				PointChange(POINT_STAMINA, GetMaxStamina() - GetStamina());

				SetPoint(POINT_LEVEL_STEP, val);
				SetRealPoint(POINT_LEVEL_STEP, val);

				Save();
			}
			else
				val = GetPoint(POINT_LEVEL_STEP);

			break;

		case POINT_HP:
			{
				if (IsDead() || IsStun())
					return;

				int32_t prev_hp = GetHP();

				amount = MIN(GetMaxHP() - GetHP(), amount);
				SetHP(GetHP() + amount);
				val = GetHP();

				BroadcastTargetPacket();

				if (GetParty() && IsPC() && val != prev_hp)
					GetParty()->SendPartyInfoOneToAll(this);
			}
			break;

		case POINT_SP:
			{
				if (IsDead() || IsStun())
					return;

				amount = MIN(GetMaxSP() - GetSP(), amount);
				SetSP(GetSP() + amount);
				val = GetSP();
			}
			break;

		case POINT_STAMINA:
			{
				if (IsDead() || IsStun())
					return;

				int32_t prev_val = GetStamina();
				amount = MIN(GetMaxStamina() - GetStamina(), amount);
				SetStamina(GetStamina() + amount);
				val = GetStamina();

				if (val == 0)
				{
					SetNowWalking(true);
				}
				else if (prev_val == 0)
				{
					ResetWalking();
				}

				if (amount < 0 && val != 0)
					return;
			}
			break;

		case POINT_MAX_HP:
			{
				SetPoint(type, GetPoint(type) + amount);

				int32_t hp = GetRealPoint(POINT_MAX_HP);
				int32_t add_hp = MIN(3500, hp * GetPoint(POINT_MAX_HP_PCT) / 100);
				add_hp += GetPoint(POINT_MAX_HP);
				add_hp += GetPoint(POINT_PARTY_TANKER_BONUS);

				SetMaxHP(hp + add_hp);

				val = GetMaxHP();
			}
			break;

		case POINT_MAX_SP:
			{
				SetPoint(type, GetPoint(type) + amount);

				int32_t sp = GetRealPoint(POINT_MAX_SP);
				int32_t add_sp = MIN(800, sp * GetPoint(POINT_MAX_SP_PCT) / 100);
				add_sp += GetPoint(POINT_MAX_SP);
				add_sp += GetPoint(POINT_PARTY_SKILL_MASTER_BONUS);

				SetMaxSP(sp + add_sp);

				val = GetMaxSP();
			}
			break;

		case POINT_MAX_HP_PCT:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			PointChange(POINT_MAX_HP, 0);
			break;

		case POINT_MAX_SP_PCT:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			PointChange(POINT_MAX_SP, 0);
			break;

		case POINT_MAX_STAMINA:
			SetMaxStamina(GetMaxStamina() + amount);
			val = GetMaxStamina();
			break;

		case POINT_SKILL:
		case POINT_STAT:
		case POINT_SUB_SKILL:
		case POINT_STAT_RESET_COUNT:
		case POINT_HORSE_SKILL:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			SetRealPoint(type, val);
			break;

		case POINT_DEF_GRADE:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);

			PointChange(POINT_CLIENT_DEF_GRADE, amount);
			break;

		case POINT_CLIENT_DEF_GRADE:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;

		case POINT_ST:
		case POINT_HT:
		case POINT_DX:
		case POINT_IQ:
		case POINT_HP_REGEN:
		case POINT_SP_REGEN:
		case POINT_ATT_SPEED:
		case POINT_ATT_GRADE:
		case POINT_MOV_SPEED:
		case POINT_CASTING_SPEED:
		case POINT_MAGIC_ATT_GRADE:
		case POINT_MAGIC_DEF_GRADE:
		case POINT_BOW_DISTANCE:
		case POINT_HP_RECOVERY:
		case POINT_SP_RECOVERY:

		case POINT_ATTBONUS_HUMAN:
		case POINT_ATTBONUS_ANIMAL:
		case POINT_ATTBONUS_ORC:
		case POINT_ATTBONUS_MILGYO:
		case POINT_ATTBONUS_UNDEAD:
		case POINT_ATTBONUS_DEVIL:

		case POINT_ATTBONUS_MONSTER:
		case POINT_ATTBONUS_SURA:
		case POINT_ATTBONUS_ASSASSIN:
		case POINT_ATTBONUS_WARRIOR:
		case POINT_ATTBONUS_SHAMAN:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case POINT_ATTBONUS_WOLFMAN:
#endif

		case POINT_POISON_PCT:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case POINT_BLEEDING_PCT:
#endif
		case POINT_STUN_PCT:
		case POINT_SLOW_PCT:

		case POINT_BLOCK:
		case POINT_DODGE:

		case POINT_CRITICAL_PCT:
		case POINT_RESIST_CRITICAL:
		case POINT_PENETRATE_PCT:
		case POINT_RESIST_PENETRATE:
		case POINT_CURSE_PCT:

		case POINT_STEAL_HP:
		case POINT_STEAL_SP:

		case POINT_MANA_BURN_PCT:
		case POINT_DAMAGE_SP_RECOVER:
		case POINT_RESIST_NORMAL_DAMAGE:
		case POINT_RESIST_SWORD:
		case POINT_RESIST_TWOHAND:
		case POINT_RESIST_DAGGER:
		case POINT_RESIST_BELL:
		case POINT_RESIST_FAN:
		case POINT_RESIST_BOW:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case POINT_RESIST_CLAW:
#endif
		case POINT_RESIST_FIRE:
		case POINT_RESIST_ELEC:
		case POINT_RESIST_MAGIC:
		case POINT_RESIST_WIND:
		case POINT_RESIST_ICE:
		case POINT_RESIST_EARTH:
		case POINT_RESIST_DARK:
		case POINT_REFLECT_MELEE:
		case POINT_REFLECT_CURSE:
		case POINT_POISON_REDUCE:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case POINT_BLEEDING_REDUCE:
#endif
		case POINT_KILL_SP_RECOVER:
		case POINT_KILL_HP_RECOVERY:
		case POINT_HIT_HP_RECOVERY:
		case POINT_HIT_SP_RECOVERY:
		case POINT_MANASHIELD:
		case POINT_ATT_BONUS:
		case POINT_DEF_BONUS:
		case POINT_SKILL_DAMAGE_BONUS:
		case POINT_NORMAL_HIT_DAMAGE_BONUS:

		case POINT_SKILL_DEFEND_BONUS:
		case POINT_NORMAL_HIT_DEFEND_BONUS:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;

		case POINT_PARTY_ATTACKER_BONUS:
		case POINT_PARTY_TANKER_BONUS:
		case POINT_PARTY_BUFFER_BONUS:
		case POINT_PARTY_SKILL_MASTER_BONUS:
		case POINT_PARTY_HASTE_BONUS:
		case POINT_PARTY_DEFENDER_BONUS:

		case POINT_RESIST_WARRIOR :
		case POINT_RESIST_ASSASSIN :
		case POINT_RESIST_SURA :
		case POINT_RESIST_SHAMAN :
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case POINT_RESIST_WOLFMAN :
#endif

			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;

		case POINT_MALL_ATTBONUS:
		case POINT_MALL_DEFBONUS:
		case POINT_MALL_EXPBONUS:
		case POINT_MALL_ITEMBONUS:
		case POINT_MALL_GOLDBONUS:
		case POINT_MELEE_MAGIC_ATT_BONUS_PER:
			// if (GetPoint(type) + amount > 100)
			// {
				// sys_err("MALL_BONUS exceeded over 100!! point type: %d name: %s amount %d", type, GetName(), amount);
				// amount = 100 - GetPoint(type);
			// }

			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;

		case POINT_PC_BANG_EXP_BONUS :
		case POINT_PC_BANG_DROP_BONUS :
		case POINT_RAMADAN_CANDY_BONUS_EXP:
			SetPoint(type, amount);
			val = GetPoint(type);
			break;

		case POINT_EXP_DOUBLE_BONUS:
		case POINT_GOLD_DOUBLE_BONUS:
		case POINT_ITEM_DROP_BONUS:
		case POINT_POTION_BONUS:
			// if (GetPoint(type) + amount > 100)
			// {
				// sys_err("BONUS exceeded over 100!! point type: %d name: %s amount %d", type, GetName(), amount);
				// amount = 100 - GetPoint(type);
			// }

			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;

		case POINT_IMMUNE_STUN:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			if (val)
			{
				SET_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_STUN);
			}
			else
			{
				REMOVE_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_STUN);
			}
			break;

		case POINT_IMMUNE_SLOW:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			if (val)
			{
				SET_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_SLOW);
			}
			else
			{
				REMOVE_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_SLOW);
			}
			break;

		case POINT_IMMUNE_FALL:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			if (val)
			{
				SET_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_FALL);
			}
			else
			{
				REMOVE_BIT(m_pointsInstant.dwImmuneFlag, IMMUNE_FALL);
			}
			break;

		case POINT_ATT_GRADE_BONUS:
			SetPoint(type, GetPoint(type) + amount);
			PointChange(POINT_ATT_GRADE, amount);
			val = GetPoint(type);
			break;

		case POINT_DEF_GRADE_BONUS:
			SetPoint(type, GetPoint(type) + amount);
			PointChange(POINT_DEF_GRADE, amount);
			val = GetPoint(type);
			break;

		case POINT_MAGIC_ATT_GRADE_BONUS:
			SetPoint(type, GetPoint(type) + amount);
			PointChange(POINT_MAGIC_ATT_GRADE, amount);
			val = GetPoint(type);
			break;

		case POINT_MAGIC_DEF_GRADE_BONUS:
			SetPoint(type, GetPoint(type) + amount);
			PointChange(POINT_MAGIC_DEF_GRADE, amount);
			val = GetPoint(type);
			break;

		case POINT_VOICE:
		case POINT_EMPIRE_POINT:
			val = GetRealPoint(type);
			break;

		case POINT_POLYMORPH:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			SetPolymorph(val);
			break;

		case POINT_MOUNT:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			MountVnum(val);
			break;

		case POINT_ENERGY:
		case POINT_COSTUME_ATTR_BONUS:
			{
				int32_t old_val = GetPoint(type);
				SetPoint(type, old_val + amount);
				val = GetPoint(type);
				BuffOnAttr_ValueChange(type, old_val, val);
			}
			break;
#ifdef __ACCE_SYSTEM__ // @correction191
		case POINT_ACCEDRAIN_RATE:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;
#endif
#ifdef __ANTI_RESIST_MAGIC_REDUCTION__
		case POINT_RESIST_MAGIC_REDUCTION:
			if (GetRaceNum() == 2 or GetRaceNum() == 6)
				amount = amount / 2;

			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;
#endif
#ifdef __ELEMENT_ADD__
		case POINT_ENCHANT_ELECT:
		case POINT_ENCHANT_FIRE:
		case POINT_ENCHANT_ICE:
		case POINT_ENCHANT_WIND:
		case POINT_ENCHANT_EARTH:
		case POINT_ENCHANT_DARK:
		case POINT_ENCHANT_AVATAR:
		case POINT_ATTBONUS_CZ:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;
#endif
#ifdef __PENDANT__
		case POINT_ATTBONUS_SWORD:
		case POINT_ATTBONUS_TWOHAND:
		case POINT_ATTBONUS_DAGGER:
		case POINT_ATTBONUS_BELL:
		case POINT_ATTBONUS_FAN:
		case POINT_ATTBONUS_BOW:
#ifdef ENABLE_WOLFMAN_CHARACTER
		case POINT_ATTBONUS_CLAW:
#endif
		case POINT_RESIST_HUMAN:
		case POINT_RESIST_MOUNT_FALL:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;
#endif
#ifdef __NEW_BONUSES__
		case POINT_ATTBONUS_STONE:
		case POINT_ATTBONUS_BOSS:
			SetPoint(type, GetPoint(type) + amount);
			val = GetPoint(type);
			break;
#endif
		default:
			sys_err("CHARACTER::PointChange: %s: unknown point change type %d", GetName(), type);
			return;
	}

	switch (type)
	{
		case POINT_LEVEL:
		case POINT_ST:
		case POINT_DX:
		case POINT_IQ:
		case POINT_HT:
			ComputeBattlePoints();
			break;
		case POINT_MAX_HP:
		case POINT_MAX_SP:
		case POINT_MAX_STAMINA:
			break;
	}

	if (type == POINT_HP && amount == 0)
		return;

	if (GetDesc())
	{
		struct packet_point_change pack;

		pack.header = HEADER_GC_CHARACTER_POINT_CHANGE;
		pack.dwVID = m_vid;
		pack.type = type;
		pack.value = val;

		if (bAmount)
			pack.amount = amount;
		else
			pack.amount = 0;

		if (!bBroadcast)
			GetDesc()->Packet(&pack, sizeof(struct packet_point_change));
		else
			PacketAround(&pack, sizeof(pack));
	}
}

void CHARACTER::ApplyPoint(uint8_t bApplyType, int32_t iVal)
{
	switch (bApplyType)
	{
		case APPLY_NONE:
			break;

		case APPLY_CON:
			PointChange(POINT_HT, iVal);
			PointChange(POINT_MAX_HP, (iVal * JobInitialPoints[GetJob()].hp_per_ht));
			PointChange(POINT_MAX_STAMINA, (iVal * JobInitialPoints[GetJob()].stamina_per_con));
			break;

		case APPLY_INT:
			PointChange(POINT_IQ, iVal);
			PointChange(POINT_MAX_SP, (iVal * JobInitialPoints[GetJob()].sp_per_iq));
			break;

		case APPLY_SKILL:
			{
				uint8_t bSkillVnum = (uint8_t) (((uint32_t)iVal) >> 24);
				int32_t iAdd = iVal & 0x00800000;
				int32_t iChange = iVal & 0x007fffff;

				sys_log(1, "APPLY_SKILL skill %d add? %d change %d", bSkillVnum, iAdd ? 1 : 0, iChange);

				if (0 == iAdd)
					iChange = -iChange;

				std::unordered_map<uint8_t, int32_t>::iterator iter = m_SkillDamageBonus.find(bSkillVnum);

				if (iter == m_SkillDamageBonus.end())
					m_SkillDamageBonus.insert(std::make_pair(bSkillVnum, iChange));
				else
					iter->second += iChange;
			}
			break;

		case APPLY_MAX_HP:
		case APPLY_MAX_HP_PCT: // @correction122
			{
				int32_t i = GetMaxHP();
				if (!i)
					break;
				PointChange(aApplyInfo[bApplyType].bPointType, iVal);
				float fRatio = (float)GetMaxHP() / (float)i;
				PointChange(POINT_HP, GetHP() * fRatio - GetHP());
			}
			break;

		case APPLY_MAX_SP:
		case APPLY_MAX_SP_PCT: // @correction122
			{
				int32_t i = GetMaxSP();
				if (!i)
					break;
				PointChange(aApplyInfo[bApplyType].bPointType, iVal);
				float fRatio = (float)GetMaxSP() / (float)i;
				PointChange(POINT_SP, GetSP() * fRatio - GetSP());
			}
			break;

		case APPLY_STR:
		case APPLY_DEX:
		case APPLY_ATT_SPEED:
		case APPLY_MOV_SPEED:
		case APPLY_CAST_SPEED:
		case APPLY_HP_REGEN:
		case APPLY_SP_REGEN:
		case APPLY_POISON_PCT:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case APPLY_BLEEDING_PCT:
#endif
		case APPLY_STUN_PCT:
		case APPLY_SLOW_PCT:
		case APPLY_CRITICAL_PCT:
		case APPLY_PENETRATE_PCT:
		case APPLY_ATTBONUS_HUMAN:
		case APPLY_ATTBONUS_ANIMAL:
		case APPLY_ATTBONUS_ORC:
		case APPLY_ATTBONUS_MILGYO:
		case APPLY_ATTBONUS_UNDEAD:
		case APPLY_ATTBONUS_DEVIL:
		case APPLY_ATTBONUS_WARRIOR:
		case APPLY_ATTBONUS_ASSASSIN:
		case APPLY_ATTBONUS_SURA:
		case APPLY_ATTBONUS_SHAMAN:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case APPLY_ATTBONUS_WOLFMAN:
#endif
		case APPLY_ATTBONUS_MONSTER:
		case APPLY_STEAL_HP:
		case APPLY_STEAL_SP:
		case APPLY_MANA_BURN_PCT:
		case APPLY_DAMAGE_SP_RECOVER:
		case APPLY_BLOCK:
		case APPLY_DODGE:
		case APPLY_RESIST_SWORD:
		case APPLY_RESIST_TWOHAND:
		case APPLY_RESIST_DAGGER:
		case APPLY_RESIST_BELL:
		case APPLY_RESIST_FAN:
		case APPLY_RESIST_BOW:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case APPLY_RESIST_CLAW:
#endif
		case APPLY_RESIST_FIRE:
		case APPLY_RESIST_ELEC:
		case APPLY_RESIST_MAGIC:
		case APPLY_RESIST_WIND:
		case APPLY_RESIST_ICE:
		case APPLY_RESIST_EARTH:
		case APPLY_RESIST_DARK:
		case APPLY_REFLECT_MELEE:
		case APPLY_REFLECT_CURSE:
		case APPLY_ANTI_CRITICAL_PCT:
		case APPLY_ANTI_PENETRATE_PCT:
		case APPLY_POISON_REDUCE:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case APPLY_BLEEDING_REDUCE:
#endif
		case APPLY_KILL_SP_RECOVER:
		case APPLY_EXP_DOUBLE_BONUS:
		case APPLY_GOLD_DOUBLE_BONUS:
		case APPLY_ITEM_DROP_BONUS:
		case APPLY_POTION_BONUS:
		case APPLY_KILL_HP_RECOVER:
		case APPLY_IMMUNE_STUN:
		case APPLY_IMMUNE_SLOW:
		case APPLY_IMMUNE_FALL:
		case APPLY_BOW_DISTANCE:
		case APPLY_ATT_GRADE_BONUS:
		case APPLY_DEF_GRADE_BONUS:
		case APPLY_MAGIC_ATT_GRADE:
		case APPLY_MAGIC_DEF_GRADE:
		case APPLY_CURSE_PCT:
		case APPLY_MAX_STAMINA:
		case APPLY_MALL_ATTBONUS:
		case APPLY_MALL_DEFBONUS:
		case APPLY_MALL_EXPBONUS:
		case APPLY_MALL_ITEMBONUS:
		case APPLY_MALL_GOLDBONUS:
		case APPLY_SKILL_DAMAGE_BONUS:
		case APPLY_NORMAL_HIT_DAMAGE_BONUS:

		case APPLY_SKILL_DEFEND_BONUS:
		case APPLY_NORMAL_HIT_DEFEND_BONUS:

		case APPLY_PC_BANG_EXP_BONUS :
		case APPLY_PC_BANG_DROP_BONUS :

		case APPLY_RESIST_WARRIOR :
		case APPLY_RESIST_ASSASSIN :
		case APPLY_RESIST_SURA :
		case APPLY_RESIST_SHAMAN :
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case APPLY_RESIST_WOLFMAN :
#endif
#ifdef __ACCE_SYSTEM__ // @correction191
		case POINT_ACCEDRAIN_RATE:
#endif
#ifdef __ANTI_RESIST_MAGIC_REDUCTION__
		case APPLY_RESIST_MAGIC_REDUCTION: // 98
#endif
#ifdef __ELEMENT_ADD__
		case APPLY_ENCHANT_ELECT:	// 99
		case APPLY_ENCHANT_FIRE:	// 100
		case APPLY_ENCHANT_ICE:	// 101
		case APPLY_ENCHANT_WIND:	// 102
		case APPLY_ENCHANT_EARTH:	// 103
		case APPLY_ENCHANT_DARK:	// 104
		case APPLY_ATTBONUS_CZ:	// 105
		case APPLY_ATTBONUS_INSECT:	// 106
		case APPLY_ATTBONUS_DESERT:	// 107
		case APPLY_ENCHANT_AVATAR:
#endif
#ifdef __PENDANT__
		case APPLY_ATTBONUS_SWORD:
		case APPLY_ATTBONUS_TWOHAND:
		case APPLY_ATTBONUS_DAGGER:
		case APPLY_ATTBONUS_BELL:
		case APPLY_ATTBONUS_FAN:
		case APPLY_ATTBONUS_BOW:
		case APPLY_ATTBONUS_CLAW:
		case APPLY_RESIST_HUMAN:
		case APPLY_RESIST_MOUNT_FALL:
#endif
#ifdef __NEW_BONUSES__
		case APPLY_ATTBONUS_STONE:
		case APPLY_ATTBONUS_BOSS:
#endif
		case APPLY_ENERGY:
		case APPLY_DEF_GRADE:
		case APPLY_COSTUME_ATTR_BONUS:
		case APPLY_MAGIC_ATTBONUS_PER:
		case APPLY_MELEE_MAGIC_ATTBONUS_PER:
			PointChange(aApplyInfo[bApplyType].bPointType, iVal);
			break;

		default:
			sys_err("Unknown apply type %d name %s", bApplyType, GetName());
			break;
	}
}

void CHARACTER::MotionPacketEncode(uint8_t motion, LPCHARACTER victim, struct packet_motion * packet)
{
	packet->header	= HEADER_GC_MOTION;
	packet->vid		= m_vid;
	packet->motion	= motion;

	if (victim)
		packet->victim_vid = victim->GetVID();
	else
		packet->victim_vid = 0;
}

void CHARACTER::Motion(uint8_t motion, LPCHARACTER victim)
{
	struct packet_motion pack_motion;
	MotionPacketEncode(motion, victim, &pack_motion);
	PacketAround(&pack_motion, sizeof(struct packet_motion));
}

EVENTFUNC(save_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );
	if ( info == nullptr )
	{
		sys_err( "save_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == nullptr) {
		return 0;
	}
	sys_log(1, "SAVE_EVENT: %s", ch->GetName());
	ch->Save();
	ch->FlushDelayedSaveItem();
	return (save_event_second_cycle);
}

void CHARACTER::StartSaveEvent()
{
	if (m_pkSaveEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;
	m_pkSaveEvent = event_create(save_event, info, save_event_second_cycle);
}

void CHARACTER::MonsterLog(const char* format, ...)
{
	if (!test_server)
		return;

	if (IsPC())
		return;

	char chatbuf[CHAT_MAX_LEN + 1];
	int32_t len = snprintf(chatbuf, sizeof(chatbuf), "%u)", (uint32_t)GetVID());

	if (len < 0 || len >= (int32_t) sizeof(chatbuf))
		len = sizeof(chatbuf) - 1;

	va_list args;

	va_start(args, format);

	int32_t len2 = vsnprintf(chatbuf + len, sizeof(chatbuf) - len, format, args);

	if (len2 < 0 || len2 >= (int32_t) sizeof(chatbuf) - len)
		len += (sizeof(chatbuf) - len) - 1;
	else
		len += len2;

	++len;

	va_end(args);

	TPacketGCChat pack_chat;

	pack_chat.header    = HEADER_GC_CHAT;
	pack_chat.size		= sizeof(TPacketGCChat) + len;
	pack_chat.type      = CHAT_TYPE_TALKING;
	pack_chat.id        = (uint32_t)GetVID();
	pack_chat.bEmpire	= 0;

	TEMP_BUFFER buf;
	buf.write(&pack_chat, sizeof(TPacketGCChat));
	buf.write(chatbuf, len);

	CHARACTER_MANAGER::instance().PacketMonsterLog(this, buf.read_peek(), buf.size());
}

void CHARACTER::ChatPacket(uint8_t type, const char * format, ...)
{
	LPDESC d = GetDesc();

	if (!d || !format)
		return;

	char chatbuf[CHAT_MAX_LEN + 1];
	va_list args;

	va_start(args, format);
	int32_t len = vsnprintf(chatbuf, sizeof(chatbuf), format, args);
	va_end(args);

	struct packet_chat pack_chat;

	pack_chat.header    = HEADER_GC_CHAT;
	pack_chat.size      = sizeof(struct packet_chat) + len;
	pack_chat.type      = type;
	pack_chat.id        = 0;
	pack_chat.bEmpire   = d->GetEmpire();

	TEMP_BUFFER buf;
	buf.write(&pack_chat, sizeof(struct packet_chat));
	buf.write(chatbuf, len);

	d->Packet(buf.read_peek(), buf.size());

	if (type == CHAT_TYPE_COMMAND && test_server)
		sys_log(0, "SEND_COMMAND %s %s", GetName(), chatbuf);
}

void CHARACTER::mining_take()
{
	m_pkMiningEvent = nullptr;
}

void CHARACTER::mining_cancel()
{
	if (m_pkMiningEvent)
	{
		sys_log(0, "XXX MINING CANCEL");
		event_cancel(&m_pkMiningEvent);
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("채광을 중단하였습니다."));
	}
}

void CHARACTER::mining(LPCHARACTER chLoad)
{
	if (m_pkMiningEvent)
	{
		mining_cancel();
		return;
	}

	if (!chLoad)
		return;

	// @correction049
	if (GetMapIndex() != chLoad->GetMapIndex() || DISTANCE_APPROX(GetX() - chLoad->GetX(), GetY() - chLoad->GetY()) > 1000)
		return;

	if (mining::GetRawOreFromLoad(chLoad->GetRaceNum()) == 0)
		return;

	LPITEM pick = GetWear(WEAR_WEAPON);

	if (!pick || pick->GetType() != ITEM_PICK)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("곡괭이를 장착하세요."));
		return;
	}

	int32_t count = number(5, 15);

	TPacketGCDigMotion p;
	p.header = HEADER_GC_DIG_MOTION;
	p.vid = GetVID();
	p.target_vid = chLoad->GetVID();
	p.count = count;

	PacketAround(&p, sizeof(p));

	m_pkMiningEvent = mining::CreateMiningEvent(this, chLoad, count);
}

void CHARACTER::fishing()
{
	if (m_pkFishingEvent)
	{
		fishing_take();
		return;
	}

	{
		LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(GetMapIndex());

		int32_t	x = GetX();
		int32_t y = GetY();

		LPSECTREE tree = pkSectreeMap->Find(x, y);
		uint32_t dwAttr = tree->GetAttribute(x, y);

		if (IS_SET(dwAttr, ATTR_BLOCK))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("낚시를 할 수 있는 곳이 아닙니다"));
			return;
		}
	}

	LPITEM rod = GetWear(WEAR_WEAPON);

	if (!rod || rod->GetType() != ITEM_ROD)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("낚시대를 장착 하세요."));
		return;
	}

	if (0 == rod->GetSocket(2))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("미끼를 끼고 던져 주세요."));
		return;
	}

	float fx, fy;
	GetDeltaByDegree(GetRotation(), 400.0f, &fx, &fy);

	m_pkFishingEvent = fishing::CreateFishingEvent(this);
}

void CHARACTER::fishing_take()
{
	LPITEM rod = GetWear(WEAR_WEAPON);
	if (rod && rod->GetType() == ITEM_ROD)
	{
		using fishing::fishing_event_info;
		if (m_pkFishingEvent)
		{
			struct fishing_event_info* info = dynamic_cast<struct fishing_event_info*>(m_pkFishingEvent->info);

			if (info)
				fishing::Take(info, this);
		}
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("낚시대가 아닌 물건으로 낚시를 할 수 없습니다!"));
	}

	event_cancel(&m_pkFishingEvent);
}

bool CHARACTER::StartStateMachine(int32_t iNextPulse)
{
	if (CHARACTER_MANAGER::instance().AddToStateList(this))
	{
		m_dwNextStatePulse = thecore_heart->pulse + iNextPulse;
		return true;
	}

	return false;
}

void CHARACTER::StopStateMachine()
{
	CHARACTER_MANAGER::instance().RemoveFromStateList(this);
}

void CHARACTER::UpdateStateMachine(uint32_t dwPulse)
{
	if (dwPulse < m_dwNextStatePulse)
		return;

	if (IsDead())
		return;

	Update();
	m_dwNextStatePulse = dwPulse + m_dwStateDuration;
}

void CHARACTER::SetNextStatePulse(int32_t iNextPulse)
{
	CHARACTER_MANAGER::instance().AddToStateList(this);
	m_dwNextStatePulse = iNextPulse;

	if (iNextPulse < 10)
		MonsterLog("다음상태로어서가자");
}


void CHARACTER::UpdateCharacter(uint32_t dwPulse)
{
	CFSM::Update();
}

void CHARACTER::SetShop(LPSHOP pkShop)
{
	if ((m_pkShop = pkShop))
		SET_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_SHOP);
	else
	{
		REMOVE_BIT(m_pointsInstant.instant_flag, INSTANT_FLAG_SHOP);
		SetShopOwner(nullptr);
	}
}

void CHARACTER::SetExchange(CExchange * pkExchange)
{
	m_pkExchange = pkExchange;
}

void CHARACTER::SetPart(uint8_t bPartPos, uint16_t wVal)
{
	assert(bPartPos < PART_MAX_NUM);
	m_pointsInstant.parts[bPartPos] = wVal;
}

uint16_t CHARACTER::GetPart(uint8_t bPartPos) const
{
	assert(bPartPos < PART_MAX_NUM);
	return m_pointsInstant.parts[bPartPos];
}

uint16_t CHARACTER::GetOriginalPart(uint8_t bPartPos) const
{
	switch (bPartPos)
	{
		case PART_MAIN:
			if (!IsPC())
				return GetPart(PART_MAIN);
			else
				return m_pointsInstant.bBasePart;

		case PART_HAIR:
			return GetPart(PART_HAIR);
#ifdef __ACCE_SYSTEM__ // @correction191
		case PART_ACCE:
			return GetPart(PART_ACCE);
#endif
#ifdef __COSTUME_WEAPON__ // @correction204
		case PART_WEAPON:
			return GetPart(PART_WEAPON);
#endif
#ifdef __COSTUME_EFFECT__
		case PART_BODY_EFFECT:
			return GetPart(PART_BODY_EFFECT);
		case PART_WEAPON_RIGHT_EFFECT:
			return GetPart(PART_WEAPON_RIGHT_EFFECT);
		case PART_WEAPON_LEFT_EFFECT:
			return GetPart(PART_WEAPON_LEFT_EFFECT);
#endif
		default:
			return 0;
	}
}

uint8_t CHARACTER::GetCharType() const
{
	return m_bCharType;
}

bool CHARACTER::SetSyncOwner(LPCHARACTER ch, bool bRemoveFromList)
{
	if (IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE))
		return false;
	
	if (ch && !battle_is_attackable(ch, this)) // @correction052
	{
		SendDamagePacket(ch, 0, DAMAGE_BLOCK);
		return false;
	}

	if (ch == this)
	{
		sys_err("SetSyncOwner owner == this (%p)", this);
		return false;
	}

	if (!ch)
	{
		if (bRemoveFromList && m_pkChrSyncOwner)
		{
			m_pkChrSyncOwner->m_kLst_pkChrSyncOwned.remove(this);
		}

		if (m_pkChrSyncOwner)
			sys_log(1, "SyncRelease %s %p from %s", GetName(), this, m_pkChrSyncOwner->GetName());

		m_pkChrSyncOwner = nullptr;
	}
	else
	{
		if (!IsSyncOwner(ch))
			return false;

		if (DISTANCE_APPROX(GetX() - ch->GetX(), GetY() - ch->GetY()) > 250)
		{
			sys_log(1, "SetSyncOwner distance over than 250 %s %s", GetName(), ch->GetName());

			if (m_pkChrSyncOwner == ch)
				return true;

			return false;
		}

		if (m_pkChrSyncOwner != ch)
		{
			if (m_pkChrSyncOwner)
			{
				sys_log(1, "SyncRelease %s %p from %s", GetName(), this, m_pkChrSyncOwner->GetName());
				m_pkChrSyncOwner->m_kLst_pkChrSyncOwned.remove(this);
			}

			m_pkChrSyncOwner = ch;
			m_pkChrSyncOwner->m_kLst_pkChrSyncOwned.push_back(this);

			static const timeval zero_tv = {0, 0};
			SetLastSyncTime(zero_tv);

			sys_log(1, "SetSyncOwner set %s %p to %s", GetName(), this, ch->GetName());
		}

		m_fSyncTime = get_float_time();
	}

	TPacketGCOwnership pack;

	pack.bHeader	= HEADER_GC_OWNERSHIP;
	pack.dwOwnerVID	= ch ? ch->GetVID() : 0;
	pack.dwVictimVID	= GetVID();

	PacketAround(&pack, sizeof(TPacketGCOwnership));
	return true;
}

struct FuncClearSync
{
	void operator () (LPCHARACTER ch)
	{
		assert(ch != nullptr);
		ch->SetSyncOwner(nullptr, false);
	}
};

void CHARACTER::ClearSync()
{
	SetSyncOwner(nullptr);

	std::for_each(m_kLst_pkChrSyncOwned.begin(), m_kLst_pkChrSyncOwned.end(), FuncClearSync());
	m_kLst_pkChrSyncOwned.clear();
}

bool CHARACTER::IsSyncOwner(LPCHARACTER ch) const
{
	if (m_pkChrSyncOwner == ch)
		return true;

	if (get_float_time() - m_fSyncTime >= 3.0f)
		return true;

	return false;
}

void CHARACTER::SetParty(LPPARTY pkParty)
{
	if (pkParty == m_pkParty)
		return;

	if (pkParty && m_pkParty)
		sys_err("%s is trying to reassigning party (current %p, new party %p)", GetName(), get_pointer(m_pkParty), get_pointer(pkParty));

	sys_log(1, "PARTY set to %p", get_pointer(pkParty));

	m_pkParty = pkParty;

	if (IsPC())
	{
		if (m_pkParty)
			SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_PARTY);
		else
			REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_PARTY);

		UpdatePacket();
	}
}

EVENTINFO(TPartyJoinEventInfo)
{
	uint32_t	dwGuestPID;
	uint32_t	dwLeaderPID;

	TPartyJoinEventInfo()
	: dwGuestPID( 0 )
	, dwLeaderPID( 0 )
	{
	}
} ;

EVENTFUNC(party_request_event)
{
	TPartyJoinEventInfo * info = dynamic_cast<TPartyJoinEventInfo *>(  event->info );

	if ( info == nullptr )
	{
		sys_err( "party_request_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(info->dwGuestPID);

	if (ch)
	{
		sys_log(0, "PartyRequestEvent %s", ch->GetName());
		ch->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
		ch->SetPartyRequestEvent(nullptr);
	}

	return 0;
}

bool CHARACTER::RequestToParty(LPCHARACTER leader)
{
	if (leader->GetParty())
		leader = leader->GetParty()->GetLeaderCharacter();

	if (!leader)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("파티장이 접속 상태가 아니라서 요청을 할 수 없습니다."));
		return false;
	}

	if (m_pkPartyRequestEvent)
		return false;

	if (!IsPC() || !leader->IsPC())
		return false;

	if (leader->IsBlockMode(BLOCK_PARTY_REQUEST))
		return false;

	PartyJoinErrCode errcode = IsPartyJoinableCondition(leader, this);

	switch (errcode)
	{
		case PERR_NONE:
			break;

		case PERR_SERVER:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다."));
			return false;

		case PERR_DIFFEMPIRE:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 다른 제국과 파티를 이룰 수 없습니다."));
			return false;

		case PERR_DUNGEON:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 던전 안에서는 파티 초대를 할 수 없습니다."));
			return false;

		case PERR_OBSERVER:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 관전 모드에선 파티 초대를 할 수 없습니다."));
			return false;

		case PERR_LVBOUNDARY:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> -30 ~ +30 레벨 이내의 상대방만 초대할 수 있습니다."));
			return false;

		case PERR_LOWLEVEL:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티내 최고 레벨 보다 30레벨이 낮아 초대할 수 없습니다."));
			return false;

		case PERR_HILEVEL:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티내 최저 레벨 보다 30레벨이 높아 초대할 수 없습니다."));
			return false;

		case PERR_ALREADYJOIN:
			return false;

		case PERR_PARTYISFULL:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 더 이상 파티원을 초대할 수 없습니다."));
			return false;

		default:
			sys_err("Do not process party join error(%d)", errcode);
			return false;
	}

	TPartyJoinEventInfo* info = AllocEventInfo<TPartyJoinEventInfo>();

	info->dwGuestPID = GetPlayerID();
	info->dwLeaderPID = leader->GetPlayerID();

	SetPartyRequestEvent(event_create(party_request_event, info, PASSES_PER_SEC(10)));

	leader->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequest %u", (uint32_t) GetVID());
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s 님에게 파티가입 신청을 했습니다."), leader->GetName());
	return true;
}

void CHARACTER::DenyToParty(LPCHARACTER member)
{
	sys_log(1, "DenyToParty %s member %s %p", GetName(), member->GetName(), get_pointer(member->m_pkPartyRequestEvent));

	if (!member->m_pkPartyRequestEvent)
		return;

	TPartyJoinEventInfo * info = dynamic_cast<TPartyJoinEventInfo *>(member->m_pkPartyRequestEvent->info);

	if (!info)
	{
		sys_err( "CHARACTER::DenyToParty> <Factor> Null pointer" );
		return;
	}

	if (info->dwGuestPID != member->GetPlayerID())
		return;

	if (info->dwLeaderPID != GetPlayerID())
		return;

	event_cancel(&member->m_pkPartyRequestEvent);

	member->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

void CHARACTER::AcceptToParty(LPCHARACTER member)
{
	sys_log(1, "AcceptToParty %s member %s %p", GetName(), member->GetName(), get_pointer(member->m_pkPartyRequestEvent));

	if (!member->m_pkPartyRequestEvent)
		return;

	TPartyJoinEventInfo * info = dynamic_cast<TPartyJoinEventInfo *>(member->m_pkPartyRequestEvent->info);

	if (!info)
	{
		sys_err( "CHARACTER::AcceptToParty> <Factor> Null pointer" );
		return;
	}

	if (info->dwGuestPID != member->GetPlayerID())
		return;

	if (info->dwLeaderPID != GetPlayerID())
		return;

	event_cancel(&member->m_pkPartyRequestEvent);

	if (!GetParty())
		member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 파티에 속해있지 않습니다."));
	else
	{
		if (GetPlayerID() != GetParty()->GetLeaderPID())
			return;

		PartyJoinErrCode errcode = IsPartyJoinableCondition(this, member);
		switch (errcode)
		{
			case PERR_NONE: 		member->PartyJoin(this); return;
			case PERR_SERVER:		member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다.")); break;
			case PERR_DUNGEON:		member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 던전 안에서는 파티 초대를 할 수 없습니다.")); break;
			case PERR_OBSERVER: 	member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 관전 모드에선 파티 초대를 할 수 없습니다.")); break;
			case PERR_LVBOUNDARY:	member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> -30 ~ +30 레벨 이내의 상대방만 초대할 수 있습니다.")); break;
			case PERR_LOWLEVEL: 	member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티내 최고 레벨 보다 30레벨이 낮아 초대할 수 없습니다.")); break;
			case PERR_HILEVEL: 		member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티내 최저 레벨 보다 30레벨이 높아 초대할 수 없습니다.")); break;
			case PERR_ALREADYJOIN: 	break;
			case PERR_PARTYISFULL: {
									   ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 더 이상 파티원을 초대할 수 없습니다."));
									   member->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티의 인원제한이 초과하여 파티에 참가할 수 없습니다."));
									   break;
								   }
			default: sys_err("Do not process party join error(%d)", errcode);
		}
	}

	member->ChatPacket(CHAT_TYPE_COMMAND, "PartyRequestDenied");
}

EVENTFUNC(party_invite_event)
{
	TPartyJoinEventInfo * pInfo = dynamic_cast<TPartyJoinEventInfo *>(  event->info );

	if ( pInfo == nullptr )
	{
		sys_err( "party_invite_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER pchInviter = CHARACTER_MANAGER::instance().FindByPID(pInfo->dwLeaderPID);

	if (pchInviter)
	{
		sys_log(1, "PartyInviteEvent %s", pchInviter->GetName());
		pchInviter->PartyInviteDeny(pInfo->dwGuestPID);
	}

	return 0;
}

void CHARACTER::PartyInvite(LPCHARACTER pchInvitee)
{
	if (GetParty() && GetParty()->GetLeaderPID() != GetPlayerID())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티원을 초대할 수 있는 권한이 없습니다."));
		return;
	}
	else if (pchInvitee->IsBlockMode(BLOCK_PARTY_INVITE))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> %s 님이 파티 거부 상태입니다."), pchInvitee->GetName());
		return;
	}

	PartyJoinErrCode errcode = IsPartyJoinableCondition(this, pchInvitee);

	switch (errcode)
	{
		case PERR_NONE:
			break;

		case PERR_SERVER:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다."));
			return;

		case PERR_DIFFEMPIRE:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 다른 제국과 파티를 이룰 수 없습니다."));
			return;

		case PERR_DUNGEON:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 던전 안에서는 파티 초대를 할 수 없습니다."));
			return;

		case PERR_OBSERVER:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 관전 모드에선 파티 초대를 할 수 없습니다."));
			return;

		case PERR_LVBOUNDARY:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> -30 ~ +30 레벨 이내의 상대방만 초대할 수 있습니다."));
			return;

		case PERR_LOWLEVEL:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티내 최고 레벨 보다 30레벨이 낮아 초대할 수 없습니다."));
			return;

		case PERR_HILEVEL:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티내 최저 레벨 보다 30레벨이 높아 초대할 수 없습니다."));
			return;

		case PERR_ALREADYJOIN:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 이미 %s님은 파티에 속해 있습니다."), pchInvitee->GetName());
			return;

		case PERR_PARTYISFULL:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 더 이상 파티원을 초대할 수 없습니다."));
			return;

		default:
			sys_err("Do not process party join error(%d)", errcode);
			return;
	}

	if (m_PartyInviteEventMap.end() != m_PartyInviteEventMap.find(pchInvitee->GetPlayerID()))
		return;

	TPartyJoinEventInfo* info = AllocEventInfo<TPartyJoinEventInfo>();

	info->dwGuestPID = pchInvitee->GetPlayerID();
	info->dwLeaderPID = GetPlayerID();

	m_PartyInviteEventMap.insert(EventMap::value_type(pchInvitee->GetPlayerID(), event_create(party_invite_event, info, PASSES_PER_SEC(10))));


	TPacketGCPartyInvite p;
	p.header = HEADER_GC_PARTY_INVITE;
	p.leader_vid = GetVID();
	pchInvitee->GetDesc()->Packet(&p, sizeof(p));
}

void CHARACTER::PartyInviteAccept(LPCHARACTER pchInvitee)
{
	EventMap::iterator itFind = m_PartyInviteEventMap.find(pchInvitee->GetPlayerID());

	if (itFind == m_PartyInviteEventMap.end())
	{
		sys_log(1, "PartyInviteAccept from not invited character(%s)", pchInvitee->GetName());
		return;
	}

	event_cancel(&itFind->second);
	m_PartyInviteEventMap.erase(itFind);

	if (GetParty() && GetParty()->GetLeaderPID() != GetPlayerID())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티원을 초대할 수 있는 권한이 없습니다."));
		return;
	}

	PartyJoinErrCode errcode = IsPartyJoinableMutableCondition(this, pchInvitee);

	switch (errcode)
	{
		case PERR_NONE:
			break;

		case PERR_SERVER:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 서버 문제로 파티 관련 처리를 할 수 없습니다."));
			return;

		case PERR_DUNGEON:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 던전 안에서는 파티 초대에 응할 수 없습니다."));
			return;

		case PERR_OBSERVER:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 관전 모드에선 파티 초대를 할 수 없습니다."));
			return;

		case PERR_LVBOUNDARY:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> -30 ~ +30 레벨 이내의 상대방만 초대할 수 있습니다."));
			return;

		case PERR_LOWLEVEL:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티내 최고 레벨 보다 30레벨이 낮아 초대할 수 없습니다."));
			return;

		case PERR_HILEVEL:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티내 최저 레벨 보다 30레벨이 높아 초대할 수 없습니다."));
			return;

		case PERR_ALREADYJOIN:
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티 초대에 응할 수 없습니다."));
			return;

		case PERR_PARTYISFULL:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 더 이상 파티원을 초대할 수 없습니다."));
			pchInvitee->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> 파티의 인원제한이 초과하여 파티에 참가할 수 없습니다."));
			return;

		default:
			sys_err("ignore party join error(%d)", errcode);
			return;
	}


	if (GetParty())
		pchInvitee->PartyJoin(this);
	else
	{
		LPPARTY pParty = CPartyManager::instance().CreateParty(this);

		pParty->Join(pchInvitee->GetPlayerID());
		pParty->Link(pchInvitee);
		pParty->SendPartyInfoAllToOne(this);
	}
}

void CHARACTER::PartyInviteDeny(uint32_t dwPID)
{
	EventMap::iterator itFind = m_PartyInviteEventMap.find(dwPID);

	if (itFind == m_PartyInviteEventMap.end())
	{
		sys_log(1, "PartyInviteDeny to not exist event(inviter PID: %d, invitee PID: %d)", GetPlayerID(), dwPID);
		return;
	}

	event_cancel(&itFind->second);
	m_PartyInviteEventMap.erase(itFind);

	LPCHARACTER pchInvitee = CHARACTER_MANAGER::instance().FindByPID(dwPID);
	if (pchInvitee)
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> %s님이 파티 초대를 거절하셨습니다."), pchInvitee->GetName());
}

void CHARACTER::PartyJoin(LPCHARACTER pLeader)
{
	pLeader->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> %s님이 파티에 참가하셨습니다."), GetName());
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<파티> %s님의 파티에 참가하셨습니다."), pLeader->GetName());

	pLeader->GetParty()->Join(GetPlayerID());
	pLeader->GetParty()->Link(this);
}

CHARACTER::PartyJoinErrCode CHARACTER::IsPartyJoinableCondition(const LPCHARACTER pchLeader, const LPCHARACTER pchGuest)
{
	if (pchLeader->GetEmpire() != pchGuest->GetEmpire())
		return PERR_DIFFEMPIRE;

	return IsPartyJoinableMutableCondition(pchLeader, pchGuest);
}

static bool __party_can_join_by_level(LPCHARACTER leader, LPCHARACTER quest)
{
	// @correction005
	int32_t	level_limit = 30;
	return (abs(leader->GetLevel() - quest->GetLevel()) <= level_limit);
}

CHARACTER::PartyJoinErrCode CHARACTER::IsPartyJoinableMutableCondition(const LPCHARACTER pchLeader, const LPCHARACTER pchGuest)
{
	if (!CPartyManager::instance().IsEnablePCParty())
		return PERR_SERVER;
	else if (pchLeader->GetDungeon())
		return PERR_DUNGEON;
	else if (pchGuest->IsObserverMode())
		return PERR_OBSERVER;
	else if (false == __party_can_join_by_level(pchLeader, pchGuest))
		return PERR_LVBOUNDARY;
	else if (pchGuest->GetParty())
		return PERR_ALREADYJOIN;
	else if (pchLeader->GetParty())
   	{
	   	if (pchLeader->GetParty()->GetMemberCount() == PARTY_MAX_MEMBER)
			return PERR_PARTYISFULL;
	}

	return PERR_NONE;
}

void CHARACTER::SetDungeon(LPDUNGEON pkDungeon)
{
	if (pkDungeon && m_pkDungeon)
		sys_err("%s is trying to reassigning dungeon (current %p, new party %p)", GetName(), get_pointer(m_pkDungeon), get_pointer(pkDungeon));

	if (m_pkDungeon == pkDungeon) {
		return;
	}

	if (m_pkDungeon)
	{
		if (IsPC())
		{
			if (GetParty())
				m_pkDungeon->DecPartyMember(GetParty(), this);
			else
				m_pkDungeon->DecMember(this);
		}
		else if (IsMonster() || IsStone())
		{
			m_pkDungeon->DecMonster();
		}
	}

	m_pkDungeon = pkDungeon;

	if (pkDungeon)
	{
		sys_log(0, "%s DUNGEON set to %p, PARTY is %p", GetName(), get_pointer(pkDungeon), get_pointer(m_pkParty));

		if (IsPC())
		{
			if (GetParty())
				m_pkDungeon->IncPartyMember(GetParty(), this);
			else
				m_pkDungeon->IncMember(this);
		}
		else if (IsMonster() || IsStone())
		{
			m_pkDungeon->IncMonster();
		}
	}
}

void CHARACTER::SetWarMap(CWarMap * pWarMap)
{
	if (m_pWarMap)
		m_pWarMap->DecMember(this);

	m_pWarMap = pWarMap;

	if (m_pWarMap)
		m_pWarMap->IncMember(this);
}

void CHARACTER::SetWeddingMap(marriage::WeddingMap* pMap)
{
	if (m_pWeddingMap)
		m_pWeddingMap->DecMember(this);

	m_pWeddingMap = pMap;

	if (m_pWeddingMap)
		m_pWeddingMap->IncMember(this);
}

void CHARACTER::SetRegen(LPREGEN pkRegen)
{
	m_pkRegen = pkRegen;
	if (pkRegen != nullptr) {
		regen_id_ = pkRegen->id;
	}
	m_fRegenAngle = GetRotation();
	m_posRegen = GetXYZ();
}

bool CHARACTER::OnIdle()
{
	return false;
}

void CHARACTER::OnMove(bool bIsAttack)
{
	m_dwLastMoveTime = get_dword_time();

	if (bIsAttack)
	{
		m_dwLastAttackTime = m_dwLastMoveTime;

		if (IsAffectFlag(AFF_REVIVE_INVISIBLE))
			RemoveAffect(AFFECT_REVIVE_INVISIBLE);

		if (IsAffectFlag(AFF_EUNHYUNG))
		{
			RemoveAffect(SKILL_EUNHYUNG);
			SetAffectedEunhyung();
		}
		else
		{
			ClearAffectedEunhyung();
		}

	}


	mining_cancel();
}

void CHARACTER::OnClick(LPCHARACTER pkChrCauser)
{
	if (!pkChrCauser)
	{
		sys_err("OnClick %s by nullptr", GetName());
		return;
	}

	uint32_t vid = GetVID();
	sys_log(0, "OnClick %s[vnum %d ServerUniqueID %d, pid %d] by %s", GetName(), GetRaceNum(), vid, GetPlayerID(), pkChrCauser->GetName());

#ifdef __GUILD_DRAGONLAIR__
	if ((IsNPC()) && (GetRaceNum() == (uint16_t)(GATE_VNUM)) && (CMgr::instance().IsMeleyMap(pkChrCauser->GetMapIndex())))
	{
		sys_log(0, "Start Meley %s[vnum %d ServerUniqueID %d, pid %d] by %s", GetName(), GetRaceNum(), vid, GetPlayerID(), pkChrCauser->GetName());
		CMgr::instance().Start(pkChrCauser);
		return;
	}
#endif

	{
		if (pkChrCauser->GetMyShop() && pkChrCauser != this)
		{
			sys_err("OnClick Fail (%s->%s) - pc has shop", pkChrCauser->GetName(), GetName());
			return;
		}
	}

	{
		if (pkChrCauser->GetExchange())
		{
			sys_err("OnClick Fail (%s->%s) - pc is exchanging", pkChrCauser->GetName(), GetName());
			return;
		}
	}

	if (IsPC())
	{
		if (!CTargetManager::instance().GetTargetInfo(pkChrCauser->GetPlayerID(), TARGET_TYPE_VID, GetVID()))
		{
			if (GetMyShop())
			{
				if (pkChrCauser->IsDead() == true) return;

				if (pkChrCauser == this)
				{
					if ((GetExchange() || IsOpenSafebox() || GetShopOwner()) || IsCubeOpen())
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중(창고,교환,상점)에는 개인상점을 사용할 수 없습니다."));
						return;
					}
				}
				else
				{
					if ((pkChrCauser->GetExchange() || pkChrCauser->IsOpenSafebox() || pkChrCauser->GetMyShop() || pkChrCauser->GetShopOwner()) || pkChrCauser->IsCubeOpen() )
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중(창고,교환,상점)에는 개인상점을 사용할 수 없습니다."));
						return;
					}

					if ((GetExchange() || IsOpenSafebox() || IsCubeOpen()))
					{
						pkChrCauser->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 다른 거래를 하고 있는 중입니다."));
						return;
					}
				}

				if (pkChrCauser->GetShop())
				{
					pkChrCauser->GetShop()->RemoveGuest(pkChrCauser);
					pkChrCauser->SetShop(nullptr);
				}

				GetMyShop()->AddGuest(pkChrCauser, GetVID(), false);
				pkChrCauser->SetShopOwner(this);
				return;
			}

			if (test_server)
				sys_err("%s.OnClickFailure(%s) - target is PC", pkChrCauser->GetName(), GetName());

			return;
		}
	}

	// @correction005
	if (g_bChinaIntoxicationCheck) // @correction215
	{
		if (pkChrCauser->IsOverTime(OT_3HOUR))
		{
			sys_log(0, "Teen OverTime : name = %s, hour = %d)", pkChrCauser->GetName(), 3);
			return;
		}
		else if (pkChrCauser->IsOverTime(OT_5HOUR))
		{
			sys_log(0, "Teen OverTime : name = %s, hour = %d)", pkChrCauser->GetName(), 5);
			return;
		}
	}


	pkChrCauser->SetQuestNPCID(GetVID());

	if (quest::CQuestManager::instance().Click(pkChrCauser->GetPlayerID(), this))
	{
		return;
	}


	if (!IsPC())
	{
		if (!m_triggerOnClick.pFunc)
		{
			return;
		}

		m_triggerOnClick.pFunc(this, pkChrCauser);
	}

}

uint8_t CHARACTER::GetGMLevel() const
{
	if (test_server)
		return GM_IMPLEMENTOR;
	return m_pointsInstant.gm_level;
}

void CHARACTER::SetGMLevel()
{
	if (GetDesc())
	{
	    m_pointsInstant.gm_level =  gm_get_level(GetName(), GetDesc()->GetHostName(), GetDesc()->GetAccountTable().login);
	}
	else
	{
	    m_pointsInstant.gm_level = GM_PLAYER;
	}
}

bool CHARACTER::IsGM() const
{
	if (m_pointsInstant.gm_level != GM_PLAYER)
		return true;
	if (test_server)
		return true;
	return false;
}

void CHARACTER::SetStone(LPCHARACTER pkChrStone)
{
	m_pkChrStone = pkChrStone;

	if (m_pkChrStone)
	{
		if (pkChrStone->m_set_pkChrSpawnedBy.find(this) == pkChrStone->m_set_pkChrSpawnedBy.end())
			pkChrStone->m_set_pkChrSpawnedBy.insert(this);
	}
}

struct FuncDeadSpawnedByStone
{
	void operator () (LPCHARACTER ch)
	{
		ch->Dead(nullptr);
		ch->SetStone(nullptr);
	}
};

void CHARACTER::ClearStone()
{
	if (!m_set_pkChrSpawnedBy.empty())
	{
		FuncDeadSpawnedByStone f;
		std::for_each(m_set_pkChrSpawnedBy.begin(), m_set_pkChrSpawnedBy.end(), f);
		m_set_pkChrSpawnedBy.clear();
	}

	if (!m_pkChrStone)
		return;

	m_pkChrStone->m_set_pkChrSpawnedBy.erase(this);
	m_pkChrStone = nullptr;
}

void CHARACTER::ClearTarget()
{
	if (m_pkChrTarget)
	{
		m_pkChrTarget->m_set_pkChrTargetedBy.erase(this);
		m_pkChrTarget = nullptr;
	}

	TPacketGCTarget p;

	p.header = HEADER_GC_TARGET;
	p.dwVID = 0;
	p.bHPPercent = 0;
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
	p.iMinHP = 0;
	p.iMaxHP = 0;
#endif

	CHARACTER_SET::iterator it = m_set_pkChrTargetedBy.begin();

	while (it != m_set_pkChrTargetedBy.end())
	{
		LPCHARACTER pkChr = *(it++);
		pkChr->m_pkChrTarget = nullptr;

		if (!pkChr->GetDesc())
		{
			sys_err("%s %p does not have desc", pkChr->GetName(), get_pointer(pkChr));
			abort();
		}

		pkChr->GetDesc()->Packet(&p, sizeof(TPacketGCTarget));
	}

	m_set_pkChrTargetedBy.clear();
}

void CHARACTER::SetTarget(LPCHARACTER pkChrTarget)
{
	if (m_pkChrTarget == pkChrTarget)
		return;

	if (IS_CASTLE_MAP(GetMapIndex()) && !IsGM())
		return;

	if (m_pkChrTarget)
		m_pkChrTarget->m_set_pkChrTargetedBy.erase(this);

	m_pkChrTarget = pkChrTarget;

	TPacketGCTarget p;

	p.header = HEADER_GC_TARGET;

	if (m_pkChrTarget)
	{
		m_pkChrTarget->m_set_pkChrTargetedBy.insert(this);

		p.dwVID	= m_pkChrTarget->GetVID();
#ifdef __VIEW_TARGET_PLAYER_HP__ // @correction198
		if ((m_pkChrTarget->GetMaxHP() <= 0))
		{
			p.bHPPercent = 0;
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
			p.iMinHP = 0;
			p.iMaxHP = 0;
#endif
		}
		else if (m_pkChrTarget->IsPC() && !m_pkChrTarget->IsPolymorphed())
		{
			uint64_t qwHp = m_pkChrTarget->GetHP();
			uint64_t qwMaxHp = m_pkChrTarget->GetMaxHP();
			uint64_t qwPerc = (qwHp * 100);
			uint64_t bPerc = qwPerc / qwMaxHp;
			p.bHPPercent = MINMAX(0, bPerc, 100);
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
			p.iMinHP = m_pkChrTarget->GetHP();
			p.iMaxHP = m_pkChrTarget->GetMaxHP();
#endif
#else
		if ((m_pkChrTarget->IsPC() && !m_pkChrTarget->IsPolymorphed()) || (m_pkChrTarget->GetMaxHP() <= 0))
		{
			p.bHPPercent = 0;
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
			p.iMinHP = 0;
			p.iMaxHP = 0;
#endif
#endif
		}
		else
		{
			if (m_pkChrTarget->GetRaceNum() == 20101 ||
					m_pkChrTarget->GetRaceNum() == 20102 ||
					m_pkChrTarget->GetRaceNum() == 20103 ||
					m_pkChrTarget->GetRaceNum() == 20104 ||
					m_pkChrTarget->GetRaceNum() == 20105 ||
					m_pkChrTarget->GetRaceNum() == 20106 ||
					m_pkChrTarget->GetRaceNum() == 20107 ||
					m_pkChrTarget->GetRaceNum() == 20108 ||
					m_pkChrTarget->GetRaceNum() == 20109)
			{
				LPCHARACTER owner = m_pkChrTarget->GetVictim();

				if (owner)
				{
					int32_t iHorseHealth = owner->GetHorseHealth();
					int32_t iHorseMaxHealth = owner->GetHorseMaxHealth();

					if (iHorseMaxHealth)
					{
						p.bHPPercent = MINMAX(0,  iHorseHealth * 100 / iHorseMaxHealth, 100);
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
						p.iMinHP = 100;
						p.iMaxHP = 100;
#endif
					}
					else
					{
						p.bHPPercent = 100;
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
						p.iMinHP = 100;
						p.iMaxHP = 100;
#endif
					}
				}
				else
				{
					p.bHPPercent = 100;
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
					p.iMinHP = 100;
					p.iMaxHP = 100;
#endif
				}
			}
			else
			{
				if (m_pkChrTarget->GetMaxHP() <= 0) // @correction056
				{
					p.bHPPercent = 0;
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
					p.iMinHP = 0;
					p.iMaxHP = 0;
#endif
				}
				else
				{
					uint64_t qwHp = m_pkChrTarget->GetHP();
					uint64_t qwMaxHp = m_pkChrTarget->GetMaxHP();
					uint64_t qwPerc = (qwHp * 100);
					uint64_t bPerc = qwPerc / qwMaxHp;
					p.bHPPercent = MINMAX(0, bPerc, 100);
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
					p.iMinHP = m_pkChrTarget->GetHP();
					p.iMaxHP = m_pkChrTarget->GetMaxHP();
#endif
				}
			}
		}
	}
	else
	{
		p.dwVID = 0;
		p.bHPPercent = 0;
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
		p.iMinHP = 0;
		p.iMaxHP = 0;
#endif
	}

	GetDesc()->Packet(&p, sizeof(TPacketGCTarget));
}

void CHARACTER::BroadcastTargetPacket()
{
	if (m_set_pkChrTargetedBy.empty())
		return;

	TPacketGCTarget p;
	p.header = HEADER_GC_TARGET;
	p.dwVID = GetVID();
	if (GetMaxHP() <= 0) // @correction056
	{
		p.bHPPercent = 0;
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
		p.iMinHP = 0;
		p.iMaxHP = 0;
#endif
	}
	else
	{
#ifdef __VIEW_TARGET_PLAYER_HP__ // @correction198
		uint64_t qwHp = GetHP();
		uint64_t qwMaxHp = GetMaxHP();
		uint64_t qwPerc = (qwHp * 100);
		uint64_t bPerc = qwPerc / qwMaxHp;
		p.bHPPercent = MINMAX(0, bPerc, 100);
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
		p.iMinHP = GetHP();
		p.iMaxHP = GetMaxHP();
#endif
#else
		if (IsPC())
		{
			p.bHPPercent = 0;
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
			p.iMinHP = 0;
			p.iMaxHP = 0;
#endif
		}
		else
		{
			uint64_t qwHp = GetHP();
			uint64_t qwMaxHp = GetMaxHP();
			uint64_t qwPerc = (qwHp * 100);
			uint64_t bPerc = qwPerc / qwMaxHp;
			p.bHPPercent = MINMAX(0, bPerc, 100);
#ifdef __VIEW_TARGET_DECIMAL_HP__ // @correction199
			p.iMinHP = GetHP();
			p.iMaxHP = GetMaxHP();
#endif
		}
#endif
	}

	CHARACTER_SET::iterator it = m_set_pkChrTargetedBy.begin();
	while (it != m_set_pkChrTargetedBy.end())
	{
		LPCHARACTER pkChr = *it++;
		if (!pkChr->GetDesc())
		{
			sys_err("%s %p does not have desc", pkChr->GetName(), get_pointer(pkChr));
			abort();
		}

		pkChr->GetDesc()->Packet(&p, sizeof(TPacketGCTarget));
	}
}

void CHARACTER::CheckTarget()
{
	if (!m_pkChrTarget)
		return;

	if (DISTANCE_APPROX(GetX() - m_pkChrTarget->GetX(), GetY() - m_pkChrTarget->GetY()) >= 4800)
		SetTarget(nullptr);
}

void CHARACTER::SetWarpLocation(int32_t lMapIndex, int32_t x, int32_t y)
{
	m_posWarp.x = x * 100;
	m_posWarp.y = y * 100;
	m_lWarpMapIndex = lMapIndex;
}

void CHARACTER::SaveExitLocation()
{
	m_posExit = GetXYZ();
	m_lExitMapIndex = GetMapIndex();
}

void CHARACTER::ExitToSavedLocation()
{
	sys_log (0, "ExitToSavedLocation");
	WarpSet(m_posWarp.x, m_posWarp.y, m_lWarpMapIndex);

	m_posExit.x = m_posExit.y = m_posExit.z = 0;
	m_lExitMapIndex = 0;
}

bool CHARACTER::WarpSet(int32_t x, int32_t y, int32_t lPrivateMapIndex)
{
	if (!IsPC())
		return false;

	int32_t lAddr;
	int32_t lMapIndex;
	uint16_t wPort;

	if (!CMapLocation::instance().Get(x, y, lMapIndex, lAddr, wPort))
	{
		sys_err("cannot find map location index %d x %d y %d name %s", lMapIndex, x, y, GetName());
		return false;
	}

	{
		int32_t lCurAddr;
		int32_t lCurMapIndex = 0;
		uint16_t wCurPort;

		CMapLocation::instance().Get(GetX(), GetY(), lCurMapIndex, lCurAddr, wCurPort);

		if( lCurMapIndex != lMapIndex )
		{
			const TMapRegion * rMapRgn = SECTREE_MANAGER::instance().GetMapRegion(lMapIndex);
			{
				DESC_MANAGER::instance().SendClientPackageSDBToLoadMap( GetDesc(), rMapRgn->strMapName.c_str() );
			}
		}
	}

	if (lPrivateMapIndex >= 10000)
	{
		if (lPrivateMapIndex / 10000 != lMapIndex)
		{
			sys_err("Invalid map index %d, must be child of %d", lPrivateMapIndex, lMapIndex);
			return false;
		}

		lMapIndex = lPrivateMapIndex;
	}

	Stop();
	Save();

	if (GetSectree())
	{
		GetSectree()->RemoveEntity(this);
		ViewCleanup();

		EncodeRemovePacket(this);
	}

	m_lWarpMapIndex = lMapIndex;
	m_posWarp.x = x;
	m_posWarp.y = y;

	sys_log(0, "WarpSet %s %d %d current map %d target map %d", GetName(), x, y, GetMapIndex(), lMapIndex);

	TPacketGCWarp p;

	p.bHeader	= HEADER_GC_WARP;
	p.lX	= x;
	p.lY	= y;
	p.lAddr	= lAddr;
// @correction123 BEGIN
	if (!g_stProxyIP.empty())
		p.lAddr = inet_addr(g_stProxyIP.c_str());
// @correction123 END
	p.wPort	= wPort;

#ifdef __SWITCHBOT__
	CSwitchbotManager::Instance().SetIsWarping(GetPlayerID(), true);

	if (p.wPort != mother_port)
	{
		CSwitchbotManager::Instance().P2PSendSwitchbot(GetPlayerID(), p.wPort);
	}
#endif

	GetDesc()->Packet(&p, sizeof(TPacketGCWarp));

	char buf[256];
	snprintf(buf, sizeof(buf), "%s MapIdx %d DestMapIdx%d DestX%d DestY%d Empire%d", GetName(), GetMapIndex(), lPrivateMapIndex, x, y, GetEmpire());
	LogManager::instance().CharLog(this, 0, "WARP", buf);

	return true;
}

void CHARACTER::WarpEnd()
{
	if (test_server)
		sys_log(0, "WarpEnd %s", GetName());

	if (m_posWarp.x == 0 && m_posWarp.y == 0)
		return;

	int32_t index = m_lWarpMapIndex;

	if (index > 10000)
		index /= 10000;

	if (!map_allow_find(index))
	{
		sys_err("location %d %d not allowed to login this server", m_posWarp.x, m_posWarp.y);
#ifdef ENABLE_GOHOME_IF_MAP_NOT_ALLOWED // @correction173
		GoHome();
#else
		GetDesc()->SetPhase(PHASE_CLOSE);
#endif
		return;
	}

	sys_log(0, "WarpEnd %s %d %u %u", GetName(), m_lWarpMapIndex, m_posWarp.x, m_posWarp.y);

	Show(m_lWarpMapIndex, m_posWarp.x, m_posWarp.y, 0);
	Stop();

	m_lWarpMapIndex = 0;
	m_posWarp.x = m_posWarp.y = m_posWarp.z = 0;

	{
		TPacketGGLogin p;

		p.bHeader = HEADER_GG_LOGIN;
		strlcpy(p.szName, GetName(), sizeof(p.szName));
		p.dwPID = GetPlayerID();
		p.bEmpire = GetEmpire();
		p.lMapIndex = SECTREE_MANAGER::instance().GetMapIndex(GetX(), GetY());
		p.bChannel = g_bChannel;

		P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGLogin));
	}
}

bool CHARACTER::Return()
{
	if (!IsNPC())
		return false;

	int32_t x, y;
	SetVictim(nullptr);

	x = m_pkMobInst->m_posLastAttacked.x;
	y = m_pkMobInst->m_posLastAttacked.y;

	SetRotationToXY(x, y);

	if (!Goto(x, y))
		return false;

	SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	if (test_server)
		sys_log(0, "%s %p 포기하고 돌아가자! %d %d", GetName(), this, x, y);

	if (GetParty())
		GetParty()->SendMessage(this, PM_RETURN, x, y);

	return true;
}

bool CHARACTER::Follow(LPCHARACTER pkChr, float fMinDistance)
{
	if (IsPC())
	{
		sys_err("CHARACTER::Follow : PC cannot use this method", GetName());
		return false;
	}

	if (IS_SET(m_pointsInstant.dwAIFlag, AIFLAG_NOMOVE))
	{
		if (pkChr->IsPC())
		{
			if (!GetParty() || !GetParty()->GetLeader() || GetParty()->GetLeader() == this)
			{
				if (get_dword_time() - m_pkMobInst->m_dwLastAttackedTime >= 15000)
				{
					if (m_pkMobData->m_table.wAttackRange < DISTANCE_APPROX(pkChr->GetX() - GetX(), pkChr->GetY() - GetY()))
						if (Return())
							return true;
				}
			}
		}
		return false;
	}

	int32_t x = pkChr->GetX();
	int32_t y = pkChr->GetY();

	if (pkChr->IsPC())
	{
		if (!GetParty() || !GetParty()->GetLeader() || GetParty()->GetLeader() == this)
		{
			if (get_dword_time() - m_pkMobInst->m_dwLastAttackedTime >= 15000)
			{
				if (5000 < DISTANCE_APPROX(m_pkMobInst->m_posLastAttacked.x - GetX(), m_pkMobInst->m_posLastAttacked.y - GetY()))
					if (Return())
						return true;
			}
		}
	}

	if (IsGuardNPC())
	{
		if (5000 < DISTANCE_APPROX(m_pkMobInst->m_posLastAttacked.x - GetX(), m_pkMobInst->m_posLastAttacked.y - GetY()))
			if (Return())
				return true;
	}

	if (pkChr->IsState(pkChr->m_stateMove) &&
		GetMobBattleType() != BATTLE_TYPE_RANGE &&
		GetMobBattleType() != BATTLE_TYPE_MAGIC &&
		false == IsPet()
#ifdef __MOUNT__
&& false == IsMountSystem()
#endif
#ifdef __GROWTH_PET__
&& false == IsNewPet()
#endif
)
	{
		float rot = pkChr->GetRotation();
		float rot_delta = GetDegreeDelta(rot, GetDegreeFromPositionXY(GetX(), GetY(), pkChr->GetX(), pkChr->GetY()));

		float yourSpeed = pkChr->GetMoveSpeed();
		float mySpeed = GetMoveSpeed();

		float fDist = DISTANCE_SQRT(x - GetX(), y - GetY());
		float fFollowSpeed = mySpeed - yourSpeed * cos(rot_delta * M_PI / 180);

		if (fFollowSpeed >= 0.1f)
		{
			float fMeetTime = fDist / fFollowSpeed;
			float fYourMoveEstimateX, fYourMoveEstimateY;

			if( fMeetTime * yourSpeed <= 100000.0f )
			{
				GetDeltaByDegree(pkChr->GetRotation(), fMeetTime * yourSpeed, &fYourMoveEstimateX, &fYourMoveEstimateY);

				x += (int32_t) fYourMoveEstimateX;
				y += (int32_t) fYourMoveEstimateY;

				float fDistNew = sqrt(((double)x - GetX())*(x-GetX())+((double)y - GetY())*(y-GetY()));
				if (fDist < fDistNew)
				{
					x = (int32_t)(GetX() + (x - GetX()) * fDist / fDistNew);
					y = (int32_t)(GetY() + (y - GetY()) * fDist / fDistNew);
				}
			}
		}
	}

	SetRotationToXY(x, y);

	float fDist = DISTANCE_SQRT(x - GetX(), y - GetY());

	if (fDist <= fMinDistance)
		return false;

	float fx, fy;

	if (IsChangeAttackPosition(pkChr) && GetMobRank() < MOB_RANK_BOSS)
	{
		SetChangeAttackPositionTime();

		int32_t retry = 16;
		int32_t dx, dy;
		int32_t rot = (int32_t) GetDegreeFromPositionXY(x, y, GetX(), GetY());

		while (--retry)
		{
			if (fDist < 500.0f)
				GetDeltaByDegree((rot + number(-90, 90) + number(-90, 90)) % 360, fMinDistance, &fx, &fy);
			else
				GetDeltaByDegree(number(0, 359), fMinDistance, &fx, &fy);

			dx = x + (int32_t) fx;
			dy = y + (int32_t) fy;

			LPSECTREE tree = SECTREE_MANAGER::instance().Get(GetMapIndex(), dx, dy);

			if (nullptr == tree)
				break;

			if (0 == (tree->GetAttribute(dx, dy) & (ATTR_BLOCK | ATTR_OBJECT)))
				break;
		}

		if (!Goto(dx, dy))
			return false;
	}
	else
	{
		float fDistToGo = fDist - fMinDistance;
		GetDeltaByDegree(GetRotation(), fDistToGo, &fx, &fy);

		if (!Goto(GetX() + (int32_t) fx, GetY() + (int32_t) fy))
			return false;
	}

	SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);
	return true;
}

float CHARACTER::GetDistanceFromSafeboxOpen() const
{
	return DISTANCE_APPROX(GetX() - m_posSafeboxOpen.x, GetY() - m_posSafeboxOpen.y);
}

void CHARACTER::SetSafeboxOpenPosition()
{
	m_posSafeboxOpen = GetXYZ();
}

CSafebox * CHARACTER::GetSafebox() const
{
	return m_pkSafebox;
}

void CHARACTER::ReqSafeboxLoad(const char* pszPassword)
{
	if (!*pszPassword || strlen(pszPassword) > SAFEBOX_PASSWORD_MAX_LEN)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 잘못된 암호를 입력하셨습니다."));
		return;
	}
	else if (m_pkSafebox)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 창고가 이미 열려있습니다."));
		return;
	}

	int32_t iPulse = thecore_pulse();

	if (iPulse - GetSafeboxLoadTime()  < PASSES_PER_SEC(10))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 창고를 닫은지 10초 안에는 열 수 없습니다."));
		return;
	}
	else if (GetDistanceFromSafeboxOpen() > 1000)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("<창고> 거리가 멀어서 창고를 열 수 없습니다."));
		return;
	}
	else if (m_bOpeningSafebox)
	{
		sys_log(0, "Overlapped safebox load request from %s", GetName());
		return;
	}

	SetSafeboxLoadTime();
	m_bOpeningSafebox = true;

	TSafeboxLoadPacket p;
	p.dwID = GetDesc()->GetAccountTable().id;
	strlcpy(p.szLogin, GetDesc()->GetAccountTable().login, sizeof(p.szLogin));
	strlcpy(p.szPassword, pszPassword, sizeof(p.szPassword));

	db_clientdesc->DBPacket(HEADER_GD_SAFEBOX_LOAD, GetDesc()->GetHandle(), &p, sizeof(p));
}

void CHARACTER::LoadSafebox(int32_t iSize, uint32_t dwGold, int32_t iItemCount, TPlayerItem * pItems)
{
	bool bLoaded = false;

	SetOpenSafebox(true);

	if (m_pkSafebox)
		bLoaded = true;

	if (!m_pkSafebox)
		m_pkSafebox = M2_NEW CSafebox(this, iSize, dwGold);
	else
		m_pkSafebox->ChangeSize(iSize);

	m_iSafeboxSize = iSize;

	TPacketCGSafeboxSize p;

	p.bHeader = HEADER_GC_SAFEBOX_SIZE;
	p.bSize = iSize;

	GetDesc()->Packet(&p, sizeof(TPacketCGSafeboxSize));

	if (!bLoaded)
	{
		for (int32_t i = 0; i < iItemCount; ++i, ++pItems)
		{
			if (!m_pkSafebox->IsValidPosition(pItems->pos))
				continue;

			LPITEM item = ITEM_MANAGER::instance().CreateItem(pItems->vnum, pItems->count, pItems->id);

			if (!item)
			{
				sys_err("cannot create item vnum %d id %u (name: %s)", pItems->vnum, pItems->id, GetName());
				continue;
			}

			item->SetSkipSave(true);
			item->SetSockets(pItems->alSockets);
			item->SetAttributes(pItems->aAttr);

#ifdef __CHANGE_LOOK__ // @correction192
			item->SetChangeLook(pItems->changelook);
#endif

			if (!m_pkSafebox->Add(pItems->pos, item))
			{
				M2_DESTROY_ITEM(item);
			}
			else
				item->SetSkipSave(false);
		}
	}
}

void CHARACTER::ChangeSafeboxSize(uint8_t bSize)
{
	TPacketCGSafeboxSize p;

	p.bHeader = HEADER_GC_SAFEBOX_SIZE;
	p.bSize = bSize;

	GetDesc()->Packet(&p, sizeof(TPacketCGSafeboxSize));

	if (m_pkSafebox)
		m_pkSafebox->ChangeSize(bSize);

	m_iSafeboxSize = bSize;
}

void CHARACTER::CloseSafebox()
{
	if (!m_pkSafebox)
		return;

	SetOpenSafebox(false);

	m_pkSafebox->Save();

	M2_DELETE(m_pkSafebox);
	m_pkSafebox = nullptr;

	ChatPacket(CHAT_TYPE_COMMAND, "CloseSafebox");

	SetSafeboxLoadTime();
	m_bOpeningSafebox = false;

	Save();
}

CSafebox * CHARACTER::GetMall() const
{
	return m_pkMall;
}

void CHARACTER::LoadMall(int32_t iItemCount, TPlayerItem * pItems)
{
	bool bLoaded = false;

	if (m_pkMall)
		bLoaded = true;

	if (!m_pkMall)
		m_pkMall = M2_NEW CSafebox(this, 3 * SAFEBOX_PAGE_SIZE, 0);
	else
		m_pkMall->ChangeSize(3 * SAFEBOX_PAGE_SIZE);

	m_pkMall->SetWindowMode(MALL);

	TPacketCGSafeboxSize p;

	p.bHeader = HEADER_GC_MALL_OPEN;
	p.bSize = 3 * SAFEBOX_PAGE_SIZE;

	GetDesc()->Packet(&p, sizeof(TPacketCGSafeboxSize));

	if (!bLoaded)
	{
		for (int32_t i = 0; i < iItemCount; ++i, ++pItems)
		{
			if (!m_pkMall->IsValidPosition(pItems->pos))
				continue;

			LPITEM item = ITEM_MANAGER::instance().CreateItem(pItems->vnum, pItems->count, pItems->id);

			if (!item)
			{
				sys_err("cannot create item vnum %d id %u (name: %s)", pItems->vnum, pItems->id, GetName());
				continue;
			}

			item->SetSkipSave(true);
			item->SetSockets(pItems->alSockets);
			item->SetAttributes(pItems->aAttr);

#ifdef __CHANGE_LOOK__ // @correction192
			item->SetChangeLook(pItems->changelook);
#endif

			if (!m_pkMall->Add(pItems->pos, item))
				M2_DESTROY_ITEM(item);
			else
				item->SetSkipSave(false);
		}
	}
}

void CHARACTER::CloseMall()
{
	if (!m_pkMall)
		return;

	m_pkMall->Save();

	M2_DELETE(m_pkMall);
	m_pkMall = nullptr;

	ChatPacket(CHAT_TYPE_COMMAND, "CloseMall");
}

bool CHARACTER::BuildUpdatePartyPacket(TPacketGCPartyUpdate & out)
{
	if (!GetParty())
		return false;

	memset(&out, 0, sizeof(out));

	out.header		= HEADER_GC_PARTY_UPDATE;
	out.pid		= GetPlayerID();
	if (GetMaxHP() <= 0) // @correction056
		out.percent_hp	= 0;
	else
		out.percent_hp	= MINMAX(0, GetHP() * 100 / GetMaxHP(), 100);
	out.role		= GetParty()->GetRole(GetPlayerID());

	sys_log(1, "PARTY %s role is %d", GetName(), out.role);

	LPCHARACTER l = GetParty()->GetLeaderCharacter();

	if (l && DISTANCE_APPROX(GetX() - l->GetX(), GetY() - l->GetY()) < PARTY_DEFAULT_RANGE)
	{
		out.affects[0] = GetParty()->GetPartyBonusExpPercent();
		out.affects[1] = GetPoint(POINT_PARTY_ATTACKER_BONUS);
		out.affects[2] = GetPoint(POINT_PARTY_TANKER_BONUS);
		out.affects[3] = GetPoint(POINT_PARTY_BUFFER_BONUS);
		out.affects[4] = GetPoint(POINT_PARTY_SKILL_MASTER_BONUS);
		out.affects[5] = GetPoint(POINT_PARTY_HASTE_BONUS);
		out.affects[6] = GetPoint(POINT_PARTY_DEFENDER_BONUS);
	}

	return true;
}

int32_t CHARACTER::GetLeadershipSkillLevel() const
{
	return GetSkillLevel(SKILL_LEADERSHIP);
}

void CHARACTER::QuerySafeboxSize()
{
	if (m_iSafeboxSize == -1)
	{
		DBManager::instance().ReturnQuery(QID_SAFEBOX_SIZE,
				GetPlayerID(),
				nullptr,
				"SELECT size FROM safebox%s WHERE account_id = %u",
				get_table_postfix(),
				GetDesc()->GetAccountTable().id);
	}
}

void CHARACTER::SetSafeboxSize(int32_t iSize)
{
	sys_log(1, "SetSafeboxSize: %s %d", GetName(), iSize);
	m_iSafeboxSize = iSize;
	DBManager::instance().Query("UPDATE safebox%s SET size = %d WHERE account_id = %u", get_table_postfix(), iSize / SAFEBOX_PAGE_SIZE, GetDesc()->GetAccountTable().id);
}

int32_t CHARACTER::GetSafeboxSize() const
{
	return m_iSafeboxSize;
}

void CHARACTER::SetNowWalking(bool bWalkFlag)
{
	if (m_bNowWalking != bWalkFlag)
	{
		if (bWalkFlag)
		{
			m_bNowWalking = true;
			m_dwWalkStartTime = get_dword_time();
		}
		else
		{
			m_bNowWalking = false;
		}

		{
			TPacketGCWalkMode p;
			p.vid = GetVID();
			p.header = HEADER_GC_WALK_MODE;
			p.mode = m_bNowWalking ? WALKMODE_WALK : WALKMODE_RUN;

			PacketView(&p, sizeof(p));
		}

		if (IsNPC())
		{
			if (m_bNowWalking)
				MonsterLog("걷는다");
			else
				MonsterLog("뛴다");
		}

	}
}

void CHARACTER::StartStaminaConsume()
{
	if (m_bStaminaConsume)
		return;
	PointChange(POINT_STAMINA, 0);
	m_bStaminaConsume = true;
	if (IsStaminaHalfConsume())
		ChatPacket(CHAT_TYPE_COMMAND, "StartStaminaConsume %d %d", STAMINA_PER_STEP * passes_per_sec / 2, GetStamina());
	else
		ChatPacket(CHAT_TYPE_COMMAND, "StartStaminaConsume %d %d", STAMINA_PER_STEP * passes_per_sec, GetStamina());
}

void CHARACTER::StopStaminaConsume()
{
	if (!m_bStaminaConsume)
		return;
	PointChange(POINT_STAMINA, 0);
	m_bStaminaConsume = false;
	ChatPacket(CHAT_TYPE_COMMAND, "StopStaminaConsume %d", GetStamina());
}

bool CHARACTER::IsStaminaConsume() const
{
	return m_bStaminaConsume;
}

bool CHARACTER::IsStaminaHalfConsume() const
{
	return IsEquipUniqueItem(UNIQUE_ITEM_HALF_STAMINA);
}

void CHARACTER::ResetStopTime()
{
	m_dwStopTime = get_dword_time();
}

uint32_t CHARACTER::GetStopTime() const
{
	return m_dwStopTime;
}

void CHARACTER::ResetPoint(int32_t iLv)
{
	uint8_t bJob = GetJob();

	PointChange(POINT_LEVEL, iLv - GetLevel());

	SetRealPoint(POINT_ST, JobInitialPoints[bJob].st);
	SetPoint(POINT_ST, GetRealPoint(POINT_ST));

	SetRealPoint(POINT_HT, JobInitialPoints[bJob].ht);
	SetPoint(POINT_HT, GetRealPoint(POINT_HT));

	SetRealPoint(POINT_DX, JobInitialPoints[bJob].dx);
	SetPoint(POINT_DX, GetRealPoint(POINT_DX));

	SetRealPoint(POINT_IQ, JobInitialPoints[bJob].iq);
	SetPoint(POINT_IQ, GetRealPoint(POINT_IQ));

	SetRandomHP((iLv - 1) * number(JobInitialPoints[GetJob()].hp_per_lv_begin, JobInitialPoints[GetJob()].hp_per_lv_end));
	SetRandomSP((iLv - 1) * number(JobInitialPoints[GetJob()].sp_per_lv_begin, JobInitialPoints[GetJob()].sp_per_lv_end));

	// @correction031
	PointChange(POINT_STAT, (MINMAX(1, iLv, g_iStatusPointGetLevelLimit) * 3) + GetPoint(POINT_LEVEL_STEP) - GetPoint(POINT_STAT)); // @correction217

	ComputePoints();

	PointChange(POINT_HP, GetMaxHP() - GetHP());
	PointChange(POINT_SP, GetMaxSP() - GetSP());

	PointsPacket();

	LogManager::instance().CharLog(this, 0, "RESET_POINT", "");
}

bool CHARACTER::IsChangeAttackPosition(LPCHARACTER target) const
{
	if (!IsNPC())
		return true;

	uint32_t dwChangeTime = AI_CHANGE_ATTACK_POISITION_TIME_NEAR;

	if (DISTANCE_APPROX(GetX() - target->GetX(), GetY() - target->GetY()) >
		AI_CHANGE_ATTACK_POISITION_DISTANCE + GetMobAttackRange())
		dwChangeTime = AI_CHANGE_ATTACK_POISITION_TIME_FAR;

	return get_dword_time() - m_dwLastChangeAttackPositionTime > dwChangeTime;
}

void CHARACTER::GiveRandomSkillBook()
{
	LPITEM item = AutoGiveItem(50300);

	if (nullptr != item)
	{
// @correction124 BEGIN
		extern const uint32_t GetRandomSkillVnum(uint8_t bJob = JOB_MAX_NUM);
		uint32_t dwSkillVnum = 0;
		if (!number(0, 1))
			dwSkillVnum = GetRandomSkillVnum(GetJob());
		else
			dwSkillVnum = GetRandomSkillVnum();
// @correction124 END
		item->SetSocket(0, dwSkillVnum);
	}
}

void CHARACTER::ReviveInvisible(int32_t iDur)
{
	AddAffect(AFFECT_REVIVE_INVISIBLE, POINT_NONE, 0, AFF_REVIVE_INVISIBLE, iDur, 0, true);
}

void CHARACTER::ToggleMonsterLog()
{
	m_bMonsterLog = !m_bMonsterLog;

	if (m_bMonsterLog)
	{
		CHARACTER_MANAGER::instance().RegisterForMonsterLog(this);
	}
	else
	{
		CHARACTER_MANAGER::instance().UnregisterForMonsterLog(this);
	}
}

void CHARACTER::SetGuild(CGuild* pGuild)
{
	if (m_pGuild != pGuild)
	{
		m_pGuild = pGuild;
		UpdatePacket();
	}
}

void CHARACTER::SendGreetMessage()
{
	decltype(DBManager::instance().GetGreetMessage()) v = DBManager::instance().GetGreetMessage();

	for (itertype(v) it = v.begin(); it != v.end(); ++it)
	{
		ChatPacket(CHAT_TYPE_NOTICE, it->c_str());
	}
}

void CHARACTER::BeginStateEmpty()
{
	MonsterLog("!");
}

void CHARACTER::EffectPacket(int32_t enumEffectType)
{
	TPacketGCSpecialEffect p;

	p.header = HEADER_GC_SEPCIAL_EFFECT;
	p.type = enumEffectType;
	p.vid = GetVID();

	PacketAround(&p, sizeof(TPacketGCSpecialEffect));
}

void CHARACTER::SpecificEffectPacket(const char filename[MAX_EFFECT_FILE_NAME])
{
	TPacketGCSpecificEffect p;

	p.header = HEADER_GC_SPECIFIC_EFFECT;
	p.vid = GetVID();
	memcpy (p.effect_file, filename, MAX_EFFECT_FILE_NAME);

	PacketAround(&p, sizeof(TPacketGCSpecificEffect));
}

void CHARACTER::MonsterChat(uint8_t bMonsterChatType)
{
	if (IsPC())
		return;

	char sbuf[256+1];

	if (IsMonster())
	{
		if (number(0, 60))
			return;

		snprintf(sbuf, sizeof(sbuf),
				"(locale.monster_chat[%i] and locale.monster_chat[%i][%d] or '')",
				GetRaceNum(), GetRaceNum(), bMonsterChatType*3 + number(1, 3));
	}
	else
	{
		if (bMonsterChatType != MONSTER_CHAT_WAIT)
			return;

		if (IsGuardNPC())
		{
			if (number(0, 6))
				return;
		}
		else
		{
			if (number(0, 30))
				return;
		}

		snprintf(sbuf, sizeof(sbuf), "(locale.monster_chat[%i] and locale.monster_chat[%i][number(1, table.getn(locale.monster_chat[%i]))] or '')", GetRaceNum(), GetRaceNum(), GetRaceNum());
	}

	std::string text = quest::ScriptToString(sbuf);

	if (text.empty())
		return;

	struct packet_chat pack_chat;

	pack_chat.header    = HEADER_GC_CHAT;
	pack_chat.size	= sizeof(struct packet_chat) + text.size() + 1;
	pack_chat.type      = CHAT_TYPE_TALKING;
	pack_chat.id        = GetVID();
	pack_chat.bEmpire	= 0;

	TEMP_BUFFER buf;
	buf.write(&pack_chat, sizeof(struct packet_chat));
	buf.write(text.c_str(), text.size() + 1);

	PacketAround(buf.read_peek(), buf.size());
}

void CHARACTER::SetQuestNPCID(uint32_t vid)
{
	m_dwQuestNPCVID = vid;
}

LPCHARACTER CHARACTER::GetQuestNPC() const
{
	return CHARACTER_MANAGER::instance().Find(m_dwQuestNPCVID);
}

void CHARACTER::SetQuestItemPtr(LPITEM item)
{
	m_pQuestItem = item;
}

void CHARACTER::ClearQuestItemPtr()
{
	m_pQuestItem = nullptr;
}

LPITEM CHARACTER::GetQuestItemPtr() const
{
	return m_pQuestItem;
}

LPDUNGEON CHARACTER::GetDungeonForce() const
{
	if (m_lWarpMapIndex > 10000)
		return CDungeonManager::instance().FindByMapIndex(m_lWarpMapIndex);

	return m_pkDungeon;
}

void CHARACTER::SetBlockMode(uint8_t bFlag)
{
	m_pointsInstant.bBlockMode = bFlag;

	ChatPacket(CHAT_TYPE_COMMAND, "setblockmode %d", m_pointsInstant.bBlockMode);

	SetQuestFlag("game_option.block_exchange", bFlag & BLOCK_EXCHANGE ? 1 : 0);
	SetQuestFlag("game_option.block_party_invite", bFlag & BLOCK_PARTY_INVITE ? 1 : 0);
	SetQuestFlag("game_option.block_guild_invite", bFlag & BLOCK_GUILD_INVITE ? 1 : 0);
	SetQuestFlag("game_option.block_whisper", bFlag & BLOCK_WHISPER ? 1 : 0);
	SetQuestFlag("game_option.block_messenger_invite", bFlag & BLOCK_MESSENGER_INVITE ? 1 : 0);
	SetQuestFlag("game_option.block_party_request", bFlag & BLOCK_PARTY_REQUEST ? 1 : 0);
	SetQuestFlag("game_option.block_view_equipment", bFlag & BLOCK_VIEW_EQUIPMENT ? 1 : 0);
}

void CHARACTER::SetBlockModeForce(uint8_t bFlag)
{
	m_pointsInstant.bBlockMode = bFlag;
	ChatPacket(CHAT_TYPE_COMMAND, "setblockmode %d", m_pointsInstant.bBlockMode);
}

bool CHARACTER::IsGuardNPC() const
{
	return IsNPC() && (GetRaceNum() == 11000 || GetRaceNum() == 11002 || GetRaceNum() == 11004
#ifdef __12ZI__
|| GetRaceNum() == 20464
#endif
);
}

int32_t CHARACTER::GetPolymorphPower() const
{
	if (test_server)
	{
		int32_t value = quest::CQuestManager::instance().GetEventFlag("poly");
		if (value)
			return value;
	}
	return aiPolymorphPowerByLevel[MINMAX(0, GetSkillLevel(SKILL_POLYMORPH), 40)];
}

void CHARACTER::SetPolymorph(uint32_t dwRaceNum, bool bMaintainStat)
{
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	if (dwRaceNum < MAIN_RACE_MAX_NUM)
#else
	if (dwRaceNum < JOB_MAX_NUM)
#endif
	{
		dwRaceNum = 0;
		bMaintainStat = false;
	}

	if (m_dwPolymorphRace == dwRaceNum)
		return;

	m_bPolyMaintainStat = bMaintainStat;
	m_dwPolymorphRace = dwRaceNum;

	sys_log(0, "POLYMORPH: %s race %u ", GetName(), dwRaceNum);

	if (dwRaceNum != 0)
		StopRiding();

	SET_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);
	m_afAffectFlag.Set(AFF_SPAWN);

	ViewReencode();

	REMOVE_BIT(m_bAddChrState, ADD_CHARACTER_STATE_SPAWN);

	if (!bMaintainStat)
	{
		PointChange(POINT_ST, 0);
		PointChange(POINT_DX, 0);
		PointChange(POINT_IQ, 0);
		PointChange(POINT_HT, 0);
	}

	SetValidComboInterval(0);
	SetComboSequence(0);

	ComputeBattlePoints();
}

int32_t CHARACTER::GetQuestFlag(const std::string& flag) const
{
	quest::CQuestManager& q = quest::CQuestManager::instance();
	quest::PC* pPC = q.GetPC(GetPlayerID());

// @correction125 BEGIN
	if (!pPC)
	{
		sys_err("Nullpointer in CHARACTER::GetQuestFlag %lu", GetPlayerID());
		return 0;
	}
// @correction125 END

	return pPC->GetFlag(flag);
}

void CHARACTER::SetQuestFlag(const std::string& flag, int32_t value)
{
	quest::CQuestManager& q = quest::CQuestManager::instance();
	quest::PC* pPC = q.GetPC(GetPlayerID());
	pPC->SetFlag(flag, value);
}

void CHARACTER::DetermineDropMetinStone()
{
// @correction228 BEGIN
	if (g_NoDropMetinStone)
	{
		m_dwDropMetinStone = 0;
		return;
	}
// @correction228 END

	static const uint32_t c_adwMetin[] =
	{
#if defined(ENABLE_WOLFMAN_CHARACTER) && defined(USE_WOLFMAN_STONES) // @correction190
		28012,
#endif
		28030,
		28031,
		28032,
		28033,
		28034,
		28035,
		28036,
		28037,
		28038,
		28039,
		28040,
		28041,
		28042,
		28043,
#ifdef __ANTI_RESIST_MAGIC_REDUCTION__
		28044,
		28045,
#endif
	};
	uint32_t stone_num = GetRaceNum();
	int32_t idx = std::lower_bound(aStoneDrop, aStoneDrop+STONE_INFO_MAX_NUM, stone_num) - aStoneDrop;
	if (idx >= STONE_INFO_MAX_NUM || aStoneDrop[idx].dwMobVnum != stone_num)
	{
		m_dwDropMetinStone = 0;
	}
	else
	{
		const SStoneDropInfo & info = aStoneDrop[idx];
		m_bDropMetinStonePct = info.iDropPct;
		{
			m_dwDropMetinStone = c_adwMetin[number(0, sizeof(c_adwMetin)/sizeof(uint32_t) - 1)];
			int32_t iGradePct = number(1, 100);
			for (int32_t iStoneLevel = 0; iStoneLevel < STONE_LEVEL_MAX_NUM; iStoneLevel ++)
			{
				int32_t iLevelGradePortion = info.iLevelPct[iStoneLevel];
				if (iGradePct <= iLevelGradePortion)
				{
					break;
				}
				else
				{
					iGradePct -= iLevelGradePortion;
					m_dwDropMetinStone += 100;
				}
			}
		}
	}
}

void CHARACTER::SendEquipment(LPCHARACTER ch)
{
	TPacketViewEquip p;
	p.header = HEADER_GC_VIEW_EQUIP;
	p.vid    = GetVID();
	for (int32_t i = 0; i<WEAR_MAX_NUM; i++)
	{
		LPITEM item = GetWear(i);
		if (item)
		{
			p.equips[i].vnum = item->GetVnum();
			p.equips[i].count = item->GetCount();

			memcpy(p.equips[i].alSockets, item->GetSockets(), sizeof(p.equips[i].alSockets));
			memcpy(p.equips[i].aAttr, item->GetAttributes(), sizeof(p.equips[i].aAttr));
		}
		else
		{
			p.equips[i].vnum = 0;
		}
	}
	ch->GetDesc()->Packet(&p, sizeof(p));
}

bool CHARACTER::CanSummon(int32_t iLeaderShip)
{
	return ((iLeaderShip >= 20) || ((iLeaderShip >= 12) && ((m_dwLastDeadTime + 180) > get_dword_time())));
}


void CHARACTER::MountVnum(uint32_t vnum)
{
	if (m_dwMountVnum == vnum)
		return;

	if ((m_dwMountVnum) && vnum) // @correction035
		MountVnum(0);

	m_dwMountVnum = vnum;
	m_dwMountTime = get_dword_time();

	if (m_bIsObserver)
		return;

	m_posDest.x = m_posStart.x = GetX();
	m_posDest.y = m_posStart.y = GetY();
	EncodeInsertPacket(this);

	ENTITY_MAP::iterator it = m_map_view.begin();

	while (it != m_map_view.end())
	{
		LPENTITY entity = (it++)->first;

		EncodeInsertPacket(entity);

	}

	SetValidComboInterval(0);
	SetComboSequence(0);

	ComputePoints();
}

namespace {
	class FuncCheckWarp
	{
		public:
			FuncCheckWarp(LPCHARACTER pkWarp)
			{
				m_lTargetY = 0;
				m_lTargetX = 0;

				m_lX = pkWarp->GetX();
				m_lY = pkWarp->GetY();

				m_bInvalid = false;
				m_bEmpire = pkWarp->GetEmpire();

				char szTmp[64];

				if (3 != sscanf(pkWarp->GetName(), " %s %d %d ", szTmp, &m_lTargetX, &m_lTargetY)
#ifdef __DEFENSE_WAVE__
&& pkWarp->GetRaceNum() != 3949
#endif
)
				{
					if (number(1, 100) < 5)
						sys_err("Warp NPC name wrong : vnum(%d) name(%s)", pkWarp->GetRaceNum(), pkWarp->GetName());

					m_bInvalid = true;

					return;
				}

				m_lTargetX *= 100;
				m_lTargetY *= 100;

				m_bUseWarp = true;

				if (pkWarp->IsGoto())
				{
					LPSECTREE_MAP pkSectreeMap = SECTREE_MANAGER::instance().GetMap(pkWarp->GetMapIndex());
					m_lTargetX += pkSectreeMap->m_setting.iBaseX;
					m_lTargetY += pkSectreeMap->m_setting.iBaseY;
					m_bUseWarp = false;
				}
#ifdef __DEFENSE_WAVE__
				pkWarps = pkWarp;
#endif
			}

			bool Valid()
			{
				return !m_bInvalid;
			}

			void operator () (LPENTITY ent)
			{
				if (!Valid())
					return;

				if (!ent->IsType(ENTITY_CHARACTER))
					return;

				LPCHARACTER pkChr = (LPCHARACTER) ent;

				if (!pkChr->IsPC())
					return;

				int32_t iDist = DISTANCE_APPROX(pkChr->GetX() - m_lX, pkChr->GetY() - m_lY);

				if (iDist > 300)
					return;

				if (m_bEmpire && pkChr->GetEmpire() && m_bEmpire != pkChr->GetEmpire())
					return;

				if (pkChr->IsHack())
					return;

				if (!pkChr->CanHandleItem(false, true))
					return;

				if (m_bUseWarp)
#ifdef __DEFENSE_WAVE__
				{
					if (pkWarps->GetRaceNum() == 3949)
						pkChr->WarpSet(168600, 611200);
					else
						pkChr->WarpSet(m_lTargetX, m_lTargetY);
				}
#else
					pkChr->WarpSet(m_lTargetX, m_lTargetY);
#endif
				else
				{
					pkChr->Show(pkChr->GetMapIndex(), m_lTargetX, m_lTargetY);
					pkChr->Stop();
				}
			}

			bool m_bInvalid;
			bool m_bUseWarp;

			int32_t m_lX;
			int32_t m_lY;
			int32_t m_lTargetX;
			int32_t m_lTargetY;

			uint8_t m_bEmpire;
#ifdef __DEFENSE_WAVE__
			LPCHARACTER pkWarps;
#endif
	};
}

EVENTFUNC(warp_npc_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );
	if ( info == nullptr )
	{
		sys_err( "warp_npc_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;

	if (ch == nullptr) {
		return 0;
	}

	if (!ch->GetSectree())
	{
		ch->m_pkWarpNPCEvent = nullptr;
		return 0;
	}

	FuncCheckWarp f(ch);
	if (f.Valid())
		ch->GetSectree()->ForEachAround(f);

	return passes_per_sec / 2;
}


void CHARACTER::StartWarpNPCEvent()
{
	if (m_pkWarpNPCEvent)
		return;

	if (!IsWarp() && !IsGoto()
#ifdef __DEFENSE_WAVE__
		&& GetRaceNum() != 3949
#endif
)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	m_pkWarpNPCEvent = event_create(warp_npc_event, info, passes_per_sec / 2);
}

void CHARACTER::SyncPacket()
{
	TEMP_BUFFER buf;

	TPacketCGSyncPositionElement elem;

	elem.dwVID = GetVID();
	elem.lX = GetX();
	elem.lY = GetY();

	TPacketGCSyncPosition pack;

	pack.bHeader = HEADER_GC_SYNC_POSITION;
	pack.wSize = sizeof(TPacketGCSyncPosition) + sizeof(elem);

	buf.write(&pack, sizeof(pack));
	buf.write(&elem, sizeof(elem));

	PacketAround(buf.read_peek(), buf.size());
}

LPCHARACTER CHARACTER::GetMarryPartner() const
{
	return m_pkChrMarried;
}

void CHARACTER::SetMarryPartner(LPCHARACTER ch)
{
	m_pkChrMarried = ch;
}

int32_t CHARACTER::GetMarriageBonus(uint32_t dwItemVnum, bool bSum)
{
	if (IsNPC())
		return 0;

	marriage::TMarriage* pMarriage = marriage::CManager::instance().Get(GetPlayerID());

	if (!pMarriage)
		return 0;

	return pMarriage->GetBonus(dwItemVnum, bSum, this);
}

void CHARACTER::ConfirmWithMsg(const char* szMsg, int32_t iTimeout, uint32_t dwRequestPID)
{
	if (!IsPC())
		return;

	TPacketGCQuestConfirm p;

	p.header = HEADER_GC_QUEST_CONFIRM;
	p.requestPID = dwRequestPID;
	p.timeout = iTimeout;
	strlcpy(p.msg, szMsg, sizeof(p.msg));

	GetDesc()->Packet(&p, sizeof(p));
}

int32_t CHARACTER::GetPremiumRemainSeconds(uint8_t bType) const
{
	if (bType >= PREMIUM_MAX_NUM)
		return 0;

	return m_aiPremiumTimes[bType] - get_global_time();
}

bool CHARACTER::WarpToPID(uint32_t dwPID)
{
	LPCHARACTER victim;
	if ((victim = (CHARACTER_MANAGER::instance().FindByPID(dwPID))))
	{
		int32_t mapIdx = victim->GetMapIndex();
		if (IS_SUMMONABLE_ZONE(mapIdx))
		{
			if (CAN_ENTER_ZONE(this, mapIdx))
			{
				WarpSet(victim->GetX(), victim->GetY());
			}
			else
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 있는 곳으로 워프할 수 없습니다."));
				return false;
			}
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 있는 곳으로 워프할 수 없습니다."));
			return false;
		}
	}
	else
	{
		CCI * pcci = P2P_MANAGER::instance().FindByPID(dwPID);

		if (!pcci)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 온라인 상태가 아닙니다."));
			return false;
		}

		if (pcci->bChannel != g_bChannel)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 %d 채널에 있습니다. (현재 채널 %d)"), pcci->bChannel, g_bChannel);
			return false;
		}
		else if (false == IS_SUMMONABLE_ZONE(pcci->lMapIndex))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 있는 곳으로 워프할 수 없습니다."));
			return false;
		}
		else
		{
			if (!CAN_ENTER_ZONE(this, pcci->lMapIndex))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상대방이 있는 곳으로 워프할 수 없습니다."));
				return false;
			}

			TPacketGGFindPosition p;
			p.header = HEADER_GG_FIND_POSITION;
			p.dwFromPID = GetPlayerID();
			p.dwTargetPID = dwPID;
			pcci->pkDesc->Packet(&p, sizeof(TPacketGGFindPosition));

			if (test_server)
				ChatPacket(CHAT_TYPE_PARTY, "sent find position packet for teleport");
		}
	}
	return true;
}

CGuild* CHARACTER::GetRefineGuild() const
{
	LPCHARACTER chRefineNPC = CHARACTER_MANAGER::instance().Find(m_dwRefineNPCVID);

	return (chRefineNPC ? chRefineNPC->GetGuild() : nullptr);
}

bool CHARACTER::IsRefineThroughGuild() const
{
	return GetRefineGuild() != nullptr;
}

uint64_t CHARACTER::ComputeRefineFee(uint64_t iCost, int32_t iMultiply) const
{
	CGuild* pGuild = GetRefineGuild();
	if (pGuild)
	{
		if (pGuild == GetGuild())
			return iCost * iMultiply * 9 / 10;

		LPCHARACTER chRefineNPC = CHARACTER_MANAGER::instance().Find(m_dwRefineNPCVID);
		if (chRefineNPC && chRefineNPC->GetEmpire() != GetEmpire())
			return iCost * iMultiply * 3;

		return iCost * iMultiply;
	}
	else
		return iCost;
}

void CHARACTER::PayRefineFee(uint64_t iTotalMoney)
{
	uint64_t iFee = iTotalMoney / 10;
	CGuild* pGuild = GetRefineGuild();

	uint64_t iRemain = iTotalMoney;

	if (pGuild)
	{
		if (pGuild != GetGuild())
		{
			pGuild->RequestDepositMoney(this, iFee);
			iRemain -= iFee;
		}
	}

	ChangeGold(false, iRemain);
}

bool CHARACTER::IsHack(bool bSendMsg, bool bCheckShopOwner, int32_t limittime)
{
	const int32_t iPulse = thecore_pulse();

	if (test_server)
		bSendMsg = true;

	if (iPulse - GetSafeboxLoadTime() < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("창고를 연후 %d초 이내에는 다른곳으로 이동할수 없습니다."), limittime);

		if (test_server)
			ChatPacket(CHAT_TYPE_INFO, "[TestOnly]Pulse %d LoadTime %d PASS %d", iPulse, GetSafeboxLoadTime(), PASSES_PER_SEC(limittime));
		return true;
	}

	if (bCheckShopOwner)
	{
		if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen())
		{
			if (bSendMsg)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("거래창,창고 등을 연 상태에서는 다른곳으로 이동,종료 할수 없습니다"));

			return true;
		}
	}
	else
	{
		if (GetExchange() || GetMyShop() || IsOpenSafebox() || IsCubeOpen())
		{
			if (bSendMsg)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("거래창,창고 등을 연 상태에서는 다른곳으로 이동,종료 할수 없습니다"));

			return true;
		}
	}

	if (iPulse - GetExchangeTime()  < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("거래 후 %d초 이내에는 다른지역으로 이동 할 수 없습니다."), limittime );
		return true;
	}

	if (iPulse - GetMyShopTime() < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("거래 후 %d초 이내에는 다른지역으로 이동 할 수 없습니다."), limittime);
		return true;
	}

	if (iPulse - GetRefineTime() < PASSES_PER_SEC(limittime))
	{
		if (bSendMsg)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아이템 개량후 %d초 이내에는 귀환부,귀환기억부를 사용할 수 없습니다."), limittime);
		return true;
	}

	return false;
}

bool CHARACTER::IsMonarch() const
{
	if (CMonarch::instance().IsMonarch(GetPlayerID(), GetEmpire()))
		return true;

	return false;

}
void CHARACTER::Say(const std::string & s)
{
	struct ::packet_script packet_script;

	packet_script.header = HEADER_GC_SCRIPT;
	packet_script.skin = 1;
	packet_script.src_size = s.size();
	packet_script.size = packet_script.src_size + sizeof(struct packet_script);

	TEMP_BUFFER buf;

	buf.write(&packet_script, sizeof(struct packet_script));
	buf.write(&s[0], s.size());

	if (IsPC())
	{
		GetDesc()->Packet(buf.read_peek(), buf.size());
	}
}

void CHARACTER::InitMC()
{
	for (int32_t n = 0; n < MI_MAX; ++n)
	{
		m_dwMonarchCooltime[n] = thecore_pulse();
	}

	m_dwMonarchCooltimelimit[MI_HEAL] = PASSES_PER_SEC(MC_HEAL);
	m_dwMonarchCooltimelimit[MI_WARP] = PASSES_PER_SEC(MC_WARP);
	m_dwMonarchCooltimelimit[MI_TRANSFER] = PASSES_PER_SEC(MC_TRANSFER);
	m_dwMonarchCooltimelimit[MI_TAX] = PASSES_PER_SEC(MC_TAX);
	m_dwMonarchCooltimelimit[MI_SUMMON] = PASSES_PER_SEC(MC_SUMMON);

	m_dwMonarchCooltime[MI_HEAL] -= PASSES_PER_SEC(GetMCL(MI_HEAL));
	m_dwMonarchCooltime[MI_WARP] -= PASSES_PER_SEC(GetMCL(MI_WARP));
	m_dwMonarchCooltime[MI_TRANSFER] -= PASSES_PER_SEC(GetMCL(MI_TRANSFER));
	m_dwMonarchCooltime[MI_TAX] -= PASSES_PER_SEC(GetMCL(MI_TAX));
	m_dwMonarchCooltime[MI_SUMMON] -= PASSES_PER_SEC(GetMCL(MI_SUMMON));
}

uint32_t CHARACTER::GetMC(enum MONARCH_INDEX e) const
{
	return m_dwMonarchCooltime[e];
}

void CHARACTER::SetMC(enum MONARCH_INDEX e)
{
	m_dwMonarchCooltime[e] = thecore_pulse();
}

bool CHARACTER::IsMCOK(enum MONARCH_INDEX e) const
{
	int32_t iPulse = thecore_pulse();

	if ((iPulse -  GetMC(e)) <  GetMCL(e))
	{
		if (test_server)
			sys_log(0, " Pulse %d cooltime %d, limit %d", iPulse, GetMC(e), GetMCL(e));

		return false;
	}

	if (test_server)
		sys_log(0, " Pulse %d cooltime %d, limit %d", iPulse, GetMC(e), GetMCL(e));

	return true;
}

uint32_t CHARACTER::GetMCL(enum MONARCH_INDEX e) const
{
	return m_dwMonarchCooltimelimit[e];
}

uint32_t CHARACTER::GetMCLTime(enum MONARCH_INDEX e) const
{
	int32_t iPulse = thecore_pulse();

	if (test_server)
		sys_log(0, " Pulse %d cooltime %d, limit %d", iPulse, GetMC(e), GetMCL(e));

	return  (GetMCL(e)) / passes_per_sec   -  (iPulse - GetMC(e)) / passes_per_sec;
}

bool CHARACTER::IsSiegeNPC() const
{
	return IsNPC() && (GetRaceNum() == 11000 || GetRaceNum() == 11002 || GetRaceNum() == 11004);
}

void CHARACTER::UpdateDepositPulse()
{
	m_deposit_pulse = thecore_pulse() + PASSES_PER_SEC(60*5);
}

bool CHARACTER::CanDeposit() const
{
	return (m_deposit_pulse == 0 || (m_deposit_pulse < thecore_pulse()));
}

ESex GET_SEX(LPCHARACTER ch)
{
	switch (ch->GetRaceNum())
	{
		case MAIN_RACE_WARRIOR_M:
		case MAIN_RACE_SURA_M:
		case MAIN_RACE_ASSASSIN_M:
		case MAIN_RACE_SHAMAN_M:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case MAIN_RACE_WOLFMAN_M:
#endif
			return SEX_MALE;

		case MAIN_RACE_ASSASSIN_W:
		case MAIN_RACE_SHAMAN_W:
		case MAIN_RACE_WARRIOR_W:
		case MAIN_RACE_SURA_W:
			return SEX_FEMALE;
	}

	return SEX_MALE;
}

int32_t CHARACTER::GetHPPct() const
{
	if (GetMaxHP() <= 0) // @correction056
		return 0;
	return (GetHP() * 100) / GetMaxHP();
}

bool CHARACTER::IsBerserk() const
{
	if (m_pkMobInst != nullptr)
		return m_pkMobInst->m_IsBerserk;
	else
		return false;
}

void CHARACTER::SetBerserk(bool mode)
{
	if (m_pkMobInst != nullptr)
		m_pkMobInst->m_IsBerserk = mode;
}

bool CHARACTER::IsGodSpeed() const
{
	if (m_pkMobInst != nullptr)
	{
		return m_pkMobInst->m_IsGodSpeed;
	}
	else
	{
		return false;
	}
}

void CHARACTER::SetGodSpeed(bool mode)
{
	if (m_pkMobInst != nullptr)
	{
		m_pkMobInst->m_IsGodSpeed = mode;

		if (mode == true)
		{
			SetPoint(POINT_ATT_SPEED, 250);
		}
		else
		{
			SetPoint(POINT_ATT_SPEED, m_pkMobData->m_table.sAttackSpeed);
		}
	}
}

bool CHARACTER::IsDeathBlow() const
{
	if (number(1, 100) <= m_pkMobData->m_table.bDeathBlowPoint)
	{
		return true;
	}
	else
	{
		return false;
	}
}

struct FFindReviver
{
	FFindReviver()
	{
		pChar = nullptr;
		HasReviver = false;
	}

	void operator() (LPCHARACTER ch)
	{
		if (ch->IsMonster() != true)
		{
			return;
		}

		if (ch->IsReviver() == true && pChar != ch && ch->IsDead() != true)
		{
			if (number(1, 100) <= ch->GetMobTable().bRevivePoint)
			{
				HasReviver = true;
				pChar = ch;
			}
		}
	}

	LPCHARACTER pChar;
	bool HasReviver;
};

bool CHARACTER::HasReviverInParty() const
{
	LPPARTY party = GetParty();

	if (party != nullptr)
	{
		if (party->GetMemberCount() == 1) return false;

		FFindReviver f;
		party->ForEachMemberPtr(f);
		return f.HasReviver;
	}

	return false;
}

bool CHARACTER::IsRevive() const
{
	if (m_pkMobInst != nullptr)
	{
		return m_pkMobInst->m_IsRevive;
	}

	return false;
}

void CHARACTER::SetRevive(bool mode)
{
	if (m_pkMobInst != nullptr)
	{
		m_pkMobInst->m_IsRevive = mode;
	}
}

#define IS_SPEED_HACK_PLAYER(ch) (ch->m_speed_hack_count > SPEEDHACK_LIMIT_COUNT)

EVENTFUNC(check_speedhack_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );
	if ( info == nullptr )
	{
		sys_err( "check_speedhack_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;

	if (nullptr == ch || ch->IsNPC())
		return 0;

	if (IS_SPEED_HACK_PLAYER(ch))
	{
		LogManager::instance().SpeedHackLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(), ch->m_speed_hack_count);

		// @correction005
		if (g_bEnableSpeedHackCrash) // @correction216
		{
			LPDESC desc = ch->GetDesc();

			if (desc)
			{
				DESC_MANAGER::instance().DestroyDesc(desc);
				return 0;
			}
		}
	}

	ch->m_speed_hack_count = 0;

	ch->ResetComboHackCount();
	return PASSES_PER_SEC(60);
}

void CHARACTER::StartCheckSpeedHackEvent()
{
	if (m_pkCheckSpeedHackEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	m_pkCheckSpeedHackEvent = event_create(check_speedhack_event, info, PASSES_PER_SEC(60));
}

void CHARACTER::GoHome()
{
	WarpSet(EMPIRE_START_X(GetEmpire()), EMPIRE_START_Y(GetEmpire()));
}

void CHARACTER::SendGuildName(CGuild* pGuild)
{
	if (nullptr == pGuild) return;

	DESC	*desc = GetDesc();

	if (nullptr == desc) return;
	if (m_known_guild.find(pGuild->GetID()) != m_known_guild.end()) return;

	m_known_guild.insert(pGuild->GetID());

	TPacketGCGuildName	pack;
	memset(&pack, 0x00, sizeof(pack));

	pack.header		= HEADER_GC_GUILD;
	pack.subheader	= GUILD_SUBHEADER_GC_GUILD_NAME;
	pack.size		= sizeof(TPacketGCGuildName);
	pack.guildID	= pGuild->GetID();
	memcpy(pack.guildName, pGuild->GetName(), GUILD_NAME_MAX_LEN);

	desc->Packet(&pack, sizeof(pack));
}

void CHARACTER::SendGuildName(uint32_t dwGuildID)
{
	SendGuildName(CGuildManager::instance().FindGuild(dwGuildID));
}

EVENTFUNC(destroy_when_idle_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );
	if ( info == nullptr )
	{
		sys_err( "destroy_when_idle_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = info->ch;
	if (ch == nullptr) {
		return 0;
	}

	if (ch->GetVictim())
	{
		return PASSES_PER_SEC(300);
	}

	sys_log(1, "DESTROY_WHEN_IDLE: %s", ch->GetName());

	ch->m_pkDestroyWhenIdleEvent = nullptr;
	M2_DESTROY_CHARACTER(ch);
	return 0;
}

void CHARACTER::StartDestroyWhenIdleEvent()
{
	if (m_pkDestroyWhenIdleEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;

	m_pkDestroyWhenIdleEvent = event_create(destroy_when_idle_event, info, PASSES_PER_SEC(300));
}

void CHARACTER::SetComboSequence(uint8_t seq)
{
	m_bComboSequence = seq;
}

uint8_t CHARACTER::GetComboSequence() const
{
	return m_bComboSequence;
}

void CHARACTER::SetLastComboTime(uint32_t time)
{
	m_dwLastComboTime = time;
}

uint32_t CHARACTER::GetLastComboTime() const
{
	return m_dwLastComboTime;
}

void CHARACTER::SetValidComboInterval(int32_t interval)
{
	m_iValidComboInterval = interval;
}

int32_t CHARACTER::GetValidComboInterval() const
{
	return m_iValidComboInterval;
}

uint8_t CHARACTER::GetComboIndex() const
{
	return m_bComboIndex;
}

void CHARACTER::IncreaseComboHackCount(int32_t k)
{
	m_iComboHackCount += k;

	if (m_iComboHackCount >= 10)
	{
		if (GetDesc())
			if (GetDesc()->DelayedDisconnect(number(2, 7)))
			{
				sys_log(0, "COMBO_HACK_DISCONNECT: %s count: %d", GetName(), m_iComboHackCount);
				LogManager::instance().HackLog("Combo", this);
			}
	}
}

void CHARACTER::ResetComboHackCount()
{
	m_iComboHackCount = 0;
}

void CHARACTER::SkipComboAttackByTime(int32_t interval)
{
	m_dwSkipComboAttackByTime = get_dword_time() + interval;
}

uint32_t CHARACTER::GetSkipComboAttackByTime() const
{
	return m_dwSkipComboAttackByTime;
}

void CHARACTER::ResetChatCounter()
{
	m_bChatCounter = 0;
}

uint8_t CHARACTER::IncreaseChatCounter()
{
	return ++m_bChatCounter;
}

uint8_t CHARACTER::GetChatCounter() const
{
	return m_bChatCounter;
}

bool CHARACTER::IsRiding() const
{
	return IsHorseRiding() || GetMountVnum();
}

bool CHARACTER::CanWarp() const
{
	const int32_t iPulse = thecore_pulse();
	const int32_t limit_time = PASSES_PER_SEC(g_nPortalLimitTime);

	if ((iPulse - GetSafeboxLoadTime()) < limit_time)
		return false;

	if ((iPulse - GetExchangeTime()) < limit_time)
		return false;

	if ((iPulse - GetMyShopTime()) < limit_time)
		return false;

	if ((iPulse - GetRefineTime()) < limit_time)
		return false;

	if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen())
		return false;

	return true;
}

uint32_t CHARACTER::GetNextExp() const
{
	if (PLAYER_MAX_LEVEL_CONST < GetLevel())
		return 2500000000u;
	else
		return exp_table[GetLevel()];
}

int32_t	CHARACTER::GetSkillPowerByLevel(int32_t level, bool bMob) const
{
	return CTableBySkill::instance().GetSkillPowerByLevelFromType(GetJob(), GetSkillGroup(), MINMAX(0, level, SKILL_MAX_LEVEL), bMob);
}

#ifdef __ACCE_SYSTEM__ // @correction191
bool Is30Abs(uint32_t vnum)
{
	switch(vnum)
	{
		case 85044:
		case 85054:
		case 85064:
		case 85074:
		case 86104:
		case 86108:
		case 86114:
		case 86118:
		case 86130:
		case 86126:
		case 86122:
			return true;
	}

	return false;
}

void CHARACTER::OpenAcce(bool bCombination)
{
	if (isAcceOpened(bCombination))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The acce window it's already opened."));
		return;
	}
	
	if (bCombination)
	{
		if (m_bAcceAbsorption)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Before you may close the acce absorption window."));
			return;
		}
		
		m_bAcceCombination = true;
	}
	else
	{
		if (m_bAcceCombination)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Before you may close the acce combine window."));
			return;
		}
		
		m_bAcceAbsorption = true;
	}
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketAcce sPacket;
	sPacket.header = HEADER_GC_ACCE;
	sPacket.subheader = ACCE_SUBHEADER_GC_OPEN;
	sPacket.bWindow = bCombination;
	sPacket.dwPrice = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	GetDesc()->Packet(&sPacket, sizeof(TPacketAcce));
	
	ClearAcceMaterials();
}

void CHARACTER::CloseAcce()
{
	if ((!m_bAcceCombination) && (!m_bAcceAbsorption))
		return;
	
	bool bWindow = (m_bAcceCombination == true ? true : false);
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketAcce sPacket;
	sPacket.header = HEADER_GC_ACCE;
	sPacket.subheader = ACCE_SUBHEADER_GC_CLOSE;
	sPacket.bWindow = bWindow;
	sPacket.dwPrice = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	GetDesc()->Packet(&sPacket, sizeof(TPacketAcce));
	
	if (bWindow)
		m_bAcceCombination = false;
	else
		m_bAcceAbsorption = false;
	
	ClearAcceMaterials();
}

void CHARACTER::ClearAcceMaterials()
{
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetAcceMaterials();
	for (int32_t i = 0; i < ACCE_WINDOW_MAX_MATERIALS; ++i)
	{
		if (!pkItemMaterial[i])
			continue;
		
		pkItemMaterial[i]->Lock(false);
		pkItemMaterial[i] = nullptr;
	}
}

bool CHARACTER::AcceIsSameGrade(int32_t lGrade)
{
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetAcceMaterials();
	if (!pkItemMaterial[0])
		return false;
	
	bool bReturn = (pkItemMaterial[0]->GetAcceValue() == lGrade ? true : false);
	return bReturn;
}

uint64_t CHARACTER::GetAcceCombinePrice(int32_t lGrade)
{
	uint64_t dwPrice = 0;
	switch (lGrade)
	{
		case 2:
			{
				dwPrice = ACCE_GRADE_2_PRICE;
			}
			break;
		case 3:
			{
				dwPrice = ACCE_GRADE_3_PRICE;
			}
			break;
		case 4:
			{
				dwPrice = ACCE_GRADE_4_PRICE;
			}
			break;
		default:
			{
				dwPrice = ACCE_GRADE_1_PRICE;
			}
			break;
	}
	
	return dwPrice;
}

uint8_t CHARACTER::CheckEmptyMaterialSlot()
{
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetAcceMaterials();
	for (int32_t i = 0; i < ACCE_WINDOW_MAX_MATERIALS; ++i)
	{
		if (!pkItemMaterial[i])
			return i;
	}
	
	return 255;
}

void CHARACTER::GetAcceCombineResult(uint32_t & dwItemVnum, uint32_t & dwMinAbs, uint32_t & dwMaxAbs)
{
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetAcceMaterials();
	
	if (m_bAcceCombination)
	{
		if ((pkItemMaterial[0]) && (pkItemMaterial[1]))
		{
			int32_t lVal = pkItemMaterial[0]->GetAcceValue();
			if (lVal == 4)
			{
				dwItemVnum = pkItemMaterial[0]->GetOriginalVnum();
				dwMinAbs = pkItemMaterial[0]->GetSocket(ACCE_ABSORPTION_SOCKET);
				uint32_t dwMaxAbsCalc = 0;
				if (Is30Abs(dwItemVnum))
					dwMaxAbsCalc = (dwMinAbs + (uint32_t)ACCE_GRADE_4_ABS_RANGE > (uint32_t)ACCE_GRADE_NEW_ABS_30 ? (uint32_t)ACCE_GRADE_NEW_ABS_30 : (dwMinAbs + (uint32_t)ACCE_GRADE_4_ABS_RANGE));
				else
					dwMaxAbsCalc = (dwMinAbs + (uint32_t)ACCE_GRADE_4_ABS_RANGE > (uint32_t)ACCE_GRADE_4_ABS_MAX ? (uint32_t)ACCE_GRADE_4_ABS_MAX : (dwMinAbs + (uint32_t)ACCE_GRADE_4_ABS_RANGE));

				dwMaxAbs = dwMaxAbsCalc;
			}
			else
			{
				uint32_t dwMaskVnum = pkItemMaterial[0]->GetOriginalVnum();
				TItemTable * pTable = ITEM_MANAGER::instance().GetTable(dwMaskVnum + 1);
				if (pTable)
					dwMaskVnum += 1;
				
				dwItemVnum = dwMaskVnum;
				switch (lVal)
				{
					case 2:
						{
							dwMinAbs = ACCE_GRADE_3_ABS;
							dwMaxAbs = ACCE_GRADE_3_ABS;
						}
						break;
					case 3:
						{
							dwMinAbs = ACCE_GRADE_4_ABS_MIN;
							dwMaxAbs = ACCE_GRADE_4_ABS_MAX_COMB;
						}
						break;
					default:
						{
							dwMinAbs = ACCE_GRADE_2_ABS;
							dwMaxAbs = ACCE_GRADE_2_ABS;
						}
						break;
				}
			}
		}
		else
		{
			dwItemVnum = 0;
			dwMinAbs = 0;
			dwMaxAbs = 0;
		}
	}
	else
	{
		if ((pkItemMaterial[0]) && (pkItemMaterial[1]))
		{
			dwItemVnum = pkItemMaterial[0]->GetOriginalVnum();
			dwMinAbs = pkItemMaterial[0]->GetSocket(ACCE_ABSORPTION_SOCKET);
			dwMaxAbs = dwMinAbs;
		}
		else
		{
			dwItemVnum = 0;
			dwMinAbs = 0;
			dwMaxAbs = 0;
		}
	}
}

void CHARACTER::AddAcceMaterial(TItemPos tPos, uint8_t bPos)
{
	if (bPos >= ACCE_WINDOW_MAX_MATERIALS)
	{
		if (bPos == 255)
		{
			bPos = CheckEmptyMaterialSlot();
			if (bPos >= ACCE_WINDOW_MAX_MATERIALS)
				return;
		}
		else
			return;
	}
	
	LPITEM pkItem = GetItem(tPos);
	if (!pkItem)
		return;
	else if ((pkItem->GetCell() >= INVENTORY_MAX_NUM) || (pkItem->IsEquipped()) || (tPos.IsBeltInventoryPosition()) || (pkItem->IsDragonSoul()))
		return;
	else if ((pkItem->GetType() != ITEM_COSTUME) && (m_bAcceCombination))
		return;
	else if ((pkItem->GetType() != ITEM_COSTUME) && (bPos == 0) && (m_bAcceAbsorption))
		return;
	else if (pkItem->isLocked())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't add locked items."));
		return;
	}
#ifdef __SOULBIND__ // @correction202
	else if (pkItem->IsSealed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't add binded items."));
		return;
	}
#endif
	else if ((m_bAcceCombination) && (bPos == 1) && (!AcceIsSameGrade(pkItem->GetAcceValue())))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can combine just accees of same grade."));
		return;
	}
	else if ((m_bAcceCombination) && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= ACCE_GRADE_4_ABS_MAX))
	{
		if (Is30Abs(pkItem->GetVnum()))
		{
			if ((m_bAcceCombination) && (pkItem->GetSocket(ACCE_ABSORPTION_SOCKET) >= ACCE_GRADE_NEW_ABS_30))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This acce got already maximum absorption chance."));
				return;
			}
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This acce got already maximum absorption chance."));
			return;
		}
	}
	else if ((bPos == 1) && (m_bAcceAbsorption))
	{
		if ((pkItem->GetType() != ITEM_WEAPON) && (pkItem->GetType() != ITEM_ARMOR))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can absorb just the bonuses from armors and weapons."));
			return;
		}
		else if ((pkItem->GetType() == ITEM_ARMOR) && (pkItem->GetSubType() != ARMOR_BODY))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can absorb just the bonuses from armors and weapons."));
			return;
		}
	}
	else if ((pkItem->GetSubType() != COSTUME_ACCE) && (m_bAcceCombination))
		return;
	else if ((pkItem->GetSubType() != COSTUME_ACCE) && (bPos == 0) && (m_bAcceAbsorption))
		return;
	else if ((pkItem->GetSocket(ACCE_ABSORBED_SOCKET) > 0) && (bPos == 0) && (m_bAcceAbsorption))
		return;
	
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetAcceMaterials();
	if ((bPos == 1) && (!pkItemMaterial[0]))
		return;
	
	if (pkItemMaterial[bPos])
		return;
	
	pkItemMaterial[bPos] = pkItem;
	pkItemMaterial[bPos]->Lock(true);
	
	uint32_t dwItemVnum, dwMinAbs, dwMaxAbs;
	GetAcceCombineResult(dwItemVnum, dwMinAbs, dwMaxAbs);
	
	TPacketAcce sPacket;
	sPacket.header = HEADER_GC_ACCE;
	sPacket.subheader = ACCE_SUBHEADER_GC_ADDED;
	sPacket.bWindow = m_bAcceCombination == true ? true : false;
	sPacket.dwPrice = GetAcceCombinePrice(pkItem->GetAcceValue());
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = dwItemVnum;
	sPacket.dwMinAbs = dwMinAbs;
	sPacket.dwMaxAbs = dwMaxAbs;
	GetDesc()->Packet(&sPacket, sizeof(TPacketAcce));
}

void CHARACTER::RemoveAcceMaterial(uint8_t bPos)
{
	if (bPos >= ACCE_WINDOW_MAX_MATERIALS)
		return;
	
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetAcceMaterials();
	
	uint64_t dwPrice = 0;
	
	if (bPos == 1)
	{
		if (pkItemMaterial[bPos])
		{
			pkItemMaterial[bPos]->Lock(false);
			pkItemMaterial[bPos] = nullptr;
		}
		
		if (pkItemMaterial[0])
			dwPrice = GetAcceCombinePrice(pkItemMaterial[0]->GetAcceValue());
	}
	else
		ClearAcceMaterials();
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketAcce sPacket;
	sPacket.header = HEADER_GC_ACCE;
	sPacket.subheader = ACCE_SUBHEADER_GC_REMOVED;
	sPacket.bWindow = m_bAcceCombination == true ? true : false;
	sPacket.dwPrice = dwPrice;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	GetDesc()->Packet(&sPacket, sizeof(TPacketAcce));
}

uint8_t CHARACTER::CanRefineAcceMaterials()
{
	uint8_t bReturn = 0;
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetAcceMaterials();
	if (m_bAcceCombination)
	{
		for (int32_t i = 0; i < ACCE_WINDOW_MAX_MATERIALS; ++i)
		{
			if (pkItemMaterial[i])
			{
				if ((pkItemMaterial[i]->GetType() == ITEM_COSTUME) && (pkItemMaterial[i]->GetSubType() == COSTUME_ACCE))
					bReturn = 1;
				else
				{
					bReturn = 0;
					break;
				}
			}
			else
			{
				bReturn = 0;
				break;
			}
		}
	}
	else if (m_bAcceAbsorption)
	{
		if ((pkItemMaterial[0]) && (pkItemMaterial[1]))
		{
			if ((pkItemMaterial[0]->GetType() == ITEM_COSTUME) && (pkItemMaterial[0]->GetSubType() == COSTUME_ACCE))
				bReturn = 2;
			else
				bReturn = 0;
			
			if ((pkItemMaterial[1]->GetType() == ITEM_WEAPON) || ((pkItemMaterial[1]->GetType() == ITEM_ARMOR) && (pkItemMaterial[1]->GetSubType() == ARMOR_BODY)))
				bReturn = 2;
			else
				bReturn = 0;
			
			if (pkItemMaterial[0]->GetSocket(ACCE_ABSORBED_SOCKET) > 0)
				bReturn = 0;
		}
		else
			bReturn = 0;
	}
	
	return bReturn;
}

void CHARACTER::RefineAcceMaterials()
{
	uint8_t bCan = CanRefineAcceMaterials();
	if (bCan == 0)
		return;
	
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetAcceMaterials();
	
	uint32_t dwItemVnum, dwMinAbs, dwMaxAbs;
	GetAcceCombineResult(dwItemVnum, dwMinAbs, dwMaxAbs);
	uint64_t dwPrice = GetAcceCombinePrice(pkItemMaterial[0]->GetAcceValue());
	
	if (bCan == 1)
	{
		int32_t iSuccessChance = 0;
		int32_t lVal = pkItemMaterial[0]->GetAcceValue();
		switch (lVal)
		{
			case 2:
				{
					iSuccessChance = ACCE_COMBINE_GRADE_2;
				}
				break;
			case 3:
				{
					iSuccessChance = ACCE_COMBINE_GRADE_3;
				}
				break;
			case 4:
				{
					iSuccessChance = ACCE_COMBINE_GRADE_4;
				}
				break;
			default:
				{
					iSuccessChance = ACCE_COMBINE_GRADE_1;
				}
				break;
		}
		
		if (GetGold() < dwPrice)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You don't have enough Yang."));
			return;
		}
		
		int32_t iChance = number(1, 100);
		bool bSucces = (iChance <= iSuccessChance ? true : false);
		if (bSucces)
		{
			LPITEM pkItem = ITEM_MANAGER::instance().CreateItem(dwItemVnum, 1, 0, false);
			if (!pkItem)
			{
				sys_err("%d can't be created.", dwItemVnum);
				return;
			}
			
			ITEM_MANAGER::CopyAllAttrTo(pkItemMaterial[0], pkItem);
			LogManager::instance().ItemLog(this, pkItem, "COMBINE SUCCESS", pkItem->GetName());
			uint32_t dwAbs = (dwMinAbs == dwMaxAbs ? dwMinAbs : number(dwMinAbs + 1, dwMaxAbs));
			pkItem->SetSocket(ACCE_ABSORPTION_SOCKET, dwAbs);
			pkItem->SetSocket(ACCE_ABSORBED_SOCKET, pkItemMaterial[0]->GetSocket(ACCE_ABSORBED_SOCKET));
			
			ChangeGold(false, dwPrice);
			DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, pkItemMaterial[0]->GetVnum(), -dwPrice);
			
			uint16_t wCell = pkItemMaterial[0]->GetCell();
			ITEM_MANAGER::instance().RemoveItem(pkItemMaterial[0], "COMBINE (REFINE SUCCESS)");
			ITEM_MANAGER::instance().RemoveItem(pkItemMaterial[1], "COMBINE (REFINE SUCCESS)");
			
			pkItem->AddToCharacter(this, TItemPos(INVENTORY, wCell));
			ITEM_MANAGER::instance().FlushDelayedSave(pkItem);
			pkItem->AttrLog();
			
			if (lVal == 4)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("New absorption rate: %d%"), dwAbs);
			else
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Success."));
			
			EffectPacket(SE_EFFECT_ACCE_SUCCEDED);
			LogManager::instance().AcceLog(GetPlayerID(), GetX(), GetY(), dwItemVnum, pkItem->GetID(), 1, dwAbs, 1);
			
			ClearAcceMaterials();
		}
		else
		{
			ChangeGold(false, dwPrice);
			DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, pkItemMaterial[0]->GetVnum(), -dwPrice);
			
			ITEM_MANAGER::instance().RemoveItem(pkItemMaterial[1], "COMBINE (REFINE FAIL)");
			
			if (lVal == 4)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("New absorption rate: %d%"), pkItemMaterial[0]->GetSocket(ACCE_ABSORPTION_SOCKET));
			else
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Failed."));
			
			LogManager::instance().AcceLog(GetPlayerID(), GetX(), GetY(), dwItemVnum, 0, 0, 0, 0);
			
			pkItemMaterial[1] = nullptr;
		}
		
		TItemPos tPos;
		tPos.window_type = INVENTORY;
		tPos.cell = 0;
		
		TPacketAcce sPacket;
		sPacket.header = HEADER_GC_ACCE;
		sPacket.subheader = ACCE_SUBHEADER_GC_REFINED;
		sPacket.bWindow = m_bAcceCombination == true ? true : false;
		sPacket.dwPrice = dwPrice;
		sPacket.bPos = 0;
		sPacket.tPos = tPos;
		sPacket.dwItemVnum = 0;
		sPacket.dwMinAbs = 0;
		if (bSucces)
			sPacket.dwMaxAbs = 100;
		else
			sPacket.dwMaxAbs = 0;
		
		GetDesc()->Packet(&sPacket, sizeof(TPacketAcce));
	}
	else
	{
		pkItemMaterial[1]->CopyAttributeTo(pkItemMaterial[0]);
		LogManager::instance().ItemLog(this, pkItemMaterial[0], "ABSORB (REFINE SUCCESS)", pkItemMaterial[0]->GetName());
		pkItemMaterial[0]->SetSocket(ACCE_ABSORBED_SOCKET, pkItemMaterial[1]->GetOriginalVnum());
		for (int32_t i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		{
			if (pkItemMaterial[0]->GetAttributeValue(i) < 0)
				pkItemMaterial[0]->SetForceAttribute(i, pkItemMaterial[0]->GetAttributeType(i), 0);
		}
		
		ITEM_MANAGER::instance().RemoveItem(pkItemMaterial[1], "ABSORBED (REFINE SUCCESS)");
		
		ITEM_MANAGER::instance().FlushDelayedSave(pkItemMaterial[0]);
		pkItemMaterial[0]->AttrLog();
		
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Success."));
		
		ClearAcceMaterials();
		
		TItemPos tPos;
		tPos.window_type = INVENTORY;
		tPos.cell = 0;
		
		TPacketAcce sPacket;
		sPacket.header = HEADER_GC_ACCE;
		sPacket.subheader = ACCE_SUBHEADER_GC_REFINED;
		sPacket.bWindow = m_bAcceCombination == true ? true : false;
		sPacket.dwPrice = dwPrice;
		sPacket.bPos = 255;
		sPacket.tPos = tPos;
		sPacket.dwItemVnum = 0;
		sPacket.dwMinAbs = 0;
		sPacket.dwMaxAbs = 1;
		GetDesc()->Packet(&sPacket, sizeof(TPacketAcce));
	}
}

bool CHARACTER::CleanAcceAttr(LPITEM pkItem, LPITEM pkTarget)
{
	if (!CanHandleItem())
		return false;
	else if ((!pkItem) || (!pkTarget))
		return false;
	else if ((pkTarget->GetType() != ITEM_COSTUME) && (pkTarget->GetSubType() != COSTUME_ACCE))
		return false;
	
	if (pkTarget->GetSocket(ACCE_ABSORBED_SOCKET) <= 0)
		return false;
	
	pkTarget->SetSocket(ACCE_ABSORBED_SOCKET, 0);
	for (int32_t i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		pkTarget->SetForceAttribute(i, 0, 0);
	
	pkItem->SetCount(pkItem->GetCount() - 1);
	LogManager::instance().ItemLog(this, pkTarget, "USE_DETACHMENT (CLEAN ATTR)", pkTarget->GetName());
	return true;
}
#endif

#ifdef __CHANGE_LOOK__ // @correction192
void CHARACTER::ChangeLookWindow(bool bOpen, bool bRequest)
{
	if ((bOpen) && (isChangeLookOpened()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] The window is already opened."));
		return;
	}
	
	if ((!bOpen) && (!isChangeLookOpened()))
	{
		if (!bRequest)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] The window is not opened."));
		
		return;
	}
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketChangeLook sPacket;
	sPacket.header = HEADER_GC_CL;
	sPacket.subheader = CL_SUBHEADER_OPEN;
	sPacket.subheader = bOpen == true ? CL_SUBHEADER_OPEN : CL_SUBHEADER_CLOSE;
	sPacket.dwCost = bOpen == true ? CL_PRICE : 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	GetDesc()->Packet(&sPacket, sizeof(TPacketChangeLook));
	
	m_bChangeLook = bOpen;
	ClearClWindowMaterials();
}

void CHARACTER::ClearClWindowMaterials()
{
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetClWindowMaterials();
	for (int32_t i = 0; i < CL_WINDOW_MAX_MATERIALS; ++i)
	{
		if (!pkItemMaterial[i])
			continue;
		
		pkItemMaterial[i]->Lock(false);
		pkItemMaterial[i] = nullptr;
	}
}

uint8_t CHARACTER::CheckClEmptyMaterialSlot()
{
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetClWindowMaterials();
	for (int32_t i = 0; i < CL_WINDOW_MAX_MATERIALS; ++i)
	{
		if (!pkItemMaterial[i])
			return i;
	}
	
	return 255;
}

void CHARACTER::AddClMaterial(TItemPos tPos, uint8_t bPos)
{
	if (!isChangeLookOpened())
		return;
	else if (bPos >= CL_WINDOW_MAX_MATERIALS)
	{
		if (bPos != 255)
			return;
		
		bPos = CheckClEmptyMaterialSlot();
		if (bPos >= CL_WINDOW_MAX_MATERIALS)
			return;
	}
#ifdef __CHANGE_LOOK_ITEM__ // @correction192
	else if (bPos == 2)
	{
		LPITEM * pkItemMaterial = GetClWindowMaterials();

		if (pkItemMaterial[2])
			return;
	}
#endif

	LPITEM pkItem = GetItem(tPos);
	if (!pkItem)
		return;
	else if ((pkItem->GetCell() >= INVENTORY_MAX_NUM) || (tPos.IsBeltInventoryPosition()))
		return;
	else if (pkItem->IsEquipped())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] You cannot transmute an item while it is equipped."));
		return;
	}
	else if ((pkItem->GetType() != ITEM_WEAPON) && (pkItem->GetType() != ITEM_ARMOR) && (pkItem->GetType() != ITEM_COSTUME) && (pkItem->GetType() != ITEM_PET)
#ifdef __CHANGE_LOOK_ITEM__ // @correction192
 && (pkItem->GetVnum() != 72326 && pkItem->GetVnum() != 72341)
#endif
)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] This item cannot be transmuted."));
		ChatPacket(CHAT_TYPE_INFO, "%d", pkItem->GetVnum());
		return;
	}
	else if ((pkItem->GetType() == ITEM_WEAPON) && ((pkItem->GetSubType() == WEAPON_ARROW) || (pkItem->GetSubType() == WEAPON_MOUNT_SPEAR)
#ifdef __QUIVER__ // @correction203
|| (pkItem->GetSubType() == WEAPON_QUIVER))
#endif
)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] This item cannot be transmuted."));
		return;
	}
	else if ((pkItem->GetType() == ITEM_PET) && (pkItem->GetSubType() != PET_PAY))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] This item cannot be transmuted."));
		return;
	}
	else if ((pkItem->GetType() == ITEM_ARMOR) && (pkItem->GetSubType() != ARMOR_BODY))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] This item cannot be transmuted."));
		return;
	}
	else if ((pkItem->GetType() == ITEM_COSTUME) && (pkItem->GetSubType() != COSTUME_BODY) && (pkItem->GetSubType() != COSTUME_HAIR)
#ifdef __COSTUME_WEAPON__ // @correction204
&& (pkItem->GetSubType() != COSTUME_WEAPON)
#endif
#ifdef __COSTUME_MOUNT__
&& (pkItem->GetSubType() != COSTUME_MOUNT)
#endif
)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] This item cannot be transmuted."));
		return;
	}
#ifdef __SOULBIND__ // @correction202
	else if (pkItem->IsSealed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] You can't add binded items."));
		return;
	}
#endif
	else if (pkItem->isLocked())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] You can't add locked items."));
		return;
	}
	else if (pkItem->GetChangeLook() != 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] You can't add items which are transmuted yet."));
		return;
	}
	
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetClWindowMaterials();
	if ((bPos == 1) && (!pkItemMaterial[0]))
		return;
	
	if (pkItemMaterial[bPos])
		return;
	
	if (bPos == 1)
	{
		bool bStop = false;
		if (pkItemMaterial[0]->GetType() != pkItem->GetType())
		{
			if ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_BODY) && (pkItemMaterial[0]->GetType() == ITEM_ARMOR && pkItemMaterial[0]->GetSubType() == ARMOR_BODY))
				bStop = false;
			else if ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_WEAPON) && (pkItemMaterial[0]->GetType() == ITEM_WEAPON))
				bStop = false;
			else if ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_BODY) && (pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_BODY))
				bStop = false;
			else if ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_HAIR) && (pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_HAIR))
				bStop = false;
			else if ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_WEAPON) && (pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_WEAPON))
				bStop = false;
			else if ((pkItem->GetType() == ITEM_ARMOR && pkItem->GetSubType() == ARMOR_BODY) && (pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_BODY))
				bStop = false;
			else if ((pkItem->GetType() == ITEM_WEAPON) && (pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_WEAPON))
				bStop = false;
			else if ((pkItem->GetType() == ITEM_COSTUME) && (pkItem->GetSubType() == COSTUME_MOUNT) && pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_MOUNT)
				bStop = false;
			else if ((pkItem->GetType() == ITEM_PET) && (pkItem->GetSubType() == PET_PAY) && pkItemMaterial[0]->GetType() == ITEM_PET && pkItemMaterial[0]->GetSubType() == PET_PAY)
				bStop = false;
			else
				bStop = true;
		}
		else
		{
			if (pkItemMaterial[0]->GetType() != pkItem->GetType())
				bStop = true;

			else if (pkItemMaterial[0]->GetSubType() != pkItem->GetSubType())
				bStop = true;			
		}

		if (pkItemMaterial[0]->GetOriginalVnum() == pkItem->GetOriginalVnum())
			bStop = true;
		else if (((IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_FEMALE)) && (!IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_FEMALE))) || ((IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_MALE)) && (!IS_SET(pkItem->GetAntiFlag(), ITEM_ANTIFLAG_MALE))))
			bStop = true;
		else if ((pkItem->GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR) && (!IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_WARRIOR)))
			bStop = true;
		else if ((pkItem->GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN) && (!IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_ASSASSIN)))
			bStop = true;
		else if ((pkItem->GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN) && (!IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_SHAMAN)))
			bStop = true;
		else if ((pkItem->GetAntiFlag() & ITEM_ANTIFLAG_SURA) && (!IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_SURA)))
			bStop = true;
#ifdef ENABLE_WOLFMAN_CHARACTER
		else if ((pkItem->GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN) && (!IS_SET(pkItemMaterial[0]->GetAntiFlag(), ITEM_ANTIFLAG_WOLFMAN)))
			bStop = true;
#endif
		else if ((pkItem->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetType() == ITEM_COSTUME) && ((pkItem->GetType() == ITEM_COSTUME && pkItem->GetSubType() == COSTUME_WEAPON) && (pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_WEAPON)))
			if (pkItem->GetValue(3) != pkItemMaterial[0]->GetValue(3))
				bStop = true;
		
		if (bStop)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] You cannot submit this item."));
			return;
		}
	}
	
	pkItemMaterial[bPos] = pkItem;
	pkItemMaterial[bPos]->Lock(true);
	
	TPacketChangeLook sPacket;
	sPacket.header = HEADER_GC_CL;
	sPacket.subheader = CL_SUBHEADER_ADD;
	sPacket.dwCost = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	GetDesc()->Packet(&sPacket, sizeof(TPacketChangeLook));
}

void CHARACTER::RemoveClMaterial(uint8_t bPos)
{
	if (bPos >= CL_WINDOW_MAX_MATERIALS)
		return;
	
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetClWindowMaterials();
	
	if (!pkItemMaterial[bPos])
		return;
	
	if (bPos == 1)
	{
		pkItemMaterial[bPos]->Lock(false);
		pkItemMaterial[bPos] = nullptr;
	}
	else
		ClearClWindowMaterials();
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketChangeLook sPacket;
	sPacket.header = HEADER_GC_CL;
	sPacket.subheader = CL_SUBHEADER_REMOVE;
	sPacket.dwCost = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	GetDesc()->Packet(&sPacket, sizeof(TPacketChangeLook));
}

void CHARACTER::RefineClMaterials()
{
	LPITEM * pkItemMaterial;
	pkItemMaterial = GetClWindowMaterials();
	if (!pkItemMaterial[0])
		return;
	else if (!pkItemMaterial[1])
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] Please submit the item you want to transmute."));
		return;
	}

	uint64_t iPrice = CL_PRICE;
#ifdef __CHANGE_LOOK_ITEM__ // @correction192
	if (!pkItemMaterial[2])
	{
		if (GetGold() < iPrice)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] You don't have enough Yang."));
			return;
		}
	}
	else
	{
		if (!CountSpecifyItem(pkItemMaterial[2]->GetVnum()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Biletin yok lan"));
			return;
		}
	}
#else
	if (GetGold() < iPrice)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("[ChangeLook] You don't have enough Yang."));
		return;
	}
#endif

	uint32_t dwVnum = pkItemMaterial[1]->GetVnum();
#ifdef __COSTUME_MOUNT__
	if ((pkItemMaterial[0]->GetType() == ITEM_COSTUME && pkItemMaterial[0]->GetSubType() == COSTUME_MOUNT) && (pkItemMaterial[1]->GetType() == ITEM_COSTUME && pkItemMaterial[1]->GetSubType() == COSTUME_MOUNT))
		dwVnum = pkItemMaterial[1]->GetValue(4);
#endif
	if ((pkItemMaterial[0]->GetType() == ITEM_PET && pkItemMaterial[0]->GetSubType() == PET_PAY) && (pkItemMaterial[1]->GetType() == ITEM_PET && pkItemMaterial[1]->GetSubType() == PET_PAY))
		dwVnum = pkItemMaterial[1]->GetValue(0);

#ifdef __CHANGE_LOOK_ITEM__ // @correction192
	if (!pkItemMaterial[2])
	{
		ChangeGold(false, iPrice);
		DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, pkItemMaterial[0]->GetVnum(), -iPrice);
	}
	else
		RemoveSpecifyItem(pkItemMaterial[2]->GetVnum(), 1);
#else
	ChangeGold(false, iPrice);
	DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, pkItemMaterial[0]->GetVnum(), -iPrice);
#endif
	ITEM_MANAGER::instance().RemoveItem(pkItemMaterial[1], "TRANSMUTED (SUCCESSFULLY)");
	
	pkItemMaterial[0]->SetChangeLook(dwVnum, true);
	ClearClWindowMaterials();
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketChangeLook sPacket;
	sPacket.header = HEADER_GC_CL;
	sPacket.subheader = CL_SUBHEADER_REFINE;
	sPacket.dwCost = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	GetDesc()->Packet(&sPacket, sizeof(TPacketChangeLook));
}

bool CHARACTER::CleanChangeLook(LPITEM pkItem, LPITEM pkTarget)
{
	if (!CanHandleItem())
		return false;
	else if ((!pkItem) || (!pkTarget))
		return false;
	else if ((pkTarget->GetType() != ITEM_WEAPON) && (pkTarget->GetType() != ITEM_ARMOR) && (pkTarget->GetType() != ITEM_COSTUME))
		return false;
#ifdef __SOULBIND__ // @correction202
	else if (pkTarget->IsSealed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't remove the transmute because item is binded."));
		return false;
	}
#endif
	else if (pkTarget->isLocked())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't remove the transmute because item is locked."));
		return false;
	}
	
	if (pkTarget->GetChangeLook() == 0)
		return false;
	
	pkTarget->SetChangeLook(0);
	pkItem->SetCount(pkItem->GetCount() - 1);
	LogManager::instance().ItemLog(this, pkTarget, "USE_DETACHMENT (CLEAN TRANSMUTED)", pkTarget->GetName());
	return true;
}
#endif

#ifdef __OFFLINE_PRIVATE_SHOP__
void CHARACTER::SaveAndFlush()
{
	m_bSkipSave = false;
	CHARACTER_MANAGER::instance().DelayedSave(this);
	CHARACTER_MANAGER::instance().FlushDelayedSave(this);
	uint32_t dwPID = GetPlayerID();
	db_clientdesc->DBPacketHeader(HEADER_GD_FLUSH_CACHE, 0, sizeof(uint32_t));
	db_clientdesc->Packet(&dwPID, sizeof(uint32_t));
}
#endif

void CHARACTER::ChangeGold(bool bIsAdd, uint64_t amount, bool unused)
{
	if (bIsAdd)
	{
		uint64_t nTotalMoney = GetGold() + amount;
		if (GOLD_MAX <= nTotalMoney)
		{
			sys_err("[OVERFLOW_GOLD] OriGold %llu AddedGold %llu id %u Name %s ", GetGold(), amount, GetPlayerID(), GetName());
			LogManager::instance().CharLog(this, GetGold() + amount, "OVERFLOW_GOLD", "");
			return;
		}
		SetGold(GetGold() + amount);
	}
	else
	{
		// uint64_t nTotalMoney = GetGold() - amount;
		int64_t nTotalMoney2 = GetGold() - amount;
		if (nTotalMoney2 < 0)
		{
			sys_err("[OVERFLOW_GOLD] OriGold %llu FeeGold %llu id %u Name %s ", GetGold(), amount, GetPlayerID(), GetName());
			LogManager::instance().CharLog(this, GetGold() - amount, "OVERFLOW_GOLD", "");
			return;
		}
		SetGold(GetGold() - amount);
	}

	SendGold(unused);
}

void CHARACTER::SendGold(bool bIsLogin)
{
	if (!GetDesc())
		return;

	TPacketGold pack;
	memset(&pack, 0, sizeof(TPacketGold));
	pack.header = HEADER_GC_GOLD;
	pack.vid = GetVID();
	pack.gold = GetGold();
	pack.bIsLogin = !bIsLogin;
	GetDesc()->Packet(&pack, sizeof(pack));
}

#ifdef __ELEMENT_ADD__
uint8_t CHARACTER::GetElement() const
{
	if (GetPoint(POINT_ENCHANT_ELECT) > 0)
		return 1;
	else if (GetPoint(POINT_ENCHANT_FIRE) > 0)
		return 2;
	else if (GetPoint(POINT_ENCHANT_ICE) > 0)
		return 3;
	else if (GetPoint(POINT_ENCHANT_WIND) > 0)
		return 4;
	else if (GetPoint(POINT_ENCHANT_EARTH) > 0)
		return 5;
	else if (GetPoint(POINT_ENCHANT_DARK) > 0)
		return 6;

	return 0;
}
#endif

#ifdef __MOVE_CHANNEL__
void CHARACTER::ChannelSwitch(int32_t iNewChannel)
{

	int32_t lAddr;
	int32_t lMapIndex;
	uint16_t wPort;
	int32_t x = this->GetX();
	int32_t y = this->GetY();

	if (!CMapLocation::instance().Get(x, y, lMapIndex, lAddr, wPort))
		return;

	if (lMapIndex >= 10000)
		return;

	Stop();
	Save();

	if (GetSectree())
	{
		GetSectree()->RemoveEntity(this);
		ViewCleanup();
		EncodeRemovePacket(this);
	}

	int32_t iDiff = iNewChannel - g_bChannel;
	TPacketGCWarp p;
	p.bHeader = HEADER_GC_WARP;
	p.lX = x;
	p.lY = y;
	p.lAddr = lAddr;
	p.wPort = wPort + (iDiff * 28 * 2);

	ChatPacket(CHAT_TYPE_COMMAND, "Update %d", iNewChannel);
	GetDesc()->Packet(&p, sizeof(TPacketGCWarp));
}
#endif

void CHARACTER::CheckWears()
{
#ifdef __COSTUME_MOUNT__
	LPITEM item = GetWear(WEAR_COSTUME_MOUNT);
	if (item && item->IsRideItem())
	{
#ifdef __MOUNT__
		CMountSystem* MountSystem = GetMountSystem();
		if (MountSystem)
		{
#ifdef __CHANGE_LOOK__
			uint32_t dwVnum = item->GetChangeLook() ? item->GetChangeLook() : item->GetValue(4);
#else
			uint32_t dwVnum = item->GetValue(4);
#endif
			MountSystem->Summon(dwVnum, GetName());
			MountSystem->Mount(GetMountingVnumM());
		}
#else
		UseItem(TItemPos(INVENTORY, item->GetCell()));
#endif
	}

#endif
#ifdef __COSTUME_PET__
	item = GetWear(WEAR_PET);
	if (item)
	{
		if (item->GetType() == ITEM_PET && item->GetSubType() == PET_PAY)
		{
			CPetSystem* petSystem = GetPetSystem();
#ifdef __CHANGE_LOOK__
			uint32_t dwVnum = item->GetChangeLook() ? item->GetChangeLook() : item->GetValue(0);
#else
			uint32_t dwVnum = item->GetValue(0);
#endif
			if (petSystem)
				petSystem->Summon(dwVnum, item, 0, false);
		}
	}
#endif
}

#ifdef __ALIGN_RENEWAL__
void CHARACTER::OnAlignUpdate(int32_t lOldAlignment, bool manuelUpdate)
{
	if (GetRealAlignment() >= 300000)
	{
		if (lOldAlignment < 300000)
		{
			RemoveAffect(AFFECT_ALIGN_BONUS1);
			RemoveAffect(AFFECT_ALIGN_BONUS2);
			RemoveAffect(AFFECT_ALIGN_BONUS3);
		}
		AddAffect(AFFECT_ALIGN_BONUS1, POINT_MAX_HP, 5000, 0, 60 * 60 * 60 * 365, 0, true);
		AddAffect(AFFECT_ALIGN_BONUS2, POINT_ATTBONUS_STONE, 20, 0, 60 * 60 * 60 * 365, 0, true);
		AddAffect(AFFECT_ALIGN_BONUS3, POINT_ATTBONUS_MONSTER, 20, 0, 60 * 60 * 60 * 365, 0, true);
	}
	else if (GetRealAlignment() >= 250000)
	{
		if (lOldAlignment < 250000 && lOldAlignment >= 300000)
		{
			RemoveAffect(AFFECT_ALIGN_BONUS1);
			RemoveAffect(AFFECT_ALIGN_BONUS2);
			RemoveAffect(AFFECT_ALIGN_BONUS3);
		}
		AddAffect(AFFECT_ALIGN_BONUS1, POINT_MAX_HP, 3000, 0, 60 * 60 * 60 * 365, 0, true);
		AddAffect(AFFECT_ALIGN_BONUS2, POINT_ATTBONUS_MONSTER, 18, 0, 60 * 60 * 60 * 365, 0, true);
	}
	else if (GetRealAlignment() >= 200000)
	{
		if (lOldAlignment < 200000 && lOldAlignment >= 250000)
		{
			RemoveAffect(AFFECT_ALIGN_BONUS1);
			RemoveAffect(AFFECT_ALIGN_BONUS2);
			RemoveAffect(AFFECT_ALIGN_BONUS3);
		}
		AddAffect(AFFECT_ALIGN_BONUS1, POINT_MAX_HP, 2500, 0, 60 * 60 * 60 * 365, 0, true);
		AddAffect(AFFECT_ALIGN_BONUS2, POINT_ATTBONUS_MONSTER, 16, 0, 60 * 60 * 60 * 365, 0, true);
	}
	else if (GetRealAlignment() >= 180000)
	{
		if (lOldAlignment < 180000 && lOldAlignment >= 200000)
		{
			RemoveAffect(AFFECT_ALIGN_BONUS1);
			RemoveAffect(AFFECT_ALIGN_BONUS2);
			RemoveAffect(AFFECT_ALIGN_BONUS3);
		}
		AddAffect(AFFECT_ALIGN_BONUS1, POINT_MAX_HP, 2000, 0, 60 * 60 * 60 * 365, 0, true);
		AddAffect(AFFECT_ALIGN_BONUS2, POINT_ATTBONUS_MONSTER, 14, 0, 60 * 60 * 60 * 365, 0, true);
	}
	else if (GetRealAlignment() >= 120000)
	{
		if (lOldAlignment < 120000 && lOldAlignment >= 180000)
		{
			RemoveAffect(AFFECT_ALIGN_BONUS1);
			RemoveAffect(AFFECT_ALIGN_BONUS2);
			RemoveAffect(AFFECT_ALIGN_BONUS3);
		}
		AddAffect(AFFECT_ALIGN_BONUS1, POINT_MAX_HP, 1500, 0, 60 * 60 * 60 * 365, 0, true);
		AddAffect(AFFECT_ALIGN_BONUS2, POINT_ATTBONUS_MONSTER, 12, 0, 60 * 60 * 60 * 365, 0, true);
	}
	else if (GetRealAlignment() >= 80000)
	{
		if (lOldAlignment < 80000 && lOldAlignment >= 120000)
		{
			RemoveAffect(AFFECT_ALIGN_BONUS1);
			RemoveAffect(AFFECT_ALIGN_BONUS2);
			RemoveAffect(AFFECT_ALIGN_BONUS3);
		}
		AddAffect(AFFECT_ALIGN_BONUS1, POINT_MAX_HP, 1000, 0, 60 * 60 * 60 * 365, 0, true);
		AddAffect(AFFECT_ALIGN_BONUS2, POINT_ATTBONUS_MONSTER, 10, 0, 60 * 60 * 60 * 365, 0, true);
	}
	else if (GetRealAlignment() >= 40000)
	{
		if (lOldAlignment < 40000 && lOldAlignment >= 80000)
		{
			RemoveAffect(AFFECT_ALIGN_BONUS1);
			RemoveAffect(AFFECT_ALIGN_BONUS2);
			RemoveAffect(AFFECT_ALIGN_BONUS3);
		}
		AddAffect(AFFECT_ALIGN_BONUS1, POINT_MAX_HP, 750, 0, 60 * 60 * 60 * 365, 0, true);
		AddAffect(AFFECT_ALIGN_BONUS2, POINT_ATTBONUS_MONSTER, 5, 0, 60 * 60 * 60 * 365, 0, true);
	}
	else if (GetRealAlignment() >= 10000)
	{
		if (lOldAlignment < 10000 && lOldAlignment >= 40000)
		{
			RemoveAffect(AFFECT_ALIGN_BONUS1);
			RemoveAffect(AFFECT_ALIGN_BONUS2);
			RemoveAffect(AFFECT_ALIGN_BONUS3);
		}
		AddAffect(AFFECT_ALIGN_BONUS1, POINT_MAX_HP, 500, 0, 60 * 60 * 60 * 365, 0, true);
		AddAffect(AFFECT_ALIGN_BONUS2, POINT_ATTBONUS_MONSTER, 3, 0, 60 * 60 * 60 * 365, 0, true);
	}
	else
	{
		if (lOldAlignment >= 10000 && GetRealAlignment() >= 0)
		{
			RemoveAffect(AFFECT_ALIGN_BONUS1);
			RemoveAffect(AFFECT_ALIGN_BONUS2);
			RemoveAffect(AFFECT_ALIGN_BONUS3);
		}
		else if (lOldAlignment >= 0 && GetRealAlignment() < 0)
		{
			RemoveAffect(AFFECT_ALIGN_BONUS1);
			RemoveAffect(AFFECT_ALIGN_BONUS2);
			RemoveAffect(AFFECT_ALIGN_BONUS3);
		}
	}
}
#endif

#ifdef __SUPPORT__
void CHARACTER::SendSupportSkillPacket(uint32_t skill_vnum)
{
	TPacketGCSupportUseSkill pack;
	pack.bHeader = HEADER_GC_SUPPORT_SKILL;
	pack.dwVnum = skill_vnum;
	pack.dwVid = GetVID();
	pack.dwLevel = 41;

	PacketView(&pack, sizeof(TPacketGCSupportUseSkill), this);
}

uint32_t CHARACTER::SupportGetNextExp() const
{
	if (gPlayerMaxLevel < GetLevel())
		return 2500000000u;
	else
		return exp_support_table[GetLevel()];
}
#endif


#ifdef __GROWTH_PET__
void CHARACTER::SendPetLevelUpEffect(int32_t vid, int32_t type, int32_t value, int32_t amount)
{
	struct packet_point_change pack;

	pack.header = HEADER_GC_CHARACTER_POINT_CHANGE;
	pack.dwVID = vid;
	pack.type = type;
	pack.value = value;
	pack.amount = amount;
	PacketAround(&pack, sizeof(pack));
}

uint32_t CHARACTER::PetGetNextExp() const
{
	if (gPlayerMaxLevel < GetLevel())
		return 2500000000u;
	else
		return exppet_table[GetLevel()];
}
#endif


#ifdef __BOSS_TRACKING_SYSTEM__
EVENTFUNC(boss_tracking_data_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>(event->info);
	if (!info)
		return 0;

	LPCHARACTER	ch = info->ch;
	if (!ch || !ch->GetDesc())
		return 0;

	if (ch->IsPC() == true)
	{
		if (ch->IsBossTrackingWindow == true)
		{
			for (int32_t i = 0; i < 7; i++)
			{
				if (ch && ch->GetDesc())
					CBossTracking::instance().SendClientPacket(ch, i, ch->BossTrackingMobVnum);
			}
		}
	}
	return PASSES_PER_SEC(1);
}

void CHARACTER::StartBossTrackingDataEvent()
{
	if (m_pkUpdateBossTracking)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();
	info->ch = this;
	m_pkUpdateBossTracking = event_create(boss_tracking_data_event, info, PASSES_PER_SEC(1));
}

void CHARACTER::StopBossTrackingDataEvent()
{
	event_cancel(&m_pkUpdateBossTracking);
	m_pkUpdateBossTracking = nullptr;
	BossTrackingMobVnum = 0;
	IsBossTrackingWindow = false;
}
#endif

#ifdef __MINI_GAME_RUMI__
void CHARACTER::MiniGameRumiStart()
{
	if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중(창고,교환,상점)에는 사용할 수 없습니다."));
		return;
	}

	if (GetGold() < 30000)
	{
		ChatPacket(CHAT_TYPE_INFO, "You dont have enough money.");
		return;
	}

	if (CountSpecifyItem(79506) < 1)
	{
		ChatPacket(CHAT_TYPE_INFO, "You dont have any cardsets.");
		return;
	}

	if (m_MiniGameRumiCardDeckCount <= 0)
	{
		TPacketGCMiniGameRumi pack;
		pack.header = HEADER_GC_MINI_GAME_RUMI;
		pack.subheader = MINI_GAME_RUMI_SUBHEADER_GC_START;

		ChangeGold(false, 30000, true);
		RemoveSpecifyItem(79506, 1);
		MiniGameRumiClean();
		MiniGameRumiRandomDeck();
		GetDesc()->Packet(&pack, sizeof(pack));
	}
	else
		ChatPacket(CHAT_TYPE_INFO, "BUG YAPMA LAN");
}

void CHARACTER::MiniGameRumiClean()
{
	m_MiniGameRumiCardDeckCount = 0;
	m_MiniGameRumiPoints = 0;
	memset(&m_MiniGameRumiHandCards, 0, sizeof(m_MiniGameRumiHandCards));
	memset(&m_MiniGameRumiFieldCards, 0, sizeof(m_MiniGameRumiFieldCards));
	memset(&m_MiniGameRumiDeckCards, 0, sizeof(m_MiniGameRumiDeckCards));
}

void CHARACTER::MiniGameRumiExit()
{
	if (m_MiniGameRumiPoints >= 400)
		AutoGiveItem(50267); //Golden box
	else if (m_MiniGameRumiPoints < 400 && m_MiniGameRumiPoints >= 250)
		AutoGiveItem(50268); //Silver box
	else if (m_MiniGameRumiPoints > 0)
		AutoGiveItem(50269); //Bronze box

	MiniGameRumiClean();

	TPacketGCMiniGameRumi pack;
	pack.header = HEADER_GC_MINI_GAME_RUMI;
	pack.subheader = MINI_GAME_RUMI_SUBHEADER_GC_EXIT;

	GetDesc()->Packet(&pack, sizeof(pack));
}

void CHARACTER::MiniGameRumiHandCardClick(bool bAdd, uint8_t bIndex)
{
	if (bIndex + 1 > MINI_GAME_RUMI_HAND_SLOTS_MAX_NUM)
		return;

	if (m_MiniGameRumiHandCards[bIndex].bNumber == 0)
		return;

	if (MiniGameRumiGetFieldSpace() == -1)
		return;

	TPacketGCMiniGameRumi pack;
	pack.header = HEADER_GC_MINI_GAME_RUMI;
	pack.subheader = MINI_GAME_RUMI_SUBHEADER_GC_HAND_CARD_CLICK;
	TEMP_BUFFER buf;

	if (bAdd == true)
	{
		TMiniGameRumiMoveCard src, dst;

		src.bPos = MINI_GAME_RUMI_HAND_CARD;
		src.bIndex = bIndex;
		src.bColor = m_MiniGameRumiHandCards[bIndex].bColor;
		src.bNumber = m_MiniGameRumiHandCards[bIndex].bNumber;

		dst.bPos = MINI_GAME_RUMI_FIELD_CARD;
		dst.bIndex = MiniGameRumiGetFieldSpace();
		dst.bColor = m_MiniGameRumiHandCards[bIndex].bColor;
		dst.bNumber = m_MiniGameRumiHandCards[bIndex].bNumber;

		TMiniGameRumiMove table;
		table.src = src;
		table.dst = dst;

		buf.write(&table, sizeof(table));

		pack.size = sizeof(pack) + buf.size();

		GetDesc()->BufferedPacket(&pack, sizeof(pack));
		GetDesc()->Packet(buf.read_peek(), buf.size());

		m_MiniGameRumiFieldCards[MiniGameRumiGetFieldSpace()].bColor = m_MiniGameRumiHandCards[bIndex].bColor;
		m_MiniGameRumiHandCards[bIndex].bColor = 0;
		m_MiniGameRumiFieldCards[MiniGameRumiGetFieldSpace()].bNumber = m_MiniGameRumiHandCards[bIndex].bNumber;
		m_MiniGameRumiHandCards[bIndex].bNumber = 0;
	}
	else
	{
		TMiniGameRumiMoveCard src, dst;

		src.bPos = MINI_GAME_RUMI_HAND_CARD;
		src.bIndex = bIndex;
		src.bColor = m_MiniGameRumiHandCards[bIndex].bColor;
		src.bNumber = m_MiniGameRumiHandCards[bIndex].bNumber;

		dst.bPos = MINI_GAME_RUMI_NONE_POS;
		dst.bIndex = 0;
		dst.bColor = 0;
		dst.bNumber = 0;

		TMiniGameRumiMove table;
		table.src = src;
		table.dst = dst;

		buf.write(&table, sizeof(table));

		pack.size = sizeof(pack) + buf.size();

		GetDesc()->BufferedPacket(&pack, sizeof(pack));
		GetDesc()->Packet(buf.read_peek(), buf.size());
		m_MiniGameRumiHandCards[bIndex].bNumber = 0;
		m_MiniGameRumiHandCards[bIndex].bColor = 0;
	}

	if (MiniGameRumiGetFieldSpace() == -1)
	{
		if (MiniGameRumiCheckFieldCards())
			MiniGameRumiResetField();
		else
			for (uint8_t i = 0; i < MINI_GAME_RUMI_FIELD_SLOTS_MAX_NUM; ++i)
				MiniGameRumiFieldCardClick(i);
	}
}

int32_t CHARACTER::MiniGameRumiGetHandSpace()
{
	for (int32_t i = 0; i < MINI_GAME_RUMI_HAND_SLOTS_MAX_NUM; ++i)
		if (m_MiniGameRumiHandCards[i].bNumber == 0)
			return i;

	return -1;
}

uint8_t CHARACTER::MiniGameRumiHandCardSpaceCount()
{
	uint8_t bCount = 0;
	for (int32_t i = 0; i < MINI_GAME_RUMI_HAND_SLOTS_MAX_NUM; ++i)
		if (m_MiniGameRumiHandCards[i].bNumber == 0)
			bCount++;

	return bCount;
}

uint8_t CHARACTER::MiniGameRumiGetDeckSpace(uint8_t number, uint8_t color)
{
	for (int32_t i = 0; i < MINI_GAME_RUMI_DECK_CARDS_MAX_NUM; ++i)
		if (m_MiniGameRumiDeckCards[i].bNumber == number && m_MiniGameRumiDeckCards[i].bColor == color)
			return 0;

	return 1;
}

void CHARACTER::MiniGameRumiRandomDeck()
{
	for (int32_t i = 0; i < MINI_GAME_RUMI_DECK_CARDS_MAX_NUM; ++i)
	{
	bas:
		uint8_t number = number(1, 8);
		uint8_t color = number(1, 3);
		if (MiniGameRumiGetDeckSpace(number, color))
		{
			m_MiniGameRumiDeckCards[i].bNumber = number;
			m_MiniGameRumiDeckCards[i].bColor = color;
		}
		else
			goto bas;
	}

	m_MiniGameRumiCardDeckCount = MINI_GAME_RUMI_DECK_CARDS_MAX_NUM;
}

void CHARACTER::MiniGameRumiDeckCardClick()
{
	if (MiniGameRumiGetHandSpace() == -1 || MiniGameRumiFieldCardSpaceCount() < 3)
		return;

	if (m_MiniGameRumiCardDeckCount < 1)
		return;

	uint8_t bC = MiniGameRumiHandCardSpaceCount();
	for (int32_t i = 0; i < bC; ++i)
	{
		if (m_MiniGameRumiCardDeckCount < 1)
			return;

		TPacketGCMiniGameRumi pack;
		pack.header = HEADER_GC_MINI_GAME_RUMI;
		pack.subheader = MINI_GAME_RUMI_SUBHEADER_GC_DECK_CARD_CLICK;

		TMiniGameRumiMoveCard src, dst;

		src.bPos = MINI_GAME_RUMI_DECK_CARD;
		src.bIndex = m_MiniGameRumiCardDeckCount;
		src.bColor = m_MiniGameRumiDeckCards[m_MiniGameRumiCardDeckCount - 1].bColor;
		src.bNumber = m_MiniGameRumiDeckCards[m_MiniGameRumiCardDeckCount - 1].bNumber;

		dst.bPos = MINI_GAME_RUMI_HAND_CARD;
		dst.bIndex = MiniGameRumiGetHandSpace();
		dst.bColor = m_MiniGameRumiDeckCards[m_MiniGameRumiCardDeckCount - 1].bColor;
		dst.bNumber = m_MiniGameRumiDeckCards[m_MiniGameRumiCardDeckCount - 1].bNumber;

		TMiniGameRumiMove table;
		table.src = src;
		table.dst = dst;

		TEMP_BUFFER buf;
		buf.write(&table, sizeof(table));

		pack.size = sizeof(pack) + buf.size();

		m_MiniGameRumiHandCards[MiniGameRumiGetHandSpace()].bColor = m_MiniGameRumiDeckCards[m_MiniGameRumiCardDeckCount - 1].bColor;
		m_MiniGameRumiHandCards[MiniGameRumiGetHandSpace()].bNumber = m_MiniGameRumiDeckCards[m_MiniGameRumiCardDeckCount - 1].bNumber;

		GetDesc()->BufferedPacket(&pack, sizeof(pack));
		GetDesc()->Packet(buf.read_peek(), buf.size());

		m_MiniGameRumiCardDeckCount--;
	}
}

void CHARACTER::MiniGameRumiFieldCardClick(uint8_t bIndex)
{
	if (m_MiniGameRumiFieldCards[bIndex].bColor == 0 && m_MiniGameRumiFieldCards[bIndex].bNumber == 0)
		return;

	TPacketGCMiniGameRumi pack;
	pack.header = HEADER_GC_MINI_GAME_RUMI;
	pack.subheader = MINI_GAME_RUMI_SUBHEADER_GC_FIELD_CARD_CLICK;

	TEMP_BUFFER buf;

	TMiniGameRumiMoveCard src, dst;

	src.bPos = MINI_GAME_RUMI_FIELD_CARD;
	src.bIndex = bIndex;
	src.bColor = m_MiniGameRumiFieldCards[bIndex].bColor;
	src.bNumber = m_MiniGameRumiFieldCards[bIndex].bNumber;

	dst.bPos = MINI_GAME_RUMI_HAND_CARD;
	dst.bIndex = MiniGameRumiGetHandSpace();
	dst.bColor = m_MiniGameRumiFieldCards[bIndex].bColor;
	dst.bNumber = m_MiniGameRumiFieldCards[bIndex].bNumber;

	TMiniGameRumiMove table;
	table.src = src;
	table.dst = dst;

	buf.write(&table, sizeof(table));

	pack.size = sizeof(pack) + buf.size();

	GetDesc()->BufferedPacket(&pack, sizeof(pack));
	GetDesc()->Packet(buf.read_peek(), buf.size());

	m_MiniGameRumiHandCards[MiniGameRumiGetHandSpace()].bColor = m_MiniGameRumiFieldCards[bIndex].bColor;
	m_MiniGameRumiFieldCards[bIndex].bColor = 0;

	m_MiniGameRumiHandCards[MiniGameRumiGetHandSpace()].bNumber = m_MiniGameRumiFieldCards[bIndex].bNumber;
	m_MiniGameRumiFieldCards[bIndex].bNumber = 0;
}

bool CHARACTER::MiniGameRumiFieldCardsMatch()
{
	if (m_MiniGameRumiFieldCards[0].bNumber == m_MiniGameRumiFieldCards[1].bNumber - 1 && m_MiniGameRumiFieldCards[1].bNumber == m_MiniGameRumiFieldCards[2].bNumber - 1)
		return true;
	if (m_MiniGameRumiFieldCards[0].bNumber == m_MiniGameRumiFieldCards[2].bNumber - 1 && m_MiniGameRumiFieldCards[2].bNumber == m_MiniGameRumiFieldCards[1].bNumber - 1)
		return true;
	if (m_MiniGameRumiFieldCards[1].bNumber == m_MiniGameRumiFieldCards[0].bNumber - 1 && m_MiniGameRumiFieldCards[0].bNumber == m_MiniGameRumiFieldCards[2].bNumber - 1)
		return true;
	if (m_MiniGameRumiFieldCards[0].bNumber == m_MiniGameRumiFieldCards[1].bNumber - 1 && m_MiniGameRumiFieldCards[2].bNumber == m_MiniGameRumiFieldCards[0].bNumber - 1)
		return true;
	if (m_MiniGameRumiFieldCards[1].bNumber == m_MiniGameRumiFieldCards[0].bNumber - 1 && m_MiniGameRumiFieldCards[2].bNumber == m_MiniGameRumiFieldCards[1].bNumber - 1)
		return true;
	if (m_MiniGameRumiFieldCards[1].bNumber == m_MiniGameRumiFieldCards[2].bNumber - 1 && m_MiniGameRumiFieldCards[2].bNumber == m_MiniGameRumiFieldCards[0].bNumber - 1)
		return true;
	if (m_MiniGameRumiFieldCards[0].bNumber == m_MiniGameRumiFieldCards[1].bNumber && m_MiniGameRumiFieldCards[1].bNumber == m_MiniGameRumiFieldCards[2].bNumber)
		return true;

	return false;
}

void CHARACTER::MiniGameRumiResetField()
{
	for (uint8_t i = 0; i < MINI_GAME_RUMI_FIELD_SLOTS_MAX_NUM; ++i)
	{
		m_MiniGameRumiFieldCards[i].bColor = 0;
		m_MiniGameRumiFieldCards[i].bNumber = 0;
	}
}

int32_t CHARACTER::MiniGameRumiGetFieldSpace()
{
	for (int32_t i = 0; i < MINI_GAME_RUMI_FIELD_SLOTS_MAX_NUM; ++i)
		if (m_MiniGameRumiFieldCards[i].bNumber == 0)
			return i;

	return -1;
}

uint8_t CHARACTER::MiniGameRumiFieldCardSpaceCount()
{
	uint8_t bCount = 0;
	for (int32_t i = 0; i < MINI_GAME_RUMI_FIELD_SLOTS_MAX_NUM; ++i)
		if (m_MiniGameRumiFieldCards[i].bNumber == 0)
			bCount++;

	return bCount;
}

bool CHARACTER::MiniGameRumiCheckFieldCards()
{
	uint8_t bScore;

	if ((m_MiniGameRumiFieldCards[0].bColor == m_MiniGameRumiFieldCards[1].bColor && m_MiniGameRumiFieldCards[1].bColor == m_MiniGameRumiFieldCards[2].bColor) && (m_MiniGameRumiFieldCards[0].bNumber == m_MiniGameRumiFieldCards[1].bNumber && m_MiniGameRumiFieldCards[1].bNumber == m_MiniGameRumiFieldCards[2].bNumber))
	{
		m_MiniGameRumiPoints += 150;
		bScore = 150;
	}
	else if ((m_MiniGameRumiFieldCards[0].bColor == m_MiniGameRumiFieldCards[1].bColor && m_MiniGameRumiFieldCards[1].bColor == m_MiniGameRumiFieldCards[2].bColor) && MiniGameRumiFieldCardsMatch())
	{
		m_MiniGameRumiPoints += 100;
		bScore = 100;
	}
	else if (m_MiniGameRumiFieldCards[0].bColor == m_MiniGameRumiFieldCards[1].bColor && m_MiniGameRumiFieldCards[1].bColor == m_MiniGameRumiFieldCards[2].bColor)
	{
		m_MiniGameRumiPoints += MIN(m_MiniGameRumiFieldCards[0].bNumber, MIN(m_MiniGameRumiFieldCards[1].bNumber, m_MiniGameRumiFieldCards[2].bNumber)) * 10 + 10;
		bScore = MIN(m_MiniGameRumiFieldCards[0].bNumber, MIN(m_MiniGameRumiFieldCards[1].bNumber, m_MiniGameRumiFieldCards[2].bNumber)) * 10 + 10;
	}
	else if (MiniGameRumiFieldCardsMatch())
	{
		m_MiniGameRumiPoints += MIN(m_MiniGameRumiFieldCards[0].bNumber, MIN(m_MiniGameRumiFieldCards[1].bNumber, m_MiniGameRumiFieldCards[2].bNumber)) * 10;
		bScore = MIN(m_MiniGameRumiFieldCards[0].bNumber, MIN(m_MiniGameRumiFieldCards[1].bNumber, m_MiniGameRumiFieldCards[2].bNumber)) * 10;
	}
	else
		bScore = 0;

	if (bScore)
	{
		TPacketGCMiniGameRumi pack;
		pack.header = HEADER_GC_MINI_GAME_RUMI;
		pack.subheader = MINI_GAME_RUMI_SUBHEADER_GC_SCORE_INCREASE;
		TMiniGameRumiScore table;
		table.bScore = bScore;
		table.sTotalScore = m_MiniGameRumiPoints;

		TEMP_BUFFER buf;
		buf.write(&table, sizeof(table));

		pack.size = sizeof(pack) + buf.size();

		GetDesc()->BufferedPacket(&pack, sizeof(pack));
		GetDesc()->Packet(buf.read_peek(), buf.size());

		return true;
	}

	return false;
}
#endif

#ifdef __MINI_GAME_FISH__
void CHARACTER::FishEventRequestBlock()
{
	if (!GetDesc())
		return;

	TPacketGCFishEvent packFishEvent;
	packFishEvent.bHeader = HEADER_GC_FISH_EVENT;
	packFishEvent.bSubheader = FISH_EVENT_SUBHEADER_GC_COUNT;
	packFishEvent.dwFirstArg = GetFishEventUseCount();
	packFishEvent.dwSecondArg = 0;
	GetDesc()->Packet(&packFishEvent, sizeof(TPacketGCFishEvent));

	if (!GetFishEventUseCount())
	{
		for (int32_t i = 0; i < FISH_EVENT_SLOTS_NUM; i++)
		{
			m_fishSlots[i].bIsMain = false;
			m_fishSlots[i].bShape = 0;
		}
	}

	for (int32_t i = 0; i < FISH_EVENT_SLOTS_NUM; i++)
	{
		if (m_fishSlots[i].bIsMain)
		{
			TPacketGCFishEvent packFishEvent;
			packFishEvent.bHeader = HEADER_GC_FISH_EVENT;
			packFishEvent.bSubheader = FISH_EVENT_SUBHEADER_GC_SHAPE_ADD;
			packFishEvent.dwFirstArg = i;
			packFishEvent.dwSecondArg = m_fishSlots[i].bShape;
			GetDesc()->Packet(&packFishEvent, sizeof(TPacketGCFishEvent));
		}
	}
}

void CHARACTER::FishEventBoxUse(TItemPos itemPos)
{
	if (itemPos.window_type != INVENTORY)
		return;

	if (!GetDesc())
		return;

	LPITEM item;

	if (!CanHandleItem())
		return;

	if (!IsValidItemPosition(itemPos) || !(item = GetItem(itemPos)))
		return;

	if (item->IsExchanging())
		return;

	if (item->GetVnum() == ITEM_FISH_EVENT_BOX)
	{
		uint8_t randomShape = number(FISH_EVENT_SHAPE_1, FISH_EVENT_SHAPE_6);
		SetFishAttachedShape(randomShape);
		FishEventIncreaseUseCount();
		item->SetCount(item->GetCount() - 1);
	}
	else if (item->GetVnum() == ITEM_FISH_EVENT_BOX_SPECIAL)
	{
		SetFishAttachedShape(FISH_EVENT_SHAPE_7);
		FishEventIncreaseUseCount();
		item->SetCount(item->GetCount() - 1);
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can not use this item here."));
		return;
	}

	TPacketGCFishEvent packFishEvent;
	packFishEvent.bHeader = HEADER_GC_FISH_EVENT;
	packFishEvent.bSubheader = FISH_EVENT_SUBHEADER_GC_BOX_USE;
	packFishEvent.dwFirstArg = GetFishAttachedShape();
	packFishEvent.dwSecondArg = GetFishEventUseCount();
	GetDesc()->Packet(&packFishEvent, sizeof(TPacketGCFishEvent));
}

bool CHARACTER::FishEventIsValidPosition(uint8_t shapePos, uint8_t shapeType)
{
	bool finalReturn = false;

	uint8_t positionList[7][8] = {
		{ FISH_EVENT_SHAPE_1, 12, 0, 6, 12, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_2, 24, 0, 0, 0, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_3, 17, 0, 6, 7, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_4, 17, 0, 1, 7, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_5, 17, 0, 1, 6, 7, 0, 0 },
		{ FISH_EVENT_SHAPE_6, 16, 0, 1, 7, 8, 0, 0 },
		{ FISH_EVENT_SHAPE_7, 16, 0, 1, 2, 6, 7, 8 },
	};

	for (size_t i = 0; i < sizeof(positionList) / sizeof(positionList[0]); i++)
	{
		if (positionList[i][0] == shapeType)
		{
			for (int32_t j = 0; j < positionList[i][1]; j++)
			{
				if (shapePos == j
					&& m_fishSlots[j + positionList[i][2]].bShape == 0
					&& m_fishSlots[j + positionList[i][3]].bShape == 0
					&& m_fishSlots[j + positionList[i][4]].bShape == 0
					&& m_fishSlots[j + positionList[i][5]].bShape == 0
					&& m_fishSlots[j + positionList[i][6]].bShape == 0
					&& m_fishSlots[j + positionList[i][7]].bShape == 0)
				{
					finalReturn = true;
					break;
				}
			}

			break;
		}
	}

	return finalReturn;
}

void CHARACTER::FishEventPlaceShape(uint8_t shapePos, uint8_t shapeType)
{
	uint8_t positionList[7][7] = {
		{ FISH_EVENT_SHAPE_1, 0, 6, 12, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_2, 0, 0, 0, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_3, 0, 6, 7, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_4, 0, 1, 7, 0, 0, 0 },
		{ FISH_EVENT_SHAPE_5, 0, 1, 6, 7, 0, 0 },
		{ FISH_EVENT_SHAPE_6, 0, 1, 7, 8, 0, 0 },
		{ FISH_EVENT_SHAPE_7, 0, 1, 2, 6, 7, 8 },
	};

	for (size_t i = 0; i < sizeof(positionList) / sizeof(positionList[0]); i++)
	{
		if (positionList[i][0] == shapeType)
		{
			for (int32_t j = 1; j < 7; j++)
			{
				if (j > 1 && positionList[i][j] == 0)
					continue;

				if (positionList[i][j] == 0)
				{
					m_fishSlots[shapePos].bIsMain = true;
					m_fishSlots[shapePos].bShape = shapeType;
				}
				else
				{
					m_fishSlots[shapePos + positionList[i][j]].bIsMain = false;
					m_fishSlots[shapePos + positionList[i][j]].bShape = shapeType;
				}
			}

			break;
		}
	}
}

void CHARACTER::FishEventCheckEnd()
{
	bool isComplete = true;

	for (int32_t i = 0; i < FISH_EVENT_SLOTS_NUM; i++)
	{
		if (m_fishSlots[i].bShape == 0)
		{
			isComplete = false;
			break;
		}
	}

	if (isComplete)
	{
		uint32_t dwUseCount = GetFishEventUseCount();
		uint32_t dwRewardVnum = dwUseCount <= 10 ? 83003 : (dwUseCount <= 24 ? 83002 : 83001);

		for (int32_t i = 0; i < FISH_EVENT_SLOTS_NUM; i++)
		{
			m_fishSlots[i].bIsMain = false;
			m_fishSlots[i].bShape = 0;
		}

		AutoGiveItem(dwRewardVnum);
		PointChange(POINT_EXP, 30000);

		m_dwFishUseCount = 0;
		SetFishAttachedShape(0);

		TPacketGCFishEvent packFishEvent;
		packFishEvent.bHeader = HEADER_GC_FISH_EVENT;
		packFishEvent.bSubheader = FISH_EVENT_SUBHEADER_GC_REWARD;
		packFishEvent.dwFirstArg = dwRewardVnum;
		packFishEvent.dwSecondArg = 0;
		GetDesc()->Packet(&packFishEvent, sizeof(TPacketGCFishEvent));
	}
}

void CHARACTER::FishEventShapeAdd(uint8_t shapePos)
{
	if (!GetDesc())
		return;

	if (shapePos >= FISH_EVENT_SLOTS_NUM)
		return;

	uint8_t lastAttachedShape = GetFishAttachedShape();

	if (!lastAttachedShape)
		return;

	if (!FishEventIsValidPosition(shapePos, lastAttachedShape))
	{
		TPacketGCFishEvent packFishEvent;
		packFishEvent.bHeader = HEADER_GC_FISH_EVENT;
		packFishEvent.bSubheader = FISH_EVENT_SUBHEADER_GC_BOX_USE;
		packFishEvent.dwFirstArg = GetFishAttachedShape();
		packFishEvent.dwSecondArg = GetFishEventUseCount();
		GetDesc()->Packet(&packFishEvent, sizeof(TPacketGCFishEvent));

		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("That shape can not fit in this position."));
		return;
	}

	FishEventPlaceShape(shapePos, lastAttachedShape);

	TPacketGCFishEvent packFishEvent;
	packFishEvent.bHeader = HEADER_GC_FISH_EVENT;
	packFishEvent.bSubheader = FISH_EVENT_SUBHEADER_GC_SHAPE_ADD;
	packFishEvent.dwFirstArg = shapePos;
	packFishEvent.dwSecondArg = lastAttachedShape;
	GetDesc()->Packet(&packFishEvent, sizeof(TPacketGCFishEvent));

	FishEventCheckEnd();
}
#endif

#ifdef __MINI_GAME_CATCH_KING__
void CHARACTER::MiniGameCatchKingStartGame(uint8_t bSetCount)
{
	if (!GetDesc() || MiniGameCatchKingGetGameStatus())
		return;

	if (bSetCount < 1 || bSetCount > 5)
	{
		ChatPacket(CHAT_TYPE_INFO, "Numarul de seturi pariate este invalid, incearca din nou.");
		return;
	}

	if (GetGold() < (30000 * bSetCount))
	{
		ChatPacket(CHAT_TYPE_INFO, "Nu ai suficient yang pentru a incepe acest joc.");
		return;
	}

	if (CountSpecifyItem(79604) < bSetCount)
	{
		ChatPacket(CHAT_TYPE_INFO, "Nu ai suficiente seturi pentru a incepe acest joc.");
		return;
	}

	RemoveSpecifyItem(79604, bSetCount);
	ChangeGold(false, (30000 * bSetCount));

	std::vector<TCatchKingCard> m_vecFieldCards;

	std::srand(unsigned(time(0)));

	for (int32_t i = 0; i < 25; i++)
	{
		if (i >= 0 && i < 7)
			m_vecFieldCards.push_back(TCatchKingCard(1, false));
		else if (i >= 7 && i < 11)
			m_vecFieldCards.push_back(TCatchKingCard(2, false));
		else if (i >= 11 && i < 16)
			m_vecFieldCards.push_back(TCatchKingCard(3, false));
		else if (i >= 16 && i < 21)
			m_vecFieldCards.push_back(TCatchKingCard(4, false));
		else if (i >= 21 && i < 24)
			m_vecFieldCards.push_back(TCatchKingCard(5, false));
		else if (i >= 24)
			m_vecFieldCards.push_back(TCatchKingCard(6, false)); // 6 = K
	}

	std::random_shuffle(m_vecFieldCards.begin(), m_vecFieldCards.end());

	MiniGameCatchKingSetFieldCards(m_vecFieldCards);

	MiniGameCatchKingSetBetNumber(bSetCount);
	MiniGameCatchKingSetHandCardLeft(12);
	MiniGameCatchKingSetGameStatus(true);

	uint32_t dwBigScore = MiniGameCatchKingGetMyScore();

	TPacketGCMiniGameCatchKing packet;
	packet.bHeader = HEADER_GC_MINI_GAME_CATCH_KING;
	packet.bSubheader = SUBHEADER_GC_CATCH_KING_START;

	packet.wSize = sizeof(packet) + sizeof(dwBigScore);

	GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameCatchKing));
	GetDesc()->Packet(&dwBigScore, sizeof(uint32_t));
}

void CHARACTER::MiniGameCatchKingDeckCardClick()
{
	if (!GetDesc() || !MiniGameCatchKingGetGameStatus() || MiniGameCatchKingGetHandCard())
		return;

	uint8_t bCardLeft = MiniGameCatchKingGetHandCardLeft();

	if (bCardLeft)
	{
		if (bCardLeft <= 12 && bCardLeft > 7)
			MiniGameCatchKingSetHandCard(1);
		else if (bCardLeft <= 7 && bCardLeft > 5)
			MiniGameCatchKingSetHandCard(2);
		else if (bCardLeft <= 5 && bCardLeft > 3)
			MiniGameCatchKingSetHandCard(3);
		else if (bCardLeft == 3)
			MiniGameCatchKingSetHandCard(4);
		else if (bCardLeft == 2)
			MiniGameCatchKingSetHandCard(5);
		else if (bCardLeft == 1)
			MiniGameCatchKingSetHandCard(6);
	}
	else
		return;

	uint8_t bCardInHand = MiniGameCatchKingGetHandCard();

	if (!bCardInHand)
		return;

	MiniGameCatchKingSetHandCardLeft(bCardLeft - 1);

	TPacketGCMiniGameCatchKing packet;
	packet.bHeader = HEADER_GC_MINI_GAME_CATCH_KING;
	packet.bSubheader = SUBHEADER_GC_CATCH_KING_SET_CARD;

	packet.wSize = sizeof(packet) + sizeof(bCardInHand);

	GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameCatchKing));
	GetDesc()->Packet(&bCardInHand, sizeof(uint8_t));
}

void CHARACTER::MiniGameCatchKingFieldCardClick(uint8_t bFieldPos)
{
	if (!GetDesc() || !MiniGameCatchKingGetGameStatus() || bFieldPos > 24)
		return;

	uint8_t bHandCard = MiniGameCatchKingGetHandCard();
	TCatchKingCard filedCard = m_vecCatchKingFieldCards[bFieldPos];

	if (!bHandCard)
		return;

	if (filedCard.bIsExposed == true)
		return;

	uint32_t dwPoints = 0;
	bool bDestroyCard = false;
	bool bKeepFieldCard = false;
	bool bGetReward = false;

	if (bHandCard < 5)
	{
		if (bHandCard < filedCard.bIndex)
		{
			dwPoints = 0;
			bDestroyCard = true;
			bKeepFieldCard = false;
		}
		else if (bHandCard == filedCard.bIndex)
		{
			dwPoints = filedCard.bIndex * 10;
			bDestroyCard = true;
			bKeepFieldCard = true;
		}
		else
		{
			dwPoints = filedCard.bIndex * 10;
			bDestroyCard = false;
			bKeepFieldCard = true;
		}
	}

	int32_t checkPos[8];
	checkPos[0] = bFieldPos - 5;
	checkPos[1] = bFieldPos + 5;
	checkPos[2] = (bFieldPos % 10 == 4 || bFieldPos % 10 == 9) ? -1 : (bFieldPos + 1);
	checkPos[3] = (bFieldPos % 10 == 0 || bFieldPos % 10 == 5) ? -1 : (bFieldPos - 1);
	checkPos[4] = (bFieldPos % 10 == 4 || bFieldPos % 10 == 9) ? -1 : (bFieldPos - 5 + 1);
	checkPos[5] = (bFieldPos % 10 == 4 || bFieldPos % 10 == 9) ? -1 : (bFieldPos + 5 + 1);
	checkPos[6] = (bFieldPos % 10 == 0 || bFieldPos % 10 == 5) ? -1 : (bFieldPos - 5 - 1);
	checkPos[7] = (bFieldPos % 10 == 0 || bFieldPos % 10 == 5) ? -1 : (bFieldPos + 5 - 1);

	bool isFiveNearby = false;

	for (int32_t i = 0; i < 25; i++)
	{
		if (isFiveNearby)
			break;

		for (size_t j = 0; j < sizeof(checkPos) / sizeof(checkPos[0]); j++)
		{
			if (checkPos[j] < 0 || checkPos[j] >= 25)
				continue;

			if (i == checkPos[j] && m_vecCatchKingFieldCards[i].bIndex == 5)
			{
				isFiveNearby = true;
				break;
			}
		}
	}

	if (bHandCard == 5)
	{
		if (isFiveNearby)
		{
			dwPoints = 0;
			bDestroyCard = true;
			bKeepFieldCard = (bHandCard >= filedCard.bIndex) ? true : false;
		}
		else
		{
			dwPoints = (bHandCard >= filedCard.bIndex) ? filedCard.bIndex * 10 : 0;
			bDestroyCard = (bHandCard > filedCard.bIndex) ? false : true;
			bKeepFieldCard = (bHandCard >= filedCard.bIndex) ? true : false;
		}
	}

	if (bHandCard == 6)
	{
		dwPoints = (bHandCard == filedCard.bIndex) ? 100 : 0;
		bDestroyCard = true;
		bKeepFieldCard = (bHandCard == filedCard.bIndex) ? true : false;
	}

	if (bKeepFieldCard)
		m_vecCatchKingFieldCards[bFieldPos].bIsExposed = true;

	int32_t checkRowPos[4];
	checkRowPos[0] = 5 * (bFieldPos / 5);
	checkRowPos[1] = 4 + (5 * (bFieldPos / 5));
	checkRowPos[2] = bFieldPos - (5 * (bFieldPos / 5));
	checkRowPos[3] = bFieldPos + 20 - (5 * (bFieldPos / 5));

	bool isHorizontalRow = true;
	bool isVerticalRow = true;

	for (int32_t row = checkRowPos[0]; row <= checkRowPos[1]; row += 1)
	{
		if (!m_vecCatchKingFieldCards[row].bIsExposed)
		{
			isHorizontalRow = false;
			break;
		}
	}

	for (int32_t col = checkRowPos[2]; col <= checkRowPos[3]; col += 5)
	{
		if (!m_vecCatchKingFieldCards[col].bIsExposed)
		{
			isVerticalRow = false;
			break;
		}
	}

	dwPoints += isHorizontalRow ? 10 : 0;
	dwPoints += isVerticalRow ? 10 : 0;

	if (dwPoints)
		MiniGameCatchKingSetScore(MiniGameCatchKingGetScore() + dwPoints);

	bool isTheEnd = false;

	if (bDestroyCard)
	{
		bGetReward = (bHandCard == 6 && MiniGameCatchKingGetScore() >= 10) ? true : false;
		isTheEnd = (bHandCard == 6) ? true : false;
		MiniGameCatchKingSetHandCard(0);
	}

	uint8_t bRowType = 0;
	if (isHorizontalRow && !isVerticalRow)
		bRowType = 1;
	else if (!isHorizontalRow && isVerticalRow)
		bRowType = 2;
	else if (isHorizontalRow && isVerticalRow)
		bRowType = 3;

	TPacketGCMiniGameCatchKing packet;
	packet.bHeader = HEADER_GC_MINI_GAME_CATCH_KING;
	packet.bSubheader = SUBHEADER_GC_CATCH_KING_RESULT_FIELD;

	TPacketGCMiniGameCatchKingResult packetSecond;
	packetSecond.dwPoints = MiniGameCatchKingGetScore();
	packetSecond.bRowType = bRowType;
	packetSecond.bCardPos = bFieldPos;
	packetSecond.bCardValue = filedCard.bIndex;
	packetSecond.bKeepFieldCard = bKeepFieldCard;
	packetSecond.bDestroyHandCard = bDestroyCard;
	packetSecond.bGetReward = bGetReward;
	packetSecond.bIsFiveNearBy = isFiveNearby;

	packet.wSize = sizeof(packet) + sizeof(packetSecond);

	GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameCatchKing));
	GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameCatchKingResult));

	if (isTheEnd)
	{
		for (int32_t i = 0; i < 25; i++)
		{
			if (!m_vecCatchKingFieldCards[i].bIsExposed)
			{
				TPacketGCMiniGameCatchKing packet;
				packet.bHeader = HEADER_GC_MINI_GAME_CATCH_KING;
				packet.bSubheader = SUBHEADER_GC_CATCH_KING_SET_END_CARD;

				TPacketGCMiniGameCatchKingSetEndCard packetSecond;
				packetSecond.bCardPos = i;
				packetSecond.bCardValue = m_vecCatchKingFieldCards[i].bIndex;

				packet.wSize = sizeof(packet) + sizeof(packetSecond);

				GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameCatchKing));
				GetDesc()->Packet(&packetSecond, sizeof(TPacketGCMiniGameCatchKingSetEndCard));
			}
		}
	}
}

void CHARACTER::MiniGameCatchKingGetReward()
{
	if (!GetDesc() || !MiniGameCatchKingGetGameStatus() || MiniGameCatchKingGetHandCard() || MiniGameCatchKingGetHandCardLeft())
		return;

	uint32_t dwRewardVnum = 0;
	uint8_t bReturnCode = 0;
	uint32_t dwScore = MiniGameCatchKingGetScore();

	if (dwScore >= 10 && dwScore < 400)
		dwRewardVnum = 50930;
	else if (dwScore >= 400 && dwScore < 550)
		dwRewardVnum = 50929;
	else if (dwScore >= 550)
		dwRewardVnum = 50928;

	MiniGameCatchKingRegisterScore(dwScore);

	if (dwRewardVnum)
	{
		MiniGameCatchKingSetScore(0);
		AutoGiveItem(dwRewardVnum, MiniGameCatchKingGetBetNumber());
		MiniGameCatchKingSetBetNumber(0);
		MiniGameCatchKingSetGameStatus(false);
		m_vecCatchKingFieldCards.clear();

		bReturnCode = 0;
	}
	else
		bReturnCode = 1;

	TPacketGCMiniGameCatchKing packet;
	packet.bHeader = HEADER_GC_MINI_GAME_CATCH_KING;
	packet.bSubheader = SUBHEADER_GC_CATCH_KING_REWARD;

	packet.wSize = sizeof(packet) + sizeof(bReturnCode);

	GetDesc()->BufferedPacket(&packet, sizeof(TPacketGCMiniGameCatchKing));
	GetDesc()->Packet(&bReturnCode, sizeof(uint8_t));
}

void CHARACTER::MiniGameCatchKingRegisterScore(uint32_t dwScore)
{
	if (!GetDesc())
		return;

	char querySelect[256];

	snprintf(querySelect, sizeof(querySelect),
		"SELECT max_score FROM catck_king_event WHERE name = '%s' LIMIT 1;", GetName());

	std::unique_ptr<SQLMsg> pSelectMsg(DBManager::instance().DirectQuery(querySelect));
	SQLResult* resSelect = pSelectMsg->Get();

	if (resSelect && resSelect->uiNumRows > 0)
	{
		uint32_t maxScore = 0;
		MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);
		str_to_number(maxScore, row[0]);

		if (dwScore > maxScore)
			DBManager::instance().DirectQuery("UPDATE catck_king_event SET max_score = %d, total_score = total_score + %d WHERE name = '%s';", dwScore, dwScore, GetName());
		else
			DBManager::instance().DirectQuery("UPDATE catck_king_event SET total_score = total_score + %d WHERE name = '%s';", dwScore, GetName());
	}
	else
		DBManager::instance().DirectQuery("REPLACE INTO catck_king_event (name, empire, max_score, total_score) VALUES ('%s', %d, %d, %d);",
			GetName(), GetEmpire(), dwScore, dwScore);
}

int32_t CHARACTER::MiniGameCatchKingGetMyScore()
{
	if (!GetDesc())
		return 0;

	char querySelect[256];

	snprintf(querySelect, sizeof(querySelect), "SELECT max_score FROM catck_king_event WHERE name = '%s' LIMIT 1;", GetName());

	std::unique_ptr<SQLMsg> pSelectMsg(DBManager::instance().DirectQuery(querySelect));
	SQLResult* resSelect = pSelectMsg->Get();

	if (resSelect && resSelect->uiNumRows > 0)
	{
		uint32_t dwScore = 0;
		MYSQL_ROW row = mysql_fetch_row(resSelect->pSQLResult);

		str_to_number(dwScore, row[0]);

		return dwScore;
	}

	return 0;
}
#endif

#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
void CHARACTER::RewardAttendance()
{
	for (TDamageMap::iterator it = m_map_kDamage.begin(); it != m_map_kDamage.end(); ++it)
	{
		int32_t iDamage = it->second.iTotalDamage;
		if (iDamage > 0)
		{
			LPCHARACTER ch = CHARACTER_MANAGER::instance().Find(it->first);

			if (ch)
			{
#ifdef __ACCUMULATE_DAMAGE_DISPLAY__
				uint32_t dwCount = 0;

				std::string flagNames[5] = {
					"attendance_event.red_dragon_hit_points",
					"attendance_event.jotun_thrym_hit_points",
					"attendance_event.razador_hit_points",
					"attendance_event.nemere_hit_points",
					"attendance_event.beran_hit_points",
				};

				if (!ch->m_Accumulate.empty())
				{
					for (uint32_t i = 0; i < ch->m_Accumulate.size(); i++)
					{
						if (ch->m_Accumulate[i].dwVid == GetVID())
						{
							dwCount = ch->m_Accumulate[i].dwCount;
							break;
						}
					}
				}

				if (dwCount)
#endif
				{
					if (ch->GetLevel() < 30)
						continue;

					if (!ch->FindAffect(AFFECT_EXP_BONUS_EVENT))
						ch->AddAffect(AFFECT_EXP_BONUS_EVENT, POINT_EXP, 20, 0, 1800, 0, false);

					if (ch->FindAffect(AFFECT_ATT_SPEED_SLOW))
						ch->RemoveAffect(AFFECT_ATT_SPEED_SLOW);

#ifdef __ACCUMULATE_DAMAGE_DISPLAY__				
					ch->SetQuestFlag(flagNames[6419 - GetRaceNum()], ch->GetQuestFlag(flagNames[6419 - GetRaceNum()]) + dwCount);
#endif

					time_t iTime;
					time(&iTime);
					tm* pTimeInfo = localtime(&iTime);
					char szFlagname[32 + 1];
					snprintf(szFlagname, sizeof(szFlagname), "attendance.clear_day_%d", pTimeInfo->tm_yday);

					if (ch->GetQuestFlag(szFlagname) == 0)
					{
						ch->SetQuestFlag(szFlagname, 1);
						ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You have earned your daily target - collect your reward."));

						TPacketGCAttendanceRequestData pack;
						pack.bHeader = HEADER_GC_ATTENDANCE_REQUEST_DATA;
						pack.bType = ATTENDANCE_DATA_TYPE_MISSION_CLEAR;
						pack.bValue = 1;
						ch->GetDesc()->Packet(&pack, sizeof(TPacketGCAttendanceRequestData));
					}
				}
			}
		}
	}
}
#endif

#ifdef __12ZI__
uint32_t CHARACTER::GetLastPlay()
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT UNIX_TIMESTAMP(last_play) FROM player.player WHERE name = '%s'", GetName()));

	if (pMsg->Get()->uiNumRows)
	{
		MYSQL_ROW row = mysql_fetch_row(pMsg->Get()->pSQLResult);
		return atoi(row[0]);
	}
	else
		return 0;
}

EVENTINFO(bead_event_info)
{
	LPCHARACTER	ch;

	bead_event_info() : ch(nullptr)
	{

	}
};

EVENTFUNC(bead_event)
{
	bead_event_info* info = dynamic_cast<bead_event_info*>(event->info);

	if (!info)
	{
		sys_err("bead_event> <Factor> Null pointer");
		return 0;
	}

	if (!info->ch)
		return 0;

	info->ch->SetBead(info->ch->GetBead() + 1, true);

	if (info->ch->GetBead() >= 36)
	{
		info->ch->SetBeadTime(0, true);
		return 0;
	}
	else
	{
		info->ch->SetBeadTime(3600, true);
		return PASSES_PER_SEC(3600);
	}
}

void CHARACTER::StartBeadEvent()
{
	if (m_pkBeadEvent)
		return;

	bead_event_info* info = AllocEventInfo<bead_event_info>();
	info->ch = this;

	m_pkBeadEvent = event_create(bead_event, info, PASSES_PER_SEC(GetBeadTime()));
}
#endif

bool CHARACTER::IsBoss()
{
	return m_pkMobData ? m_pkMobData->m_table.bRank > 3 && IsMonster() : false;
}

#ifdef WJ_SECURITY_SYSTEM
bool CHARACTER::bIsActivate()
{
	if (!IsPC())
		return false;

	uint32_t currTime = time(nullptr);
	return currTime >= uint32_t(GetQuestFlag("sec.cpt"));
}

void CHARACTER::SendSecurityGui()
{
	if (!IsActivateSecurityGui())
	{
		int32_t sayi1 = number(1, 9);
		int32_t sayi2 = number(1, 9);
		int32_t sayi3 = number(1, 9);
		int32_t sayi4 = number(1, 9);
		int32_t sayi5 = number(1, 9);
		m_captcha = (sayi1 * 10000) + (sayi2 * 1000) + (sayi3 * 100) + (sayi4 * 10) + sayi5;
		ChatPacket(CHAT_TYPE_COMMAND, "captcha %d %d %d %d %d", sayi1, sayi2, sayi3, sayi4, sayi5);
		PrepareSecurityGui(true);
	}
}
#endif

