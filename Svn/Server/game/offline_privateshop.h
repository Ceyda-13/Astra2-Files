#pragma once
#include "vid.h"
#include "entity.h"
#include "sectree_manager.h"
#include "../common/tables.h"
#include "packet.h"

class COfflinePrivateShop : public CEntity
{
	public:
		COfflinePrivateShop(int32_t iTitle, int32_t iPoly, uint32_t dwTime, LPSHOP pkShop);
		virtual ~COfflinePrivateShop(void);

		void Initialize(void);
		void Destroy(void);

		uint32_t GetRaceVnum() { return m_dwRaceVnum; }
		uint32_t GetVid() { return m_dwVid; }
		uint32_t GetTime() { return m_dwEndTime; }
		uint32_t GetShopVid() { return m_dwShopVid; }
		uint32_t GetPlayerID() { return m_dwPlayerID; }
		std::string GetOwnerName() { return m_strOwnerName; }
		LPSHOP GetShop() { return m_pkShop; }
		uint32_t GetDeco() { return m_dwTitle; }
		void SetDeco(uint32_t dwTitle) { m_dwTitle = dwTitle; }
		void SetShopVid(uint32_t dwShopVid) { m_dwShopVid = dwShopVid; }
		void SetSign(const std::string& strSign) { m_strSign = strSign; }
		void SetOwnerName(const std::string& strOwnerName) { m_strOwnerName = strOwnerName; }
		void SetPlayerID(uint32_t dwPlayerID) { m_dwPlayerID = dwPlayerID; }

		const uint64_t GetPrices();
		void GetInfo(TOfflinePrivateShopInfo * pkTOfflinePrivateShopInfo);

	protected:
		virtual void EncodeInsertPacket(LPENTITY ent);
		virtual void EncodeRemovePacket(LPENTITY ent);

		uint32_t m_dwRaceVnum;
		uint32_t m_dwVid;
		uint32_t m_dwEndTime;
		uint32_t m_dwStartTime;
		uint32_t m_dwShopVid;
		uint32_t m_dwPlayerID;
		LPSHOP m_pkShop;
		std::string m_strSign;
		std::string m_strOwnerName;
		uint32_t m_dwTitle;
};

