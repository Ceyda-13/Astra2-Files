#pragma once
class CHARACTER;

class CSupportActor
{
	public:
		enum EShamanOptions
		{
			ESupportOption_Followable		= 1 << 0,
			ESupportOption_Mountable		= 1 << 1,
			ESupportOption_Summonable		= 1 << 2,
			ESupportOption_Combatable		= 1 << 3,		
		};

	protected:
		friend class CSupport;
		CSupportActor(LPCHARACTER owner, uint32_t vnum, uint32_t options = ESupportOption_Followable | ESupportOption_Summonable);
		virtual ~CSupportActor();
		virtual bool	Update(uint32_t deltaTime);

	protected:
		virtual bool	_UpdateFollowAI();
		virtual bool	_UpdatAloneActionAI(float fMinDist, float fMaxDist);

	private:
		bool Follow(float fMinDistance = 50.f);

	public:
		LPCHARACTER		GetCharacter()	const					{ return m_pkChar; }
		LPCHARACTER		GetOwner()	const						{ return m_pkOwner; }
		uint32_t			GetVID() const							{ return m_dwVID; }
		uint32_t			GetVnum() const							{ return m_dwVnum; }

		bool			HasOption(EShamanOptions option) const		{ return m_dwOptions & option; }

		void			SetName();
		void			SetLevel(uint32_t level);

		bool			Mount();
		void			Unmount();

		uint32_t			Summon(const char* supportName, LPITEM pSummonItem, bool bSpawnFar = false);
		void			Unsummon();

		bool			IsSummoned() const			{ return 0 != m_pkChar; }
		void			SetSummonItem (LPITEM pItem);
		uint32_t			GetLevel() { return m_dwlevel; }
		void			SetLastSkillTime(uint32_t time)	{ m_dwLastSkillTime = time; }
		void			SetCostume();
		void			SetHair();
		void			SetWeapon();
		uint32_t			GetLastSkillTime()	{ return m_dwLastSkillTime; }
		void			SetLastExpUpdate(uint32_t time)	{ m_dwLastExpUpdateTime = time; }
		uint32_t			GetLastExpUpdate()	{ return m_dwLastExpUpdateTime; }
		void			SetExp(uint32_t exp);
		void			UpdateItemExp();
		void			UpdatePacketSupportActor();
		void			RefreshCostume();
		uint32_t			GetExp() { return m_dwExp; }
		void			SetNextExp(int32_t nextExp);
		uint32_t			GetNextExp() { return m_dwNextExp; }
		int32_t				GetIntSkill();
		void			UseSkill();
		void			SetIntSkill();
		uint32_t			GetSummonItemVID () { return m_dwSummonItemVID; }

		void			GiveBuff();
		void			ClearBuff();

	private:
		uint32_t			m_dwVnum;
		uint32_t			m_dwVID;
		uint32_t			m_dwOptions;
		uint32_t			m_dwLastActionTime;
		uint32_t			m_dwSummonItemVID;
		uint32_t			m_dwSummonItemVnum;
		uint32_t			m_dwExp;
		uint32_t			m_dwLastSkillTime;
		uint32_t			m_dwLastExpUpdateTime;
		uint32_t			m_dwIntSkill;
		uint32_t			m_dwlevel;
		uint32_t			m_dwArmor;
		uint32_t			m_dwHair;
		uint32_t			m_dwHairVnum;
		uint32_t			m_dwWeapon;
		uint32_t			m_dwNextExp;
		short			m_originalMoveSpeed;
		
		std::string		first_name;
		std::string		second_name;
		std::string		m_speciality;
		LPCHARACTER		m_pkChar;
		LPCHARACTER		m_pkOwner;
};

class CSupport
{
	public:
		typedef	std::unordered_map<uint32_t,	CSupportActor*>		TSupportActorMap;		

	public:
		CSupport(LPCHARACTER owner);
		virtual ~CSupport();

		CSupportActor*	GetByVID(uint32_t vid) const;
		CSupportActor*	GetByVnum(uint32_t vnum) const;

		bool		Update(uint32_t deltaTime);
		void		Destroy();

		size_t		CountSummoned() const;

	public:
		void		SetUpdatePeriod(uint32_t ms);

		CSupportActor*	Summon(uint32_t mobVnum, LPITEM pSummonItem, const char* supportName, bool bSpawnFar, uint32_t options = CSupportActor::ESupportOption_Followable | CSupportActor::ESupportOption_Summonable);

		void		Unsummon(uint32_t mobVnum, bool bDeleteFromList = false);
		void		Unsummon(CSupportActor* supportActor, bool bDeleteFromList = false);
		void		SetExp(int32_t iExp);
		int32_t			GetLevel();
		uint32_t		GetExp();
		bool		IsActiveSupport();
		CSupportActor*   GetActiveSupport();

		void		DeleteSupport(uint32_t mobVnum);
		void		DeleteSupport(CSupportActor* supportActor);
		void		RefreshBuff();
		
	private:
		TSupportActorMap	m_SupportActorMap;
		LPCHARACTER		m_pkOwner;					
		uint32_t			m_dwUpdatePeriod;			
		uint32_t			m_dwLastUpdateTime;
		LPEVENT			m_pkSupportUpdateEvent;
};

