#include "stdafx.h"
#include "../common/billing.h"
#include "config.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "p2p.h"
#include "guild.h"
#include "guild_manager.h"
#include "party.h"
#include "messenger_manager.h"
#include "empire_text_convert.h"
#include "unique_item.h"
#include "xmas_event.h"
#include "affect.h"
#include "castle.h"
#include "dev_log.h"
#include "locale_service.h"
#include "questmanager.h"
#include "pcbang.h"
#include "skill.h"
#include "threeway_war.h"
#ifdef __OFFLINE_PRIVATE_SHOP__
#include "item.h"
#include "shop.h"
#include "item_manager.h"
#include "shop_manager.h"
#include "offline_privateshop.h"
#include "buffer_manager.h"
extern bool CreateItemTableFromRes(MYSQL_RES * res, std::vector<TPlayerItem> * pVec);
#endif
#ifdef __BOSS_TRACKING_SYSTEM__
#include "boss_tracking.h"
#endif

CInputP2P::CInputP2P()
{
	BindPacketInfo(&m_packetInfoGG);
}

void CInputP2P::Login(LPDESC d, const char * c_pData)
{
	P2P_MANAGER::instance().Login(d, (TPacketGGLogin *) c_pData);
}

void CInputP2P::Logout(LPDESC d, const char * c_pData)
{
	TPacketGGLogout * p = (TPacketGGLogout *) c_pData;
	P2P_MANAGER::instance().Logout(p->szName);
}

int32_t CInputP2P::Relay(LPDESC d, const char * c_pData, size_t uiBytes)
{
	TPacketGGRelay * p = (TPacketGGRelay *) c_pData;

	if (uiBytes < sizeof(TPacketGGRelay) + p->lSize)
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	sys_log(0, "InputP2P::Relay : %s size %d", p->szName, p->lSize);

	LPCHARACTER pkChr = CHARACTER_MANAGER::instance().FindPC(p->szName);

	const uint8_t* c_pbData = (const uint8_t *) (c_pData + sizeof(TPacketGGRelay));

	if (!pkChr)
		return p->lSize;

	if (*c_pbData == HEADER_GC_WHISPER)
	{
		if (pkChr->IsBlockMode(BLOCK_WHISPER))
		{
			return p->lSize;
		}

		char buf[1024];
		memcpy(buf, c_pbData, MIN(p->lSize, sizeof(buf)));

		TPacketGCWhisper* p2 = (TPacketGCWhisper*) buf;
		uint8_t bToEmpire = (p2->bType >> 4);
		p2->bType = p2->bType & 0x0F;
		if(p2->bType == 0x0F) {
			p2->bType = WHISPER_TYPE_SYSTEM;
		} else {
			if (!pkChr->IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE))
				if (bToEmpire >= 1 && bToEmpire <= 3 && pkChr->GetEmpire() != bToEmpire)
				{
					ConvertEmpireText(bToEmpire,
							buf + sizeof(TPacketGCWhisper),
							p2->wSize - sizeof(TPacketGCWhisper),
							10+2*pkChr->GetSkillPower(SKILL_LANGUAGE1 + bToEmpire - 1));
				}
		}

		pkChr->GetDesc()->Packet(buf, p->lSize);
	}
