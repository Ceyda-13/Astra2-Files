#include "stdafx.h"

#include <stack>

#include "utils.h"
#include "config.h"
#include "char.h"
#include "char_manager.h"
#include "item_manager.h"
#include "desc.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "packet.h"
#include "affect.h"
#include "skill.h"
#include "start_position.h"
#include "mob_manager.h"
#include "db.h"
#include "log.h"
#include "vector.h"
#include "buffer_manager.h"
#include "questmanager.h"
#include "fishing.h"
#include "party.h"
#include "dungeon.h"
#include "refine.h"
#include "unique_item.h"
#include "war_map.h"
#include "xmas_event.h"
#include "marriage.h"
#include "monarch.h"
#include "polymorph.h"
#include "blend_item.h"
#include "castle.h"
#include "BattleArena.h"
#include "arena.h"
#include "dev_log.h"
#include "pcbang.h"
#include "threeway_war.h"

#include "safebox.h"
#include "shop.h"
#include "pvp.h" // @correction230

#include "../common/VnumHelper.h"
#include "DragonSoul.h"
#include "buff_on_attributes.h"
#include "belt_inventory_helper.h"

#ifdef __AUCTION__
#include "auction_manager.h"
#endif

#ifdef __COSTUME_PET__
#include "PetSystem.h"
#endif

#ifdef __MOUNT__
#include "mountsystem.h"
#endif

#ifdef __GUILD_DRAGONLAIR__
#include "meleylair.h"
#endif

#ifdef __GROWTH_PET__
#include "new_petsystem.h"
#endif

#ifdef __SWITCHBOT__
#include "new_switchbot.h"
#endif

const int32_t ITEM_BROKEN_METIN_VNUM = 28960;

const uint32_t CHARACTER::msc_dwDefaultChangeItemAttrCycle = 10;
const char CHARACTER::msc_szLastChangeItemAttrFlag[] = "Item.LastChangeItemAttr";
const char CHARACTER::msc_szChangeItemAttrCycleFlag[] = "change_itemattr_cycle";
const uint8_t g_aBuffOnAttrPoints[] = { POINT_ENERGY, POINT_COSTUME_ATTR_BONUS };

struct FFindStone
{
	std::map<uint32_t, LPCHARACTER> m_mapStone;

	void operator()(LPENTITY pEnt)
	{
		if (pEnt->IsType(ENTITY_CHARACTER) == true)
		{
			LPCHARACTER pChar = (LPCHARACTER)pEnt;

			if (pChar->IsStone() == true)
			{
				m_mapStone[(uint32_t)pChar->GetVID()] = pChar;
			}
		}
	}
};


static bool IS_SUMMON_ITEM(int32_t vnum)
{
	switch (vnum)
	{
		case 22000:
		case 22010:
		case 22011:
		case 22020:
		case ITEM_MARRIAGE_RING:
			return true;
	}

	return false;
}

static bool IS_MONKEY_DUNGEON(int32_t map_index)
{
	switch (map_index)
	{
		case 5:
		case 25:
		case 45:
		case 108:
		case 109:
			return true;
	}

	return false;
}

bool IS_SUMMONABLE_ZONE(int32_t map_index)
{
	if (IS_MONKEY_DUNGEON(map_index))
		return false;
	if (IS_CASTLE_MAP(map_index))
		return false;

	switch (map_index)
	{
		case 66 :
		case 71 :
		case 72 :
		case 73 :
		case 193 :
#if 0
		case 184 :
		case 185 :
		case 186 :
		case 187 :
		case 188 :
		case 189 :
#endif
		case 216 :
		case 217 :
		case 208 :

		case 28 :
			return false;
	}

	if (CBattleArena::IsBattleArenaMap(map_index)) return false;

	if (map_index > 10000) return false;

	return true;
}

bool IS_BOTARYABLE_ZONE(int32_t nMapIndex)
{
	// @correction005
// @correction211 BEGIN
	if (!g_bEnableBootaryCheck)
		return true;
// @correction211 END

	switch (nMapIndex)
	{
		case 1 :
		case 3 :
		case 21 :
		case 23 :
		case 41 :
		case 43 :
			return true;
	}

	return false;
}

static bool FN_check_item_socket(LPITEM item)
{
	for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		if (item->GetSocket(i) != item->GetProto()->alSockets[i])
			return false;
	}

	return true;
}

static void FN_copy_item_socket(LPITEM dest, LPITEM src)
{
	for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		dest->SetSocket(i, src->GetSocket(i));
	}
}
static bool FN_check_item_sex(LPCHARACTER ch, LPITEM item)
{
	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_MALE))
	{
		if (SEX_MALE==GET_SEX(ch))
			return false;
	}
	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_FEMALE))
	{
		if (SEX_FEMALE==GET_SEX(ch))
			return false;
	}

	return true;
}


bool CHARACTER::CanHandleItem(bool bSkipCheckRefine, bool bSkipObserver)
{
	if (!bSkipObserver)
		if (m_bIsObserver)
			return false;

	if (GetMyShop())
		return false;

	if (!bSkipCheckRefine)
		if (m_bUnderRefine)
			return false;

	if (IsCubeOpen() || nullptr != DragonSoul_RefineWindow_GetOpener())
		return false;

	if (IsWarping())
		return false;

#ifdef __ACCE_SYSTEM__ // @correction191
	if ((m_bAcceCombination) || (m_bAcceAbsorption))
		return false;
#endif

#ifdef __CHANGE_LOOK__ // @correction192
	if (m_bChangeLook)
		return false;
#endif

	return true;
}

LPITEM CHARACTER::GetInventoryItem(uint16_t wCell) const
{
	return GetItem(TItemPos(INVENTORY, wCell));
}
LPITEM CHARACTER::GetItem(TItemPos Cell) const
{
	if (!IsValidItemPosition(Cell))
		return nullptr;
	uint16_t wCell = Cell.cell;
	uint8_t window_type = Cell.window_type;
	switch (window_type)
	{
	case INVENTORY:
	case EQUIPMENT:
		if (wCell >= INVENTORY_AND_EQUIP_SLOT_MAX)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid item cell %d", wCell);
			return nullptr;
		}
		return m_pointsInstant.pItems[wCell];
	case DRAGON_SOUL_INVENTORY:
		if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid DS item cell %d", wCell);
			return nullptr;
		}
		return m_pointsInstant.pDSItems[wCell];

#ifdef __SWITCHBOT__
	case SWITCHBOT:
		if (wCell >= SWITCHBOT_SLOT_COUNT)
		{
			sys_err("CHARACTER::GetInventoryItem: invalid switchbot item cell %d", wCell);
			return nullptr;
		}
		return m_pointsInstant.pSwitchbotItems[wCell];
#endif

	default:
		return nullptr;
	}
	return nullptr;
}

void CHARACTER::SetItem(TItemPos Cell, LPITEM pItem, bool isHighLight)
{
	uint16_t wCell = Cell.cell;
	uint8_t window_type = Cell.window_type;
	if ((uint32_t)((CItem*)pItem) == 0xff || (uint32_t)((CItem*)pItem) == 0xffffffff)
	{
		sys_err("!!! FATAL ERROR !!! item == 0xff (char: %s cell: %u)", GetName(), wCell);
		core_dump();
		return;
	}

	if (pItem && pItem->GetOwner()
#ifdef __OFFLINE_PRIVATE_SHOP__
&& !pItem->GetOfflinePrivateShopVid()
#endif
)
	{
		assert(!"GetOwner exist");
		return;
	}
	switch(window_type)
	{
	case INVENTORY:
	case EQUIPMENT:
		{
			if (wCell >= INVENTORY_AND_EQUIP_SLOT_MAX)
			{
				sys_err("CHARACTER::SetItem: invalid item cell %d", wCell);
				return;
			}

			LPITEM pOld = m_pointsInstant.pItems[wCell];

			if (pOld)
			{
				if (wCell < INVENTORY_MAX_NUM)
				{
					for (int32_t i = 0; i < pOld->GetSize(); ++i)
					{
						int32_t p = wCell + (i * INVENTORY_PAGE_COLUMN);

						if (p >= INVENTORY_MAX_NUM)
							continue;

						if (m_pointsInstant.pItems[p] && m_pointsInstant.pItems[p] != pOld)
							continue;

						m_pointsInstant.wItemGrid[p] = 0;
					}
				}
				else
					m_pointsInstant.wItemGrid[wCell] = 0;
			}

			if (pItem)
			{
				if (wCell < INVENTORY_MAX_NUM)
				{
					for (int32_t i = 0; i < pItem->GetSize(); ++i)
					{
						int32_t p = wCell + (i * INVENTORY_PAGE_COLUMN);

						if (p >= INVENTORY_MAX_NUM)
							continue;

						m_pointsInstant.wItemGrid[p] = wCell + 1;
					}
				}
				else
					m_pointsInstant.wItemGrid[wCell] = wCell + 1;
			}

			m_pointsInstant.pItems[wCell] = pItem;
		}
		break;
	case DRAGON_SOUL_INVENTORY:
		{
			LPITEM pOld = m_pointsInstant.pDSItems[wCell];

			if (pOld)
			{
				if (wCell < DRAGON_SOUL_INVENTORY_MAX_NUM)
				{
					for (int32_t i = 0; i < pOld->GetSize(); ++i)
					{
						int32_t p = wCell + (i * DRAGON_SOUL_BOX_COLUMN_NUM);

						if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							continue;

						if (m_pointsInstant.pDSItems[p] && m_pointsInstant.pDSItems[p] != pOld)
							continue;

						m_pointsInstant.wDSItemGrid[p] = 0;
					}
				}
				else
					m_pointsInstant.wDSItemGrid[wCell] = 0;
			}

			if (pItem)
			{
				if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
				{
					sys_err("CHARACTER::SetItem: invalid DS item cell %d", wCell);
					return;
				}

				if (wCell < DRAGON_SOUL_INVENTORY_MAX_NUM)
				{
					for (int32_t i = 0; i < pItem->GetSize(); ++i)
					{
						int32_t p = wCell + (i * DRAGON_SOUL_BOX_COLUMN_NUM);

						if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							continue;

						m_pointsInstant.wDSItemGrid[p] = wCell + 1;
					}
				}
				else
					m_pointsInstant.wDSItemGrid[wCell] = wCell + 1;
			}

			m_pointsInstant.pDSItems[wCell] = pItem;
		}
		break;
#ifdef __SWITCHBOT__
	case SWITCHBOT:
	{
		LPITEM pOld = m_pointsInstant.pSwitchbotItems[wCell];
		if (pItem && pOld)
		{
			return;
		}

		if (wCell >= SWITCHBOT_SLOT_COUNT)
		{
			sys_err("CHARACTER::SetItem: invalid switchbot item cell %d", wCell);
			return;
		}

		if (pItem)
		{
			CSwitchbotManager::Instance().RegisterItem(GetPlayerID(), pItem->GetID(), wCell);
		}
		else
		{
			CSwitchbotManager::Instance().UnregisterItem(GetPlayerID(), wCell);
		}

		m_pointsInstant.pSwitchbotItems[wCell] = pItem;
	}
	break;
#endif
	default:
		sys_err ("Invalid Inventory type %d", window_type);
		return;
	}

	if (GetDesc())
	{
		if (pItem)
		{
			TPacketGCItemSet pack;
			pack.header = HEADER_GC_ITEM_SET;
			pack.Cell = Cell;

			pack.count = pItem->GetCount();
			pack.vnum = pItem->GetVnum();
			pack.flags = pItem->GetFlag();
			pack.anti_flags	= pItem->GetAntiFlag();
			pack.highlight = isHighLight ? true : (Cell.window_type == DRAGON_SOUL_INVENTORY);

			memcpy(pack.alSockets, pItem->GetSockets(), sizeof(pack.alSockets));
			memcpy(pack.aAttr, pItem->GetAttributes(), sizeof(pack.aAttr));
#ifdef __SOULBIND__ // @correction202
			pack.sealbind = pItem->GetSealBindTime();
#endif
#ifdef __CHANGE_LOOK__ // @correction192
			pack.changelook = pItem->GetChangeLook();
#endif
			GetDesc()->Packet(&pack, sizeof(TPacketGCItemSet));
		}
		else
		{
			TPacketGCItemDelDeprecated pack;
			pack.header = HEADER_GC_ITEM_DEL;
			pack.Cell = Cell;
			pack.count = 0;
			pack.vnum = 0;
			memset(pack.alSockets, 0, sizeof(pack.alSockets));
			memset(pack.aAttr, 0, sizeof(pack.aAttr));
			GetDesc()->Packet(&pack, sizeof(TPacketGCItemDelDeprecated));
		}
	}

	if (pItem)
	{
		pItem->SetCell(this, wCell);
		switch (window_type)
		{
		case INVENTORY:
		case EQUIPMENT:
			if ((wCell < INVENTORY_MAX_NUM) || (BELT_INVENTORY_SLOT_START <= wCell && BELT_INVENTORY_SLOT_END > wCell))
				pItem->SetWindow(INVENTORY);
			else
				pItem->SetWindow(EQUIPMENT);
			break;
		case DRAGON_SOUL_INVENTORY:
			pItem->SetWindow(DRAGON_SOUL_INVENTORY);
			break;
#ifdef __SWITCHBOT__
		case SWITCHBOT:
			pItem->SetWindow(SWITCHBOT);
			break;
#endif
		}
	}
}

LPITEM CHARACTER::GetWear(uint16_t wCell) const
{
	if (wCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
	{
		sys_err("CHARACTER::GetWear: invalid wear cell %d", wCell);
		return nullptr;
	}

	return m_pointsInstant.pItems[INVENTORY_MAX_NUM + wCell];
}

void CHARACTER::SetWear(uint16_t wCell, LPITEM item)
{
	if (wCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
	{
		sys_err("CHARACTER::SetItem: invalid item cell %d", wCell);
		return;
	}

	SetItem(TItemPos (INVENTORY, INVENTORY_MAX_NUM + wCell), item);

	// if (!item && wCell == WEAR_WEAPON)
	// {
		// if (IsAffectFlag(AFF_GWIGUM))
			// RemoveAffect(SKILL_GWIGEOM);

		// if (IsAffectFlag(AFF_GEOMGYEONG))
			// RemoveAffect(SKILL_GEOMKYUNG);
	// }
}

void CHARACTER::ClearItem()
{
	int32_t		i;
	LPITEM	item;

	for (i = 0; i < INVENTORY_AND_EQUIP_SLOT_MAX; ++i)
	{
		if ((item = GetInventoryItem(i)))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);

			SyncQuickslot(QUICKSLOT_TYPE_ITEM, i, 255);
		}
	}
	for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetItem(TItemPos(DRAGON_SOUL_INVENTORY, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}

#ifdef __SWITCHBOT__
	for (i = 0; i < SWITCHBOT_SLOT_COUNT; ++i)
	{
		if ((item = GetItem(TItemPos(SWITCHBOT, i))))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
#endif
}

bool CHARACTER::IsEmptyItemGrid(TItemPos Cell, LPITEM item, int32_t iExceptionCell) const
{
	if (!item)
		return false;

	uint8_t bSize = item->GetSize();
	switch (Cell.window_type)
	{
		case INVENTORY:
		{
			uint16_t wCell = Cell.cell; // @correction129

			++iExceptionCell;

			if (Cell.IsBeltInventoryPosition())
			{
				LPITEM beltItem = GetWear(WEAR_BELT);

				if (nullptr == beltItem)
					return false;

				if (false == CBeltInventoryHelper::IsAvailableCell(wCell - BELT_INVENTORY_SLOT_START, beltItem->GetValue(0)))
					return false;

				if (m_pointsInstant.wItemGrid[wCell])
				{
					if (m_pointsInstant.wItemGrid[wCell] == iExceptionCell)
						return true;

					return false;
				}

				if (bSize == 1)
					return true;

			}
			else if (wCell >= GetInventoryMax(item->GetInventoryType()))
				return false;

			if (m_pointsInstant.wItemGrid[wCell])
			{
				if (m_pointsInstant.wItemGrid[wCell] == iExceptionCell)
				{
					if (bSize == 1)
						return true;

					int32_t j = 1;
					uint8_t bPage = wCell / INVENTORY_PAGE_SIZE; // @correction130

					do
					{
						uint8_t p = wCell + (INVENTORY_PAGE_COLUMN * j);

						if (p >= GetInventoryMax(item->GetInventoryType()))
							return false;

						if (p / INVENTORY_PAGE_SIZE != bPage) // @correction130
							return false;

						if (m_pointsInstant.wItemGrid[p])
							if (m_pointsInstant.wItemGrid[p] != iExceptionCell)
								return false;
					}
					while (++j < bSize);

					return true;
				}
				else
					return false;
			}

			if (1 == bSize)
				return true;
			else
			{
				int32_t j = 1;
				uint8_t bPage = wCell / INVENTORY_PAGE_SIZE; // @correction130

				do
				{
					uint8_t p = wCell + (INVENTORY_PAGE_COLUMN * j);

					if (p >= GetInventoryMax(item->GetInventoryType()))
						return false;

					if (p / INVENTORY_PAGE_SIZE != bPage) // @correction130
						return false;

					if (m_pointsInstant.wItemGrid[p])
						if (m_pointsInstant.wItemGrid[p] != iExceptionCell)
							return false;
				}
				while (++j < bSize);

				return true;
			}
		}
		break;
		case DRAGON_SOUL_INVENTORY:
		{
			uint16_t wCell = Cell.cell;
			if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
				return false;

			iExceptionCell++;

			if (m_pointsInstant.wDSItemGrid[wCell])
			{
				if (m_pointsInstant.wDSItemGrid[wCell] == iExceptionCell)
				{
					if (bSize == 1)
						return true;

					int32_t j = 1;

					do
					{
						uint16_t p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);  // @correction129

						if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							return false;

						if (m_pointsInstant.wDSItemGrid[p])
							if (m_pointsInstant.wDSItemGrid[p] != iExceptionCell)
								return false;
					}
					while (++j < bSize);

					return true;
				}
				else
					return false;
			}

			if (1 == bSize)
				return true;
			else
			{
				int32_t j = 1;

				do
				{
					uint16_t p = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * j);  // @correction129

					if (p >= DRAGON_SOUL_INVENTORY_MAX_NUM)
						return false;

					if (m_pointsInstant.wItemGrid[p])
						if (m_pointsInstant.wDSItemGrid[p] != iExceptionCell)
							return false;
				}
				while (++j < bSize);

				return true;
			}
		}
		break;

#ifdef __SWITCHBOT__
		case SWITCHBOT:
		{
			uint16_t wCell = Cell.cell;
			if (wCell >= SWITCHBOT_SLOT_COUNT)
			{
				return false;
			}

			if (m_pointsInstant.pSwitchbotItems[wCell])
			{
				return false;
			}

			return true;
		}
#endif
	}
	return false;
}

int32_t CHARACTER::GetEmptyInventory(LPITEM item, int32_t iExceptionCell) const
{
	if (!item)
		return -1;

	uint8_t bType = item->GetInventoryType();
	int32_t i = INVENTORY_ONE_MAX_NUM * bType; 
	if (iExceptionCell)
		i = iExceptionCell;

	for (; i < GetInventoryMax(bType); ++i)
		if (IsEmptyItemGrid(TItemPos(INVENTORY, i), item))
			return i;

	return -1;
}

int32_t CHARACTER::GetEmptyDragonSoulInventory(LPITEM pItem) const
{
	if (nullptr == pItem || !pItem->IsDragonSoul())
		return -1;
	if (!DragonSoul_IsQualified())
	{
		return -1;
	}
	uint16_t wBaseCell = DSManager::instance().GetBasePosition(pItem);

	if (WORD_MAX == wBaseCell)
		return -1;

	for (int32_t i = 0; i < DRAGON_SOUL_BOX_SIZE; ++i)
		if (IsEmptyItemGrid(TItemPos(DRAGON_SOUL_INVENTORY, i + wBaseCell), pItem))
			return i + wBaseCell;

	return -1;
}

void CHARACTER::CopyDragonSoulItemGrid(std::vector<uint16_t>& vDragonSoulItemGrid) const
{
	vDragonSoulItemGrid.resize(DRAGON_SOUL_INVENTORY_MAX_NUM);

	std::copy(m_pointsInstant.wDSItemGrid, m_pointsInstant.wDSItemGrid + DRAGON_SOUL_INVENTORY_MAX_NUM, vDragonSoulItemGrid.begin());
}

int32_t CHARACTER::CountEmptyInventory(uint8_t bType) const
{
	int32_t	count = 0;
	for (int32_t i = INVENTORY_ONE_MAX_NUM * bType; i < GetInventoryMax(bType); ++i)
		if (GetInventoryItem(i))
			count += GetInventoryItem(i)->GetSize();

	return (GetInventoryMax(bType) - count);
}

void TransformRefineItem(LPITEM pkOldItem, LPITEM pkNewItem)
{
	if (pkOldItem->IsAccessoryForSocket())
	{
		for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			pkNewItem->SetSocket(i, pkOldItem->GetSocket(i));
		}
	}
	else
	{
		for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			if (!pkOldItem->GetSocket(i))
				break;
			else
				pkNewItem->SetSocket(i, 1);
		}

		int32_t slot = 0;

		for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		{
			int32_t socket = pkOldItem->GetSocket(i);

			if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
				pkNewItem->SetSocket(slot++, socket);
		}

	}

	pkOldItem->CopyAttributeTo(pkNewItem);
}

void NotifyRefineSuccess(LPCHARACTER ch, LPITEM item, const char* way)
{
	if (nullptr != ch && item != nullptr)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineSuceeded");

		LogManager::instance().RefineLog(ch->GetPlayerID(), item->GetName(), item->GetID(), item->GetRefineLevel(), 1, way);
	}
}

void NotifyRefineFail(LPCHARACTER ch, LPITEM item, const char* way, int32_t success = 0)
{
	if (nullptr != ch && nullptr != item)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineFailed");

		LogManager::instance().RefineLog(ch->GetPlayerID(), item->GetName(), item->GetID(), item->GetRefineLevel(), success, way);
	}
}

void CHARACTER::SetRefineNPC(LPCHARACTER ch)
{
	if ( ch != nullptr )
	{
		m_dwRefineNPCVID = ch->GetVID();
	}
	else
	{
		m_dwRefineNPCVID = 0;
	}
}

bool CHARACTER::DoRefine(LPITEM item, bool bMoneyOnly)
{
	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

	if (quest::CQuestManager::instance().GetEventFlag("update_refine_time") != 0)
	{
		if (get_global_time() < quest::CQuestManager::instance().GetEventFlag("update_refine_time") + (60 * 5))
		{
			sys_log(0, "can't refine %d %s", GetPlayerID(), GetName());
			return false;
		}
	}

	const TRefineTable * prt = CRefineManager::instance().GetRefineRecipe(item->GetRefineSet());

	if (!prt)
		return false;

	uint32_t result_vnum = item->GetRefinedVnum();

	uint64_t cost = ComputeRefineFee(prt->cost);

	int32_t RefineChance = GetQuestFlag("main_quest_lv7.refine_chance");

	if (RefineChance > 0)
	{
		if (!item->CheckItemUseLevel(20) || item->GetType() != ITEM_WEAPON)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹«·á °³·® ±âÈ¸´Â 20 ÀÌÇÏÀÇ ¹«±â¸¸ °¡´ÉÇÕ´Ï´Ù"));
			return false;
		}

		cost = 0;
		SetQuestFlag("main_quest_lv7.refine_chance", RefineChance - 1);
	}

	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TUNING)
		return false;

	TItemTable * pProto = ITEM_MANAGER::instance().GetTable(item->GetRefinedVnum());

	if (!pProto)
	{
		sys_err("DoRefine NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (GetGold() < cost)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù."));
		return false;
	}

	if (!bMoneyOnly && !RefineChance)
	{
		for (int32_t i = 0; i < prt->material_count; ++i)
		{
#ifdef __PENDANT__
			if (CountSpecifyItemTheOutItem(prt->materials[i].vnum, item) < prt->materials[i].count)
#else
			if (CountSpecifyItem(prt->materials[i].vnum) < prt->materials[i].count)
#endif
			{
				if (test_server)
				{
					ChatPacket(CHAT_TYPE_INFO, "Find %d, count %d, require %d", prt->materials[i].vnum, CountSpecifyItem(prt->materials[i].vnum), prt->materials[i].count);
				}
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
				return false;
			}
#ifdef __PENDANT__
			LPITEM src = FindSpecifyItemTheOutItem(prt->materials[i].vnum, item);
			if (!src)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
				return false;
			}
#endif
		}

		for (int32_t i = 0; i < prt->material_count; ++i)
#ifdef __PENDANT__
			RemoveSpecifyItemTheOutItem(prt->materials[i].vnum, prt->materials[i].count, item);
#else
			RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count);
#endif
	}

	int32_t prob = number(1, 100);

	if (IsRefineThroughGuild() || bMoneyOnly)
		prob -= 10;


	if (prob <= prt->prob)
	{
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(result_vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
			LogManager::instance().ItemLog(this, pkNewItem, "REFINE SUCCESS", pkNewItem->GetName());

#ifdef __CHANGE_LOOK__ // @correction192
			pkNewItem->SetChangeLook(item->GetChangeLook());
#endif

			uint16_t wCell = item->GetCell();

			NotifyRefineSuccess(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER");
			DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -cost);
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");

			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell), true);
			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);

			sys_log(0, "Refine Success %llu", cost);
			pkNewItem->AttrLog();
			sys_log(0, "PayPee %llu", cost);
			PayRefineFee(cost);
			sys_log(0, "PayPee End %llu", cost);
		}
		else
		{
			sys_err("cannot create item %u", result_vnum);
			NotifyRefineFail(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER");
		}
	}
	else
	{
		DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -cost);
		NotifyRefineFail(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER");
		item->AttrLog();
		ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE FAIL)");

		PayRefineFee(cost);
	}

	return true;
}

enum enum_RefineScrolls
{
	CHUKBOK_SCROLL = 0, // Kutsama Kagidi
	HYUNIRON_CHN   = 1, // Büyülü Metal
	YONGSIN_SCROLL = 2, // Ejderha Parsomeni
	MUSIN_SCROLL   = 3, // Savas Kagidi
	YAGONG_SCROLL  = 4, // Demircinin El Kitabi
	MEMO_SCROLL	   = 5, // Demircinin Notu
	BDRAGON_SCROLL	= 6,
#ifdef __RITUEL_STONE__
	RITUALS_SCROLL = 7,
#endif
};

bool CHARACTER::DoRefineWithScroll(LPITEM item)
{
	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

	ClearRefineMode();

	if (quest::CQuestManager::instance().GetEventFlag("update_refine_time") != 0)
	{
		if (get_global_time() < quest::CQuestManager::instance().GetEventFlag("update_refine_time") + (60 * 5))
		{
			sys_log(0, "can't refine %d %s", GetPlayerID(), GetName());
			return false;
		}
	}

	const TRefineTable * prt = CRefineManager::instance().GetRefineRecipe(item->GetRefineSet());

	if (!prt)
		return false;

	LPITEM pkItemScroll;

	if (m_iRefineAdditionalCell < 0)
		return false;

	pkItemScroll = GetInventoryItem(m_iRefineAdditionalCell);

	if (!pkItemScroll)
		return false;

	if (!(pkItemScroll->GetType() == ITEM_USE && pkItemScroll->GetSubType() == USE_TUNING))
		return false;

	if (pkItemScroll->GetVnum() == item->GetVnum())
		return false;

	uint32_t result_vnum = item->GetRefinedVnum();
	uint32_t result_fail_vnum = item->GetRefineFromVnum();

	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (pkItemScroll->GetValue(0) == MUSIN_SCROLL)
	{
		if (item->GetRefineLevel() >= 4)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ °³·®¼­·Î ´õ ÀÌ»ó °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
	}

	else if (pkItemScroll->GetValue(0) == MEMO_SCROLL)
	{
		if (item->GetRefineLevel() != pkItemScroll->GetValue(1))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ °³·®¼­·Î °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
	}
	else if (pkItemScroll->GetValue(0) == BDRAGON_SCROLL)
	{
		if (item->GetType() != ITEM_METIN || item->GetRefineLevel() != 4)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀ¸·Î °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
	}
#ifdef __RITUEL_STONE__
	else if (pkItemScroll->GetValue(0) == RITUALS_SCROLL)
	{
		if (item->GetLevelLimit() < pkItemScroll->GetValue(1))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("you can only upgrade items above level %d"), pkItemScroll->GetValue(1));
			return false;
		}
	}
#endif

	TItemTable * pProto = ITEM_MANAGER::instance().GetTable(item->GetRefinedVnum());

	if (!pProto)
	{
		sys_err("DoRefineWithScroll NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (GetGold() < prt->cost)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ µ·ÀÌ ºÎÁ·ÇÕ´Ï´Ù."));
		return false;
	}

	for (int32_t i = 0; i < prt->material_count; ++i)
	{
		if (CountSpecifyItem(prt->materials[i].vnum) < prt->materials[i].count)
		{
			if (test_server)
			{
				ChatPacket(CHAT_TYPE_INFO, "Find %d, count %d, require %d", prt->materials[i].vnum, CountSpecifyItem(prt->materials[i].vnum), prt->materials[i].count);
			}
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³·®À» ÇÏ±â À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
			return false;
		}
	}

	for (int32_t i = 0; i < prt->material_count; ++i)
		RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count);

	int32_t prob = number(1, 100);
	int32_t success_prob = prt->prob;
	bool bDestroyWhenFail = false;

	const char* szRefineType = "SCROLL";

	if (pkItemScroll->GetValue(0) == HYUNIRON_CHN ||
		pkItemScroll->GetValue(0) == YONGSIN_SCROLL ||
		pkItemScroll->GetValue(0) == YAGONG_SCROLL
#ifdef __RITUEL_STONE__
|| pkItemScroll->GetValue(0) == RITUALS_SCROLL
#endif
)
	{
		// const char hyuniron_prob[9] =	{ 100, 75, 65, 55, 45, 40, 35, 25, 20 };
		// const char yagong_prob[9] =		{ 100, 100, 90, 80, 70, 60, 50, 30, 20 };

		if (pkItemScroll->GetValue(0) == YONGSIN_SCROLL)
		{
			// @correction005
			// success_prob = hyuniron_prob[MINMAX(0, item->GetRefineLevel(), 8)];
		}
		else if (pkItemScroll->GetValue(0) == YAGONG_SCROLL)
		{
			// @correction005
			// success_prob = yagong_prob[MINMAX(0, item->GetRefineLevel(), 8)];
		}
		else if (pkItemScroll->GetValue(0) == HYUNIRON_CHN
#ifdef __RITUEL_STONE__
|| pkItemScroll->GetValue(0) == RITUALS_SCROLL
#endif
)
		{
			// @correction044
		}
		else
		{
			sys_err("REFINE : Unknown refine scroll item. Value0: %d", pkItemScroll->GetValue(0));
		}

		if (test_server)
		{
			ChatPacket(CHAT_TYPE_INFO, "[Only Test] Success_Prob %d, RefineLevel %d ", success_prob, item->GetRefineLevel());
		}
		if (pkItemScroll->GetValue(0) == HYUNIRON_CHN
#ifdef __RITUEL_STONE__
|| pkItemScroll->GetValue(0) == RITUALS_SCROLL
#endif
)
			bDestroyWhenFail = true;

		if (pkItemScroll->GetValue(0) == HYUNIRON_CHN)
		{
			szRefineType = "HYUNIRON";
		}
		else if (pkItemScroll->GetValue(0) == YONGSIN_SCROLL)
		{
			szRefineType = "GOD_SCROLL";
		}
		else if (pkItemScroll->GetValue(0) == YAGONG_SCROLL)
		{
			szRefineType = "YAGONG_SCROLL";
		}
#ifdef __RITUEL_STONE__
		else if (pkItemScroll->GetValue(0) == RITUALS_SCROLL)
		{
			success_prob += pkItemScroll->GetValue(2);
			szRefineType = "RITUALS_SCROLL";
		}
#endif
	}

	if (pkItemScroll->GetValue(0) == MUSIN_SCROLL)
	{
		success_prob = 100;

		szRefineType = "MUSIN_SCROLL";
	}
	else if (pkItemScroll->GetValue(0) == MEMO_SCROLL)
	{
		success_prob = 100;
		szRefineType = "MEMO_SCROLL";
	}
	else if (pkItemScroll->GetValue(0) == BDRAGON_SCROLL)
	{
		success_prob = 80;
		szRefineType = "BDRAGON_SCROLL";
	}

	pkItemScroll->SetCount(pkItemScroll->GetCount() - 1);

	if (prob <= success_prob)
	{
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(result_vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
			LogManager::instance().ItemLog(this, pkNewItem, "REFINE SUCCESS", pkNewItem->GetName());

#ifdef __CHANGE_LOOK__ // @correction192
			pkNewItem->SetChangeLook(item->GetChangeLook());
#endif

			uint16_t wCell = item->GetCell();

			NotifyRefineSuccess(this, item, szRefineType);
			DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -prt->cost);
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");

			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell), true);
			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);
			pkNewItem->AttrLog();
			PayRefineFee(prt->cost);
		}
		else
		{
			sys_err("cannot create item %u", result_vnum);
			NotifyRefineFail(this, item, szRefineType);
		}
	}
	else if (!bDestroyWhenFail && result_fail_vnum)
	{
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(result_fail_vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);
			LogManager::instance().ItemLog(this, pkNewItem, "REFINE FAIL", pkNewItem->GetName());

#ifdef __CHANGE_LOOK__ // @correction192
			pkNewItem->SetChangeLook(item->GetChangeLook());
#endif

			uint16_t wCell = item->GetCell();

			DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -prt->cost);
			NotifyRefineFail(this, item, szRefineType, -1);
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE FAIL)");

			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, wCell), true);
			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);

			pkNewItem->AttrLog();

			PayRefineFee(prt->cost);
		}
		else
		{
			sys_err("cannot create item %u", result_fail_vnum);
			NotifyRefineFail(this, item, szRefineType);
		}
	}
	else
	{
		NotifyRefineFail(this, item, szRefineType);
		PayRefineFee(prt->cost);
	}

	return true;
}

