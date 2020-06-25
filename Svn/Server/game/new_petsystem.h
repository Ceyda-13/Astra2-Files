#ifndef	__HEADER_NEWPET_SYSTEM__
#define	__HEADER_NEWPET_SYSTEM__
class CHARACTER;

class CNewPetActor
{
	public:
		enum ENewPetOptions
		{
			EPetOption_Followable		= 1 << 0,
			EPetOption_Mountable		= 1 << 1,
			EPetOption_Summonable		= 1 << 2,
			EPetOption_Combatable		= 1 << 3,		
		};


	protected:
		friend class CNewPetSystem;

		CNewPetActor(LPCHARACTER owner, uint32_t vnum, uint32_t options = EPetOption_Followable | EPetOption_Summonable);

		virtual ~CNewPetActor();

		virtual bool	Update(uint32_t deltaTime);

	protected:
		virtual bool	_UpdateFollowAI();
		virtual bool	_UpdatAloneActionAI(float fMinDist, float fMaxDist);

	private:
		bool Follow(float fMinDistance = 50.f);

	public:
		LPCHARACTER		GetCharacter()	const { return m_pkChar; }
		LPCHARACTER		GetOwner()	const { return m_pkOwner; }
		uint32_t			GetVID() const { return m_dwVID; }
		uint32_t			GetVnum() const { return m_dwVnum; }

		bool			HasOption(ENewPetOptions option) const { return m_dwOptions & option; }

		void			SetName(const char* petName);
		void			SetLevel(uint8_t level);

		bool			Mount();
		void			Unmount();

		void			Summon(const char* petName, LPITEM pSummonItem, bool bSpawnFar = false);
		void			Unsummon();

		bool			IsSummoned() const { return 0 != m_pkChar; }
		void			SetSummonItem (LPITEM pItem);
		uint32_t			GetSummonItemVID () { return m_dwSummonItemVID; }
		uint32_t			GetSummonItemID () { return m_dwSummonItemID; }
		uint8_t			GetEvolution() { return m_dwevolution; }
		uint8_t			GetLevel() { return m_dwlevel; }
		void			SetEvolution(int32_t lv);
		void			SetExp(uint32_t exp, int32_t mode);
		uint32_t			GetExp() { return m_dwexp; }
		uint32_t			GetAge() { return m_dwaduration / 60 / 60 / 24; }
		uint32_t			GetExpI() { return m_dwexpitem; }
		void			SetNextExp(int32_t nextExp);
		uint32_t			GetNextExpFromMob() { return m_dwExpFromMob; }
		uint32_t			GetNextExpFromItem() { return m_dwExpFromItem; }
		uint32_t			GetLevelStep() { return m_dwlevelstep; }

		void			IncreasePetBonus();
		void			SetItemCube(int32_t pos, int32_t invpos);
		void			ItemCubeFeed(int32_t type);
		void			DoPetSkill(int32_t skillslot);
		void			UpdateTime();

		bool			IncreasePetSkill(int32_t skill);
		bool			IncreasePetEvolution();

		void			GiveBuff();
		void			ClearBuff();
		bool 			ItemFeed(LPITEM pkItem);
		void			SetType(int16_t type);
		bool			Revert(int8_t type, LPITEM item);

	private:
		uint32_t			m_dwlevelstep;
		uint32_t			m_dwExpFromMob;
		uint32_t			m_dwExpFromItem;
		uint32_t			m_dwexpitem;
		uint8_t				m_dwevolution;
		uint32_t			m_dwTimePet;
		uint32_t			m_dwslotimm;

		uint32_t		m_dwImmTime;

		int32_t				m_dwpetslotitem[9];
		uint8_t				m_dwskill[3];
		int16_t				m_dwskillslot[3];
		int32_t				m_dwbonuspet[3][2];
		uint32_t			m_dwVnum;
		uint32_t			m_dwVID;
		uint8_t				m_dwlevel;	
		uint32_t			m_dwexp;
		uint32_t			m_dwOptions;
		uint32_t			m_dwLastActionTime;
		uint32_t			m_dwSummonItemVID;
		uint32_t			m_dwSummonItemID;
		uint32_t			m_dwSummonItemVnum;
		int16_t				m_dwPetType;

		uint32_t			m_dwaduration;

		short			m_originalMoveSpeed;

		std::string		m_name;

		LPCHARACTER		m_pkChar;
		LPCHARACTER		m_pkOwner;
};

class CNewPetSystem
{
	public:
		typedef	std::unordered_map<uint32_t,	CNewPetActor*>		TNewPetActorMap;

	public:
		CNewPetSystem(LPCHARACTER owner);
		virtual ~CNewPetSystem();

		CNewPetActor*	GetByVID(uint32_t vid) const;
		CNewPetActor*	GetByVnum(uint32_t vnum) const;

		bool		Update(uint32_t deltaTime);
		void		Destroy();

		size_t		CountSummoned() const;

	public:
		void		SetUpdatePeriod(uint32_t ms);

		CNewPetActor*	Summon(uint32_t mobVnum, LPITEM pSummonItem, const char* petName, bool bSpawnFar, uint32_t options = CNewPetActor::EPetOption_Followable | CNewPetActor::EPetOption_Summonable);

		void		Unsummon(uint32_t mobVnum, bool bDeleteFromList = false);
		void		Unsummon(CNewPetActor* petActor, bool bDeleteFromList = false);
		
		bool		IncreasePetSkill(int32_t skill);
		bool		IncreasePetEvolution();	
		
		void		DeletePet(uint32_t mobVnum);
		void		DeletePet(CNewPetActor* petActor);
		void		RefreshBuff();
		bool		IsActivePet();
		uint32_t		GetNewPetITemID();
		void		SetExp(int32_t iExp, int32_t mode);
		int32_t			GetEvolution();
		int32_t			GetLevel();
		int32_t			GetExp();
		int32_t			GetAge();
		int32_t			GetLevelStep();
		void		SetItemCube(int32_t pos, int32_t invpos);
		void		ItemCubeFeed(int32_t type);
		void		DoPetSkill(int32_t skillslot);
		void		UpdateTime();
		bool 		ItemFeed(LPITEM pkItem);
		uint32_t 	GetSummonItemVID();
		void		SetType(int16_t type);
		bool		Revert(int8_t type, LPITEM item);

	private:
		TNewPetActorMap	m_petActorMap;
		LPCHARACTER		m_pkOwner;
		uint32_t			m_dwUpdatePeriod;
		uint32_t			m_dwLastUpdateTime;
		LPEVENT			m_pkNewPetSystemUpdateEvent;
		LPEVENT			m_pkNewPetSystemExpireEvent;
};
#endif	//__HEADER_PET_SYSTEM__