#ifdef __OFFLINE_PRIVATE_SHOP__
	else if (*c_pbData == HEADER_GC_OFFLINE_PRIVATE_SHOP)
	{
		uint8_t bHeader = *((uint8_t *) c_pbData);
		c_pbData += sizeof(uint8_t);
		switch(*c_pbData)
		{
			case OFFLINE_PRIVATE_SHOP_ITEM_DEL:
			{
				c_pbData += sizeof(uint8_t);
				uint32_t dwPID = *((uint32_t *) c_pbData);
				c_pbData += sizeof(uint32_t);
				TShopItemData* pData = ((TShopItemData *) c_pbData);
				c_pbData += sizeof(TShopItemData);
				const uint8_t* pBuf = (const uint8_t *)c_pbData;
				c_pbData += sizeof(TPacketGCOfflinePrivateShopInfo) + sizeof(TOfflinePrivateShopItemUpdate);
				LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);
				if (ch)
				{
					LPITEM item = ITEM_MANAGER::instance().CreateItem(pData->vnum, pData->count);
					if (!item)
						break;

					item->SetSkipSave(true);
					item->SetOwnerPID(ch->GetPlayerID());
					item->SetSockets((const int32_t *)pData->alSockets);
					item->SetAttributes(pData->aAttr);
#ifdef __CHANGE_LOOK__
					item->SetChangeLook(pData->changelook);
#endif
					item->SetOfflinePrivateShopVid(0);
					item->SetOfflinePrivateShopPrice(0);
					item->SetLock(0);
					item->SetSkipSave(false);
					int32_t iEmptyPos;
					if (item->IsDragonSoul())
						iEmptyPos = ch->GetEmptyDragonSoulInventory(item);
					else
						iEmptyPos = ch->GetEmptyInventory(item);

					if (iEmptyPos < 0)
					{
						PIXEL_POSITION coord;
						coord.x = ch->GetX();
						coord.y = ch->GetY();

						item->AddToGround(ch->GetMapIndex(), coord);
						item->SetOwnership(ch, 180);
						item->StartDestroyEvent();
					}
					else
					{
						if (item->IsDragonSoul())
							item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
						else
							item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
					}
					ITEM_MANAGER::instance().FlushDelayedSave(item);
					// item->SaveAndFlush();
					ch->Save();
					pkChr->GetDesc()->Packet(pBuf, sizeof(TPacketGCOfflinePrivateShopInfo) + sizeof(TOfflinePrivateShopItemUpdate));
				}
			}
			break;
			case OFFLINE_PRIVATE_SHOP_ITEM_ADD:
			{
				c_pbData += sizeof(uint8_t);
				uint32_t dwPID = *((uint32_t *) c_pbData);
				c_pbData += sizeof(uint32_t);

				uint8_t bInvenType = *((uint8_t *) c_pbData);
				c_pbData += sizeof(uint8_t);

				uint32_t dwSourcePos = *((uint32_t *) c_pbData);
				c_pbData += sizeof(uint32_t);

				const uint8_t* pBuf = (const uint8_t *)c_pbData;
				c_pbData += sizeof(TPacketGCOfflinePrivateShopInfo) + sizeof(TOfflinePrivateShopItemUpdate);

				LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);
				if (ch)
				{
					LPITEM item = ch->GetItem(TItemPos(bInvenType, dwSourcePos));
					if (!item)
						break;

					item->SaveAndFlush();
					ITEM_MANAGER::instance().RemoveItem(item);

					ch->SaveAndFlush();
					pkChr->GetDesc()->Packet(pBuf, sizeof(TPacketGCOfflinePrivateShopInfo) + sizeof(TOfflinePrivateShopItemUpdate));
				}
			}
			break;
			case OFFLINE_PRIVATE_SHOP_ITEM_BUY:
			{
				c_pbData += sizeof(uint8_t);
				uint32_t dwPID = *((uint32_t *) c_pbData);
				c_pbData += sizeof(uint32_t);
				uint32_t dwShopId = *((uint32_t *) c_pbData);
				c_pbData += sizeof(uint32_t);
				TShopItemData* pData = ((TShopItemData *) c_pbData);
				c_pbData += sizeof(TShopItemData);

				LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(dwPID);
				LPITEM item = ITEM_MANAGER::instance().CreateItem(pData->vnum, pData->count);
				if (ch && item)
				{
					ch->ChangeGold(false, pData->dwPrices, false);
					item->SetSkipSave(true);
					item->SetOwnerPID(ch->GetPlayerID());
					item->SetSockets((const int32_t *)pData->alSockets);
					item->SetAttributes(pData->aAttr);
#ifdef __CHANGE_LOOK__
					item->SetChangeLook(pData->changelook);
#endif
					item->SetOfflinePrivateShopVid(0);
					item->SetOfflinePrivateShopPrice(0);
					item->SetLock(0);
					item->SetSkipSave(false);

					int32_t iEmptyPos;
					if (item->IsDragonSoul())
						iEmptyPos = ch->GetEmptyDragonSoulInventory(item);
					else
						iEmptyPos = ch->GetEmptyInventory(item);

					if (iEmptyPos < 0)
					{
						PIXEL_POSITION coord;
						coord.x = ch->GetX();
						coord.y = ch->GetY();

						item->AddToGround(ch->GetMapIndex(), coord);
						item->SetOwnership(ch, 180);
						item->StartDestroyEvent();
					}
					else
					{
						if (item->IsDragonSoul())
							item->AddToCharacter(ch, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyPos));
						else
							item->AddToCharacter(ch, TItemPos(INVENTORY, iEmptyPos));
					}
					ITEM_MANAGER::instance().FlushDelayedSave(item);
					// item->SaveAndFlush();
					ch->Save();

					if (ch->GetDesc())
					{
						TPacketGCOfflinePrivateShopInfo pack;
						pack.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
						pack.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_BUY;
						pack.wSize = sizeof(pack) + sizeof(dwShopId) + sizeof(pData->display_pos);
						ch->GetDesc()->BufferedPacket(&pack, sizeof(pack));
						ch->GetDesc()->Packet(&dwShopId, sizeof(dwShopId));
						ch->GetDesc()->Packet(&pData->display_pos, sizeof(pData->display_pos));
					}
				}
			}
			break;
			default:
				pkChr->GetDesc()->BufferedPacket(&bHeader, sizeof(bHeader));
				pkChr->GetDesc()->Packet(c_pbData, p->lSize - sizeof(bHeader));
				break;
		}
		return (p->lSize);
	}
#endif
	else
		pkChr->GetDesc()->Packet(c_pbData, p->lSize);

	return (p->lSize);
}

