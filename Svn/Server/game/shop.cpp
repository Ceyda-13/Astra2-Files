#include "stdafx.h"
#include "../libsrc/libthecore/grid.h"
#include "constants.h"
#include "utils.h"
#include "config.h"
#include "shop.h"
#include "desc.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "item.h"
#include "item_manager.h"
#include "buffer_manager.h"
#include "packet.h"
#include "log.h"
#include "db.h"
#include "questmanager.h"
#include "monarch.h"
#include "mob_manager.h"
#include "locale_service.h"
#ifdef __OFFLINE_PRIVATE_SHOP__
#include "desc_client.h"
#include "shop_manager.h"
#include "offline_privateshop.h"
#endif

CShop::CShop()
	: m_dwVnum(0), m_dwNPCVnum(0), m_pkPC(nullptr)
#ifdef __OFFLINE_PRIVATE_SHOP__
, m_bIsOfflinePrivateShop(false), m_bUnlockSlots(0)
#endif
{
#ifdef ENABLE_SHOP_TAB // @correction196
	memset(&m_pkGrid, 0, sizeof(m_pkGrid));
	m_itemVector.resize(SHOP_HOST_ITEM_MAX_NUM);
#else
	m_pGrid = M2_NEW CGrid(5, 9);
#endif
}

CShop::~CShop()
{
	TPacketGCShop pack;

	pack.header		= HEADER_GC_SHOP;
	pack.subheader	= SHOP_SUBHEADER_GC_END;
	pack.size		= sizeof(TPacketGCShop);

	Broadcast(&pack, sizeof(pack));

	GuestMapType::iterator it;

	it = m_map_guest.begin();

	while (it != m_map_guest.end())
	{
		LPCHARACTER ch = it->first;
		ch->SetShop(nullptr);
		++it;
	}

#ifndef ENABLE_SHOP_TAB // @correction196
	M2_DELETE(m_pGrid);
#endif

#ifdef __OFFLINE_PRIVATE_SHOP__
	if (m_bIsOfflinePrivateShop)
	{
		for (int32_t i = 0; i < SHOP_HOST_ITEM_MAX_NUM; ++i)
		{
			SHOP_ITEM & item = m_itemVector[i];
			if (!item.pkItem)
				continue;

			item.pkItem->SetSkipSave(true);
			M2_DESTROY_ITEM(item.pkItem);
			item.pkItem = nullptr;
		}
	}
#endif
}

void CShop::SetPCShop(LPCHARACTER ch)
{
	m_pkPC = ch;
}

bool CShop::Create(uint32_t dwVnum, uint32_t dwNPCVnum, TShopItemTable * pTable)
{
	sys_log(0, "SHOP #%d (Shopkeeper %d)", dwVnum, dwNPCVnum);

	m_dwVnum = dwVnum;
	m_dwNPCVnum = dwNPCVnum;

	uint8_t bItemCount;

	for (bItemCount = 0; bItemCount < SHOP_HOST_ITEM_MAX_NUM; ++bItemCount)
		if (0 == (pTable + bItemCount)->vnum)
			break;

	SetShopItems(pTable, bItemCount);
	return true;
}

