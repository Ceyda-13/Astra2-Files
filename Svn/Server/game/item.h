#pragma once

#include "entity.h"

class CShop; // @correction080

class CItem : public CEntity
{
	protected:
		virtual void	EncodeInsertPacket(LPENTITY entity);
		virtual void	EncodeRemovePacket(LPENTITY entity);

	public:
		CItem(uint32_t dwVnum);
		virtual ~CItem();

		int32_t			GetLevelLimit();

		bool		CheckItemUseLevel(int32_t nLevel);

		bool		IsPCBangItem();

		int32_t		FindApplyValue(uint8_t bApplyType);

		bool		IsStackable()		{ return (GetFlag() & ITEM_FLAG_STACKABLE)?true:false; }

		void		Initialize();
		void		Destroy();

		void		Save();

		void		SetWindow(uint8_t b)	{ m_bWindow = b; }
		uint8_t		GetWindow()		{ return m_bWindow; }

		void		SetID(uint32_t id)		{ m_dwID = id;	}
		uint32_t		GetID()			{ return m_dwID; }

		void			SetProto(const TItemTable * table);
		TItemTable const *	GetProto()	{ return m_pProto; }

		int32_t		GetGold();
		int32_t		GetShopBuyPrice();
		const char *	GetName()		{ return m_pProto ? m_pProto->szLocaleName : nullptr; }
		const char *	GetBaseName()		{ return m_pProto ? m_pProto->szName : nullptr; }
		uint8_t		GetSize()		{ return m_pProto ? m_pProto->bSize : 0;	}

		void		SetFlag(int32_t flag)	{ m_lFlag = flag;	}
		int32_t		GetFlag()		{ return m_lFlag;	}

		void		AddFlag(int32_t bit);
		void		RemoveFlag(int32_t bit);

		uint32_t		GetWearFlag()		{ return m_pProto ? m_pProto->dwWearFlags : 0; }
		uint32_t		GetAntiFlag()		{ return m_pProto ? m_pProto->dwAntiFlags : 0; }
		uint32_t		GetImmuneFlag()		{ return m_pProto ? m_pProto->dwImmuneFlag : 0; }

		void		SetVID(uint32_t vid)	{ m_dwVID = vid;	}
		uint32_t		GetVID()		{ return m_dwVID;	}

		bool		SetCount(uint32_t count);
		uint32_t		GetCount();

		uint32_t		GetVnum() const		{ return m_dwMaskVnum ? m_dwMaskVnum : m_dwVnum;	}
		uint32_t		GetOriginalVnum() const		{ return m_dwVnum;	}
		uint8_t		GetType() const		{ return m_pProto ? m_pProto->bType : 0;	}
		uint8_t		GetSubType() const	{ return m_pProto ? m_pProto->bSubType : 0;	}
		uint8_t		GetLimitType(uint32_t idx) const { return m_pProto ? m_pProto->aLimits[idx].bType : 0;	}
		int32_t		GetLimitValue(uint32_t idx) const { return m_pProto ? m_pProto->aLimits[idx].lValue : 0;	}

		int32_t		GetValue(uint32_t idx);

		void		SetCell(LPCHARACTER ch, uint16_t pos)	{ m_pOwner = ch, m_wCell = pos;	}
		uint16_t		GetCell()				{ return m_wCell;	}

		LPITEM		RemoveFromCharacter();
		bool		AddToCharacter(LPCHARACTER ch, TItemPos Cell, bool isHighLight = false);
		LPCHARACTER	GetOwner()		{ return m_pOwner; }

		LPITEM		RemoveFromGround();
		bool		AddToGround(int32_t lMapIndex, const PIXEL_POSITION & pos, bool skipOwnerCheck = false);

		int32_t			FindEquipCell(LPCHARACTER ch, int32_t bCandidateCell = -1);
		bool		IsEquipped() const		{ return m_bEquipped;	}
		bool		EquipTo(LPCHARACTER ch, uint8_t bWearCell);
		bool		IsEquipable() const;

		bool		CanUsedBy(LPCHARACTER ch);

		bool		DistanceValid(LPCHARACTER ch);

		void		UpdatePacket();
		void		UsePacketEncode(LPCHARACTER ch, LPCHARACTER victim, struct packet_item_use * packet);

		void		SetExchanging(bool isOn = true);
		bool		IsExchanging()		{ return m_bExchanging;	}

		bool		IsTwohanded();

		bool		IsPolymorphItem();

		void		ModifyPoints(bool bAdd);

		bool		CreateSocket(uint8_t bSlot, uint8_t bGold);
		const int32_t *	GetSockets()		{ return &m_alSockets[0];	}
		int32_t		GetSocket(int32_t i)	{ return m_alSockets[i];	}

		void		SetSockets(const int32_t * al);
		void		SetSocket(int32_t i, int32_t v, bool bLog = true);

		int32_t		GetSocketCount();
		bool		AddSocket();

		const TPlayerItemAttribute* GetAttributes()		{ return m_aAttr;	}
		const TPlayerItemAttribute& GetAttribute(int32_t i)	{ return m_aAttr[i];	}

