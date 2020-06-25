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
#include "desc_client.h"
#include "shop_manager.h"
#include "group_text_parse_tree.h"
#include "shopEx.h"
#include <boost/algorithm/string/predicate.hpp>
#include "shop_manager.h"
#include <cctype>
#ifdef __OFFLINE_PRIVATE_SHOP__
#include "offline_privateshop.h"
#include "sectree_manager.h"
#endif

CShopManager::CShopManager()
{
}

CShopManager::~CShopManager()
{
	Destroy();
}

bool CShopManager::Initialize(TShopTable * table, int32_t size)
{
	if (!m_map_pkShop.empty())
		return false;

	int32_t i;

	for (i = 0; i < size; ++i, ++table)
	{
		LPSHOP shop = M2_NEW CShop;

		if (!shop->Create(table->dwVnum, table->dwNPCVnum, table->items))
		{
			M2_DELETE(shop);
			continue;
		}

		m_map_pkShop.insert(TShopMap::value_type(table->dwVnum, shop));
		m_map_pkShopByNPCVnum.insert(TShopMap::value_type(table->dwNPCVnum, shop));
	}
	char szShopTableExFileName[256];

	snprintf(szShopTableExFileName, sizeof(szShopTableExFileName),
		"%s/shop_table_ex.txt", LocaleService_GetBasePath().c_str());

	return ReadShopTableEx(szShopTableExFileName);
}

void CShopManager::Destroy()
{
	TShopMap::iterator it = m_map_pkShop.begin();

	while (it != m_map_pkShop.end())
	{
		M2_DELETE(it->second);
		++it;
	}

	m_map_pkShop.clear();

#ifdef __OFFLINE_PRIVATE_SHOP__
	TOfflinePrivateShopMap::iterator it2;

	for (it2 = m_map_pkPrivateOfflineShop.begin(); it2 != m_map_pkPrivateOfflineShop.end(); ++it2)
	{
		it2->second->Destroy();
		M2_DELETE(it2->second);
	}

	m_map_pkPrivateOfflineShop.clear();
	vec_check.clear();
#endif
}

LPSHOP CShopManager::Get(uint32_t dwVnum)
{
	TShopMap::const_iterator it = m_map_pkShop.find(dwVnum);

	if (it == m_map_pkShop.end())
		return nullptr;

	return (it->second);
}

LPSHOP CShopManager::GetByNPCVnum(uint32_t dwVnum)
{
	TShopMap::const_iterator it = m_map_pkShopByNPCVnum.find(dwVnum);

	if (it == m_map_pkShopByNPCVnum.end())
		return nullptr;

	return (it->second);
}


bool CShopManager::StartShopping(LPCHARACTER pkChr, LPCHARACTER pkChrShopKeeper, int32_t iShopVnum)
{
	if (pkChr->GetShopOwner() == pkChrShopKeeper)
		return false;
	if (pkChrShopKeeper->IsPC())
		return false;

	if (pkChr->IsOpenSafebox() || pkChr->GetExchange() || pkChr->GetMyShop() || pkChr->IsCubeOpen())
	{
		pkChr->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래창이 열린상태에서는 상점거래를 할수 가 없습니다."));
		return false;
	}

	/*int32_t distance = DISTANCE_APPROX(pkChr->GetX() - pkChrShopKeeper->GetX(), pkChr->GetY() - pkChrShopKeeper->GetY());

	if (distance >= SHOP_MAX_DISTANCE)
	{
		sys_log(1, "SHOP: TOO_FAR: %s distance %d", pkChr->GetName(), distance);
		return false;
	}*/

	LPSHOP pkShop;

	if (iShopVnum)
		pkShop = Get(iShopVnum);
	else
		pkShop = GetByNPCVnum(pkChrShopKeeper->GetRaceNum());

	if (!pkShop)
	{
		sys_log(1, "SHOP: NO SHOP");
		return false;
	}

	bool bOtherEmpire = false;

	if (pkChr->GetEmpire() != pkChrShopKeeper->GetEmpire())
		bOtherEmpire = true;

	pkShop->AddGuest(pkChr, pkChrShopKeeper->GetVID(), bOtherEmpire);
	pkChr->SetShopOwner(pkChrShopKeeper);
	sys_log(0, "SHOP: START: %s", pkChr->GetName());
	return true;
}

LPSHOP CShopManager::FindPCShop(uint32_t dwVID)
{
	TShopMap::iterator it = m_map_pkShopByPC.find(dwVID);

	if (it == m_map_pkShopByPC.end())
		return nullptr;

	return it->second;
}

LPSHOP CShopManager::CreatePCShop(LPCHARACTER ch, TShopItemTable * pTable, uint8_t bItemCount)
{
	if (FindPCShop(ch->GetVID()))
		return nullptr;

	LPSHOP pkShop = M2_NEW CShop;
	pkShop->SetPCShop(ch);
	pkShop->SetShopItems(pTable, bItemCount);

	m_map_pkShopByPC.insert(TShopMap::value_type(ch->GetVID(), pkShop));
	return pkShop;
}

