#pragma once
#include "GameType.h"
class CPythonNonPlayer : public CSingleton<CPythonNonPlayer>
{
	public:
		enum  EClickEvent
		{
			ON_CLICK_EVENT_NONE		= 0,
			ON_CLICK_EVENT_BATTLE	= 1,
			ON_CLICK_EVENT_SHOP		= 2,
			ON_CLICK_EVENT_TALK		= 3,
			ON_CLICK_EVENT_VEHICLE	= 4,

			ON_CLICK_EVENT_MAX_NUM,
		};

		enum EMobEnchants
		{
			MOB_ENCHANT_CURSE,
			MOB_ENCHANT_SLOW,
			MOB_ENCHANT_POISON,
			MOB_ENCHANT_STUN,
			MOB_ENCHANT_CRITICAL,
			MOB_ENCHANT_PENETRATE,
			MOB_ENCHANTS_MAX_NUM
		};
		enum EMobResists
		{
			MOB_RESIST_SWORD,
			MOB_RESIST_TWOHAND,
			MOB_RESIST_DAGGER,
			MOB_RESIST_BELL,
			MOB_RESIST_FAN,
			MOB_RESIST_BOW,
			MOB_RESIST_FIRE,
			MOB_RESIST_ELECT,
			MOB_RESIST_MAGIC,
			MOB_RESIST_WIND,
			MOB_RESIST_POISON,
#ifdef ENABLE_WOLFMAN_CHARACTER
			MOB_RESIST_CLAW,
#endif
			MOB_RESISTS_MAX_NUM
		};

		enum EMobMaxNum
		{
			MOB_ATTRIBUTE_MAX_NUM = 12,
			MOB_SKILL_MAX_NUM = 5,
		};

#ifdef ENABLE_ELEMENT_ADD
		enum EElements
		{
			ELEMENT_ELEC = 1,
			ELEMENT_FIRE,
			ELEMENT_ICE,
			ELEMENT_WIND,
			ELEMENT_EARTH,
			ELEMENT_DARK,
		};
#endif
		enum EMobAttFlags
		{
			ATT_ELEC,
			ATT_FIRE,
			ATT_ICE,
			ATT_WIND,
			ATT_EARTH,
			ATT_DARK,
			ATT_MAX_NUM
		};

		enum ERaceFlags
		{
			RACE_FLAG_ANIMAL = (1 << 0),
			RACE_FLAG_UNDEAD = (1 << 1),
			RACE_FLAG_DEVIL = (1 << 2),
			RACE_FLAG_HUMAN = (1 << 3),
			RACE_FLAG_ORC = (1 << 4),
			RACE_FLAG_MILGYO = (1 << 5),
			RACE_FLAG_INSECT = (1 << 6),
			RACE_FLAG_FIRE = (1 << 7),
			RACE_FLAG_ICE = (1 << 8),
			RACE_FLAG_DESERT = (1 << 9),
			RACE_FLAG_TREE = (1 << 10),
			RACE_FLAG_ATT_ELEC = (1 << 11),
			RACE_FLAG_ATT_FIRE = (1 << 12),
			RACE_FLAG_ATT_ICE = (1 << 13),
			RACE_FLAG_ATT_WIND = (1 << 14),
			RACE_FLAG_ATT_EARTH = (1 << 15),
			RACE_FLAG_ATT_DARK = (1 << 16),
			RACE_FLAG_DECO = (1 << 17),
			RACE_FLAG_HIDE = (1 << 18),
			RACE_FLAG_ATT_CZ = (1 << 19),
		};
		enum EMobAIFlags
		{
			AIFLAG_AGGRESSIVE		= (1 <<  0),
			AIFLAG_NOMOVE			= (1 <<  1),
			AIFLAG_COWARD			= (1 <<  2),
			AIFLAG_NOATTACKSHINSU	= (1 <<  3),
			AIFLAG_NOATTACKJINNO	= (1 <<  4),
			AIFLAG_NOATTACKCHUNJO	= (1 <<  5),
			AIFLAG_ATTACKMOB		= (1 <<  6),
			AIFLAG_BERSERK			= (1 <<  7),
			AIFLAG_STONESKIN		= (1 <<  8),
			AIFLAG_GODSPEED			= (1 <<  9),
			AIFLAG_DEATHBLOW		= (1 << 10),
			AIFLAG_REVIVE			= (1 << 11),
			AIFLAG_HEALER			= (1 << 12),
			AIFLAG_COUNT			= (1 << 13),
			AIFLAG_NORECOVERY		= (1 << 14),
			AIFLAG_REFLECT			= (1 << 15),
			AIFLAG_FALL				= (1 << 16),
			AIFLAG_VIT				= (1 << 17),
			AIFLAG_RATTSPEED		= (1 << 18),
			AIFLAG_RCASTSPEED		= (1 << 19),
			AIFLAG_RHP_REGEN		= (1 << 20),
			AIFLAG_TIMEVIT			= (1 << 21),
			AIFLAG_11				= (1 << 22),
		};

#pragma pack(push)
#pragma pack(1)
		typedef struct SMobSkillLevel
		{
			DWORD       dwVnum;
			BYTE        bLevel;
		} TMobSkillLevel;