bool CHARACTER::RefineInformation(uint16_t wCell, uint8_t bType, int32_t iAdditionalCell)
{
	if (wCell > INVENTORY_MAX_NUM)
		return false;

	LPITEM item = GetInventoryItem(wCell);

	if (!item)
		return false;

	if (bType == REFINE_TYPE_MONEY_ONLY && !GetQuestFlag("deviltower_zone.can_refine"))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç±Í Å¸¿ö ¿Ï·á º¸»óÀº ÇÑ¹ø±îÁö »ç¿ë°¡´ÉÇÕ´Ï´Ù."));
		return false;
	}

	TPacketGCRefineInformation p;

	p.header = HEADER_GC_REFINE_INFORMATION;
	p.pos = wCell;
	p.src_vnum = item->GetVnum();
	p.result_vnum = item->GetRefinedVnum();
	p.type = bType;

	if (p.result_vnum == 0)
	{
		sys_err("RefineInformation p.result_vnum == 0");
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TUNING)
	{
		if (bType == 0)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº ÀÌ ¹æ½ÄÀ¸·Î´Â °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
		else
		{
			LPITEM itemScroll = GetInventoryItem(iAdditionalCell);
			if (!itemScroll || item->GetVnum() == itemScroll->GetVnum())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°°Àº °³·®¼­¸¦ ÇÕÄ¥ ¼ö´Â ¾ø½À´Ï´Ù."));
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ãàº¹ÀÇ ¼­¿Í ÇöÃ¶À» ÇÕÄ¥ ¼ö ÀÖ½À´Ï´Ù."));
				return false;
			}
		}
	}

	CRefineManager & rm = CRefineManager::instance();

	const TRefineTable* prt = rm.GetRefineRecipe(item->GetRefineSet());

	if (!prt)
	{
		sys_err("RefineInformation NOT GET REFINE SET %d", item->GetRefineSet());
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}


	if (GetQuestFlag("main_quest_lv7.refine_chance") > 0)
	{
		if (!item->CheckItemUseLevel(20) || item->GetType() != ITEM_WEAPON)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹«·á °³·® ±âÈ¸´Â 20 ÀÌÇÏÀÇ ¹«±â¸¸ °¡´ÉÇÕ´Ï´Ù"));
			return false;
		}
		p.cost = 0;
	}
	else
		p.cost = ComputeRefineFee(prt->cost);

	p.prob = prt->prob;
	if (bType == REFINE_TYPE_MONEY_ONLY)
	{
		p.material_count = 0;
		memset(p.materials, 0, sizeof(p.materials));
	}
	else
	{
		p.material_count = prt->material_count;
		memcpy(&p.materials, prt->materials, sizeof(prt->materials));
	}

	GetDesc()->Packet(&p, sizeof(TPacketGCRefineInformation));

	SetRefineMode(iAdditionalCell);
	return true;
}

bool CHARACTER::RefineItem(LPITEM pkItem, LPITEM pkTarget)
{
	if (!CanHandleItem())
		return false;

	if (pkItem->GetSubType() == USE_TUNING)
	{
		if (pkItem->GetValue(0) == MUSIN_SCROLL)
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_MUSIN, pkItem->GetCell());
		else if (pkItem->GetValue(0) == HYUNIRON_CHN)
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_HYUNIRON, pkItem->GetCell());
		else if (pkItem->GetValue(0) == BDRAGON_SCROLL)
		{
			if (pkTarget->GetRefineSet() != 702) return false;
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_BDRAGON, pkItem->GetCell());
		}
#ifdef __RITUEL_STONE__
		else if (pkItem->GetValue(0) == RITUALS_SCROLL)
		{
			if (pkTarget->GetLevelLimit() <= pkItem->GetValue(1))
				return false;

			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_RITUALS_SCROLL, pkItem->GetCell());
		}
#endif
		else
		{
			if (pkTarget->GetRefineSet() == 501) return false;
			RefineInformation(pkTarget->GetCell(), REFINE_TYPE_SCROLL, pkItem->GetCell());
		}
	}
	else if (pkItem->GetSubType() == USE_DETACHMENT && IS_SET(pkTarget->GetFlag(), ITEM_FLAG_REFINEABLE))
	{
		LogManager::instance().ItemLog(this, pkTarget, "USE_DETACHMENT", pkTarget->GetName());

		bool bHasMetinStone = false;

		for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
		{
			int32_t socket = pkTarget->GetSocket(i);
			if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
			{
				bHasMetinStone = true;
				break;
			}
		}

		if (bHasMetinStone)
		{
			for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
			{
				int32_t socket = pkTarget->GetSocket(i);
				if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
				{
					AutoGiveItem(socket);
					pkTarget->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
				}
			}
			pkItem->SetCount(pkItem->GetCount() - 1);
			return true;
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»©³¾ ¼ö ÀÖ´Â ¸ÞÆ¾¼®ÀÌ ¾ø½À´Ï´Ù."));
			return false;
		}
	}

	return false;
}

EVENTFUNC(kill_campfire_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "kill_campfire_event> <Factor> Null pointer" );
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

bool CHARACTER::GiveRecallItem(LPITEM item)
{
	int32_t idx = GetMapIndex();
	int32_t iEmpireByMapIndex = -1;

	if (idx < 20)
		iEmpireByMapIndex = 1;
	else if (idx < 40)
		iEmpireByMapIndex = 2;
	else if (idx < 60)
		iEmpireByMapIndex = 3;
	else if (idx < 10000)
		iEmpireByMapIndex = 0;

	switch (idx)
	{
		case 66:
		case 216:
			iEmpireByMapIndex = -1;
			break;
	}

	if (iEmpireByMapIndex && GetEmpire() != iEmpireByMapIndex)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±â¾ïÇØ µÑ ¼ö ¾ø´Â À§Ä¡ ÀÔ´Ï´Ù."));
		return false;
	}

	int32_t pos;

	if (item->GetCount() == 1)
	{
		item->SetSocket(0, GetX());
		item->SetSocket(1, GetY());
	}
	else if ((pos = GetEmptyInventory(item)) != -1)
	{
		LPITEM item2 = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), 1);

		if (nullptr != item2)
		{
			item2->SetSocket(0, GetX());
			item2->SetSocket(1, GetY());
			item2->AddToCharacter(this, TItemPos(INVENTORY, pos), true);

			item->SetCount(item->GetCount() - 1);
		}
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇ°¿¡ ºó °ø°£ÀÌ ¾ø½À´Ï´Ù."));
		return false;
	}

	return true;
}

void CHARACTER::ProcessRecallItem(LPITEM item)
{
	int32_t idx;

	if ((idx = SECTREE_MANAGER::instance().GetMapIndex(item->GetSocket(0), item->GetSocket(1))) == 0)
		return;

	int32_t iEmpireByMapIndex = -1;

	if (idx < 20)
		iEmpireByMapIndex = 1;
	else if (idx < 40)
		iEmpireByMapIndex = 2;
	else if (idx < 60)
		iEmpireByMapIndex = 3;
	else if (idx < 10000)
		iEmpireByMapIndex = 0;

	switch (idx)
	{
		case 66:
		case 216:
			iEmpireByMapIndex = -1;
			break;
		case 301:
		case 302:
		case 303:
		case 304:
			if( GetLevel() < 90 )
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛÀÇ ·¹º§ Á¦ÇÑº¸´Ù ·¹º§ÀÌ ³·½À´Ï´Ù."));
				return;
			}
			else
				break;
	}

	if (iEmpireByMapIndex && GetEmpire() != iEmpireByMapIndex)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±â¾ïµÈ À§Ä¡°¡ Å¸Á¦±¹¿¡ ¼ÓÇØ ÀÖ¾î¼­ ±ÍÈ¯ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		item->SetSocket(0, 0);
		item->SetSocket(1, 0);
	}
	else
	{
		sys_log(1, "Recall: %s %d %d -> %d %d", GetName(), GetX(), GetY(), item->GetSocket(0), item->GetSocket(1));
		WarpSet(item->GetSocket(0), item->GetSocket(1));
		item->SetCount(item->GetCount() - 1);
	}
}

void CHARACTER::__OpenPrivateShop()
{
#ifdef ENABLE_OPEN_SHOP_WITH_ARMOR // @correction171
	ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
#else
	uint32_t bodyPart = GetPart(PART_MAIN);
	switch (bodyPart)
	{
		case 0:
		case 1:
		case 2:
			ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
			break;
		default:
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°©¿ÊÀ» ¹þ¾î¾ß °³ÀÎ »óÁ¡À» ¿­ ¼ö ÀÖ½À´Ï´Ù."));
			break;
	}
#endif
}

void CHARACTER::SendMyShopPriceListCmd(uint32_t dwItemVnum, uint64_t dwItemPrice)
{
	char szLine[256];
	snprintf(szLine, sizeof(szLine), "MyShopPriceList %u %llu", dwItemVnum, dwItemPrice);
	ChatPacket(CHAT_TYPE_COMMAND, szLine);
	sys_log(0, szLine);
}

void CHARACTER::UseSilkBotaryReal(const TPacketMyshopPricelistHeader* p)
{
	const TItemPriceInfo* pInfo = (const TItemPriceInfo*)(p + 1);

	if (!p->byCount)
		SendMyShopPriceListCmd(1, 0);
	else
	{
		for (int32_t idx = 0; idx < p->byCount; idx++)
			SendMyShopPriceListCmd(pInfo[ idx ].dwVnum, pInfo[ idx ].dwPrice);
	}

	__OpenPrivateShop();
}

void CHARACTER::UseSilkBotary(void)
{
	if (m_bNoOpenedShop) {
		uint32_t dwPlayerID = GetPlayerID();
		db_clientdesc->DBPacket(HEADER_GD_MYSHOP_PRICELIST_REQ, GetDesc()->GetHandle(), &dwPlayerID, sizeof(uint32_t));
		m_bNoOpenedShop = false;
	} else {
		__OpenPrivateShop();
	}
}


int32_t CalculateConsume(LPCHARACTER ch)
{
	static const int32_t WARP_NEED_LIFE_PERCENT	= 30;
	static const int32_t WARP_MIN_LIFE_PERCENT	= 10;
	int32_t consumeLife = 0;
	{
		const int32_t curLife		= ch->GetHP();
		const int32_t needPercent	= WARP_NEED_LIFE_PERCENT;
		const int32_t needLife = ch->GetMaxHP() * needPercent / 100;
		if (curLife < needLife)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³²Àº »ý¸í·Â ¾çÀÌ ¸ðÀÚ¶ó »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return -1;
		}

		consumeLife = needLife;


		const int32_t minPercent	= WARP_MIN_LIFE_PERCENT;
		const int32_t minLife	= ch->GetMaxHP() * minPercent / 100;
		if (curLife - needLife < minLife)
			consumeLife = curLife - minLife;

		if (consumeLife < 0)
			consumeLife = 0;
	}
	return consumeLife;
}

int32_t CalculateConsumeSP(LPCHARACTER lpChar)
{
	static const int32_t NEED_WARP_SP_PERCENT = 30;

	const int32_t curSP = lpChar->GetSP();
	const int32_t needSP = lpChar->GetMaxSP() * NEED_WARP_SP_PERCENT / 100;

	if (curSP < needSP)
	{
		lpChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³²Àº Á¤½Å·Â ¾çÀÌ ¸ðÀÚ¶ó »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return -1;
	}

	return needSP;
}

bool CHARACTER::UseItemEx(LPITEM item, TItemPos DestCell
#ifdef __FAST_USE_GIFTBOX__
, uint16_t wCount
#endif
)
{
	int32_t iLimitRealtimeStartFirstUseFlagIndex = -1;

	uint16_t wDestCell = DestCell.cell;
	uint8_t bDestInven = DestCell.window_type;
	for (int32_t i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		int32_t limitValue = item->GetProto()->aLimits[i].lValue;

		switch (item->GetProto()->aLimits[i].bType)
		{
			case LIMIT_LEVEL:
				if (GetLevel() < limitValue)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛÀÇ ·¹º§ Á¦ÇÑº¸´Ù ·¹º§ÀÌ ³·½À´Ï´Ù."));
					return false;
				}
				break;

			case LIMIT_REAL_TIME_START_FIRST_USE:
				iLimitRealtimeStartFirstUseFlagIndex = i;
				break;

			case LIMIT_TIMER_BASED_ON_WEAR:
				break;
		}
	}

	if (test_server)
	{
		sys_log(0, "USE_ITEM %s, Inven %d, Cell %d, ItemType %d, SubType %d", item->GetName(), bDestInven, wDestCell, item->GetType(), item->GetSubType());
	}

	if ( CArenaManager::instance().IsLimitedItem( GetMapIndex(), item->GetVnum() ) == true )
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
		return false;
	}

// @correction230 BEGIN
	else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && IsLimitedPotionOnPVP(item->GetVnum()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
		return false;
	}
// @correction230 END

// @correction076 BEGIN (IsLoadedAffect to block affect hacking)
	if (!IsLoadedAffect())
	{
		ChatPacket(CHAT_TYPE_INFO, "Affects are not loaded yet!");
		return false;
	}
// @correction076 END

// @correction061 BEGIN
	if (TItemPos(item->GetWindow(), item->GetCell()).IsBeltInventoryPosition())
	{
		LPITEM beltItem = GetWear(WEAR_BELT);

		if (nullptr == beltItem)
		{
			ChatPacket(CHAT_TYPE_INFO, "<Belt> You can't use this item if you have no equipped belt.");
			return false;
		}

		if (false == CBeltInventoryHelper::IsAvailableCell(item->GetCell() - BELT_INVENTORY_SLOT_START, beltItem->GetValue(0)))
		{
			ChatPacket(CHAT_TYPE_INFO, "<Belt> You can't use this item if you don't upgrade your belt.");
			return false;
		}
	}
// @correction061 END

	if (-1 != iLimitRealtimeStartFirstUseFlagIndex)
	{
		if (0 == item->GetSocket(1))
		{
			int32_t duration = (0 != item->GetSocket(0)) ? item->GetSocket(0) : item->GetProto()->aLimits[iLimitRealtimeStartFirstUseFlagIndex].lValue;

			if (0 == duration)
				duration = 60 * 60 * 24 * 7;

			item->SetSocket(0, time(0) + duration);
			item->StartRealTimeExpireEvent();
		}

		if (false == item->IsEquipped())
			item->SetSocket(1, item->GetSocket(1) + 1);
	}

#ifdef __GROWTH_PET__
	if (item->GetVnum() == 55009 || item->GetVnum() == 76044)
	{
		int32_t n = number(55010, 55027);
		AutoGiveItem(n);
		item->SetCount(item->GetCount() - 1);
	}
#endif

	switch (item->GetType())
	{
#ifdef __GROWTH_PET__
	case ITEM_PET:
	{
		switch (item->GetSubType())
		{
			case PET_EGG:
			{
				if (quest::CQuestManager::instance().GetEventFlag("DISABLE_GROWTH") == 1)
				{
					ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
					return false;
				}

				SetEggVid(item->GetVnum());
				ChatPacket(CHAT_TYPE_COMMAND, "OpenPetIncubator %d %d", item->GetCell(), item->GetVnum() - 55401);
			}
			break;

			case PET_UPBRINGING:
			{
				if (quest::CQuestManager::instance().GetEventFlag("DISABLE_GROWTH") == 1)
				{
					ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
					return false;
				}

				{
					CNewPetSystem* petSystem = GetNewPetSystem();
					if (petSystem)
					{
#ifdef __CHANGE_LOOK__
						uint32_t dwVnum = item->GetChangeLook() ? item->GetChangeLook() : item->GetValue(0);
#else
						uint32_t dwVnum = item->GetValue(0);
#endif
						CNewPetActor* pet = petSystem->GetByVnum(dwVnum);
						if (pet && pet->IsSummoned())
						{
							petSystem->Unsummon(dwVnum);
							return true;
						}
						else
						{
							size_t count = petSystem->CountSummoned();
							if (count > 0)
							{
								ChatPacket(CHAT_TYPE_INFO, "Zaten Petin Var");
								return true;
							}
							else
							{
								const CMob* pMob = CMobManager::instance().Get(dwVnum);
								// std::string nm = GetName();
								std::string nm = "";
								nm += GetName();
								nm += "'s ";
								if (pMob)
									nm += pMob->m_table.szLocaleName;
								const char* mobname = nm.c_str();
								petSystem->Summon(dwVnum, item, mobname, false);
								return true;
							}
						}
					}
					else
						return false;
				}
			}
			break;

			case PET_FEEDSTUFF:
			{
				if (quest::CQuestManager::instance().GetEventFlag("DISABLE_GROWTH") == 1)
				{
					ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
					return false;
				}
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging())
					return false;

				if (item2->GetSubType() != PET_UPBRINGING)
					return false;

				uint32_t dwtduration = item2->GetLimitValue(0);
				uint32_t dwduration = item2->GetSocket(0) - time(nullptr);
				int32_t tmp_dur = dwtduration / 2;
				if (dwduration + tmp_dur > dwtduration)
					dwduration = dwtduration;
				else
					dwduration += tmp_dur;

				item2->SetSocket(0, time(nullptr) + dwduration);

				item->SetCount(item->GetCount() - 1);
				return true;
			}
			break;

			case PET_SKILL:
			{
				if (quest::CQuestManager::instance().GetEventFlag("DISABLE_GROWTH") == 1)
				{
					ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
					return false;
				}

				CNewPetSystem* petSystem = GetNewPetSystem();
				if (petSystem)
				{
					if (petSystem->GetLevel() >= 81 && petSystem->GetEvolution() >= 3)
					{
						if (petSystem->IncreasePetSkill(item->GetVnum() - 55009))
							item->SetCount(item->GetCount() - 1);
					}
				}
			}
			break;

			case PET_NAME_CHANGE:
			{

			}
			break;

			case PET_SKILL_ALL_DEL_BOOK:
			{
				if (quest::CQuestManager::instance().GetEventFlag("DISABLE_GROWTH") == 1)
				{
					ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
					return false;
				}

				CNewPetSystem* petSystem = GetNewPetSystem();
				if (!petSystem)
					return false;

				LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(petSystem->GetSummonItemVID());
				if (petSystem->Revert(-1, pSummonItem))
				{
					ChatPacket(CHAT_TYPE_INFO, "Skilller Basari ile temizlendi !");
					item->SetCount(item->GetCount() - 1);
					return true;
				}
			}
			break;

			case PET_SKILL_DEL_BOOK:
			{

			}
			break;

			case PET_EXPFOOD:
			{
				if (quest::CQuestManager::instance().GetEventFlag("DISABLE_GROWTH") == 1)
				{
					ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
					return false;
				}

				CNewPetSystem* petSystem = GetNewPetSystem();
				if (petSystem && petSystem->ItemFeed(item))
					item->SetCount(item->GetCount() - 1);
			}
			break;

			case PET_EXPFOOD_PER:
			{

			}
			break;

			case PET_ATTR_CHANGE:
				break;

			case PET_ATTR_CHANGE_ITEMVNUM:
				break;

			case PET_PAY:
			{
#ifdef __COSTUME_PET__
				if (!item->IsEquipped())
				{
					if (GetPetSystem() && GetPetSystem()->CountSummoned() > 0)
						return false;

					EquipItem(item);
				}
				else
					UnequipItem(item);
#endif
			}
			break;
		}
	}
	break;
#endif

		case ITEM_HAIR:
			return ItemProcess_Hair(item, wDestCell);

		case ITEM_POLYMORPH:
			return ItemProcess_Polymorph(item);

		case ITEM_QUEST:
			if (GetArena() != nullptr || IsObserverMode() == true)
			{
				if (item->GetVnum() == 50051 || item->GetVnum() == 50052 || item->GetVnum() == 50053)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
					return false;
				}
			}

			if (!IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE))
			{
				if (item->GetSIGVnum() == 0)
				{
					quest::CQuestManager::instance().UseItem(GetPlayerID(), item, false);
				}
				else
				{
					quest::CQuestManager::instance().SIGUse(GetPlayerID(), item->GetSIGVnum(), item, false);
				}
			}
			break;

		case ITEM_CAMPFIRE:
			{
				float fx, fy;
				GetDeltaByDegree(GetRotation(), 100.0f, &fx, &fy);

				LPSECTREE tree = SECTREE_MANAGER::instance().Get(GetMapIndex(), (int32_t)(GetX()+fx), (int32_t)(GetY()+fy));

				if (!tree)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ð´ÚºÒÀ» ÇÇ¿ï ¼ö ¾ø´Â ÁöÁ¡ÀÔ´Ï´Ù."));
					return false;
				}

				if (tree->IsAttr((int32_t)(GetX()+fx), (int32_t)(GetY()+fy), ATTR_WATER))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹° ¼Ó¿¡ ¸ð´ÚºÒÀ» ÇÇ¿ï ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}

				//@correction090 BEGIN
				auto it = m_ItemUseTimesMap.find(0);
				if (it != m_ItemUseTimesMap.end())
				{
					if (time(nullptr) < m_ItemUseTimesMap[0])
					{
						ChatPacket(CHAT_TYPE_INFO, "Bunu yapabilmek i?n %d saniye beklemelisin!", m_ItemUseTimesMap[0] - time(nullptr));
						return false;
					}
					else
						it->second = time(nullptr) + 30;
				}
				else
					m_ItemUseTimesMap.emplace(0, time(nullptr) + 30);
				//@correction090 END

				LPCHARACTER campfire = CHARACTER_MANAGER::instance().SpawnMob(fishing::CAMPFIRE_MOB, GetMapIndex(), (int32_t)(GetX()+fx), (int32_t)(GetY()+fy), 0, false, number(0, 359));

				char_event_info* info = AllocEventInfo<char_event_info>();

				info->ch = campfire;

				campfire->m_pkMiningEvent = event_create(kill_campfire_event, info, PASSES_PER_SEC(40));

				item->SetCount(item->GetCount() - 1);
			}
			break;

		case ITEM_UNIQUE:
			{
				switch (item->GetSubType())
				{
					case USE_ABILITY_UP:
						{
							switch (item->GetValue(0))
							{
								case APPLY_MOV_SPEED:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_MOV_SPEED, item->GetValue(2), AFF_MOV_SPEED_POTION, item->GetValue(1), 0, true, true);
									break;

								case APPLY_ATT_SPEED:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ATT_SPEED, item->GetValue(2), AFF_ATT_SPEED_POTION, item->GetValue(1), 0, true, true);
									break;

								case APPLY_STR:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ST, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_DEX:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_DX, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_CON:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_HT, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_INT:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_IQ, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_CAST_SPEED:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_CASTING_SPEED, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_RESIST_MAGIC:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_RESIST_MAGIC, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_ATT_GRADE_BONUS:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ATT_GRADE_BONUS,
											item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;

								case APPLY_DEF_GRADE_BONUS:
									AddAffect(AFFECT_UNIQUE_ABILITY, POINT_DEF_GRADE_BONUS,
											item->GetValue(2), 0, item->GetValue(1), 0, true, true);
									break;
							}
						}

						if (GetDungeon())
							GetDungeon()->UsePotion(this);

						if (GetWarMap())
							GetWarMap()->UsePotion(this, item);

						item->SetCount(item->GetCount() - 1);
						break;

#ifdef __OFFLINE_PRIVATE_SHOP__
					case USE_SPECIAL:
						switch (item->GetVnum())
						{
							case 71221:
							case 71260:
								ChatPacket(CHAT_TYPE_COMMAND, "OpenMyShopDecoWnd");
								break;
							default:
								break;
						}
						break;
#endif

					default:
						{
							if (item->GetSubType() == USE_SPECIAL)
							{
								sys_log(0, "ITEM_UNIQUE: USE_SPECIAL %u", item->GetVnum());

								switch (item->GetVnum())
								{
									case 71049:
										// @correction005
										if (g_bEnableBootaryCheck) // @correction211
										{
											if (IS_BOTARYABLE_ZONE(GetMapIndex()) == true)
											{
												UseSilkBotary();
											}
											else
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³ÀÎ »óÁ¡À» ¿­ ¼ö ¾ø´Â Áö¿ªÀÔ´Ï´Ù"));
											}
										}
										else
										{
											UseSilkBotary();
										}
										break;
								}
							}
							else
							{
								if (!item->IsEquipped())
									EquipItem(item);
								else
									UnequipItem(item);
							}
						}
						break;
				}
			}
			break;

		case ITEM_COSTUME:
		case ITEM_WEAPON:
		case ITEM_ARMOR:
		case ITEM_ROD:
		case ITEM_RING:
		case ITEM_BELT:
		case ITEM_PICK:
			if (!item->IsEquipped())
				EquipItem(item);
			else
				UnequipItem(item);
			break;
		case ITEM_DS:
			{
				if (!item->IsEquipped())
					return false;
				return DSManager::instance().PullOut(this, NPOS, item);
			break;
			}
		case ITEM_SPECIAL_DS:
			if (!item->IsEquipped())
				EquipItem(item);
			else
				UnequipItem(item);
			break;

		case ITEM_FISH:
			{
				if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
					return false;
				}
// @correction230 BEGIN
				else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
					return false;
				}
// @correction230 END

				if (item->GetSubType() == FISH_ALIVE)
					fishing::UseFish(this, item);
			}
			break;

		case ITEM_TREASURE_BOX:
			{
				return false;
			}
			break;

		case ITEM_TREASURE_KEY:
			{
				LPITEM item2;

				if (!GetItem(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() || item2->IsEquipped()) // @correction038
					return false;

				if (item2->GetType() != ITEM_TREASURE_BOX)
				{
					ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("¿­¼è·Î ¿©´Â ¹°°ÇÀÌ ¾Æ´Ñ°Í °°´Ù."));
					return false;
				}

				if (item->GetValue(0) == item2->GetValue(0))
				{
					uint32_t dwBoxVnum = item2->GetVnum();
					std::vector <uint32_t> dwVnums;
					std::vector <uint32_t> dwCounts;
					std::vector <LPITEM> item_gets(0);
					int32_t count = 0;

					if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count, wCount))
					{
						ITEM_MANAGER::instance().RemoveItem(item);
						ITEM_MANAGER::instance().RemoveItem(item2);

						for (int32_t i = 0; i < count; i++){
							switch (dwVnums[i])
							{
								case CSpecialItemGroup::GOLD:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µ· %d ³ÉÀ» È¹µæÇß½À´Ï´Ù."), dwCounts[i]);
									break;
								case CSpecialItemGroup::EXP:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ºÎÅÍ ½ÅºñÇÑ ºûÀÌ ³ª¿É´Ï´Ù."));
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÀÇ °æÇèÄ¡¸¦ È¹µæÇß½À´Ï´Ù."), dwCounts[i]);
									break;
								case CSpecialItemGroup::MOB:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ¸ó½ºÅÍ°¡ ³ªÅ¸³µ½À´Ï´Ù!"));
									break;
								case CSpecialItemGroup::SLOW:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ³ª¿Â »¡°£ ¿¬±â¸¦ µéÀÌ¸¶½ÃÀÚ ¿òÁ÷ÀÌ´Â ¼Óµµ°¡ ´À·ÁÁ³½À´Ï´Ù!"));
									break;
								case CSpecialItemGroup::DRAIN_HP:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ°¡ °©ÀÚ±â Æø¹ßÇÏ¿´½À´Ï´Ù! »ý¸í·ÂÀÌ °¨¼ÒÇß½À´Ï´Ù."));
									break;
								case CSpecialItemGroup::POISON:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ³ª¿Â ³ì»ö ¿¬±â¸¦ µéÀÌ¸¶½ÃÀÚ µ¶ÀÌ ¿Â¸öÀ¸·Î ÆÛÁý´Ï´Ù!"));
									break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
								case CSpecialItemGroup::BLEEDING:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ³ª¿Â ³ì»ö ¿¬±â¸¦ µéÀÌ¸¶½ÃÀÚ µ¶ÀÌ ¿Â¸öÀ¸·Î ÆÛÁý´Ï´Ù!"));
									break;
#endif
								case CSpecialItemGroup::MOB_GROUP:
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ¸ó½ºÅÍ°¡ ³ªÅ¸³µ½À´Ï´Ù!"));
									break;
								default:
									if (item_gets[i])
									{
										if (dwCounts[i] > 1)
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ %s °¡ %d °³ ³ª¿Ô½À´Ï´Ù."), item_gets[i]->GetName(), dwCounts[i]);
										else
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ %s °¡ ³ª¿Ô½À´Ï´Ù."), item_gets[i]->GetName());

									}
							}
						}
					}
					else
					{
						ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("¿­¼è°¡ ¸ÂÁö ¾Ê´Â °Í °°´Ù."));
						return false;
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("¿­¼è°¡ ¸ÂÁö ¾Ê´Â °Í °°´Ù."));
					return false;
				}
			}
			break;

		case ITEM_GIFTBOX:
			{
// @correction226 BEGIN
				if (0 != g_BoxUseTimeLimitValue)
				{
					if (get_dword_time() < m_dwLastBoxUseTime + g_BoxUseTimeLimitValue)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ °ñµå¸¦ ¹ö¸± ¼ö ¾ø½À´Ï´Ù."));
						return false;
					}
				}

				m_dwLastBoxUseTime = get_dword_time();
// @correction226 END

				if (quest::CQuestManager::instance().GetEventFlag("DISABLE_GIFTBOX") == 1)
				{
					ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
					return false;
				}

				if ((GetMapIndex() >= 1 && GetMapIndex() <= 6))
				{
					ChatPacket(CHAT_TYPE_INFO, "Köylerde Sandik Açamazsin !");
					return false;
				}

				uint32_t dwBoxVnum = item->GetVnum();
				std::vector <uint32_t> dwVnums;
				std::vector <uint32_t> dwCounts;
				std::vector <LPITEM> item_gets(0);
				int32_t count = 0;

				// @correction005
				if( dwBoxVnum > 51500 && dwBoxVnum < 52000 )
				{
					if( !(this->DragonSoul_IsQualified()) )
					{
						ChatPacket(CHAT_TYPE_INFO,LC_TEXT("¸ÕÀú ¿ëÈ¥¼® Äù½ºÆ®¸¦ ¿Ï·áÇÏ¼Å¾ß ÇÕ´Ï´Ù."));
						return false;
					}
				}
#ifdef __FAST_USE_GIFTBOX__
				if (item)
					item->Lock(true);

				wCount = wCount > item->GetCount() ? item->GetCount() : wCount;
#endif
				if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count
#ifdef __FAST_USE_GIFTBOX__
, wCount
#endif
))
				{
					for (int32_t i = 0; i < count; i++)
					{
						switch (dwVnums[i])
						{
							case CSpecialItemGroup::GOLD:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µ· %d ³ÉÀ» È¹µæÇß½À´Ï´Ù."), dwCounts[i]);
								break;
							case CSpecialItemGroup::EXP:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ºÎÅÍ ½ÅºñÇÑ ºûÀÌ ³ª¿É´Ï´Ù."));
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÀÇ °æÇèÄ¡¸¦ È¹µæÇß½À´Ï´Ù."), dwCounts[i]);
								break;
							case CSpecialItemGroup::MOB:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ¸ó½ºÅÍ°¡ ³ªÅ¸³µ½À´Ï´Ù!"));
								break;
							case CSpecialItemGroup::SLOW:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ³ª¿Â »¡°£ ¿¬±â¸¦ µéÀÌ¸¶½ÃÀÚ ¿òÁ÷ÀÌ´Â ¼Óµµ°¡ ´À·ÁÁ³½À´Ï´Ù!"));
								break;
							case CSpecialItemGroup::DRAIN_HP:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ°¡ °©ÀÚ±â Æø¹ßÇÏ¿´½À´Ï´Ù! »ý¸í·ÂÀÌ °¨¼ÒÇß½À´Ï´Ù."));
								break;
							case CSpecialItemGroup::POISON:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ³ª¿Â ³ì»ö ¿¬±â¸¦ µéÀÌ¸¶½ÃÀÚ µ¶ÀÌ ¿Â¸öÀ¸·Î ÆÛÁý´Ï´Ù!"));
								break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
							case CSpecialItemGroup::BLEEDING:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ³ª¿Â ³ì»ö ¿¬±â¸¦ µéÀÌ¸¶½ÃÀÚ µ¶ÀÌ ¿Â¸öÀ¸·Î ÆÛÁý´Ï´Ù!"));
								break;
#endif
							case CSpecialItemGroup::MOB_GROUP:
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ¸ó½ºÅÍ°¡ ³ªÅ¸³µ½À´Ï´Ù!"));
								break;
							default:
								if (item_gets[i])
								{
									if (dwCounts[i] > 1)
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ %s °¡ %d °³ ³ª¿Ô½À´Ï´Ù."), item_gets[i]->GetName(), dwCounts[i]);
									else
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ %s °¡ ³ª¿Ô½À´Ï´Ù."), item_gets[i]->GetName());
								}
						}
					}
				}