void CShopManager::DestroyPCShop(LPCHARACTER ch)
{
	LPSHOP pkShop = FindPCShop(ch->GetVID());

	if (!pkShop)
		return;

	ch->SetMyShopTime();

	m_map_pkShopByPC.erase(ch->GetVID());
	M2_DELETE(pkShop);
}

void CShopManager::StopShopping(LPCHARACTER ch)
{
	LPSHOP shop;

#ifdef __OFFLINE_PRIVATE_SHOP__
	LPOPSHOP pkOfflinePrivateShop = FindOfflinePrivateShop(ch->GetOfflinePrivateShopVid());

	if (!(shop = ch->GetShop()) || (pkOfflinePrivateShop && !(shop = pkOfflinePrivateShop->GetShop())))
		return;

	ch->SetOfflinePrivateShopVid(0);
#else
	if (!(shop = ch->GetShop()))
		return;
#endif

	ch->SetMyShopTime();

	shop->RemoveGuest(ch);
	sys_log(0, "SHOP: END: %s", ch->GetName());
}

void CShopManager::Buy(LPCHARACTER ch, uint8_t pos)
{
// @correction227 BEGIN
	if (0 != g_BuySellTimeLimitValue)
	{
		if (get_dword_time() < ch->GetLastBuySellTime() + g_BuySellTimeLimitValue)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 골드를 버릴 수 없습니다."));
			return;
		}
	}

	ch->SetLastBuySellTime(get_dword_time());
// @correction227 END

	if (!ch->GetShop())
		return;

	if (!ch->GetShopOwner()
#ifdef __OFFLINE_PRIVATE_SHOP__
 && !ch->GetOfflinePrivateShopVid()
#endif
)
		return;

#ifndef ENABLE_FAST_NPC // @correction194
#ifdef __OFFLINE_PRIVATE_SHOP__
	if (!ch->GetOfflinePrivateShopVid())
#endif
	if (DISTANCE_APPROX(ch->GetX() - ch->GetShopOwner()->GetX(), ch->GetY() - ch->GetShopOwner()->GetY()) > 2000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상점과의 거리가 너무 멀어 물건을 살 수 없습니다."));
		return;
	}
#endif

	CShop* pkShop = ch->GetShop();

	if (!pkShop->IsPCShop())
	{

	}
	else
	{
	}

	ch->SetMyShopTime();

	int32_t ret = pkShop->Buy(ch, pos);

	if (SHOP_SUBHEADER_GC_OK != ret)
	{
		TPacketGCShop pack;

		pack.header	= HEADER_GC_SHOP;
		pack.subheader	= ret;
		pack.size	= sizeof(TPacketGCShop);

		ch->GetDesc()->Packet(&pack, sizeof(pack));
	}
}

void CShopManager::Sell(LPCHARACTER ch, uint8_t bCell, uint16_t wCount)
{
// @correction227 BEGIN
	if (0 != g_BuySellTimeLimitValue)
	{
		if (get_dword_time() < ch->GetLastBuySellTime()+g_BuySellTimeLimitValue)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("아직 골드를 버릴 수 없습니다."));
			return;
		}
	}

	ch->SetLastBuySellTime(get_dword_time());
// @correction227 END

	if (!ch->GetShop())
		return;

	if (!ch->GetShopOwner())
		return;

	if (!ch->CanHandleItem())
		return;

	if (ch->GetShop()->IsPCShop())
		return;

#ifndef ENABLE_FAST_NPC // @correction194
	if (DISTANCE_APPROX(ch->GetX()-ch->GetShopOwner()->GetX(), ch->GetY()-ch->GetShopOwner()->GetY())>2000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("상점과의 거리가 너무 멀어 물건을 팔 수 없습니다."));
		return;
	}
#endif

	LPITEM item = ch->GetInventoryItem(bCell);

	if (!item)
		return;

	if (item->IsEquipped() == true)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("착용 중인 아이템은 판매할 수 없습니다."));
		return;
	}

	if (true == item->isLocked())
	{
		return;
	}

#ifdef __SOULBIND__ // @correction202
	if (item->IsSealed())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Can't sell sealed item."));
		return;
	}
