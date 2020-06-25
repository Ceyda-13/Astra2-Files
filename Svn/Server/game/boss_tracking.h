#pragma once
#include <unordered_map>

#include "packet.h"
enum EBossTracking
{
	BOSS_TRACKING_MAX_NUM = 6,
};

class CBossTracking : public singleton<CBossTracking>
{
	public:
		// typedef struct BossTracking
		// {
			// int32_t BTKey;
			// int32_t dead_time;
			// int32_t regen_time;
			// uint8_t channel;
			// uint32_t mob_vnum;
			// uint32_t map_index;

			// BossTracking()
			// {
				// BTKey = 0;
				// dead_time = 0;
				// regen_time = 0;
				// channel = 0;
				// mob_vnum = 0;
				// map_index = 0;
			// }
		// } TBossTracking;
		
		// std::vector<TBossTracking> m_bossTrackingVector;
		std::unordered_map<int32_t, uint32_t>	dead_time;
		std::unordered_map<int32_t, uint32_t>	regen_time;
		std::unordered_map<int32_t, uint32_t>	channel;
		std::unordered_map<int32_t, uint32_t>	mob_vnum;
		std::unordered_map<int32_t, uint32_t>	map_index;
		
		CBossTracking();
		~CBossTracking();

		int32_t GetDeadTime(uint8_t channel, uint32_t dwMobVnum);
		int32_t GetRegenTime(uint8_t channel, uint32_t dwMobVnum);
		int32_t GetMapIndex(uint8_t channel, uint32_t dwMobVnum);
		bool IsBossTrackingSystem();
		void SetDeadTime(uint8_t xchannel, uint32_t dwMobVnum, uint32_t dwTime, uint32_t xmap_index);
		void SetRegenTime(uint8_t xchannel, uint32_t dwMobVnum, uint32_t dwTime, uint32_t xmap_index);
		void SendP2PPacket(uint32_t dwMobVnum, uint32_t channel);
		bool IsTrackingMob(uint32_t dwMobVnum);
		void SendClientPacket(LPCHARACTER pkChr, uint8_t channel, uint32_t dwMobVnum);
};

