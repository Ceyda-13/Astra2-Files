#pragma once
#include <cstdint>

#include "Packet.h"

/*
 *	상점 처리
 *
 *	2003-01-16 anoa	일차 완료
 *	2003-12-26 levites 수정
 *
 *	2012-10-29 rtsummit 새로운 화폐 출현 및 tab 기능 추가로 인한 shop 확장.
 *
 */
typedef enum
{
	SHOP_COIN_TYPE_GOLD, // DEFAULT VALUE
	SHOP_COIN_TYPE_SECONDARY_COIN,
} EShopCoinType;

class CPythonShop : public CSingleton<CPythonShop>
{
	public:
		CPythonShop(void);
		virtual ~CPythonShop(void);

		void Clear();

		void SetItemData(DWORD dwIndex, const TShopItemData & c_rShopItemData);
		BOOL GetItemData(DWORD dwIndex, const TShopItemData ** c_ppItemData);

#ifndef ENABLE_SHOP_TAB
		void SetItemData(BYTE tabIdx, DWORD dwSlotPos, const TShopItemData & c_rShopItemData);
		BOOL GetItemData(BYTE tabIdx, DWORD dwSlotPos, const TShopItemData ** c_ppItemData);
		void SetTabCoinType(BYTE tabIdx, BYTE coinType);
		BYTE GetTabCoinType(BYTE tabIdx);
		void SetTabName(BYTE tabIdx, const char* name);
		const char* GetTabName(BYTE tabIdx);
#endif
		void SetTabCount(BYTE bTabCount) { m_bTabCount = bTabCount; }
		BYTE GetTabCount() { return m_bTabCount; }

		//BOOL GetSlotItemID(DWORD dwSlotPos, DWORD* pdwItemID);

		void Open(BOOL isPrivateShop, BOOL isMainPrivateShop);
		void Close();
		BOOL IsOpen();
		BOOL IsPrivateShop();
		BOOL IsMainPlayerPrivateShop();

		void ClearPrivateShopStock();
		void AddPrivateShopItemStock(TItemPos ItemPos, BYTE byDisplayPos, uint64_t dwPrice);
		void DelPrivateShopItemStock(TItemPos ItemPos);
		uint64_t GetPrivateShopItemPrice(TItemPos ItemPos);
		void BuildPrivateShop(const char * c_szName
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
, int iTime, int iTitle, int iPoly
#endif
);

	protected:
		BOOL	CheckSlotIndex(DWORD dwIndex);

	protected:
		BOOL				m_isShoping;
		BOOL				m_isPrivateShop;
		BOOL				m_isMainPlayerPrivateShop;

		struct ShopTab
		{
			ShopTab()
			{
				coinType = SHOP_COIN_TYPE_GOLD;
			}
			BYTE				coinType;
			std::string			name;
			TShopItemData		items[SHOP_HOST_ITEM_MAX_NUM];
		};

		BYTE m_bTabCount;
#ifdef ENABLE_SHOP_TAB
		TShopItemData items[SHOP_HOST_ITEM_MAX_NUM];
#else
		ShopTab m_aShoptabs[SHOP_TAB_COUNT_MAX];
#endif
		typedef std::map<TItemPos, TShopItemTable> TPrivateShopItemStock;
		TPrivateShopItemStock	m_PrivateShopItemStock;

#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
	public:
		uint8_t& GetUnlockSlots() { return m_bUnlockSlots; }
		void SetUnlockSlots(uint8_t bUnlockSlots) { m_bUnlockSlots = bUnlockSlots; }
	protected:
		uint8_t m_bUnlockSlots;
#endif
};