#endif

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_SELL))
		return;

	uint64_t dwPrice;

	if (wCount == 0 || wCount > item->GetCount())
		wCount = item->GetCount();

	dwPrice = item->GetShopBuyPrice();

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

	if (test_server)
		sys_log(0, "Sell Item price id %d %s itemid %d", ch->GetPlayerID(), ch->GetName(), item->GetID());

	const uint64_t nTotalMoney = ch->GetGold() + dwPrice;

	if (GOLD_MAX <= nTotalMoney)
	{
		sys_err("[OVERFLOW_GOLD] id %u name %s gold %llu", ch->GetPlayerID(), ch->GetName(), ch->GetGold());
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("20억냥이 초과하여 물품을 팔수 없습니다."));
		return;
	}

	sys_log(0, "SHOP: SELL: %s item name: %s(x%d):%u price: %u", ch->GetName(), item->GetName(), wCount, item->GetID(), dwPrice);

	DBManager::instance().SendMoneyLog(MONEY_LOG_SHOP, item->GetVnum(), dwPrice);

	if (wCount == item->GetCount())
		ITEM_MANAGER::instance().RemoveItem(item, "SELL");
	else
		item->SetCount(item->GetCount() - wCount);

	ch->ChangeGold(true, dwPrice, false);
}

bool CompareShopItemName(const SShopItemTable& lhs, const SShopItemTable& rhs)
{
	TItemTable* lItem = ITEM_MANAGER::instance().GetTable(lhs.vnum);
	TItemTable* rItem = ITEM_MANAGER::instance().GetTable(rhs.vnum);
	if (lItem && rItem)
		return strcmp(lItem->szLocaleName, rItem->szLocaleName) < 0;
	else
		return true;
}

bool ConvertToShopItemTable(IN CGroupNode* pNode, OUT TShopTableEx& shopTable)
{
	if (!pNode->GetValue("vnum", 0, shopTable.dwVnum))
	{
		sys_err("Group %s does not have vnum.", pNode->GetNodeName().c_str());
		return false;
	}

	if (!pNode->GetValue("name", 0, shopTable.name))
	{
		sys_err("Group %s does not have name.", pNode->GetNodeName().c_str());
		return false;
	}

	if (shopTable.name.length() >= SHOP_TAB_NAME_MAX)
	{
		sys_err("Shop name length must be less than %d. Error in Group %s, name %s", SHOP_TAB_NAME_MAX, pNode->GetNodeName().c_str(), shopTable.name.c_str());
		return false;
	}

	std::string stCoinType;
	if (!pNode->GetValue("cointype", 0, stCoinType))
	{
		stCoinType = "Gold";
	}

	if (boost::iequals(stCoinType, "Gold"))
	{
		shopTable.coinType = SHOP_COIN_TYPE_GOLD;
	}
	else if (boost::iequals(stCoinType, "SecondaryCoin"))
	{
		shopTable.coinType = SHOP_COIN_TYPE_SECONDARY_COIN;
	}
	else
	{
		sys_err("Group %s has undefine cointype(%s).", pNode->GetNodeName().c_str(), stCoinType.c_str());
		return false;
	}

	CGroupNode* pItemGroup = pNode->GetChildNode("items");
	if (!pItemGroup)
	{
		sys_err("Group %s does not have 'group items'.", pNode->GetNodeName().c_str());
		return false;
	}

	int32_t itemGroupSize = pItemGroup->GetRowCount();
	std::vector <TShopItemTable> shopItems(itemGroupSize);
	if (itemGroupSize >= SHOP_HOST_ITEM_MAX_NUM)
	{
		sys_err("count(%d) of rows of group items of group %s must be smaller than %d", itemGroupSize, pNode->GetNodeName().c_str(), SHOP_HOST_ITEM_MAX_NUM);
		return false;
	}

	for (int32_t i = 0; i < itemGroupSize; i++)
	{
		if (!pItemGroup->GetValue(i, "vnum", shopItems[i].vnum))
		{
			sys_err("row(%d) of group items of group %s does not have vnum column", i, pNode->GetNodeName().c_str());
			return false;
		}

		if (!pItemGroup->GetValue(i, "count", shopItems[i].count))
		{
			sys_err("row(%d) of group items of group %s does not have count column", i, pNode->GetNodeName().c_str());
			return false;
		}
		if (!pItemGroup->GetValue(i, "price", shopItems[i].price))
		{
			sys_err("row(%d) of group items of group %s does not have price column", i, pNode->GetNodeName().c_str());
			return false;
		}
	}
	std::string stSort;
	if (!pNode->GetValue("sort", 0, stSort))
	{
		stSort = "None";
	}

	if (boost::iequals(stSort, "Asc"))
	{
		std::sort(shopItems.begin(), shopItems.end(), CompareShopItemName);
	}
	else if(boost::iequals(stSort, "Desc"))
	{
		std::sort(shopItems.rbegin(), shopItems.rend(), CompareShopItemName);
	}

	CGrid grid = CGrid(5, 9);
	int32_t iPos;

	memset((void*)&shopTable.items[0], 0, sizeof(shopTable.items));

	for (size_t i = 0; i < shopItems.size(); i++)
	{
		TItemTable * item_table = ITEM_MANAGER::instance().GetTable(shopItems[i].vnum);
		if (!item_table)
		{
			sys_err("vnum(%d) of group items of group %s does not exist", shopItems[i].vnum, pNode->GetNodeName().c_str());
			return false;
		}

		iPos = grid.FindBlank(1, item_table->bSize);

		grid.Put(iPos, 1, item_table->bSize);
		shopTable.items[iPos] = shopItems[i];
	}

	shopTable.byItemCount = shopItems.size();
	return true;
}

