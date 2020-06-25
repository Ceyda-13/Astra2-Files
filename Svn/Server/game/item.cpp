#include "stdafx.h"
#include "utils.h"
#include "config.h"
#include "char.h"
#include "desc.h"
#include "sectree_manager.h"
#include "packet.h"
#include "protocol.h"
#include "log.h"
#include "skill.h"
#include "unique_item.h"
#include "profiler.h"
#include "marriage.h"
#include "item_addon.h"
#include "dev_log.h"
#include "locale_service.h"
#include "item.h"
#include "item_manager.h"
#include "affect.h"
#include "DragonSoul.h"
#include "buff_on_attributes.h"
#include "belt_inventory_helper.h"
#include "../common/VnumHelper.h"
#include "../common/CommonDefines.h"
#ifdef __OFFLINE_PRIVATE_SHOP__
#include "desc_client.h"
#include "db.h"
#endif
#ifdef __COSTUME_PET__
#include "PetSystem.h"
#endif

CItem::CItem(uint32_t dwVnum)
	: m_dwVnum(dwVnum), m_bWindow(0), m_dwID(0), m_bEquipped(false), m_dwVID(0), m_wCell(0), m_dwCount(0), m_lFlag(0), m_dwLastOwnerPID(0),
	m_bExchanging(false), m_pkDestroyEvent(nullptr), m_pkExpireEvent(nullptr), m_pkUniqueExpireEvent(nullptr),
	m_pkTimerBasedOnWearExpireEvent(nullptr), m_pkRealTimeExpireEvent(nullptr),
   	m_pkAccessorySocketExpireEvent(nullptr), m_pkOwnershipEvent(nullptr), m_dwOwnershipPID(0), m_bSkipSave(false), m_isLocked(false),
	m_dwMaskVnum(0), m_dwSIGVnum (0), m_pkShop(nullptr) // @correction080
#ifdef __SOULBIND__ // @correction202
	, m_lSealbind(0), m_pkUnSealTimerExpireEvent(nullptr)
#endif
#ifdef __CHANGE_LOOK__ // @correction192
	, m_dwChangeLook(0)
#endif
#ifdef __OFFLINE_PRIVATE_SHOP__
	, m_dwShopId(0), m_dwPrices(0), m_dwOwnerPID(0), m_bIsLocked(false)
#endif
{
	memset( &m_alSockets, 0, sizeof(m_alSockets) );
	memset( &m_aAttr, 0, sizeof(m_aAttr) );
}

CItem::~CItem()
{
	Destroy();
}

void CItem::Initialize()
{
	CEntity::Initialize(ENTITY_ITEM);

	m_bWindow = RESERVED_WINDOW;
	m_pOwner = nullptr;
	m_dwID = 0;
	m_bEquipped = false;
	m_dwVID = m_wCell = m_dwCount = m_lFlag = 0;
	m_pProto = nullptr;
	m_bExchanging = false;
	memset(&m_alSockets, 0, sizeof(m_alSockets));
	memset(&m_aAttr, 0, sizeof(m_aAttr));
#ifdef __SOULBIND__ // @correction202
	m_lSealbind = 0;
	m_pkUnSealTimerExpireEvent = nullptr;
#endif
#ifdef __CHANGE_LOOK__ // @correction192
	m_dwChangeLook = 0;
#endif
#ifdef __OFFLINE_PRIVATE_SHOP__
	m_dwShopId = 0;
#endif
	m_pkDestroyEvent = nullptr;
	m_pkOwnershipEvent = nullptr;
	m_dwOwnershipPID = 0;
	m_pkUniqueExpireEvent = nullptr;
	m_pkTimerBasedOnWearExpireEvent = nullptr;
	m_pkRealTimeExpireEvent = nullptr;

	m_pkAccessorySocketExpireEvent = nullptr;

	m_bSkipSave = false;
	m_dwLastOwnerPID = 0;
}

void CItem::Destroy()
{
	event_cancel(&m_pkDestroyEvent);
	event_cancel(&m_pkOwnershipEvent);
	event_cancel(&m_pkUniqueExpireEvent);
	event_cancel(&m_pkTimerBasedOnWearExpireEvent);
	event_cancel(&m_pkRealTimeExpireEvent);
	event_cancel(&m_pkAccessorySocketExpireEvent);
#ifdef __SOULBIND__ // @correction202
	event_cancel(&m_pkUnSealTimerExpireEvent);
#endif
	CEntity::Destroy();

	if (GetSectree())
		GetSectree()->RemoveEntity(this);
}

EVENTFUNC(item_destroy_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "item_destroy_event> <Factor> Null pointer" );
		return 0;
	}

	LPITEM pkItem = info->item;

	if (pkItem->GetOwner())
		sys_err("item_destroy_event: Owner exist. (item %s owner %s)", pkItem->GetName(), pkItem->GetOwner()->GetName());

	pkItem->SetDestroyEvent(nullptr);
	M2_DESTROY_ITEM(pkItem);
	return 0;
}

void CItem::SetDestroyEvent(LPEVENT pkEvent)
{
	m_pkDestroyEvent = pkEvent;
}