#ifndef __FAST_USE_GIFTBOX__
				else
				{
					ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("¾Æ¹«°Íµµ ¾òÀ» ¼ö ¾ø¾ú½À´Ï´Ù."));
					return false;
				}
				item->SetCount(item->GetCount() - 1);
#else
				item->SetCount(item->GetCount() - wCount);
				if (item)
					item->Lock(false);
#endif
			}
			break;

		case ITEM_SKILLFORGET:
			{
				if (!item->GetSocket(0))
				{
					ITEM_MANAGER::instance().RemoveItem(item);
					return false;
				}

				uint32_t dwVnum = item->GetSocket(0);

				if (SkillLevelDown(dwVnum))
				{
					item->SetCount(item->GetCount() - 1); // @correction088
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("½ºÅ³ ·¹º§À» ³»¸®´Âµ¥ ¼º°øÇÏ¿´½À´Ï´Ù."));
				}
				else
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("½ºÅ³ ·¹º§À» ³»¸± ¼ö ¾ø½À´Ï´Ù."));
			}
			break;

		case ITEM_SKILLBOOK:
			{
#ifdef __FAST_RUHBK__
				if (item->GetVnum() == 50300)
				{
					ChatPacket(CHAT_TYPE_COMMAND, "fastbk");
				}
				else
#endif
				{
					if (IsPolymorphed())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯½ÅÁß¿¡´Â Ã¥À» ÀÐÀ»¼ö ¾ø½À´Ï´Ù."));
						return false;
					}

					uint32_t dwVnum = 0;

					if (item->GetVnum() == 50300)
					{
						dwVnum = item->GetSocket(0);
					}
					else
					{
						dwVnum = item->GetValue(0);
					}

					if (0 == dwVnum)
					{
						ITEM_MANAGER::instance().RemoveItem(item);

						return false;
					}

					if (true == LearnSkillByBook(dwVnum))
					{
						item->SetCount(item->GetCount() - 1); // @correction088

						int32_t iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

						if (distribution_test_server)
							iReadDelay /= 3;

						// @correction005
						SetSkillNextReadTime(dwVnum, get_global_time() + iReadDelay);
					}
				}
			}
			break;

		case ITEM_USE:
			{
				if (item->GetVnum() > 50800 && item->GetVnum() <= 50820)
				{
					if (test_server)
						sys_log (0, "ADD addtional effect : vnum(%d) subtype(%d)", item->GetOriginalVnum(), item->GetSubType());

					int32_t affect_type = AFFECT_EXP_BONUS_EURO_FREE;
					int32_t apply_type = aApplyInfo[item->GetValue(0)].bPointType;
					int32_t apply_value = item->GetValue(2);
					int32_t apply_duration = item->GetValue(1);

					switch (item->GetSubType())
					{
						case USE_ABILITY_UP:
							if (FindAffect(affect_type, apply_type))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
								return false;
							}

							{
								switch (item->GetValue(0))
								{
									case APPLY_MOV_SPEED:
										AddAffect(affect_type, apply_type, apply_value, AFF_MOV_SPEED_POTION, apply_duration, 0, true, true);
										break;

									case APPLY_ATT_SPEED:
										AddAffect(affect_type, apply_type, apply_value, AFF_ATT_SPEED_POTION, apply_duration, 0, true, true);
										break;

									case APPLY_STR:
									case APPLY_DEX:
									case APPLY_CON:
									case APPLY_INT:
									case APPLY_CAST_SPEED:
									case APPLY_RESIST_MAGIC:
									case APPLY_ATT_GRADE_BONUS:
									case APPLY_DEF_GRADE_BONUS:
										AddAffect(affect_type, apply_type, apply_value, 0, apply_duration, 0, true, true);
										break;
								}
							}

							if (GetDungeon())
								GetDungeon()->UsePotion(this);

							if (GetWarMap())
								GetWarMap()->UsePotion(this, item);

							item->SetCount(item->GetCount() - 1);
							break;

					case USE_AFFECT :
						{
							if (FindAffect(AFFECT_EXP_BONUS_EURO_FREE, aApplyInfo[item->GetValue(1)].bPointType))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
							}
							else
							{
								if (item->IsPCBangItem() == true)
								{
									if (CPCBangManager::instance().IsPCBangIP(GetDesc()->GetHostName()) == false)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº PC¹æ¿¡¼­¸¸ »ç¿ëÇÒ ¼ö ÀÖ½À´Ï´Ù."));
										return false;
									}
								}

								AddAffect(AFFECT_EXP_BONUS_EURO_FREE, aApplyInfo[item->GetValue(1)].bPointType, item->GetValue(2), 0, item->GetValue(3), 0, false, true);
								item->SetCount(item->GetCount() - 1);
							}
						}
						break;

					case USE_POTION_NODELAY:
						{
							if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
							{
								if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit") > 0)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
									return false;
								}

								switch (item->GetVnum())
								{
									case 70020 :
									case 71018 :
									case 71019 :
									case 71020 :
										if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count") < 10000)
										{
											if (m_nPotionLimit <= 0)
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ë Á¦ÇÑ·®À» ÃÊ°úÇÏ¿´½À´Ï´Ù."));
												return false;
											}
										}
										break;

									default :
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
										return false;
										break;
								}
							}
// @correction230 BEGIN
							else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
								return false;
							}
// @correction230 END

							bool used = false;

							if (item->GetValue(0) != 0)
							{
								if (GetHP() < GetMaxHP())
								{
									PointChange(POINT_HP, item->GetValue(0) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
									EffectPacket(SE_HPUP_RED);
									used = true;
								}
							}

							if (item->GetValue(1) != 0)
							{
								if (GetSP() < GetMaxSP())
								{
									PointChange(POINT_SP, item->GetValue(1) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
									EffectPacket(SE_SPUP_BLUE);
									used = true;
								}
							}

							if (item->GetValue(3) != 0)
							{
								if (GetHP() < GetMaxHP())
								{
									PointChange(POINT_HP, item->GetValue(3) * GetMaxHP() / 100);
									EffectPacket(SE_HPUP_RED);
									used = true;
								}
							}

							if (item->GetValue(4) != 0)
							{
								if (GetSP() < GetMaxSP())
								{
									PointChange(POINT_SP, item->GetValue(4) * GetMaxSP() / 100);
									EffectPacket(SE_SPUP_BLUE);
									used = true;
								}
							}

							if (used)
							{
								if (item->GetVnum() == 50085 || item->GetVnum() == 50086)
								{
									if (test_server)
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ùº´ ¶Ç´Â Á¾ÀÚ ¸¦ »ç¿ëÇÏ¿´½À´Ï´Ù"));
									SetUseSeedOrMoonBottleTime();
								}
								if (GetDungeon())
									GetDungeon()->UsePotion(this);

								if (GetWarMap())
									GetWarMap()->UsePotion(this, item);

								m_nPotionLimit--;

								item->SetCount(item->GetCount() - 1);
							}
						}
						break;
					}

					return true;
				}


				if (item->GetVnum() >= 27863 && item->GetVnum() <= 27883)
				{
					if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
						return false;
					}
// @correction230 BEGIN
					else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
						return false;
					}
// @correction230 END
				}

				if (test_server)
				{
					 sys_log (0, "USE_ITEM %s Type %d SubType %d vnum %d", item->GetName(), item->GetType(), item->GetSubType(), item->GetOriginalVnum());
				}

				switch (item->GetSubType())
				{
					case USE_TIME_CHARGE_PER:
						{
							LPITEM pDestItem = GetItem(DestCell);
							if (nullptr == pDestItem)
							{
								return false;
							}
							if (pDestItem->IsDragonSoul())
							{
								int32_t ret;
								char buf[128];
								if (item->GetVnum() == DRAGON_HEART_VNUM)
								{
									ret = pDestItem->GiveMoreTime_Per((float)item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
								}
								else
								{
									ret = pDestItem->GiveMoreTime_Per((float)item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
								}
								if (ret > 0)
								{
									if (item->GetVnum() == DRAGON_HEART_VNUM)
									{
										sprintf(buf, "Inc %ds by item{VN:%d SOC%d:%d}", ret, item->GetVnum(), ITEM_SOCKET_CHARGING_AMOUNT_IDX, item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
									}
									else
									{
										sprintf(buf, "Inc %ds by item{VN:%d VAL%d:%d}", ret, item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
									}

									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÃÊ ¸¸Å­ ÃæÀüµÇ¾ú½À´Ï´Ù."), ret);
									item->SetCount(item->GetCount() - 1);
									LogManager::instance().ItemLog(this, item, "DS_CHARGING_SUCCESS", buf);
									return true;
								}
								else
								{
									if (item->GetVnum() == DRAGON_HEART_VNUM)
									{
										sprintf(buf, "No change by item{VN:%d SOC%d:%d}", item->GetVnum(), ITEM_SOCKET_CHARGING_AMOUNT_IDX, item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
									}
									else
									{
										sprintf(buf, "No change by item{VN:%d VAL%d:%d}", item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
									}

									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÃæÀüÇÒ ¼ö ¾ø½À´Ï´Ù."));
									LogManager::instance().ItemLog(this, item, "DS_CHARGING_FAILED", buf);
									return false;
								}
							}
							else
								return false;
						}
						break;
					case USE_TIME_CHARGE_FIX:
						{
							LPITEM pDestItem = GetItem(DestCell);
							if (nullptr == pDestItem)
							{
								return false;
							}
							if (pDestItem->IsDragonSoul())
							{
								int32_t ret = pDestItem->GiveMoreTime_Fix(item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
								char buf[128];
								if (ret)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÃÊ ¸¸Å­ ÃæÀüµÇ¾ú½À´Ï´Ù."), ret);
									sprintf(buf, "Increase %ds by item{VN:%d VAL%d:%d}", ret, item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
									LogManager::instance().ItemLog(this, item, "DS_CHARGING_SUCCESS", buf);
									item->SetCount(item->GetCount() - 1);
									return true;
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÃæÀüÇÒ ¼ö ¾ø½À´Ï´Ù."));
									sprintf(buf, "No change by item{VN:%d VAL%d:%d}", item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
									LogManager::instance().ItemLog(this, item, "DS_CHARGING_FAILED", buf);
									return false;
								}
							}
							else
								return false;
						}
						break;
					case USE_SPECIAL:
#ifdef __UNLIMITED_POTIONS__
						if (item->GetValue(3) == 4248)
						{
							uint32_t dwType = item->GetValue(0);
							uint8_t bBonus = aApplyInfo[item->GetValue(1)].bPointType;

							if (AFFECT_NONE == dwType)
								break;

							CAffect * pAffect = FindAffect(dwType, bBonus);

							if (!pAffect)
							{
								uint32_t dwFlag = item->GetValue(4);
								AddAffect(dwType, bBonus, item->GetValue(2), dwFlag, INFINITE_AFFECT_DURATION, 0, true, true);

								item->Lock(true);
								item->SetSocket(0, true);
							}
							else
							{
								if (item->GetSocket(0))
								{
									RemoveAffect(pAffect);
									item->Lock(false);
									item->SetSocket(0, false);
								}
							}
							break;
						}
#endif
#ifdef __SOUL__
						if (item->GetValue(3) == 42480)
						{
							const auto* pTable = item->GetProto();
							uint32_t dwType = AFFECT_SOUL;
							uint8_t bBonus1 = aApplyInfo[pTable->aApplies[0].bType].bPointType;
							uint8_t bBonus2 = aApplyInfo[pTable->aApplies[1].bType].bPointType;
							uint8_t bBonus3 = aApplyInfo[pTable->aApplies[2].bType].bPointType;

							CAffect * pAffect1 = FindAffect(dwType, bBonus1);
							CAffect * pAffect2 = FindAffect(dwType, bBonus2);
							CAffect * pAffect3 = FindAffect(dwType, bBonus3);

							if (!pAffect1 && !pAffect2 && !pAffect3)
							{
								AddAffect(dwType, bBonus1, pTable->aApplies[0].lValue, AFF_SOUL_RED, INFINITE_AFFECT_DURATION, 0, true, true);
								AddAffect(dwType, bBonus2, pTable->aApplies[1].lValue, AFF_SOUL_BLUE, INFINITE_AFFECT_DURATION, 0, true, true);
								AddAffect(dwType, bBonus3, pTable->aApplies[2].lValue, AFF_SOUL_MIX, INFINITE_AFFECT_DURATION, 0, true, true);

								item->Lock(true);
								item->SetSocket(0, true);
							}
							else
							{
								if (pAffect1)
									RemoveAffect(pAffect1);
								if (pAffect2)
									RemoveAffect(pAffect2);
								if (pAffect3)
									RemoveAffect(pAffect3);
								if (item->GetSocket(0))
								{

									item->Lock(false);
									item->SetSocket(0, false);
								}
							}
							break;
						}
#endif

						switch (item->GetVnum())
						{
#ifdef __AFAR_BIOLOG__
							case 31129:
							{
								for (int i = 0; i < 10; ++i)
								{
									for (int j = 0; j < 3; ++j)
									{
										CAffect * pAffect = FindAffect(AFFECT_COLLECT, aApplyInfo[BiyologSistemi[i+1][6 + j * 2]].bPointType);
										if (pAffect)
											RemoveAffect(pAffect);
									}
								}
								CAffect * pAffect = FindAffect(AFFECT_COLLECT, 140);
								CAffect * pAffect2 = FindAffect(AFFECT_COLLECT, 141);
								if (pAffect)
									RemoveAffect(pAffect);
								if (pAffect2)
									RemoveAffect(pAffect2);
								SetQuestFlag("bio.durum", 1);
								SetQuestFlag("bio.verilen", 0);
								SetQuestFlag("bio.ruhtasi", 0);
								auto time = get_global_time();
								SetQuestFlag("bio.kalan", time);
								ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[1][0], 0, BiyologSistemi[1][1], time);
								item->SetCount(item->GetCount() - 1);
							}
							break;
#endif
#ifdef __12ZI__
							case 72329:
							{
								if (GetBead() + 30 >= BEAD_MAX)
								{
									ChatPacket(CHAT_TYPE_INFO, "Gecemez la 99 u");
									return false;
								}

								SetBead(GetBead() + 30, true);
								item->SetCount(item->GetCount() - 1);
							}
							break;
							case 72330:
							{
								if (GetBead() + 1 >= BEAD_MAX)
								{
									ChatPacket(CHAT_TYPE_INFO, "Gecemez la 99 u");
									return false;
								}

								SetBead(GetBead() + 1, true);
								item->SetCount(item->GetCount() - 1);
							}
							break;
							case 72328:
							{
								if (!FindAffect(AFFECT_CZ_UNLIMIT_ENTER))
								{
									AddAffect(AFFECT_CZ_UNLIMIT_ENTER, 0, 0, AFF_NONE, 60 * 60 * 3, 0, false);
									item->SetCount(item->GetCount() - 1);
								}
								else
									ChatPacket(CHAT_TYPE_INFO, "Zaten Aktif");
							}
							break;
#endif
#ifdef __SOULBIND__ // @correction202
							case 50263:
								{
									LPITEM item2;
									if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
										return false;
									
									if (item2->IsEquipped() || item2->IsExchanging())
										return false;
									
									if (item2->IsSealed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Item already sealed."));
										return false;
									}

									if (item2->GetType() != ITEM_WEAPON && item2->GetType() != ITEM_ARMOR && item2->GetType() != ITEM_COSTUME)
										return false;

									item2->SetSealBind();
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Item sealbind success."));
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());
										LogManager::instance().ItemLog(this, item, "SET_SEALBIND_SUCCESS", buf);
									}
									item->SetCount(item->GetCount() - 1);
								}
								break;
							
							case 50264:
								{
									LPITEM item2;
									if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
										return false;
									
									if (item2->isLocked() || item2->IsEquipped() || item2->GetSealBindTime() >= 0)
										return false;

									if (item2->GetType() != ITEM_WEAPON && item2->GetType() != ITEM_ARMOR && item2->GetType() != ITEM_COSTUME && item2->GetType() != ITEM_DS)
										return false;

									int32_t duration = 72 * 60 * 60;
									item2->SetSealBind(time(0) + duration);
									item2->StartUnSealBindTimerExpireEvent();
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Item unsealbind success."));
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());
										LogManager::instance().ItemLog(this, item, "REMOVE_SEALBIND_TIME_BEGIN", buf);
									}
									item->SetCount(item->GetCount() - 1);
								}
								break;
#endif
							case ITEM_NOG_POCKET:
								{
									if (FindAffect(AFFECT_NOG_ABILITY))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
										return false;
									}
									int32_t time = item->GetValue(0);
									int32_t moveSpeedPer	= item->GetValue(1);
									int32_t attPer	= item->GetValue(2);
									int32_t expPer			= item->GetValue(3);
									AddAffect(AFFECT_NOG_ABILITY, POINT_MOV_SPEED, moveSpeedPer, AFF_MOV_SPEED_POTION, time, 0, true, true);
									AddAffect(AFFECT_NOG_ABILITY, POINT_MALL_ATTBONUS, attPer, AFF_NONE, time, 0, true, true);
									AddAffect(AFFECT_NOG_ABILITY, POINT_MALL_EXPBONUS, expPer, AFF_NONE, time, 0, true, true);
									item->SetCount(item->GetCount() - 1);
								}
								break;

							case ITEM_RAMADAN_CANDY:
								{
									// @correction065 BEGIN
									if (FindAffect(AFFECT_RAMADAN_ABILITY))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
										return false;
									}
									// @correction065 END
									int32_t time = item->GetValue(0);
									int32_t moveSpeedPer	= item->GetValue(1);
									int32_t attPer	= item->GetValue(2);
									int32_t expPer			= item->GetValue(3);
									AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MOV_SPEED, moveSpeedPer, AFF_MOV_SPEED_POTION, time, 0, true, true);
									AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MALL_ATTBONUS, attPer, AFF_NONE, time, 0, true, true);
									AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MALL_EXPBONUS, expPer, AFF_NONE, time, 0, true, true);
									item->SetCount(item->GetCount() - 1);
								}
								break;
							case ITEM_MARRIAGE_RING:
								{
									marriage::TMarriage* pMarriage = marriage::CManager::instance().Get(GetPlayerID());
									if (pMarriage)
									{
										if (pMarriage->ch1 != nullptr)
										{
											if (CArenaManager::instance().IsArenaMap(pMarriage->ch1->GetMapIndex()) == true)
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
												break;
											}
										}

										if (pMarriage->ch2 != nullptr)
										{
											if (CArenaManager::instance().IsArenaMap(pMarriage->ch2->GetMapIndex()) == true)
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
												break;
											}
										}

										int32_t consumeSP = CalculateConsumeSP(this);

										if (consumeSP < 0)
											return false;

										PointChange(POINT_SP, -consumeSP, false);

										WarpToPID(pMarriage->GetOther(GetPlayerID()));
									}
									else
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°áÈ¥ »óÅÂ°¡ ¾Æ´Ï¸é °áÈ¥¹ÝÁö¸¦ »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
								}
								break;

							case UNIQUE_ITEM_CAPE_OF_COURAGE:
							case 70057:
							case 39006:
							case 70029:
							case REWARD_BOX_UNIQUE_ITEM_CAPE_OF_COURAGE:
								AggregateMonster();
								// item->SetCount(item->GetCount()-1);
								break;

							case UNIQUE_ITEM_WHITE_FLAG:
								ForgetMyAttacker();
								// item->SetCount(item->GetCount()-1);
								break;

							case UNIQUE_ITEM_TREASURE_BOX:
								break;

							case 30093:
							case 30094:
							case 30095:
							case 30096:
								{
									const int32_t MAX_BAG_INFO = 26;
									static struct LuckyBagInfo
									{
										uint32_t count;
										int32_t prob;
										uint32_t vnum;
									} b1[MAX_BAG_INFO] =
									{
										{ 1000,	302,	1 },
										{ 10,	150,	27002 },
										{ 10,	75,	27003 },
										{ 10,	100,	27005 },
										{ 10,	50,	27006 },
										{ 10,	80,	27001 },
										{ 10,	50,	27002 },
										{ 10,	80,	27004 },
										{ 10,	50,	27005 },
										{ 1,	10,	50300 },
										{ 1,	6,	92 },
										{ 1,	2,	132 },
										{ 1,	6,	1052 },
										{ 1,	2,	1092 },
										{ 1,	6,	2082 },
										{ 1,	2,	2122 },
										{ 1,	6,	3082 },
										{ 1,	2,	3122 },
										{ 1,	6,	5052 },
										{ 1,	2,	5082 },
										{ 1,	6,	7082 },
										{ 1,	2,	7122 },
										{ 1,	1,	11282 },
										{ 1,	1,	11482 },
										{ 1,	1,	11682 },
										{ 1,	1,	11882 },
									};

									LuckyBagInfo * bi = nullptr;
									// @correction005
									bi = b1;

									int32_t pct = number(1, 1000);

									int32_t i;
									for (i=0;i<MAX_BAG_INFO;i++)
									{
										if (pct <= bi[i].prob)
											break;
										pct -= bi[i].prob;
									}
									if (i>=MAX_BAG_INFO)
										return false;

									if (bi[i].vnum == 50300)
									{
										GiveRandomSkillBook();
									}
									else if (bi[i].vnum == 1)
									{
										ChangeGold(true, 1000, true);
									}
									else
									{
										AutoGiveItem(bi[i].vnum, bi[i].count);
									}
									item->SetCount(item->GetCount() - 1); // @correction088
								}
								break;

							case 50004:
								{
									if (item->GetSocket(0))
									{
										item->SetSocket(0, item->GetSocket(0) + 1);
									}
									else
									{
										int32_t iMapIndex = GetMapIndex();

										PIXEL_POSITION pos;

										if (SECTREE_MANAGER::instance().GetRandomLocation(iMapIndex, pos, 700))
										{
											item->SetSocket(0, 1);
											item->SetSocket(1, pos.x);
											item->SetSocket(2, pos.y);
										}
										else
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ °÷¿¡¼± ÀÌº¥Æ®¿ë °¨Áö±â°¡ µ¿ÀÛÇÏÁö ¾Ê´Â°Í °°½À´Ï´Ù."));
											return false;
										}
									}

									int32_t dist = 0;
									float distance = (DISTANCE_SQRT(GetX()-item->GetSocket(1), GetY()-item->GetSocket(2)));

									if (distance < 1000.0f)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌº¥Æ®¿ë °¨Áö±â°¡ ½Åºñ·Î¿î ºûÀ» ³»¸ç »ç¶óÁý´Ï´Ù."));

										struct TEventStoneInfo
										{
											uint32_t dwVnum;
											int32_t count;
											int32_t prob;
										};
										const int32_t EVENT_STONE_MAX_INFO = 15;
										TEventStoneInfo info_10[EVENT_STONE_MAX_INFO] =
										{
											{ 27001, 10,  8 },
											{ 27004, 10,  6 },
											{ 27002, 10, 12 },
											{ 27005, 10, 12 },
											{ 27100,  1,  9 },
											{ 27103,  1,  9 },
											{ 27101,  1, 10 },
											{ 27104,  1, 10 },
											{ 27999,  1, 12 },

											{ 25040,  1,  4 },

											{ 27410,  1,  0 },
											{ 27600,  1,  0 },
											{ 25100,  1,  0 },

											{ 50001,  1,  0 },
											{ 50003,  1,  1 },
										};
										TEventStoneInfo info_7[EVENT_STONE_MAX_INFO] =
										{
											{ 27001, 10,  1 },
											{ 27004, 10,  1 },
											{ 27004, 10,  9 },
											{ 27005, 10,  9 },
											{ 27100,  1,  5 },
											{ 27103,  1,  5 },
											{ 27101,  1, 10 },
											{ 27104,  1, 10 },
											{ 27999,  1, 14 },

											{ 25040,  1,  5 },

											{ 27410,  1,  5 },
											{ 27600,  1,  5 },
											{ 25100,  1,  5 },

											{ 50001,  1,  0 },
											{ 50003,  1,  5 },

										};
										TEventStoneInfo info_4[EVENT_STONE_MAX_INFO] =
										{
											{ 27001, 10,  0 },
											{ 27004, 10,  0 },
											{ 27002, 10,  0 },
											{ 27005, 10,  0 },
											{ 27100,  1,  0 },
											{ 27103,  1,  0 },
											{ 27101,  1,  0 },
											{ 27104,  1,  0 },
											{ 27999,  1, 25 },

											{ 25040,  1,  0 },

											{ 27410,  1,  0 },
											{ 27600,  1,  0 },
											{ 25100,  1, 15 },

											{ 50001,  1, 10 },
											{ 50003,  1, 50 },

										};

										{
											TEventStoneInfo* info;
											if (item->GetSocket(0) <= 4)
												info = info_4;
											else if (item->GetSocket(0) <= 7)
												info = info_7;
											else
												info = info_10;

											int32_t prob = number(1, 100);

											for (int32_t i = 0; i < EVENT_STONE_MAX_INFO; ++i)
											{
												if (!info[i].prob)
													continue;

												if (prob <= info[i].prob)
												{
													if (info[i].dwVnum == 50001)
													{
														uint32_t * pdw = M2_NEW uint32_t[2];

														pdw[0] = info[i].dwVnum;
														pdw[1] = info[i].count;

														DBManager::instance().ReturnQuery(QID_LOTTO, GetPlayerID(), pdw,
																"INSERT INTO lotto_list VALUES(0, 'server%s', %u, NOW())",
																get_table_postfix(), GetPlayerID());
													}
													else
														AutoGiveItem(info[i].dwVnum, info[i].count);

													break;
												}
												prob -= info[i].prob;
											}
										}

										char chatbuf[CHAT_MAX_LEN + 1];
										int32_t len = snprintf(chatbuf, sizeof(chatbuf), "StoneDetect %u 0 0", (uint32_t)GetVID());

										if (len < 0 || len >= (int32_t) sizeof(chatbuf))
											len = sizeof(chatbuf) - 1;

										++len;

										TPacketGCChat pack_chat;
										pack_chat.header	= HEADER_GC_CHAT;
										pack_chat.size		= sizeof(TPacketGCChat) + len;
										pack_chat.type		= CHAT_TYPE_COMMAND;
										pack_chat.id		= 0;
										pack_chat.bEmpire	= GetDesc()->GetEmpire();

										TEMP_BUFFER buf;
										buf.write(&pack_chat, sizeof(TPacketGCChat));
										buf.write(chatbuf, len);

										PacketAround(buf.read_peek(), buf.size());

										ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (DETECT_EVENT_STONE) 1");
										return true;
									}
									else if (distance < 20000)
										dist = 1;
									else if (distance < 70000)
										dist = 2;
									else
										dist = 3;

									const int32_t STONE_DETECT_MAX_TRY = 10;
									if (item->GetSocket(0) >= STONE_DETECT_MAX_TRY)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌº¥Æ®¿ë °¨Áö±â°¡ ÈçÀûµµ ¾øÀÌ »ç¶óÁý´Ï´Ù."));
										ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (DETECT_EVENT_STONE) 0");
										AutoGiveItem(27002);
										return true;
									}

									if (dist)
									{
										char chatbuf[CHAT_MAX_LEN + 1];
										int32_t len = snprintf(chatbuf, sizeof(chatbuf),
												"StoneDetect %u %d %d",
											   	(uint32_t)GetVID(), dist, (int32_t)GetDegreeFromPositionXY(GetX(), item->GetSocket(2), item->GetSocket(1), GetY()));

										if (len < 0 || len >= (int32_t) sizeof(chatbuf))
											len = sizeof(chatbuf) - 1;

										++len;

										TPacketGCChat pack_chat;
										pack_chat.header	= HEADER_GC_CHAT;
										pack_chat.size		= sizeof(TPacketGCChat) + len;
										pack_chat.type		= CHAT_TYPE_COMMAND;
										pack_chat.id		= 0;
										pack_chat.bEmpire	= GetDesc()->GetEmpire();

										TEMP_BUFFER buf;
										buf.write(&pack_chat, sizeof(TPacketGCChat));
										buf.write(chatbuf, len);

										PacketAround(buf.read_peek(), buf.size());
									}

								}
								break;
#ifdef __FAST_RUHBK__
							case 50513:
								ChatPacket(CHAT_TYPE_COMMAND, "fastruh");
								break;
#endif
#ifdef __AFAR_BIOLOG__
							case 39023:
								if (int32_t(GetQuestFlag("bio.sans")) == 1)
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosanszatenaktif"));
								else if (GetQuestFlag("bio.durum") == 31 || GetQuestFlag("bio.durum") == 41 || GetQuestFlag("bio.durum") == 51 || GetQuestFlag("bio.durum") == 61 || GetQuestFlag("bio.durum") == 71 || GetQuestFlag("bio.durum") == 81 || GetQuestFlag("bio.durum") == 91 || GetQuestFlag("bio.durum") == 93 || GetQuestFlag("bio.durum") == 11)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevinyok"));
									ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
								}
								else if (GetQuestFlag("bio.ruhtasi") == 1)
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhdayapamazsin"));
								else
								{
									item->SetCount(item->GetCount() - 1);
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosansverildi"));
									SetQuestFlag("bio.sans", 1);
								}
								break;
							case 31029:
								if (int32_t(GetQuestFlag("bio.sure")) == 1)
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosurezatenaktif"));
								else if (GetQuestFlag("bio.durum") == 31 || GetQuestFlag("bio.durum") == 41 || GetQuestFlag("bio.durum") == 51 || GetQuestFlag("bio.durum") == 61 || GetQuestFlag("bio.durum") == 71 || GetQuestFlag("bio.durum") == 81 || GetQuestFlag("bio.durum") == 91 || GetQuestFlag("bio.durum") == 93 || GetQuestFlag("bio.durum") == 11)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevinyok"));
									ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
								}
								else if (GetQuestFlag("bio.ruhtasi") == 1)
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("bioruhdayapamazsin"));
								else
								{
									item->SetCount(item->GetCount() - 1);
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosureverildi"));
									SetQuestFlag("bio.sure", 1);
									SetQuestFlag("bio.kalan", 0);
									ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[GetQuestFlag("bio.durum")][0], GetQuestFlag("bio.verilen"), BiyologSistemi[GetQuestFlag("bio.durum")][1], GetQuestFlag("bio.kalan"));
								}
								break;
							case 31030:
								if (int32_t(GetQuestFlag("bio.sure")) == 1)
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosurezatenaktif"));
								else if (GetQuestFlag("bio.durum") == 31 || GetQuestFlag("bio.durum") == 41 || GetQuestFlag("bio.durum") == 51 || GetQuestFlag("bio.durum") == 61 || GetQuestFlag("bio.durum") == 71 || GetQuestFlag("bio.durum") == 81 || GetQuestFlag("bio.durum") == 91 || GetQuestFlag("bio.durum") == 93 || GetQuestFlag("bio.durum") == 11)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biogorevinyok"));
									ChatPacket(CHAT_TYPE_COMMAND, "biyolog 0 0 0 0");
								}
								else
								{
									item->SetCount(item->GetCount() - 1);
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosureverildi"));
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("biosansverildi"));
									SetQuestFlag("bio.sure", 1);
									SetQuestFlag("bio.sans", 1);
									SetQuestFlag("bio.kalan", 0);
									ChatPacket(CHAT_TYPE_COMMAND, "biyolog %d %d %d %d ", BiyologSistemi[GetQuestFlag("bio.durum")][0], GetQuestFlag("bio.verilen"), BiyologSistemi[GetQuestFlag("bio.durum")][1], GetQuestFlag("bio.kalan"));
								}
								break;
