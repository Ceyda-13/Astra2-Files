#pragma once

namespace mining
{
	LPEVENT CreateMiningEvent(LPCHARACTER ch, LPCHARACTER load, int32_t count);
	uint32_t GetRawOreFromLoad(uint32_t dwLoadVnum);
	bool OreRefine(LPCHARACTER ch, LPCHARACTER npc, LPITEM item, uint64_t cost, int32_t pct, LPITEM metinstone_item);
	int32_t GetFractionCount();

	int32_t RealRefinePick(LPCHARACTER ch, LPITEM item);
	void CHEAT_MAX_PICK(LPCHARACTER ch, LPITEM item);

	bool IsVeinOfOre (uint32_t vnum);
}