void CShop::SetShopItems(TShopItemTable * pTable, uint8_t bItemCount
#ifdef __OFFLINE_PRIVATE_SHOP__
, uint32_t dwShopVid
#endif
)
{
	if (bItemCount > SHOP_HOST_ITEM_MAX_NUM)
		return;

#ifndef ENABLE_SHOP_TAB // @correction196
	m_pGrid->Clear();
#endif

	m_itemVector.resize(SHOP_HOST_ITEM_MAX_NUM);

	for (int32_t i = 0; i < bItemCount; ++i)
	{
		LPITEM pkItem = nullptr;
		const TItemTable * item_table;

		if (m_pkPC)
		{
			pkItem = m_pkPC->GetItem(pTable->pos);

			if (!pkItem)
			{
				sys_err("cannot find item on pos (%d, %d) (name: %s)", pTable->pos.window_type, pTable->pos.cell, m_pkPC->GetName());
				continue;
			}

			item_table = pkItem->GetProto();
		}
		else
		{
			if (!pTable->vnum)
				continue;

			item_table = ITEM_MANAGER::instance().GetTable(pTable->vnum);
		}

		if (!item_table)
		{
			sys_err("Shop: no item table by item vnum #%d", pTable->vnum);
			continue;
		}

		int32_t iPos;

		if (IsPCShop())
		{
			sys_log(0, "MyShop: use position %d", pTable->display_pos);
			iPos = pTable->display_pos;
		}
		else
#ifdef ENABLE_SHOP_TAB // @correction196
			iPos = GetEmptyShop(item_table->bSize);
#else
			iPos = m_pGrid->FindBlank(1, item_table->bSize);
#endif

		if (iPos < 0)
		{
			sys_err("not enough shop window");
			continue;
		}

#ifdef ENABLE_SHOP_TAB // @correction196
		if (!IsEmpty(iPos, item_table->bSize))
#else
		if (!m_pGrid->IsEmpty(iPos, 1, item_table->bSize))
#endif
		{
			if (IsPCShop())
			{
				sys_err("not empty position for pc shop %s[%d]", m_pkPC->GetName(), m_pkPC->GetPlayerID());
			}
			else
			{
				sys_err("not empty position for npc shop");
			}
			continue;
		}

#ifdef ENABLE_SHOP_TAB // @correction196
		for (int32_t i = 0; i < item_table->bSize; ++i)
		{
			int32_t p = iPos + (i * SHOP_PAGE_COLUMN);

			if (!IsValidPosition(p))
				continue;

			m_pkGrid[p] = iPos + 1;
		}
#else
		m_pGrid->Put(iPos, 1, item_table->bSize);
#endif

		SHOP_ITEM & item = m_itemVector[iPos];

		item.pkItem = pkItem;
		item.itemid = 0;
#ifdef ENABLE_MULTISHOP // @correction197
		item.item.wPriceVnum = pTable->wPriceVnum;
#endif
		item.item.dwPrices = pTable->price;
		item.item.display_pos = iPos;
		if (pkItem)
		{
			item.item.vnum = pkItem->GetVnum();
			item.item.count = pkItem->GetCount();
			item.itemid	= pkItem->GetID();
#ifdef __CHANGE_LOOK__
			item.item.changelook = pkItem->GetChangeLook();
#endif
			memcpy(item.item.alSockets, pkItem->GetSockets(), sizeof(item.item.alSockets));
			memcpy(item.item.aAttr, pkItem->GetAttributes(), sizeof(item.item.aAttr));
		}
		else
		{
			item.item.vnum = pTable->vnum;
			item.item.count = pTable->count;
		}

#ifdef __OFFLINE_PRIVATE_SHOP__
		if (m_bIsOfflinePrivateShop && pkItem)
		{
			pkItem->SetOfflinePrivateShopPrice(pTable->price);
			pkItem->AddOfflinePrivateShop(iPos, dwShopVid);
		}
#endif

		char name[36 + 64];
		snprintf(name, sizeof(name), "%-20s(#%-5d) (x %d)", item_table->szName, (int32_t) item.item.vnum, item.item.count);

		sys_log(0, "SHOP_ITEM: %-36s PRICE %llu", name, item.item.dwPrices);
		++pTable;
	}
}

