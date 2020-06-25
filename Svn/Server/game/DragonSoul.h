#pragma once
#include "../common/length.h"

class CHARACTER;
class CItem;

class DragonSoulTable;

class DSManager: public singleton<DSManager>
{
	public:
		DSManager();
		~DSManager();
		bool	ReadDragonSoulTableFile(const char* c_pszFileName);

		void	GetDragonSoulInfo(uint32_t dwVnum, OUT uint8_t& bType, OUT uint8_t& bGrade, OUT uint8_t& bStep, OUT uint8_t& bRefine) const;
		uint16_t	GetBasePosition(const LPITEM pItem) const;
		bool	IsValidCellForThisItem(const LPITEM pItem, const TItemPos& Cell) const;
		int32_t		GetDuration(const LPITEM pItem) const;

		bool	ExtractDragonHeart(LPCHARACTER ch, LPITEM pItem, LPITEM pExtractor = nullptr);

		bool	PullOut(LPCHARACTER ch, TItemPos DestCell, IN OUT LPITEM& pItem, LPITEM pExtractor = nullptr);

		bool	DoRefineGrade(LPCHARACTER ch, TItemPos* aItemPoses);
		bool	DoRefineStep(LPCHARACTER ch, TItemPos* aItemPoses);
		bool	DoRefineStrength(LPCHARACTER ch, TItemPos* aItemPoses);

		bool	DragonSoulItemInitialize(LPITEM pItem);

		bool	IsTimeLeftDragonSoul(LPITEM pItem) const;
		int32_t		LeftTime(LPITEM pItem) const;
		bool	ActivateDragonSoul(LPITEM pItem);
		bool	DeactivateDragonSoul(LPITEM pItem, bool bSkipRefreshOwnerActiveState = false);
		bool	IsActiveDragonSoul(LPITEM pItem) const;
		void	SendRefineResultPacket(LPCHARACTER ch, uint8_t bSubHeader, const TItemPos& pos);

		void	RefreshDragonSoulState(LPCHARACTER ch);

		uint32_t	MakeDragonSoulVnum(uint8_t bType, uint8_t grade, uint8_t step, uint8_t refine);
		bool	PutAttributes(LPITEM pDS);
		bool	RefreshItemAttributes(LPITEM pItem);

	private:
		DragonSoulTable*	m_pTable;
};