#endif
							case 27989:
							case 76006:
								{
									LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(GetMapIndex());

									if (pMap != nullptr)
									{
										// item->SetSocket(0, item->GetSocket(0) + 1);

										FFindStone f;

										pMap->for_each(f);

										if (f.m_mapStone.size() > 0)
										{
											std::map<uint32_t, LPCHARACTER>::iterator stone = f.m_mapStone.begin();

											uint32_t max = UINT_MAX;
											LPCHARACTER pTarget = stone->second;

											while (stone != f.m_mapStone.end())
											{
												uint32_t dist = (uint32_t)DISTANCE_SQRT(GetX()-stone->second->GetX(), GetY()-stone->second->GetY());

												if (dist != 0 && max > dist)
												{
													max = dist;
													pTarget = stone->second;
												}
												stone++;
											}

											if (pTarget != nullptr)
											{
												int32_t val = 3;

												if (max < 10000) val = 2;
												else if (max < 70000) val = 1;

												ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u %d %d", (uint32_t)GetVID(), val,
														(int32_t)GetDegreeFromPositionXY(GetX(), pTarget->GetY(), pTarget->GetX(), GetY()));
											}
											else
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°¨Áö±â¸¦ ÀÛ¿ëÇÏ¿´À¸³ª °¨ÁöµÇ´Â ¿µ¼®ÀÌ ¾ø½À´Ï´Ù."));
											}
										}
										else
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°¨Áö±â¸¦ ÀÛ¿ëÇÏ¿´À¸³ª °¨ÁöµÇ´Â ¿µ¼®ÀÌ ¾ø½À´Ï´Ù."));
										}

										// if (item->GetSocket(0) >= 6)
										// {
											// ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u 0 0", (uint32_t)GetVID());
											// ITEM_MANAGER::instance().RemoveItem(item);
										// }
									}
									break;
								}
								break;

							case 27996:
								item->SetCount(item->GetCount() - 1);
								AttackedByPoison(nullptr); // @correction007
								break;

							case 27987:
								{
									item->SetCount(item->GetCount() - 1);

									int32_t r = number(1, 100);

									if (r <= 50)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á¶°³¿¡¼­ µ¹Á¶°¢ÀÌ ³ª¿Ô½À´Ï´Ù."));
										AutoGiveItem(27990);
									}
									else
									{
										const int32_t prob_table_gb2312[] =
										{
											95, 97, 99
										};

										const int32_t * prob_table = prob_table_gb2312;

										if (r <= prob_table[0])
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á¶°³°¡ ÈçÀûµµ ¾øÀÌ »ç¶óÁý´Ï´Ù."));
										}
										else if (r <= prob_table[1])
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á¶°³¿¡¼­ ¹éÁøÁÖ°¡ ³ª¿Ô½À´Ï´Ù."));
											AutoGiveItem(27992);
										}
										else if (r <= prob_table[2])
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á¶°³¿¡¼­ Ã»ÁøÁÖ°¡ ³ª¿Ô½À´Ï´Ù."));
											AutoGiveItem(27993);
										}
										else
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á¶°³¿¡¼­ ÇÇÁøÁÖ°¡ ³ª¿Ô½À´Ï´Ù."));
											AutoGiveItem(27994);
										}
									}
								}
								break;

							case 71013:
								CreateFly(number(FLY_FIREWORK1, FLY_FIREWORK6), this);
								item->SetCount(item->GetCount() - 1);
								break;

							case 50100:
							case 50101:
							case 50102:
							case 50103:
							case 50104:
							case 50105:
							case 50106:
								CreateFly(item->GetVnum() - 50100 + FLY_FIREWORK1, this);
								item->SetCount(item->GetCount() - 1);
								break;

							case 50200:
								// @correction005
								if (g_bEnableBootaryCheck) // @correction211
								{
									if (IS_BOTARYABLE_ZONE(GetMapIndex()) == true)
									{
										__OpenPrivateShop();
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³ÀÎ »óÁ¡À» ¿­ ¼ö ¾ø´Â Áö¿ªÀÔ´Ï´Ù"));
									}
								}
								else
								{
									__OpenPrivateShop();
								}
								break;

							case fishing::FISH_MIND_PILL_VNUM:
								AddAffect(AFFECT_FISH_MIND_PILL, POINT_NONE, 0, AFF_FISH_MIND, 20*60, 0, true);
								item->SetCount(item->GetCount() - 1);
								break;

							case 50301:
							case 50302:
							case 50303:
								{
									if (IsPolymorphed() == true)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µÐ°© Áß¿¡´Â ´É·ÂÀ» ¿Ã¸± ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}

									int32_t lv = GetSkillLevel(SKILL_LEADERSHIP);

									if (lv < item->GetValue(0))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ³Ê¹« ¾î·Á¿ö ÀÌÇØÇÏ±â°¡ Èûµì´Ï´Ù."));
										return false;
									}

									if (lv >= item->GetValue(1))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ¾Æ¹«¸® ºÁµµ µµ¿òÀÌ µÉ °Í °°Áö ¾Ê½À´Ï´Ù."));
										return false;
									}

									if (LearnSkillByBook(SKILL_LEADERSHIP))
									{
										item->SetCount(item->GetCount() - 1); // @correction088
										int32_t iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
										if (distribution_test_server) iReadDelay /= 3;

										SetSkillNextReadTime(SKILL_LEADERSHIP, get_global_time() + iReadDelay);
									}
								}
								break;

							case 50304:
							case 50305:
							case 50306:
								{
									if (IsPolymorphed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯½ÅÁß¿¡´Â Ã¥À» ÀÐÀ»¼ö ¾ø½À´Ï´Ù."));
										return false;

									}
									if (GetSkillLevel(SKILL_COMBO) == 0 && GetLevel() < 30)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("·¹º§ 30ÀÌ µÇ±â Àü¿¡´Â ½ÀµæÇÒ ¼ö ÀÖÀ» °Í °°Áö ¾Ê½À´Ï´Ù."));
										return false;
									}

									if (GetSkillLevel(SKILL_COMBO) == 1 && GetLevel() < 50)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("·¹º§ 50ÀÌ µÇ±â Àü¿¡´Â ½ÀµæÇÒ ¼ö ÀÖÀ» °Í °°Áö ¾Ê½À´Ï´Ù."));
										return false;
									}

									if (GetSkillLevel(SKILL_COMBO) >= 2)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿¬°è±â´Â ´õÀÌ»ó ¼ö·ÃÇÒ ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}

									int32_t iPct = item->GetValue(0);

									if (LearnSkillByBook(SKILL_COMBO, iPct))
									{
										item->SetCount(item->GetCount() - 1); // @correction088
										int32_t iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
										if (distribution_test_server) iReadDelay /= 3;

										SetSkillNextReadTime(SKILL_COMBO, get_global_time() + iReadDelay);
									}
								}
								break;
							case 50311:
							case 50312:
							case 50313:
								{
									if (IsPolymorphed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯½ÅÁß¿¡´Â Ã¥À» ÀÐÀ»¼ö ¾ø½À´Ï´Ù."));
										return false;

									}
									uint32_t dwSkillVnum = item->GetValue(0);
									int32_t iPct = MINMAX(0, item->GetValue(1), 100);
									if (GetSkillLevel(dwSkillVnum)>=20 || dwSkillVnum-SKILL_LANGUAGE1+1 == GetEmpire())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì ¿Ïº®ÇÏ°Ô ¾Ë¾ÆµéÀ» ¼ö ÀÖ´Â ¾ð¾îÀÌ´Ù."));
										return false;
									}

									if (LearnSkillByBook(dwSkillVnum, iPct))
									{
										item->SetCount(item->GetCount() - 1); // @correction088

										int32_t iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
										if (distribution_test_server) iReadDelay /= 3;

										SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
									}
								}
								break;

							case 50061 :
								{
									if (IsPolymorphed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯½ÅÁß¿¡´Â Ã¥À» ÀÐÀ»¼ö ¾ø½À´Ï´Ù."));
										return false;

									}
									uint32_t dwSkillVnum = item->GetValue(0);
									int32_t iPct = MINMAX(0, item->GetValue(1), 100);

									if (GetSkillLevel(dwSkillVnum) >= 10)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ¼ö·ÃÇÒ ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}

									if (LearnSkillByBook(dwSkillVnum, iPct))
									{
										item->SetCount(item->GetCount() - 1); // @correction088

										int32_t iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
										if (distribution_test_server) iReadDelay /= 3;

										SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
									}
								}
								break;

							case 50314: case 50315: case 50316:
							case 50327: case 50328:
							case 50329: case 50330:
								{
									if (IsPolymorphed() == true)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µÐ°© Áß¿¡´Â ´É·ÂÀ» ¿Ã¸± ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}

									int32_t iSkillLevelLowLimit = item->GetValue(0);
									int32_t iSkillLevelHighLimit = item->GetValue(1);
									int32_t iPct = MINMAX(0, item->GetValue(2), 100);
									int32_t iLevelLimit = item->GetValue(3);
									uint32_t dwSkillVnum = 0;

									switch (item->GetVnum())
									{
										case 50314: case 50315: case 50316:
											dwSkillVnum = SKILL_POLYMORPH;
											break;

										case 50327: case 50328:
											dwSkillVnum = SKILL_ADD_HP;
											break;

										case 50329: case 50330:
											dwSkillVnum = SKILL_RESIST_PENETRATE;
											break;

										default:
											return false;
									}

									if (0 == dwSkillVnum)
										return false;

									if (GetLevel() < iLevelLimit)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥À» ÀÐÀ¸·Á¸é ·¹º§À» ´õ ¿Ã·Á¾ß ÇÕ´Ï´Ù."));
										return false;
									}

									if (GetSkillLevel(dwSkillVnum) >= 40)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ¼ö·ÃÇÒ ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}

									if (GetSkillLevel(dwSkillVnum) < iSkillLevelLowLimit)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥Àº ³Ê¹« ¾î·Á¿ö ÀÌÇØÇÏ±â°¡ Èûµì´Ï´Ù."));
										return false;
									}

									if (GetSkillLevel(dwSkillVnum) >= iSkillLevelHighLimit)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ Ã¥À¸·Î´Â ´õ ÀÌ»ó ¼ö·ÃÇÒ ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}

									if (LearnSkillByBook(dwSkillVnum, iPct))
									{
										item->SetCount(item->GetCount() - 1); // @correction088

										int32_t iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
										if (distribution_test_server) iReadDelay /= 3;

										SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
									}
								}
								break;

							case 50902:
							case 50903:
							case 50904:
								{
									if (IsPolymorphed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯½ÅÁß¿¡´Â Ã¥À» ÀÐÀ»¼ö ¾ø½À´Ï´Ù."));
										return false;

									}
									uint32_t dwSkillVnum = SKILL_CREATE;
									int32_t iPct = MINMAX(0, item->GetValue(1), 100);

									if (GetSkillLevel(dwSkillVnum)>=40)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ¼ö·ÃÇÒ ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}

									if (LearnSkillByBook(dwSkillVnum, iPct))
									{
										item->SetCount(item->GetCount() - 1); // @correction088

										int32_t iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
										if (distribution_test_server) iReadDelay /= 3;

										SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);

										if (test_server)
										{
											ChatPacket(CHAT_TYPE_INFO, "[TEST_SERVER] Success to learn skill ");
										}
									}
									else
									{
										if (test_server)
										{
											ChatPacket(CHAT_TYPE_INFO, "[TEST_SERVER] Failed to learn skill ");
										}
									}
								}
								break;

							case ITEM_MINING_SKILL_TRAIN_BOOK:
								{
									if (IsPolymorphed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯½ÅÁß¿¡´Â Ã¥À» ÀÐÀ»¼ö ¾ø½À´Ï´Ù."));
										return false;

									}
									uint32_t dwSkillVnum = SKILL_MINING;
									int32_t iPct = MINMAX(0, item->GetValue(1), 100);

									if (GetSkillLevel(dwSkillVnum)>=40)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ¼ö·ÃÇÒ ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}

									if (LearnSkillByBook(dwSkillVnum, iPct))
									{
										item->SetCount(item->GetCount() - 1); // @correction088

										int32_t iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
										if (distribution_test_server) iReadDelay /= 3;

										SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
									}
								}
								break;

							case ITEM_HORSE_SKILL_TRAIN_BOOK:
								{
									if (IsPolymorphed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯½ÅÁß¿¡´Â Ã¥À» ÀÐÀ»¼ö ¾ø½À´Ï´Ù."));
										return false;

									}
									uint32_t dwSkillVnum = SKILL_HORSE;
									int32_t iPct = MINMAX(0, item->GetValue(1), 100);

									if (GetLevel() < 50)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ ½Â¸¶ ½ºÅ³À» ¼ö·ÃÇÒ ¼ö ÀÖ´Â ·¹º§ÀÌ ¾Æ´Õ´Ï´Ù."));
										return false;
									}

									if (!test_server && get_global_time() < GetSkillNextReadTime(dwSkillVnum))
									{
										if (FindAffect(AFFECT_SKILL_NO_BOOK_DELAY))
										{
											RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÁÖ¾È¼ú¼­¸¦ ÅëÇØ ÁÖÈ­ÀÔ¸¶¿¡¼­ ºüÁ®³ª¿Ô½À´Ï´Ù."));
										}
										else
										{
											SkillLearnWaitMoreTimeMessage(GetSkillNextReadTime(dwSkillVnum) - get_global_time());
											return false;
										}
									}

									if (GetPoint(POINT_HORSE_SKILL) >= 20 ||
											GetSkillLevel(SKILL_HORSE_WILDATTACK) + GetSkillLevel(SKILL_HORSE_CHARGE) + GetSkillLevel(SKILL_HORSE_ESCAPE) >= 60 ||
											GetSkillLevel(SKILL_HORSE_WILDATTACK_RANGE) + GetSkillLevel(SKILL_HORSE_CHARGE) + GetSkillLevel(SKILL_HORSE_ESCAPE) >= 60)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ½Â¸¶ ¼ö·Ã¼­¸¦ ÀÐÀ» ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}

									if (number(1, 100) <= iPct)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("½Â¸¶ ¼ö·Ã¼­¸¦ ÀÐ¾î ½Â¸¶ ½ºÅ³ Æ÷ÀÎÆ®¸¦ ¾ò¾ú½À´Ï´Ù."));
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾òÀº Æ÷ÀÎÆ®·Î´Â ½Â¸¶ ½ºÅ³ÀÇ ·¹º§À» ¿Ã¸± ¼ö ÀÖ½À´Ï´Ù."));
										PointChange(POINT_HORSE_SKILL, 1);

										int32_t iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
										if (distribution_test_server) iReadDelay /= 3;

										if (!test_server)
											SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("½Â¸¶ ¼ö·Ã¼­ ÀÌÇØ¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
									}
									item->SetCount(item->GetCount() - 1); // @correction088
								}
								break;

							case 70102:
							case 70103:
								{
									if (GetAlignment() >= 0)
										return false;

									int32_t delta = MIN(-GetAlignment(), item->GetValue(0));

									sys_log(0, "%s ALIGNMENT ITEM %d", GetName(), delta);

									UpdateAlignment(delta);
									item->SetCount(item->GetCount() - 1);

									if (delta / 10 > 0)
									{
										ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("¸¶À½ÀÌ ¸¼¾ÆÁö´Â±º. °¡½¿À» Áþ´©¸£´ø ¹«¾ð°¡°¡ Á» °¡º­¿öÁø ´À³¦ÀÌ¾ß."));
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼±¾ÇÄ¡°¡ %d Áõ°¡ÇÏ¿´½À´Ï´Ù."), delta/10);
									}
								}
								break;

							case 39032:
							case 71107:
								{
									int32_t val = item->GetValue(0);
									int32_t interval = item->GetValue(1);
									quest::PC* pPC = quest::CQuestManager::instance().GetPC(GetPlayerID());
									if (!pPC) // @correction019
										return false;
									int32_t last_use_time = pPC->GetFlag("mythical_peach.last_use_time");

									if (get_global_time() - last_use_time < interval * 60 * 60)
									{
										if (test_server == false)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
											return false;
										}
										else
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Å×½ºÆ® ¼­¹ö ½Ã°£Á¦ÇÑ Åë°ú"));
										}
									}

									if (GetAlignment() == 300000)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼±¾ÇÄ¡¸¦ ´õ ÀÌ»ó ¿Ã¸± ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}

									if (300000 - GetAlignment() < val * 10)
									{
										val = (300000 - GetAlignment()) / 10;
									}

									int32_t old_alignment = GetAlignment() / 10;

									UpdateAlignment(val*10);

									item->SetCount(item->GetCount()-1);
									pPC->SetFlag("mythical_peach.last_use_time", get_global_time());

									ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("¸¶À½ÀÌ ¸¼¾ÆÁö´Â±º. °¡½¿À» Áþ´©¸£´ø ¹«¾ð°¡°¡ Á» °¡º­¿öÁø ´À³¦ÀÌ¾ß."));
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼±¾ÇÄ¡°¡ %d Áõ°¡ÇÏ¿´½À´Ï´Ù."), val);

									char buf[256 + 1];
									snprintf(buf, sizeof(buf), "%d %d", old_alignment, GetAlignment() / 10);
									LogManager::instance().CharLog(this, val, "MYTHICAL_PEACH", buf);
								}
								break;

							case 71109:
							case 72719:
								{
									LPITEM item2;

									if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
										return false;

									if (item2->IsExchanging() || item2->IsEquipped()) // @correction038
										return false;

									if (item2->GetSocketCount() == 0)
										return false;

									switch( item2->GetType() )
									{
										case ITEM_WEAPON:
											break;
										case ITEM_ARMOR:
											switch (item2->GetSubType())
											{
											case ARMOR_EAR:
											case ARMOR_WRIST:
											case ARMOR_NECK:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»©³¾ ¿µ¼®ÀÌ ¾ø½À´Ï´Ù"));
												return false;
											}
											break;

										default:
											return false;
									}

									std::stack<int32_t> socket;

									for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
										socket.push(item2->GetSocket(i));

									int32_t idx = ITEM_SOCKET_MAX_NUM - 1;

									while (socket.size() > 0)
									{
										if (socket.top() > 2 && socket.top() != ITEM_BROKEN_METIN_VNUM)
											break;

										idx--;
										socket.pop();
									}

									if (socket.size() == 0)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»©³¾ ¿µ¼®ÀÌ ¾ø½À´Ï´Ù"));
										return false;
									}

									LPITEM pItemReward = AutoGiveItem(socket.top());

									if (pItemReward != nullptr)
									{
										item2->SetSocket(idx, 1);

										char buf[256+1];
										snprintf(buf, sizeof(buf), "%s(%u) %s(%u)",
												item2->GetName(), item2->GetID(), pItemReward->GetName(), pItemReward->GetID());
										LogManager::instance().ItemLog(this, item, "USE_DETACHMENT_ONE", buf);

										item->SetCount(item->GetCount() - 1);
									}
								}
								break;

							case 70201:
							case 70202:
							case 70203:
							case 70204:
							case 70205:
							case 70206:
								{
									if (GetPart(PART_HAIR) >= 1001)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÇöÀç Çì¾î½ºÅ¸ÀÏ¿¡¼­´Â ¿°»ö°ú Å»»öÀÌ ºÒ°¡´ÉÇÕ´Ï´Ù."));
									}
									else
									{
										quest::CQuestManager& q = quest::CQuestManager::instance();
										quest::PC* pPC = q.GetPC(GetPlayerID());

										if (pPC)
										{
											int32_t last_dye_level = pPC->GetFlag("dyeing_hair.last_dye_level");

											if (last_dye_level == 0 ||
													last_dye_level+3 <= GetLevel() ||
													item->GetVnum() == 70201)
											{
												SetPart(PART_HAIR, item->GetVnum() - 70201);

												if (item->GetVnum() == 70201)
													pPC->SetFlag("dyeing_hair.last_dye_level", 0);
												else
													pPC->SetFlag("dyeing_hair.last_dye_level", GetLevel());

												item->SetCount(item->GetCount() - 1);
												UpdatePacket();
											}
											else
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d ·¹º§ÀÌ µÇ¾î¾ß ´Ù½Ã ¿°»öÇÏ½Ç ¼ö ÀÖ½À´Ï´Ù."), last_dye_level+3);
											}
										}
									}
								}
								break;

							case ITEM_NEW_YEAR_GREETING_VNUM:
								{
									uint32_t dwBoxVnum = ITEM_NEW_YEAR_GREETING_VNUM;
									std::vector <uint32_t> dwVnums;
									std::vector <uint32_t> dwCounts;
									std::vector <LPITEM> item_gets;
									int32_t count = 0;

									if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count, wCount))
									{
										for (int32_t i = 0; i < count; i++)
										{
											if (dwVnums[i] == CSpecialItemGroup::GOLD)
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µ· %d ³ÉÀ» È¹µæÇß½À´Ï´Ù."), dwCounts[i]);
										}

										item->SetCount(item->GetCount() - 1);
									}
								}
								break;

							case ITEM_VALENTINE_ROSE:
							case ITEM_VALENTINE_CHOCOLATE:
								{
									uint32_t dwBoxVnum = item->GetVnum();
									std::vector <uint32_t> dwVnums;
									std::vector <uint32_t> dwCounts;
									std::vector <LPITEM> item_gets(0);
									int32_t count = 0;


									if (((item->GetVnum() == ITEM_VALENTINE_ROSE) && (SEX_MALE==GET_SEX(this))) ||
										((item->GetVnum() == ITEM_VALENTINE_CHOCOLATE) && (SEX_FEMALE==GET_SEX(this))))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ºº°ÀÌ ¸ÂÁö¾Ê¾Æ ÀÌ ¾ÆÀÌÅÛÀ» ¿­ ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}


									if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count, wCount))
										item->SetCount(item->GetCount()-1);
								}
								break;

							case ITEM_WHITEDAY_CANDY:
							case ITEM_WHITEDAY_ROSE:
								{
									uint32_t dwBoxVnum = item->GetVnum();
									std::vector <uint32_t> dwVnums;
									std::vector <uint32_t> dwCounts;
									std::vector <LPITEM> item_gets(0);
									int32_t count = 0;


									if (((item->GetVnum() == ITEM_WHITEDAY_CANDY) && (SEX_MALE==GET_SEX(this))) ||
										((item->GetVnum() == ITEM_WHITEDAY_ROSE) && (SEX_FEMALE==GET_SEX(this))))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ºº°ÀÌ ¸ÂÁö¾Ê¾Æ ÀÌ ¾ÆÀÌÅÛÀ» ¿­ ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}


									if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count, wCount))
										item->SetCount(item->GetCount()-1);
								}
								break;

							case 50011:
								{
									uint32_t dwBoxVnum = 50011;
									std::vector <uint32_t> dwVnums;
									std::vector <uint32_t> dwCounts;
									std::vector <LPITEM> item_gets(0);
									int32_t count = 0;

									if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count, wCount))
									{
										for (int32_t i = 0; i < count; i++)
										{
											char buf[50 + 1];
											snprintf(buf, sizeof(buf), "%u %u", dwVnums[i], dwCounts[i]);
											LogManager::instance().ItemLog(this, item, "MOONLIGHT_GET", buf);

											item->SetCount(item->GetCount() - 1);

											switch (dwVnums[i])
											{
											case CSpecialItemGroup::GOLD:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µ· %d ³ÉÀ» È¹µæÇß½À´Ï´Ù."), dwCounts[i]);
												break;

											case CSpecialItemGroup::EXP:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ºÎÅÍ ½ÅºñÇÑ ºûÀÌ ³ª¿É´Ï´Ù."));
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÀÇ °æÇèÄ¡¸¦ È¹µæÇß½À´Ï´Ù."), dwCounts[i]);
												break;

											case CSpecialItemGroup::MOB:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ¸ó½ºÅÍ°¡ ³ªÅ¸³µ½À´Ï´Ù!"));
												break;

											case CSpecialItemGroup::SLOW:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ³ª¿Â »¡°£ ¿¬±â¸¦ µéÀÌ¸¶½ÃÀÚ ¿òÁ÷ÀÌ´Â ¼Óµµ°¡ ´À·ÁÁ³½À´Ï´Ù!"));
												break;

											case CSpecialItemGroup::DRAIN_HP:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ°¡ °©ÀÚ±â Æø¹ßÇÏ¿´½À´Ï´Ù! »ý¸í·ÂÀÌ °¨¼ÒÇß½À´Ï´Ù."));
												break;

											case CSpecialItemGroup::POISON:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ³ª¿Â ³ì»ö ¿¬±â¸¦ µéÀÌ¸¶½ÃÀÚ µ¶ÀÌ ¿Â¸öÀ¸·Î ÆÛÁý´Ï´Ù!"));
												break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
											case CSpecialItemGroup::BLEEDING:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ³ª¿Â ³ì»ö ¿¬±â¸¦ µéÀÌ¸¶½ÃÀÚ µ¶ÀÌ ¿Â¸öÀ¸·Î ÆÛÁý´Ï´Ù!"));
												break;
#endif
											case CSpecialItemGroup::MOB_GROUP:
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ¸ó½ºÅÍ°¡ ³ªÅ¸³µ½À´Ï´Ù!"));
												break;

											default:
												if (item_gets[i])
												{
													if (dwCounts[i] > 1)
														ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ %s °¡ %d °³ ³ª¿Ô½À´Ï´Ù."), item_gets[i]->GetName(), dwCounts[i]);
													else
														ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ %s °¡ ³ª¿Ô½À´Ï´Ù."), item_gets[i]->GetName());
												}
												break;
											}
										}
									}
									else
									{
										ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("¾Æ¹«°Íµµ ¾òÀ» ¼ö ¾ø¾ú½À´Ï´Ù."));
										return false;
									}
								}
								break;

							case ITEM_GIVE_STAT_RESET_COUNT_VNUM:
								{
									PointChange(POINT_STAT_RESET_COUNT, 1);
									item->SetCount(item->GetCount()-1);
								}
								break;

							case 50107:
								{
// @correction131 BEGIN
									if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
										return false;
									}
// @correction131 END
// @correction230 BEGIN
									else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
										return false;
									}
// @correction230 END

									EffectPacket(SE_CHINA_FIREWORK);
#ifdef ENABLE_FIREWORK_STUN // @correction165
									AddAffect(AFFECT_CHINA_FIREWORK, POINT_STUN_PCT, 30, AFF_CHINA_FIREWORK, 5*60, 0, true);
#endif
									item->SetCount(item->GetCount()-1);
								}
								break;

							case 50108:
								{
									if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
										return false;
									}
// @correction230 BEGIN
									else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
										return false;
									}
// @correction230 END

									EffectPacket(SE_SPIN_TOP);
#ifdef ENABLE_FIREWORK_STUN // @correction165
									AddAffect(AFFECT_CHINA_FIREWORK, POINT_STUN_PCT, 30, AFF_CHINA_FIREWORK, 5*60, 0, true);
#endif
									item->SetCount(item->GetCount()-1);
								}
								break;

							case ITEM_WONSO_BEAN_VNUM:
								PointChange(POINT_HP, GetMaxHP() - GetHP());
								item->SetCount(item->GetCount()-1);
								break;

							case ITEM_WONSO_SUGAR_VNUM:
								PointChange(POINT_SP, GetMaxSP() - GetSP());
								item->SetCount(item->GetCount()-1);
								break;

							case ITEM_WONSO_FRUIT_VNUM:
								PointChange(POINT_STAMINA, GetMaxStamina()-GetStamina());
								item->SetCount(item->GetCount()-1);
								break;

							case 90008:
							case 90009:
								VCardUse(this, this, item);
								break;

							case ITEM_ELK_VNUM:
								{
									int32_t iGold = item->GetSocket(0);
									ITEM_MANAGER::instance().RemoveItem(item);
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µ· %d ³ÉÀ» È¹µæÇß½À´Ï´Ù."), iGold);
									ChangeGold(true, iGold);
								}
								break;

							case 70021:
								{
									int32_t HealPrice = quest::CQuestManager::instance().GetEventFlag("MonarchHealGold");
									if (HealPrice == 0)
										HealPrice = 2000000;

									if (CMonarch::instance().HealMyEmpire(this, HealPrice))
									{
										char szNotice[256];
										snprintf(szNotice, sizeof(szNotice), LC_TEXT("±ºÁÖÀÇ Ãàº¹À¸·Î ÀÌÁö¿ª %s À¯Àú´Â HP,SP°¡ ¸ðµÎ Ã¤¿öÁý´Ï´Ù."), EMPIRE_NAME(GetEmpire()));
										SendNoticeMap(szNotice, GetMapIndex(), false);

										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁÖÀÇ Ãàº¹À» »ç¿ëÇÏ¿´½À´Ï´Ù."));
									}
								}
								break;

							case 27995:
								{
								}
								break;

							case 71092 :
								{
									if (m_pkChrTarget != nullptr)
									{
										if (m_pkChrTarget->IsPolymorphed())
										{
											m_pkChrTarget->SetPolymorph(0);
											m_pkChrTarget->RemoveAffect(AFFECT_POLYMORPH);
										}
									}
									else
									{
										if (IsPolymorphed())
										{
											SetPolymorph(0);
											RemoveAffect(AFFECT_POLYMORPH);
										}
									}
								}
								break;
#ifdef __CHANGE_LOOK__ // @correction192
							case 72325:
								{
									LPITEM item2;

									if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
										return false;

									if (item2->IsExchanging() || item2->IsEquipped()) // @correction038
										return false;

									if (!CleanChangeLook(item, item2))
										return false;
								}
								break;
#endif
#ifdef WJ_ELDER_ATTRIBUTE_SYSTEM // @correction193
							case 71051 : // ÁøÀç°¡
								{
									LPITEM item2;
									if (!IsValidItemPosition(DestCell) || !(item2 = GetInventoryItem(wDestCell)))
										return false;
									if (ITEM_COSTUME == item2->GetType()){ // @correction047
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
										return false;
									}
									if (item2->IsExchanging() || item2->IsEquipped()) // @correction038
										return false;
									if (item2->GetAttributeSetIndex() == -1){
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
										return false;
									}
									if (item2->AddRareAttribute() == true){
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼º°øÀûÀ¸·Î ¼Ó¼ºÀÌ Ãß°¡ µÇ¾ú½À´Ï´Ù"));
										int32_t iAddedIdx = item2->GetRareAttrCount() + 4;
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());
										LogManager::instance().ItemLog(GetPlayerID(), item2->GetAttributeType(iAddedIdx), item2->GetAttributeValue(iAddedIdx), item->GetID(), "ADD_RARE_ATTR", buf, GetDesc()->GetHostName(), item->GetOriginalVnum());
										// item->SetCount(item->GetCount() - 1);
									}
									else
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ¸·Î ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù"));
								}
								break;
							case 71052:
								{
									LPITEM item2;
									if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
										return false;
									if (ITEM_COSTUME == item2->GetType()){ // @correction047
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
										return false;
									}
									if (item2->IsExchanging() || item2->IsEquipped()) // @correction038
										return false;
									if (item2->GetAttributeSetIndex() == -1){
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
										return false;
									}
									if (item2->GetAttributeCount() == 5){
										item2->RemoveAttributeAt(4);
										item2->AddAttribute();
										// item->SetCount(item->GetCount()-1);
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_SUCCESS_MARBLE"));
									}
									else if (item2->GetAttributeCount() == 4){
										item2->AddAttribute();
										// item->SetCount(item->GetCount()-1);
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_SUCCESS_MARBLE"));										
									}
									else
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("NEW_ATTRIBUTE_MARBLE_ENOUGH_ATTR_COUNT"));
								}
								break;
#else
							case 71051 :
								{
									// @correction005
									LPITEM item2;

									if (!IsValidItemPosition(DestCell) || !(item2 = GetInventoryItem(wDestCell)))
										return false;

									if (ITEM_COSTUME == item2->GetType()) // @correction047
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
										return false;
									}

									if (item2->IsExchanging() || item2->IsEquipped()) // @correction038
										return false;

									if (item2->GetAttributeSetIndex() == -1)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
										return false;
									}

									if (item2->AddRareAttribute() == true)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼º°øÀûÀ¸·Î ¼Ó¼ºÀÌ Ãß°¡ µÇ¾ú½À´Ï´Ù"));

										int32_t iAddedIdx = item2->GetRareAttrCount() + 4;
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());

										LogManager::instance().ItemLog(
												GetPlayerID(),
												item2->GetAttributeType(iAddedIdx),
												item2->GetAttributeValue(iAddedIdx),
												item->GetID(),
												"ADD_RARE_ATTR",
												buf,
												GetDesc()->GetHostName(),
												item->GetOriginalVnum());

										item->SetCount(item->GetCount() - 1);
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ¸·Î ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù"));
									}
								}
								break;

							case 71052 :
								{
									// @correction005
									LPITEM item2;

									if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
										return false;

									if (ITEM_COSTUME == item2->GetType()) // @correction047
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
										return false;
									}

									if (item2->IsExchanging() || item2->IsEquipped()) // @correction038
										return false;

									if (item2->GetAttributeSetIndex() == -1)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
										return false;
									}

									if (item2->ChangeRareAttribute() == true)
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());
										LogManager::instance().ItemLog(this, item, "CHANGE_RARE_ATTR", buf);

										item->SetCount(item->GetCount() - 1);
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æ ½ÃÅ³ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù"));
									}
								}
								break;