void CItem::StartDestroyEvent(int32_t iSec)
{
	if (m_pkDestroyEvent)
		return;

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetDestroyEvent(event_create(item_destroy_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::EncodeInsertPacket(LPENTITY ent)
{
	LPDESC d;

	if (!(d = ent->GetDesc()))
		return;

	const PIXEL_POSITION & c_pos = GetXYZ();

	struct packet_item_ground_add pack;

	pack.bHeader	= HEADER_GC_ITEM_GROUND_ADD;
	pack.x		= c_pos.x;
	pack.y		= c_pos.y;
	pack.z		= c_pos.z;
	pack.dwVnum		= GetVnum();
	pack.dwVID		= m_dwVID;

	d->Packet(&pack, sizeof(pack));

	if (m_pkOwnershipEvent != nullptr)
	{
		item_event_info * info = dynamic_cast<item_event_info *>(m_pkOwnershipEvent->info);

		if ( info == nullptr )
		{
			sys_err( "CItem::EncodeInsertPacket> <Factor> Null pointer" );
			return;
		}

		TPacketGCItemOwnership p;

		p.bHeader = HEADER_GC_ITEM_OWNERSHIP;
		p.dwVID = m_dwVID;
		strlcpy(p.szName, info->szOwnerName, sizeof(p.szName));

		d->Packet(&p, sizeof(TPacketGCItemOwnership));
	}
}

void CItem::EncodeRemovePacket(LPENTITY ent)
{
	LPDESC d;

	if (!(d = ent->GetDesc()))
		return;

	struct packet_item_ground_del pack;

	pack.bHeader	= HEADER_GC_ITEM_GROUND_DEL;
	pack.dwVID		= m_dwVID;

	d->Packet(&pack, sizeof(pack));
	sys_log(2, "Item::EncodeRemovePacket %s to %s", GetName(), ((LPCHARACTER) ent)->GetName());
}

void CItem::SetProto(const TItemTable * table)
{
	assert(table != nullptr);
	m_pProto = table;
	SetFlag(m_pProto->dwFlags);
}

void CItem::UsePacketEncode(LPCHARACTER ch, LPCHARACTER victim, struct packet_item_use *packet)
{
	if (!GetVnum())
		return;

	packet->header 	= HEADER_GC_ITEM_USE;
	packet->ch_vid 	= ch->GetVID();
	packet->victim_vid 	= victim->GetVID();
	packet->Cell = TItemPos(GetWindow(), m_wCell);
	packet->vnum	= GetVnum();
}

void CItem::RemoveFlag(int32_t bit)
{
	REMOVE_BIT(m_lFlag, bit);
}

void CItem::AddFlag(int32_t bit)
{
	SET_BIT(m_lFlag, bit);
}

void CItem::UpdatePacket()
{
	if (!m_pOwner || !m_pOwner->GetDesc())
		return;

#ifdef __SWITCHBOT__
	if (m_bWindow == SWITCHBOT)
		return;
#endif

	TPacketGCItemUpdate pack;

	pack.header = HEADER_GC_ITEM_UPDATE;
	pack.Cell = TItemPos(GetWindow(), m_wCell);
	pack.count	= m_dwCount;
#ifdef __SOULBIND__ // @correction202
	pack.sealbind = m_lSealbind;
#endif
#ifdef __CHANGE_LOOK__ // @correction192
	pack.changelook = m_dwChangeLook;
#endif

	for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		pack.alSockets[i] = m_alSockets[i];

	memcpy(pack.aAttr, GetAttributes(), sizeof(pack.aAttr));

	sys_log(2, "UpdatePacket %s -> %s", GetName(), m_pOwner->GetName());
	m_pOwner->GetDesc()->Packet(&pack, sizeof(pack));
}

uint32_t CItem::GetCount()
{
	if (GetType() == ITEM_ELK) return MIN(m_dwCount, INT_MAX);
	else
	{
		return MIN(m_dwCount, g_wItemCountLimit); // @correction002 200 -> g_wItemCountLimit
	}
}

bool CItem::SetCount(uint32_t count)
{
	if (GetType() == ITEM_ELK)
	{
		m_dwCount = MIN(count, INT_MAX);
	}
	else
	{
		m_dwCount = MIN(count, g_wItemCountLimit); // @correction002 ITEM_MAX_COUNT -> g_wItemCountLimit
	}

	if (count == 0 && m_pOwner)
	{
		if (GetSubType() == USE_ABILITY_UP || GetSubType() == USE_POTION || GetVnum() == 70020)
		{
			LPCHARACTER pOwner = GetOwner();
			uint16_t wCell = GetCell();

			RemoveFromCharacter();

			if (!IsDragonSoul())
			{
				LPITEM pItem = pOwner->FindSpecifyItem(GetVnum());

				if (nullptr != pItem)
				{
					pOwner->ChainQuickslotItem(pItem, QUICKSLOT_TYPE_ITEM, wCell);
				}
				else
				{
					pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, wCell, 255);
				}
			}

			M2_DESTROY_ITEM(this);
		}
		else
		{
			if (!IsDragonSoul())
			{
				m_pOwner->SyncQuickslot(QUICKSLOT_TYPE_ITEM, m_wCell, 255);
			}
			M2_DESTROY_ITEM(RemoveFromCharacter());
		}

		return false;
	}

	UpdatePacket();

	Save();
	return true;
}

#ifdef __CHANGE_LOOK__ // @correction192
void CItem::SetChangeLook(uint32_t dwVnum, bool bLog)
{
	m_dwChangeLook = dwVnum;
	UpdatePacket();
	Save();
}
#endif

LPITEM CItem::RemoveFromCharacter()
{
	if (!m_pOwner)
	{
		sys_err("Item::RemoveFromCharacter owner null");
		return (this);
	}

	LPCHARACTER pOwner = m_pOwner;

	if (m_bEquipped)
	{
		Unequip();

		SetWindow(RESERVED_WINDOW);
		Save();
		return (this);
	}
	else
	{
		if (GetWindow() != SAFEBOX && GetWindow() != MALL)
		{
			if (IsDragonSoul())
			{
				if (m_wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
					sys_err("CItem::RemoveFromCharacter: pos >= DRAGON_SOUL_INVENTORY_MAX_NUM");
				else
					pOwner->SetItem(TItemPos(m_bWindow, m_wCell), nullptr);
			}
#ifdef __SWITCHBOT__
			else if (m_bWindow == SWITCHBOT)
			{
				if (m_wCell >= SWITCHBOT_SLOT_COUNT)
				{
					sys_err("CItem::RemoveFromCharacter: pos >= SWITCHBOT_SLOT_COUNT");
				}
				else
				{
					pOwner->SetItem(TItemPos(SWITCHBOT, m_wCell), nullptr);
				}
			}
#endif
			else
			{
				TItemPos cell(INVENTORY, m_wCell);

				if (false == cell.IsDefaultInventoryPosition() && false == cell.IsBeltInventoryPosition())
					sys_err("CItem::RemoveFromCharacter: Invalid Item Position");
				else
				{
					pOwner->SetItem(cell, nullptr);
				}
			}
		}

		m_pOwner = nullptr;
		m_wCell = 0;

		SetWindow(RESERVED_WINDOW);
		Save();
		return (this);
	}
}

bool CItem::AddToCharacter(LPCHARACTER ch, TItemPos Cell, bool isHighLight)
{
	assert(GetSectree() == nullptr);
	assert(m_pOwner == nullptr);
	uint16_t pos = Cell.cell;
	uint8_t window_type = Cell.window_type;

	if (INVENTORY == window_type)
	{
		if (m_wCell >= INVENTORY_MAX_NUM && BELT_INVENTORY_SLOT_START > m_wCell)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
	else if (DRAGON_SOUL_INVENTORY == window_type)
	{
		if (m_wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
		{
			sys_err("CItem::AddToCharacter: cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
#ifdef __SWITCHBOT__
	else if (SWITCHBOT == window_type)
	{
		if (m_wCell >= SWITCHBOT_SLOT_COUNT)
		{
			sys_err("CItem::AddToCharacter:switchbot cell overflow: %s to %s cell %d", m_pProto->szName, ch->GetName(), m_wCell);
			return false;
		}
	}
#endif

	if (ch->GetDesc())
		m_dwLastOwnerPID = ch->GetPlayerID();

#ifdef __ACCE_SYSTEM__ // @correction191
	if ((GetType() == ITEM_COSTUME) && (GetSubType() == COSTUME_ACCE) && (GetSocket(ACCE_ABSORPTION_SOCKET) == 0))
	{
		int32_t lVal = FindApplyValue(APPLY_ACCEDRAIN_RATE);
		SetSocket(ACCE_ABSORPTION_SOCKET, lVal);
	}
#endif

	event_cancel(&m_pkDestroyEvent);
	ch->SetItem(TItemPos(window_type, pos), this, isHighLight);
	m_pOwner = ch;

	Save();
	return true;
}

LPITEM CItem::RemoveFromGround()
{
	if (GetSectree())
	{
		SetOwnership(nullptr);

		GetSectree()->RemoveEntity(this);

		ViewCleanup();

		Save();
	}

	return (this);
}

bool CItem::AddToGround(int32_t lMapIndex, const PIXEL_POSITION & pos, bool skipOwnerCheck)
{
	if (0 == lMapIndex)
	{
		sys_err("wrong map index argument: %d", lMapIndex);
		return false;
	}

	if (GetSectree())
	{
		sys_err("sectree already assigned");
		return false;
	}

	if (!skipOwnerCheck && m_pOwner)
	{
		sys_err("owner pointer not null");
		return false;
	}

	LPSECTREE tree = SECTREE_MANAGER::instance().Get(lMapIndex, pos.x, pos.y);

	if (!tree)
	{
		sys_err("cannot find sectree by %dx%d", pos.x, pos.y);
		return false;
	}


	SetWindow(GROUND);
	SetXYZ(pos.x, pos.y, pos.z);
	tree->InsertEntity(this);
	UpdateSectree();
	Save();
	return true;
}

bool CItem::DistanceValid(LPCHARACTER ch)
{
	if (!GetSectree())
		return false;

	int32_t iDist = DISTANCE_APPROX(GetX() - ch->GetX(), GetY() - ch->GetY());

	if (iDist > 1000)
		return false;

	return true;
}

bool CItem::CanUsedBy(LPCHARACTER ch)
{
	switch (ch->GetJob())
	{
		case JOB_WARRIOR:
			if (GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR)
				return false;
			break;

		case JOB_ASSASSIN:
			if (GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN)
				return false;
			break;

		case JOB_SHAMAN:
			if (GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
				return false;
			break;

		case JOB_SURA:
			if (GetAntiFlag() & ITEM_ANTIFLAG_SURA)
				return false;
			break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case JOB_WOLFMAN:
			if (GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN)
				return false;
			break;
#endif
	}

	return true;
}

int32_t CItem::FindEquipCell(LPCHARACTER ch, int32_t iCandidateCell)
{
	if ((0 == GetWearFlag() || ITEM_TOTEM == GetType()) && ITEM_COSTUME != GetType() && ITEM_DS != GetType() && ITEM_SPECIAL_DS != GetType() && ITEM_RING != GetType() && ITEM_BELT != GetType()
#ifdef __COSTUME_PET__
&& (GetType() != ITEM_PET && GetSubType() != PET_PAY)
#endif
)		return -1;

	if (GetType() == ITEM_DS || GetType() == ITEM_SPECIAL_DS)
	{
		if (iCandidateCell < 0)
		{
			return WEAR_MAX_NUM + GetSubType();
		}
		else
		{
			for (int32_t i = 0; i < DRAGON_SOUL_DECK_MAX_NUM; i++)
			{
				if (WEAR_MAX_NUM + i * DS_SLOT_MAX + GetSubType() == iCandidateCell)
				{
					return iCandidateCell;
				}
			}
			return -1;
		}
	}
	else if (GetType() == ITEM_COSTUME)
	{
		if (GetSubType() == COSTUME_BODY)
			return WEAR_COSTUME_BODY;
		else if (GetSubType() == COSTUME_HAIR)
			return WEAR_COSTUME_HAIR;
#ifdef __COSTUME_MOUNT__
		else if (GetSubType() == COSTUME_MOUNT)
			return WEAR_COSTUME_MOUNT;
#endif
#ifdef __ACCE_SYSTEM__ // @correction191
		else if (GetSubType() == COSTUME_ACCE)
			return WEAR_COSTUME_ACCE;
#endif
#ifdef __COSTUME_WEAPON__ // @correction204
		else if (GetSubType() == COSTUME_WEAPON)
			return WEAR_COSTUME_WEAPON;
#endif
#ifdef __COSTUME_EFFECT__
		else if (GetSubType() == COSTUME_EFFECT)
		{
			if (GetWearFlag() & WEARABLE_COSTUME_EFFECT_WEAPON)
				return WEAR_COSTUME_EFFECT_WEAPON;
			else
				return WEAR_COSTUME_EFFECT_BODY;
		}
#endif
	}
	else if (GetType() == ITEM_RING)
	{
		if (ch->GetWear(WEAR_RING1))
			return WEAR_RING2;
		else
			return WEAR_RING1;
	}
	else if (GetType() == ITEM_BELT)
		return WEAR_BELT;
	else if (GetWearFlag() & WEARABLE_BODY)
		return WEAR_BODY;
	else if (GetWearFlag() & WEARABLE_HEAD)
		return WEAR_HEAD;
	else if (GetWearFlag() & WEARABLE_FOOTS)
		return WEAR_FOOTS;
	else if (GetWearFlag() & WEARABLE_WRIST)
		return WEAR_WRIST;
	else if (GetWearFlag() & WEARABLE_WEAPON)
		return WEAR_WEAPON;
	else if (GetWearFlag() & WEARABLE_SHIELD)
		return WEAR_SHIELD;
	else if (GetWearFlag() & WEARABLE_NECK)
		return WEAR_NECK;
	else if (GetWearFlag() & WEARABLE_EAR)
		return WEAR_EAR;
	else if (GetWearFlag() & WEARABLE_ARROW)
		return WEAR_ARROW;
	else if (GetWearFlag() & WEARABLE_UNIQUE)
	{
		if (ch->GetWear(WEAR_UNIQUE1))
			return WEAR_UNIQUE2;
		else
			return WEAR_UNIQUE1;
	}

	else if (GetWearFlag() & WEARABLE_ABILITY)
	{
		if (!ch->GetWear(WEAR_ABILITY1))
		{
			return WEAR_ABILITY1;
		}
		else if (!ch->GetWear(WEAR_ABILITY2))
		{
			return WEAR_ABILITY2;
		}
		else if (!ch->GetWear(WEAR_ABILITY3))
		{
			return WEAR_ABILITY3;
		}
		else if (!ch->GetWear(WEAR_ABILITY4))
		{
			return WEAR_ABILITY4;
		}
		else if (!ch->GetWear(WEAR_ABILITY5))
		{
			return WEAR_ABILITY5;
		}
		else if (!ch->GetWear(WEAR_ABILITY6))
		{
			return WEAR_ABILITY6;
		}
		else if (!ch->GetWear(WEAR_ABILITY7))
		{
			return WEAR_ABILITY7;
		}
		else if (!ch->GetWear(WEAR_ABILITY8))
		{
			return WEAR_ABILITY8;
		}
		else
		{
			return -1;
		}
	}
#ifdef __PENDANT__
	else if (GetWearFlag() & WEARABLE_PENDANT)
		return WEAR_PENDANT;
#endif
#ifdef __COSTUME_PET__
	else if (GetType() == ITEM_PET && GetSubType() == PET_PAY)
		return WEAR_PET;
#endif
	return -1;
}

void CItem::ModifyPoints(bool bAdd)
{
	int32_t accessoryGrade;

	if (false == IsAccessoryForSocket())
	{
		if (m_pProto->bType == ITEM_WEAPON || m_pProto->bType == ITEM_ARMOR)
		{
			for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
			{
				uint32_t dwVnum;

				if ((dwVnum = GetSocket(i)) <= 2)
					continue;

				TItemTable * p = ITEM_MANAGER::instance().GetTable(dwVnum);

				if (!p)
				{
#ifdef __QUIVER__ // @correction203
					if (m_pProto->bSubType != WEAPON_QUIVER)
#endif
						sys_err("cannot find table by vnum %u", dwVnum);
					continue;
				}

				if (ITEM_METIN == p->bType)
				{
					for (int32_t i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
					{
						if (p->aApplies[i].bType == APPLY_NONE)
							continue;

						if (p->aApplies[i].bType == APPLY_SKILL)
							m_pOwner->ApplyPoint(p->aApplies[i].bType, bAdd ? p->aApplies[i].lValue : p->aApplies[i].lValue ^ 0x00800000);
						else
							m_pOwner->ApplyPoint(p->aApplies[i].bType, bAdd ? p->aApplies[i].lValue : -p->aApplies[i].lValue);
					}
				}
			}
		}

		accessoryGrade = 0;
	}
	else
	{
		accessoryGrade = MIN(GetAccessorySocketGrade(), ITEM_ACCESSORY_SOCKET_MAX_NUM);
	}

#ifdef __ACCE_SYSTEM__ // @correction191
	if ((GetType() == ITEM_COSTUME) && (GetSubType() == COSTUME_ACCE) && (GetSocket(ACCE_ABSORBED_SOCKET)))
	{
		TItemTable * pkItemAbsorbed = ITEM_MANAGER::instance().GetTable(GetSocket(ACCE_ABSORBED_SOCKET));
		if (pkItemAbsorbed)
		{
			if ((pkItemAbsorbed->bType == ITEM_ARMOR) && (pkItemAbsorbed->bSubType == ARMOR_BODY))
			{
				int32_t lDefGrade = pkItemAbsorbed->alValues[1] + int32_t(pkItemAbsorbed->alValues[5] * 2);
				double dValue = lDefGrade * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				lDefGrade = (int32_t) dValue;
				if ((pkItemAbsorbed->alValues[1] > 0) && (lDefGrade <= 0 || pkItemAbsorbed->alValues[5] > 0) && (lDefGrade < 1))
					lDefGrade += 1;
				else if ((pkItemAbsorbed->alValues[1] > 0) || (pkItemAbsorbed->alValues[5] > 0))
					lDefGrade += 1;
				
				m_pOwner->ApplyPoint(APPLY_DEF_GRADE_BONUS, bAdd ? lDefGrade : -lDefGrade);
				
				int32_t lDefMagicBonus = pkItemAbsorbed->alValues[0];
				dValue = lDefMagicBonus * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				lDefMagicBonus = (int32_t) dValue;
				if ((pkItemAbsorbed->alValues[0] > 0) && (lDefMagicBonus < 1))
					lDefMagicBonus += 1;
				else if (pkItemAbsorbed->alValues[0] > 0)
					lDefMagicBonus += 1;
				
				m_pOwner->ApplyPoint(APPLY_MAGIC_DEF_GRADE, bAdd ? lDefMagicBonus : -lDefMagicBonus);
			}
			else if (pkItemAbsorbed->bType == ITEM_WEAPON)
			{
				int32_t lAttGrade = pkItemAbsorbed->alValues[4] + pkItemAbsorbed->alValues[5];
				if (pkItemAbsorbed->alValues[3] > pkItemAbsorbed->alValues[4])
					lAttGrade = pkItemAbsorbed->alValues[3] + pkItemAbsorbed->alValues[5];
				
				double dValue = lAttGrade * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				lAttGrade = (int32_t) dValue;
				if (((pkItemAbsorbed->alValues[3] > 0) && (lAttGrade < 1)) || ((pkItemAbsorbed->alValues[4] > 0) && (lAttGrade < 1)))
					lAttGrade += 1;
				else if ((pkItemAbsorbed->alValues[3] > 0) || (pkItemAbsorbed->alValues[4] > 0))
					lAttGrade += 1;
				
				m_pOwner->ApplyPoint(APPLY_ATT_GRADE_BONUS, bAdd ? lAttGrade : -lAttGrade);
				
				int32_t lAttMagicGrade = pkItemAbsorbed->alValues[2] + pkItemAbsorbed->alValues[5];
				if (pkItemAbsorbed->alValues[1] > pkItemAbsorbed->alValues[2])
					lAttMagicGrade = pkItemAbsorbed->alValues[1] + pkItemAbsorbed->alValues[5];
				
				dValue = lAttMagicGrade * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				lAttMagicGrade = (int32_t) dValue;
				if (((pkItemAbsorbed->alValues[1] > 0) && (lAttMagicGrade < 1)) || ((pkItemAbsorbed->alValues[2] > 0) && (lAttMagicGrade < 1)))
					lAttMagicGrade += 1;
				else if ((pkItemAbsorbed->alValues[1] > 0) || (pkItemAbsorbed->alValues[2] > 0))
					lAttMagicGrade += 1;
				
				m_pOwner->ApplyPoint(APPLY_MAGIC_ATT_GRADE, bAdd ? lAttMagicGrade : -lAttMagicGrade);
			}
		}
	}
#endif

	for (int32_t i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
#ifdef __ACCE_SYSTEM__ // @correction191
		if ((m_pProto->aApplies[i].bType == APPLY_NONE) && (GetType() != ITEM_COSTUME) && (GetSubType() != COSTUME_ACCE))
#else
		if (m_pProto->aApplies[i].bType == APPLY_NONE)
#endif
			continue;
		
		uint8_t bType = m_pProto->aApplies[i].bType;
		int32_t value = m_pProto->aApplies[i].lValue;
#ifdef __ACCE_SYSTEM__ // @correction191
		if ((GetType() == ITEM_COSTUME) && (GetSubType() == COSTUME_ACCE))
		{
			TItemTable * pkItemAbsorbed = ITEM_MANAGER::instance().GetTable(GetSocket(ACCE_ABSORBED_SOCKET));
			if (pkItemAbsorbed)
			{
				if (pkItemAbsorbed->aApplies[i].bType == APPLY_NONE)
					continue;
				
				bType = pkItemAbsorbed->aApplies[i].bType;
				value = pkItemAbsorbed->aApplies[i].lValue;
				if (value < 0)
					continue;
				
				double dValue = value * GetSocket(ACCE_ABSORPTION_SOCKET);
				dValue = (double)dValue / 100;
				dValue = (double)dValue + .5;
				value = (int32_t) dValue;
				if ((pkItemAbsorbed->aApplies[i].lValue > 0) && (value <= 0))
					value += 1;
			}
			else
				continue;
		}
#endif
		if (bType != APPLY_SKILL)
		{
			if (accessoryGrade != 0 && i < ITEM_APPLY_MAX_NUM - 1) // @correction022
				value += MAX(accessoryGrade, value * aiAccessorySocketEffectivePct[accessoryGrade] / 100);
			
			m_pOwner->ApplyPoint(bType, bAdd ? value : -value);
		}
		else
			m_pOwner->ApplyPoint(bType, bAdd ? value : value ^ 0x00800000);
	}
	if (true == CItemVnumHelper::IsRamadanMoonRing(GetVnum()) || true == CItemVnumHelper::IsHalloweenCandy(GetVnum())
		|| true == CItemVnumHelper::IsHappinessRing(GetVnum()) || true == CItemVnumHelper::IsLovePendant(GetVnum()))
	{
	}
	else
	{
		for (int32_t i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
		{
			if (GetAttributeType(i))
			{
				const TPlayerItemAttribute& ia = GetAttribute(i);
				int32_t sValue = ia.sValue;
#ifdef __ACCE_SYSTEM__ // @correction191
				if ((GetType() == ITEM_COSTUME) && (GetSubType() == COSTUME_ACCE))
				{
					double dValue = sValue * GetSocket(ACCE_ABSORPTION_SOCKET);
					dValue = (double)dValue / 100;
					dValue = (double)dValue + .5;
					sValue = (int32_t) dValue;
					if ((ia.sValue > 0) && (sValue <= 0))
						sValue += 1;
				}
#endif
				if (ia.bType == APPLY_SKILL)
					m_pOwner->ApplyPoint(ia.bType, bAdd ? sValue : sValue ^ 0x00800000);
				else
					m_pOwner->ApplyPoint(ia.bType, bAdd ? sValue : -sValue);
			}
		}
	}

	switch (m_pProto->bType)
	{
		case ITEM_PICK:
		case ITEM_ROD:
			{
				if (bAdd)
				{
					if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
						m_pOwner->SetPart(PART_WEAPON, GetVnum());
				}
				else
				{
					if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
#ifdef __COSTUME_WEAPON__ // @correction204
						m_pOwner->SetPart(PART_WEAPON, 0);
#else
						m_pOwner->SetPart(PART_WEAPON, m_pOwner->GetOriginalPart(PART_WEAPON));
#endif
				}
			}
			break;

		case ITEM_WEAPON:
			{
#ifdef __QUIVER__ // @correction203
				if (m_pProto->bSubType == WEAPON_ARROW || m_pProto->bSubType == WEAPON_QUIVER)
				{
					if (bAdd)
					{
						if (m_wCell == INVENTORY_MAX_NUM + WEAR_ARROW)
						{
							m_pOwner->SetPart(PART_ARROW_TYPE, m_pProto->bSubType);
							const CItem* pWeapon = m_pOwner->GetWear(WEAR_WEAPON);
							if (pWeapon != nullptr && pWeapon->GetSubType() == WEAPON_BOW)
							{
#ifdef __COSTUME_WEAPON__ // @correction204
								const CItem* pCostumeWeapon = m_pOwner->GetWear(WEAR_COSTUME_WEAPON);
								if (pCostumeWeapon != nullptr)
									m_pOwner->SetPart(PART_WEAPON, pCostumeWeapon->GetVnum());
								else
									m_pOwner->SetPart(PART_WEAPON, pWeapon->GetVnum());
#else
								m_pOwner->SetPart(PART_WEAPON, pWeapon->GetVnum());
#endif
							}
						}
					}
					else
					{
						if (m_wCell == INVENTORY_MAX_NUM + WEAR_ARROW)
							m_pOwner->SetPart(PART_ARROW_TYPE, m_pOwner->GetOriginalPart(PART_ARROW_TYPE));
					}
					
					break;
				}

				const CItem* pArrow = m_pOwner->GetWear(WEAR_ARROW);
#endif
#ifdef __COSTUME_WEAPON__ // @correction204
				if (m_pOwner->GetWear(WEAR_COSTUME_WEAPON) != 0)
					break;
#endif
				if (bAdd)
				{
#ifdef __QUIVER__ // @correction203
					if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
						if (pArrow != nullptr)
							m_pOwner->SetPart(PART_ARROW_TYPE, pArrow->GetSubType());
#endif
					if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
#ifdef __CHANGE_LOOK__ // @correction192
					{
						uint32_t dwRes = GetChangeLook() != 0 ? GetChangeLook() : GetVnum();
						m_pOwner->SetPart(PART_WEAPON, dwRes);
					}
#else
						m_pOwner->SetPart(PART_WEAPON, GetVnum());
#endif
				}
				else
				{
					if (m_wCell == INVENTORY_MAX_NUM + WEAR_WEAPON)
#ifdef __COSTUME_WEAPON__ // @correction204
						m_pOwner->SetPart(PART_WEAPON, 0);
#else
						m_pOwner->SetPart(PART_WEAPON, m_pOwner->GetOriginalPart(PART_WEAPON));
#endif
				}
			}
			break;

		case ITEM_ARMOR:
			{
				if (0 != m_pOwner->GetWear(WEAR_COSTUME_BODY))
					break;

				if (GetSubType() == ARMOR_BODY || GetSubType() == ARMOR_HEAD || GetSubType() == ARMOR_FOOTS || GetSubType() == ARMOR_SHIELD)
				{
					if (bAdd)
					{
						if (GetProto()->bSubType == ARMOR_BODY)
#ifdef __CHANGE_LOOK__ // @correction192
						{
							uint32_t dwRes = GetChangeLook() != 0 ? GetChangeLook() : GetVnum();
							m_pOwner->SetPart(PART_MAIN, dwRes);
						}
#else
							m_pOwner->SetPart(PART_MAIN, GetVnum());
#endif
					}
					else
					{
						if (GetProto()->bSubType == ARMOR_BODY)
							m_pOwner->SetPart(PART_MAIN, m_pOwner->GetOriginalPart(PART_MAIN));
					}
				}
			}
			break;

		case ITEM_COSTUME:
			{
				uint32_t toSetValue = this->GetVnum();
				EParts toSetPart = PART_MAX_NUM;

				if (GetSubType() == COSTUME_BODY)
				{
					toSetPart = PART_MAIN;
					if (false == bAdd)
					{
						const CItem* pArmor = m_pOwner->GetWear(WEAR_BODY);
						toSetValue = (nullptr != pArmor) ? pArmor->GetVnum() : m_pOwner->GetOriginalPart(PART_MAIN);
#ifdef __CHANGE_LOOK__ // @correction192
						if (pArmor)
							toSetValue = pArmor->GetChangeLook() != 0 ? pArmor->GetChangeLook() : pArmor->GetVnum();
#endif
					}
#ifdef __CHANGE_LOOK__ // @correction192
					else
						toSetValue = GetChangeLook() != 0 ? GetChangeLook() : GetVnum();
#endif

				}
				else if (GetSubType() == COSTUME_HAIR)
				{
					toSetPart = PART_HAIR;
					if (!bAdd)
						toSetValue = 0;
					else
					{
#ifdef __CHANGE_LOOK__ // @correction192
						uint32_t dwChangeLook = GetChangeLook();
						if (dwChangeLook != 0)
						{
							TItemTable* pItemTable = ITEM_MANAGER::instance().GetTable(dwChangeLook);
							toSetValue = (pItemTable != nullptr) ? pItemTable->alValues[3] : GetValue(3);
						}
						else
							toSetValue = GetValue(3);
#else
						toSetValue = GetValue(3);
#endif
					}
				}
#ifdef __ACCE_SYSTEM__ // @correction191
				else if (GetSubType() == COSTUME_ACCE)
				{
					toSetValue -= 85000;
					if (GetSocket(ACCE_ABSORPTION_SOCKET) >= ACCE_EFFECT_FROM_ABS)
						toSetValue += 10000;
					
					toSetValue = bAdd ? toSetValue : 0;
					toSetPart = PART_ACCE;
				}
#endif
#ifdef __COSTUME_WEAPON__ // @correction204
				else if (GetSubType() == COSTUME_WEAPON)
				{
					toSetPart = PART_WEAPON;
					if (!bAdd)
					{
						const CItem* pWeapon = m_pOwner->GetWear(WEAR_WEAPON);
						toSetValue = (nullptr != pWeapon) ? pWeapon->GetVnum() : m_pOwner->GetPart(PART_WEAPON);
#ifdef __CHANGE_LOOK__
						if (pWeapon)
							toSetValue = pWeapon->GetChangeLook() != 0 ? pWeapon->GetChangeLook() : pWeapon->GetVnum();
#endif	
					}
#ifdef __CHANGE_LOOK__
					else
						toSetValue = GetChangeLook() != 0 ? GetChangeLook() : GetVnum();
#endif
				}
#endif
#ifdef __COSTUME_EFFECT__
				else if (GetSubType() == COSTUME_EFFECT)
				{
					if (GetWearFlag() & WEARABLE_COSTUME_EFFECT_ARMOR)
					{
						if (!bAdd)
							m_pOwner->SetPart((uint8_t)PART_BODY_EFFECT, 0);
						else
							m_pOwner->SetPart((uint8_t)PART_BODY_EFFECT, GetVnum());
					}
					else
					{
						if (!bAdd)
						{
							m_pOwner->SetPart((uint8_t)PART_WEAPON_RIGHT_EFFECT, 0);
							m_pOwner->SetPart((uint8_t)PART_WEAPON_LEFT_EFFECT, 0);
						}
						else
						{
							const CItem* pWeaponEffectCheck = m_pOwner->GetWear(WEAR_WEAPON);
							if (pWeaponEffectCheck != nullptr)
							{
								// uint32_t dwEffectRight = 0;
								// uint32_t dwEffectLeft = 0;
								// if (pWeaponEffectCheck->GetSubType() == WEAPON_DAGGER)
								// {
									// dwEffectRight = GetValue(1);
									// dwEffectLeft = GetValue(2);
								// }
								// else if (pWeaponEffectCheck->GetSubType() == WEAPON_BOW)
								// {
									// dwEffectRight = GetValue(3);
								// }
								// else if (pWeaponEffectCheck->GetSubType() == WEAPON_BELL)
								// {
									// dwEffectRight = GetValue(1);
								// }
								// else if (pWeaponEffectCheck->GetSubType() == WEAPON_FAN)
								// {
									// dwEffectRight = GetValue(4);
								// }
// #ifdef ENABLE_WOLFMAN_CHARACTER
								// else if (pWeaponEffectCheck->GetSubType() == WEAPON_CLAW)
								// {
									// dwEffectRight = GetValue(1);
									// dwEffectLeft = GetValue(2);
								// }
// #endif
								// else
								// {
									// dwEffectRight = GetValue(0);
								// }
								
								m_pOwner->SetPart((uint8_t)PART_WEAPON_RIGHT_EFFECT, GetVnum());
								m_pOwner->SetPart((uint8_t)PART_WEAPON_LEFT_EFFECT, GetVnum());
							}
						}
					}
					
					m_pOwner->UpdatePacket();
				}
#endif
				if (PART_MAX_NUM != toSetPart)
				{
					m_pOwner->SetPart((uint8_t)toSetPart, toSetValue);
					m_pOwner->UpdatePacket();
				}
			}
			break;
		case ITEM_UNIQUE:
			{
				if (0 != GetSIGVnum())
				{
					const CSpecialItemGroup* pItemGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(GetSIGVnum());
					if (nullptr == pItemGroup)
						break;
					uint32_t dwAttrVnum = pItemGroup->GetAttrVnum(GetVnum());
					const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::instance().GetSpecialAttrGroup(dwAttrVnum);
					if (nullptr == pAttrGroup)
						break;
					for (itertype (pAttrGroup->m_vecAttrs) it = pAttrGroup->m_vecAttrs.begin(); it != pAttrGroup->m_vecAttrs.end(); it++)
					{
						m_pOwner->ApplyPoint(it->apply_type, bAdd ? it->apply_value : -it->apply_value);
					}
				}
			}
			break;
	}
}

bool CItem::IsEquipable() const
{
	switch (this->GetType())
	{
	case ITEM_COSTUME:
	case ITEM_ARMOR:
	case ITEM_WEAPON:
	case ITEM_ROD:
	case ITEM_PICK:
	case ITEM_UNIQUE:
	case ITEM_DS:
	case ITEM_SPECIAL_DS:
	case ITEM_RING:
	case ITEM_BELT:
#ifdef __COSTUME_PET__
	case ITEM_PET:
#endif
		return true;
	}

	return false;
}

bool CItem::EquipTo(LPCHARACTER ch, uint8_t bWearCell)
{
	if (!ch)
	{
		sys_err("EquipTo: nil character");
		return false;
	}

	if (IsDragonSoul())
	{
		if (bWearCell < WEAR_MAX_NUM || bWearCell >= WEAR_MAX_NUM + DRAGON_SOUL_DECK_MAX_NUM * DS_SLOT_MAX)
		{
			sys_err("EquipTo: invalid dragon soul cell (this: #%d %s wearflag: %d cell: %d)", GetOriginalVnum(), GetName(), GetSubType(), bWearCell - WEAR_MAX_NUM);
			return false;
		}
	}
	else
	{
		if (bWearCell >= WEAR_MAX_NUM)
		{
			sys_err("EquipTo: invalid wear cell (this: #%d %s wearflag: %d cell: %d)", GetOriginalVnum(), GetName(), GetWearFlag(), bWearCell);
			return false;
		}
	}

	if (ch->GetWear(bWearCell))
	{
		sys_err("EquipTo: item already exist (this: #%d %s cell: %d %s)", GetOriginalVnum(), GetName(), bWearCell, ch->GetWear(bWearCell)->GetName());
		return false;
	}

	if (GetOwner())
		RemoveFromCharacter();

	ch->SetWear(bWearCell, this);

	m_pOwner = ch;
	m_bEquipped = true;
	m_wCell	= INVENTORY_MAX_NUM + bWearCell;

#ifdef __COSTUME_PET__
	if (GetType() == ITEM_PET && GetSubType() == PET_PAY)
	{
		CPetSystem* petSystem = ch->GetPetSystem();
#ifdef __CHANGE_LOOK__
		uint32_t dwVnum = GetChangeLook() ? GetChangeLook() : GetValue(0);
#else
		uint32_t dwVnum = GetValue(0);
#endif
		if (petSystem)
			petSystem->Summon(dwVnum, this, GetName(), false);
	}
#endif

#ifndef ENABLE_IMMUNE_FIX // @correction153
	uint32_t dwImmuneFlag = 0;

	for (int32_t i = 0; i < WEAR_MAX_NUM; ++i)
	{
		if (m_pOwner->GetWear(i))
		{
			SET_BIT(dwImmuneFlag, m_pOwner->GetWear(i)->m_pProto->dwImmuneFlag);
		}
	}

	m_pOwner->SetImmuneFlag(dwImmuneFlag);
#endif

	if (IsDragonSoul())
	{
		DSManager::instance().ActivateDragonSoul(this);
	}
	else
	{
#ifdef __COSTUME_PET__
		if (!(GetType() == ITEM_PET && GetSubType() == PET_PAY)
#ifdef __COSTUME_MOUNT__
&& !(ITEM_COSTUME == GetType() && COSTUME_MOUNT == GetSubType())
#endif
)
			ModifyPoints(true);
#else
#ifdef __COSTUME_MOUNT__
		if (!(ITEM_COSTUME == GetType() && COSTUME_MOUNT == GetSubType()))
			ModifyPoints(true);
#else
		ModifyPoints(true);
#endif
#endif
		StartUniqueExpireEvent();
		if (-1 != GetProto()->cLimitTimerBasedOnWearIndex)
			StartTimerBasedOnWearExpireEvent();

		StartAccessorySocketExpireEvent();
	}

	ch->BuffOnAttr_AddBuffsFromItem(this);

	m_pOwner->ComputeBattlePoints();

	m_pOwner->UpdatePacket();

	Save();

	return (true);
}

bool CItem::Unequip()
{
	if (!m_pOwner || GetCell() < INVENTORY_MAX_NUM)
	{
		sys_err("%s %u m_pOwner %p, GetCell %d",
				GetName(), GetID(), get_pointer(m_pOwner), GetCell());
		return false;
	}

	if (this != m_pOwner->GetWear(GetCell() - INVENTORY_MAX_NUM))
	{
		sys_err("m_pOwner->GetWear() != this");
		return false;
	}

#ifdef __COSTUME_PET__
	if (GetType() == ITEM_PET && GetSubType() == PET_PAY)
	{
		CPetSystem* petSystem = m_pOwner->GetPetSystem();
#ifdef __CHANGE_LOOK__
		uint32_t dwVnum = GetChangeLook() ? GetChangeLook() : GetValue(0);
#else
		uint32_t dwVnum = GetValue(0);
#endif
		if (petSystem)
			petSystem->Unsummon(dwVnum);
	}
#endif

	if (IsRideItem())
		ClearMountAttributeAndAffect();

	if (IsDragonSoul())
	{
		DSManager::instance().DeactivateDragonSoul(this);
	}
	else
	{
		ModifyPoints(false);
	}

	StopUniqueExpireEvent();

	if (-1 != GetProto()->cLimitTimerBasedOnWearIndex)
		StopTimerBasedOnWearExpireEvent();

	StopAccessorySocketExpireEvent();


	m_pOwner->BuffOnAttr_RemoveBuffsFromItem(this);

	m_pOwner->SetWear(GetCell() - INVENTORY_MAX_NUM, nullptr);

#ifndef ENABLE_IMMUNE_FIX // @correction153
	uint32_t dwImmuneFlag = 0;

	for (int32_t i = 0; i < WEAR_MAX_NUM; ++i)
	{
		if (m_pOwner->GetWear(i))
		{
			SET_BIT(dwImmuneFlag, m_pOwner->GetWear(i)->m_pProto->dwImmuneFlag);
		}
	}

	m_pOwner->SetImmuneFlag(dwImmuneFlag);
#endif

	m_pOwner->ComputeBattlePoints();

	m_pOwner->UpdatePacket();

	m_pOwner = nullptr;
	m_wCell = 0;
	m_bEquipped	= false;

	return true;
}

int32_t CItem::GetValue(uint32_t idx)
{
	assert(idx < ITEM_VALUES_MAX_NUM);
	return GetProto()->alValues[idx];
}

void CItem::SetExchanging(bool bOn)
{
	m_bExchanging = bOn;
}

void CItem::Save()
{
	if (m_bSkipSave)
		return;

	ITEM_MANAGER::instance().DelayedSave(this);
}

bool CItem::CreateSocket(uint8_t bSlot, uint8_t bGold)
{
	assert(bSlot < ITEM_SOCKET_MAX_NUM);

	if (m_alSockets[bSlot] != 0)
	{
		sys_err("Item::CreateSocket : socket already exist %s %d", GetName(), bSlot);
		return false;
	}

	if (bGold)
		m_alSockets[bSlot] = 2;
	else
		m_alSockets[bSlot] = 1;

	UpdatePacket();

	Save();
	return true;
}

void CItem::SetSockets(const int32_t * c_al)
{
	memcpy(m_alSockets, c_al, sizeof(m_alSockets));
	Save();
}

void CItem::SetSocket(int32_t i, int32_t v, bool bLog)
{
	assert(i < ITEM_SOCKET_MAX_NUM);
	m_alSockets[i] = v;
	UpdatePacket();
	Save();
	if (bLog)
		if (g_iDbLogLevel >= LOG_LEVEL_MAX) // @correction222
			LogManager::instance().ItemLog(i, v, 0, GetID(), "SET_SOCKET", "", "", GetOriginalVnum());
}

int32_t CItem::GetGold()
{
	if (IS_SET(GetFlag(), ITEM_FLAG_COUNT_PER_1GOLD))
	{
		if (GetProto()->dwGold == 0)
			return GetCount();
		else
			return GetCount() / GetProto()->dwGold;
	}
	else
		return GetProto()->dwGold;
}

int32_t CItem::GetShopBuyPrice()
{
	return GetProto()->dwShopBuyPrice;
}

bool CItem::IsOwnership(LPCHARACTER ch)
{
	if (!m_pkOwnershipEvent)
		return true;

	return m_dwOwnershipPID == ch->GetPlayerID() ? true : false;
}

EVENTFUNC(ownership_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "ownership_event> <Factor> Null pointer" );
		return 0;
	}

	LPITEM pkItem = info->item;

	pkItem->SetOwnershipEvent(nullptr);

	TPacketGCItemOwnership p;

	p.bHeader	= HEADER_GC_ITEM_OWNERSHIP;
	p.dwVID	= pkItem->GetVID();
	p.szName[0]	= '\0';

	pkItem->PacketAround(&p, sizeof(p));
	return 0;
}

void CItem::SetOwnershipEvent(LPEVENT pkEvent)
{
	m_pkOwnershipEvent = pkEvent;
}

void CItem::SetOwnership(LPCHARACTER ch, int32_t iSec)
{
	if (!ch)
	{
		if (m_pkOwnershipEvent)
		{
			event_cancel(&m_pkOwnershipEvent);
			m_dwOwnershipPID = 0;

			TPacketGCItemOwnership p;

			p.bHeader	= HEADER_GC_ITEM_OWNERSHIP;
			p.dwVID	= m_dwVID;
			p.szName[0]	= '\0';

			PacketAround(&p, sizeof(p));
		}
		return;
	}

	if (m_pkOwnershipEvent)
		return;

	// @correction005
	if (iSec <= 10)
		iSec = 30;

	m_dwOwnershipPID = ch->GetPlayerID();

	item_event_info* info = AllocEventInfo<item_event_info>();
	strlcpy(info->szOwnerName, ch->GetName(), sizeof(info->szOwnerName));
	info->item = this;

	SetOwnershipEvent(event_create(ownership_event, info, PASSES_PER_SEC(iSec)));

	TPacketGCItemOwnership p;

	p.bHeader = HEADER_GC_ITEM_OWNERSHIP;
	p.dwVID = m_dwVID;
	strlcpy(p.szName, ch->GetName(), sizeof(p.szName));

	PacketAround(&p, sizeof(p));
}

int32_t CItem::GetSocketCount()
{
	for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
	{
		if (GetSocket(i) == 0)
			return i;
	}
	return ITEM_SOCKET_MAX_NUM;
}

bool CItem::AddSocket()
{
	int32_t count = GetSocketCount();
	if (count == ITEM_SOCKET_MAX_NUM)
		return false;
	m_alSockets[count] = 1;
	return true;
}

void CItem::AlterToSocketItem(int32_t iSocketCount)
{
	if (iSocketCount >= ITEM_SOCKET_MAX_NUM)
	{
		sys_log(0, "Invalid Socket Count %d, set to maximum", ITEM_SOCKET_MAX_NUM);
		iSocketCount = ITEM_SOCKET_MAX_NUM;
	}

	for (int32_t i = 0; i < iSocketCount; ++i)
		SetSocket(i, 1);
}

void CItem::AlterToMagicItem()
{
	int32_t idx = GetAttributeSetIndex();

	if (idx < 0)
		return;


	int32_t iSecondPct;
	int32_t iThirdPct;

	switch (GetType())
	{
		case ITEM_WEAPON:
			iSecondPct = 20;
			iThirdPct = 5;
			break;

		case ITEM_ARMOR:
		case ITEM_COSTUME:
			if (GetSubType() == ARMOR_BODY)
			{
				iSecondPct = 10;
				iThirdPct = 2;
			}
			else
			{
				iSecondPct = 10;
				iThirdPct = 1;
			}
			break;

		default:
			return;
	}

	PutAttribute(aiItemMagicAttributePercentHigh);

	if (number(1, 100) <= iSecondPct)
		PutAttribute(aiItemMagicAttributePercentLow);

	if (number(1, 100) <= iThirdPct)
		PutAttribute(aiItemMagicAttributePercentLow);
}

uint32_t CItem::GetRefineFromVnum()
{
	return ITEM_MANAGER::instance().GetRefineFromVnum(GetVnum());
}

int32_t CItem::GetRefineLevel()
{
	const char* name = GetBaseName();
	char* p = const_cast<char*>(strrchr(name, '+'));

	if (!p)
		return 0;

	int32_t	rtn = 0;
	str_to_number(rtn, p+1);

	const char* locale_name = GetName();
	p = const_cast<char*>(strrchr(locale_name, '+'));

	if (p)
	{
		int32_t	locale_rtn = 0;
		str_to_number(locale_rtn, p+1);
		if (locale_rtn != rtn)
		{
			sys_err("refine_level_based_on_NAME(%d) is not equal to refine_level_based_on_LOCALE_NAME(%d).", rtn, locale_rtn);
		}
	}

	return rtn;
}

bool CItem::IsPolymorphItem()
{
	return GetType() == ITEM_POLYMORPH;
}

EVENTFUNC(unique_expire_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "unique_expire_event> <Factor> Null pointer" );
		return 0;
	}

	LPITEM pkItem = info->item;

	if (pkItem->GetValue(2) == 0)
	{
		if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) <= 1)
		{
			sys_log(0, "UNIQUE_ITEM: expire %s %u", pkItem->GetName(), pkItem->GetID());
			pkItem->SetUniqueExpireEvent(nullptr);
			ITEM_MANAGER::instance().RemoveItem(pkItem, "UNIQUE_EXPIRE");
			return 0;
		}
		else
		{
			pkItem->SetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME, pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - 1);
			return PASSES_PER_SEC(60);
		}
	}
	else
	{
		time_t cur = get_global_time();

		if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) <= cur)
		{
			pkItem->SetUniqueExpireEvent(nullptr);
			ITEM_MANAGER::instance().RemoveItem(pkItem, "UNIQUE_EXPIRE");
			return 0;
		}
		else
		{
			if (pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - cur < 600)
				return PASSES_PER_SEC(pkItem->GetSocket(ITEM_SOCKET_UNIQUE_REMAIN_TIME) - cur);
			else
				return PASSES_PER_SEC(600);
		}
	}
}

EVENTFUNC(timer_based_on_wear_expire_event)
{
	item_event_info* info = dynamic_cast<item_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "expire_event <Factor> Null pointer" );
		return 0;
	}

	LPITEM pkItem = info->item;
	int32_t remain_time = pkItem->GetSocket(ITEM_SOCKET_REMAIN_SEC) - processing_time/passes_per_sec;
	if (remain_time <= 0)
	{
		sys_log(0, "ITEM EXPIRED : expired %s %u", pkItem->GetName(), pkItem->GetID());
		pkItem->SetTimerBasedOnWearExpireEvent(nullptr);
		pkItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, 0);

		if (pkItem->IsDragonSoul())
		{
			DSManager::instance().DeactivateDragonSoul(pkItem);
		}
		else
		{
			ITEM_MANAGER::instance().RemoveItem(pkItem, "TIMER_BASED_ON_WEAR_EXPIRE");
		}
		return 0;
	}
	pkItem->SetSocket(ITEM_SOCKET_REMAIN_SEC, remain_time);
	return PASSES_PER_SEC (MIN (60, remain_time));
}