#ifdef __OFFLINE_PRIVATE_SHOP__
int32_t CInputP2P::OfflinePrivateShop(uint8_t bSubHeader, LPDESC d, const char * c_pData)
{
	switch (bSubHeader)
	{
		case OFFLINE_PRIVATE_SHOP_INFO:
		{
			TPacketGGOfflinePrivateShopInfo * p = (TPacketGGOfflinePrivateShopInfo *)c_pData;

			uint32_t& dwPID = p->dwPID;
			const uint8_t bCount = CShopManager::instance().GetOfflinePrivateShopCount(dwPID);
			CCI * pkCCI = P2P_MANAGER::instance().FindByPID(dwPID);
			if (pkCCI && pkCCI->pkDesc && bCount)
			{
				TEMP_BUFFER buf;
				LPOPSHOP pkOfflinePrivateShops[bCount];
				CShopManager::instance().SetOfflinePrivateShops(pkOfflinePrivateShops, bCount, dwPID);
				TOfflinePrivateShopInfo pkTOfflinePrivateShopInfo[bCount];
				memset(pkTOfflinePrivateShopInfo, 0, sizeof(TOfflinePrivateShopInfo) * bCount);
				for (uint8_t i = 0; i < bCount; ++i)
				{
					TPacketGCOfflinePrivateShopInfo packet;
					packet.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
					packet.bSubHeader = OFFLINE_PRIVATE_SHOP_INFO;
					packet.wSize = sizeof(packet) + sizeof(TOfflinePrivateShopInfo);
					buf.write(&packet, sizeof(packet));
					pkOfflinePrivateShops[i]->GetInfo(&pkTOfflinePrivateShopInfo[i]);
					buf.write(&pkTOfflinePrivateShopInfo[i], sizeof(TOfflinePrivateShopInfo));
					pkCCI->pkDesc->SetRelay(pkCCI->szName);
					pkCCI->pkDesc->LargePacket(buf.read_peek(), buf.size());
					pkCCI->pkDesc->SetRelay("");
					buf.reset();
				}

			}
		}
		break;
		case OFFLINE_PRIVATE_SHOP_ITEM_DEL:
		{
			TPacketGGOfflinePrivateShopItemDel * p = (TPacketGGOfflinePrivateShopItemDel *)c_pData;

			uint32_t& dwPID = p->dwPID;
			uint32_t& dwShopId = p->dwShopId;
			uint32_t& dwTargetPos = p->dwTargetPos;

			LPOPSHOP pkShop = CShopManager::instance().FindOfflinePrivateShop(dwShopId);
			CCI * pkCCI = P2P_MANAGER::instance().FindByPID(dwPID);
			if (pkCCI && pkCCI->pkDesc && pkShop)
			{
				TEMP_BUFFER buf;
				LPSHOP pShop = pkShop->GetShop();
				if (pShop)
				{
					TShopItemData * pData = new TShopItemData;
					uint32_t dwID = pShop->DelOfflinePrivateShopItem(dwTargetPos, nullptr, pData);
					if (dwID)
					{
						TPacketGGOfflinePrivateShop packet;
						packet.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
						packet.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_DEL;
						buf.write(&packet, sizeof(packet));
						buf.write(&dwPID, sizeof(dwPID));
						buf.write(pData, sizeof(*pData));

						TPacketGCOfflinePrivateShopInfo packet2;
						packet2.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
						packet2.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_UPDATE;
						packet2.wSize = sizeof(packet2) + sizeof(TOfflinePrivateShopItemUpdate);
						buf.write(&packet2, sizeof(packet2));
						TOfflinePrivateShopItemUpdate update_packet;
						update_packet.dwVid = dwShopId;
						update_packet.item = pShop->GetItemVector()[dwTargetPos].item;
						update_packet.item.display_pos = dwTargetPos;
						buf.write(&update_packet, sizeof(update_packet));

						pkCCI->pkDesc->SetRelay(pkCCI->szName);
						pkCCI->pkDesc->Packet(buf.read_peek(), buf.size());
						pkCCI->pkDesc->SetRelay("");
					}
					delete pData;
					pData = nullptr;
				}
			}
		}
		break;
		case OFFLINE_PRIVATE_SHOP_ITEM_ADD:
		{
			TPacketGGOfflinePrivateShopItemAdd * p = (TPacketGGOfflinePrivateShopItemAdd *)c_pData;

			uint32_t& dwPID = p->dwPID;
			uint32_t& dwShopId = p->dwShopId;
			uint32_t& dwTargetPos = p->dwTargetPos;
			uint8_t& bSize = p->bSize;
			uint8_t& bInvenType = p->bInvenType;
			uint32_t& dwSourcePos = p->dwSourcePos;
			uint64_t& dwPrices = p->dwPrices;
			TShopItemData* pData = &p->pData;

			LPOPSHOP pkShop = CShopManager::instance().FindOfflinePrivateShop(dwShopId);
			CCI * pkCCI = P2P_MANAGER::instance().FindByPID(dwPID);
			// sys_err("RELAY %d %d %d %d %d %d %d %d", dwPID, dwId, dwTargetPos, dwShopId, bSize, bInvenType, dwSourcePos, dwPrices);

			if (pkShop)
			{
				LPSHOP pShop = pkShop->GetShop();
				if (pShop && pkCCI && pkCCI->pkDesc)
				{
					uint8_t bIsEmpty = pShop->IsEmpty(dwTargetPos, bSize);
					if (bIsEmpty)
					{
						LPITEM item = ITEM_MANAGER::instance().CreateItem(pData->vnum, pData->count);
						if (!item)
							break;

						item->SetSkipSave(true);
						item->SetOwnerPID(dwPID);
						item->SetCell(nullptr, pData->display_pos);
						item->SetWindow(RESERVED_WINDOW);
						item->SetSockets((const int32_t *)pData->alSockets);
						item->SetAttributes(pData->aAttr);
#ifdef __CHANGE_LOOK__
						item->SetChangeLook(pData->changelook);
#endif
						item->SetOfflinePrivateShopPrice(dwPrices);
						item->SetOfflinePrivateShopVid(pkShop->GetShopVid());
						item->SetLock(0);
						item->SetSkipSave(false);
						item->SaveAndFlush();

						pShop->SetOfflinePrivateShopItem(item);
						pShop->BroadcastUpdateItem(pData->display_pos);

						TEMP_BUFFER buf;
						TPacketGGOfflinePrivateShop packet;
						packet.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
						packet.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_ADD;
						buf.write(&packet, sizeof(packet));
						buf.write(&dwPID, sizeof(dwPID));
						buf.write(&bInvenType, sizeof(bInvenType));
						buf.write(&dwSourcePos, sizeof(dwSourcePos));

						TPacketGCOfflinePrivateShopInfo packet2;
						packet2.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
						packet2.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_UPDATE;
						packet2.wSize = sizeof(packet2) + sizeof(TOfflinePrivateShopItemUpdate);
						buf.write(&packet2, sizeof(packet2));
						TOfflinePrivateShopItemUpdate update_packet;
						update_packet.dwVid = dwShopId;
						update_packet.item = pShop->GetItemVector()[pData->display_pos].item;
						buf.write(&update_packet, sizeof(update_packet));

						pkCCI->pkDesc->SetRelay(pkCCI->szName);
						pkCCI->pkDesc->Packet(buf.read_peek(), buf.size());
						pkCCI->pkDesc->SetRelay("");
					}
				}
			}
		}
		break;
		case OFFLINE_PRIVATE_SHOP_RENAME:
		{
			TPacketGGOfflinePrivateShopRename * p = (TPacketGGOfflinePrivateShopRename *)c_pData;

			uint32_t& dwShopId = p->dwShopId;
			LPOPSHOP pkShop = CShopManager::instance().FindOfflinePrivateShop(dwShopId);
			if (pkShop)
			{
				char queryStr[256];
				snprintf(queryStr, sizeof(queryStr), "UPDATE player.player_shop SET name = '%s' WHERE id = %u", p->szSign, pkShop->GetShopVid());
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(queryStr));
				pkShop->SetSign(p->szSign);
				pkShop->UpdateSectree();
			}
		};
		break;
		case OFFLINE_PRIVATE_SHOP_ITEM_SEARCH:
		{
			TPacketGGOfflinePrivateShopSearch * p = (TPacketGGOfflinePrivateShopSearch *)c_pData;

			uint32_t& dwPID = p->dwPID;
			uint64_t* dwArgs = p->dwArgs;

			CCI * pkCCI = P2P_MANAGER::instance().FindByPID(dwPID);
			if (pkCCI && pkCCI->pkDesc)
			{
				std::vector<CShop::SHOP_ITEM> sendItems = {};
				CShopManager::instance().SearchShops(sendItems, dwArgs, 9, p->szItemName, dwPID);
				size_t size = sendItems.size();
				if (size > 200)
					size = 200;

				for (size_t i = 0; i < size; ++i)
				{
					const CShop::SHOP_ITEM * item = &sendItems[i];
					LPITEM pkItem = item->pkItem;
					
					if (pkItem)
					{
						if (!pkCCI)
							break;

						LPDESC pkDesc = pkCCI->pkDesc;
						if (!pkDesc)
							break;

						TEMP_BUFFER buf;
						TPacketGCOfflinePrivateShopInfo pack;
						TOfflinePrivateShopItemSearch data;
						data.dwVid = pkItem->GetOfflinePrivateShopVid();
						strlcpy(data.szSellerName, pkItem->GetOwnerName(), sizeof(data.szSellerName));
						data.item = item->item;
						pack.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
						pack.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_SEARCH;
						pack.wSize = sizeof(pack) + sizeof(data);
						buf.write(&pack, sizeof(pack));
						buf.write(&data, sizeof(data));
						pkDesc->SetRelay(pkCCI->szName);
						pkDesc->Packet(buf.read_peek(), buf.size());
						pkDesc->SetRelay("");
					}
				}
			}
		}
		break;
		case OFFLINE_PRIVATE_SHOP_ITEM_BUY:
		{
			TPacketGGOfflinePrivateShopBuy * p = (TPacketGGOfflinePrivateShopBuy *)c_pData;

			uint32_t& dwPID = p->dwPID;
			uint32_t& dwShopId = p->dwShopId;
			uint64_t& dwPrices = p->dwPrices;
			uint32_t& dwTargetPos = p->dwTargetPos;

			CCI * pkCCI = P2P_MANAGER::instance().FindByPID(dwPID);
			if (pkCCI && pkCCI->pkDesc)
			{
				LPOPSHOP pkShop = CShopManager::instance().FindOfflinePrivateShop(dwShopId);
				if (pkShop )//&& pkShop->GetPlayerID() != dwPID)
				{
					LPSHOP pShop = pkShop->GetShop();
					if (pShop)
					{
						TShopItemData * pData = new TShopItemData;
						int32_t returnHeader = pShop->RemoteBuy(pData, dwPrices, dwTargetPos);
						if (SHOP_SUBHEADER_GC_OK == returnHeader)
						{
							TEMP_BUFFER buf;
							TPacketGGOfflinePrivateShop pack;
							pack.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
							pack.bSubHeader = OFFLINE_PRIVATE_SHOP_ITEM_BUY;
							buf.write(&pack, sizeof(pack));
							buf.write(&dwPID, sizeof(dwPID));
							buf.write(&dwShopId, sizeof(dwShopId));
							buf.write(pData, sizeof(*pData));
							pkCCI->pkDesc->SetRelay(pkCCI->szName);
							pkCCI->pkDesc->Packet(buf.read_peek(), buf.size());
							pkCCI->pkDesc->SetRelay("");
						}
						else
						{
							TPacketGCShop pack;
							pack.header	= HEADER_GC_SHOP;
							pack.subheader = returnHeader;
							pack.size = sizeof(TPacketGCShop);
							pkCCI->pkDesc->SetRelay(pkCCI->szName);
							pkCCI->pkDesc->Packet(&pack, sizeof(pack));
							pkCCI->pkDesc->SetRelay("");
						}
						delete pData;
						pData = nullptr;
					}
				}
			}
		}
		break;
		case OFFLINE_PRIVATE_SHOP_UNLOCK_SLOTS:
		{
			TPacketGGOfflinePrivateShopUnlockSlots * p = (TPacketGGOfflinePrivateShopUnlockSlots *)c_pData;

			uint32_t& dwShopId = p->dwShopId;
			uint32_t& dwPID = p->dwPID;

			LPOPSHOP pkShop = CShopManager::instance().FindOfflinePrivateShop(dwShopId);
			if (pkShop)
			{
				uint8_t& bUnlockSlots = pkShop->GetShop()->GetUnlockSlots();
				bUnlockSlots += 1;
				char queryStr[256];
				snprintf(queryStr, sizeof(queryStr), "UPDATE player.player_shop SET unlock_slots = %u WHERE id = %u", bUnlockSlots, pkShop->GetShopVid());
				std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(queryStr));

				CCI* pkCCI = P2P_MANAGER::instance().FindByPID(dwPID);
				if (pkCCI && pkCCI->pkDesc)
				{
					TEMP_BUFFER buf;
					TPacketGCOfflinePrivateShopInfo packet;
					packet.bHeader = HEADER_GC_OFFLINE_PRIVATE_SHOP;
					packet.bSubHeader = OFFLINE_PRIVATE_SHOP_UNLOCK_SLOTS;
					packet.wSize = sizeof(packet) + (sizeof(dwShopId)) + (sizeof(bUnlockSlots));
					buf.write(&packet, sizeof(packet));
					buf.write(&dwShopId, sizeof(dwShopId));
					buf.write(&bUnlockSlots, sizeof(bUnlockSlots));

					pkCCI->pkDesc->SetRelay(pkCCI->szName);
					pkCCI->pkDesc->Packet(buf.read_peek(), buf.size());
					pkCCI->pkDesc->SetRelay("");
				}
			}
		}
		break;
		default:
			sys_err ("UNKNOWN OPS SUB PACKET");
			break;
	}

	return 0;
}
#endif