int32_t CShop::Buy(LPCHARACTER ch, uint8_t pos)
{
	if (pos >= m_itemVector.size())
	{
		sys_log(0, "Shop::Buy : invalid position %d : %s", pos, ch->GetName());
		return SHOP_SUBHEADER_GC_INVALID_POS;
	}

	sys_log(0, "Shop::Buy : name %s pos %d", ch->GetName(), pos);

	/*GuestMapType::iterator it = m_map_guest.find(ch);

	if (it == m_map_guest.end())
		return SHOP_SUBHEADER_GC_END;*/

	SHOP_ITEM& r_item = m_itemVector[pos];

	if ((int64_t)r_item.item.dwPrices < 0) // @correction143
	{
		LogManager::instance().HackLog("SHOP_BUY_GOLD_OVERFLOW", ch);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}

	LPITEM pkSelectedItem = ITEM_MANAGER::instance().Find(r_item.itemid);

	if (IsPCShop()
#ifdef __OFFLINE_PRIVATE_SHOP__
 && !m_bIsOfflinePrivateShop
#endif
)
	{
		if (!pkSelectedItem)
		{
			sys_log(0, "Shop::Buy : Critical: This user seems to be a hacker : invalid pcshop item : BuyerPID:%d SellerPID:%d",
					ch->GetPlayerID(),
					m_pkPC->GetPlayerID());

			return SHOP_SUBHEADER_GC_SOLD_OUT; // @correction053 false to SHOP_SUBHEADER_GC_SOLD_OUT
		}

		if ((pkSelectedItem->GetOwner() != m_pkPC))
		{
			sys_log(0, "Shop::Buy : Critical: This user seems to be a hacker : invalid pcshop item : BuyerPID:%d SellerPID:%d",
					ch->GetPlayerID(),
					m_pkPC->GetPlayerID());

			return SHOP_SUBHEADER_GC_SOLD_OUT; // @correction053 false to SHOP_SUBHEADER_GC_SOLD_OUT
		}
	}

	uint64_t dwPrice = r_item.item.dwPrices;
#ifdef ENABLE_MULTISHOP // @correction197
	uint32_t dwWItemVnum = r_item.item.wPriceVnum;

	if (dwWItemVnum)
	{
		if (ch->CountSpecifyItem(dwWItemVnum) < (int32_t)dwPrice)
			return SHOP_SUBHEADER_GC_NOT_ENOUGH_ITEM;
	}
	else if (ch->GetGold() < dwPrice)
#else
	if (ch->GetGold() < dwPrice)
#endif
	{
		sys_log(1, "Shop::Buy : Not enough money : %s has %llu, price %llu", ch->GetName(), ch->GetGold(), dwPrice);
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;
	}

	LPITEM item;

	if (m_pkPC
#ifdef __OFFLINE_PRIVATE_SHOP__
 || m_bIsOfflinePrivateShop
#endif
)
		item = r_item.pkItem;
	else
		item = ITEM_MANAGER::instance().CreateItem(r_item.item.vnum, r_item.item.count);

	if (!item)
		return SHOP_SUBHEADER_GC_SOLD_OUT;

#ifdef __12ZI__
	if (!m_pkPC)
	{
		for (uint32_t i = 0; i < 22; ++i)
		{
			std::string flag("zodiac.Insignia");
			flag += std::to_string(i + 1);
			if (ch->GetQuestFlag(flag) == 1 && item->GetVnum() == 33000 + i + 1)
			{
				M2_DESTROY_ITEM(item);
				return SHOP_SUBHEADER_GC_ZODIAC_SHOP;
			}

			if (item->GetVnum() == 33000 + i + 1)
				ch->SetQuestFlag(flag, 1);
		}

		if ((ch->GetQuestFlag("zodiac.Insignia23") == 1 && item->GetVnum() == 33032) || (ch->GetQuestFlag("zodiac.Insignia24") == 1 && item->GetVnum() == 72329))
		{
			M2_DESTROY_ITEM(item);
			return SHOP_SUBHEADER_GC_ZODIAC_SHOP;
		}

		if (item->GetVnum() == 33032)
			ch->SetQuestFlag("zodiac.Insignia23", 1);

		if (item->GetVnum() == 72329)
			ch->SetQuestFlag("zodiac.Insignia24", 1);

		if (item->GetVnum() == 72329 || item->GetVnum() == 33032 || (item->GetVnum() >= 33001 && item->GetVnum() <= 33022))
			ch->ChatPacket(CHAT_TYPE_COMMAND, "RefreshShopItemToolTip %d", item->GetVnum());
	}
#endif

	int32_t iEmptyPos;
	if (item->IsDragonSoul())
	{
		iEmptyPos = ch->GetEmptyDragonSoulInventory(item);
	}
	else
	{
		iEmptyPos = ch->GetEmptyInventory(item);
	}

	if (iEmptyPos < 0)
	{
		if (m_pkPC
#ifdef __OFFLINE_PRIVATE_SHOP__
 || m_bIsOfflinePrivateShop
#endif
)
		{
			sys_log(1, "Shop::Buy at PC Shop : Inventory full : %s size %d", ch->GetName(), item->GetSize());
			return SHOP_SUBHEADER_GC_INVENTORY_FULL;
		}
		else
		{
			sys_log(1, "Shop::Buy : Inventory full : %s size %d", ch->GetName(), item->GetSize());
			M2_DESTROY_ITEM(item);
			return SHOP_SUBHEADER_GC_INVENTORY_FULL;
		}
	}

#ifdef __OFFLINE_PRIVATE_SHOP__
	if (m_bIsOfflinePrivateShop)
	{
		if (item->GetLock())
			return SHOP_SUBHEADER_GC_SOLD_OUT;

		item->SetLock(1, true);
	}
#endif

#ifdef ENABLE_MULTISHOP // @correction197
	if (dwWItemVnum)
		ch->RemoveSpecifyItem(dwWItemVnum, dwPrice);
	else
		ch->ChangeGold(false, dwPrice, false);
#else
	ch->ChangeGold(false, dwPrice, false);
#endif

	for (int32_t i = 0; i < item->GetSize(); ++i)
	{
		int32_t p = pos + (i * SHOP_PAGE_COLUMN);

		if (!IsValidPosition(p))
			continue;

		m_pkGrid[p] = 0;
	}

#ifdef __OFFLINE_PRIVATE_SHOP__
	if (m_bIsOfflinePrivateShop)
	{
		item->SetCell(nullptr, iEmptyPos);
		if (item->IsDragonSoul())
			item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
		else
			item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));

		CShopManager::instance().AddBankOfflinePrivateShop(this, r_item.item.dwPrices);
		auto* pShop = CShopManager::instance().FindOfflinePrivateShop(item->GetOfflinePrivateShopVid());
		if (pShop)
		{
			uint32_t dwPID = pShop->GetPlayerID();
			char queryStr[256];
			snprintf(queryStr, sizeof(queryStr), "INSERT INTO player.player_shop_logs VALUES(%u, %llu, %u, FROM_UNIXTIME(%u))", dwPID, r_item.item.dwPrices, r_item.item.vnum, static_cast<uint32_t>(time(nullptr)));
			std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(queryStr));
		}
		item->SetOfflinePrivateShopPrice(0);
		item->SetOfflinePrivateShopVid(0);
		item->SetLock(0);
		item->SaveAndFlush();
		memset((void*)&r_item, 0, sizeof(r_item));
		r_item.pkItem = nullptr;
		BroadcastUpdateItem(pos);
	}
	else if (m_pkPC)
