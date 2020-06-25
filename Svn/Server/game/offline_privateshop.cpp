#include "stdafx.h"
#include "offline_privateshop.h"
#include "char_manager.h"
#include "shop.h"
#include "desc.h"
#include "db.h"

COfflinePrivateShop::COfflinePrivateShop(int32_t iTitle, int32_t iPoly, uint32_t dwTime, LPSHOP pkShop) : m_dwRaceVnum(iPoly), m_dwVid(CHARACTER_MANAGER::instance().AllocVID()), m_dwEndTime(dwTime), m_pkShop(pkShop), m_dwTitle(iTitle)
{
	Initialize();
}

COfflinePrivateShop::~COfflinePrivateShop(void)
{
	Destroy();
}

void COfflinePrivateShop::Initialize(void)
{
	CEntity::Initialize(ENTITY_OFFLINE_PRIVATE_SHOP);
	m_dwShopVid = 0;
	m_dwPlayerID = 0;
	m_strSign = "";
	m_strOwnerName = "";
	m_dwStartTime = time(0);
}

void COfflinePrivateShop::Destroy(void)
{
	CEntity::Destroy();

	if (GetSectree())
		GetSectree()->RemoveEntity(this);

	m_dwRaceVnum = 0;
	m_dwVid = 0;
	m_dwStartTime = 0;
	m_dwEndTime = 0;
	m_dwShopVid = 0;
	m_dwPlayerID = 0;
	
	M2_DELETE(m_pkShop);
	m_pkShop = nullptr;
	m_strSign = "";
	m_strOwnerName = "";
	m_dwTitle = 0;
}

void COfflinePrivateShop::EncodeInsertPacket(LPENTITY ent)
{
	if (ent->GetType() != ENTITY_CHARACTER)
		return;

	LPDESC d;

	if (!(d = ent->GetDesc()))
		return;

	TPacketGCCharacterAdd pack;
	memset(&pack, 0, sizeof(pack));

	pack.header = HEADER_GC_CHARACTER_ADD;
	pack.dwVID = m_dwVid;
	pack.bType = CHAR_TYPE_NPC;
	pack.x = GetX();
	pack.y = GetY();
	pack.z = GetZ();
	pack.wRaceNum = m_dwRaceVnum;
	pack.bStateFlag = ADD_CHARACTER_STATE_GUNGON;

	d->Packet(&pack, sizeof(pack));

	TPacketGCCharacterAdditionalInfo addPacket;
	memset(&addPacket, 0, sizeof(TPacketGCCharacterAdditionalInfo));

	addPacket.header = HEADER_GC_CHAR_ADDITIONAL_INFO;
	addPacket.dwVID = m_dwVid;
	strlcpy(addPacket.name, m_strOwnerName.c_str(), sizeof(addPacket.name));

	d->Packet(&addPacket, sizeof(addPacket));

	TPacketGCShopSign p;

	p.bHeader = HEADER_GC_SHOP_SIGN;
	p.dwVID = m_dwVid;
	p.iTitle = m_dwTitle;
	strlcpy(p.szSign, m_strSign.c_str(), sizeof(p.szSign));

	d->Packet(&p, sizeof(p));
}

void COfflinePrivateShop::EncodeRemovePacket(LPENTITY ent)
{
	if (ent->GetType() != ENTITY_CHARACTER)
		return;

	LPDESC d;

	if (!(d = ent->GetDesc()))
		return;

	TPacketGCCharacterDelete pack;

	pack.header	= HEADER_GC_CHARACTER_DEL;
	pack.id	= m_dwVid;

	d->Packet(&pack, sizeof(TPacketGCCharacterDelete));

	TPacketGCShopSign p;

	p.bHeader = HEADER_GC_SHOP_SIGN;
	p.dwVID = m_dwVid;
	p.szSign[0] = '\0';

	d->Packet(&p, sizeof(p));
}

const uint64_t COfflinePrivateShop::GetPrices()
{
	std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT prices FROM player.player_shop WHERE id = %u and player_id = %u", m_dwShopVid, m_dwPlayerID));
	SQLResult * pRes = pMsg->Get();
	if (!pMsg || !pRes || !pRes->uiNumRows)
		return 0;

	uint64_t dwPrices = 0;
	MYSQL_ROW row = mysql_fetch_row(pRes->pSQLResult);
	str_to_number(dwPrices, row[0]);

	return dwPrices;
}

void COfflinePrivateShop::GetInfo(TOfflinePrivateShopInfo * pkTOfflinePrivateShopInfo)
{
	if (m_pkShop)
		m_pkShop->SetOfflinePrivateShopInfo(pkTOfflinePrivateShopInfo);

	pkTOfflinePrivateShopInfo->dwVid = m_dwShopVid;
	strlcpy(pkTOfflinePrivateShopInfo->szSign, m_strSign.c_str(), sizeof(pkTOfflinePrivateShopInfo->szSign));
	pkTOfflinePrivateShopInfo->dwPrices = GetPrices();
	pkTOfflinePrivateShopInfo->bUnlockSlots = m_pkShop->GetUnlockSlots();
	pkTOfflinePrivateShopInfo->dwX = GetX();
	pkTOfflinePrivateShopInfo->dwY = GetY();
	pkTOfflinePrivateShopInfo->dwMapIndex = GetMapIndex();
	pkTOfflinePrivateShopInfo->dwStartTime = m_dwStartTime;
	pkTOfflinePrivateShopInfo->dwEndTime = m_dwEndTime;
}