#ifdef ENABLE_FULL_NOTICE // @correction188
int32_t CInputP2P::Notice(LPDESC d, const char * c_pData, size_t uiBytes, bool bBigFont)
#else
int32_t CInputP2P::Notice(LPDESC d, const char * c_pData, size_t uiBytes)
#endif
{
	TPacketGGNotice * p = (TPacketGGNotice *) c_pData;

	if (uiBytes < sizeof(TPacketGGNotice) + p->lSize)
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char szBuf[256+1];
	strlcpy(szBuf, c_pData + sizeof(TPacketGGNotice), MIN(p->lSize + 1, sizeof(szBuf)));
#ifdef ENABLE_FULL_NOTICE // @correction188
	SendNotice(szBuf, bBigFont);
#else
	SendNotice(szBuf);
#endif
	return (p->lSize);
}

int32_t CInputP2P::MonarchNotice(LPDESC d, const char * c_pData, size_t uiBytes)
{
	TPacketGGMonarchNotice * p = (TPacketGGMonarchNotice *) c_pData;

	if (uiBytes < p->lSize + sizeof(TPacketGGMonarchNotice))
		return -1;

	if (p->lSize < 0)
	{
		sys_err("invalid packet length %d", p->lSize);
		d->SetPhase(PHASE_CLOSE);
		return -1;
	}

	char szBuf[256+1];
	strlcpy(szBuf, c_pData + sizeof(TPacketGGMonarchNotice), MIN(p->lSize + 1, sizeof(szBuf)));
	SendMonarchNotice(p->bEmpire, szBuf);
	return (p->lSize);
}