#else
	if (m_pkPC)
#endif
	{
		m_pkPC->SyncQuickslot(QUICKSLOT_TYPE_ITEM, item->GetCell(), 255);

		if (item->GetVnum() == 90008 || item->GetVnum() == 90009)
		{
			VCardUse(m_pkPC, ch, item);
			item = nullptr;
		}
		else
		{
			char buf[512];

			if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
			{
				snprintf(buf, sizeof(buf), "%s FROM: %u TO: %u PRICE: %llu", item->GetName(), ch->GetPlayerID(), m_pkPC->GetPlayerID(), dwPrice);
				LogManager::instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), SHOP_BUY, buf);
				LogManager::instance().GoldBarLog(m_pkPC->GetPlayerID(), item->GetID(), SHOP_SELL, buf);
			}

			item->RemoveFromCharacter();
			if (item->IsDragonSoul())
				item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
			else
				item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
			ITEM_MANAGER::instance().FlushDelayedSave(item);


			snprintf(buf, sizeof(buf), "%s %u(%s) %llu %u", item->GetName(), m_pkPC->GetPlayerID(), m_pkPC->GetName(), dwPrice, item->GetCount());
			LogManager::instance().ItemLog(ch, item, "SHOP_BUY", buf);

			snprintf(buf, sizeof(buf), "%s %u(%s) %llu %u", item->GetName(), ch->GetPlayerID(), ch->GetName(), dwPrice, item->GetCount());
			LogManager::instance().ItemLog(m_pkPC, item, "SHOP_SELL", buf);
		}

		memset((void*)&r_item, 0, sizeof(r_item));
		r_item.pkItem = nullptr;
		BroadcastUpdateItem(pos);

		if (dwPrice)
			m_pkPC->ChangeGold(true, dwPrice, false);
	}
	else
	{
		if (item->IsDragonSoul())
			item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
		else
			item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
		ITEM_MANAGER::instance().FlushDelayedSave(item);
		LogManager::instance().ItemLog(ch, item, "BUY", item->GetName());

		if (item->GetVnum() >= 80003 && item->GetVnum() <= 80007)
		{
			LogManager::instance().GoldBarLog(ch->GetPlayerID(), item->GetID(), PERSONAL_SHOP_BUY, "");
		}

		DBManager::instance().SendMoneyLog(MONEY_LOG_SHOP, item->GetVnum(), -dwPrice);
	}

	if (item)
		sys_log(0, "SHOP: BUY: name %s %s(x %d):%u price %llu", ch->GetName(), item->GetName(), item->GetCount(), item->GetID(), dwPrice);

	ch->Save();

	return (SHOP_SUBHEADER_GC_OK);
}

