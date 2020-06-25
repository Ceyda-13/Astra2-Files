#pragma once

#include "char.h"
#include "item.h"

class CBeltInventoryHelper
{
public:
	typedef uint8_t	TGradeUnit;

	static TGradeUnit GetBeltGradeByRefineLevel(uint32_t level)
	{
		static TGradeUnit beltGradeByLevelTable[] =
		{
			0,
			1,
			1,
			2,
			2,
			3,
			4,
			5,
			6,
			7,
		};

		if (level >= _countof(beltGradeByLevelTable))
		{
			sys_err("CBeltInventoryHelper::GetBeltGradeByRefineLevel - Overflow level (%u)", level);
			return 0;
		}

		return beltGradeByLevelTable[level];
	}

	static const TGradeUnit* GetAvailableRuleTableByGrade()
	{
		static TGradeUnit availableRuleByGrade[BELT_INVENTORY_SLOT_COUNT] = {
			1, 2, 4, 6,
			3, 3, 4, 6,
			5, 5, 5, 6,
			7, 7, 7, 7
		};

		return availableRuleByGrade;
	}

	static bool IsAvailableCell(uint16_t cell, int32_t beltGrade)
	{
		const TGradeUnit* ruleTable = GetAvailableRuleTableByGrade();

		return ruleTable[cell] <= beltGrade;
	}

	static bool IsExistItemInBeltInventory(LPCHARACTER pc)
	{
		for (uint16_t i = BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END; ++i)
		{
			LPITEM beltInventoryItem = pc->GetInventoryItem(i);

			if (nullptr != beltInventoryItem)
				return true;
		}

		return false;
	}

	static bool CanMoveIntoBeltInventory(LPITEM item)
	{
		bool canMove = false;

		if (item->GetType() == ITEM_USE)
		{
			switch (item->GetSubType())
			{
			case USE_POTION:
			case USE_POTION_NODELAY:
			case USE_ABILITY_UP:
				canMove = true;
				break;
			}
		}

		return canMove;
	}

};