bool CShopManager::ReadShopTableEx(const char* stFileName)
{
	FILE* fp = fopen(stFileName, "rb");
	if (nullptr == fp)
		return true;
	fclose(fp);

	CGroupTextParseTreeLoader loader;
	if (!loader.Load(stFileName))
	{
		sys_err("%s Load fail.", stFileName);
		return false;
	}

	CGroupNode* pShopNPCGroup = loader.GetGroup("shopnpc");
	if (nullptr == pShopNPCGroup)
	{
		sys_err("Group ShopNPC is not exist.");
		return false;
	}

	typedef std::multimap <uint32_t, TShopTableEx> TMapNPCshop;
	TMapNPCshop map_npcShop;
	for (int32_t i = 0; i < pShopNPCGroup->GetRowCount(); i++)
	{
		uint32_t npcVnum;
		std::string shopName;
		if (!pShopNPCGroup->GetValue(i, "npc", npcVnum) || !pShopNPCGroup->GetValue(i, "group", shopName))
		{
			sys_err("Invalid row(%d). Group ShopNPC rows must have 'npc', 'group' columns", i);
			return false;
		}
		std::transform(shopName.begin(), shopName.end(), shopName.begin(), (int32_t(*)(int32_t))std::tolower);
		CGroupNode* pShopGroup = loader.GetGroup(shopName.c_str());
		if (!pShopGroup)
		{
			sys_err("Group %s is not exist.", shopName.c_str());
			return false;
		}
		TShopTableEx table;
		if (!ConvertToShopItemTable(pShopGroup, table))
		{
			sys_err("Cannot read Group %s.", shopName.c_str());
			return false;
		}
		if (m_map_pkShopByNPCVnum.find(npcVnum) != m_map_pkShopByNPCVnum.end())
		{
			sys_err("%d cannot have both original shop and extended shop", npcVnum);
			return false;
		}

		map_npcShop.insert(TMapNPCshop::value_type(npcVnum, table));
	}

	for (TMapNPCshop::iterator it = map_npcShop.begin(); it != map_npcShop.end(); ++it)
	{
		uint32_t npcVnum = it->first;
		TShopTableEx& table = it->second;
		if (m_map_pkShop.find(table.dwVnum) != m_map_pkShop.end())
		{
			sys_err("Shop vnum(%d) already exists", table.dwVnum);
			return false;
		}
		TShopMap::iterator shop_it = m_map_pkShopByNPCVnum.find(npcVnum);

		LPSHOPEX pkShopEx = nullptr;
		if (m_map_pkShopByNPCVnum.end() == shop_it)
		{
			pkShopEx = M2_NEW CShopEx;
			pkShopEx->Create(0, npcVnum);
			m_map_pkShopByNPCVnum.insert(TShopMap::value_type(npcVnum, pkShopEx));
		}
		else
		{
			pkShopEx = dynamic_cast <CShopEx*> (shop_it->second);
			if (nullptr == pkShopEx)
			{
				sys_err("WTF!!! It can't be happend. NPC(%d) Shop is not extended version.", shop_it->first);
				return false;
			}
		}

		if (pkShopEx->GetTabCount() >= SHOP_TAB_COUNT_MAX)
		{
			sys_err("ShopEx cannot have tab more than %d", SHOP_TAB_COUNT_MAX);
			return false;
		}

		if (pkShopEx->GetVnum() != 0 && m_map_pkShop.find(pkShopEx->GetVnum()) != m_map_pkShop.end())
		{
			sys_err("Shop vnum(%d) already exist.", pkShopEx->GetVnum());
			return false;
		}
		m_map_pkShop.insert(TShopMap::value_type (pkShopEx->GetVnum(), pkShopEx));
		pkShopEx->AddShopTable(table);
	}

	return true;
}


