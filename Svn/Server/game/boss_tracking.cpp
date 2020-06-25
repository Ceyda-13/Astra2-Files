#include "stdafx.h"
#include "char.h"
#include "utils.h"
#include "log.h"
#include "db.h"
#include "locale_service.h"
#include <stdlib.h>
#include <sstream>
#include "config.h"
#include "desc.h"
#include "desc_manager.h"
#include "char_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "mob_manager.h"
#include "desc_client.h"
#include "group_text_parse_tree.h"
#include <cctype>
#include "p2p.h"
#include "entity.h"
#include "sectree_manager.h"
#include "regen.h"
#include "questmanager.h"
#include "boss_tracking.h"

CBossTracking::CBossTracking()
{
	dead_time.clear();
	regen_time.clear();
	channel.clear();
	mob_vnum.clear();
	map_index.clear();
	// m_bossTrackingVector.resize(6);
}

CBossTracking::~CBossTracking(){}

bool CBossTracking::IsBossTrackingSystem()
{
	// if (quest::CQuestManager::instance().GetEventFlag("enable_boss_tracking") == 1)
		// return false;

	return true;
}

int32_t CBossTracking::GetDeadTime(uint8_t channel, uint32_t dwMobVnum)
{
	if (IsBossTrackingSystem() == true)
	{
		if (channel < 1 || channel > 6 || dwMobVnum <= 0)
			return 0;

		int32_t BTKey = dwMobVnum+channel;
		auto it = dead_time.find(BTKey); // c++11
		if (dead_time.end() != it)
			return it->second;

		// TBossTracking & vec_tracking = m_bossTrackingVector[BTKey];
		// if (vec_tracking.BTKey == BTKey)
			// return vec_tracking.dead_time;
	}
	return 0;
}

int32_t CBossTracking::GetRegenTime(uint8_t channel, uint32_t dwMobVnum)
{
	if (IsBossTrackingSystem() == true)
	{
		if (channel < 1 || channel > 6 || dwMobVnum <= 0)
			return 0;

		int32_t BTKey = dwMobVnum+channel;
		auto it = regen_time.find(BTKey); // c++11
		if (regen_time.end() != it)
			return it->second;
		// TBossTracking & vec_tracking = m_bossTrackingVector[BTKey];
		// if (vec_tracking.BTKey == BTKey)
			// return vec_tracking.regen_time;
		// return 0;
	}
	return 0;
}

int32_t CBossTracking::GetMapIndex(uint8_t channel, uint32_t dwMobVnum)
{

	if (IsBossTrackingSystem() == true)
	{
		if (channel < 1 || channel > 6 || dwMobVnum <= 0)
			return 0;
	
		int32_t BTKey = dwMobVnum+channel;
		auto it = map_index.find(BTKey); // c++11
		if (map_index.end() != it)
			return it->second;
		
		// TBossTracking & vec_tracking = m_bossTrackingVector[BTKey];
		// if (vec_tracking.BTKey == BTKey)
			// return vec_tracking.map_index;
		// return 0;
	}
	return 0;
}

void CBossTracking::SetDeadTime(uint8_t xchannel, uint32_t dwMobVnum, uint32_t dwTime, uint32_t xmap_index)
{
	if (IsBossTrackingSystem() == false)
		return;
	
	if (CBossTracking::instance().IsTrackingMob(dwMobVnum) == false)
		return;
	
	if (xchannel < 1 || xchannel > 6 || dwMobVnum <=0 || dwTime <= 0 || xmap_index <=0)
		return;

	int32_t BTKey = dwMobVnum+xchannel;

	auto it = dead_time.find(BTKey); // c++11
	if (dead_time.end() == it)
		dead_time.insert(std::make_pair(BTKey, dwTime));
	else
		it->second = dwTime;
	
	auto it_c = channel.find(BTKey); // c++11
	if (channel.end() == it_c)
		channel.insert(std::make_pair(BTKey, xchannel));
	else
		it_c->second = xchannel;
	
	auto it_m = mob_vnum.find(BTKey); // c++11
	if (mob_vnum.end() == it_m)
		mob_vnum.insert(std::make_pair(BTKey, dwMobVnum));
	else
		it_m->second = dwMobVnum;
	
	auto it_map = map_index.find(BTKey); // c++11
	if (map_index.end() == it_map)
		map_index.insert(std::make_pair(BTKey, xmap_index));
	else
		it_map->second = xmap_index;
	
	// TBossTracking & vec_tracking = m_bossTrackingVector[BTKey];
	// vec_tracking.BTKey = BTKey;
	// vec_tracking.mob_vnum = dwMobVnum;
	// vec_tracking.dead_time = dwTime;
	// vec_tracking.map_index = map_index;
}