		uint8_t		GetAttributeType(int32_t i)	{ return m_aAttr[i].bType;	}
		int16_t		GetAttributeValue(int32_t i){ return m_aAttr[i].sValue;	}

		void		SetAttributes(const TPlayerItemAttribute* c_pAttribute);

		int32_t		FindAttribute(uint8_t bType);
		bool		RemoveAttributeAt(int32_t index);
		bool		RemoveAttributeType(uint8_t bType);

		bool		HasAttr(uint8_t bApply);
		bool		HasRareAttr(uint8_t bApply);

		void		SetDestroyEvent(LPEVENT pkEvent);
		void		StartDestroyEvent(int32_t iSec=300);

		uint32_t		GetRefinedVnum()	{ return m_pProto ? m_pProto->dwRefinedVnum : 0; }
		uint32_t		GetRefineFromVnum();
		int32_t		GetRefineLevel();

		void		SetSkipSave(bool b)	{ m_bSkipSave = b; }
		bool		GetSkipSave()		{ return m_bSkipSave; }

		bool		IsOwnership(LPCHARACTER ch);
		void		SetOwnership(LPCHARACTER ch, int32_t iSec = 10);
		void		SetOwnershipEvent(LPEVENT pkEvent);

		uint32_t		GetLastOwnerPID()	{ return m_dwLastOwnerPID; }

		int32_t		GetAttributeSetIndex();
		void		AlterToMagicItem();
		void		AlterToSocketItem(int32_t iSocketCount);

		uint16_t		GetRefineSet()		{ return m_pProto ? m_pProto->wRefineSet : 0;	}

		void		StartUniqueExpireEvent();
		void		SetUniqueExpireEvent(LPEVENT pkEvent);

		void		StartTimerBasedOnWearExpireEvent();
		void		SetTimerBasedOnWearExpireEvent(LPEVENT pkEvent);

		void		StartRealTimeExpireEvent();
		bool		IsRealTimeItem();

		void		StopUniqueExpireEvent();
		void		StopTimerBasedOnWearExpireEvent();
		void		StopAccessorySocketExpireEvent();

		int32_t			GetDuration();

		int32_t		GetAttributeCount();
		void		ClearAttribute();
		void		ChangeAttribute(const int32_t* aiChangeProb=nullptr);
		void		AddAttribute();
		void		AddAttribute(uint8_t bType, int16_t sValue);

		void		ApplyAddon(int32_t iAddonType);

		int32_t		GetSpecialGroup() const;
		bool	IsSameSpecialGroup(const LPITEM item) const;

		bool		IsAccessoryForSocket();

		int32_t		GetAccessorySocketGrade();
		int32_t		GetAccessorySocketMaxGrade();
		int32_t		GetAccessorySocketDownGradeTime();

		void		SetAccessorySocketGrade(int32_t iGrade);
		void		SetAccessorySocketMaxGrade(int32_t iMaxGrade);
		void		SetAccessorySocketDownGradeTime(uint32_t time);

		void		AccessorySocketDegrade();

		void		StartAccessorySocketExpireEvent();
		void		SetAccessorySocketExpireEvent(LPEVENT pkEvent);

		bool		CanPutInto(LPITEM item);

		void		CopyAttributeTo(LPITEM pItem);
		void		CopySocketTo(LPITEM pItem);

		int32_t			GetRareAttrCount();
		bool		AddRareAttribute();
		bool		ChangeRareAttribute();

		void		AttrLog();

		void		Lock(bool f) { m_isLocked = f; }
		bool		isLocked() const { return m_isLocked; }

	private :
		void		SetAttribute(int32_t i, uint8_t bType, int16_t sValue);
	public:
		void		SetForceAttribute(int32_t i, uint8_t bType, int16_t sValue);

	protected:
		bool		EquipEx(bool is_equip);
		bool		Unequip();

		void		AddAttr(uint8_t bApply, uint8_t bLevel);
		void		PutAttribute(const int32_t * aiAttrPercentTable);
		void		PutAttributeWithLevel(uint8_t bLevel);

	public:
		void		AddRareAttribute2(const int32_t * aiAttrPercentTable = nullptr);
	protected:
		void		AddRareAttr(uint8_t bApply, uint8_t bLevel);
		void		PutRareAttribute(const int32_t * aiAttrPercentTable);
		void		PutRareAttributeWithLevel(uint8_t bLevel);

	protected:
		friend class CInputDB;
		bool		OnAfterCreatedItem();

	public:
		bool		IsRideItem();
		bool		IsRamadanRing();

		void		ClearMountAttributeAndAffect();
		bool		IsNewMountItem();

#ifdef __AUCTION__
		bool		MoveToAuction ();
		void		CopyToRawData (TPlayerItem* item);
#endif

		void		SetMaskVnum(uint32_t vnum)	{	m_dwMaskVnum = vnum; }
		uint32_t		GetMaskVnum()			{	return m_dwMaskVnum; }
		bool		IsMaskedItem()	{	return m_dwMaskVnum != 0;	}

		bool		IsDragonSoul();
		int32_t		GiveMoreTime_Per(float fPercent);
		int32_t		GiveMoreTime_Fix(uint32_t dwTime);