int32_t CInputP2P::MonarchTransfer(LPDESC d, const char* c_pData)
{
	TPacketMonarchGGTransfer* p = (TPacketMonarchGGTransfer*) c_pData;
	LPCHARACTER pTargetChar = CHARACTER_MANAGER::instance().FindByPID(p->dwTargetPID);

	if (pTargetChar != nullptr)
	{
		uint32_t qIndex = quest::CQuestManager::instance().GetQuestIndexByName("monarch_transfer");

		if (qIndex != 0)
		{
			pTargetChar->SetQuestFlag("monarch_transfer.x", p->x);
			pTargetChar->SetQuestFlag("monarch_transfer.y", p->y);
			quest::CQuestManager::instance().Letter(pTargetChar->GetPlayerID(), qIndex, 0);
		}
	}

	return 0;
}

int32_t CInputP2P::Guild(LPDESC d, const char* c_pData, size_t uiBytes)
{
	TPacketGGGuild * p = (TPacketGGGuild *) c_pData;
	uiBytes -= sizeof(TPacketGGGuild);
	c_pData += sizeof(TPacketGGGuild);

	CGuild * g = CGuildManager::instance().FindGuild(p->dwGuild);

	switch (p->bSubHeader)
	{
		case GUILD_SUBHEADER_GG_CHAT:
			{
				if (uiBytes < sizeof(TPacketGGGuildChat))
					return -1;

				TPacketGGGuildChat * p = (TPacketGGGuildChat *) c_pData;

				if (g)
					g->P2PChat(p->szText);

				return sizeof(TPacketGGGuildChat);
			}

		case GUILD_SUBHEADER_GG_SET_MEMBER_COUNT_BONUS:
			{
				if (uiBytes < sizeof(int32_t))
					return -1;

				int32_t iBonus = *((int32_t *) c_pData);
				CGuild* pGuild = CGuildManager::instance().FindGuild(p->dwGuild);
				if (pGuild)
				{
					pGuild->SetMemberCountBonus(iBonus);
				}
				return sizeof(int32_t);
			}
		default:
			sys_err ("UNKNOWN GUILD SUB PACKET");
			break;
	}
	return 0;
}