#endif
#ifdef __SOCKET_UPGRADE__ // @correction132 BEGIN
								case 90060:
								case 90061:
								case 90062:
								{
									LPITEM item2;

									if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
										return false;

									if (item2->IsExchanging() == true)
										return false;

									if (item2->IsEquipped())
										return false;

									if ((item2->GetType() == ITEM_WEAPON && (item->GetVnum() == 90062 || item->GetVnum() == 90060)) || (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_BODY && (item->GetVnum() == 90062 || item->GetVnum() == 90061)))
									{
										if (item2->GetSocketCount() == ITEM_SOCKET_MAX_NUM)
										{
											ChatPacket(CHAT_TYPE_INFO, "Bu iteme daha fazla soket ekleyemezsin.");
											return false;
										}
										item2->AddSocket();
										item2->UpdatePacket();
										item2->Save();
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SUCCESS_OPEN_SOCKET"));
										item->SetCount(item->GetCount() - 1);
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, "Bu iteme soket eklenmez.");
										break;
									}
								}
								break;
#endif// @correction132 END
#ifdef __ADDON_ATTR__
							case 71984:
							{
								if (quest::CQuestManager::instance().GetEventFlag("BECORT_71984") == 1)
								{
									ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
									return false;
								}
								LPITEM item2;
								if (!IsValidItemPosition(DestCell) || !(item2 = GetInventoryItem(wDestCell)))
									return false;

								if (ITEM_COSTUME == item2->GetType()){ // @correction047
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
									return false;
								}

								if (item2->IsExchanging() || item2->IsEquipped()) // @correction038
									return false;

								const TItemTable * pProto = item2->GetProto();
								if (!pProto || pProto->sAddonType != -1)
									return false;

								uint32_t dwNum = number(55, 60);
								// item2->RemoveAttributeAt(0);
								// item2->RemoveAttributeAt(1);
								item2->SetForceAttribute(0, APPLY_NORMAL_HIT_DAMAGE_BONUS, dwNum);
								item2->SetForceAttribute(1, APPLY_SKILL_DAMAGE_BONUS, -20);
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼º°øÀûÀ¸·Î ¼Ó¼ºÀÌ Ãß°¡ µÇ¾ú½À´Ï´Ù"));
								item->SetCount(item->GetCount() - 1);
							}
							break;
							case 71985:
							{
								if (quest::CQuestManager::instance().GetEventFlag("BECORT_71985") == 1)
								{
									ChatPacket(CHAT_TYPE_INFO, "Bu Sistem Bakimdadir !");
									return false;
								}
								LPITEM item2;
								if (!IsValidItemPosition(DestCell) || !(item2 = GetInventoryItem(wDestCell)))
									return false;

								if (ITEM_COSTUME == item2->GetType()){ // @correction047
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
									return false;
								}

								if (item2->IsExchanging() || item2->IsEquipped()) // @correction038
									return false;

								const TItemTable * pProto = item2->GetProto();
								if (!pProto || pProto->sAddonType != -1)
									return false;

								uint32_t dwNum = number(25, 30);
								// item2->RemoveAttributeAt(0);
								// item2->RemoveAttributeAt(1);
								item2->SetForceAttribute(0, APPLY_NORMAL_HIT_DAMAGE_BONUS, -50);
								item2->SetForceAttribute(1, APPLY_SKILL_DAMAGE_BONUS, dwNum);
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼º°øÀûÀ¸·Î ¼Ó¼ºÀÌ Ãß°¡ µÇ¾ú½À´Ï´Ù"));
								item->SetCount(item->GetCount() - 1);
							}
							break;
#endif
							case ITEM_AUTO_HP_RECOVERY_S:
							case ITEM_AUTO_HP_RECOVERY_M:
							case ITEM_AUTO_HP_RECOVERY_L:
							case ITEM_AUTO_HP_RECOVERY_X:
							case ITEM_AUTO_SP_RECOVERY_S:
							case ITEM_AUTO_SP_RECOVERY_M:
							case ITEM_AUTO_SP_RECOVERY_L:
							case ITEM_AUTO_SP_RECOVERY_X:
							case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
							case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
							case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
							case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
							case FUCKING_BRAZIL_ITEM_AUTO_SP_RECOVERY_S:
							case FUCKING_BRAZIL_ITEM_AUTO_HP_RECOVERY_S:
								{
									if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}
// @correction230 BEGIN
									else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
										return false;
									}
// @correction230 END

									EAffectTypes type = AFFECT_NONE;
									bool isSpecialPotion = false;

									switch (item->GetVnum())
									{
										case ITEM_AUTO_HP_RECOVERY_X:
											isSpecialPotion = true;
											type = AFFECT_AUTO_HP_RECOVERY;
											break;

										case ITEM_AUTO_HP_RECOVERY_S:
										case ITEM_AUTO_HP_RECOVERY_M:
										case ITEM_AUTO_HP_RECOVERY_L:
										case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
										case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
										case FUCKING_BRAZIL_ITEM_AUTO_HP_RECOVERY_S:
											type = AFFECT_AUTO_HP_RECOVERY;
											break;

										case ITEM_AUTO_SP_RECOVERY_X:
											isSpecialPotion = true;
											type = AFFECT_AUTO_SP_RECOVERY;
											break;

										case ITEM_AUTO_SP_RECOVERY_S:
										case ITEM_AUTO_SP_RECOVERY_M:
										case ITEM_AUTO_SP_RECOVERY_L:
										case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
										case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
										case FUCKING_BRAZIL_ITEM_AUTO_SP_RECOVERY_S:
											type = AFFECT_AUTO_SP_RECOVERY;
											break;
									}

									if (AFFECT_NONE == type)
										break;

									if (item->GetCount() > 1)
									{
										int32_t pos = GetEmptyInventory(item);

										if (-1 == pos)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇ°¿¡ ºó °ø°£ÀÌ ¾ø½À´Ï´Ù."));
											break;
										}

										item->SetCount( item->GetCount() - 1 );

										LPITEM item2 = ITEM_MANAGER::instance().CreateItem( item->GetVnum(), 1 );
										item2->AddToCharacter(this, TItemPos(INVENTORY, pos), true);

										if (item->GetSocket(1) != 0)
										{
											item2->SetSocket(1, item->GetSocket(1));
										}

										item = item2;
									}

									CAffect* pAffect = FindAffect( type );

									if (nullptr == pAffect)
									{
										EPointTypes bonus = POINT_NONE;

										if (true == isSpecialPotion)
										{
											if (type == AFFECT_AUTO_HP_RECOVERY)
											{
												bonus = POINT_MAX_HP_PCT;
											}
											else if (type == AFFECT_AUTO_SP_RECOVERY)
											{
												bonus = POINT_MAX_SP_PCT;
											}
										}

										AddAffect( type, bonus, 4, item->GetID(), INFINITE_AFFECT_DURATION, 0, true, false);

										item->Lock(true);
										item->SetSocket(0, true);

										AutoRecoveryItemProcess( type );
									}
									else
									{
										if (item->GetID() == pAffect->dwFlag)
										{
											RemoveAffect( pAffect );

											item->Lock(false);
											item->SetSocket(0, false);
										}
										else
										{
											LPITEM old = FindItemByID( pAffect->dwFlag );

											if (nullptr != old)
											{
												old->Lock(false);
												old->SetSocket(0, false);
											}

											RemoveAffect( pAffect );

											EPointTypes bonus = POINT_NONE;

											if (true == isSpecialPotion)
											{
												if (type == AFFECT_AUTO_HP_RECOVERY)
												{
													bonus = POINT_MAX_HP_PCT;
												}
												else if (type == AFFECT_AUTO_SP_RECOVERY)
												{
													bonus = POINT_MAX_SP_PCT;
												}
											}

											AddAffect( type, bonus, 4, item->GetID(), INFINITE_AFFECT_DURATION, 0, true, false);

											item->Lock(true);
											item->SetSocket(0, true);

											AutoRecoveryItemProcess( type );
										}
									}
								}
								break;
						}
						break;

					case USE_CLEAR:
						{
							switch (item->GetVnum())
							{
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
								case 27124:
									RemoveBleeding();
									break;
#endif
								case 27874:
								default:
									RemoveBadAffect();
									break;
							}
							item->SetCount(item->GetCount() - 1);
						}
						break;

					case USE_INVISIBILITY:
						{
							if (item->GetVnum() == 70026)
							{
								quest::CQuestManager& q = quest::CQuestManager::instance();
								quest::PC* pPC = q.GetPC(GetPlayerID());

								if (pPC != nullptr)
								{
									int32_t last_use_time = pPC->GetFlag("mirror_of_disapper.last_use_time");

									if (get_global_time() - last_use_time < 10*60)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
										return false;
									}

									pPC->SetFlag("mirror_of_disapper.last_use_time", get_global_time());
								}
							}

							AddAffect(AFFECT_INVISIBILITY, POINT_NONE, 0, AFF_INVISIBILITY, 300, 0, true);
							item->SetCount(item->GetCount() - 1);
						}
						break;

					case USE_POTION_NODELAY:
						{
							if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
							{
								if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit") > 0)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
									return false;
								}

								switch (item->GetVnum())
								{
									case 70020 :
									case 71018 :
									case 71019 :
									case 71020 :
										if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count") < 10000)
										{
											if (m_nPotionLimit <= 0)
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ë Á¦ÇÑ·®À» ÃÊ°úÇÏ¿´½À´Ï´Ù."));
												return false;
											}
										}
										break;

									default :
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
										return false;
								}
							}
// @correction230 BEGIN
							else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
								return false;
							}
// @correction230 END

							bool used = false;

							if (item->GetValue(0) != 0)
							{
								if (GetHP() < GetMaxHP())
								{
									PointChange(POINT_HP, item->GetValue(0) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
									EffectPacket(SE_HPUP_RED);
									used = true;
								}
							}

							if (item->GetValue(1) != 0)
							{
								if (GetSP() < GetMaxSP())
								{
									PointChange(POINT_SP, item->GetValue(1) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
									EffectPacket(SE_SPUP_BLUE);
									used = true;
								}
							}

							if (item->GetValue(3) != 0)
							{
								if (GetHP() < GetMaxHP())
								{
									PointChange(POINT_HP, item->GetValue(3) * GetMaxHP() / 100);
									EffectPacket(SE_HPUP_RED);
									used = true;
								}
							}

							if (item->GetValue(4) != 0)
							{
								if (GetSP() < GetMaxSP())
								{
									PointChange(POINT_SP, item->GetValue(4) * GetMaxSP() / 100);
									EffectPacket(SE_SPUP_BLUE);
									used = true;
								}
							}

							if (used)
							{
								if (item->GetVnum() == 50085 || item->GetVnum() == 50086)
								{
									if (test_server)
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ùº´ ¶Ç´Â Á¾ÀÚ ¸¦ »ç¿ëÇÏ¿´½À´Ï´Ù"));
									SetUseSeedOrMoonBottleTime();
								}
								if (GetDungeon())
									GetDungeon()->UsePotion(this);

								if (GetWarMap())
									GetWarMap()->UsePotion(this, item);

								m_nPotionLimit--;

								item->SetCount(item->GetCount() - 1);
							}
						}
						break;

					case USE_POTION:
						if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
						{
							if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit") > 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
								return false;
							}

							switch (item->GetVnum())
							{
								case 27001 :
								case 27002 :
								case 27003 :
								case 27004 :
								case 27005 :
								case 27006 :
									if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count") < 10000)
									{
										if (m_nPotionLimit <= 0)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ë Á¦ÇÑ·®À» ÃÊ°úÇÏ¿´½À´Ï´Ù."));
											return false;
										}
									}
									break;

								default :
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·ÃÀå¿¡¼­ »ç¿ëÇÏ½Ç ¼ö ¾ø½À´Ï´Ù."));
									return false;
							}
						}
// @correction230 BEGIN
						else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
							return false;
						}
// @correction230 END

						if (item->GetValue(1) != 0)
						{
							if (GetPoint(POINT_SP_RECOVERY) + GetSP() >= GetMaxSP())
							{
								return false;
							}

							PointChange(POINT_SP_RECOVERY, item->GetValue(1) * MIN(200, (100 + GetPoint(POINT_POTION_BONUS))) / 100);
							StartAffectEvent();
							EffectPacket(SE_SPUP_BLUE);
						}

						if (item->GetValue(0) != 0)
						{
							if (GetPoint(POINT_HP_RECOVERY) + GetHP() >= GetMaxHP())
							{
								return false;
							}

							PointChange(POINT_HP_RECOVERY, item->GetValue(0) * MIN(200, (100 + GetPoint(POINT_POTION_BONUS))) / 100);
							StartAffectEvent();
							EffectPacket(SE_HPUP_RED);
						}

						if (GetDungeon())
							GetDungeon()->UsePotion(this);

						if (GetWarMap())
							GetWarMap()->UsePotion(this, item);

						item->SetCount(item->GetCount() - 1);
						m_nPotionLimit--;
						break;

					case USE_POTION_CONTINUE:
						{
							if (item->GetValue(0) != 0)
							{
								AddAffect(AFFECT_HP_RECOVER_CONTINUE, POINT_HP_RECOVER_CONTINUE, item->GetValue(0), 0, item->GetValue(2), 0, true);
							}
							else if (item->GetValue(1) != 0)
							{
								AddAffect(AFFECT_SP_RECOVER_CONTINUE, POINT_SP_RECOVER_CONTINUE, item->GetValue(1), 0, item->GetValue(2), 0, true);
							}
							else
								return false;
						}

						if (GetDungeon())
							GetDungeon()->UsePotion(this);

						if (GetWarMap())
							GetWarMap()->UsePotion(this, item);

						item->SetCount(item->GetCount() - 1);
						break;

					case USE_ABILITY_UP:
						{
							switch (item->GetValue(0))
							{
								case APPLY_MOV_SPEED:
// @correction133 BEGIN
									if (FindAffect(AFFECT_MOV_SPEED))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
										return false;
									}
// @correction133 END
									AddAffect(AFFECT_MOV_SPEED, POINT_MOV_SPEED, item->GetValue(2), AFF_MOV_SPEED_POTION, item->GetValue(1), 0, true);
#ifdef ENABLE_EFFECT_EXTRAPOT // @correction167
									EffectPacket(SE_DXUP_PURPLE);
#endif
									break;

								case APPLY_ATT_SPEED:
// @correction133 BEGIN
									if (FindAffect(AFFECT_ATT_SPEED))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
										return false;
									}
// @correction133 END
									AddAffect(AFFECT_ATT_SPEED, POINT_ATT_SPEED, item->GetValue(2), AFF_ATT_SPEED_POTION, item->GetValue(1), 0, true);
#ifdef ENABLE_EFFECT_EXTRAPOT // @correction167
									EffectPacket(SE_SPEEDUP_GREEN);
#endif
									break;

								case APPLY_STR:
									AddAffect(AFFECT_STR, POINT_ST, item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;

								case APPLY_DEX:
									AddAffect(AFFECT_DEX, POINT_DX, item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;

								case APPLY_CON:
									AddAffect(AFFECT_CON, POINT_HT, item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;

								case APPLY_INT:
									AddAffect(AFFECT_INT, POINT_IQ, item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;

								case APPLY_CAST_SPEED:
									AddAffect(AFFECT_CAST_SPEED, POINT_CASTING_SPEED, item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;

								case APPLY_ATT_GRADE_BONUS:
									AddAffect(AFFECT_ATT_GRADE, POINT_ATT_GRADE_BONUS,
											item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;

								case APPLY_DEF_GRADE_BONUS:
									AddAffect(AFFECT_DEF_GRADE, POINT_DEF_GRADE_BONUS,
											item->GetValue(2), 0, item->GetValue(1), 0, true);
									break;
							}
						}

						if (GetDungeon())
							GetDungeon()->UsePotion(this);

						if (GetWarMap())
							GetWarMap()->UsePotion(this, item);

						item->SetCount(item->GetCount() - 1);
						break;

					case USE_TALISMAN:
						{
							const int32_t TOWN_PORTAL	= 1;
							const int32_t MEMORY_PORTAL = 2;


							if (GetMapIndex() == 200 || GetMapIndex() == 28)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÇöÀç À§Ä¡¿¡¼­ »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
								return false;
							}

							if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
								return false;
							}
// @correction230 BEGIN
							else if (g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(item->GetVnum()))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ë·Ã Áß¿¡´Â ÀÌ¿ëÇÒ ¼ö ¾ø´Â ¹°Ç°ÀÔ´Ï´Ù."));
								return false;
							}
// @correction230 END

							if (m_pkWarpEvent)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌµ¿ÇÒ ÁØºñ°¡ µÇ¾îÀÖÀ½À¸·Î ±ÍÈ¯ºÎ¸¦ »ç¿ëÇÒ¼ö ¾ø½À´Ï´Ù"));
								return false;
							}

							int32_t consumeLife = CalculateConsume(this);

							if (consumeLife < 0)
								return false;

							if (item->GetValue(0) == TOWN_PORTAL)
							{
								if (item->GetSocket(0) == 0)
								{
									if (!GetDungeon())
										if (!GiveRecallItem(item))
											return false;

									PIXEL_POSITION posWarp;

									if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(GetMapIndex(), GetEmpire(), posWarp))
									{
										PointChange(POINT_HP, -consumeLife, false);

										WarpSet(posWarp.x, posWarp.y);
									}
									else
									{
										sys_err("CHARACTER::UseItem : cannot find spawn position (name %s, %d x %d)", GetName(), GetX(), GetY());
									}
								}
								else
								{
									if (test_server)
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿ø·¡ À§Ä¡·Î º¹±Í"));

									ProcessRecallItem(item);
								}
							}
							else if (item->GetValue(0) == MEMORY_PORTAL)
							{
								if (item->GetSocket(0) == 0)
								{
									if (GetDungeon())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´øÀü ¾È¿¡¼­´Â %s%s »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."), item->GetName(), "");
										return false;
									}

									if (!GiveRecallItem(item))
										return false;
								}
								else
								{
									PointChange(POINT_HP, -consumeLife, false);

									ProcessRecallItem(item);
								}
							}
						}
						break;

					case USE_TUNING:
					case USE_DETACHMENT:
						{
							LPITEM item2;

							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item2->IsExchanging() || item2->IsEquipped()) // @correction038
								return false;

#ifdef __ACCE_SYSTEM__ // @correction191
							if (item->GetValue(0) == ACCE_CLEAN_ATTR_VALUE0 && item->GetSubType() == USE_DETACHMENT)
							{
								if (!CleanAcceAttr(item, item2))
									return false;
								
								return true;
							}
#endif

#ifdef __RITUEL_STONE__
							if (item->GetValue(0) == RITUALS_SCROLL && item->GetSubType() == USE_TUNING)
							{
								if (item2->GetLevelLimit() <= item->GetValue(1))
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("you can only upgrade items above level %d"), item->GetValue(1));
									return false;
								}
								else
									RefineItem(item, item2);
							}
#endif
							if (item2->GetVnum() >= 28330 && item2->GetVnum() <= 28343)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("+3 ¿µ¼®Àº ÀÌ ¾ÆÀÌÅÛÀ¸·Î °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù"));
								return false;
							}
							if (item2->GetVnum() >= 28430 && item2->GetVnum() <= 28443)
							{
								if (item->GetVnum() == 71056)
								{
									RefineItem(item, item2);
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¿µ¼®Àº ÀÌ ¾ÆÀÌÅÛÀ¸·Î °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù"));
								}
							}
							else
							{
								RefineItem(item, item2);
							}
						}
						break;

					case USE_PUT_INTO_BELT_SOCKET:
					case USE_PUT_INTO_RING_SOCKET:
					case USE_PUT_INTO_ACCESSORY_SOCKET:
					case USE_ADD_ACCESSORY_SOCKET:
					case USE_CLEAN_SOCKET:
					case USE_CHANGE_ATTRIBUTE:
					case USE_CHANGE_ATTRIBUTE2 :
					case USE_ADD_ATTRIBUTE:
					case USE_ADD_ATTRIBUTE2:
						{
							LPITEM item2;
							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item2->IsEquipped())
							{
								BuffOnAttr_RemoveBuffsFromItem(item2);
							}

							if (ITEM_COSTUME == item2->GetType())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
								return false;
							}

							if (item2->IsExchanging() || item2->IsEquipped()) // @correction038
								return false;

							switch (item->GetSubType())
							{
								case USE_CLEAN_SOCKET:
									{
										int32_t i;
										for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
										{
											if (item2->GetSocket(i) == ITEM_BROKEN_METIN_VNUM)
												break;
										}

										if (i == ITEM_SOCKET_MAX_NUM)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ã»¼ÒÇÒ ¼®ÀÌ ¹ÚÇôÀÖÁö ¾Ê½À´Ï´Ù."));
											return false;
										}

										int32_t j = 0;

										for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
										{
											if (item2->GetSocket(i) != ITEM_BROKEN_METIN_VNUM && item2->GetSocket(i) != 0)
												item2->SetSocket(j++, item2->GetSocket(i));
										}

										for (; j < ITEM_SOCKET_MAX_NUM; ++j)
										{
											if (item2->GetSocket(j) > 0)
												item2->SetSocket(j, 1);
										}

										{
											char buf[21];
											snprintf(buf, sizeof(buf), "%u", item2->GetID());
											LogManager::instance().ItemLog(this, item, "CLEAN_SOCKET", buf);
										}

										item->SetCount(item->GetCount() - 1);

									}
									break;

								case USE_CHANGE_ATTRIBUTE :
								case USE_CHANGE_ATTRIBUTE2 : // @correction046
									if (item2->GetAttributeSetIndex() == -1)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
										return false;
									}

									if (item2->GetAttributeCount() == 0)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("º¯°æÇÒ ¼Ó¼ºÀÌ ¾ø½À´Ï´Ù."));
										return false;
									}

#ifdef __SOULBIND__ // @correction202
									if (item2->IsSealed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't change attr an sealbind item."));
										return false;
									}
#endif

#ifdef __PENDANT__
									if (!(item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT) && item->GetVnum() == 19983)
										return false;

									if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT && 19983 != item->GetVnum())
										return false;
#endif

									if ((GM_PLAYER == GetGMLevel()) && (false == test_server) && (g_dwItemBonusChangeTime > 0)) // @correction210
									{

										// uint32_t dwChangeItemAttrCycle = quest::CQuestManager::instance().GetEventFlag(msc_szChangeItemAttrCycleFlag);
										// if (dwChangeItemAttrCycle < msc_dwDefaultChangeItemAttrCycle)
											// dwChangeItemAttrCycle = msc_dwDefaultChangeItemAttrCycle;
										uint32_t dwChangeItemAttrCycle = g_dwItemBonusChangeTime; // @correction210

										quest::PC* pPC = quest::CQuestManager::instance().GetPC(GetPlayerID());

										if (pPC)
										{
											uint32_t dwNowSec = get_global_time();

											uint32_t dwLastChangeItemAttrSec = pPC->GetFlag(msc_szLastChangeItemAttrFlag);

											if (dwLastChangeItemAttrSec + dwChangeItemAttrCycle > dwNowSec)
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» ¹Ù²ÛÁö %dºÐ ÀÌ³»¿¡´Â ´Ù½Ã º¯°æÇÒ ¼ö ¾ø½À´Ï´Ù.(%d ºÐ ³²À½)"),
														dwChangeItemAttrCycle, dwChangeItemAttrCycle - (dwNowSec - dwLastChangeItemAttrSec));
												return false;
											}

											pPC->SetFlag(msc_szLastChangeItemAttrFlag, dwNowSec);
										}
									}

									if (item->GetSubType() == USE_CHANGE_ATTRIBUTE2)
									{
										int32_t aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] =
										{
											0, 0, 30, 40, 3
										};

										item2->ChangeAttribute(aiChangeProb);
									}
									else if (item->GetVnum() == 76014)
									{
										int32_t aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] =
										{
											0, 10, 50, 39, 1
										};

										item2->ChangeAttribute(aiChangeProb);
									}
									else
									{
										if (item->GetVnum() == 71151 || item->GetVnum() == 76023)
										{
											if ((item2->GetType() == ITEM_WEAPON)
												|| (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_BODY))
											{
												bool bCanUse = true;
												for (int32_t i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
												{
													if (item2->GetLimitType(i) == LIMIT_LEVEL && item2->GetLimitValue(i) > 40)
													{
														bCanUse = false;
														break;
													}
												}
												if (false == bCanUse)
												{
													ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Àû¿ë ·¹º§º¸´Ù ³ô¾Æ »ç¿ëÀÌ ºÒ°¡´ÉÇÕ´Ï´Ù."));
													break;
												}
											}
											else
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹«±â¿Í °©¿Ê¿¡¸¸ »ç¿ë °¡´ÉÇÕ´Ï´Ù."));
												break;
											}
										}
										item2->ChangeAttribute();
									}

									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÏ¿´½À´Ï´Ù."));
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());
										LogManager::instance().ItemLog(this, item, "CHANGE_ATTRIBUTE", buf);
									}

									item->SetCount(item->GetCount() - 1);
									break;

								case USE_ADD_ATTRIBUTE :
									if (item2->GetAttributeSetIndex() == -1)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
										return false;
									}

#ifdef __SOULBIND__ // @correction202
									if (item2->IsSealed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't change attr an sealbind item."));
										return false;
									}
#endif

									if (item2->GetAttributeCount() < 5)
									{
										if (item->GetVnum() == 71152 || item->GetVnum() == 76024)
										{
											if ((item2->GetType() == ITEM_WEAPON)
												|| (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_BODY))
											{
												bool bCanUse = true;
												for (int32_t i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
												{
													if (item2->GetLimitType(i) == LIMIT_LEVEL && item2->GetLimitValue(i) > 40)
													{
														bCanUse = false;
														break;
													}
												}
												if (false == bCanUse)
												{
													ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Àû¿ë ·¹º§º¸´Ù ³ô¾Æ »ç¿ëÀÌ ºÒ°¡´ÉÇÕ´Ï´Ù."));
													break;
												}
											}
											else
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹«±â¿Í °©¿Ê¿¡¸¸ »ç¿ë °¡´ÉÇÕ´Ï´Ù."));
												break;
											}
										}
										// char buf[21];
										// snprintf(buf, sizeof(buf), "%u", item2->GetID());

#ifdef __PENDANT__
										if (!(item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT) && item->GetVnum() == 19984)
											return false;

										if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT && 19984 != item->GetVnum())
											return false;
#endif

										uint32_t dwCount = CountSpecifyItem(item->GetVnum());
										uint32_t bCount = item2->GetAttributeCount();
										bCount = ITEM_MANAGER::MAX_NORM_ATTR_NUM - bCount < dwCount ? ITEM_MANAGER::MAX_NORM_ATTR_NUM - bCount : dwCount;
										for (size_t i = 0; i < bCount; ++i)
										{
											item2->AddAttribute();
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));

											/*int32_t iAddedIdx = item2->GetAttributeCount() - 1;
											LogManager::instance().ItemLog(
													GetPlayerID(),
													item2->GetAttributeType(iAddedIdx),
													item2->GetAttributeValue(iAddedIdx),
													item->GetID(),
													"ADD_ATTRIBUTE_SUCCESS",
													buf,
													GetDesc()->GetHostName(),
													item->GetOriginalVnum());*/

											item->SetCount(item->GetCount() - 1);
										}
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
									}
									break;

								case USE_ADD_ATTRIBUTE2 :
									if (item2->GetAttributeSetIndex() == -1)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼ºÀ» º¯°æÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
										return false;
									}
#ifdef __SOULBIND__ // @correction202
									if (item2->IsSealed())
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't change attr an sealbind item."));
										return false;
									}
#endif

									if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_PENDANT && 19983 != item->GetVnum())
										return false;

									if (item2->GetAttributeCount() == 4)
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());

										if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()])
										{
											item2->AddAttribute();
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));

											int32_t iAddedIdx = item2->GetAttributeCount() - 1;
											LogManager::instance().ItemLog(
													GetPlayerID(),
													item2->GetAttributeType(iAddedIdx),
													item2->GetAttributeValue(iAddedIdx),
													item->GetID(),
													"ADD_ATTRIBUTE2_SUCCESS",
													buf,
													GetDesc()->GetHostName(),
													item->GetOriginalVnum());
										}
										else
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼Ó¼º Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
											LogManager::instance().ItemLog(this, item, "ADD_ATTRIBUTE2_FAIL", buf);
										}

										item->SetCount(item->GetCount() - 1);
									}
									else if (item2->GetAttributeCount() == 5)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´õ ÀÌ»ó ÀÌ ¾ÆÀÌÅÛÀ» ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡ÇÒ ¼ö ¾ø½À´Ï´Ù."));
									}
									else if (item2->GetAttributeCount() < 4)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú Àç°¡ºñ¼­¸¦ ÀÌ¿ëÇÏ¿© ¼Ó¼ºÀ» Ãß°¡½ÃÄÑ ÁÖ¼¼¿ä."));
									}
									else
									{
										sys_err("ADD_ATTRIBUTE2 : Item has wrong AttributeCount(%d)", item2->GetAttributeCount());
									}
									break;

								case USE_ADD_ACCESSORY_SOCKET:
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());

										if (item2->IsAccessoryForSocket())
										{
											if (item2->GetAccessorySocketMaxGrade() < ITEM_ACCESSORY_SOCKET_MAX_NUM)
											{
// @correction134 BEGIN
												uint32_t dwCount = item->GetCount();
												uint32_t bCount = item2->GetAccessorySocketMaxGrade();
												bCount = ITEM_ACCESSORY_SOCKET_MAX_NUM - bCount < dwCount ? ITEM_ACCESSORY_SOCKET_MAX_NUM - bCount : dwCount;
												for (size_t i = 0; i < bCount; ++i)
												{
#ifdef ENABLE_ADDSTONE_FAILURE // @correction166
													if (number(1, 100) <= 50)
#else
													if (1)
#endif
													{
														item2->SetAccessorySocketMaxGrade(item2->GetAccessorySocketMaxGrade() + 1);
														ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÄÏÀÌ ¼º°øÀûÀ¸·Î Ãß°¡µÇ¾ú½À´Ï´Ù."));
														LogManager::instance().ItemLog(this, item, "ADD_SOCKET_SUCCESS", buf);
													}
													else
													{
														ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÄÏ Ãß°¡¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
														LogManager::instance().ItemLog(this, item, "ADD_SOCKET_FAIL", buf);
													}
														
													item->SetCount(item->GetCount() - 1);
// @correction134 END
												}
											}
											else
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾×¼¼¼­¸®¿¡´Â ´õÀÌ»ó ¼ÒÄÏÀ» Ãß°¡ÇÒ °ø°£ÀÌ ¾ø½À´Ï´Ù."));
											}
										}
										else
										{
											ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀ¸·Î ¼ÒÄÏÀ» Ãß°¡ÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
										}
									}
									break;

								case USE_PUT_INTO_BELT_SOCKET:
								case USE_PUT_INTO_ACCESSORY_SOCKET:
									if (item2->IsAccessoryForSocket() && item->CanPutInto(item2))
									{
										char buf[21];
										snprintf(buf, sizeof(buf), "%u", item2->GetID());

										if (item2->GetAccessorySocketGrade() < item2->GetAccessorySocketMaxGrade())
										{
// @correction134 BEGIN
											uint32_t dwCount = item->GetCount();
											uint32_t bCount = item2->GetAccessorySocketGrade();
											bCount = item2->GetAccessorySocketMaxGrade() - bCount < dwCount ? item2->GetAccessorySocketMaxGrade() - bCount : dwCount;
											for (size_t i = 0; i < bCount; ++i)
											{
#ifdef UNLIMITED_ACCESSORY_SOCKET_TIME
												if (item2->GetAccessorySocketGrade() && item->GetValue(0) != 4248 && item2->GetSocket(3) == 4248)
													return false;

												if (item2->GetAccessorySocketGrade() && item->GetValue(0) == 4248 && item2->GetSocket(3) != 4248)
													return false;
#endif
#ifdef ENABLE_ADDSTONE_FAILURE // @correction166
												if (number(1, 100) <= aiAccessorySocketPutPct[item2->GetAccessorySocketGrade()])
#else
												if (1)
#endif
												{
#ifdef UNLIMITED_ACCESSORY_SOCKET_TIME
													// if (!quest::CQuestManager::instance().GetEventFlag("unlimited_accessory_socket"))
													{
														if (item->GetValue(0) == 4248 && item2->GetAccessorySocketMaxGrade() && !item2->GetAccessorySocketGrade())
															item2->SetSocket(3, 4248);
													}
#endif
													item2->SetAccessorySocketGrade(item2->GetAccessorySocketGrade() + 1);
													ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀåÂø¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
													LogManager::instance().ItemLog(this, item, "PUT_SOCKET_SUCCESS", buf);
												}
												else
												{
													ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀåÂø¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
													LogManager::instance().ItemLog(this, item, "PUT_SOCKET_FAIL", buf);
												}

												item->SetCount(item->GetCount() - 1);
// @correction134 END
											}
										}
										else
										{
											if (item2->GetAccessorySocketMaxGrade() == 0)
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÕÀú ´ÙÀÌ¾Æ¸óµå·Î ¾Ç¼¼¼­¸®¿¡ ¼ÒÄÏÀ» Ãß°¡ÇØ¾ßÇÕ´Ï´Ù."));
											else if (item2->GetAccessorySocketMaxGrade() < ITEM_ACCESSORY_SOCKET_MAX_NUM)
											{
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾×¼¼¼­¸®¿¡´Â ´õÀÌ»ó ÀåÂøÇÒ ¼ÒÄÏÀÌ ¾ø½À´Ï´Ù."));
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("´ÙÀÌ¾Æ¸óµå·Î ¼ÒÄÏÀ» Ãß°¡ÇØ¾ßÇÕ´Ï´Ù."));
											}
											else
												ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾×¼¼¼­¸®¿¡´Â ´õÀÌ»ó º¸¼®À» ÀåÂøÇÒ ¼ö ¾ø½À´Ï´Ù."));
										}
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀ» ÀåÂøÇÒ ¼ö ¾ø½À´Ï´Ù."));
									}
									break;
							}
							if (item2->IsEquipped())
							{
								BuffOnAttr_AddBuffsFromItem(item2);
							}
						}
						break;