void CItem::SetUniqueExpireEvent(LPEVENT pkEvent)
{
	m_pkUniqueExpireEvent = pkEvent;
}

void CItem::SetTimerBasedOnWearExpireEvent(LPEVENT pkEvent)
{
	m_pkTimerBasedOnWearExpireEvent = pkEvent;
}

EVENTFUNC(real_time_expire_event)
{
	const item_vid_event_info* info = reinterpret_cast<const item_vid_event_info*>(event->info);

	if (nullptr == info)
		return 0;

	const LPITEM item = ITEM_MANAGER::instance().FindByVID( info->item_vid );

	if (nullptr == item)
		return 0;

	const time_t current = get_global_time();

	if (current > item->GetSocket(0)
#ifdef __GROWTH_PET__
&& !(item->GetType() == ITEM_PET && item->GetSubType() == PET_UPBRINGING)
#endif
)
	{
		if (item->IsNewMountItem())
			if (item->GetSocket(2) != 0)
				item->ClearMountAttributeAndAffect();

		ITEM_MANAGER::instance().RemoveItem(item, "REAL_TIME_EXPIRE");
		return 0;
	}

	return PASSES_PER_SEC(1);
}

void CItem::StartRealTimeExpireEvent()
{
	if (m_pkRealTimeExpireEvent)
		return;
	for (int32_t i=0 ; i < ITEM_LIMIT_MAX_NUM ; i++)
	{
		if (LIMIT_REAL_TIME == GetProto()->aLimits[i].bType || LIMIT_REAL_TIME_START_FIRST_USE == GetProto()->aLimits[i].bType)
		{
			item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
			info->item_vid = GetVID();

			m_pkRealTimeExpireEvent = event_create( real_time_expire_event, info, PASSES_PER_SEC(1));

			sys_log(0, "REAL_TIME_EXPIRE: StartRealTimeExpireEvent");

			return;
		}
	}
}

