#pragma once

#include <map>
#include <set>
#include "Peer.h"

typedef struct SItemAward
{
    uint32_t	dwID;
    char	szLogin[LOGIN_MAX_LEN+1];
    uint32_t	dwVnum;
    uint32_t	dwCount;
    uint32_t	dwSocket0;
    uint32_t	dwSocket1;
    uint32_t	dwSocket2;
// @correction144 BEGIN
    uint32_t	dwSocket3;
    uint32_t	dwSocket4;
    uint32_t	dwSocket5;
// @correction144 END
    char	szWhy[ITEM_AWARD_WHY_MAX_LEN+1];
    bool	bTaken;
    bool	bMall;
} TItemAward;

class ItemAwardManager : public singleton<ItemAwardManager>
{
    public:
	ItemAwardManager();
	virtual ~ItemAwardManager();

	void				RequestLoad();
	void				Load(SQLMsg * pMsg);
	std::set<TItemAward *> *	GetByLogin(const char * c_pszLogin);

	void				Taken(uint32_t dwAwardID, uint32_t dwItemID);
	std::map<uint32_t, TItemAward *>& GetMapAward();
	std::map<std::string, std::set<TItemAward *> >& GetMapkSetAwardByLogin();
    private:
	std::map<uint32_t, TItemAward *>			m_map_award;
	std::map<std::string, std::set<TItemAward *> >	m_map_kSetAwardByLogin;
};

