#pragma once
#include "shop.h"

class CShopManager : public singleton<CShopManager>
{
public:
	typedef std::map<uint32_t, CShop *> TShopMap;
#ifdef __OFFLINE_PRIVATE_SHOP__
	typedef std::map<uint32_t, COfflinePrivateShop*> TOfflinePrivateShopMap;
	typedef std::vector<TPriceCheck> TPriceCheckVector;
#endif


public:
	CShopManager();
	virtual ~CShopManager();

	bool	Initialize(TShopTable * table, int32_t size);
	void	Destroy();

	LPSHOP	Get(uint32_t dwVnum);
	LPSHOP	GetByNPCVnum(uint32_t dwVnum);

	bool	StartShopping(LPCHARACTER pkChr, LPCHARACTER pkShopKeeper, int32_t iShopVnum = 0);
	void	StopShopping(LPCHARACTER ch);

	void	Buy(LPCHARACTER ch, uint8_t pos);
	void	Sell(LPCHARACTER ch, uint8_t bCell, uint16_t bCount=0);

	LPSHOP	CreatePCShop(LPCHARACTER ch, TShopItemTable * pTable, uint8_t bItemCount);
	LPSHOP	FindPCShop(uint32_t dwVID);
	void	DestroyPCShop(LPCHARACTER ch);

private:
	TShopMap	m_map_pkShop;
	TShopMap	m_map_pkShopByNPCVnum;
	TShopMap	m_map_pkShopByPC;

	bool	ReadShopTableEx(const char* stFileName);

#ifdef __OFFLINE_PRIVATE_SHOP__
	public:
		void CreateOfflinePrivateShop(LPCHARACTER ch, const char* szSign, int32_t iTitle, int32_t iPoly, uint32_t dwTime, TShopItemTable * pTable, uint8_t bItemCount);
		LPOPSHOP FindOfflinePrivateShop(uint32_t dwShopVid);
		LPOPSHOP FindOfflinePrivateShopByShopVID(uint32_t dwVid);
		void DestroyOfflinePrivateShop(uint32_t dwShopVid);
		void BuildOfflinePrivateShop();
		void StartOfflinePrivateShop(LPCHARACTER ch, uint32_t dwVid);
		void AddBankOfflinePrivateShop(LPSHOP pkShop, uint64_t dwPrices);
		uint8_t GetOfflinePrivateShopCount(uint32_t dwVid);
		void SetOfflinePrivateShops(LPOPSHOP pkOfflinePrivateShops[], uint8_t bCount, uint32_t dwVid);
		TOfflinePrivateShopMap& GetOfflinePrivateShops() { return m_map_pkPrivateOfflineShop; }
		void SearchShops(std::vector<CShop::SHOP_ITEM>& sendItems, uint64_t* dwArgs, uint8_t bArgSize, const char* itemName, uint32_t dwSearcherPID);
		TPriceCheckVector& GetPriceChecks() { return vec_check; }
		bool ReadPriceCheckTable(const char* stFileName);

	private:
		TOfflinePrivateShopMap m_map_pkPrivateOfflineShop;
		TPriceCheckVector vec_check;
#endif
};

