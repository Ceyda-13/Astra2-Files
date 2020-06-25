#pragma once

#include "../common/length.h"
#include "../common/item_length.h"
#include "../common/tables.h"
#include "char_manager.h"
#include "sectree_manager.h"

enum eConfig
{
	MAP_INDEX = 41,
	SUBMAP_INDEX = 41,
	MIN_LVL = 3,
	PARTECIPANTS_LIMIT = 16,
	LADDER_POINTS_COST = 600,
	LADDER_POINTS_RETURN = 300,
	COOLDOWN_DUNGEON = 10800,
	NPC_VNUM = 20419,
	GATE_VNUM = 20388,
	BOSS_VNUM = 6193,
	STATUE_VNUM = 6118,
	CHEST_VNUM = 20420,
	REWARD_ITEMCHEST_VNUM_1 = 50270,
	REWARD_ITEMCHEST_VNUM_2 = 50271,
	TIME_LIMIT_DUNGEON = 3600,
	SEAL_VNUM_KILL_STATUE = 30341,
	TIME_LIMIT_TO_KILL_STATUE = 15,
	TIME_RESPAWN_COMMON_STEP1 = 5,
	MOBCOUNT_RESPAWN_COMMON_STEP = 12,
	MOBVNUM_RESPAWN_COMMON_STEP1 = 6112,
	TIME_RESPAWN_COMMON_STEP2 = 30,
	MOBVNUM_RESPAWN_COMMON_STEP2 = 6113,
	MOBVNUM_RESPAWN_STONE_STEP2 = 20422,
	MOBCOUNT_RESPAWN_STONE_STEP2 = 4,
	TIME_RESPAWN_COMMON_STEP3 = 30,
	MOBVNUM_RESPAWN_COMMON_STEP3 = 6115,
	MOBVNUM_RESPAWN_BOSS_STEP3 = 6116,
	MOBVNUM_RESPAWN_SUBBOSS_STEP3 = 6117,
	MOBCOUNT_RESPAWN_BOSS_STEP3 = 4,
};

extern int32_t stoneSpawnPos[MOBCOUNT_RESPAWN_STONE_STEP2][2];
extern int32_t monsterSpawnPos[MOBCOUNT_RESPAWN_COMMON_STEP][2];

class CMgrMap
{
	public:
		CMgrMap(int32_t lMapIndex, uint32_t dwID);
		~CMgrMap();
		void	Destroy();
		int32_t	GetMapIndex() const	{return map_index;}
		uint32_t	GetID() const	{return m_dwID;}
		uint8_t	GetDungeonStep() const	{return dungeon_step;}
		void	SetDungeonStep(uint8_t bStep);
		void	StartDungeonStep(uint8_t bStep);
		uint32_t	GetDungeonTimeStart() const	{return time_start;}
		void	SetDungeonTimeStart(uint32_t dwTime)	{time_start = dwTime;}
		uint32_t	GetLastStoneKilledTime() const	{return last_stoneKilled;}
		void	SetLastStoneKilledTime(uint32_t dwTime)	{last_stoneKilled = dwTime;}
		uint32_t	GetKillCountStones() const	{return kill_stonesCount;}
		void	SetKillCountStones(uint32_t dwCount)	{kill_stonesCount = dwCount;}
		uint32_t	GetKillCountBosses() const	{return kill_bossesCount;}
		void	SetKillCountBosses(uint32_t dwCount)	{kill_bossesCount = dwCount;}
		uint32_t	GetRewardTime() const	{return reward;}
		void	SetRewardTime(uint32_t dwTime)	{reward = dwTime;}
		uint32_t	GetPartecipantsCount() const;
		void	Partecipant(bool bInsert, uint32_t dwPlayerID);
		bool	IsPartecipant(uint32_t dwPlayerID);
		LPCHARACTER	Spawn(uint32_t dwVnum, int32_t iX, int32_t iY, int32_t iDir = 0, bool bSpawnMotion = false);
		void	Start();
		void	StartDungeon(LPCHARACTER pkChar);
		void	EndDungeon(bool bSuccess, bool bGiveBack);
		void	EndDungeonWarp();
		bool	Damage(LPCHARACTER pkStatue);
		void	OnKill(uint32_t dwVnum);
		void	OnKillStatue(LPITEM pkItem, LPCHARACTER pkChar, LPCHARACTER pkStatue);
		void	DungeonResult();
		bool	CheckRewarder(uint32_t dwPlayerID);
		void	GiveReward(LPCHARACTER pkChar, uint8_t bReward);
		LPSECTREE_MAP &	GetMapSectree()	{return pkSectreeMap;}
		LPCHARACTER &	GetBossChar()	{return pkBoss;}
		LPCHARACTER &	GetStatue1Char()	{return pkStatue1;}
		LPCHARACTER &	GetStatue2Char()	{return pkStatue2;}
		LPCHARACTER &	GetStatue3Char()	{return pkStatue3;}
		LPCHARACTER &	GetStatue4Char()	{return pkStatue4;}
	
	private:
		int32_t	map_index;
		uint32_t	m_dwID, time_start, last_stoneKilled, kill_stonesCount, kill_bossesCount, reward;
		uint8_t	dungeon_step;
		std::vector<uint32_t>	v_Partecipants, v_Already, v_Rewards;
		LPSECTREE_MAP	pkSectreeMap;
		LPCHARACTER	pkMainNPC, pkGate, pkBoss, pkStatue1, pkStatue2, pkStatue3, pkStatue4;
	
	protected:
		LPEVENT	e_pEndEvent, e_pWarpEvent, e_SpawnEvent, e_SEffectEvent, e_DestroyStatues;
};

class CMgr : public singleton<CMgr>
{
	public:
		void	Initialize();
		void	Destroy();
		bool	Enter(LPCHARACTER pkChar);
		void	Leave(LPCHARACTER pkChar, bool bSendOut);
		void	LeaveRequest(LPCHARACTER pkChar);
		bool	IsMeleyMap(int32_t lMapIndex);
		void	Check(LPCHARACTER pkChar);
		void	WarpOut(LPCHARACTER pkChar);
		void	SetXYZ(int32_t lX, int32_t lY, int32_t lZ)	{lMapCenterPos.x = lX, lMapCenterPos.y = lY, lMapCenterPos.z = lZ;}
		const PIXEL_POSITION &	GetXYZ() const	{return lMapCenterPos;}
		void	SetSubXYZ(int32_t lX, int32_t lY, int32_t lZ)	{lSubMapPos.x = lX, lSubMapPos.y = lY, lSubMapPos.z = lZ;}
		const PIXEL_POSITION &	GetSubXYZ() const	{return lSubMapPos;}
		void	Start(LPCHARACTER pkChar);
		bool	Damage(LPCHARACTER pkStatue, LPCHARACTER pkChar);
		void	Remove(uint32_t dwID);
		void	OnKill(uint32_t dwVnum, LPCHARACTER pkKiller);
		void	OnKillStatue(LPITEM pkItem, LPCHARACTER pkChar, LPCHARACTER pkStatue);
		void	OnKillCommon(LPCHARACTER pkMonster, LPCHARACTER pkChar);
		bool	CanGetReward(LPCHARACTER pkChar);
		void	Reward(LPCHARACTER pkChar, uint8_t bReward);
		void	OpenRanking(LPCHARACTER pkChar);
		void	DungeonSet(LPCHARACTER pkChar);
	
	private:
		PIXEL_POSITION	lMapCenterPos, lSubMapPos;
		std::unordered_map<uint32_t, CMgrMap*> m_mapMeleys;
		uint32_t dwUniqueID;
};