	private:
		TItemTable const * m_pProto;

		uint32_t		m_dwVnum;
		LPCHARACTER	m_pOwner;

		uint8_t		m_bWindow;
		uint32_t		m_dwID;
		bool		m_bEquipped;
		uint32_t		m_dwVID;
		uint16_t		m_wCell;
		uint32_t		m_dwCount;

		int32_t		m_lFlag;
		uint32_t		m_dwLastOwnerPID;

		bool		m_bExchanging;

		int32_t		m_alSockets[ITEM_SOCKET_MAX_NUM];
		TPlayerItemAttribute	m_aAttr[ITEM_ATTRIBUTE_MAX_NUM];

		LPEVENT		m_pkDestroyEvent;
		LPEVENT		m_pkExpireEvent;
		LPEVENT		m_pkUniqueExpireEvent;
		LPEVENT		m_pkTimerBasedOnWearExpireEvent;
		LPEVENT		m_pkRealTimeExpireEvent;
		LPEVENT		m_pkAccessorySocketExpireEvent;
		LPEVENT		m_pkOwnershipEvent;

		uint32_t		m_dwOwnershipPID;

		bool		m_bSkipSave;

		bool		m_isLocked;

		uint32_t		m_dwMaskVnum;
		uint32_t		m_dwSIGVnum;

	public:
		void SetSIGVnum(uint32_t dwSIG) { m_dwSIGVnum = dwSIG; }
		uint32_t	GetSIGVnum() const { return m_dwSIGVnum;  }

	// @correction080 BEGIN
	protected:
		LPSHOP		m_pkShop;

	public:
		LPSHOP GetShop() { return m_pkShop; }
		void SetShop(LPSHOP pkShop) { m_pkShop = pkShop; }
	// @correction080 END

#ifdef __ACCE_SYSTEM__ // @correction191
	public:
		uint8_t		GetAcceValue();
#endif
#ifdef __SOULBIND__ // @correction202
	public:
		int32_t		GetSealBindTime() const { return m_lSealbind;}
		void		SetSealBind(int32_t sb = -1);
		bool		IsSealed() const { return m_lSealbind != 0;}
		void		StartUnSealBindTimerExpireEvent();
	private:
		int32_t		m_lSealbind;
		LPEVENT		m_pkUnSealTimerExpireEvent;
#endif
#ifdef __CHANGE_LOOK__ // @correction192
	public:
		uint32_t		GetChangeLook() const	{return m_dwChangeLook;}
		void		SetChangeLook(uint32_t dwVnum, bool bLog = false);
	protected:
		uint32_t		m_dwChangeLook;
#endif
#ifdef WJ_ELDER_ATTRIBUTE_SYSTEM // @correction193
	public:
		bool		HasApply(uint8_t bApply);
		bool		HasOnlyAttr(uint8_t bApply);
		void		AddNewAttr(uint8_t index, uint8_t bApply, uint8_t bLevel);
#endif

#ifdef __OFFLINE_PRIVATE_SHOP__
	public:
		void 		AddOfflinePrivateShop(int32_t iPos, uint32_t dwShopId);
		void 		SetOfflinePrivateShopVid(uint32_t dwVid) { m_dwShopId = dwVid; }
		uint32_t 		GetOfflinePrivateShopVid() { return m_dwShopId; }
		void		SetOfflinePrivateShopPrice(uint64_t dwPrice) { m_dwPrices = dwPrice; }
		const uint64_t		GetOfflinePrivateShopPrices() { return m_dwPrices; }
		void 		SetOwnerPID(uint32_t dwVid) { m_dwOwnerPID = dwVid; }
		uint32_t	GetOwnerPID() { return m_dwOwnerPID; }
		void		SetOwnerName(const char* szName) { strlcpy(m_szOwnerName, szName, sizeof(m_szOwnerName)); }
		const char*	GetOwnerName() { return m_szOwnerName; }
		void 		SetLock(bool bIsLocked, bool bIsDirect = false);
		bool		GetLock() { return m_bIsLocked; }
		void 		SaveAndFlush();

	protected:
		uint32_t		m_dwShopId;
		uint64_t		m_dwPrices;
		uint32_t		m_dwOwnerPID;
		char			m_szOwnerName[CHARACTER_NAME_MAX_LEN + 1];
		bool			m_bIsLocked;
#endif
	public:
		uint8_t		GetInventoryType();
#ifdef __EXTEND_INVEN__
		bool 		IsUpgradeItem();
		bool 		IsBook();
		bool 		IsStone();
		bool 		IsAttr();
		bool 		IsGiftBox();
		bool		IsStorageItem();
#endif
};

EVENTINFO(item_event_info)
{
	LPITEM item;
	char szOwnerName[CHARACTER_NAME_MAX_LEN];

	item_event_info()
	: item( 0 )
	{
		::memset( szOwnerName, 0, CHARACTER_NAME_MAX_LEN );
	}
};

EVENTINFO(item_vid_event_info)
{
	uint32_t item_vid;

	item_vid_event_info()
	: item_vid( 0 )
	{
	}
};