bool CItem::IsRealTimeItem()
{
	if(!GetProto())
		return false;
	for (int32_t i=0 ; i < ITEM_LIMIT_MAX_NUM ; i++)
	{
		if (LIMIT_REAL_TIME == GetProto()->aLimits[i].bType)
			return true;
	}
	return false;
}

void CItem::StartUniqueExpireEvent()
{
	if (GetType() != ITEM_UNIQUE)
		return;

	if (m_pkUniqueExpireEvent)
		return;

	if (IsRealTimeItem())
		return;

	if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		m_pOwner->ShowAlignment(false);

	int32_t iSec = GetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME);

	if (iSec == 0)
		iSec = 60;
	else
		iSec = MIN(iSec, 60);

	SetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME, 0);

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetUniqueExpireEvent(event_create(unique_expire_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::StartTimerBasedOnWearExpireEvent()
{
	if (m_pkTimerBasedOnWearExpireEvent)
		return;

	if (IsRealTimeItem())
		return;

	if (-1 == GetProto()->cLimitTimerBasedOnWearIndex)
		return;

	int32_t iSec = GetSocket(0);

	if (0 != iSec)
	{
		iSec %= 60;
		if (0 == iSec)
			iSec = 60;
	}

	item_event_info* info = AllocEventInfo<item_event_info>();
	info->item = this;

	SetTimerBasedOnWearExpireEvent(event_create(timer_based_on_wear_expire_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::StopUniqueExpireEvent()
{
	if (!m_pkUniqueExpireEvent)
		return;

	if (GetValue(2) != 0)
		return;

	if (GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		m_pOwner->ShowAlignment(true);

	SetSocket(ITEM_SOCKET_UNIQUE_SAVE_TIME, event_time(m_pkUniqueExpireEvent) / passes_per_sec);
	event_cancel(&m_pkUniqueExpireEvent);

	ITEM_MANAGER::instance().SaveSingleItem(this);
}

void CItem::StopTimerBasedOnWearExpireEvent()
{
	if (!m_pkTimerBasedOnWearExpireEvent)
		return;

	int32_t remain_time = GetSocket(ITEM_SOCKET_REMAIN_SEC) - event_processing_time(m_pkTimerBasedOnWearExpireEvent) / passes_per_sec;

	SetSocket(ITEM_SOCKET_REMAIN_SEC, remain_time);
	event_cancel(&m_pkTimerBasedOnWearExpireEvent);

	ITEM_MANAGER::instance().SaveSingleItem(this);
}

void CItem::ApplyAddon(int32_t iAddonType)
{
	CItemAddonManager::instance().ApplyAddonTo(iAddonType, this);
}

int32_t CItem::GetSpecialGroup() const
{
	return ITEM_MANAGER::instance().GetSpecialGroupFromItem(GetVnum());
}

bool CItem::IsAccessoryForSocket()
{
	return (m_pProto->bType == ITEM_ARMOR && (m_pProto->bSubType == ARMOR_WRIST || m_pProto->bSubType == ARMOR_NECK || m_pProto->bSubType == ARMOR_EAR)) ||
		(m_pProto->bType == ITEM_BELT);
}

void CItem::SetAccessorySocketGrade(int32_t iGrade)
{
	SetSocket(0, MINMAX(0, iGrade, GetAccessorySocketMaxGrade()));

	int32_t iDownTime = aiAccessorySocketDegradeTime[GetAccessorySocketGrade()];


	SetAccessorySocketDownGradeTime(iDownTime);
}

void CItem::SetAccessorySocketMaxGrade(int32_t iMaxGrade)
{
	SetSocket(1, MINMAX(0, iMaxGrade, ITEM_ACCESSORY_SOCKET_MAX_NUM));
}

void CItem::SetAccessorySocketDownGradeTime(uint32_t time)
{
#ifdef UNLIMITED_ACCESSORY_SOCKET_TIME // @correction205
	if (GetSocket(3) != 4248)
		SetSocket(2, time);
#else
	SetSocket(2, time);
#endif

	if (test_server && GetOwner())
		GetOwner()->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s에서 소켓 빠질때까지 남은 시간 %d"), GetName(), time);
}

EVENTFUNC(accessory_socket_expire_event)
{
	item_vid_event_info* info = dynamic_cast<item_vid_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "accessory_socket_expire_event> <Factor> Null pointer" );
		return 0;
	}

	LPITEM item = ITEM_MANAGER::instance().FindByVID(info->item_vid);

	if (item->GetAccessorySocketDownGradeTime() <= 1)
	{
degrade:
		item->SetAccessorySocketExpireEvent(nullptr);
		item->AccessorySocketDegrade();
		return 0;
	}
	else
	{
		int32_t iTime = item->GetAccessorySocketDownGradeTime() - 60;

		if (iTime <= 1)
			goto degrade;

		item->SetAccessorySocketDownGradeTime(iTime);

		if (iTime > 60)
			return PASSES_PER_SEC(60);
		else
			return PASSES_PER_SEC(iTime);
	}
}

void CItem::StartAccessorySocketExpireEvent()
{
#ifdef UNLIMITED_ACCESSORY_SOCKET_TIME // @correction205
	if (GetSocket(3) == 4248)
		return;
#endif

	if (!IsAccessoryForSocket())
		return;

	if (m_pkAccessorySocketExpireEvent)
		return;

	if (GetAccessorySocketMaxGrade() == 0)
		return;

	if (GetAccessorySocketGrade() == 0)
		return;

	int32_t iSec = GetAccessorySocketDownGradeTime();
	SetAccessorySocketExpireEvent(nullptr);

	if (iSec <= 1)
		iSec = 5;
	else
		iSec = MIN(iSec, 60);

	item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
	info->item_vid = GetVID();

	SetAccessorySocketExpireEvent(event_create(accessory_socket_expire_event, info, PASSES_PER_SEC(iSec)));
}

void CItem::StopAccessorySocketExpireEvent()
{
	if (!m_pkAccessorySocketExpireEvent)
		return;

	if (!IsAccessoryForSocket())
		return;

	int32_t new_time = GetAccessorySocketDownGradeTime() - (60 - event_time(m_pkAccessorySocketExpireEvent) / passes_per_sec);

	event_cancel(&m_pkAccessorySocketExpireEvent);

	if (new_time <= 1)
	{
		AccessorySocketDegrade();
	}
	else
	{
		SetAccessorySocketDownGradeTime(new_time);
	}
}

bool CItem::IsRideItem()
{
	if (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_RIDE == GetSubType())
		return true;
	if (ITEM_UNIQUE == GetType() && UNIQUE_SPECIAL_MOUNT_RIDE == GetSubType())
		return true;
#ifdef __COSTUME_MOUNT__
	if (ITEM_COSTUME == GetType() && COSTUME_MOUNT == GetSubType())
		return true;
#endif
	return false;
}

bool CItem::IsRamadanRing()
{
	if (GetVnum() == UNIQUE_ITEM_RAMADAN_RING)
		return true;
	return false;
}

void CItem::ClearMountAttributeAndAffect()
{
	LPCHARACTER ch = GetOwner();

	ch->RemoveAffect(AFFECT_MOUNT);
	ch->RemoveAffect(AFFECT_MOUNT_BONUS);

	ch->MountVnum(0);

	ch->PointChange(POINT_ST, 0);
	ch->PointChange(POINT_DX, 0);
	ch->PointChange(POINT_HT, 0);
	ch->PointChange(POINT_IQ, 0);
}

bool CItem::IsNewMountItem()
{
	switch(GetVnum())
	{
		case 76000: case 76001: case 76002: case 76003:
		case 76004: case 76005: case 76006: case 76007:
		case 76008: case 76009: case 76010: case 76011:
		case 76012: case 76013: case 76014:
			return true;
	}
	return false;
}

void CItem::SetAccessorySocketExpireEvent(LPEVENT pkEvent)
{
	m_pkAccessorySocketExpireEvent = pkEvent;
}

void CItem::AccessorySocketDegrade()
{
	if (GetAccessorySocketGrade() > 0)
	{
		LPCHARACTER ch = GetOwner();

		if (ch)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s에 박혀있던 보석이 사라집니다."), GetName());
		}

		ModifyPoints(false);
		SetAccessorySocketGrade(GetAccessorySocketGrade()-1);
		ModifyPoints(true);

		int32_t iDownTime = aiAccessorySocketDegradeTime[GetAccessorySocketGrade()];

		if (test_server)
			iDownTime /= 60;

		SetAccessorySocketDownGradeTime(iDownTime);

		if (iDownTime)
			StartAccessorySocketExpireEvent();
	}
}

static const bool CanPutIntoRing(LPITEM ring, LPITEM item)
{
	return false;
}

bool CItem::CanPutInto(LPITEM item)
{
	if (item->GetType() == ITEM_BELT)
		return this->GetSubType() == USE_PUT_INTO_BELT_SOCKET;

	else if(item->GetType() == ITEM_RING)
		return CanPutIntoRing(item, this);

	else if (item->GetType() != ITEM_ARMOR)
		return false;

	uint32_t vnum = item->GetVnum();

	struct JewelAccessoryInfo
	{
		uint32_t jewel;
		uint32_t wrist;
		uint32_t neck;
		uint32_t ear;
	};
	const static JewelAccessoryInfo infos[] = {
        { 50634, 14220, 16220, 17220 },
        { 50635, 14500, 16500, 17500 },
        { 50636, 14520, 16520, 17520 },
        { 50637, 14540, 16540, 17540 },
        { 50638, 14560, 16560, 17560 },
        { 50639, 14570, 16570, 17570 },
        { 50640, 14230, 16230, 17230 },
        { 50635, 24500, 19500, 18500 },
        { 50636, 24520, 19520, 18520 },
        { 50637, 24540, 19540, 18540 },
        { 50638, 24560, 19560, 18560 },
        { 50639, 24570, 19570, 18570 },
        { 50641, 14220, 16220, 17220 },//unlimited 
        { 50642, 14500, 16500, 17500 },//unlimited 
        { 50643, 14520, 16520, 17520 },//unlimited 
        { 50644, 14540, 16540, 17540 },//unlimited 
        { 50645, 14560, 16560, 17560 },//unlimited 
        { 50646, 14570, 16570, 17570 },//unlimited 
        { 50647, 14230, 16230, 17230 },//unlimited
        { 50650, 14240, 16240, 17240 },//limited
        { 50657, 14240, 16240, 17240 },//unlimited
        { 50642, 24500, 19500, 18500 },//unlimited 
        { 50643, 24520, 19520, 18520 },//unlimited 
        { 50644, 24540, 19540, 18540 },//unlimited 
        { 50645, 24560, 19560, 18560 },//unlimited 
        { 50646, 24570, 19570, 18570 },//unlimited
    };

	uint32_t item_type = (item->GetVnum() / 10) * 10;
	for (size_t i = 0; i < sizeof(infos) / sizeof(infos[0]); i++)
	{
		const JewelAccessoryInfo& info = infos[i];
		switch(item->GetSubType())
		{
		case ARMOR_WRIST:
			if (info.jewel == GetVnum())
			{
				if (info.wrist == item_type)
				{
					return true;
				}
				else
				{
					// return false;
				}
			}
			break;
		case ARMOR_NECK:
			if (info.jewel == GetVnum())
			{
				if (info.neck == item_type)
				{
					return true;
				}
				else
				{
					// return false;
				}
			}
			break;
		case ARMOR_EAR:
			if (info.jewel == GetVnum())
			{
				if (info.ear == item_type)
				{
					return true;
				}
				else
				{
					// return false;
				}
			}
			break;
		}
	}
	if (item->GetSubType() == ARMOR_WRIST)
		vnum -= 14000;
	else if (item->GetSubType() == ARMOR_NECK)
		vnum -= 16000;
	else if (item->GetSubType() == ARMOR_EAR)
		vnum -= 17000;
	else
		return false;

	uint32_t type = vnum / 20;

	if ((int32_t)type < 0 || type > 11)
	{
		type = (vnum - 170) / 20;

		if (50623 + type != GetVnum())
			return false;
		else
			return true;
	}
	else if (item->GetVnum() >= 16210 && item->GetVnum() <= 16219)
	{
		if (50625 != GetVnum())
			return false;
		else
			return true;
	}
	else if (item->GetVnum() >= 16230 && item->GetVnum() <= 16239)
	{
		if (50626 != GetVnum())
			return false;
		else
			return true;
	}

	return 50623 + type == GetVnum();
}

bool CItem::IsPCBangItem()
{
	for (int32_t i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		if (m_pProto->aLimits[i].bType == LIMIT_PCBANG)
			return true;
	}
	return false;
}

bool CItem::CheckItemUseLevel(int32_t nLevel)
{
	for (int32_t i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		if (this->m_pProto->aLimits[i].bType == LIMIT_LEVEL)
		{
			if (this->m_pProto->aLimits[i].lValue > nLevel) return false;
			else return true;
		}
	}
	return true;
}

int32_t CItem::FindApplyValue(uint8_t bApplyType)
{
	if (m_pProto == nullptr)
		return 0;

	for (int32_t i = 0; i < ITEM_APPLY_MAX_NUM; ++i)
	{
		if (m_pProto->aApplies[i].bType == bApplyType)
			return m_pProto->aApplies[i].lValue;
	}

	return 0;
}

void CItem::CopySocketTo(LPITEM pItem)
{
	for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		pItem->m_alSockets[i] = m_alSockets[i];
	}
}

int32_t CItem::GetAccessorySocketGrade()
{
   	return MINMAX(0, GetSocket(0), GetAccessorySocketMaxGrade());
}

int32_t CItem::GetAccessorySocketMaxGrade()
{
   	return MINMAX(0, GetSocket(1), ITEM_ACCESSORY_SOCKET_MAX_NUM);
}

int32_t CItem::GetAccessorySocketDownGradeTime()
{
	return MINMAX(0, GetSocket(2), aiAccessorySocketDegradeTime[GetAccessorySocketGrade()]);
}

void CItem::AttrLog()
{
	const char * pszIP = nullptr;

	if (GetOwner() && GetOwner()->GetDesc())
		pszIP = GetOwner()->GetDesc()->GetHostName();

	for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
		if (m_alSockets[i])
			if (g_iDbLogLevel >= LOG_LEVEL_MAX) // @correction222
				LogManager::instance().ItemLog(i, m_alSockets[i], 0, GetID(), "INFO_SOCKET", "", pszIP ? pszIP : "", GetOriginalVnum());

	for (int32_t i = 0; i < ITEM_ATTRIBUTE_MAX_NUM; ++i)
	{
		int32_t	type	= m_aAttr[i].bType;
		int32_t value	= m_aAttr[i].sValue;

		if (type)
			if (g_iDbLogLevel >= LOG_LEVEL_MAX) // @correction222
				LogManager::instance().ItemLog(i, type, value, GetID(), "INFO_ATTR", "", pszIP ? pszIP : "", GetOriginalVnum());
	}
}

int32_t CItem::GetLevelLimit()
{
	for (int32_t i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		if (this->m_pProto->aLimits[i].bType == LIMIT_LEVEL)
		{
			return this->m_pProto->aLimits[i].lValue;
		}
	}
	return 0;
}

bool CItem::OnAfterCreatedItem()
{
	if (-1 != this->GetProto()->cLimitRealTimeFirstUseIndex)
	{
		if (0 != GetSocket(1))
		{
			StartRealTimeExpireEvent();
		}
	}

#ifdef __SOULBIND__ // @correction202
	if (GetSealBindTime() > 0)
		StartUnSealBindTimerExpireEvent();
#endif

	return true;
}


#ifdef __AUCTION__


bool CItem::MoveToAuction()
{
	LPCHARACTER owner = GetOwner();
	if (owner == nullptr)
	{
		sys_err ("Item those owner is not exist cannot regist in auction");
		return false;
	}

	if (GetWindow() == AUCTION)
	{
		sys_err ("Item is already in auction.");
	}

	SetWindow(AUCTION);
	owner->SetItem(m_wCell, nullptr);
	Save();
	ITEM_MANAGER::instance().FlushDelayedSave(this);

	return true;
}

void CItem::CopyToRawData (TPlayerItem* new_item)
{
	if (new_item != nullptr)
		return;

	new_item->id = m_dwID;
	new_item->window = m_bWindow;
	new_item->pos = m_wCell;
	new_item->count = m_dwCount;

	new_item->vnum = GetVnum();
	memcpy (new_item->alSockets, m_alSockets, sizeof (m_alSockets));
	memcpy (new_item->aAttr, m_aAttr, sizeof (m_aAttr));

	new_item->owner = m_pOwner->GetPlayerID();
}
#endif

bool CItem::IsDragonSoul()
{
	return GetType() == ITEM_DS;
}

int32_t CItem::GiveMoreTime_Per(float fPercent)
{
	if (IsDragonSoul())
	{
		uint32_t duration = DSManager::instance().GetDuration(this);
		uint32_t remain_sec = GetSocket(ITEM_SOCKET_REMAIN_SEC);
		uint32_t given_time = fPercent * duration / 100u;
		if (remain_sec == duration)
			return false;
		if ((given_time + remain_sec) >= duration)
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, duration);
			return duration - remain_sec;
		}
		else
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, given_time + remain_sec);
			return given_time;
		}
	}
	else
		return 0;
}