#ifdef __OFFLINE_PRIVATE_SHOP__
void CShopManager::CreateOfflinePrivateShop(LPCHARACTER ch, const char* szSign, int32_t iTitle, int32_t iPoly, uint32_t dwTime, TShopItemTable * pTable, uint8_t bItemCount)
{
	if (!(ch->GetMapIndex() >= 1 && ch->GetMapIndex() <= 6))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Sadece Koylerde Pazar Acabilirsin !");
		return;
	}

	if (m_map_pkPrivateOfflineShop.size() >= 1000)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Bu Mapda Daha Fazla Pazar Acilamaz !");
		return;
	}

	std::unique_ptr<SQLMsg> pMsg2(DBManager::instance().DirectQuery("SELECT id FROM player.player_shop WHERE player_id = %d", ch->GetPlayerID()));
	SQLResult * pRes2 = pMsg2->Get();
	if (!pMsg2 || !pRes2)
		return;

	if (pRes2->uiNumRows)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, "Sadece 1 Pazar Acabilirsin !");
		return;
	}

	LPSECTREE sectree = SECTREE_MANAGER::instance().Get(ch->GetMapIndex(), ch->GetX(), ch->GetY());

	if (!sectree)
	{
		sys_log(0, "cannot find sectree by %dx%d mapindex %d", ch->GetX(), ch->GetY(), ch->GetMapIndex());
		return;
	}

	LPSHOP pkShop = M2_NEW CShop;
	pkShop->SetOfflinePrivateShop(true);
	pkShop->SetPCShop(ch);

	uint32_t dwLocalTime = dwTime + time(nullptr);
	LPOPSHOP pkOfflinePrivateShop = M2_NEW COfflinePrivateShop(iTitle, iPoly, dwLocalTime, pkShop);
	pkOfflinePrivateShop->SetMapIndex(ch->GetMapIndex());
	pkOfflinePrivateShop->SetXYZ(ch->GetXYZ());
	pkOfflinePrivateShop->SetOwnerName(ch->GetName());
	pkOfflinePrivateShop->SetSign(szSign);
	pkOfflinePrivateShop->SetPlayerID(ch->GetPlayerID());
	sectree->InsertEntity(pkOfflinePrivateShop);
	pkOfflinePrivateShop->UpdateSectree();

	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("INSERT INTO player.player_shop SET player_id = %d, race = %d, deco = %d, item_count = %d, name = '%s', player_name = '%s', map_index = %d, x = %d, y = %d, z = %d, date = NOW(), date_close = FROM_UNIXTIME(%d), ip = '%s', host_name = '%s', status = 1",
		ch->GetPlayerID(), iPoly, iTitle, bItemCount, szSign, ch->GetName(), ch->GetMapIndex(), ch->GetX(), ch->GetY(), ch->GetZ(), dwLocalTime, inet_ntoa(ch->GetDesc()->GetAddr().sin_addr), g_stHostname.c_str()));
	SQLResult * pRes = pMsg->Get();
	uint32_t dwShopVid = pRes->uiInsertID;
	pkOfflinePrivateShop->SetShopVid(dwShopVid);
	ch->SaveAndFlush();
	pkShop->SetShopItems(pTable, bItemCount, dwShopVid);
	m_map_pkPrivateOfflineShop.insert(TOfflinePrivateShopMap::value_type(pkOfflinePrivateShop->GetShopVid(), pkOfflinePrivateShop));
}

LPOPSHOP CShopManager::FindOfflinePrivateShop(uint32_t dwShopVid)
{
	for (auto& it : m_map_pkPrivateOfflineShop)
		if (it.second->GetShopVid() == dwShopVid)
			return it.second;

	return nullptr;
}

LPOPSHOP CShopManager::FindOfflinePrivateShopByShopVID(uint32_t dwVid)
{
	for (auto& it : m_map_pkPrivateOfflineShop)
		if (it.second->GetVid() == dwVid)
			return it.second;

	return nullptr;
}

void CShopManager::DestroyOfflinePrivateShop(uint32_t dwShopVid)
{
	LPOPSHOP pkOfflinePrivateShop = FindOfflinePrivateShop(dwShopVid);

	if (!pkOfflinePrivateShop)
		return;

	DBManager::instance().DirectQuery("UPDATE player.player_shop SET status = 0 WHERE id = %d and player_id = %d", pkOfflinePrivateShop->GetShopVid(), pkOfflinePrivateShop->GetPlayerID());

	for (auto& it : m_map_pkPrivateOfflineShop)
		if (it.second->GetShopVid() == dwShopVid)
			m_map_pkPrivateOfflineShop.erase(it.first);

	// pkOfflinePrivateShop->Destroy();
	M2_DELETE(pkOfflinePrivateShop);
}

bool CreateItemTableFromRes(MYSQL_RES * res, std::vector<TPlayerItem> * pVec)
{
	if (!res)
	{
		pVec->clear();
		return true;
	}

	int32_t rows;

	if ((rows = mysql_num_rows(res)) <= 0)
	{
		pVec->clear();
		return true;
	}

	pVec->resize(rows);

	for (int32_t i = 0; i < rows; ++i)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		TPlayerItem & item = pVec->at(i);

		int32_t cur = 0;

		str_to_number(item.id, row[cur++]);
		str_to_number(item.owner, row[cur++]);
		str_to_number(item.window, row[cur++]);
		str_to_number(item.pos, row[cur++]);
		str_to_number(item.count, row[cur++]);
		str_to_number(item.vnum, row[cur++]);
		// for (int32_t j = 0; j < ITEM_SOCKET_MAX_NUM; j++)
		str_to_number(item.alSockets[0], row[cur++]);
		str_to_number(item.alSockets[1], row[cur++]);
		str_to_number(item.alSockets[2], row[cur++]);
// @correction144 BEGIN
		str_to_number(item.alSockets[3], row[cur++]);
		str_to_number(item.alSockets[4], row[cur++]);
		str_to_number(item.alSockets[5], row[cur++]);
// @correction144 END

		for (int32_t j = 0; j < ITEM_ATTRIBUTE_MAX_NUM; j++)
		{
			str_to_number(item.aAttr[j].bType, row[cur++]);
			str_to_number(item.aAttr[j].sValue, row[cur++]);
		}
#ifdef __SOULBIND__ // @correction202
		str_to_number(item.sealbind, row[cur++]);
#endif
#ifdef __CHANGE_LOOK__ // @correction192
		str_to_number(item.changelook, row[cur++]);
#endif
#ifdef __OFFLINE_PRIVATE_SHOP__
		str_to_number(item.shop_id, row[cur++]);
		str_to_number(item.prices, row[cur++]);
		str_to_number(item.bIsLocked, row[cur++]);
#endif
	}

	return true;
}