struct FuncShout
{
	const char * m_str;
	uint8_t m_bEmpire;

	FuncShout(const char * str, uint8_t bEmpire) : m_str(str), m_bEmpire(bEmpire)
	{
	}

	void operator () (LPDESC d)
	{
		if (!d->GetCharacter() || (!g_bGlobalShoutEnable && d->GetCharacter()->GetGMLevel() == GM_PLAYER && d->GetEmpire() != m_bEmpire)) // @correction207
			return;

		d->GetCharacter()->ChatPacket(CHAT_TYPE_SHOUT, "%s", m_str);
	}
};

void SendShout(const char * szText, uint8_t bEmpire)
{
	const DESC_MANAGER::DESC_SET & c_ref_set = DESC_MANAGER::instance().GetClientSet();
	std::for_each(c_ref_set.begin(), c_ref_set.end(), FuncShout(szText, bEmpire));
}

void CInputP2P::Shout(const char * c_pData)
{
	TPacketGGShout * p = (TPacketGGShout *) c_pData;
	SendShout(p->szText, p->bEmpire);
}

void CInputP2P::Disconnect(const char * c_pData)
{
	TPacketGGDisconnect * p = (TPacketGGDisconnect *) c_pData;

	LPDESC d = DESC_MANAGER::instance().FindByLoginName(p->szLogin);

	if (!d)
		return;

	if (!d->GetCharacter())
	{
		d->SetPhase(PHASE_CLOSE);
	}
	else
		d->DisconnectOfSameLogin();
}

void CInputP2P::Setup(LPDESC d, const char * c_pData)
{
	TPacketGGSetup * p = (TPacketGGSetup *) c_pData;
	sys_log(0, "P2P: Setup %s:%d", d->GetHostName(), p->wPort);
	d->SetP2P(d->GetHostName(), p->wPort, p->bChannel);
}

void CInputP2P::MessengerAdd(const char * c_pData)
{
	TPacketGGMessenger * p = (TPacketGGMessenger *) c_pData;
	sys_log(0, "P2P: Messenger Add %s %s", p->szAccount, p->szCompanion);
	MessengerManager::instance().__AddToList(p->szAccount, p->szCompanion);
}

void CInputP2P::MessengerRemove(const char * c_pData)
{
	TPacketGGMessenger * p = (TPacketGGMessenger *) c_pData;
	sys_log(0, "P2P: Messenger Remove %s %s", p->szAccount, p->szCompanion);
	MessengerManager::instance().__RemoveFromList(p->szAccount, p->szCompanion);
}

void CInputP2P::FindPosition(LPDESC d, const char* c_pData)
{
	TPacketGGFindPosition* p = (TPacketGGFindPosition*) c_pData;
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(p->dwTargetPID);
	if (ch && ch->GetMapIndex() < 10000)
	{
		TPacketGGWarpCharacter pw;
		pw.header = HEADER_GG_WARP_CHARACTER;
		pw.pid = p->dwFromPID;
		pw.x = ch->GetX();
		pw.y = ch->GetY();
		d->Packet(&pw, sizeof(pw));
	}
}

