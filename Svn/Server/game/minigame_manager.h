#pragma once
#include "../common/length.h"
#include "../common/tables.h"
#include "../common/CommonDefines.h"
#include "packet.h"

class CMiniGameManager : public singleton<CMiniGameManager>
{
	public:
		CMiniGameManager();
		virtual ~CMiniGameManager();

		void Initialize();
		void Destroy();

		void SpawnNPC(uint32_t dwVnum, uint8_t bType);
		void SpawnRegen(bool bIsEnable, uint8_t bType);
		void SetEvent(bool bIsEnable, uint8_t bType);
		void SetCharecterEvents(LPCHARACTER ch);

#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
		bool ReadRewardItemFile(const char * c_pszFileName);
		void RequestRewardList(LPCHARACTER pChar);
		void AttendanceButtonClick(LPCHARACTER pChar, const char * data);
		void RequestData(LPCHARACTER pChar, const char * data);
#endif

	protected:
		uint8_t bEventState[MINIGAME_TYPE_MAX];

#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
		std::vector<TRewardItem>	attendanceRewardVec;
#endif
};