#ifdef __MOVE_COSTUME_ATTR__
					case USE_ADD_COSTUME_ATTRIBUTE:
					case USE_RESET_COSTUME_ATTR:
					{
						LPITEM item2;
						if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
							return false;

						if (item2->IsEquipped())
							BuffOnAttr_RemoveBuffsFromItem(item2);

						if (item2->IsExchanging() || item2->IsEquipped())
							return false;

#ifdef __SOULBIND__
						if (item2->IsSealed())
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't change attr an sealbind item."));
							return false;
						}
#endif

						/*if (item2->IsBasicItem())
							return false;*/

						if (item2->GetType() != ITEM_COSTUME)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can use this just on costumes."));
							return false;
						}

						if (item2->GetAttributeCount() == 0)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("This costume doesn't have any bonus."));
							return false;
						}

						switch (item->GetSubType())
						{
							case USE_ADD_COSTUME_ATTRIBUTE:
							{
								int32_t aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] = { 100, 100, 100, 100, 100 };
								item2->ChangeAttribute(aiChangeProb);
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You enchant succesfully the costume."));
								{
									char buf[21];
									snprintf(buf, sizeof(buf), "%u", item2->GetID());
									LogManager::instance().ItemLog(this, item, "USE_ADD_COSTUME_ATTRIBUTE", buf);
								}

								item->SetCount(item->GetCount() - 1);
							}
							break;
							case USE_RESET_COSTUME_ATTR:
							{
								item2->ClearAttribute();
								int32_t iCostumePercent = number(1, 3);
								for (int32_t i = 0; i < iCostumePercent; ++i)
								{
									item2->AddAttribute();
								}

								int32_t aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] = { 100, 100, 100, 100, 100 };
								item2->ChangeAttribute(aiChangeProb);
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You transform succesfully the costume."));
								{
									char buf[21];
									snprintf(buf, sizeof(buf), "%u", item2->GetID());
									LogManager::instance().ItemLog(this, item, "USE_RESET_COSTUME_ATTR", buf);
								}

								item->SetCount(item->GetCount() - 1);
							}
							break;
						}
					}
					break;
#endif

					case USE_BAIT:
						{

							if (m_pkFishingEvent)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³¬½Ã Áß¿¡ ¹Ì³¢¸¦ °¥¾Æ³¢¿ï ¼ö ¾ø½À´Ï´Ù."));
								return false;
							}

							LPITEM weapon = GetWear(WEAR_WEAPON);

							if (!weapon || weapon->GetType() != ITEM_ROD)
								return false;

							if (weapon->GetSocket(2))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì ²ÈÇôÀÖ´ø ¹Ì³¢¸¦ »©°í %s¸¦ ³¢¿ó´Ï´Ù."), item->GetName());
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³¬½Ã´ë¿¡ %s¸¦ ¹Ì³¢·Î ³¢¿ó´Ï´Ù."), item->GetName());
							}

							weapon->SetSocket(2, item->GetValue(0));
							item->SetCount(item->GetCount() - 1);
						}
						break;

					case USE_MOVE:
					case USE_TREASURE_BOX:
					case USE_MONEYBAG:
						break;

					case USE_AFFECT :
						{
							if (FindAffect(item->GetValue(0), aApplyInfo[item->GetValue(1)].bPointType))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
							}
							else
							{
								if (item->IsPCBangItem() == true)
								{
									if (CPCBangManager::instance().IsPCBangIP(GetDesc()->GetHostName()) == false)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº PC¹æ¿¡¼­¸¸ »ç¿ëÇÒ ¼ö ÀÖ½À´Ï´Ù."));
										return false;
									}
								}

								AddAffect(item->GetValue(0), aApplyInfo[item->GetValue(1)].bPointType, item->GetValue(2), 0, item->GetValue(3), 0, false);
								item->SetCount(item->GetCount() - 1);
							}
						}
						break;

					case USE_CREATE_STONE:
						AutoGiveItem(number(28000, 28013));
						item->SetCount(item->GetCount() - 1);
						break;

					case USE_RECIPE :
						{
							LPITEM pSource1 = FindSpecifyItem(item->GetValue(1));
							uint32_t dwSourceCount1 = item->GetValue(2);

							LPITEM pSource2 = FindSpecifyItem(item->GetValue(3));
							uint32_t dwSourceCount2 = item->GetValue(4);

							if (dwSourceCount1 != 0)
							{
								if (pSource1 == nullptr)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹°¾à Á¶ÇÕÀ» À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
									return false;
								}
							}

							if (dwSourceCount2 != 0)
							{
								if (pSource2 == nullptr)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹°¾à Á¶ÇÕÀ» À§ÇÑ Àç·á°¡ ºÎÁ·ÇÕ´Ï´Ù."));
									return false;
								}
							}

							if (pSource1 != nullptr)
							{
								if (pSource1->GetCount() < dwSourceCount1)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Àç·á(%s)°¡ ºÎÁ·ÇÕ´Ï´Ù."), pSource1->GetName());
									return false;
								}

								pSource1->SetCount(pSource1->GetCount() - dwSourceCount1);
							}

							if (pSource2 != nullptr)
							{
								if (pSource2->GetCount() < dwSourceCount2)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Àç·á(%s)°¡ ºÎÁ·ÇÕ´Ï´Ù."), pSource2->GetName());
									return false;
								}

								pSource2->SetCount(pSource2->GetCount() - dwSourceCount2);
							}

							LPITEM pBottle = FindSpecifyItem(50901);

							if (!pBottle || pBottle->GetCount() < 1)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ºó º´ÀÌ ¸ðÀÚ¸¨´Ï´Ù."));
								return false;
							}

							pBottle->SetCount(pBottle->GetCount() - 1);

							if (number(1, 100) > item->GetValue(5))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹°¾à Á¦Á¶¿¡ ½ÇÆÐÇß½À´Ï´Ù."));
								return false;
							}

							AutoGiveItem(item->GetValue(0));
						}
						break;
				}
			}
			break;

		case ITEM_METIN:
			{
				LPITEM item2;

				if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
					return false;

				if (item2->IsExchanging() || item2->IsEquipped()) // @correction038
					return false;

				if (item2->GetType() == ITEM_PICK) return false;
				if (item2->GetType() == ITEM_ROD) return false;

				int32_t i;

				for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				{
					uint32_t dwVnum;

					if ((dwVnum = item2->GetSocket(i)) <= 2)
						continue;

					TItemTable * p = ITEM_MANAGER::instance().GetTable(dwVnum);

					if (!p)
						continue;

					if (item->GetValue(5) == p->alValues[5])
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°°Àº Á¾·ùÀÇ ¸ÞÆ¾¼®Àº ¿©·¯°³ ºÎÂøÇÒ ¼ö ¾ø½À´Ï´Ù."));
						return false;
					}
				}

				if (item2->GetType() == ITEM_ARMOR)
				{
					if (!IS_SET(item->GetWearFlag(), WEARABLE_BODY) || !IS_SET(item2->GetWearFlag(), WEARABLE_BODY))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¸ÞÆ¾¼®Àº Àåºñ¿¡ ºÎÂøÇÒ ¼ö ¾ø½À´Ï´Ù."));
						return false;
					}
				}
				else if (item2->GetType() == ITEM_WEAPON)
				{
					if (!IS_SET(item->GetWearFlag(), WEARABLE_WEAPON))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¸ÞÆ¾¼®Àº ¹«±â¿¡ ºÎÂøÇÒ ¼ö ¾ø½À´Ï´Ù."));
						return false;
					}
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ºÎÂøÇÒ ¼ö ÀÖ´Â ½½·ÔÀÌ ¾ø½À´Ï´Ù."));
					return false;
				}

				for (i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				{
					if (item2->GetSocket(i) >= 1 && item2->GetSocket(i) <= 2 && item2->GetSocket(i) >= item->GetValue(2))
					{
#ifdef ENABLE_ADDSTONE_FAILURE // @correction166
						if (number(1, 100) <= 30)
#else
						if (1)
#endif
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÞÆ¾¼® ºÎÂø¿¡ ¼º°øÇÏ¿´½À´Ï´Ù."));
							item2->SetSocket(i, item->GetVnum());
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸ÞÆ¾¼® ºÎÂø¿¡ ½ÇÆÐÇÏ¿´½À´Ï´Ù."));
							item2->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
						}

						LogManager::instance().ItemLog(this, item2, "SOCKET", item->GetName());
						item->SetCount(item->GetCount() - 1); // @correction088
						break;
					}
				}

				if (i == ITEM_SOCKET_MAX_NUM)
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ºÎÂøÇÒ ¼ö ÀÖ´Â ½½·ÔÀÌ ¾ø½À´Ï´Ù."));
			}
			break;

		case ITEM_AUTOUSE:
		case ITEM_MATERIAL:
		case ITEM_SPECIAL:
		case ITEM_TOOL:
		case ITEM_LOTTERY:
			break;

		case ITEM_TOTEM:
			{
				if (!item->IsEquipped())
					EquipItem(item);
			}
			break;

		case ITEM_BLEND:
			sys_log(0,"ITEM_BLEND!!");
			if (Blend_Item_find(item->GetVnum()))
			{
				int32_t		affect_type		= AFFECT_BLEND;
				int32_t		apply_type		= aApplyInfo[item->GetSocket(0)].bPointType;
				int32_t		apply_value		= item->GetSocket(1);
				int32_t		apply_duration	= item->GetSocket(2);

				if (FindAffect(affect_type, apply_type))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
				}
				else
				{
					if (FindAffect(AFFECT_EXP_BONUS_EURO_FREE, POINT_RESIST_MAGIC))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì È¿°ú°¡ °É·Á ÀÖ½À´Ï´Ù."));
					}
					else
					{
						AddAffect(affect_type, apply_type, apply_value, 0, apply_duration, 0, false);
						item->SetCount(item->GetCount() - 1);
					}
				}
			}
			break;
		case ITEM_EXTRACT:
			{
				LPITEM pDestItem = GetItem(DestCell);
				if (nullptr == pDestItem)
				{
					return false;
				}
				switch (item->GetSubType())
				{
				case EXTRACT_DRAGON_SOUL:
					if (pDestItem->IsDragonSoul())
					{
						return DSManager::instance().PullOut(this, NPOS, pDestItem, item);
					}
					return false;
				case EXTRACT_DRAGON_HEART:
#ifdef __DS_CHANGE_ATTRIBUTE__
					if (item->GetVnum() == 80039)
					{
						if (pDestItem->IsDragonSoul())
						{
							if (DSManager::instance().IsActiveDragonSoul(pDestItem) == true)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SIMYA_GIYILIYOR"));
								return false;
							}

							pDestItem->SetForceAttribute(0, 0, 0);
							pDestItem->SetForceAttribute(1, 0, 0);
							pDestItem->SetForceAttribute(2, 0, 0);
							pDestItem->SetForceAttribute(3, 0, 0);
							pDestItem->SetForceAttribute(4, 0, 0);
							pDestItem->SetForceAttribute(5, 0, 0);
							pDestItem->SetForceAttribute(6, 0, 0);

							bool ret = DSManager::instance().PutAttributes(pDestItem);
							if (ret == true)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SIMYA_DEGISTIRME_BASARILI!"));
								item->SetCount(item->GetCount() - 1);
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("SIMYA_DEGISTIRME_BASARISIZ!"));
							}
						}
					}
					else
#endif
					{
						if (pDestItem->IsDragonSoul())
						{
							return DSManager::instance().ExtractDragonHeart(this, pDestItem, item);
						}
					}
					return false;
				default:
					return false;
				}
			}
			break;

#ifdef __SPECIAL_GACHA__
		case ITEM_GACHA:
		{
			uint32_t dwBoxVnum = item->GetVnum();
			std::vector <uint32_t> dwVnums;
			std::vector <uint32_t> dwCounts;
			std::vector <LPITEM> item_gets(0);
			int32_t count = 0;

			if (GiveItemFromSpecialItemGroup(dwBoxVnum, dwVnums, dwCounts, item_gets, count, wCount))
			{
				for (int32_t i = 0; i < count; i++)
				{
					switch (dwVnums[i])
					{
					case CSpecialItemGroup::GOLD:
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µ· %d ³ÉÀ» È¹µæÇß½À´Ï´Ù."), dwCounts[i]);
						break;
					case CSpecialItemGroup::EXP:
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ºÎÅÍ ½ÅºñÇÑ ºûÀÌ ³ª¿É´Ï´Ù."));
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%dÀÇ °æÇèÄ¡¸¦ È¹µæÇß½À´Ï´Ù."), dwCounts[i]);
						break;
					case CSpecialItemGroup::MOB:
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ¸ó½ºÅÍ°¡ ³ªÅ¸³µ½À´Ï´Ù!"));
						break;
					case CSpecialItemGroup::SLOW:
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ³ª¿Â »¡°£ ¿¬±â¸¦ µéÀÌ¸¶½ÃÀÚ ¿òÁ÷ÀÌ´Â ¼Óµµ°¡ ´À·ÁÁ³½À´Ï´Ù!"));
						break;
					case CSpecialItemGroup::DRAIN_HP:
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ°¡ °©ÀÚ±â Æø¹ßÇÏ¿´½À´Ï´Ù! »ý¸í·ÂÀÌ °¨¼ÒÇß½À´Ï´Ù."));
						break;
					case CSpecialItemGroup::POISON:
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ³ª¿Â ³ì»ö ¿¬±â¸¦ µéÀÌ¸¶½ÃÀÚ µ¶ÀÌ ¿Â¸öÀ¸·Î ÆÛÁý´Ï´Ù!"));
						break;
#ifdef ENABLE_WOLFMAN_CHARACTER
					case CSpecialItemGroup::BLEEDING:
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ³ª¿Â ³ì»ö ¿¬±â¸¦ µéÀÌ¸¶½ÃÀÚ µ¶ÀÌ ¿Â¸öÀ¸·Î ÆÛÁý´Ï´Ù!"));
						break;
#endif
					case CSpecialItemGroup::MOB_GROUP:
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ ¸ó½ºÅÍ°¡ ³ªÅ¸³µ½À´Ï´Ù!"));
						break;
					default:
						if (item_gets[i])
						{
							if (dwCounts[i] > 1)
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ %s °¡ %d °³ ³ª¿Ô½À´Ï´Ù."), item_gets[i]->GetName(), dwCounts[i]);
							else
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»óÀÚ¿¡¼­ %s °¡ ³ª¿Ô½À´Ï´Ù."), item_gets[i]->GetName());
						}
					}
				}

				if (item->GetSocket(0) > 1)
					item->SetSocket(0, item->GetSocket(0) - 1);
				else
					ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (ITEM_GACHA)");
			}
			else
			{
				ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("¾Æ¹«°Íµµ ¾òÀ» ¼ö ¾ø¾ú½À´Ï´Ù."));
				return false;
			}
		}
		break;
#endif

		case ITEM_NONE:
			sys_err("Item type NONE %s", item->GetName());
			break;

		default:
			sys_log(0, "UseItemEx: Unknown type %s %d", item->GetName(), item->GetType());
			return false;
	}

	return true;
}

int32_t g_nPortalLimitTime = 10;

bool CHARACTER::UseItem(TItemPos Cell, TItemPos DestCell
#ifdef __FAST_USE_GIFTBOX__
, uint16_t wCount
#endif
)
{
	uint16_t wCell = Cell.cell;
	uint8_t window_type = Cell.window_type;
	LPITEM item;

	if (!CanHandleItem())
		return false;

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
			return false;

	sys_log(0, "%s: USE_ITEM %s (inven %d, cell: %d)", GetName(), item->GetName(), window_type, wCell);

	if (item->IsExchanging())
		return false;

#ifdef __SWITCHBOT__
	if (Cell.IsSwitchbotPosition())
	{
		CSwitchbot* pkSwitchbot = CSwitchbotManager::Instance().FindSwitchbot(GetPlayerID());
		if (pkSwitchbot && pkSwitchbot->IsActive(Cell.cell))
		{
			return false;
		}

		int32_t iEmptyCell = GetEmptyInventory(item);

		if (iEmptyCell == -1)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot remove item from switchbot. Inventory is full."));
			return false;
		}

		MoveItem(Cell, TItemPos(INVENTORY, iEmptyCell), item->GetCount());
		return true;
	}
#endif

	if (!item->CanUsedBy(this))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±ºÁ÷ÀÌ ¸ÂÁö¾Ê¾Æ ÀÌ ¾ÆÀÌÅÛÀ» »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (IsStun())
		return false;

	if (false == FN_check_item_sex(this, item))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ºº°ÀÌ ¸ÂÁö¾Ê¾Æ ÀÌ ¾ÆÀÌÅÛÀ» »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (IS_SUMMON_ITEM(item->GetVnum()))
	{
		if (false == IS_SUMMONABLE_ZONE(GetMapIndex()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ç¿ëÇÒ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}


		if (CThreeWayWar::instance().IsThreeWayWarMapIndex(GetMapIndex()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("»ï°Å¸® ÀüÅõ Âü°¡Áß¿¡´Â ±ÍÈ¯ºÎ,±ÍÈ¯±â¾ïºÎ¸¦ »ç¿ëÇÒ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
		int32_t iPulse = thecore_pulse();

		if (iPulse - GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ã¢°í¸¦ ¿¬ÈÄ %dÃÊ ÀÌ³»¿¡´Â ±ÍÈ¯ºÎ,±ÍÈ¯±â¾ïºÎ¸¦ »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."), g_nPortalLimitTime);

			if (test_server)
				ChatPacket(CHAT_TYPE_INFO, "[TestOnly]Pulse %d LoadTime %d PASS %d", iPulse, GetSafeboxLoadTime(), PASSES_PER_SEC(g_nPortalLimitTime));
			return false;
		}

		if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡Ã¢,Ã¢°í µîÀ» ¿¬ »óÅÂ¿¡¼­´Â ±ÍÈ¯ºÎ,±ÍÈ¯±â¾ïºÎ ¸¦ »ç¿ëÇÒ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}

		{
			if (iPulse - GetRefineTime() < PASSES_PER_SEC(g_nPortalLimitTime))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ °³·®ÈÄ %dÃÊ ÀÌ³»¿¡´Â ±ÍÈ¯ºÎ,±ÍÈ¯±â¾ïºÎ¸¦ »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."), g_nPortalLimitTime);
				return false;
			}
		}


		{
			if (iPulse - GetMyShopTime() < PASSES_PER_SEC(g_nPortalLimitTime))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°³ÀÎ»óÁ¡ »ç¿ëÈÄ %dÃÊ ÀÌ³»¿¡´Â ±ÍÈ¯ºÎ,±ÍÈ¯±â¾ïºÎ¸¦ »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."), g_nPortalLimitTime);
				return false;
			}

		}


		if (item->GetVnum() != 70302)
		{
			PIXEL_POSITION posWarp;

			int32_t x = 0;
			int32_t y = 0;

			double nDist = 0;
			const double nDistant = 5000.0;
			if (item->GetVnum() == 22010)
			{
				x = item->GetSocket(0) - GetX();
				y = item->GetSocket(1) - GetY();
			}
			else if (item->GetVnum() == 22000)
			{
				SECTREE_MANAGER::instance().GetRecallPositionByEmpire(GetMapIndex(), GetEmpire(), posWarp);

				if (item->GetSocket(0) == 0)
				{
					x = posWarp.x - GetX();
					y = posWarp.y - GetY();
				}
				else
				{
					x = item->GetSocket(0) - GetX();
					y = item->GetSocket(1) - GetY();
				}
			}

			nDist = sqrt(pow((float)x,2) + pow((float)y,2));

			if (nDistant > nDist)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌµ¿ µÇ¾îÁú À§Ä¡¿Í ³Ê¹« °¡±î¿ö ±ÍÈ¯ºÎ¸¦ »ç¿ëÇÒ¼ö ¾ø½À´Ï´Ù."));
				if (test_server)
					ChatPacket(CHAT_TYPE_INFO, "PossibleDistant %f nNowDist %f", nDistant,nDist);
				return false;
			}
		}

		if (iPulse - GetExchangeTime()  < PASSES_PER_SEC(g_nPortalLimitTime))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡ ÈÄ %dÃÊ ÀÌ³»¿¡´Â ±ÍÈ¯ºÎ,±ÍÈ¯±â¾ïºÎµîÀ» »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."), g_nPortalLimitTime);
			return false;
		}

	}

	if ((item->GetVnum() == 50200) || (item->GetVnum() == 71049))
	{
		if (GetExchange() || GetMyShop() || GetShopOwner() || IsOpenSafebox() || IsCubeOpen())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°Å·¡Ã¢,Ã¢°í µîÀ» ¿¬ »óÅÂ¿¡¼­´Â º¸µû¸®,ºñ´Üº¸µû¸®¸¦ »ç¿ëÇÒ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}

	}

	// @correction068 BEGIN
	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot use this item if you're using quests"));
		return false;
	}
	// @correction068 END

	if (IS_SET(item->GetFlag(), ITEM_FLAG_LOG))
	{
		uint32_t vid = item->GetVID();
		uint32_t oldCount = item->GetCount();
		uint32_t vnum = item->GetVnum();

		char hint[ITEM_NAME_MAX_LEN + 32 + 1];
		int32_t len = snprintf(hint, sizeof(hint) - 32, "%s", item->GetName());

		if (len < 0 || len >= (int32_t) sizeof(hint) - 32)
			len = (sizeof(hint) - 32) - 1;

		bool ret = UseItemEx(item, DestCell
#ifdef __FAST_USE_GIFTBOX__
, wCount
#endif
);

		if (nullptr == ITEM_MANAGER::instance().FindByVID(vid))
		{
			LogManager::instance().ItemLog(this, vid, vnum, "REMOVE", hint);
		}
		else if (oldCount != item->GetCount())
		{
			snprintf(hint + len, sizeof(hint) - len, " %u", oldCount - 1);
			LogManager::instance().ItemLog(this, vid, vnum, "USE_ITEM", hint);
		}
		return (ret);
	}
	else
		return UseItemEx(item, DestCell
#ifdef __FAST_USE_GIFTBOX__
, wCount
#endif
);
}

bool CHARACTER::DropItem(TItemPos Cell, uint16_t wCount)
{
	LPITEM item = nullptr;

	if (!CanHandleItem())
	{
		if (nullptr != DragonSoul_RefineWindow_GetOpener())
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°­È­Ã¢À» ¿¬ »óÅÂ¿¡¼­´Â ¾ÆÀÌÅÛÀ» ¿Å±æ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

// @correction225 BEGIN
	if (0 != g_ItemDropTimeLimitValue)
	{
		if (get_dword_time() < m_dwLastItemDropTime+g_ItemDropTimeLimitValue)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ °ñµå¸¦ ¹ö¸± ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
	}

	m_dwLastItemDropTime = get_dword_time();
// @correction225 END

	if (IsDead())
		return false;

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (true == item->isLocked())
		return false;

#ifdef __SOULBIND__ // @correction202 
	if (item->IsSealed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't drop sealbind item."));
		return false;
	}
#endif

	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		return false;

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP | ITEM_ANTIFLAG_GIVE))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹ö¸± ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
		return false;
	}

	if (wCount == 0 || wCount > item->GetCount())
		wCount = item->GetCount();

	SyncQuickslot(QUICKSLOT_TYPE_ITEM, Cell.cell, 255);

	LPITEM pkItemToDrop;

	if (wCount == item->GetCount())
	{
		item->RemoveFromCharacter();
		pkItemToDrop = item;
	}
	else
	{
		if (wCount == 0)
		{
			if (test_server)
				sys_log(0, "[DROP_ITEM] drop item count == 0");
			return false;
		}

		item->SetCount(item->GetCount() - wCount);
		ITEM_MANAGER::instance().FlushDelayedSave(item);

		pkItemToDrop = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), wCount);

		FN_copy_item_socket(pkItemToDrop, item);

		char szBuf[64 + 1];
		snprintf(szBuf, sizeof(szBuf), "%u %u", pkItemToDrop->GetID(), pkItemToDrop->GetCount());
		LogManager::instance().ItemLog(this, item, "ITEM_SPLIT", szBuf);
	}

	PIXEL_POSITION pxPos = GetXYZ();

	if (pkItemToDrop->AddToGround(GetMapIndex(), pxPos))
	{
		// @correction005
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¶³¾îÁø ¾ÆÀÌÅÛÀº 3ºÐ ÈÄ »ç¶óÁý´Ï´Ù."));
		pkItemToDrop->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPITEM]); // @correction224

		ITEM_MANAGER::instance().FlushDelayedSave(pkItemToDrop);

		char szHint[64 + 1];
		snprintf(szHint, sizeof(szHint), "%s %u %u", pkItemToDrop->GetName(), pkItemToDrop->GetCount(), pkItemToDrop->GetOriginalVnum());
		LogManager::instance().ItemLog(this, pkItemToDrop, "DROP", szHint);
	}

	return true;
}

// @correction126 BEGIN
bool CHARACTER::DestroyItem(TItemPos Cell, uint8_t type)
{
	LPITEM item = nullptr;

	if (!CanHandleItem())
		return false;

	if (IsDead() || IsStun())
		return false;

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	if (true == item->isLocked() || item->IsExchanging())
		return false;

	if (Cell.IsEquipPosition() && !CanUnequipNow(item))
		return false;

	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
		return false;

	if (!type && IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_SELL))
		return false;

	if (item->GetCount() <= 0)
		return false;

#ifdef __SOULBIND__
	if (item->IsSealed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't do this because item is sealed!"));
		return false;
	}