void CShopManager::BuildOfflinePrivateShop()
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT id, player_id, race, deco, item_count, name, player_name, map_index, x, y, z, UNIX_TIMESTAMP(date_close), unlock_slots FROM player.player_shop WHERE status = 1 and host_name = '%s'", g_stHostname.c_str()));
	SQLResult * pRes = pMsg->Get();
	MYSQL_ROW row;
	if (!pMsg || !pRes || !pRes->uiNumRows)
		return;

	while ((row = mysql_fetch_row(pRes->pSQLResult)))
	{
		uint32_t dwShopVid = 0, dwPlayerId = 0, dwTime = 0, dwRace = 0;
		int32_t map_index = 0, x = 0, y = 0, z = 0, iTitle = 0;
		uint8_t bItemCount = 0, bUnlockSlots = 0;
		str_to_number(dwShopVid, row[0]);
		str_to_number(dwPlayerId, row[1]);
		str_to_number(dwRace, row[2]);
		str_to_number(iTitle, row[3]);
		str_to_number(bItemCount, row[4]);
		str_to_number(map_index, row[7]);
		str_to_number(x, row[8]);
		str_to_number(y, row[9]);
		str_to_number(z, row[10]);
		str_to_number(dwTime, row[11]);
		str_to_number(bUnlockSlots, row[12]);
		LPSECTREE sectree = SECTREE_MANAGER::instance().Get(map_index, x, y);

		if (!sectree)
		{
			sys_log(0, "cannot find sectree by %dx%d mapindex %d", x, y, map_index);
			return;
		}

		LPSHOP pkShop = M2_NEW CShop;
		pkShop->SetOfflinePrivateShop(true);
		char queryStr[1024];
		snprintf(queryStr, sizeof(queryStr),
			"SELECT id,owner_id,window+0,pos,count,vnum,"
			"socket0,socket1,socket2,socket3,socket4,socket5,attrtype0,attrvalue0,attrtype1,attrvalue1,attrtype2,attrvalue2,attrtype3,attrvalue3,attrtype4,attrvalue4,attrtype5,attrvalue5,attrtype6,attrvalue6 "
#ifdef __SOULBIND__
			",sealbind"
#endif
#ifdef __CHANGE_LOOK__
			",changelook"
#endif
#ifdef __OFFLINE_PRIVATE_SHOP__
			",shop_id,prices,locked"
#endif
			" FROM player.item WHERE shop_id=%u", dwShopVid);

		std::unique_ptr<SQLMsg> pMsg2(DBManager::instance().DirectQuery(queryStr));
		SQLResult * pRes2 = pMsg2->Get();
		static std::vector<TPlayerItem> s_items;
		CreateItemTableFromRes(pRes2->pSQLResult, &s_items);
		for (size_t i = 0; i < s_items.size(); ++i)
		{
			TPlayerItem & p = s_items.at(i);

			LPITEM item = ITEM_MANAGER::instance().CreateItem(p.vnum, p.count, p.id);
			if (!item)
			{
				sys_err("cannot create item by vnum %u id %u", p.vnum, p.id);
				continue;
			}

			item->SetSkipSave(true);
			item->SetOwnerPID(p.owner);
			item->SetOwnerName(row[6]);
			item->SetCell(nullptr, p.pos);
			item->SetSockets(p.alSockets);
			item->SetAttributes(p.aAttr);
#ifdef __SOULBIND__
			item->SetSealBind(p.sealbind);
#endif
#ifdef __CHANGE_LOOK__
			item->SetChangeLook(p.changelook);
#endif
#ifdef __OFFLINE_PRIVATE_SHOP__
			item->SetOfflinePrivateShopVid(p.shop_id);
			item->SetOfflinePrivateShopPrice(p.prices);
			item->SetLock(p.bIsLocked);
#endif
			item->SetSkipSave(false);
			if (!p.owner && !p.pos)
			{
				item->SetOwnerPID(dwPlayerId);
				int iPos = pkShop->GetEmptyShopForUnlocks(item->GetSize());
				if (iPos != -1)
					item->SetCell(nullptr, iPos);
			}
			pkShop->SetOfflinePrivateShopItem(item);
		}
		LPOPSHOP pkOfflinePrivateShop = M2_NEW COfflinePrivateShop(iTitle, dwRace, dwTime, pkShop);
		pkOfflinePrivateShop->SetMapIndex(map_index);
		pkOfflinePrivateShop->SetXYZ(x, y, z);
		pkOfflinePrivateShop->SetOwnerName(row[6]);
		pkOfflinePrivateShop->SetSign(row[5]);
		pkOfflinePrivateShop->SetPlayerID(dwPlayerId);
		pkOfflinePrivateShop->GetShop()->SetUnlockSlots(bUnlockSlots);
		sectree->InsertEntity(pkOfflinePrivateShop);
		pkOfflinePrivateShop->UpdateSectree();
		pkOfflinePrivateShop->SetShopVid(dwShopVid);
		m_map_pkPrivateOfflineShop.insert(TOfflinePrivateShopMap::value_type(pkOfflinePrivateShop->GetShopVid(), pkOfflinePrivateShop));
	}
}