void CInputP2P::WarpCharacter(const char* c_pData)
{
	TPacketGGWarpCharacter* p = (TPacketGGWarpCharacter*) c_pData;
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(p->pid);
	if (ch)
	{
		ch->WarpSet(p->x, p->y);
	}
}

void CInputP2P::GuildWarZoneMapIndex(const char* c_pData)
{
	TPacketGGGuildWarMapIndex * p = (TPacketGGGuildWarMapIndex*) c_pData;
	CGuildManager & gm = CGuildManager::instance();

	sys_log(0, "P2P: GuildWarZoneMapIndex g1(%u) vs g2(%u), mapIndex(%d)", p->dwGuildID1, p->dwGuildID2, p->lMapIndex);

	CGuild * g1 = gm.FindGuild(p->dwGuildID1);
	CGuild * g2 = gm.FindGuild(p->dwGuildID2);

	if (g1 && g2)
	{
		g1->SetGuildWarMapIndex(p->dwGuildID2, p->lMapIndex);
		g2->SetGuildWarMapIndex(p->dwGuildID1, p->lMapIndex);
	}
}

void CInputP2P::Transfer(const char * c_pData)
{
	TPacketGGTransfer * p = (TPacketGGTransfer *) c_pData;

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(p->szName);

	if (ch)
		ch->WarpSet(p->lX, p->lY);
}

void CInputP2P::XmasWarpSanta(const char * c_pData)
{
	TPacketGGXmasWarpSanta * p =(TPacketGGXmasWarpSanta *) c_pData;

	if (p->bChannel == g_bChannel && map_allow_find(p->lMapIndex))
	{
		// @correction005
		int32_t	iNextSpawnDelay = 50 * 60;

		xmas::SpawnSanta(p->lMapIndex, iNextSpawnDelay);

		TPacketGGXmasWarpSantaReply pack_reply;
		pack_reply.bHeader = HEADER_GG_XMAS_WARP_SANTA_REPLY;
		pack_reply.bChannel = g_bChannel;
		P2P_MANAGER::instance().Send(&pack_reply, sizeof(pack_reply));
	}
}

void CInputP2P::XmasWarpSantaReply(const char* c_pData)
{
	TPacketGGXmasWarpSantaReply* p = (TPacketGGXmasWarpSantaReply*) c_pData;

	if (p->bChannel == g_bChannel)
	{
		CharacterVectorInteractor i;

		if (CHARACTER_MANAGER::instance().GetCharactersByRaceNum(xmas::MOB_SANTA_VNUM, i))
		{
			CharacterVectorInteractor::iterator it = i.begin();

			while (it != i.end()) {
				M2_DESTROY_CHARACTER(*it++);
			}
		}
	}
}

void CInputP2P::LoginPing(LPDESC d, const char * c_pData)
{
	TPacketGGLoginPing * p = (TPacketGGLoginPing *) c_pData;

	SendBillingExpire(p->szLogin, BILLING_DAY, 0, nullptr);

	if (!g_pkAuthMasterDesc)
		P2P_MANAGER::instance().Send(p, sizeof(TPacketGGLoginPing), d);
}

void CInputP2P::BlockChat(const char * c_pData)
{
	TPacketGGBlockChat * p = (TPacketGGBlockChat *) c_pData;

	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindPC(p->szName);

	if (ch)
	{
		sys_log(0, "BLOCK CHAT apply name %s dur %d", p->szName, p->lBlockDuration);
		ch->AddAffect(AFFECT_BLOCK_CHAT, POINT_NONE, 0, AFF_NONE, p->lBlockDuration, 0, true);
	}
	else
	{
		sys_log(0, "BLOCK CHAT fail name %s dur %d", p->szName, p->lBlockDuration);
	}
}

void CInputP2P::PCBangUpdate(const char* c_pData)
{
	TPacketPCBangUpdate* p = (TPacketPCBangUpdate*)c_pData;

	CPCBangManager::instance().RequestUpdateIPList(p->ulPCBangID);
}

void CInputP2P::IamAwake(LPDESC d, const char * c_pData)
{
	std::string hostNames;
	P2P_MANAGER::instance().GetP2PHostNames(hostNames);
	sys_log(0, "P2P Awakeness check from %s. My P2P connection number is %d. and details...\n%s", d->GetHostName(), P2P_MANAGER::instance().GetDescCount(), hostNames.c_str());
}