bool CShop::AddGuest(LPCHARACTER ch, uint32_t owner_vid, bool bOtherEmpire)
{
	if (!ch)
		return false;

	if (ch->GetExchange())
		return false;

	if (ch->GetShop())
		return false;

	ch->SetShop(this);

	m_map_guest.insert(GuestMapType::value_type(ch, bOtherEmpire));

	TPacketGCShop pack;

	pack.header		= HEADER_GC_SHOP;
	pack.subheader	= SHOP_SUBHEADER_GC_START;

	TPacketGCShopStart pack2;

	memset(&pack2, 0, sizeof(pack2));
	pack2.owner_vid = owner_vid;

	for (uint32_t i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		const SHOP_ITEM & item = m_itemVector[i];
		pack2.items[i] = item.item;
	}
#ifdef __OFFLINE_PRIVATE_SHOP__
	if (m_bIsOfflinePrivateShop)
		pack2.bUnlockSlots = m_bUnlockSlots;
	else
		pack2.bUnlockSlots = 135;
#endif

	pack.size = sizeof(pack) + sizeof(pack2);

	ch->GetDesc()->BufferedPacket(&pack, sizeof(TPacketGCShop));
	ch->GetDesc()->Packet(&pack2, sizeof(TPacketGCShopStart));
	return true;
}

void CShop::RemoveGuest(LPCHARACTER ch)
{
	if (ch->GetShop() != this)
		return;

	m_map_guest.erase(ch);
	ch->SetShop(nullptr);

	TPacketGCShop pack;

	pack.header		= HEADER_GC_SHOP;
	pack.subheader	= SHOP_SUBHEADER_GC_END;
	pack.size		= sizeof(TPacketGCShop);

	ch->GetDesc()->Packet(&pack, sizeof(pack));
}

void CShop::Broadcast(const void * data, int32_t bytes)
{
	sys_log(1, "Shop::Broadcast %p %d", data, bytes);

	GuestMapType::iterator it;

	it = m_map_guest.begin();

	while (it != m_map_guest.end())
	{
		LPCHARACTER ch = it->first;

		if (ch->GetDesc())
			ch->GetDesc()->Packet(data, bytes);

		++it;
	}
}