int32_t CItem::GiveMoreTime_Fix(uint32_t dwTime)
{
	if (IsDragonSoul())
	{
		uint32_t duration = DSManager::instance().GetDuration(this);
		uint32_t remain_sec = GetSocket(ITEM_SOCKET_REMAIN_SEC);
		if (remain_sec == duration)
			return false;
		if ((dwTime + remain_sec) >= duration)
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, duration);
			return duration - remain_sec;
		}
		else
		{
			SetSocket(ITEM_SOCKET_REMAIN_SEC, dwTime + remain_sec);
			return dwTime;
		}
	}
	else
		return 0;
}


int32_t CItem::GetDuration()
{
	if(!GetProto())
		return -1;

	for (int32_t i=0 ; i < ITEM_LIMIT_MAX_NUM ; i++)
	{
		if (LIMIT_REAL_TIME == GetProto()->aLimits[i].bType)
			return GetProto()->aLimits[i].lValue;
	}

	if (GetProto()->cLimitTimerBasedOnWearIndex >= 0)
	{
		uint8_t cLTBOWI = GetProto()->cLimitTimerBasedOnWearIndex;
		return GetProto()->aLimits[cLTBOWI].lValue;
	}

	return -1;
}

bool CItem::IsSameSpecialGroup(const LPITEM item) const
{
	if (this->GetVnum() == item->GetVnum())
		return true;

	if (GetSpecialGroup() && (item->GetSpecialGroup() == GetSpecialGroup()))
		return true;

	return false;
}