int32_t CInputP2P::Analyze(LPDESC d, uint8_t bHeader, const char * c_pData)
{
	if (test_server)
		sys_log(0, "CInputP2P::Anlayze[Header %d]", bHeader);

	int32_t iExtraLen = 0;

	switch (bHeader)
	{
		case HEADER_GG_SETUP:
			Setup(d, c_pData);
			break;

		case HEADER_GG_LOGIN:
			Login(d, c_pData);
			break;

		case HEADER_GG_LOGOUT:
			Logout(d, c_pData);
			break;

		case HEADER_GG_RELAY:
			if ((iExtraLen = Relay(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;
#ifdef ENABLE_FULL_NOTICE // @correction188
		case HEADER_GG_BIG_NOTICE:
			if ((iExtraLen = Notice(d, c_pData, m_iBufferLeft, true)) < 0)
				return -1;
			break;
#endif
		case HEADER_GG_NOTICE:
			if ((iExtraLen = Notice(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_GG_SHUTDOWN:
			sys_err("Accept shutdown p2p command from %s.", d->GetHostName());
			Shutdown(10);
			break;

		case HEADER_GG_GUILD:
			if ((iExtraLen = Guild(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_GG_SHOUT:
			Shout(c_pData);
			break;

		case HEADER_GG_DISCONNECT:
			Disconnect(c_pData);
			break;

		case HEADER_GG_MESSENGER_ADD:
			MessengerAdd(c_pData);
			break;

		case HEADER_GG_MESSENGER_REMOVE:
			MessengerRemove(c_pData);
			break;

		case HEADER_GG_FIND_POSITION:
			FindPosition(d, c_pData);
			break;

		case HEADER_GG_WARP_CHARACTER:
			WarpCharacter(c_pData);
			break;

		case HEADER_GG_GUILD_WAR_ZONE_MAP_INDEX:
			GuildWarZoneMapIndex(c_pData);
			break;

		case HEADER_GG_TRANSFER:
			Transfer(c_pData);
			break;

		case HEADER_GG_XMAS_WARP_SANTA:
			XmasWarpSanta(c_pData);
			break;

		case HEADER_GG_XMAS_WARP_SANTA_REPLY:
			XmasWarpSantaReply(c_pData);
			break;

		case HEADER_GG_RELOAD_CRC_LIST:
			LoadValidCRCList();
			break;

		case HEADER_GG_CHECK_CLIENT_VERSION:
			CheckClientVersion();
			break;

		case HEADER_GG_LOGIN_PING:
			LoginPing(d, c_pData);
			break;

		case HEADER_GG_BLOCK_CHAT:
			BlockChat(c_pData);
			break;

		case HEADER_GG_SIEGE:
			{
				TPacketGGSiege* pSiege = (TPacketGGSiege*)c_pData;
				castle_siege(pSiege->bEmpire, pSiege->bTowerCount);
			}
			break;

		case HEADER_GG_MONARCH_NOTICE:
			if ((iExtraLen = MonarchNotice(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_GG_MONARCH_TRANSFER :
			MonarchTransfer(d, c_pData);
			break;

		case HEADER_GG_PCBANG_UPDATE :
			PCBangUpdate(c_pData);
			break;

		case HEADER_GG_CHECK_AWAKENESS:
			IamAwake(d, c_pData);
			break;

// @correction235 BEGIN
		case HEADER_GG_RELOAD_COMMAND:
			ACMD(do_reload);
			do_reload(nullptr, ((TPacketGGReloadCommand*)c_pData)->argument, 0, 0);
			break;
// @correction235 END

#ifdef __OFFLINE_PRIVATE_SHOP__
		case HEADER_GG_OFFLINE_PRIVATE_SHOP_INFO:
		case HEADER_GG_OFFLINE_PRIVATE_SHOP_ITEM_DEL:
		case HEADER_GG_OFFLINE_PRIVATE_SHOP_ITEM_ADD:
		case HEADER_GG_OFFLINE_PRIVATE_SHOP_RENAME:
		case HEADER_GG_OFFLINE_PRIVATE_SHOP_ITEM_SEARCH:
		case HEADER_GG_OFFLINE_PRIVATE_SHOP_ITEM_BUY:
		case HEADER_GG_OFFLINE_PRIVATE_SHOP_UNLOCK_SLOTS:
			OfflinePrivateShop(*(uint8_t*)(c_pData + 1), d, c_pData);
			break;
#endif

#ifdef __BOSS_TRACKING_SYSTEM__
		case HEADER_GG_BOSS_TRACKING:
			BossTracking(d, c_pData);
			break;
#endif
#ifdef __SWITCHBOT__
		case HEADER_GG_SWITCHBOT:
			Switchbot(d, c_pData);
			break;
#endif
	}

	return (iExtraLen);
}

#ifdef __BOSS_TRACKING_SYSTEM__
void CInputP2P::BossTracking(LPDESC d, const char* c_pData)
{
	TPacketGGBossTracking* p = (TPacketGGBossTracking*)c_pData;
	CBossTracking::instance().SetDeadTime(p->channel, p->mob_vnum, p->dead_time, p->map_index);
	CBossTracking::instance().SetRegenTime(p->channel, p->mob_vnum, p->regen_time, p->map_index);
}
#endif

#ifdef __SWITCHBOT__
#include "new_switchbot.h"
void CInputP2P::Switchbot(LPDESC d, const char* c_pData)
{
	const TPacketGGSwitchbot* p = reinterpret_cast<const TPacketGGSwitchbot*>(c_pData);
	if (p->wPort != mother_port)
	{
		return;
	}

	CSwitchbotManager::Instance().P2PReceiveSwitchbot(p->table);
}
#endif