void CShopManager::StartOfflinePrivateShop(LPCHARACTER ch, uint32_t dwVid)
{
	if (!ch || !ch->IsPC())
		return;

	if (ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen() || ch->isChangeLookOpened() || ch->isAcceOpened(true) || ch->isAcceOpened(false))
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래창이 열린상태에서는 상점거래를 할수 가 없습니다."));
		return;
	}

	LPOPSHOP pkOfflinePrivateShop = FindOfflinePrivateShopByShopVID(dwVid);

	if (!pkOfflinePrivateShop)
		return;

	int32_t distance = DISTANCE_APPROX(pkOfflinePrivateShop->GetX() - ch->GetX(), pkOfflinePrivateShop->GetY() - ch->GetY());

	if (distance >= SHOP_MAX_DISTANCE)
	{
		sys_log(1, "SHOP: TOO_FAR: %s distance %d", ch->GetName(), distance);
		return;
	}

	LPSHOP pkShop = pkOfflinePrivateShop->GetShop();

	if (!pkShop)
		return;

	pkShop->AddGuest(ch, pkOfflinePrivateShop->GetVid(), false);
	ch->SetOfflinePrivateShopVid(pkOfflinePrivateShop->GetShopVid());
}

void CShopManager::AddBankOfflinePrivateShop(LPSHOP pkShop, uint64_t dwPrices)
{
	TOfflinePrivateShopMap::iterator it;

	for (it = m_map_pkPrivateOfflineShop.begin(); it != m_map_pkPrivateOfflineShop.end(); ++it)
	{
		if (it->second->GetShop() == pkShop)
			DBManager::instance().DirectQuery("UPDATE player.player_shop SET prices = prices + %llu WHERE id = %u and player_id = %u", dwPrices, it->second->GetShopVid(), it->second->GetPlayerID());
	}
}

uint8_t CShopManager::GetOfflinePrivateShopCount(uint32_t dwVid)
{
	uint8_t bCount = 0;
	TOfflinePrivateShopMap::iterator it;

	for (it = m_map_pkPrivateOfflineShop.begin(); it != m_map_pkPrivateOfflineShop.end(); ++it)
		if (it->second->GetPlayerID() == dwVid)
			++bCount;

	return bCount;
}

void CShopManager::SetOfflinePrivateShops(LPOPSHOP pkOfflinePrivateShops[], uint8_t bCount, uint32_t dwVid)
{
	uint8_t bTotalCount = 0;
	TOfflinePrivateShopMap::iterator it;
	for (it = m_map_pkPrivateOfflineShop.begin(); it != m_map_pkPrivateOfflineShop.end(); ++it)
	{
		if (it->second->GetPlayerID() == dwVid && bTotalCount < bCount)
		{
			pkOfflinePrivateShops[bTotalCount] = it->second;
			++bTotalCount;
		}
	}
}

bool CompareItemVnumAcPriceAC(CShop::SHOP_ITEM i, CShop::SHOP_ITEM j)
{
	return (i.item.vnum < j.item.vnum) && (i.item.dwPrices < j.item.dwPrices);
}