#ifdef __ACCE_SYSTEM__ // @correction191
uint8_t CItem::GetAcceValue()
{
	int32_t lVal = FindApplyValue(APPLY_ACCEDRAIN_RATE);

	if (lVal == 1)
		return 1;
	else if (lVal == 5)
		return 2;
	else if (lVal == 10)
		return 3;
	else if (lVal == 20)
		return 4;

	return 1;
}
#endif

#ifdef __SOULBIND__ // @correction202
void CItem::SetSealBind(int32_t sp)
{
	m_lSealbind = sp;
	UpdatePacket();
	Save();
}

EVENTFUNC(unbind_timer_expire_event)
{
	const item_vid_event_info* info = reinterpret_cast<const item_vid_event_info*>(event->info);
	if (!info)
		return 0;

	const LPITEM item = ITEM_MANAGER::instance().FindByVID( info->item_vid );
	if (!item)
		return 0;
	
	const time_t current = get_global_time();
	if (current > item->GetSealBindTime())
	{
		item->SetSealBind(0);
		return 0;
	}
	
	return PASSES_PER_SEC (1);
}

void CItem::StartUnSealBindTimerExpireEvent()
{
	if (m_pkUnSealTimerExpireEvent)
		return;

	item_vid_event_info* info = AllocEventInfo<item_vid_event_info>();
	info->item_vid = GetVID();

	m_pkUnSealTimerExpireEvent = event_create(unbind_timer_expire_event, info, PASSES_PER_SEC(1));
	sys_log(0, "UNSEAL_TIME_EXPIRE: StartUnSealBindTimerExpireEvent");
}

