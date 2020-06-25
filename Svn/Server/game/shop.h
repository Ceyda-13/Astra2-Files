#pragma once

enum
{
	SHOP_MAX_DISTANCE = 1000
};

class CGrid;

class CShop
{
	public:
		typedef struct shop_item
		{
			TShopItemData	item;
			int32_t			itemid;
			LPITEM			pkItem;
			shop_item()
			{
				memset(&item, 0, sizeof(item));
				itemid = 0;
				pkItem = nullptr;
			}
		} SHOP_ITEM;

		CShop();
		virtual ~CShop(); // @correction059 (+virtual)

		bool	Create(uint32_t dwVnum, uint32_t dwNPCVnum, TShopItemTable * pItemTable);
		void	SetShopItems(TShopItemTable * pItemTable, uint8_t bItemCount
#ifdef __OFFLINE_PRIVATE_SHOP__
, uint32_t dwShopVid = 0
#endif
);

		virtual void	SetPCShop(LPCHARACTER ch);
		virtual bool	IsPCShop()	{ return m_pkPC ? true : false; }

		virtual bool	AddGuest(LPCHARACTER ch,uint32_t owner_vid, bool bOtherEmpire);
		void	RemoveGuest(LPCHARACTER ch);

		virtual int32_t	Buy(LPCHARACTER ch, uint8_t pos);

		void	BroadcastUpdateItem(uint8_t pos);

		int32_t		GetNumberByVnum(uint32_t dwVnum);

		virtual bool	IsSellingItem(uint32_t itemID);

		uint32_t	GetVnum() { return m_dwVnum; }
		uint32_t	GetNPCVnum() { return m_dwNPCVnum; }

	protected:
		void	Broadcast(const void * data, int32_t bytes);

	protected:
		uint32_t				m_dwVnum;
		uint32_t				m_dwNPCVnum;

#ifdef ENABLE_SHOP_TAB // @correction196
		uint16_t				m_pkGrid[SHOP_HOST_ITEM_MAX_NUM];
#else
		CGrid *				m_pGrid;
#endif

		typedef std::unordered_map<LPCHARACTER, bool> GuestMapType;
		GuestMapType m_map_guest;
		std::vector<SHOP_ITEM>		m_itemVector;

		LPCHARACTER			m_pkPC;

	// @correction080 BEGIN
	public:
		void RemoveItem(LPITEM pkItem);
	// @correction080 END

#ifdef ENABLE_SHOP_TAB // @correction196
	public:
		bool	IsEmpty(uint16_t wCell, uint8_t bSize, int32_t iExceptionCell = -1);
		bool	IsValidPosition(uint32_t dwPos);
		int32_t		GetEmptyShop(uint32_t dwSize);
		int32_t		GetEmptyShopForUnlocks(uint32_t dwSize);
		uint16_t*	GetGrid() { return m_pkGrid; }
#endif

#ifdef __OFFLINE_PRIVATE_SHOP__
	public:
		void SetOfflinePrivateShop(bool bIsOfflinePrivateShop) { m_bIsOfflinePrivateShop = bIsOfflinePrivateShop; }
		void SetOfflinePrivateShopItem(LPITEM pkItem);
		void SetOfflinePrivateShopInfo(TOfflinePrivateShopInfo * pkTOfflinePrivateShopInfo);
		uint32_t DelOfflinePrivateShopItem(uint32_t dwPos, LPCHARACTER ch, TShopItemData * pData = nullptr);
		void AddOfflinePrivateShopItem(uint32_t dwPos, uint8_t bInvenType, uint32_t dwSourcePos, uint64_t dwPrices, uint32_t dwShopVid, LPCHARACTER ch);
		std::vector<SHOP_ITEM>& GetItemVector() { return m_itemVector; }
		int32_t	RemoteBuy(TShopItemData * pData, uint64_t dwPrices, uint32_t pos);
		uint8_t& GetUnlockSlots() { return m_bUnlockSlots; }
		void SetUnlockSlots(uint8_t bUnlockSlots) { m_bUnlockSlots = bUnlockSlots; }

	protected:
		bool m_bIsOfflinePrivateShop;
		uint8_t m_bUnlockSlots;
#endif
};