void CShop::BroadcastUpdateItem(uint8_t pos)
{
	TPacketGCShop pack;
	TPacketGCShopUpdateItem pack2;

	TEMP_BUFFER	buf;

	pack.header = HEADER_GC_SHOP;
	pack.subheader = SHOP_SUBHEADER_GC_UPDATE_ITEM;
	pack.size = sizeof(pack) + sizeof(pack2);

	pack2.pos = pos;
	pack2.item = m_itemVector[pos].item;

	buf.write(&pack, sizeof(pack));
	buf.write(&pack2, sizeof(pack2));

	Broadcast(buf.read_peek(), buf.size());
}

int32_t CShop::GetNumberByVnum(uint32_t dwVnum)
{
	int32_t itemNumber = 0;

	for (uint32_t i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		const SHOP_ITEM & item = m_itemVector[i];

		if (item.item.vnum == dwVnum)
		{
			itemNumber += item.item.count;
		}
	}

	return itemNumber;
}

bool CShop::IsSellingItem(uint32_t itemID)
{
	bool isSelling = false;

	for (uint32_t i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		if ((uint32_t)(m_itemVector[i].itemid) == itemID)
		{
			isSelling = true;
			break;
		}
	}

	return isSelling;

}

// @correction080 BEGIN
void CShop::RemoveItem(LPITEM pkItem)
{
	if (!pkItem)
		return;

	for (size_t i = 0; i < SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		SHOP_ITEM & item = m_itemVector[i];
		if (item.pkItem && (uint32_t)item.itemid == pkItem->GetID())
		{
			memset((void*)&item, 0, sizeof(item));
			BroadcastUpdateItem(i);
			break;
		}
	}
}
// @correction080 END

#ifdef ENABLE_SHOP_TAB // @correction196
bool CShop::IsValidPosition(uint32_t dwPos)
{
	if (dwPos >= SHOP_HOST_ITEM_MAX_NUM)
		return false;

	return true;
}

int32_t CShop::GetEmptyShop(uint32_t dwSize)
{
	for (int32_t i = 0; i < SHOP_HOST_ITEM_MAX_NUM; ++i)
		if (IsEmpty(i, dwSize))
			return i;

	return -1;
}

int32_t CShop::GetEmptyShopForUnlocks(uint32_t dwSize)
{
	for (int32_t i = 0; i < SHOP_HOST_ITEM_MAX_NUM; ++i)
		if (i % 10 < 5)
			if (IsEmpty(i, dwSize))
				return i;

	return -1;
}