void CShopManager::SearchShops(std::vector<CShop::SHOP_ITEM>& sendItems, uint64_t* dwArgs, uint8_t bArgSize, const char* itemName, uint32_t dwSearcherPID)
{
	if (bArgSize > 9)
		return;

	if (dwArgs[0] < JOB_WARRIOR || dwArgs[0] >= JOB_MAX_NUM)
		return;

	if (dwArgs[1] < ITEM_NONE || dwArgs[1] >= ITEM_GACHA)
		return;

	// if (dwArgs[2] < USE_POTION || dwArgs[2] > USE_RESET_COSTUME_ATTR)
		// return;

	if ((int64_t)dwArgs[5] < 0 || dwArgs[5] > PLAYER_MAX_LEVEL_CONST)
		return;

	if ((int64_t)dwArgs[6] < 0 || dwArgs[6] > PLAYER_MAX_LEVEL_CONST)
		return;

	if ((int64_t)dwArgs[3] < 0 || dwArgs[3] > 9)
		return;

	if ((int64_t)dwArgs[4] < 0 || dwArgs[4] > 9)
		return;

	if ((int64_t)dwArgs[7] < 0 || dwArgs[7] > GOLD_MAX)
		return;

	if ((int64_t)dwArgs[8] < 0 || dwArgs[8] > GOLD_MAX)
		return;

	if (dwArgs[5] > dwArgs[6])
		return;

	if (dwArgs[3] > dwArgs[4])
		return;

	if (dwArgs[7] > dwArgs[8])
		return;

	for (auto& it : m_map_pkPrivateOfflineShop)
	{
		if (!it.second || !it.second->GetShop())
			continue;

		auto* pkOPShop = it.second;
		auto* pkShop = pkOPShop->GetShop();

		// if (pkOPShop->GetPlayerID() == dwSearcherPID)
			// continue;

		std::vector<CShop::SHOP_ITEM>& shop_items = pkShop->GetItemVector();

		for (auto& item : shop_items)
		{
			LPITEM pkItem = item.pkItem;
			bool cont = false;
			if (pkItem)
			{
				if (strlen(itemName) <= 2)
				{
					if (dwArgs[1] && pkItem->GetType() == dwArgs[1] && pkItem->GetSubType() == dwArgs[2])
						continue;

					if ((pkItem->GetType() == ITEM_WEAPON || pkItem->GetSubType() == ITEM_ARMOR) && !(pkItem->GetRefineLevel() >= (int32_t)dwArgs[3] && pkItem->GetRefineLevel() <= (int32_t)dwArgs[4]))
						continue;
					
					if (pkItem->GetLevelLimit() && !((pkItem->GetLevelLimit() >= (int32_t)dwArgs[5] && pkItem->GetLevelLimit() <= (int32_t)dwArgs[6])))
						continue;

					if (item.item.dwPrices && !(item.item.dwPrices >= dwArgs[7] && item.item.dwPrices <= dwArgs[8]))
						continue;

					switch (dwArgs[0])
					{
						case JOB_WARRIOR:
							if (pkItem->GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR)
								cont = true;
							break;

						case JOB_ASSASSIN:
							if (pkItem->GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN)
								cont = true;
							break;

						case JOB_SHAMAN:
							if (pkItem->GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
								cont = true;
							break;

						case JOB_SURA:
							if (pkItem->GetAntiFlag() & ITEM_ANTIFLAG_SURA)
								cont = true;
							break;

						case JOB_WOLFMAN:
							if (pkItem->GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN)
								cont = true;
							break;
					}

					if (cont)
						continue;

				cont = true;
				}
				else
				{
					if (pkItem->GetName())
					{
						std::string foundName(pkItem->GetName());
						if (foundName.find(itemName) != std::string::npos)
							cont = true;
					}
				}

				if (cont)
				{
					pkItem->SetOwnerName(pkOPShop->GetOwnerName().c_str());
					sendItems.push_back(item);
				}
			}
		}
	}
	std::stable_sort(sendItems.begin(), sendItems.end(), CompareItemVnumAcPriceAC);

	return;
}

bool CShopManager::ReadPriceCheckTable(const char* stFileName)
{
	FILE* fp = fopen(stFileName, "rb");
	if (!fp)
		return false;

	fclose(fp);

	CGroupTextParseTreeLoader loader;
	if (!loader.Load(stFileName))
	{
		sys_err("%s Load fail.", stFileName);
		return false;
	}

	vec_check.clear();

	CGroupNode* pGroup = loader.GetGroup("0");
	if (!pGroup)
	{
		sys_err("Group 0 is not exist.");
		return false;
	}

	for (int32_t i = 0; i < pGroup->GetRowCount(); ++i)
	{
		TPriceCheck table;
		memset(&table, 0, sizeof(table));
		uint32_t dwVnum;
		char szPrices[24 + 1];
		if (!pGroup->GetValue(i, "vnum", dwVnum) || !pGroup->GetValue(i, "prices", szPrices))
		{
			sys_err("Invalid row(%d). Group rows must have columns", i);
			return false;
		}
		table.vnum = dwVnum;

		std::string s(szPrices);
		size_t pos = s.find("~");
		if (std::string::npos == pos)
			table.min = std::stoull(s);
		else
		{
			table.min = std::stoull(s.substr(0, pos));
			table.max = std::stoull(s.substr(pos + 1));
		}

		vec_check.emplace_back(table);
	}

	return true;
}
#endif