#endif

#ifdef __OFFLINE_PRIVATE_SHOP__
void CItem::AddOfflinePrivateShop(int32_t iPos, uint32_t dwShopId)
{
	if (!m_pOwner)
	{
		sys_err("Item::AddOfflinePrivateShop owner null");
		return;
	}

	if (GetWindow() != SAFEBOX && GetWindow() != MALL)
	{
		if (IsDragonSoul())
		{
			if (m_wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
				sys_err("CItem::AddOfflinePrivateShop: pos >= DRAGON_SOUL_INVENTORY_MAX_NUM");
			else
				m_pOwner->SetItem(TItemPos(m_bWindow, m_wCell), nullptr);
		}
		else
		{
			TItemPos cell(INVENTORY, m_wCell);

			if (false == cell.IsDefaultInventoryPosition() && false == cell.IsBeltInventoryPosition()) // 아니면 소지품에?
				sys_err("CItem::AddOfflinePrivateShop: Invalid Item Position");
			else
				m_pOwner->SetItem(cell, nullptr);
		}
	}

	m_dwShopId = dwShopId;
	m_wCell = iPos;
	SetWindow(RESERVED_WINDOW);
	SaveAndFlush();
	m_pOwner = nullptr;
}

void CItem::SaveAndFlush()
{
	ITEM_MANAGER::instance().DelayedSave(this);
	ITEM_MANAGER::instance().FlushDelayedSave(this);
	uint32_t dwID = GetID();
	db_clientdesc->DBPacketHeader(HEADER_GD_ITEM_FLUSH, 0, sizeof(uint32_t));
	db_clientdesc->Packet(&dwID, sizeof(uint32_t));
}

void CItem::SetLock(bool bIsLocked, bool bIsDirect)
{
	m_bIsLocked = bIsLocked;
	if (bIsDirect)
	{
		char queryStr[256];
		snprintf(queryStr, sizeof(queryStr), "UPDATE player.item SET locked = %d WHERE id = %u", bIsLocked, GetID());
		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery(queryStr));
	}
}
#endif