bool CShop::IsEmpty(uint16_t wCell, uint8_t bSize, int32_t iExceptionCell)
{
	if (!IsValidPosition(wCell))
		return false;

	iExceptionCell++;

	if (m_pkGrid[wCell])
	{
		if (m_pkGrid[wCell] == iExceptionCell)
		{
			if (bSize == 1)
				return true;

			int32_t j = 1;
			uint8_t bPage = wCell / SHOP_PAGE_SIZE;

			do
			{
				int32_t p = wCell + (SHOP_PAGE_COLUMN * j);

				if (!IsValidPosition(p))
					return false;

				if (p / SHOP_PAGE_SIZE != bPage)
					return false;

				if (m_pkGrid[p])
					if (m_pkGrid[p] != iExceptionCell)
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
		uint8_t bPage = wCell / SHOP_PAGE_SIZE;

		do
		{
			int32_t p = wCell + (SHOP_PAGE_COLUMN * j);

			if (!IsValidPosition(p))
				return false;

			if (p / SHOP_PAGE_SIZE != bPage)
				return false;

			if (m_pkGrid[p])
				if (m_pkGrid[p] != iExceptionCell)
					return false;
		}
		while (++j < bSize);

		return true;
	}

	return false;
}
#endif

#ifdef __OFFLINE_PRIVATE_SHOP__
void CShop::SetOfflinePrivateShopItem(LPITEM pkItem)
{
	if (!pkItem)
		sys_err("SetOfflinePrivateShopItem Not Found Item !");

	int32_t iPos = pkItem->GetCell();

	if (iPos < 0)
	{
		sys_err("not enough shop window");
		return;
	}

	if (!IsEmpty(iPos, pkItem->GetSize()))
	{
		int iPos = GetEmptyShop(pkItem->GetSize());
		if (iPos == -1)
		{
			sys_err("not empty position for npc shop");
			return;
		}
		// else
			// sys_err("Buldu: %d", iPos);
	}

	for (int32_t i = 0; i < pkItem->GetSize(); ++i)
	{
		int32_t p = iPos + (i * SHOP_PAGE_COLUMN);

		if (!IsValidPosition(p))
			continue;

		m_pkGrid[p] = iPos + 1;
	}

	SHOP_ITEM & item = m_itemVector[iPos];

	item.pkItem = pkItem;

	item.item.vnum = pkItem->GetVnum();
	item.item.count = pkItem->GetCount();
	item.item.dwPrices = pkItem->GetOfflinePrivateShopPrices();
	item.item.display_pos = iPos;
	item.itemid	= pkItem->GetID();
#ifdef __CHANGE_LOOK__
	item.item.changelook = pkItem->GetChangeLook();
#endif
	memcpy(item.item.alSockets, pkItem->GetSockets(), sizeof(item.item.alSockets));
	memcpy(item.item.aAttr, pkItem->GetAttributes(), sizeof(item.item.aAttr));
}

void CShop::SetOfflinePrivateShopInfo(TOfflinePrivateShopInfo * pkTOfflinePrivateShopInfo)
{
	for (uint32_t i = 0; i < m_itemVector.size() && i < SHOP_HOST_ITEM_MAX_NUM; ++i)
	{
		const SHOP_ITEM & item = m_itemVector[i];

		pkTOfflinePrivateShopInfo->items[i] = item.item;
	}
}

uint32_t CShop::DelOfflinePrivateShopItem(uint32_t dwPos, LPCHARACTER ch, TShopItemData * pData)
{
	SHOP_ITEM & r_item = m_itemVector[dwPos];
	LPITEM item = r_item.pkItem;
	if (!item)
		return 0;

	int32_t iEmptyPos;
	if (ch)
	{
		if (item->IsDragonSoul())
			iEmptyPos = ch->GetEmptyDragonSoulInventory(item);
		else
			iEmptyPos = ch->GetEmptyInventory(item);

		if (iEmptyPos < 0)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, "Envanterinde Yer Yok !");
			return 0;
		}
	}
	if (item->GetLock())
		return 0;

	item->SetLock(1);
	// item->SaveAndFlush();
	if (ch)
	{
		item->SetCell(nullptr, iEmptyPos);
		if (item->IsDragonSoul())
			item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
		else
			item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
		item->SetOfflinePrivateShopVid(0);
		item->SetOfflinePrivateShopPrice(0);
		item->SetLock(0);
		item->SaveAndFlush();
	}

	for (int32_t i = 0; i < item->GetSize(); ++i)
	{
		int32_t p = dwPos + (i * SHOP_PAGE_COLUMN);

		if (!IsValidPosition(p))
			continue;

		m_pkGrid[p] = 0;
	}

	if (!ch && pData)
		memcpy(pData, &r_item.item, sizeof(*pData));

	memset((void*)&r_item, 0, sizeof(r_item));
	r_item.pkItem = nullptr;
	BroadcastUpdateItem(dwPos);
	uint32_t dwId = item->GetID();
	if (!ch)
	{
		item->SetSkipSave(false);
		M2_DESTROY_ITEM(item);
	}
	return dwId;
}

