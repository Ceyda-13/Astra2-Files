#pragma once
#include "Packet.h"

class CPythonOffflinePrivateShop : public CSingleton<CPythonOffflinePrivateShop>
{
	public:
		CPythonOffflinePrivateShop(void);
		virtual ~CPythonOffflinePrivateShop(void);

		void Clear(int8_t iType);
		typedef std::vector<TOfflinePrivateShopInfo*> TOffflinePrivateShopInfo;
		typedef std::vector<TOfflinePrivateShopItemSearch*> TOffflinePrivateShopSearch;
		typedef std::vector<TOfflinePrivateShopLog*> TOffflinePrivateShopLog;

		const TShopItemData * GetOffflinePrivateShopItemData(BYTE bType, int id, DWORD dwSlotId) const;
		TShopItemData * GetOffflinePrivateShopItemData(DWORD dwVid, DWORD dwSlotId);
		DWORD GetItemMetinSocket(BYTE bType, int id, DWORD dwSlotId, int dwMetinSocketIndex);
		void GetItemAttribute(BYTE bType, int id, DWORD dwSlotId, int dwAttrSlotIndex, BYTE * pbyType, int * psValue);
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		DWORD GetItemChangeLookVnum(BYTE bType, int id, DWORD dwSlotId);
#endif
		TOffflinePrivateShopInfo & GetInfoVectorRef() { return m_OffflinePrivateShopData; }
		TOffflinePrivateShopSearch& GetSearchInfoVectorRef() { return m_OffflinePrivateShopSearchData; }
		TOffflinePrivateShopLog& GetLogVectorRef() { return m_OffflinePrivateShopLogData; }

	protected:
		TOffflinePrivateShopInfo m_OffflinePrivateShopData;
		TOffflinePrivateShopSearch m_OffflinePrivateShopSearchData;
		TOffflinePrivateShopLog m_OffflinePrivateShopLogData;
};