void CBossTracking::SetRegenTime(uint8_t xchannel, uint32_t dwMobVnum, uint32_t dwTime, uint32_t xmap_index)
{	
	if (IsBossTrackingSystem() == false)
		return;
	
	if (CBossTracking::instance().IsTrackingMob(dwMobVnum) == false)
		return;
	
	if (xchannel < 1 || xchannel > 6 || dwMobVnum <=0 || dwTime <= 0 || xmap_index <=0)
		return;
	
	int32_t BTKey = dwMobVnum+xchannel;

	auto it = regen_time.find(BTKey); // c++11
	if (regen_time.end() == it)
		regen_time.insert(std::make_pair(BTKey, dwTime));
	else
		it->second = dwTime;
	
	auto it_c = channel.find(BTKey); // c++11
	if (channel.end() == it_c)
		channel.insert(std::make_pair(BTKey, xchannel));
	else
		it_c->second = xchannel;
	
	auto it_m = mob_vnum.find(BTKey); // c++11
	if (mob_vnum.end() == it_m)
		mob_vnum.insert(std::make_pair(BTKey, dwMobVnum));
	else
		it_m->second = dwMobVnum;
	
	auto it_map = map_index.find(BTKey); // c++11
	if (map_index.end() == it_map)
		map_index.insert(std::make_pair(BTKey, xmap_index));
	else
		it_map->second = xmap_index;
	
	// TBossTracking & vec_tracking = m_bossTrackingVector[BTKey];

	// vec_tracking.BTKey = BTKey;
	// vec_tracking.mob_vnum = dwMobVnum;
	// vec_tracking.regen_time = dwTime;
	// vec_tracking.map_index = map_index;
}

bool CBossTracking::IsTrackingMob(uint32_t dwMobVnum)
{
	if (dwMobVnum <=0)
		return false;
	
	switch (dwMobVnum)
	{
		case 691:
		case 791:
		case 1304:
		case 2492:
		case 2494:
		case 2495:
		case 2091:
		case 3972:
		case 2191:
		case 2306:
		case 1901:
		case 3090:
		case 3091:
		case 3290:
		case 3291:
		case 3590:
		case 3591:
		case 3490:
		case 3491:
		case 3690:
		case 3691:
		case 3890:
		case 3891:
		case 3390:
		case 3391:
		case 3595:
		case 3596:
		case 3790:
		case 3791:
		case 3190:
		case 3191:
		case 6407:
		case 2291:
		case 1192:
		case 2206:
			return true;
	}
	return false;
}

void CBossTracking::SendP2PPacket(uint32_t dwMobVnum, uint32_t channel)
{
	if (IsBossTrackingSystem() == false)
		return;
	
	if (channel < 1 || channel > 6 || dwMobVnum <=0)
		return;
	
	if (CBossTracking::instance().IsTrackingMob(dwMobVnum) == false)
		return;

	TPacketGGBossTracking p;
	p.header = HEADER_GG_BOSS_TRACKING;
	p.dead_time = GetDeadTime(channel, dwMobVnum);
	p.regen_time = GetRegenTime(channel, dwMobVnum);
	p.map_index = GetMapIndex(channel, dwMobVnum);
	p.channel = channel;
	p.mob_vnum = dwMobVnum;
	P2P_MANAGER::instance().Send(&p, sizeof(TPacketGGBossTracking));
}

void CBossTracking::SendClientPacket(LPCHARACTER pkChr, uint8_t channel, uint32_t dwMobVnum)
{
	if (IsBossTrackingSystem() == false)
		return;

	if (nullptr == pkChr)
		return;

	if (!pkChr || !pkChr->GetDesc())
		return;

	if (CBossTracking::instance().IsTrackingMob(dwMobVnum) == false)
		return;
	
	if (channel < 1 || channel > 6 || dwMobVnum <=0)
		return;

	TPacketGCBossTracking p;
	p.header = HEADER_GC_BOSS_TRACKING;
	p.dead_time = GetDeadTime(channel, dwMobVnum);
	p.regen_time = GetRegenTime(channel, dwMobVnum);
	p.map_index = GetMapIndex(channel, dwMobVnum);
	p.channel = channel;
	p.mob_vnum = dwMobVnum;
	pkChr->GetDesc()->Packet(&p, sizeof(p));
}