void CShop::AddOfflinePrivateShopItem(uint32_t dwPos, uint8_t bInvenType, uint32_t dwSourcePos, uint64_t dwPrices, uint32_t dwShopVid, LPCHARACTER ch)
{
	if (!ch)
		return;

	LPITEM item = ch->GetItem(TItemPos(bInvenType, dwSourcePos));
	if (!item)
		return;

#ifdef ENABLE_SHOP_TAB // @correction196
	if (!IsEmpty(dwPos, item->GetSize()))
#else
	if (!m_pGrid->IsEmpty(dwPos, 1, item->GetSize()))
#endif
	{
		sys_err("not empty position for npc shop");
		return;
	}

#ifdef ENABLE_SHOP_TAB // @correction196
	for (int32_t i = 0; i < item->GetSize(); ++i)
	{
		int32_t p = dwPos + (i * SHOP_PAGE_COLUMN);

		if (!IsValidPosition(p))
			continue;

		m_pkGrid[p] = dwPos + 1;
	}
#else
	m_pGrid->Put(dwPos, 1, item->bSize);
#endif

	item->SetLock(1, true);
	item->Lock(true);

	SHOP_ITEM & r_item = m_itemVector[dwPos];

	r_item.pkItem = item;

	r_item.item.vnum = item->GetVnum();
	r_item.item.count = item->GetCount();
	r_item.item.dwPrices = dwPrices;
	r_item.itemid	= item->GetID();
	r_item.item.display_pos = dwPos;
#ifdef __CHANGE_LOOK__
	r_item.item.changelook = item->GetChangeLook();
#endif
	memcpy(r_item.item.alSockets, item->GetSockets(), sizeof(r_item.item.alSockets));
	memcpy(r_item.item.aAttr, item->GetAttributes(), sizeof(r_item.item.aAttr));

	item->SetOfflinePrivateShopPrice(dwPrices);
	item->SetLock(0);
	item->Lock(false);
	item->AddOfflinePrivateShop(dwPos, dwShopVid);
	BroadcastUpdateItem(dwPos);
	ch->SaveAndFlush();
}

int32_t CShop::RemoteBuy(TShopItemData * pData, uint64_t dwPrices, uint32_t pos)
{
	if (pos >= m_itemVector.size())
		return SHOP_SUBHEADER_GC_INVALID_POS;

	SHOP_ITEM& r_item = m_itemVector[pos];

	if ((int64_t)r_item.item.dwPrices < 0) // @correction143
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;

	uint64_t dwPrice = r_item.item.dwPrices;
	if (dwPrices < dwPrice)
		return SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY;

	LPITEM item = r_item.pkItem;
	if (!item)
		return SHOP_SUBHEADER_GC_SOLD_OUT;

	if (m_bIsOfflinePrivateShop)
	{
		if (item->GetLock())
			return SHOP_SUBHEADER_GC_SOLD_OUT;

		item->SetLock(1, true);
	}

	for (int32_t i = 0; i < item->GetSize(); ++i)
	{
		int32_t p = pos + (i * SHOP_PAGE_COLUMN);

		if (!IsValidPosition(p))
			continue;

		m_pkGrid[p] = 0;
	}

	if (m_bIsOfflinePrivateShop)
	{
		auto* pShop = CShopManager::instance().FindOfflinePrivateShop(item->GetOfflinePrivateShopVid());
		if (pShop)
		{
			uint32_t dwPID = pShop->GetPlayerID();
			char queryStr[256];
			snprintf(queryStr, sizeof(queryStr), "INSERT INTO player.player_shop_logs VALUES(%u, %llu, %u, FROM_UNIXTIME(%u))", dwPID, r_item.item.dwPrices, r_item.item.vnum, static_cast<uint32_t>(time(nullptr)));
			std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(queryStr));
		}
		item->SetSkipSave(false);
		M2_DESTROY_ITEM(item);
		CShopManager::instance().AddBankOfflinePrivateShop(this, r_item.item.dwPrices);
		memcpy(pData, &r_item.item, sizeof(*pData));
		memset((void*)&r_item, 0, sizeof(r_item));
		r_item.pkItem = nullptr;
		BroadcastUpdateItem(pos);
	}

	return (SHOP_SUBHEADER_GC_OK);
}
#endif