#ifdef __EXTEND_INVEN__
bool CItem::IsUpgradeItem()
{
	return (GetType() == ITEM_MATERIAL && GetSubType() == MATERIAL_LEATHER);
}
bool CItem::IsBook()
{
	return GetType() == ITEM_SKILLBOOK;
}
bool CItem::IsStone()
{
	return (GetType() == ITEM_METIN && GetSubType() == METIN_NORMAL);
}
bool CItem::IsAttr()
{
	return (/*ITEM_MEDIUM == GetType() || */(GetType() == ITEM_USE && (GetSubType() == USE_ADD_ATTRIBUTE || GetSubType() == USE_ADD_ATTRIBUTE2 || GetSubType() == USE_CHANGE_ATTRIBUTE || GetSubType() == USE_CHANGE_ATTRIBUTE2
#ifdef __MOVE_COSTUME_ATTR__
|| GetSubType() == USE_RESET_COSTUME_ATTR || GetSubType() == USE_ADD_COSTUME_ATTRIBUTE/* || GetSubType() == USE_CHANGE_ATTRIBUTE_PLUS*/
#endif
)));

}
bool CItem::IsGiftBox()
{
	return (GetType() == ITEM_GIFTBOX
#ifdef __SPECIAL_GACHA__
|| GetType() == ITEM_GACHA
#endif
);
}

bool CItem::IsStorageItem()
{
	return IsUpgradeItem() || IsBook() || IsStone() || IsAttr() || IsGiftBox();
}
#endif

uint8_t CItem::GetInventoryType()
{
#ifdef __EXTEND_INVEN__
	if (IsUpgradeItem())
		return INVENTORY_UPGRADE;
	else if (IsBook())
		return INVENTORY_BOOK;
	else if (IsStone())
		return INVENTORY_STONE;
	else if (IsAttr())
		return INVENTORY_ATTR;
	else if (IsGiftBox())
		return INVENTORY_GIFTBOX;
#endif

	return INVENTORY_NORMAL;
}

