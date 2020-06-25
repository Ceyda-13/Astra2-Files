#pragma once
#define TEMPLE_OCHAO_MAP_INDEX 39
#define TEMPLE_OCHAO_ROOMS 11
#define TEMPLE_OCHAO_GUARDIAN 6400
#define TEMPLE_OCHAO_PORTAL 20415
#define TEMPLE_OCHAO_PORTAL_SHOW 60
#define TEMPLE_OCHAO_CHECK_ACTIVITY 30
#define TEMPLE_OCHAO_NO_ACTIVITY 180
#define TEMPLE_OCHAO_ATTACKED 600
#define TEMPLE_OCHAO_BOSS 6192
#define TEMPLE_OCHAO_HEALLER 6409
namespace TempleOchao
{
	class CMgr : public singleton<CMgr>
	{
		public:
			void	Initialize();
			void	Destroy();
			void	Prepare();
			void	Spawn();
			void	OnGuardianKilled(int32_t iX, int32_t iY, int32_t iZ);
			void	ClearPrepare();
			void	ChangeRoom();
			void	SetRoom(int32_t iRoom) {i_room = iRoom;}
			int32_t		GetRoom() {return i_room;}
			void	RandomRoom(int32_t &iRoom, int32_t &iX, int32_t &iY, int32_t &iZ);
			void	SetGuardianVID(uint32_t dwGuardianVID) {dw_GuardianVID = dwGuardianVID;}
			uint32_t	GetGuardianVID() {return dw_GuardianVID;}
			void	SetPortalVID(uint32_t dwPortalVID) {dw_PortalVID = dwPortalVID;}
			uint32_t	GetPortalVID() {return dw_PortalVID;}
			void	GuardianAttacked();
		private:
			int32_t		i_room;
			uint32_t	dw_GuardianVID, dw_PortalVID;
			bool	b_OnGuardianKilled;
		protected:
			LPEVENT	s_pkGuardianKilled, s_pkGuardianActivity;
	};
}