#endif
	if (!type)
	{
		uint16_t wCount = item->GetCount();
		uint64_t dwPrice = item->GetShopBuyPrice();

		if (IS_SET(item->GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
		{
			if (dwPrice == 0)
				dwPrice = wCount;
			else
				dwPrice = wCount / dwPrice;
		}
		else
			dwPrice *= wCount;

		dwPrice /= 5;

		uint64_t nTotalMoney = GetGold() + dwPrice;

		if (GOLD_MAX <= nTotalMoney)
		{
			sys_err("[OVERFLOW_GOLD] id %u name %s gold %u", GetPlayerID(), GetName(), GetGold());
			return false;
		}

// #ifdef 
	// if (item->GetVnum() >= 55701 && item->GetVnum() <= 55720)
		// DBManager::instance().DirectQuery("DELETE FROM new_petsystem WHERE id = %d", item->GetID());
// #endif
		ChangeGold(true, dwPrice, false);
	}
	SyncQuickslot(QUICKSLOT_TYPE_ITEM, Cell.cell, 255);
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s has succesfully deleted"), item->GetName());
	ITEM_MANAGER::instance().RemoveItem(item);

	return true;
}
// @correction126 END

bool CHARACTER::DropGold(int32_t gold)
{
	/*if ((int32_t)gold <= 0 || gold > (int32_t)GetGold())
		return false;

	if (!CanHandleItem())
		return false;

	if (0 != g_GoldDropTimeLimitValue)
	{
		if (get_dword_time() < m_dwLastGoldDropTime+g_GoldDropTimeLimitValue)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ °ñµå¸¦ ¹ö¸± ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}
	}

	m_dwLastGoldDropTime = get_dword_time();

	LPITEM item = ITEM_MANAGER::instance().CreateItem(1, gold);

	if (item)
	{
		PIXEL_POSITION pos = GetXYZ();

		if (item->AddToGround(GetMapIndex(), pos))
		{
			ChangeGold(false, gold, true);

			// @correction005 BEGIN
			if (gold > 1000)
				LogManager::instance().CharLog(this, gold, "DROP_GOLD", "");

			item->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPGOLD]); // @correction224
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¶³¾îÁø ¾ÆÀÌÅÛÀº %dºÐ ÈÄ »ç¶óÁý´Ï´Ù."), 150/60);
			// @correction005 END
		}

		Save();
		return true;
	}*/

	return false;
}

bool CHARACTER::MoveItem(TItemPos Cell, TItemPos DestCell, uint16_t count)
{
	LPITEM item = nullptr;

	if (!IsValidItemPosition(Cell))
		return false;

	if (!(item = GetItem(Cell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (item->GetCount() < count)
		return false;

	if (INVENTORY == Cell.window_type && Cell.cell >= GetInventoryMax(item->GetInventoryType()) && IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		return false;

	if (true == item->isLocked())
		return false;

	if (!IsValidItemPosition(DestCell))
	{
		return false;
	}

	if (!CanHandleItem())
	{
		if (nullptr != DragonSoul_RefineWindow_GetOpener())
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°­È­Ã¢À» ¿¬ »óÅÂ¿¡¼­´Â ¾ÆÀÌÅÛÀ» ¿Å±æ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (DestCell.IsBeltInventoryPosition() && false == CBeltInventoryHelper::CanMoveIntoBeltInventory(item))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº º§Æ® ÀÎº¥Åä¸®·Î ¿Å±æ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

#ifdef __SWITCHBOT__
	if (Cell.IsSwitchbotPosition() && CSwitchbotManager::Instance().IsActive(GetPlayerID(), Cell.cell))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Cannot move active switchbot item."));
		return false;
	}

	if (DestCell.IsSwitchbotPosition() && !SwitchbotHelper::IsValidItem(item))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Invalid item type for switchbot."));
		return false;
	}
#endif

	if (Cell.IsEquipPosition() && !CanUnequipNow(item))
		return false;

	if (DestCell.IsEquipPosition())
	{
		if (GetItem(DestCell))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì Àåºñ¸¦ Âø¿ëÇÏ°í ÀÖ½À´Ï´Ù."));

			return false;
		}

		EquipItem(item, DestCell.cell - INVENTORY_MAX_NUM);
	}
	else
	{
		if (item->IsDragonSoul())
		{
			if (item->IsEquipped())
			{
				return DSManager::instance().PullOut(this, DestCell, item);
			}
			else
			{
				if (DestCell.window_type != DRAGON_SOUL_INVENTORY)
				{
					return false;
				}

				if (!DSManager::instance().IsValidCellForThisItem(item, DestCell))
					return false;
			}
		}
		else if (DRAGON_SOUL_INVENTORY == DestCell.window_type)
			return false;

#ifdef __EXTEND_INVEN__
		if (!DestCell.IsInventoryPosition(item->GetInventoryType()) && DestCell.window_type == INVENTORY)
			return false;
#endif

		LPITEM item2;

		if ((item2 = GetItem(DestCell)) && item != item2 && item2->IsStackable() &&
				!IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_STACK) &&
				item2->GetVnum() == item->GetVnum())
		{
			for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				if (item2->GetSocket(i) != item->GetSocket(i))
					return false;

			if (count == 0)
				count = item->GetCount();

			sys_log(0, "%s: ITEM_STACK %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);

			count = MIN(g_wItemCountLimit - item2->GetCount(), count); // @correction002 200 -> g_wItemCountLimit

			item->SetCount(item->GetCount() - count);
			item2->SetCount(item2->GetCount() + count);
			return true;
		}

		if (!IsEmptyItemGrid(DestCell, item, Cell.cell))
			return false;

		if (count == 0 || count >= item->GetCount() || !item->IsStackable() || IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
		{
			sys_log(0, "%s: ITEM_MOVE %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);

			item->RemoveFromCharacter();
			SetItem(DestCell, item);

			if (INVENTORY == Cell.window_type && INVENTORY == DestCell.window_type)
				SyncQuickslot(QUICKSLOT_TYPE_ITEM, Cell.cell, DestCell.cell);
		}
		else if (count < item->GetCount())
		{

			sys_log(0, "%s: ITEM_SPLIT %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);

			item->SetCount(item->GetCount() - count);
			LPITEM item2 = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), count);

			FN_copy_item_socket(item2, item);

			item2->AddToCharacter(this, DestCell);

			char szBuf[51+1];
			snprintf(szBuf, sizeof(szBuf), "%u %u %u %u ", item2->GetID(), item2->GetCount(), item->GetCount(), item->GetCount() + item2->GetCount());
			LogManager::instance().ItemLog(this, item, "ITEM_SPLIT", szBuf);
		}
	}

	return true;
}

namespace NPartyPickupDistribute
{
	struct FFindOwnership
	{
		LPITEM item;
		LPCHARACTER owner;

		FFindOwnership(LPITEM item)
			: item(item), owner(nullptr)
		{
		}

		void operator () (LPCHARACTER ch)
		{
			if (item->IsOwnership(ch))
				owner = ch;
		}
	};

	struct FCountNearMember
	{
		int32_t		total;
		int32_t		x, y;

		FCountNearMember(LPCHARACTER center )
			: total(0), x(center->GetX()), y(center->GetY())
		{
		}

		void operator () (LPCHARACTER ch)
		{
			if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				total += 1;
		}
	};

	struct FMoneyDistributor
	{
		int32_t		total;
		LPCHARACTER	c;
		int32_t		x, y;
		int32_t		iMoney;

		FMoneyDistributor(LPCHARACTER center, int32_t iMoney)
			: total(0), c(center), x(center->GetX()), y(center->GetY()), iMoney(iMoney)
		{
		}

		void operator ()(LPCHARACTER ch)
		{
			if (ch!=c)
				if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				{
					ch->ChangeGold(true, iMoney, true);

					if (iMoney > 1000)
						LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::instance().CharLog(ch, iMoney, "GET_GOLD", "")); // @correction222
				}
		}
	};
}

void CHARACTER::GiveGold(uint64_t iAmount)
{
	if ((int64_t)iAmount <= 0)
		return;

	sys_log(0, "GIVE_GOLD: %s %llu", GetName(), iAmount);

	if (GetParty())
	{
		LPPARTY pParty = GetParty();

		uint64_t dwTotal = iAmount;
		uint64_t dwMyAmount = dwTotal;

		NPartyPickupDistribute::FCountNearMember funcCountNearMember(this);
		pParty->ForEachOnlineMember(funcCountNearMember);

		if (funcCountNearMember.total > 1)
		{
			uint32_t dwShare = dwTotal / funcCountNearMember.total;
			dwMyAmount -= dwShare * (funcCountNearMember.total - 1);

			NPartyPickupDistribute::FMoneyDistributor funcMoneyDist(this, dwShare);

			pParty->ForEachOnlineMember(funcMoneyDist);
		}

		ChangeGold(true, dwMyAmount, true);

		if (dwMyAmount > 1000)
			LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::instance().CharLog(this, dwMyAmount, "GET_GOLD", "")); // @correction222
	}
	else
	{
		ChangeGold(true, iAmount, true);

		// @correction005
		if (iAmount > 1000)
			LOG_LEVEL_CHECK(LOG_LEVEL_MAX, LogManager::instance().CharLog(this, iAmount, "GET_GOLD", "")); // @correction222
	}
}

bool CHARACTER::PickupItem(uint32_t dwVID)
{
	if (IsObserverMode())
		return false;

	LPITEM item = nullptr;
#ifdef __PICK_ALL_ITEMS__
	auto& m_ItemMap = ITEM_MANAGER::instance().GetItemMapByID();
	for (auto& it : m_ItemMap)
#else
	for (int32_t q = 0; q < 1; ++q)
#endif
	{
#ifdef __PICK_ALL_ITEMS__
		item = it.second;
#else
		item = ITEM_MANAGER::instance().FindByVID(dwVID);
#endif
		if (item)
		{
			if (!item->GetSectree())
				continue;

#ifdef __PICK_ALL_ITEMS__
			if (item->GetWindow() != GROUND)
				continue;
#endif

			if (item->DistanceValid(this))
			{
				// @correction068 BEGIN
				if (item->GetType() == ITEM_QUEST)
				{
					if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot pickup this item if you're using quests"));
						continue;
					}
				}
				// @correction068 END

				if (item->IsOwnership(this))
				{
					if (item->GetType() == ITEM_ELK)
					{
						GiveGold(item->GetCount());
						item->RemoveFromGround();

						M2_DESTROY_ITEM(item);

						Save();
					}
					else
					{
						if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
						{
							uint16_t wCount = item->GetCount();
							uint8_t bType = item->GetInventoryType();
							bool bIsOk = false;
							for (int32_t i = INVENTORY_ONE_MAX_NUM * bType; i < GetInventoryMax(bType); ++i)
							// for (int32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
							{
								LPITEM item2 = GetInventoryItem(i);

								if (!item2)
									continue;

								if (item2->GetVnum() == item->GetVnum())
								{
									int32_t j;

									for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
										if (item2->GetSocket(j) != item->GetSocket(j))
											break;

									if (j != ITEM_SOCKET_MAX_NUM)
										continue;

									uint16_t wCount2 = MIN(g_wItemCountLimit - item2->GetCount(), wCount); // @correction002 200 -> g_wItemCountLimit
									wCount -= wCount2;

									item2->SetCount(item2->GetCount() + wCount2);

									if (wCount == 0)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ È¹µæ: %s"), item2->GetName());
										M2_DESTROY_ITEM(item);
										if (item2->GetType() == ITEM_QUEST)
											quest::CQuestManager::instance().PickupItem (GetPlayerID(), item2);
										bIsOk = true;
										break;
									}
								}
							}
							if (bIsOk)
								continue;

							item->SetCount(wCount);
						}

						int32_t iEmptyCell;
						if (item->IsDragonSoul())
						{
							if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
							{
								sys_log(0, "No empty ds inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
								continue;
							}
						}
						else
						{
							if ((iEmptyCell = GetEmptyInventory(item)) == -1)
							{
								sys_log(0, "No empty inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
								ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
								continue;
							}
						}

						item->RemoveFromGround();

						if (item->IsDragonSoul())
							item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell), true);
						else
							item->AddToCharacter(this, TItemPos(INVENTORY, iEmptyCell), true);

						char szHint[64+1];
						snprintf(szHint, sizeof(szHint), "%s %u %u", item->GetName(), item->GetCount(), item->GetOriginalVnum());
						LogManager::instance().ItemLog(this, item, "GET", szHint);
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ È¹µæ: %s"), item->GetName());

						if (item->GetType() == ITEM_QUEST)
							quest::CQuestManager::instance().PickupItem (GetPlayerID(), item);
					}

					continue;
				}
				else if (!IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_DROP) && GetParty())
				{
					NPartyPickupDistribute::FFindOwnership funcFindOwnership(item);

					GetParty()->ForEachOnlineMember(funcFindOwnership);

					LPCHARACTER owner = funcFindOwnership.owner;
					// @correction039
					if (!owner)
						continue;

					int32_t iEmptyCell;

					// @correction094 BEGIN
					if (owner)
					{
						if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
						{
							uint16_t wCount = item->GetCount();
							bool bIsOk = false;
							for (int32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
							{
								LPITEM item2 = owner->GetInventoryItem(i);

								if (!item2)
									continue;

								if (item2->GetVnum() == item->GetVnum())
								{
									int32_t j;

									for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
										if (item2->GetSocket(j) != item->GetSocket(j))
											break;

									if (j != ITEM_SOCKET_MAX_NUM)
										continue;

									uint16_t wCount2 = MIN(g_wItemCountLimit - item2->GetCount(), wCount); // @correction002 200 -> g_wItemCountLimit
									wCount -= wCount2;

									item2->SetCount(item2->GetCount() + wCount2);

									if (wCount == 0)
									{
										owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ È¹µæ: %s ´ÔÀ¸·ÎºÎÅÍ %s"), GetName(), item->GetName());
										ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ Àü´Þ: %s ´Ô¿¡°Ô %s"), owner->GetName(), item->GetName());
										M2_DESTROY_ITEM(item);
										if (item2->GetType() == ITEM_QUEST)
											quest::CQuestManager::instance().PickupItem(owner->GetPlayerID(), item2);
										bIsOk = true;
										break;
									}
								}
							}
							if (bIsOk)
								continue;

							item->SetCount(wCount);
						}
					}
					// @correction094 END

					if (item->IsDragonSoul())
					{
						if (!(owner && (iEmptyCell = owner->GetEmptyDragonSoulInventory(item)) != -1))
						{
							owner = this;

							if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
							{
								owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
								continue;
							}
						}
					}
					else
					{
						if (!(owner && (iEmptyCell = owner->GetEmptyInventory(item)) != -1))
						{
							owner = this;

							if ((iEmptyCell = GetEmptyInventory(item)) == -1)
							{
								owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ÒÁöÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÌ ³Ê¹« ¸¹½À´Ï´Ù."));
								continue;
							}
						}
					}

					item->RemoveFromGround();

					if (item->IsDragonSoul())
						item->AddToCharacter(owner, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell), true);
					else
						item->AddToCharacter(owner, TItemPos(INVENTORY, iEmptyCell), true);

					char szHint[64+1];
					snprintf(szHint, sizeof(szHint), "%s %u %u", item->GetName(), item->GetCount(), item->GetOriginalVnum());
					LogManager::instance().ItemLog(owner, item, "GET", szHint);

					if (owner == this)
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ È¹µæ: %s"), item->GetName());
					else
					{
						owner->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ È¹µæ: %s ´ÔÀ¸·ÎºÎÅÍ %s"), GetName(), item->GetName());
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ Àü´Þ: %s ´Ô¿¡°Ô %s"), owner->GetName(), item->GetName());
					}

					if (item->GetType() == ITEM_QUEST)
						quest::CQuestManager::instance().PickupItem (owner->GetPlayerID(), item);

					continue;
				}
			}
		}
	}

	return true;
}

bool CHARACTER::SwapItem(uint16_t wCell, uint16_t wDestCell)
{
	if (!CanHandleItem())
		return false;

	TItemPos srcCell(INVENTORY, wCell), destCell(INVENTORY, wDestCell);

	if (srcCell.IsDragonSoulEquipPosition() || destCell.IsDragonSoulEquipPosition())
		return false;

	if (wCell == wDestCell)
		return false;

	if (srcCell.IsEquipPosition() && destCell.IsEquipPosition())
		return false;

	LPITEM item1, item2;

	if (srcCell.IsEquipPosition())
	{
		item1 = GetInventoryItem(wDestCell);
		item2 = GetInventoryItem(wCell);
	}
	else
	{
		item1 = GetInventoryItem(wCell);
		item2 = GetInventoryItem(wDestCell);
	}

	if (!item1 || !item2)
		return false;

	if (item1 == item2)
	{
	    sys_log(0, "[WARNING][WARNING][HACK USER!] : %s %d %d", m_stName.c_str(), wCell, wDestCell);
	    return false;
	}

	if (!IsEmptyItemGrid(TItemPos (INVENTORY, item1->GetCell()), item2, item1->GetCell()))
		return false;

	if (TItemPos(EQUIPMENT, item2->GetCell()).IsEquipPosition())
	{
		uint16_t wEquipCell = item2->GetCell() - INVENTORY_MAX_NUM;
		uint16_t wInvenCell = item1->GetCell();

		if (item2->IsDragonSoul() || item2->GetType() == ITEM_BELT) // @correction041
		{
			if (false == CanUnequipNow(item2) || false == CanEquipNow(item1))
				return false;
		}

		if (wEquipCell != item1->FindEquipCell(this))
			return false;

		item2->RemoveFromCharacter();

		if (item1->EquipTo(this, wEquipCell))
		{
			item2->AddToCharacter(this, TItemPos(INVENTORY, wInvenCell));
// @correction089 BEGIN
			item2->ModifyPoints(false);
			ComputePoints();
// @correction089 END
		} 
		else
			sys_err("SwapItem cannot equip %s! item1 %s", item2->GetName(), item1->GetName());
	}
	else
	{
		uint16_t wCell1 = item1->GetCell();
		uint16_t wCell2 = item2->GetCell();

		item1->RemoveFromCharacter();
		item2->RemoveFromCharacter();

		item1->AddToCharacter(this, TItemPos(INVENTORY, wCell2));
		item2->AddToCharacter(this, TItemPos(INVENTORY, wCell1));
	}

	return true;
}

bool CHARACTER::UnequipItem(LPITEM item)
{
	int32_t pos;

	if (false == CanUnequipNow(item))
		return false;

	if (item->IsDragonSoul())
		pos = GetEmptyDragonSoulInventory(item);
	else
		pos = GetEmptyInventory(item);

	if (item->GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		ShowAlignment(true);

#ifdef __COSTUME_MOUNT__
	if ((item->GetType() == ITEM_COSTUME) && (COSTUME_MOUNT == item->GetSubType()))
#ifdef __MOUNT__
	{
		CMountSystem* MountSystem = GetMountSystem();
#ifdef __CHANGE_LOOK__
		uint32_t dwVnum = item->GetChangeLook() ? item->GetChangeLook() : item->GetValue(4);
#else
		uint32_t dwVnum = item->GetValue(4);
#endif
		if (MountSystem)
			MountSystem->Unsummon(dwVnum);
	}
#else
		quest::CQuestManager::instance().UseItem(GetPlayerID(), item, false);
#endif
#endif

	item->RemoveFromCharacter();

	if (item->IsDragonSoul())
		item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, pos));
	else
		item->AddToCharacter(this, TItemPos(INVENTORY, pos));

	CheckMaximumPoints();

	return true;
}

bool CHARACTER::EquipItem(LPITEM item, int32_t iCandidateCell)
{
	if (item->IsExchanging())
		return false;

	if (false == item->IsEquipable())
		return false;

	if (false == CanEquipNow(item))
		return false;

	int32_t iWearCell = item->FindEquipCell(this, iCandidateCell);

	if (iWearCell < 0)
		return false;

	if ((iWearCell == WEAR_BODY || iWearCell == WEAR_COSTUME_BODY) && IsRiding() && get_global_time() < GetQuestFlag("zirh.engel"))
	{
		ChatPacket(CHAT_TYPE_INFO, "Bu islemi bu kadar hizli yapamassin!");
		return false;
	}

// @correction135 BEGIN
	for (int32_t i = 0; i < 5; ++i)
	{
		for (int32_t a = 0; a < 5; ++a)
		{
			if (item->GetAttributeType(a) == item->GetAttributeType(i) && item->GetAttributeType(a) && a != i)
			{
				ITEM_MANAGER::instance().RemoveItem(item);
				//ChatPacket(CHAT_TYPE_INFO, "Bug Yapma Kocm");
				return false;
			}
		}
	}
// @correction135 END

	//@correction090 BEGIN
	// auto it = m_ItemUseTimesMap.find(1);
	// if (it != m_ItemUseTimesMap.end())
	// {
		// if (time(nullptr) < m_ItemUseTimesMap[1])
		// {
			// ChatPacket(CHAT_TYPE_INFO, "Bunu yapabilmek i?n %d saniye beklemelisin!", m_ItemUseTimesMap[1] - time(nullptr));
			// return false;
		// }
		// else
			// it->second = time(nullptr) + 3;
	// }
	// else
		// m_ItemUseTimesMap.emplace(1, time(nullptr) + 3);
	//@correction090 END

	if (iWearCell == WEAR_BODY && IsRiding() && ((item->GetVnum() >= 11901 && item->GetVnum() <= 11904) || item->GetVnum() == 50201)) // @correction015
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»À» Åº »óÅÂ¿¡¼­ ¿¹º¹À» ÀÔÀ» ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (iWearCell != WEAR_ARROW && IsPolymorphed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µÐ°© Áß¿¡´Â Âø¿ëÁßÀÎ Àåºñ¸¦ º¯°æÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if (item->GetType() == ITEM_RING && ((GetWear(WEAR_RING1) && GetWear(WEAR_RING1)->GetVnum() == item->GetVnum()) || (GetWear(WEAR_RING2) && GetWear(WEAR_RING2)->GetVnum() == item->GetVnum())))
		return false;

	if (FN_check_item_sex(this, item) == false)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¼ºº°ÀÌ ¸ÂÁö¾Ê¾Æ ÀÌ ¾ÆÀÌÅÛÀ» »ç¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return false;
	}

	if(item->IsRideItem() && IsRiding())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì Å»°ÍÀ» ÀÌ¿ëÁßÀÔ´Ï´Ù."));
		return false;
	}

#if defined(__COSTUME_MOUNT__) && defined(__MOUNT__)
	if (item->GetType() == ITEM_COSTUME && COSTUME_MOUNT == item->GetSubType())
	{
		CMountSystem* MountSystem = GetMountSystem();
		if (MountSystem)
			if (MountSystem->CountSummoned())
				return false;
	}
#endif

	uint32_t dwCurTime = get_dword_time();

	if (iWearCell != WEAR_ARROW
		&& (dwCurTime - GetLastAttackTime() <= 1500 || dwCurTime - m_dwLastSkillTime <= 1500))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°¡¸¸È÷ ÀÖÀ» ¶§¸¸ Âø¿ëÇÒ ¼ö ÀÖ½À´Ï´Ù."));
		return false;
	}

	if (item->IsDragonSoul())
	{
		if(GetInventoryItem(INVENTORY_MAX_NUM + iWearCell))
		{
			ChatPacket(CHAT_TYPE_INFO, "ÀÌ¹Ì °°Àº Á¾·ùÀÇ ¿ëÈ¥¼®À» Âø¿ëÇÏ°í ÀÖ½À´Ï´Ù.");
			return false;
		}

		if (!item->EquipTo(this, iWearCell))
		{
			return false;
		}
	}
	else
	{
		if (GetWear(iWearCell) && !IS_SET(GetWear(iWearCell)->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		{
			if (item->GetWearFlag() == WEARABLE_ABILITY)
				return false;

			if (false == SwapItem(item->GetCell(), INVENTORY_MAX_NUM + iWearCell))
			{
				return false;
			}
		}
		else
		{
			uint16_t wOldCell = item->GetCell();

			if (item->EquipTo(this, iWearCell))
			{
				SyncQuickslot(QUICKSLOT_TYPE_ITEM, wOldCell, iWearCell);
			}
		}
	}

	if (true == item->IsEquipped())
	{
		if (-1 != item->GetProto()->cLimitRealTimeFirstUseIndex)
		{
			if (0 == item->GetSocket(1))
			{
				int32_t duration = (0 != item->GetSocket(0)) ? item->GetSocket(0) : item->GetProto()->aLimits[(uint8_t)(item->GetProto()->cLimitRealTimeFirstUseIndex)].lValue;

				if (0 == duration)
					duration = 60 * 60 * 24 * 7;

				item->SetSocket(0, time(0) + duration);
				item->StartRealTimeExpireEvent();
			}

			item->SetSocket(1, item->GetSocket(1) + 1);
		}

		if (item->GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
			ShowAlignment(false);

		const uint32_t& dwVnum = item->GetVnum();

		if (true == CItemVnumHelper::IsRamadanMoonRing(dwVnum))
		{
			this->EffectPacket(SE_EQUIP_RAMADAN_RING);
		}
		else if (true == CItemVnumHelper::IsHalloweenCandy(dwVnum))
		{
			this->EffectPacket(SE_EQUIP_HALLOWEEN_CANDY);
		}
		else if (true == CItemVnumHelper::IsHappinessRing(dwVnum))
		{
			this->EffectPacket(SE_EQUIP_HAPPINESS_RING);
		}
		else if (true == CItemVnumHelper::IsLovePendant(dwVnum))
		{
			this->EffectPacket(SE_EQUIP_LOVE_PENDANT);
		}
		else if (ITEM_UNIQUE == item->GetType() && 0 != item->GetSIGVnum())
		{
			const CSpecialItemGroup* pGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(item->GetSIGVnum());
			if (nullptr != pGroup)
			{
				const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::instance().GetSpecialAttrGroup(pGroup->GetAttrVnum(item->GetVnum()));
				if (nullptr != pAttrGroup)
				{
					const std::string& std = pAttrGroup->m_stEffectFileName;
					SpecificEffectPacket(std.c_str());
				}
			}
		}
#ifdef __ACCE_SYSTEM__ // @correction191
		else if ((item->GetType() == ITEM_COSTUME) && (item->GetSubType() == COSTUME_ACCE))
			this->EffectPacket(SE_EFFECT_ACCE_EQUIP);
#endif

		if (UNIQUE_SPECIAL_RIDE == item->GetSubType() && IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE))
		{
			quest::CQuestManager::instance().UseItem(GetPlayerID(), item, false);
		}

		if ((iWearCell == WEAR_BODY || iWearCell == WEAR_COSTUME_BODY) && IsRiding())
			SetQuestFlag("zirh.engel", get_global_time() + 4);

#ifdef __COSTUME_MOUNT__
		if ((item->GetType() == ITEM_COSTUME) && (COSTUME_MOUNT == item->GetSubType()))
#ifdef __MOUNT__
		{
#ifdef __CHANGE_LOOK__
			uint32_t dwVnum = item->GetChangeLook() ? item->GetChangeLook() : item->GetValue(4);
#else
			uint32_t dwVnum = item->GetValue(4);
#endif
			CMountSystem* MountSystem = GetMountSystem();
			if (MountSystem)
				MountSystem->Summon(dwVnum, GetName());
		}
#else
			quest::CQuestManager::instance().UseItem(GetPlayerID(), item, false);
#endif
#endif
	}

	return true;
}

void CHARACTER::BuffOnAttr_AddBuffsFromItem(LPITEM pItem)
{
	for (size_t i = 0; i < sizeof(g_aBuffOnAttrPoints)/sizeof(g_aBuffOnAttrPoints[0]); i++)
	{
		TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(g_aBuffOnAttrPoints[i]);
		if (it != m_map_buff_on_attrs.end())
		{
			it->second->AddBuffFromItem(pItem);
		}
	}
}

void CHARACTER::BuffOnAttr_RemoveBuffsFromItem(LPITEM pItem)
{
	for (size_t i = 0; i < sizeof(g_aBuffOnAttrPoints)/sizeof(g_aBuffOnAttrPoints[0]); i++)
	{
		TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(g_aBuffOnAttrPoints[i]);
		if (it != m_map_buff_on_attrs.end())
		{
			it->second->RemoveBuffFromItem(pItem);
		}
	}
}

void CHARACTER::BuffOnAttr_ClearAll()
{
	for (TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.begin(); it != m_map_buff_on_attrs.end(); it++)
	{
		CBuffOnAttributes* pBuff = it->second;
		if (pBuff)
		{
			pBuff->Initialize();
		}
	}
}

void CHARACTER::BuffOnAttr_ValueChange(uint8_t bType, uint8_t bOldValue, uint8_t bNewValue)
{
	TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(bType);

	if (0 == bNewValue)
	{
		if (m_map_buff_on_attrs.end() == it)
			return;
		else
			it->second->Off();
	}
	else if(0 == bOldValue)
	{
		CBuffOnAttributes* pBuff = nullptr;
		if (m_map_buff_on_attrs.end() == it)
		{
			switch (bType)
			{
			case POINT_ENERGY:
				{
					static uint8_t abSlot[] = { WEAR_BODY, WEAR_HEAD, WEAR_FOOTS, WEAR_WRIST, WEAR_WEAPON, WEAR_NECK, WEAR_EAR, WEAR_SHIELD };
					static std::vector <uint8_t> vec_slots (abSlot, abSlot + _countof(abSlot));
					pBuff = M2_NEW CBuffOnAttributes(this, bType, &vec_slots);
				}
				break;
			case POINT_COSTUME_ATTR_BONUS:
				{
					static uint8_t abSlot[] = {WEAR_COSTUME_BODY, WEAR_COSTUME_HAIR,
#ifdef __COSTUME_WEAPON__ // @correction204
WEAR_COSTUME_WEAPON,
#endif
};
					static std::vector <uint8_t> vec_slots (abSlot, abSlot + _countof(abSlot));
					pBuff = M2_NEW CBuffOnAttributes(this, bType, &vec_slots);
				}
				break;
			default:
				break;
			}
			m_map_buff_on_attrs.insert(TMapBuffOnAttrs::value_type(bType, pBuff));

		}
		else
			pBuff = it->second;
		if (pBuff != nullptr) // @correction125
			pBuff->On(bNewValue);
	}
	else
	{
		assert (m_map_buff_on_attrs.end() != it);
		it->second->ChangeBuffValue(bNewValue);
	}
}


LPITEM CHARACTER::FindSpecifyItem(uint32_t vnum) const
{
	uint8_t bType = ITEM_MANAGER::instance().GetInventoryType(vnum);
	for (int32_t i = INVENTORY_ONE_MAX_NUM * bType; i < GetInventoryMax(bType); ++i)
	// for (int32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
		if (GetInventoryItem(i) && GetInventoryItem(i)->GetVnum() == vnum)
			return GetInventoryItem(i);

	return nullptr;
}

LPITEM CHARACTER::FindItemByID(uint32_t id) const
{
	for (int32_t i = 0; i < INVENTORY_MAX_NUM; ++i)	
	{
		if (nullptr != GetInventoryItem(i) && GetInventoryItem(i)->GetID() == id)
			return GetInventoryItem(i);
	}

	for (int32_t i=BELT_INVENTORY_SLOT_START; i < BELT_INVENTORY_SLOT_END ; ++i)
	{
		if (nullptr != GetInventoryItem(i) && GetInventoryItem(i)->GetID() == id)
			return GetInventoryItem(i);
	}

	return nullptr;
}

int32_t CHARACTER::CountSpecifyItem(uint32_t vnum) const
{
	int32_t	count = 0;
	LPITEM item;
	uint8_t bType = ITEM_MANAGER::instance().GetInventoryType(vnum);
	for (int32_t i = INVENTORY_ONE_MAX_NUM * bType; i < GetInventoryMax(bType); ++i)
	// for (int32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		item = GetInventoryItem(i);
		if (nullptr != item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
			{
				continue;
			}
			else
			{
				count += item->GetCount();
			}
		}
	}

	return count;
}

void CHARACTER::RemoveSpecifyItem(uint32_t vnum, uint32_t count)
{
	if (0 == count)
		return;

	uint8_t bType = ITEM_MANAGER::instance().GetInventoryType(vnum);
	for (int32_t i = INVENTORY_ONE_MAX_NUM * bType; i < GetInventoryMax(bType); ++i)
	// for (int32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		if (nullptr == GetInventoryItem(i))
			continue;

		if (GetInventoryItem(i)->GetVnum() != vnum)
			continue;

		if(m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (vnum >= 80003 && vnum <= 80007)
			LogManager::instance().GoldBarLog(GetPlayerID(), GetInventoryItem(i)->GetID(), QUEST, "RemoveSpecifyItem");

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}

	if (count)
		sys_log(0, "CHARACTER::RemoveSpecifyItem cannot remove enough item vnum %u, still remain %d", vnum, count);
}

int32_t CHARACTER::CountSpecifyTypeItem(uint8_t type) const
{
	int32_t	count = 0;
	for (uint32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		LPITEM pItem = GetInventoryItem(i);
		if (pItem != nullptr && pItem->GetType() == type)
		{
			count += pItem->GetCount();
		}
	}

	return count;
}

void CHARACTER::RemoveSpecifyTypeItem(uint8_t type, uint32_t count)
{
	if (0 == count)
		return;

	for (uint32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		if (nullptr == GetInventoryItem(i))
			continue;

		if (GetInventoryItem(i)->GetType() != type)
			continue;

		if(m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}
}

void CHARACTER::AutoGiveItem(LPITEM item, bool longOwnerShip)
{
	if (nullptr == item)
	{
		sys_err ("nullptr point.");
		return;
	}
	if (item->GetOwner())
	{
		sys_err ("item %d 's owner exists!",item->GetID());
		return;
	}

	int32_t cell;
	if (item->IsDragonSoul())
	{
		cell = GetEmptyDragonSoulInventory(item);
	}
	else
	{
		cell = GetEmptyInventory(item);
	}

	if (cell != -1)
	{
		if (item->IsDragonSoul())
			item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, cell), true);
		else
			item->AddToCharacter(this, TItemPos(INVENTORY, cell), true);

		LogManager::instance().ItemLog(this, item, "SYSTEM", item->GetName());

		if (item->GetType() == ITEM_USE && item->GetSubType() == USE_POTION)
		{
			TQuickslot * pSlot;

			if (GetQuickslot(0, &pSlot) && pSlot->type == QUICKSLOT_TYPE_NONE)
			{
				TQuickslot slot;
				slot.type = QUICKSLOT_TYPE_ITEM;
				slot.pos = cell;
				SetQuickslot(0, slot);
			}
		}
	}
	else
	{
		item->AddToGround (GetMapIndex(), GetXYZ());
		item->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]); // @correction224

		if (longOwnerShip)
			item->SetOwnership (this, 300);
		else
			item->SetOwnership (this, 60);
		LogManager::instance().ItemLog(this, item, "SYSTEM_DROP", item->GetName());
	}
}

LPITEM CHARACTER::AutoGiveItem(uint32_t dwItemVnum, uint16_t wCount, int32_t iRarePct, bool bMsg)
{
	TItemTable * p = ITEM_MANAGER::instance().GetTable(dwItemVnum);

	if (!p)
		return nullptr;

	DBManager::instance().SendMoneyLog(MONEY_LOG_DROP, dwItemVnum, wCount);

	if (p->dwFlags & ITEM_FLAG_STACKABLE && p->bType != ITEM_BLEND)
	{
		uint8_t bType = ITEM_MANAGER::instance().GetInventoryType(0, p);
		for (int32_t i = INVENTORY_ONE_MAX_NUM * bType; i < GetInventoryMax(bType); ++i)
		// for (int32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			LPITEM item = GetInventoryItem(i);

			if (!item)
				continue;

			if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
			{
				if (IS_SET(p->dwFlags, ITEM_FLAG_MAKECOUNT))
				{
					if (wCount < p->alValues[1])
						wCount = p->alValues[1];
				}

				uint16_t wCount2 = MIN(g_wItemCountLimit - item->GetCount(), wCount); // @correction002 200 -> g_wItemCountLimit
				wCount -= wCount2;

				item->SetCount(item->GetCount() + wCount2);

				if (wCount == 0)
				{
					if (bMsg)
						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ È¹µæ: %s"), item->GetName());

					return item;
				}
			}
		}
	}

	LPITEM item = ITEM_MANAGER::instance().CreateItem(dwItemVnum, wCount, 0, true);

	if (!item)
	{
		sys_err("cannot create item by vnum %u (name: %s)", dwItemVnum, GetName());
		return nullptr;
	}

	if (item->GetType() == ITEM_BLEND)
	{
		uint8_t bType = ITEM_MANAGER::instance().GetInventoryType(0, p);
		for (int32_t i = INVENTORY_ONE_MAX_NUM * bType; i < GetInventoryMax(bType); i++)
		// for (int32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			LPITEM inv_item = GetInventoryItem(i);

			if (inv_item == nullptr) continue;

			if (inv_item->GetType() == ITEM_BLEND)
			{
				if (inv_item->GetVnum() == item->GetVnum())
				{
					if (inv_item->GetSocket(0) == item->GetSocket(0) &&
							inv_item->GetSocket(1) == item->GetSocket(1) &&
							inv_item->GetSocket(2) == item->GetSocket(2) &&
							inv_item->GetCount() < g_wItemCountLimit) // @correction002 ITEM_MAX_COUNT -> g_wItemCountLimit
					{
						inv_item->SetCount(inv_item->GetCount() + item->GetCount());
						return inv_item;
					}
				}
			}
		}
	}

	int32_t iEmptyCell;
	if (item->IsDragonSoul())
	{
		iEmptyCell = GetEmptyDragonSoulInventory(item);
	}
	else
		iEmptyCell = GetEmptyInventory(item);

	if (iEmptyCell != -1)
	{
		if (bMsg)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÀÌÅÛ È¹µæ: %s"), item->GetName());

		if (item->IsDragonSoul())
			item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell), true);
		else
			item->AddToCharacter(this, TItemPos(INVENTORY, iEmptyCell), true);
		LogManager::instance().ItemLog(this, item, "SYSTEM", item->GetName());

		if (item->GetType() == ITEM_USE && item->GetSubType() == USE_POTION)
		{
			TQuickslot * pSlot;

			if (GetQuickslot(0, &pSlot) && pSlot->type == QUICKSLOT_TYPE_NONE)
			{
				TQuickslot slot;
				slot.type = QUICKSLOT_TYPE_ITEM;
				slot.pos = iEmptyCell;
				SetQuickslot(0, slot);
			}
		}
	}
	else
	{
// #ifdef __FAST_USE_GIFTBOX__
		// M2_DESTROY_ITEM(item);
		// if (bMsg)
			// ChatPacket(CHAT_TYPE_INFO, "Envanterinde yer yok!");

		// return nullptr;
// #else
		item->AddToGround(GetMapIndex(), GetXYZ());
		item->StartDestroyEvent(g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]); // @correction224
		if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP))
			item->SetOwnership(this, 300);
		else
			item->SetOwnership(this, 60);
		LogManager::instance().ItemLog(this, item, "SYSTEM_DROP", item->GetName());
