#pragma once
#include "sectree.h"


typedef struct SMapRegion
{
	int32_t			index;
	int32_t			sx, sy, ex, ey;
	PIXEL_POSITION	posSpawn;

	bool		bEmpireSpawnDifferent;
	PIXEL_POSITION	posEmpire[3];

	std::string		strMapName;
} TMapRegion;

struct TAreaInfo
{
	int32_t sx, sy, ex, ey, dir;
	TAreaInfo(int32_t sx, int32_t sy, int32_t ex, int32_t ey, int32_t dir)
		: sx(sx), sy(sy), ex(ex), ey(ey), dir(dir)
		{}
};

struct npc_info
{
	uint8_t bType;
	const char* name;
	int32_t x, y;
#ifdef __SHOW_MAP_BOSS_AND_STONE__
	uint32_t vid;
#endif
	npc_info(uint8_t bType, const char* name, int32_t x, int32_t y
#ifdef __SHOW_MAP_BOSS_AND_STONE__
, uint32_t vid
#endif
)
		: bType(bType), name(name), x(x), y(y)
#ifdef __SHOW_MAP_BOSS_AND_STONE__
, vid(vid)
#endif
{}
};

typedef std::map<std::string, TAreaInfo> TAreaMap;

typedef struct SSetting
{
	int32_t			iIndex;
	int32_t			iCellScale;
	int32_t			iBaseX;
	int32_t			iBaseY;
	int32_t			iWidth;
	int32_t			iHeight;

	PIXEL_POSITION	posSpawn;
} TMapSetting;

class SECTREE_MAP
{
	public:
		typedef std::map<uint32_t, LPSECTREE> MapType;

		SECTREE_MAP();
		SECTREE_MAP(SECTREE_MAP & r);
		virtual ~SECTREE_MAP();

		bool Add(uint32_t key, LPSECTREE sectree) {
			return map_.insert(MapType::value_type(key, sectree)).second;
		}

		LPSECTREE	Find(uint32_t dwPackage);
		LPSECTREE	Find(uint32_t x, uint32_t y);
		void		Build();

		TMapSetting	m_setting;

		template< typename Func >
		void for_each( Func & rfunc )
		{
			FCollectEntity collector;
			std::map<uint32_t, LPSECTREE>::iterator it = map_.begin();
			for ( ; it != map_.end(); ++it)
			{
				LPSECTREE sectree = it->second;
				sectree->for_each_entity(collector);
			}
			collector.ForEach(rfunc);
		}

		void DumpAllToSysErr() {
			SECTREE_MAP::MapType::iterator i;
			for (i = map_.begin(); i != map_.end(); ++i)
			{
				sys_err("SECTREE %x(%u, %u)", i->first, i->first & 0xffff, i->first >> 16);
			}
		}

	private:
		MapType map_;
};

enum EAttrRegionMode
{
	ATTR_REGION_MODE_SET,
	ATTR_REGION_MODE_REMOVE,
	ATTR_REGION_MODE_CHECK,
};

class SECTREE_MANAGER : public singleton<SECTREE_MANAGER>
{
	public:
		SECTREE_MANAGER();
		virtual ~SECTREE_MANAGER();

		LPSECTREE_MAP GetMap(int32_t lMapIndex);
		LPSECTREE 	Get(uint32_t dwIndex, uint32_t package);
		LPSECTREE 	Get(uint32_t dwIndex, uint32_t x, uint32_t y);

		template< typename Func >
		void for_each( int32_t iMapIndex, Func & rfunc )
		{
			LPSECTREE_MAP pSecMap = SECTREE_MANAGER::instance().GetMap( iMapIndex );
			if ( pSecMap )
			{
				pSecMap->for_each( rfunc );
			}
		}

		int32_t		LoadSettingFile(int32_t lIndex, const char * c_pszSettingFileName, TMapSetting & r_setting);
		bool		LoadMapRegion(const char * c_pszFileName, TMapSetting & r_Setting, const char * c_pszMapName);
		int32_t		Build(const char * c_pszListFileName, const char* c_pszBasePath);
		LPSECTREE_MAP BuildSectreeFromSetting(TMapSetting & r_setting);
		bool		LoadAttribute(LPSECTREE_MAP pkMapSectree, const char * c_pszFileName, TMapSetting & r_setting);
		void		LoadDungeon(int32_t iIndex, const char * c_pszFileName);
		bool		GetValidLocation(int32_t lMapIndex, int32_t x, int32_t y, int32_t & r_lValidMapIndex, PIXEL_POSITION & r_pos, uint8_t empire = 0);
		bool		GetSpawnPosition(int32_t x, int32_t y, PIXEL_POSITION & r_pos);
		bool		GetSpawnPositionByMapIndex(int32_t lMapIndex, PIXEL_POSITION & r_pos);
		bool		GetRecallPositionByEmpire(int32_t iMapIndex, uint8_t bEmpire, PIXEL_POSITION & r_pos);