		typedef struct SMobTable
		{
			DWORD       dwVnum;
			char        szName[CHARACTER_NAME_MAX_LEN + 1];
			char        szLocaleName[CHARACTER_NAME_MAX_LEN + 1];

			BYTE        bType;                  // Monster, NPC
			BYTE        bRank;                  // PAWN, KNIGHT, KING
			BYTE        bBattleType;            // MELEE, etc..
			BYTE        bLevel;                 // Level
			BYTE        bSize;

			DWORD       dwGoldMin;
			DWORD       dwGoldMax;
			DWORD       dwExp;
			DWORD       dwMaxHP;
			BYTE        bRegenCycle;
			BYTE        bRegenPercent;
			WORD        wDef;

			DWORD       dwAIFlag;
			DWORD       dwRaceFlag;
			DWORD       dwImmuneFlag;

			BYTE        bStr, bDex, bCon, bInt;
			DWORD       dwDamageRange[2];

			short       sAttackSpeed;
			short       sMovingSpeed;
			BYTE        bAggresiveHPPct;
			WORD        wAggressiveSight;
			WORD        wAttackRange;

			char        cEnchants[MOB_ENCHANTS_MAX_NUM];
			char        cResists[MOB_RESISTS_MAX_NUM];

			DWORD       dwResurrectionVnum;
			DWORD       dwDropItemVnum;

			BYTE        bMountCapacity;
			BYTE        bOnClickType;

			BYTE        bEmpire;
			char        szFolder[64 + 1];
			float       fDamMultiply;
			DWORD       dwSummonVnum;
			DWORD       dwDrainSP;
			DWORD		dwMonsterColor;
		    DWORD       dwPolymorphItemVnum;

			TMobSkillLevel	Skills[MOB_SKILL_MAX_NUM];

		    BYTE		bBerserkPoint;
			BYTE		bStoneSkinPoint;
			BYTE		bGodSpeedPoint;
			BYTE		bDeathBlowPoint;
			BYTE		bRevivePoint;
		} TMobTable;
#pragma pack(pop)

		typedef std::list<TMobTable *> TMobTableList;
		typedef std::map<DWORD, TMobTable *> TNonPlayerDataMap;

	public:
		CPythonNonPlayer(void);
		virtual ~CPythonNonPlayer(void);

		void Clear();
		void Destroy();

		bool				LoadNonPlayerData(const char * c_szFileName);

		const TMobTable *	GetTable(DWORD dwVnum);
		bool				GetName(DWORD dwVnum, const char ** c_pszName);
		bool				GetInstanceType(DWORD dwVnum, BYTE* pbType);
		BYTE				GetEventType(DWORD dwVnum);
		BYTE				GetEventTypeByVID(DWORD dwVID);
		DWORD				GetMonsterColor(DWORD dwVnum);
		const char*			GetMonsterName(DWORD dwVnum);

		// Function for outer
		void				GetMatchableMobList(int iLevel, int iInterval, TMobTableList * pMobTableList);

	protected:
		TNonPlayerDataMap	m_NonPlayerDataMap;

#ifdef ENABLE_SEND_TARGET_INFO
	public:
		DWORD				GetMonsterMaxHP(DWORD dwVnum);
		DWORD				GetMonsterRaceFlag(DWORD dwVnum);
		DWORD				GetMonsterLevel(DWORD dwVnum);
		DWORD				GetMonsterDamage1(DWORD dwVnum);
		DWORD				GetMonsterDamage2(DWORD dwVnum);
		DWORD				GetMonsterExp(DWORD dwVnum);
		float				GetMonsterDamageMultiply(DWORD dwVnum);
		DWORD				GetMonsterST(DWORD dwVnum);
		DWORD				GetMonsterDX(DWORD dwVnum);
		bool				IsMonsterStone(DWORD dwVnum);
#endif

#ifdef ENABLE_ELEMENT_ADD
	public:
		BYTE				GetElement(int iVid);
#endif

#ifdef WJ_SHOW_MOB_INFO
	public:
		DWORD				GetAIFlag(DWORD dwVnum) { return GetTable(dwVnum) ? GetTable(dwVnum)->dwAIFlag : 0; }
#endif
};