// #endif
	}

	sys_log(0,
		"7: %d %d", dwItemVnum, wCount);
	return item;
}

bool CHARACTER::GiveItem(LPCHARACTER victim, TItemPos Cell)
{
	if (!CanHandleItem())
		return false;

	// @correction068 BEGIN
	if (quest::CQuestManager::instance().GetPCForce(GetPlayerID())->IsRunning() == true)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You cannot take this item if you're using quests"));
		return false;
	}
	// @correction068 END

	LPITEM item = GetItem(Cell);

	if (item && !item->IsExchanging())
	{
		if (victim->CanReceiveItem(this, item))
		{
			victim->ReceiveItem(this, item);
			return true;
		}
	}

	return false;
}

bool CHARACTER::CanReceiveItem(LPCHARACTER from, LPITEM item) const
{
	if (IsPC())
		return false;

	if (DISTANCE_APPROX(GetX() - from->GetX(), GetY() - from->GetY()) > 2000)
		return false;

#ifdef __SOULBIND__ // @correction202
	if (item->IsSealed())
	{
		from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't upgrade sealbind item."));
		return false;
	}
#endif

	switch (GetRaceNum())
	{
		case fishing::CAMPFIRE_MOB:
			if (item->GetType() == ITEM_FISH &&
					(item->GetSubType() == FISH_ALIVE || item->GetSubType() == FISH_DEAD))
				return true;
			break;

		case fishing::FISHER_MOB:
			if (item->GetType() == ITEM_ROD)
				return true;
			break;

		case BLACKSMITH_WEAPON_MOB:
		case DEVILTOWER_BLACKSMITH_WEAPON_MOB:
			if (item->GetType() == ITEM_WEAPON &&
					item->GetRefinedVnum())
				return true;
			else
				return false;
			break;

		case BLACKSMITH_ARMOR_MOB:
		case DEVILTOWER_BLACKSMITH_ARMOR_MOB:
			if (item->GetType() == ITEM_ARMOR &&
					(item->GetSubType() == ARMOR_BODY || item->GetSubType() == ARMOR_SHIELD || item->GetSubType() == ARMOR_HEAD) &&
					item->GetRefinedVnum())
				return true;
			else
				return false;
			break;

		case BLACKSMITH_ACCESSORY_MOB:
		case DEVILTOWER_BLACKSMITH_ACCESSORY_MOB:
			if (item->GetType() == ITEM_ARMOR &&
					!(item->GetSubType() == ARMOR_BODY || item->GetSubType() == ARMOR_SHIELD || item->GetSubType() == ARMOR_HEAD) &&
					item->GetRefinedVnum())
				return true;
			else
				return false;
			break;

		case BLACKSMITH_MOB:
			if (item->GetRefinedVnum() && item->GetRefineSet() < 500)
			{
				return true;
			}
			else
			{
				return false;
			}

		case BLACKSMITH2_MOB:
			if (item->GetRefineSet() >= 500)
			{
				return true;
			}
			else
			{
				return false;
			}

		case ALCHEMIST_MOB:
			if (item->GetRefinedVnum())
				return true;
			break;

		case 20101:
		case 20102:
		case 20103:
			if (item->GetVnum() == ITEM_REVIVE_HORSE_1)
			{
				if (!IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á×Áö ¾ÊÀº ¸»¿¡°Ô ¼±ÃÊ¸¦ ¸ÔÀÏ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1)
			{
				if (IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á×Àº ¸»¿¡°Ô »ç·á¸¦ ¸ÔÀÏ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_2 || item->GetVnum() == ITEM_HORSE_FOOD_3)
			{
				return false;
			}
			break;
		case 20104:
		case 20105:
		case 20106:
			if (item->GetVnum() == ITEM_REVIVE_HORSE_2)
			{
				if (!IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á×Áö ¾ÊÀº ¸»¿¡°Ô ¼±ÃÊ¸¦ ¸ÔÀÏ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_2)
			{
				if (IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á×Àº ¸»¿¡°Ô »ç·á¸¦ ¸ÔÀÏ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1 || item->GetVnum() == ITEM_HORSE_FOOD_3)
			{
				return false;
			}
			break;
		case 20107:
		case 20108:
		case 20109:
			if (item->GetVnum() == ITEM_REVIVE_HORSE_3)
			{
				if (!IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á×Áö ¾ÊÀº ¸»¿¡°Ô ¼±ÃÊ¸¦ ¸ÔÀÏ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_3)
			{
				if (IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Á×Àº ¸»¿¡°Ô »ç·á¸¦ ¸ÔÀÏ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1 || item->GetVnum() == ITEM_HORSE_FOOD_2)
			{
				return false;
			}
			break;
	}

	{
		return true;
	}

	return false;
}

void CHARACTER::ReceiveItem(LPCHARACTER from, LPITEM item)
{
	if (IsPC())
		return;

	switch (GetRaceNum())
	{
#ifdef __GUILD_DRAGONLAIR__
		case STATUE_VNUM:
			{
				if (CMgr::instance().IsMeleyMap(from->GetMapIndex()))
					CMgr::instance().OnKillStatue(item, from, this);
			}
		break;
#endif
		case fishing::CAMPFIRE_MOB:
			if (item->GetType() == ITEM_FISH && (item->GetSubType() == FISH_ALIVE || item->GetSubType() == FISH_DEAD))
				fishing::Grill(from, item);
			else
			{
				from->SetQuestNPCID(GetVID());
				quest::CQuestManager::instance().TakeItem(from->GetPlayerID(), GetRaceNum(), item);
			}
			break;

		case DEVILTOWER_BLACKSMITH_WEAPON_MOB:
		case DEVILTOWER_BLACKSMITH_ARMOR_MOB:
		case DEVILTOWER_BLACKSMITH_ACCESSORY_MOB:
			if (item->GetRefinedVnum() != 0 && item->GetRefineSet() != 0 && item->GetRefineSet() < 500)
			{
				from->SetRefineNPC(this);
				from->RefineInformation(item->GetCell(), REFINE_TYPE_MONEY_ONLY);
			}
			else
			{
				from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			}
			break;

		case BLACKSMITH_MOB:
		case BLACKSMITH2_MOB:
		case BLACKSMITH_WEAPON_MOB:
		case BLACKSMITH_ARMOR_MOB:
		case BLACKSMITH_ACCESSORY_MOB:
			if (item->GetRefinedVnum())
			{
				from->SetRefineNPC(this);
				from->RefineInformation(item->GetCell(), REFINE_TYPE_NORMAL);
			}
			else
			{
				from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ ¾ÆÀÌÅÛÀº °³·®ÇÒ ¼ö ¾ø½À´Ï´Ù."));
			}
			break;

		case 20101:
		case 20102:
		case 20103:
		case 20104:
		case 20105:
		case 20106:
		case 20107:
		case 20108:
		case 20109:
			if (item->GetVnum() == ITEM_REVIVE_HORSE_1 ||
					item->GetVnum() == ITEM_REVIVE_HORSE_2 ||
					item->GetVnum() == ITEM_REVIVE_HORSE_3)
			{
				from->ReviveHorse();
				item->SetCount(item->GetCount()-1);
				from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»¿¡°Ô ¼±ÃÊ¸¦ ÁÖ¾ú½À´Ï´Ù."));
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1 ||
					item->GetVnum() == ITEM_HORSE_FOOD_2 ||
					item->GetVnum() == ITEM_HORSE_FOOD_3)
			{
				from->FeedHorse();
				from->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¸»¿¡°Ô »ç·á¸¦ ÁÖ¾ú½À´Ï´Ù."));
				item->SetCount(item->GetCount()-1);
				EffectPacket(SE_HPUP_RED);
			}
			break;

		default:
			sys_log(0, "TakeItem %s %d %s", from->GetName(), GetRaceNum(), item->GetName());
			from->SetQuestNPCID(GetVID());
			quest::CQuestManager::instance().TakeItem(from->GetPlayerID(), GetRaceNum(), item);
			break;
	}
}

bool CHARACTER::IsEquipUniqueItem(uint32_t dwItemVnum) const
{
	{
		LPITEM u = GetWear(WEAR_UNIQUE1);

		if (u && u->GetVnum() == dwItemVnum)
			return true;
	}

	{
		LPITEM u = GetWear(WEAR_UNIQUE2);

		if (u && u->GetVnum() == dwItemVnum)
			return true;
	}

	if (dwItemVnum == UNIQUE_ITEM_RING_OF_LANGUAGE)
		return IsEquipUniqueItem(UNIQUE_ITEM_RING_OF_LANGUAGE_SAMPLE);

#ifdef __COSTUME_MOUNT__
	{
		LPITEM u = GetWear(WEAR_COSTUME_MOUNT);

		if (u && u->GetVnum() == dwItemVnum)
			return true;
	}
#endif

	return false;
}

bool CHARACTER::IsEquipUniqueGroup(uint32_t dwGroupVnum) const
{
	{
		LPITEM u = GetWear(WEAR_UNIQUE1);

		if (u && u->GetSpecialGroup() == (int32_t) dwGroupVnum)
			return true;
	}

	{
		LPITEM u = GetWear(WEAR_UNIQUE2);

		if (u && u->GetSpecialGroup() == (int32_t) dwGroupVnum)
			return true;
	}

#ifdef __COSTUME_MOUNT__
	{
		LPITEM u = GetWear(WEAR_COSTUME_MOUNT);

		if (u && u->GetSpecialGroup() == (int32_t)dwGroupVnum)
			return true;
	}
#endif

	return false;
}

void CHARACTER::SetRefineMode(int32_t iAdditionalCell)
{
	m_iRefineAdditionalCell = iAdditionalCell;
	m_bUnderRefine = true;
}

void CHARACTER::ClearRefineMode()
{
	m_bUnderRefine = false;
	SetRefineNPC( nullptr );
}

bool CHARACTER::GiveItemFromSpecialItemGroup(uint32_t dwGroupNum, std::vector<uint32_t> &dwItemVnums,
 std::vector<uint32_t> &dwItemCounts, std::vector <LPITEM> &item_gets, int32_t &count
#ifdef __FAST_USE_GIFTBOX__
, uint16_t& wCount
#endif											
)
{
	const CSpecialItemGroup* pGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(dwGroupNum);

	if (!pGroup)
	{
		sys_err("cannot find special item group %d", dwGroupNum);
		return false;
	}

	bool bSuccess = false;
#ifdef __FAST_USE_GIFTBOX__
	bool bMsg = false;
	int32_t iCount = wCount;
	for (int32_t j = 0; j < iCount; ++j)
#endif
	{
		std::vector <int32_t> idxes;
		int32_t n = pGroup->GetMultiIndex(idxes);

		for (int32_t i = 0; i < n; i++)
		{
			bSuccess = false;
			int32_t idx = idxes[i];
			uint32_t dwVnum = pGroup->GetVnum(idx);
			uint32_t dwCount = pGroup->GetCount(idx);
			int32_t	iRarePct = pGroup->GetRarePct(idx);
			LPITEM item_get = nullptr;
			switch (dwVnum)
			{
				case CSpecialItemGroup::GOLD:
					ChangeGold(true, dwCount);
					LogManager::instance().CharLog(this, dwCount, "TREASURE_GOLD", "");

					bSuccess = true;
					break;
				case CSpecialItemGroup::EXP:
					{
						PointChange(POINT_EXP, dwCount);
						LogManager::instance().CharLog(this, dwCount, "TREASURE_EXP", "");

						bSuccess = true;
					}
					break;

				case CSpecialItemGroup::MOB:
					{
						sys_log(0, "CSpecialItemGroup::MOB %d", dwCount);
						int32_t x = GetX() + number(-500, 500);
						int32_t y = GetY() + number(-500, 500);

						LPCHARACTER ch = CHARACTER_MANAGER::instance().SpawnMob(dwCount, GetMapIndex(), x, y, 0, true, -1);
						if (ch)
							ch->SetAggressive();
						bSuccess = true;
					}
					break;
				case CSpecialItemGroup::SLOW:
					{
						sys_log(0, "CSpecialItemGroup::SLOW %d", -(int32_t)dwCount);
						AddAffect(AFFECT_SLOW, POINT_MOV_SPEED, -(int32_t)dwCount, AFF_SLOW, 300, 0, true);
						bSuccess = true;
					}
					break;
				case CSpecialItemGroup::DRAIN_HP:
					{
						int32_t iDropHP = GetMaxHP()*dwCount/100;
						sys_log(0, "CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
						iDropHP = MIN(iDropHP, GetHP()-1);
						sys_log(0, "CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
						PointChange(POINT_HP, -iDropHP);
						bSuccess = true;
					}
					break;
				case CSpecialItemGroup::POISON:
					{
						AttackedByPoison(nullptr);
						bSuccess = true;
					}
					break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
				case CSpecialItemGroup::BLEEDING:
					{
						AttackedByBleeding(nullptr);
						bSuccess = true;
					}
					break;
#endif
				case CSpecialItemGroup::MOB_GROUP:
					{
						int32_t sx = GetX() - number(300, 500);
						int32_t sy = GetY() - number(300, 500);
						int32_t ex = GetX() + number(300, 500);
						int32_t ey = GetY() + number(300, 500);
						CHARACTER_MANAGER::instance().SpawnGroup(dwCount, GetMapIndex(), sx, sy, ex, ey, nullptr, true);

						bSuccess = true;
					}
					break;
				default:
					{
#ifdef __FAST_USE_GIFTBOX__
						if (iCount == j + 1)
							bMsg = true;

						item_get = AutoGiveItem(dwVnum, dwCount, iRarePct, bMsg);
#else
						item_get = AutoGiveItem(dwVnum, dwCount, iRarePct);
#endif

						if (item_get)
						{
							bSuccess = true;
						}
					}
					break;
			}

			if (bSuccess)
			{
				dwItemVnums.push_back(dwVnum);
				dwItemCounts.push_back(dwCount);
				item_gets.push_back(item_get);
				count++;

			}
#ifndef __FAST_USE_GIFTBOX__
			else
			{
				return false;
			}
#endif
		}

#ifdef __FAST_USE_GIFTBOX__
		if (bSuccess)
			wCount--;
#endif
	}

#ifdef __FAST_USE_GIFTBOX__
	wCount = iCount - wCount;
	bSuccess = true;
#endif
	return bSuccess;
}

bool CHARACTER::ItemProcess_Hair(LPITEM item, int32_t iDestCell)
{
	if (item->CheckItemUseLevel(GetLevel()) == false)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¾ÆÁ÷ ÀÌ ¸Ó¸®¸¦ »ç¿ëÇÒ ¼ö ¾ø´Â ·¹º§ÀÔ´Ï´Ù."));
		return false;
	}

	uint32_t hair = item->GetVnum();

	switch (GetJob())
	{
		case JOB_WARRIOR :
			hair -= 72000;
			break;

		case JOB_ASSASSIN :
			hair -= 71250;
			break;

		case JOB_SURA :
			hair -= 70500;
			break;

		case JOB_SHAMAN :
			hair -= 69750;
			break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case JOB_WOLFMAN:
			break;
#endif
		default :
			return false;
			break;
	}

	if (hair == GetPart(PART_HAIR))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µ¿ÀÏÇÑ ¸Ó¸® ½ºÅ¸ÀÏ·Î´Â ±³Ã¼ÇÒ ¼ö ¾ø½À´Ï´Ù."));
		return true;
	}

	item->SetCount(item->GetCount() - 1);

	SetPart(PART_HAIR, hair);
	UpdatePacket();

	return true;
}

bool CHARACTER::ItemProcess_Polymorph(LPITEM item)
{
	if (IsPolymorphed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("ÀÌ¹Ì µÐ°©ÁßÀÎ »óÅÂÀÔ´Ï´Ù."));
		return false;
	}

	if (true == IsRiding())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("µÐ°©ÇÒ ¼ö ¾ø´Â »óÅÂÀÔ´Ï´Ù."));
		return false;
	}

	uint32_t dwVnum = item->GetSocket(0);

	if (dwVnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Àß¸øµÈ µÐ°© ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
		item->SetCount(item->GetCount()-1);
		return false;
	}

	const CMob* pMob = CMobManager::instance().Get(dwVnum);

	if (pMob == nullptr)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Àß¸øµÈ µÐ°© ¾ÆÀÌÅÛÀÔ´Ï´Ù."));
		item->SetCount(item->GetCount()-1);
		return false;
	}

	switch (item->GetVnum())
	{
		case 70104 :
		case 70105 :
		case 70106 :
		case 70107 :
		case 71093 :
			{
				sys_log(0, "USE_POLYMORPH_BALL PID(%d) vnum(%d)", GetPlayerID(), dwVnum);

				int32_t iPolymorphLevelLimit = MAX(0, 20 - GetLevel() * 3 / 10);
				if (pMob->m_table.bLevel >= GetLevel() + iPolymorphLevelLimit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("³ªº¸´Ù ³Ê¹« ³ôÀº ·¹º§ÀÇ ¸ó½ºÅÍ·Î´Â º¯½Å ÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}

				int32_t iDuration = GetSkillLevel(POLYMORPH_SKILL_ID) == 0 ? 5 : (5 + (5 + GetSkillLevel(POLYMORPH_SKILL_ID)/40 * 25));
				iDuration *= 60;

				uint32_t dwBonus = 0;

				// @correction005
				dwBonus = (2 + GetSkillLevel(POLYMORPH_SKILL_ID)/40) * 100;

				AddAffect(AFFECT_POLYMORPH, POINT_POLYMORPH, dwVnum, AFF_POLYMORPH, iDuration, 0, true);
				AddAffect(AFFECT_POLYMORPH, POINT_ATT_BONUS, dwBonus, AFF_POLYMORPH, iDuration, 0, false);

				item->SetCount(item->GetCount()-1);
			}
			break;

		case 50322:
			{

				sys_log(0, "USE_POLYMORPH_BOOK: %s(%u) vnum(%u)", GetName(), GetPlayerID(), dwVnum);

				if (CPolymorphUtils::instance().PolymorphCharacter(this, item, pMob) == true)
				{
					CPolymorphUtils::instance().UpdateBookPracticeGrade(this, item);
				}
				else
				{
				}
			}
			break;

		default :
			sys_err("POLYMORPH invalid item passed PID(%d) vnum(%d)", GetPlayerID(), item->GetOriginalVnum());
			return false;
	}

	return true;
}

bool CHARACTER::CanDoCube() const
{
	if (m_bIsObserver)	return false;
	if (GetShop())		return false;
	if (GetMyShop())	return false;
	if (m_bUnderRefine)	return false;
	if (IsWarping())	return false;

	return true;
}

bool CHARACTER::UnEquipSpecialRideUniqueItem()
{
	LPITEM Unique1 = GetWear(WEAR_UNIQUE1);
	LPITEM Unique2 = GetWear(WEAR_UNIQUE2);
#if defined(__COSTUME_MOUNT__) && !defined(__MOUNT__)
	LPITEM Unique3 = GetWear(WEAR_COSTUME_MOUNT);
#endif

	if( nullptr != Unique1 )
	{
		if( UNIQUE_GROUP_SPECIAL_RIDE == Unique1->GetSpecialGroup() )
		{
			return UnequipItem(Unique1);
		}
	}

	if( nullptr != Unique2 )
	{
		if( UNIQUE_GROUP_SPECIAL_RIDE == Unique2->GetSpecialGroup() )
		{
			return UnequipItem(Unique2);
		}
	}

#if defined(__COSTUME_MOUNT__) && !defined(__MOUNT__)
	if (nullptr != Unique3)
		if (UNIQUE_GROUP_SPECIAL_RIDE == Unique3->GetSpecialGroup())
			return UnequipItem(Unique3);
#endif

	return true;
}

void CHARACTER::AutoRecoveryItemProcess(const EAffectTypes type)
{
	if (true == IsDead() || true == IsStun())
		return;

	if (false == IsPC())
		return;

	if (AFFECT_AUTO_HP_RECOVERY != type && AFFECT_AUTO_SP_RECOVERY != type)
		return;

	if (nullptr != FindAffect(AFFECT_STUN))
		return;

	{
		const uint32_t stunSkills[] = { SKILL_TANHWAN, SKILL_GEOMPUNG, SKILL_BYEURAK, SKILL_GIGUNG };

		for (size_t i=0 ; i < sizeof(stunSkills)/sizeof(uint32_t) ; ++i)
		{
			const CAffect* p = FindAffect(stunSkills[i]);

			if (nullptr != p && AFF_STUN == p->dwFlag)
				return;
		}
	}

	const CAffect* pAffect = FindAffect(type);

	if (nullptr != pAffect)
	{
		LPITEM pItem = FindItemByID(pAffect->dwFlag);

		if (nullptr != pItem && true == pItem->GetSocket(0))
		{
			if (!CArenaManager::instance().IsArenaMap(GetMapIndex())
// @correction230 BEGIN
				&& !(g_NoPotionsOnPVP && CPVPManager::instance().IsFighting(GetPlayerID()) && !IsAllowedPotionOnPVP(pItem->GetVnum()))
// @correction230 END
			)
			{
				int32_t amount = 0;

				if (AFFECT_AUTO_HP_RECOVERY == type)
				{
					amount = GetMaxHP() - (GetHP() + GetPoint(POINT_HP_RECOVERY));
				}
				else if (AFFECT_AUTO_SP_RECOVERY == type)
				{
					amount = GetMaxSP() - (GetSP() + GetPoint(POINT_SP_RECOVERY));
				}

				if (amount > 0)
				{
// @correction136 BEGIN
					/*
					const size_t idx_of_amount_of_used = 1;
					const size_t idx_of_amount_of_full = 2;
					const int32_t amount_of_used = pItem->GetSocket(idx_of_amount_of_used);
					const int32_t amount_of_full = pItem->GetSocket(idx_of_amount_of_full);
					const int32_t avail = amount_of_full - amount_of_used;
					if (avail > amount)
					{
						const int32_t pct_of_used = amount_of_used * 100 / amount_of_full;
						const int32_t pct_of_will_used = (amount_of_used + amount) * 100 / amount_of_full;

						bool bLog = false;
						if ((pct_of_will_used / 10) - (pct_of_used / 10) >= 1)
							bLog = true;
						pItem->SetSocket(idx_of_amount_of_used, amount_of_used + amount, bLog);
					}
					else
					{
						amount = avail;

						ITEM_MANAGER::instance().RemoveItem( pItem );
					}
					*/
// @correction136 END

					if (AFFECT_AUTO_HP_RECOVERY == type)
					{
						PointChange( POINT_HP_RECOVERY, amount );
						EffectPacket( SE_AUTO_HPUP );
					}
					else if (AFFECT_AUTO_SP_RECOVERY == type)
					{
						PointChange( POINT_SP_RECOVERY, amount );
						EffectPacket( SE_AUTO_SPUP );
					}
				}
			}
			else
			{
				pItem->Lock(false);
				pItem->SetSocket(0, false);
				RemoveAffect( const_cast<CAffect*>(pAffect) );
			}
		}
		else
		{
			RemoveAffect( const_cast<CAffect*>(pAffect) );
		}
	}
}

bool CHARACTER::IsValidItemPosition(TItemPos Pos) const
{
	uint8_t window_type = Pos.window_type;
	uint16_t cell = Pos.cell;

	switch (window_type)
	{
	case RESERVED_WINDOW:
		return false;

	case INVENTORY:
	case EQUIPMENT:
		return cell < (INVENTORY_AND_EQUIP_SLOT_MAX);

	case DRAGON_SOUL_INVENTORY:
		return cell < (DRAGON_SOUL_INVENTORY_MAX_NUM);

	case SAFEBOX:
		if (nullptr != m_pkSafebox)
			return m_pkSafebox->IsValidPosition(cell);
		else
			return false;

	case MALL:
		if (nullptr != m_pkMall)
			return m_pkMall->IsValidPosition(cell);
		else
			return false;

#ifdef __SWITCHBOT__
	case SWITCHBOT:
		return cell < SWITCHBOT_SLOT_COUNT;
#endif
	default:
		return false;
	}
}


#define VERIFY_MSG(exp, msg)  \
	if (true == (exp)) { \
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT(msg)); \
			return false; \
	}

bool CHARACTER::CanEquipNow(const LPITEM item, const TItemPos& srcCell, const TItemPos& destCell)
{
	const TItemTable* itemTable = item->GetProto();

	switch (GetJob())
	{
		case JOB_WARRIOR:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR)
				return false;
			break;

		case JOB_ASSASSIN:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN)
				return false;
			break;

		case JOB_SHAMAN:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
				return false;
			break;

		case JOB_SURA:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_SURA)
				return false;
			break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case JOB_WOLFMAN:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN)
				return false;
			break;
#endif
	}

	for (int32_t i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		int32_t limit = itemTable->aLimits[i].lValue;
		switch (itemTable->aLimits[i].bType)
		{
			case LIMIT_LEVEL:
				if (GetLevel() < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("·¹º§ÀÌ ³·¾Æ Âø¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
				break;

			case LIMIT_STR:
				if (GetPoint(POINT_ST) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("±Ù·ÂÀÌ ³·¾Æ Âø¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
				break;

			case LIMIT_INT:
				if (GetPoint(POINT_IQ) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Áö´ÉÀÌ ³·¾Æ Âø¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
				break;

			case LIMIT_DEX:
				if (GetPoint(POINT_DX) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("¹ÎÃ¸ÀÌ ³·¾Æ Âø¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
				break;

			case LIMIT_CON:
				if (GetPoint(POINT_HT) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Ã¼·ÂÀÌ ³·¾Æ Âø¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
					return false;
				}
				break;
		}
	}

	if (item->GetWearFlag() & WEARABLE_UNIQUE)
	{
		if ((GetWear(WEAR_UNIQUE1) && GetWear(WEAR_UNIQUE1)->IsSameSpecialGroup(item)) ||
			(GetWear(WEAR_UNIQUE2) && GetWear(WEAR_UNIQUE2)->IsSameSpecialGroup(item))
#ifdef __COSTUME_MOUNT__
|| (GetWear(WEAR_COSTUME_MOUNT) && GetWear(WEAR_COSTUME_MOUNT)->IsSameSpecialGroup(item))
#endif
)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°°Àº Á¾·ùÀÇ À¯´ÏÅ© ¾ÆÀÌÅÛ µÎ °³¸¦ µ¿½Ã¿¡ ÀåÂøÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}

		if (marriage::CManager::instance().IsMarriageUniqueItem(item->GetVnum()) &&
			!marriage::CManager::instance().IsMarried(GetPlayerID()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("°áÈ¥ÇÏÁö ¾ÊÀº »óÅÂ¿¡¼­ ¿¹¹°À» Âø¿ëÇÒ ¼ö ¾ø½À´Ï´Ù."));
			return false;
		}

	}


#ifdef __COSTUME_WEAPON__ // @correction204
	if (item->GetType() == ITEM_WEAPON && item->GetSubType() != WEAPON_ARROW)
	{
		LPITEM pkItem = GetWear(WEAR_COSTUME_WEAPON);
		if (pkItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("For can do this unwear the costume weapon."));
			return false;
		}
	}
	else if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_WEAPON)
	{
		LPITEM pkItem = GetWear(WEAR_WEAPON);
		if (!pkItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't wear a costume weapon without have a weapon weared."));
			return false;
		}
		else if (item->GetValue(3) != pkItem->GetSubType())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You can't wear a costume weapon who has different type of your weapon."));
			return false;
		}
        else if (pkItem->GetType() == ITEM_ROD || pkItem->GetType() == ITEM_PICK)
            return false;
	}
	
	if (item->GetType() == ITEM_ROD || item->GetType() == ITEM_PICK)
	{
		LPITEM pkItem = GetWear(WEAR_COSTUME_WEAPON);
		if (pkItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("For can do this unwear the costume weapon."));
			return false;
		}
	}
#endif

#ifdef __COSTUME_EFFECT__
#ifdef __QUIVER__ // @correction203
	if (item->GetType() == ITEM_WEAPON && item->GetSubType() != WEAPON_ARROW && item->GetSubType() != WEAPON_QUIVER)
#else
	if (item->GetType() == ITEM_WEAPON && item->GetSubType() != WEAPON_ARROW)
#endif
	{
		LPITEM pkItem = GetWear(WEAR_COSTUME_EFFECT_WEAPON);
		if (pkItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You must unwear the weapon effect before."));
			return false;
		}
	}
	else if (item->GetType() == ITEM_ARMOR && item->GetSubType() == ARMOR_BODY)
	{
		LPITEM pkItem = GetWear(WEAR_COSTUME_EFFECT_BODY);
		if (pkItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("You must unwear the armor effect before."));
			return false;
		}
	}
	else if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_EFFECT)
	{
		if (item->GetWearFlag() & WEARABLE_COSTUME_EFFECT_ARMOR)
		{
			LPITEM pkItem = GetWear(WEAR_BODY);
			if (!pkItem)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("To can wear a armor effect you must have a armor weared."));
				return false;
			}
		}
		else
		{
			LPITEM pkItem = GetWear(WEAR_WEAPON);
			if (!pkItem)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("To can wear a weapon effect you must have a weapon weared."));
				return false;
			}
		}
	}
#endif

	// @correction085 BEGIN
	if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_BODY)
	{
		LPITEM pkItem = GetWear(WEAR_BODY);
		if (pkItem && (pkItem->GetVnum() >= 11901 && pkItem->GetVnum() <= 11914))
			return false;
	}
	// @correction085 END

	return true;
}

bool CHARACTER::CanUnequipNow(const LPITEM item, const TItemPos& srcCell, const TItemPos& destCell)
{
	if (ITEM_BELT == item->GetType())
		VERIFY_MSG(CBeltInventoryHelper::IsExistItemInBeltInventory(this), "º§Æ® ÀÎº¥Åä¸®¿¡ ¾ÆÀÌÅÛÀÌ Á¸ÀçÇÏ¸é ÇØÁ¦ÇÒ ¼ö ¾ø½À´Ï´Ù.");

	if (IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		return false;

	{
		int32_t pos = -1;

		if (item->IsDragonSoul())
			pos = GetEmptyDragonSoulInventory(item);
		else
			pos = GetEmptyInventory(item);

		VERIFY_MSG( -1 == pos, "¼ÒÁöÇ°¿¡ ºó °ø°£ÀÌ ¾ø½À´Ï´Ù." );
	}


#ifdef __COSTUME_WEAPON__ // @correction204
	if (item->GetType() == ITEM_WEAPON && item->GetSubType() != WEAPON_ARROW)
	{
		LPITEM pkItem = GetWear(WEAR_COSTUME_WEAPON);
		if (pkItem)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("For can do this unwear the costume weapon."));
			return false;
		}
	}
#endif

	return true;
}

#ifdef __PENDANT__
LPITEM CHARACTER::FindSpecifyItemTheOutItem(uint32_t vnum, LPITEM item) const
{
	if (!item)
		return nullptr;

	uint8_t bType = ITEM_MANAGER::instance().GetInventoryType(vnum);
	for (int32_t i = INVENTORY_ONE_MAX_NUM * bType; i < GetInventoryMax(bType); ++i)
	for (int32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
		if (GetInventoryItem(i) && GetInventoryItem(i)->GetVnum() == vnum && GetInventoryItem(i)->GetVID() != item->GetVID())
			return GetInventoryItem(i);

	return nullptr;
}

int32_t CHARACTER::CountSpecifyItemTheOutItem(uint32_t vnum, LPITEM item2) const
{
	if (!item2)
		return 0;

	int32_t	count = 0;
	LPITEM item;
	uint8_t bType = ITEM_MANAGER::instance().GetInventoryType(vnum);
	for (int32_t i = INVENTORY_ONE_MAX_NUM * bType; i < GetInventoryMax(bType); ++i)
	// for (int32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		item = GetInventoryItem(i);
		if (nullptr != item && item->GetVnum() == vnum && item->GetVID() != item2->GetVID())
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
				continue;
			else
				count += item->GetCount();
		}
	}

	return count;
}

void CHARACTER::RemoveSpecifyItemTheOutItem(uint32_t vnum, uint32_t count, LPITEM item)
{
	if (0 == count || !item)
		return;

	uint8_t bType = ITEM_MANAGER::instance().GetInventoryType(vnum);
	for (int32_t i = INVENTORY_ONE_MAX_NUM * bType; i < GetInventoryMax(bType); ++i)
	// for (int32_t i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		if (nullptr == GetInventoryItem(i))
			continue;

		if (GetInventoryItem(i)->GetVnum() != vnum)
			continue;

		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (GetInventoryItem(i)->GetVID() == item->GetVID())
			continue;

		if (vnum >= 80003 && vnum <= 80007)
			LogManager::instance().GoldBarLog(GetPlayerID(), GetInventoryItem(i)->GetID(), QUEST, "RemoveSpecifyItem");

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}

	if (count)
		sys_log(0, "CHARACTER::RemoveSpecifyItem cannot remove enough item vnum %u, still remain %d", vnum, count);
}

#endif