		const TMapRegion *	GetMapRegion(int32_t lMapIndex);
		int32_t			GetMapIndex(int32_t x, int32_t y);
		const TMapRegion *	FindRegionByPartialName(const char* szMapName);

		bool		GetMapBasePosition(int32_t x, int32_t y, PIXEL_POSITION & r_pos);
		bool		GetMapBasePositionByMapIndex(int32_t lMapIndex, PIXEL_POSITION & r_pos);
		bool		GetMovablePosition(int32_t lMapIndex, int32_t x, int32_t y, PIXEL_POSITION & pos);
		bool		IsMovablePosition(int32_t lMapIndex, int32_t x, int32_t y);
		bool		GetCenterPositionOfMap(int32_t lMapIndex, PIXEL_POSITION & r_pos);
		bool        GetRandomLocation(int32_t lMapIndex, PIXEL_POSITION & r_pos, uint32_t dwCurrentX = 0, uint32_t dwCurrentY = 0, int32_t iMaxDistance = 0);

		int32_t		CreatePrivateMap(int32_t lMapIndex);
		void		DestroyPrivateMap(int32_t lMapIndex);

		TAreaMap&	GetDungeonArea(int32_t lMapIndex);
		void		SendNPCPosition(LPCHARACTER ch);
		void		InsertNPCPosition(int32_t lMapIndex, uint8_t bType, const char* szName, int32_t x, int32_t y
#ifdef __SHOW_MAP_BOSS_AND_STONE__
, uint32_t vid = 0
#endif
);

		uint8_t		GetEmpireFromMapIndex(int32_t lMapIndex);

		void		PurgeMonstersInMap(int32_t lMapIndex);
		void		PurgeStonesInMap(int32_t lMapIndex);
		void		PurgeNPCsInMap(int32_t lMapIndex);
		size_t		GetMonsterCountInMap(int32_t lMapIndex);
#ifdef __GUILD_DRAGONLAIR__
		size_t		GetMonsterCountInMap(int32_t lMapIndex, uint32_t dwVnum, bool bForceVnum = false);
#else
		size_t		GetMonsterCountInMap(int32_t lMapIndex, uint32_t dwVnum);
#endif

		bool		ForAttrRegion(int32_t lMapIndex, int32_t lStartX, int32_t lStartY, int32_t lEndX, int32_t lEndY, int32_t lRotate, uint32_t dwAttr, EAttrRegionMode mode);

		bool		SaveAttributeToImage(int32_t lMapIndex, const char * c_pszFileName, LPSECTREE_MAP pMapSrc = nullptr);

#ifdef __TEMPLE_OCHAO__
		void		GetRestartCityPos(int32_t iMapIndex, int32_t iEmpire, int32_t &iTargetX, int32_t &iTargetY, int32_t &iTargetZ);
		void		AddRestartCityPos(int32_t iMapIndex = 0, int32_t iEmpire = 0, int32_t iX = 0, int32_t iY = 0, int32_t iZ = 0);
#endif
	private:
		bool		ForAttrRegionRightAngle( int32_t lMapIndex, int32_t lCX, int32_t lCY, int32_t lCW, int32_t lCH, int32_t lRotate, uint32_t dwAttr, EAttrRegionMode mode );

		bool		ForAttrRegionFreeAngle( int32_t lMapIndex, int32_t lCX, int32_t lCY, int32_t lCW, int32_t lCH, int32_t lRotate, uint32_t dwAttr, EAttrRegionMode mode );

		bool		ForAttrRegionCell( int32_t lMapIndex, int32_t lCX, int32_t lCY, uint32_t dwAttr, EAttrRegionMode mode );

		static uint16_t			current_sectree_version;
		std::map<uint32_t, LPSECTREE_MAP>	m_map_pkSectree;
		std::map<int32_t, TAreaMap>	m_map_pkArea;
		std::vector<TMapRegion>		m_vec_mapRegion;
		std::map<uint32_t, std::vector<npc_info> > m_mapNPCPosition;

		typedef std::unordered_map<int32_t, int32_t> PrivateIndexMapType;
		PrivateIndexMapType next_private_index_map_;
#ifdef __SHOW_MAP_BOSS_AND_STONE__
	public:
		void		RemoveNPCPosition(int32_t lMapIndex, int32_t x, int32_t y, uint32_t vid);
#endif
};


