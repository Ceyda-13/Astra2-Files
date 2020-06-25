#include "stdafx.h"
#include "utils.h"
#include "vector.h"
#include "char.h"
#include "sectree_manager.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "supportsystem.h"
#include "../common/VnumHelper.h"
#include "packet.h"
#include "item_manager.h"
#include "item.h"
#include "affect.h"
#include "skill.h"
#include "db.h"

extern int32_t passes_per_sec;
EVENTINFO(Support_event_info)
{
	CSupport* pSupport;
};

EVENTFUNC(Support_update_event)
{
	Support_event_info* info = dynamic_cast<Support_event_info*>( event->info );
	if ( info == nullptr )
	{
		sys_err( "check_speedhack_event> <Factor> Null pointer" );
		return 0;
	}

	CSupport*	pSupport = info->pSupport;

	if (nullptr == pSupport)
		return 0;

	
	pSupport->Update(0);
	return PASSES_PER_SEC(1) / 4;
}
const float SUPPORT_COUNT_LIMIT = 3;

CSupportActor::CSupportActor(LPCHARACTER owner, uint32_t vnum, uint32_t options)
{
	m_dwVnum = vnum;
	m_dwVID = 0;
	m_dwOptions = options;
	m_dwLastActionTime = 0;
	
	m_dwlevel = 1;
	m_dwExp = 0;
	m_dwNextExp = 0;
	m_dwIntSkill = 3;

	m_dwArmor = 0;
	m_dwHair = 0;
	m_dwHairVnum = 0;
	m_dwWeapon = 0;
	
	m_pkChar = 0;
	m_pkOwner = owner;

	m_originalMoveSpeed = 0;
	
	m_dwSummonItemVID = 0;
	m_dwSummonItemVnum = 0;
}

CSupportActor::~CSupportActor()
{
	this->Unsummon();

	m_pkOwner = 0;
}

void CSupportActor::SetName()
{
	std::string supportName = m_pkChar->GetOwner()->GetName();

	std::string supportnames = " - Destek";

	if (true == IsSummoned())
		m_pkChar->SetName(supportName);
	
	first_name = m_pkChar->GetOwner()->GetName();
	second_name = supportnames;
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportName %s%s", first_name.c_str(), second_name.c_str());
}

void CSupportActor::SetNextExp(int32_t nextExp)
{	
	m_dwNextExp = nextExp;
}

void CSupportActor::SetIntSkill()
{	
	if (GetLevel() * 1 >= 90)
		m_dwIntSkill = 90;
	else
		m_dwIntSkill = GetLevel()*3;
	
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != nullptr)
		pSummonItem->SetSocket(3, m_dwIntSkill);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupporInt %d", m_dwIntSkill);
}

int32_t CSupportActor::GetIntSkill()
{
	int32_t k=0;
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if(pSummonItem != nullptr)
		k = pSummonItem->GetAttributeValue(1) + pSummonItem->GetAttributeValue(2);	
	
	return k;	
}
void CSupportActor::SetLevel(uint32_t level)
{
	m_pkChar->SetLevel(static_cast<char>(level));
	m_dwlevel = level;
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportLv %d", m_dwlevel);	
	SetNextExp(m_pkChar->SupportGetNextExp());
	SetIntSkill();
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != nullptr)
		pSummonItem->SetSocket(2, m_dwlevel);
}
void CSupportActor::SetCostume()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != nullptr)
	{
		m_pkChar->SetSupportArmor(pSummonItem->GetSocket(4));
	}
}

void CSupportActor::SetHair()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != nullptr)
	{
		m_pkChar->SetSupportHair(pSummonItem->GetAttributeValue(0));
	}
}

void CSupportActor::SetWeapon()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != nullptr)
	{
		m_pkChar->SetSupportWeapon(pSummonItem->GetSocket(5));
	}
}

void CSupportActor::SetExp(uint32_t exp)
{
	if ((int32_t)exp < 0)
		exp = MAX(m_dwExp - exp, 0);

	if (GetExp() + exp >= GetNextExp())
	{
		SetLevel(GetLevel() + 1);
		m_pkChar->UpdatePacket();
		m_dwExp = 0;
		m_pkChar->SetExp(0);
		return;

	}
	m_dwExp += exp;
	m_pkChar->SetExp(m_dwExp);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportExp %d %d", m_dwExp, m_pkChar->SupportGetNextExp());
}

bool CSupportActor::Mount()
{
	if (0 == m_pkOwner)
		return false;

	if (true == HasOption(ESupportOption_Mountable))
		m_pkOwner->MountVnum(m_dwVnum);

	return m_pkOwner->GetMountVnum() == m_dwVnum;;
}

void CSupportActor::Unmount()
{
	if (0 == m_pkOwner)
		return;

	if (m_pkOwner->IsHorseRiding())
		m_pkOwner->StopRiding();
}

void CSupportActor::Unsummon()
{
	if (true == this->IsSummoned())
	{
		this->ClearBuff();
		LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
		if (pSummonItem != nullptr)
		{
			pSummonItem->SetSocket(1, m_dwExp);
			pSummonItem->SetSocket(2, m_dwlevel);
			pSummonItem->SetSocket(3, m_dwIntSkill);
			pSummonItem->SetSocket(4, m_dwArmor);
			pSummonItem->SetSocket(5, m_dwWeapon);
			pSummonItem->SetForceAttribute(0, 1, m_dwHair);
			pSummonItem->SetForceAttribute(1, 1, m_dwHairVnum);
			pSummonItem->Lock(false);
		}
		this->SetSummonItem(nullptr);
		if (nullptr != m_pkOwner)
			m_pkOwner->ComputePoints();

		if (nullptr != m_pkChar)
			M2_DESTROY_CHARACTER(m_pkChar);

		m_pkChar = 0;
		m_dwVID = 0;
		m_dwlevel = 1;
		m_dwExp = 0;
		m_dwNextExp = 0;
		m_dwIntSkill = 0;

		m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportLeave");
	}
}


uint32_t CSupportActor::Summon(const char* supportName, LPITEM pSummonItem, bool bSpawnFar)
{
	int32_t x = m_pkOwner->GetX();
	int32_t y = m_pkOwner->GetY();
	int32_t z = m_pkOwner->GetZ();

	if (true == bSpawnFar)
	{
		x += (number(0, 1) * 2 - 1) * number(2000, 2500);
		y += (number(0, 1) * 2 - 1) * number(2000, 2500);
	}
	else
	{
		x += number(-100, 100);
		y += number(-100, 100);
	}

	if (0 != m_pkChar)
	{
		m_pkChar->Show (m_pkOwner->GetMapIndex(), x, y);
		m_dwVID = m_pkChar->GetVID();

		return m_dwVID;
	}
	
	m_pkChar = CHARACTER_MANAGER::instance().SpawnMob(
				m_dwVnum, 
				m_pkOwner->GetMapIndex(), 
				x, y, z,
				false, (int32_t)(m_pkOwner->GetRotation()+180), false);

	if (0 == m_pkChar)
	{
		sys_err("[CSupport::Summon] Failed to summon the shaman. (vnum: %d)", m_dwVnum);
		return 0;
	}

	m_pkChar->SetSupport();
	
	m_pkChar->SetOwner(m_pkOwner);
	m_pkChar->SetEmpire(m_pkOwner->GetEmpire());

	m_dwVID = m_pkChar->GetVID();

	this->m_dwExp = pSummonItem->GetSocket(1);
	this->m_dwlevel = pSummonItem->GetSocket(2);
	this->m_dwIntSkill = pSummonItem->GetSocket(3);
	this->m_dwArmor = pSummonItem->GetSocket(4);
	this->m_dwWeapon = pSummonItem->GetSocket(5);
	this->m_dwHair = pSummonItem->GetAttributeValue(0);
	this->m_dwHairVnum = pSummonItem->GetAttributeValue(1);

	this->SetSummonItem(pSummonItem);

	if (!this->m_dwArmor)
	{
		pSummonItem->SetSocket(4, 11809);
		this->m_dwArmor = 11809;
	}
	if (!this->m_dwWeapon)
	{
		pSummonItem->SetSocket(5, 7009);
		this->m_dwWeapon = 7009;
	}

	this->SetCostume();
	this->SetHair();
	this->SetWeapon();
	if (!m_dwlevel)
	{
		m_dwlevel = 1;
		m_dwExp = 0;
		this->SetLevel(1);
	}
	else
		this->SetLevel(m_dwlevel);
	m_dwNextExp = m_pkChar->SupportGetNextExp();
	
	this->SetName();
	
	std::string m_speciality;
	if (m_dwSummonItemVnum == 61019 || m_dwSummonItemVnum == 61021 || m_dwSummonItemVnum == 61022)
		m_speciality = "E.Gücü";
	else if (m_dwSummonItemVnum == 61020 || m_dwSummonItemVnum == 61023)
		m_speciality = "Ýyileþtirme";

	m_pkOwner->ComputePoints();
	m_pkChar->Show(m_pkOwner->GetMapIndex(), x, y, z);
	m_pkChar->UpdatePacket();
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportLv %d", m_dwlevel);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportArmor %d", m_dwArmor);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportWeapon %u", m_dwWeapon);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportHair %u", m_dwHairVnum);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupporInt %d", m_dwIntSkill);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportExp %d %d", m_dwExp, m_pkChar->SupportGetNextExp());
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportIcon %d", m_dwSummonItemVnum);
	m_pkOwner->ChatPacket(CHAT_TYPE_COMMAND, "SupportSpeciality %s", m_speciality.c_str());
	pSummonItem->Lock(true);
	return m_dwVID;
}
void CSupportActor::RefreshCostume()
{
	SetCostume();
	SetHair();
	SetWeapon();
	m_pkChar->UpdatePacket();
}
void CSupportActor::UpdatePacketSupportActor()
{
	m_pkChar->UpdatePacket();
}
bool CSupportActor::_UpdatAloneActionAI(float fMinDist, float fMaxDist)
{
	float fDist = number(fMinDist, fMaxDist);
	float r = (float)number (0, 359);
	float dest_x = GetOwner()->GetX() + fDist * cos(r);
	float dest_y = GetOwner()->GetY() + fDist * sin(r);

	m_pkChar->SetNowWalking(true);

	if (!m_pkChar->IsStateMove() && m_pkChar->Goto(dest_x, dest_y))
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	m_dwLastActionTime = get_dword_time();

	return true;
}

bool CSupportActor::_UpdateFollowAI()
{
	if (0 == m_pkChar->m_pkMobData)
	{
		return false;
	}
	
	if (0 == m_originalMoveSpeed)
	{
		const CMob* mobData = CMobManager::Instance().Get(m_dwVnum);

		if (0 != mobData)
			m_originalMoveSpeed = mobData->m_table.sMovingSpeed;
	}
	float	START_FOLLOW_DISTANCE = 300.0f;		
	float	START_RUN_DISTANCE = 400.0f;		

	float	RESPAWN_DISTANCE = 4500.f;			
	int32_t		APPROACH = 200;						

	bool bRun = false;							

	uint32_t currentTime = get_dword_time();

	int32_t ownerX = m_pkOwner->GetX();		int32_t ownerY = m_pkOwner->GetY();
	int32_t charX = m_pkChar->GetX();			int32_t charY = m_pkChar->GetY();

	float fDist = DISTANCE_APPROX(charX - ownerX, charY - ownerY);

	if (fDist >= RESPAWN_DISTANCE)
	{
		float fOwnerRot = m_pkOwner->GetRotation() * 3.141592f / 180.f;
		float fx = -APPROACH * cos(fOwnerRot);
		float fy = -APPROACH * sin(fOwnerRot);
		if (m_pkChar->Show(m_pkOwner->GetMapIndex(), ownerX + fx, ownerY + fy))
		{
			return true;
		}
	}
	
	
	if (fDist >= START_FOLLOW_DISTANCE)
	{
		if( fDist >= START_RUN_DISTANCE)
		{
			bRun = true;
		}

		m_pkChar->SetNowWalking(!bRun);
		
		Follow(APPROACH);

		m_pkChar->SetLastAttacked(currentTime);
		m_dwLastActionTime = currentTime;
	}
	else 
		m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0);

	return true;
}

bool CSupportActor::Update(uint32_t deltaTime)
{
	bool bResult = true;

	if (m_pkOwner->IsDead() || (IsSummoned() && m_pkChar->IsDead()) 
		|| nullptr == ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())
		|| ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID())->GetOwner() != this->GetOwner()
		)
	{
		this->Unsummon();
		return true;
	}
	if(get_dword_time() - this->GetLastExpUpdate() >= 3000)
	{
		this->UpdateItemExp();
		m_pkChar->SetSupportInt(this->GetIntSkill());
		this->SetLastExpUpdate(get_dword_time());
	}
	
	this->UseSkill();
	this->RefreshCostume();
	
	if (this->IsSummoned() && HasOption(ESupportOption_Followable))
		bResult = bResult && this->_UpdateFollowAI();

	return bResult;
}

void CSupportActor::UseSkill()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (!pSummonItem)
		return;
	if (pSummonItem->GetVnum() == 61019 || pSummonItem->GetVnum() == 61021 || pSummonItem->GetVnum() == 61022)
	{
		if(m_pkChar->GetOwner() && (m_pkChar->GetOwner()->IsAffectFlag(AFF_HOSIN) == false && get_dword_time() - this->GetLastSkillTime() >= 3000))
		{
			if (m_pkChar->GetLevel() >= 1 && m_pkChar->GetLevel() <= 3)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),1);
			if (m_pkChar->GetLevel() >= 4 && m_pkChar->GetLevel() <= 6)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),2);
			if (m_pkChar->GetLevel() >= 7 && m_pkChar->GetLevel() <= 9)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),3);
			if (m_pkChar->GetLevel() >= 10 && m_pkChar->GetLevel() <= 12)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),4);
			if (m_pkChar->GetLevel() >= 13 && m_pkChar->GetLevel() <= 15)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),5);
			if (m_pkChar->GetLevel() >= 16 && m_pkChar->GetLevel() <= 18)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),6);
			if (m_pkChar->GetLevel() >= 19 && m_pkChar->GetLevel() <= 21)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),7);
			if (m_pkChar->GetLevel() >= 22 && m_pkChar->GetLevel() <= 24)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),8);
			if (m_pkChar->GetLevel() >= 25 && m_pkChar->GetLevel() <= 27)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),9);
			if (m_pkChar->GetLevel() >= 28 && m_pkChar->GetLevel() <= 30)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),10);
			if (m_pkChar->GetLevel() >= 31 && m_pkChar->GetLevel() <= 33)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),11);
			if (m_pkChar->GetLevel() >= 34 && m_pkChar->GetLevel() <= 36)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),12);
			if (m_pkChar->GetLevel() >= 37 && m_pkChar->GetLevel() <= 39)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),13);
			if (m_pkChar->GetLevel() >= 40 && m_pkChar->GetLevel() <= 42)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),14);
			if (m_pkChar->GetLevel() >= 43 && m_pkChar->GetLevel() <= 45)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),15);
			if (m_pkChar->GetLevel() >= 46 && m_pkChar->GetLevel() <= 48)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),16);
			if (m_pkChar->GetLevel() >= 49 && m_pkChar->GetLevel() <= 51)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),17);
			if (m_pkChar->GetLevel() >= 52 && m_pkChar->GetLevel() <= 54)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),18);
			if (m_pkChar->GetLevel() >= 55 && m_pkChar->GetLevel() <= 57)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),19);
			if (m_pkChar->GetLevel() >= 58 && m_pkChar->GetLevel() <= 60)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),20);
			if (m_pkChar->GetLevel() >= 61 && m_pkChar->GetLevel() <= 63)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),21);
			if (m_pkChar->GetLevel() >= 64 && m_pkChar->GetLevel() <= 66)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),22);
			if (m_pkChar->GetLevel() >= 67 && m_pkChar->GetLevel() <= 69)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),23);
			if (m_pkChar->GetLevel() >= 70 && m_pkChar->GetLevel() <= 72)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),24);
			if (m_pkChar->GetLevel() >= 73 && m_pkChar->GetLevel() <= 75)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),25);
			if (m_pkChar->GetLevel() >= 76 && m_pkChar->GetLevel() <= 78)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),26);
			if (m_pkChar->GetLevel() >= 79 && m_pkChar->GetLevel() <= 81)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),27);
			if (m_pkChar->GetLevel() >= 82 && m_pkChar->GetLevel() <= 84)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),28);
			if (m_pkChar->GetLevel() >= 85 && m_pkChar->GetLevel() <= 87)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),29);
			if (m_pkChar->GetLevel() >= 88 && m_pkChar->GetLevel() <= 90)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),30);
			if (m_pkChar->GetLevel() >= 91 && m_pkChar->GetLevel() <= 93)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),31);
			if (m_pkChar->GetLevel() >= 94 && m_pkChar->GetLevel() <= 96)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),32);
			if (m_pkChar->GetLevel() >= 97 && m_pkChar->GetLevel() <= 99)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),33);
			if (m_pkChar->GetLevel() >= 100 && m_pkChar->GetLevel() <= 102)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),34);
			if (m_pkChar->GetLevel() >= 103 && m_pkChar->GetLevel() <= 105)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),35);
			if (m_pkChar->GetLevel() >= 106 && m_pkChar->GetLevel() <= 108)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),36);
			if (m_pkChar->GetLevel() >= 109 && m_pkChar->GetLevel() <= 110)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),37);
			if (m_pkChar->GetLevel() >= 111 && m_pkChar->GetLevel() <= 113)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),38);
			if (m_pkChar->GetLevel() >= 114 && m_pkChar->GetLevel() <= 116)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),39);
			if (m_pkChar->GetLevel() >= 117 && m_pkChar->GetLevel() <= 118)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),40);
			if (m_pkChar->GetLevel() >= 119 && m_pkChar->GetLevel() <= 120)
				m_pkChar->ComputeSkill(610,m_pkChar->GetOwner(),41);
			this->SetLastSkillTime(get_dword_time());
			m_pkChar->SendSupportSkillPacket(SKILL_HOSIN);
		}
		else if((m_pkChar->GetOwner() && m_pkChar->GetOwner()->IsAffectFlag(AFF_BOHO)) == false && get_dword_time() - this->GetLastSkillTime() >= 3000)
		{
			if (m_pkChar->GetLevel() >= 1 && m_pkChar->GetLevel() <= 3)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),1);
			if (m_pkChar->GetLevel() >= 4 && m_pkChar->GetLevel() <= 6)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),2);
			if (m_pkChar->GetLevel() >= 7 && m_pkChar->GetLevel() <= 9)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),3);
			if (m_pkChar->GetLevel() >= 10 && m_pkChar->GetLevel() <= 12)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),4);
			if (m_pkChar->GetLevel() >= 13 && m_pkChar->GetLevel() <= 15)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),5);
			if (m_pkChar->GetLevel() >= 16 && m_pkChar->GetLevel() <= 18)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),6);
			if (m_pkChar->GetLevel() >= 19 && m_pkChar->GetLevel() <= 21)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),7);
			if (m_pkChar->GetLevel() >= 22 && m_pkChar->GetLevel() <= 24)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),8);
			if (m_pkChar->GetLevel() >= 25 && m_pkChar->GetLevel() <= 27)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),9);
			if (m_pkChar->GetLevel() >= 28 && m_pkChar->GetLevel() <= 30)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),10);
			if (m_pkChar->GetLevel() >= 31 && m_pkChar->GetLevel() <= 33)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),11);
			if (m_pkChar->GetLevel() >= 34 && m_pkChar->GetLevel() <= 36)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),12);
			if (m_pkChar->GetLevel() >= 37 && m_pkChar->GetLevel() <= 39)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),13);
			if (m_pkChar->GetLevel() >= 40 && m_pkChar->GetLevel() <= 42)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),14);
			if (m_pkChar->GetLevel() >= 43 && m_pkChar->GetLevel() <= 45)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),15);
			if (m_pkChar->GetLevel() >= 46 && m_pkChar->GetLevel() <= 48)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),16);
			if (m_pkChar->GetLevel() >= 49 && m_pkChar->GetLevel() <= 51)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),17);
			if (m_pkChar->GetLevel() >= 52 && m_pkChar->GetLevel() <= 54)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),18);
			if (m_pkChar->GetLevel() >= 55 && m_pkChar->GetLevel() <= 57)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),19);
			if (m_pkChar->GetLevel() >= 58 && m_pkChar->GetLevel() <= 60)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),20);
			if (m_pkChar->GetLevel() >= 61 && m_pkChar->GetLevel() <= 63)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),21);
			if (m_pkChar->GetLevel() >= 64 && m_pkChar->GetLevel() <= 66)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),22);
			if (m_pkChar->GetLevel() >= 67 && m_pkChar->GetLevel() <= 69)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),23);
			if (m_pkChar->GetLevel() >= 70 && m_pkChar->GetLevel() <= 72)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),24);
			if (m_pkChar->GetLevel() >= 73 && m_pkChar->GetLevel() <= 75)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),25);
			if (m_pkChar->GetLevel() >= 76 && m_pkChar->GetLevel() <= 78)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),26);
			if (m_pkChar->GetLevel() >= 79 && m_pkChar->GetLevel() <= 81)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),27);
			if (m_pkChar->GetLevel() >= 82 && m_pkChar->GetLevel() <= 84)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),28);
			if (m_pkChar->GetLevel() >= 85 && m_pkChar->GetLevel() <= 87)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),29);
			if (m_pkChar->GetLevel() >= 88 && m_pkChar->GetLevel() <= 90)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),30);
			if (m_pkChar->GetLevel() >= 91 && m_pkChar->GetLevel() <= 93)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),31);
			if (m_pkChar->GetLevel() >= 94 && m_pkChar->GetLevel() <= 96)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),32);
			if (m_pkChar->GetLevel() >= 97 && m_pkChar->GetLevel() <= 99)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),33);
			if (m_pkChar->GetLevel() >= 100 && m_pkChar->GetLevel() <= 102)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),34);
			if (m_pkChar->GetLevel() >= 103 && m_pkChar->GetLevel() <= 105)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),35);
			if (m_pkChar->GetLevel() >= 106 && m_pkChar->GetLevel() <= 108)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),36);
			if (m_pkChar->GetLevel() >= 109 && m_pkChar->GetLevel() <= 110)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),37);
			if (m_pkChar->GetLevel() >= 111 && m_pkChar->GetLevel() <= 113)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),38);
			if (m_pkChar->GetLevel() >= 114 && m_pkChar->GetLevel() <= 116)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),39);
			if (m_pkChar->GetLevel() >= 117 && m_pkChar->GetLevel() <= 118)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),40);
			if (m_pkChar->GetLevel() >= 119 && m_pkChar->GetLevel() <= 120)
				m_pkChar->ComputeSkill(611,m_pkChar->GetOwner(),41);
			this->SetLastSkillTime(get_dword_time());
			m_pkChar->SendSupportSkillPacket(SKILL_REFLECT);
		}
		else if((m_pkChar->GetOwner() && m_pkChar->GetOwner()->IsAffectFlag(AFF_GICHEON)) == false && get_dword_time() - this->GetLastSkillTime() >= 3000)
		{
			if (m_pkChar->GetLevel() >= 1 && m_pkChar->GetLevel() <= 3)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),1);
			if (m_pkChar->GetLevel() >= 4 && m_pkChar->GetLevel() <= 6)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),2);
			if (m_pkChar->GetLevel() >= 7 && m_pkChar->GetLevel() <= 9)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),3);
			if (m_pkChar->GetLevel() >= 10 && m_pkChar->GetLevel() <= 12)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),4);
			if (m_pkChar->GetLevel() >= 13 && m_pkChar->GetLevel() <= 15)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),5);
			if (m_pkChar->GetLevel() >= 16 && m_pkChar->GetLevel() <= 18)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),6);
			if (m_pkChar->GetLevel() >= 19 && m_pkChar->GetLevel() <= 21)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),7);
			if (m_pkChar->GetLevel() >= 22 && m_pkChar->GetLevel() <= 24)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),8);
			if (m_pkChar->GetLevel() >= 25 && m_pkChar->GetLevel() <= 27)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),9);
			if (m_pkChar->GetLevel() >= 28 && m_pkChar->GetLevel() <= 30)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),10);
			if (m_pkChar->GetLevel() >= 31 && m_pkChar->GetLevel() <= 33)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),11);
			if (m_pkChar->GetLevel() >= 34 && m_pkChar->GetLevel() <= 36)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),12);
			if (m_pkChar->GetLevel() >= 37 && m_pkChar->GetLevel() <= 39)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),13);
			if (m_pkChar->GetLevel() >= 40 && m_pkChar->GetLevel() <= 42)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),14);
			if (m_pkChar->GetLevel() >= 43 && m_pkChar->GetLevel() <= 45)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),15);
			if (m_pkChar->GetLevel() >= 46 && m_pkChar->GetLevel() <= 48)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),16);
			if (m_pkChar->GetLevel() >= 49 && m_pkChar->GetLevel() <= 51)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),17);
			if (m_pkChar->GetLevel() >= 52 && m_pkChar->GetLevel() <= 54)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),18);
			if (m_pkChar->GetLevel() >= 55 && m_pkChar->GetLevel() <= 57)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),19);
			if (m_pkChar->GetLevel() >= 58 && m_pkChar->GetLevel() <= 60)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),20);
			if (m_pkChar->GetLevel() >= 61 && m_pkChar->GetLevel() <= 63)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),21);
			if (m_pkChar->GetLevel() >= 64 && m_pkChar->GetLevel() <= 66)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),22);
			if (m_pkChar->GetLevel() >= 67 && m_pkChar->GetLevel() <= 69)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),23);
			if (m_pkChar->GetLevel() >= 70 && m_pkChar->GetLevel() <= 72)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),24);
			if (m_pkChar->GetLevel() >= 73 && m_pkChar->GetLevel() <= 75)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),25);
			if (m_pkChar->GetLevel() >= 76 && m_pkChar->GetLevel() <= 78)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),26);
			if (m_pkChar->GetLevel() >= 79 && m_pkChar->GetLevel() <= 81)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),27);
			if (m_pkChar->GetLevel() >= 82 && m_pkChar->GetLevel() <= 84)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),28);
			if (m_pkChar->GetLevel() >= 85 && m_pkChar->GetLevel() <= 87)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),29);
			if (m_pkChar->GetLevel() >= 88 && m_pkChar->GetLevel() <= 90)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),30);
			if (m_pkChar->GetLevel() >= 91 && m_pkChar->GetLevel() <= 93)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),31);
			if (m_pkChar->GetLevel() >= 94 && m_pkChar->GetLevel() <= 96)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),32);
			if (m_pkChar->GetLevel() >= 97 && m_pkChar->GetLevel() <= 99)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),33);
			if (m_pkChar->GetLevel() >= 100 && m_pkChar->GetLevel() <= 102)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),34);
			if (m_pkChar->GetLevel() >= 103 && m_pkChar->GetLevel() <= 105)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),35);
			if (m_pkChar->GetLevel() >= 106 && m_pkChar->GetLevel() <= 108)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),36);
			if (m_pkChar->GetLevel() >= 109 && m_pkChar->GetLevel() <= 110)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),37);
			if (m_pkChar->GetLevel() >= 111 && m_pkChar->GetLevel() <= 113)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),38);
			if (m_pkChar->GetLevel() >= 114 && m_pkChar->GetLevel() <= 116)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),39);
			if (m_pkChar->GetLevel() >= 117 && m_pkChar->GetLevel() <= 118)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),40);
			if (m_pkChar->GetLevel() >= 119 && m_pkChar->GetLevel() <= 120)
				m_pkChar->ComputeSkill(612,m_pkChar->GetOwner(),41);
			this->SetLastSkillTime(get_dword_time());
			m_pkChar->SendSupportSkillPacket(SKILL_GICHEON);
		}
	}
	else
	{
		if ((m_pkChar->GetOwner() && m_pkChar->GetOwner()->IsAffectFlag(AFF_KWAESOK)) == false && get_dword_time() - this->GetLastSkillTime() >= 3000)
		{
			if (m_pkChar->GetLevel() >= 1 && m_pkChar->GetLevel() <= 3)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),1);
			if (m_pkChar->GetLevel() >= 4 && m_pkChar->GetLevel() <= 6)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),2);
			if (m_pkChar->GetLevel() >= 7 && m_pkChar->GetLevel() <= 9)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),3);
			if (m_pkChar->GetLevel() >= 10 && m_pkChar->GetLevel() <= 12)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),4);
			if (m_pkChar->GetLevel() >= 13 && m_pkChar->GetLevel() <= 15)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),5);
			if (m_pkChar->GetLevel() >= 16 && m_pkChar->GetLevel() <= 18)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),6);
			if (m_pkChar->GetLevel() >= 19 && m_pkChar->GetLevel() <= 21)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),7);
			if (m_pkChar->GetLevel() >= 22 && m_pkChar->GetLevel() <= 24)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),8);
			if (m_pkChar->GetLevel() >= 25 && m_pkChar->GetLevel() <= 27)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),9);
			if (m_pkChar->GetLevel() >= 28 && m_pkChar->GetLevel() <= 30)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),10);
			if (m_pkChar->GetLevel() >= 31 && m_pkChar->GetLevel() <= 33)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),11);
			if (m_pkChar->GetLevel() >= 34 && m_pkChar->GetLevel() <= 36)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),12);
			if (m_pkChar->GetLevel() >= 37 && m_pkChar->GetLevel() <= 39)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),13);
			if (m_pkChar->GetLevel() >= 40 && m_pkChar->GetLevel() <= 42)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),14);
			if (m_pkChar->GetLevel() >= 43 && m_pkChar->GetLevel() <= 45)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),15);
			if (m_pkChar->GetLevel() >= 46 && m_pkChar->GetLevel() <= 48)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),16);
			if (m_pkChar->GetLevel() >= 49 && m_pkChar->GetLevel() <= 51)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),17);
			if (m_pkChar->GetLevel() >= 52 && m_pkChar->GetLevel() <= 54)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),18);
			if (m_pkChar->GetLevel() >= 55 && m_pkChar->GetLevel() <= 57)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),19);
			if (m_pkChar->GetLevel() >= 58 && m_pkChar->GetLevel() <= 60)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),20);
			if (m_pkChar->GetLevel() >= 61 && m_pkChar->GetLevel() <= 63)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),21);
			if (m_pkChar->GetLevel() >= 64 && m_pkChar->GetLevel() <= 66)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),22);
			if (m_pkChar->GetLevel() >= 67 && m_pkChar->GetLevel() <= 69)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),23);
			if (m_pkChar->GetLevel() >= 70 && m_pkChar->GetLevel() <= 72)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),24);
			if (m_pkChar->GetLevel() >= 73 && m_pkChar->GetLevel() <= 75)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),25);
			if (m_pkChar->GetLevel() >= 76 && m_pkChar->GetLevel() <= 78)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),26);
			if (m_pkChar->GetLevel() >= 79 && m_pkChar->GetLevel() <= 81)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),27);
			if (m_pkChar->GetLevel() >= 82 && m_pkChar->GetLevel() <= 84)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),28);
			if (m_pkChar->GetLevel() >= 85 && m_pkChar->GetLevel() <= 87)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),29);
			if (m_pkChar->GetLevel() >= 88 && m_pkChar->GetLevel() <= 90)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),30);
			if (m_pkChar->GetLevel() >= 91 && m_pkChar->GetLevel() <= 93)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),31);
			if (m_pkChar->GetLevel() >= 94 && m_pkChar->GetLevel() <= 96)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),32);
			if (m_pkChar->GetLevel() >= 97 && m_pkChar->GetLevel() <= 99)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),33);
			if (m_pkChar->GetLevel() >= 100 && m_pkChar->GetLevel() <= 102)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),34);
			if (m_pkChar->GetLevel() >= 103 && m_pkChar->GetLevel() <= 105)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),35);
			if (m_pkChar->GetLevel() >= 106 && m_pkChar->GetLevel() <= 108)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),36);
			if (m_pkChar->GetLevel() >= 109 && m_pkChar->GetLevel() <= 110)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),37);
			if (m_pkChar->GetLevel() >= 111 && m_pkChar->GetLevel() <= 113)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),38);
			if (m_pkChar->GetLevel() >= 114 && m_pkChar->GetLevel() <= 116)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),39);
			if (m_pkChar->GetLevel() >= 117 && m_pkChar->GetLevel() <= 118)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),40);
			if (m_pkChar->GetLevel() >= 119 && m_pkChar->GetLevel() <= 120)
				m_pkChar->ComputeSkill(613,m_pkChar->GetOwner(),41);
			this->SetLastSkillTime(get_dword_time());
			m_pkChar->SendSupportSkillPacket(SKILL_KWAESOK);
		}
		else if ((m_pkChar->GetOwner() && m_pkChar->GetOwner()->IsAffectFlag(AFF_JEUNGRYEOK)) == false && get_dword_time() - this->GetLastSkillTime() >= 3000)
		{
			if (m_pkChar->GetLevel() >= 1 && m_pkChar->GetLevel() <= 3)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),1);
			if (m_pkChar->GetLevel() >= 4 && m_pkChar->GetLevel() <= 6)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),2);
			if (m_pkChar->GetLevel() >= 7 && m_pkChar->GetLevel() <= 9)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),3);
			if (m_pkChar->GetLevel() >= 10 && m_pkChar->GetLevel() <= 12)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),4);
			if (m_pkChar->GetLevel() >= 13 && m_pkChar->GetLevel() <= 15)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),5);
			if (m_pkChar->GetLevel() >= 16 && m_pkChar->GetLevel() <= 18)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),6);
			if (m_pkChar->GetLevel() >= 19 && m_pkChar->GetLevel() <= 21)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),7);
			if (m_pkChar->GetLevel() >= 22 && m_pkChar->GetLevel() <= 24)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),8);
			if (m_pkChar->GetLevel() >= 25 && m_pkChar->GetLevel() <= 27)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),9);
			if (m_pkChar->GetLevel() >= 28 && m_pkChar->GetLevel() <= 30)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),10);
			if (m_pkChar->GetLevel() >= 31 && m_pkChar->GetLevel() <= 33)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),11);
			if (m_pkChar->GetLevel() >= 34 && m_pkChar->GetLevel() <= 36)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),12);
			if (m_pkChar->GetLevel() >= 37 && m_pkChar->GetLevel() <= 39)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),13);
			if (m_pkChar->GetLevel() >= 40 && m_pkChar->GetLevel() <= 42)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),14);
			if (m_pkChar->GetLevel() >= 43 && m_pkChar->GetLevel() <= 45)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),15);
			if (m_pkChar->GetLevel() >= 46 && m_pkChar->GetLevel() <= 48)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),16);
			if (m_pkChar->GetLevel() >= 49 && m_pkChar->GetLevel() <= 51)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),17);
			if (m_pkChar->GetLevel() >= 52 && m_pkChar->GetLevel() <= 54)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),18);
			if (m_pkChar->GetLevel() >= 55 && m_pkChar->GetLevel() <= 57)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),19);
			if (m_pkChar->GetLevel() >= 58 && m_pkChar->GetLevel() <= 60)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),20);
			if (m_pkChar->GetLevel() >= 61 && m_pkChar->GetLevel() <= 63)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),21);
			if (m_pkChar->GetLevel() >= 64 && m_pkChar->GetLevel() <= 66)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),22);
			if (m_pkChar->GetLevel() >= 67 && m_pkChar->GetLevel() <= 69)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),23);
			if (m_pkChar->GetLevel() >= 70 && m_pkChar->GetLevel() <= 72)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),24);
			if (m_pkChar->GetLevel() >= 73 && m_pkChar->GetLevel() <= 75)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),25);
			if (m_pkChar->GetLevel() >= 76 && m_pkChar->GetLevel() <= 78)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),26);
			if (m_pkChar->GetLevel() >= 79 && m_pkChar->GetLevel() <= 81)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),27);
			if (m_pkChar->GetLevel() >= 82 && m_pkChar->GetLevel() <= 84)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),28);
			if (m_pkChar->GetLevel() >= 85 && m_pkChar->GetLevel() <= 87)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),29);
			if (m_pkChar->GetLevel() >= 88 && m_pkChar->GetLevel() <= 90)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),30);
			if (m_pkChar->GetLevel() >= 91 && m_pkChar->GetLevel() <= 93)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),31);
			if (m_pkChar->GetLevel() >= 94 && m_pkChar->GetLevel() <= 96)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),32);
			if (m_pkChar->GetLevel() >= 97 && m_pkChar->GetLevel() <= 99)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),33);
			if (m_pkChar->GetLevel() >= 100 && m_pkChar->GetLevel() <= 102)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),34);
			if (m_pkChar->GetLevel() >= 103 && m_pkChar->GetLevel() <= 105)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),35);
			if (m_pkChar->GetLevel() >= 106 && m_pkChar->GetLevel() <= 108)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),36);
			if (m_pkChar->GetLevel() >= 109 && m_pkChar->GetLevel() <= 110)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),37);
			if (m_pkChar->GetLevel() >= 111 && m_pkChar->GetLevel() <= 113)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),38);
			if (m_pkChar->GetLevel() >= 114 && m_pkChar->GetLevel() <= 116)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),39);
			if (m_pkChar->GetLevel() >= 117 && m_pkChar->GetLevel() <= 118)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),40);
			if (m_pkChar->GetLevel() >= 119 && m_pkChar->GetLevel() <= 120)
				m_pkChar->ComputeSkill(614,m_pkChar->GetOwner(),41);
			this->SetLastSkillTime(get_dword_time());
			m_pkChar->SendSupportSkillPacket(SKILL_JEUNGRYEOK);
		}
	}
}

void CSupportActor::UpdateItemExp()
{
	LPITEM pSummonItem = ITEM_MANAGER::instance().FindByVID(this->GetSummonItemVID());
	if (pSummonItem != nullptr)
	{
		pSummonItem->SetSocket(1, m_dwExp);
		pSummonItem->SetSocket(2, m_dwlevel);
		pSummonItem->SetSocket(3, m_dwIntSkill);
	}
}

bool CSupportActor::Follow(float fMinDistance)
{
	if( !m_pkOwner || !m_pkChar) 
		return false;

	float fOwnerX = m_pkOwner->GetX();
	float fOwnerY = m_pkOwner->GetY();

	float fSupportX = m_pkChar->GetX();
	float fSupportY = m_pkChar->GetY();

	float fDist = DISTANCE_SQRT(fOwnerX - fSupportX, fOwnerY - fSupportY);
	if (fDist <= fMinDistance)
		return false;

	m_pkChar->SetRotationToXY(fOwnerX, fOwnerY);

	float fx, fy;

	float fDistToGo = fDist - fMinDistance;
	GetDeltaByDegree(m_pkChar->GetRotation(), fDistToGo, &fx, &fy);
	
	if (!m_pkChar->Goto((int32_t)(fSupportX+fx+0.5f), (int32_t)(fSupportY+fy+0.5f)) )
		return false;

	m_pkChar->SendMovePacket(FUNC_WAIT, 0, 0, 0, 0, 0);

	return true;
}

void CSupportActor::SetSummonItem (LPITEM pItem)
{
	if (nullptr == pItem)
	{
		m_dwSummonItemVID = 0;
		m_dwSummonItemVnum = 0;
		return;
	}

	m_dwSummonItemVID = pItem->GetVID();
	m_dwSummonItemVnum = pItem->GetVnum();
}

void CSupportActor::GiveBuff()
{
	if (m_dwVnum == 34077)
	{
		if (nullptr == m_pkOwner->GetDungeon())
		{
			return;
		}
	}
	LPITEM item = ITEM_MANAGER::instance().FindByVID(m_dwSummonItemVID);
	if (nullptr != item)
		item->ModifyPoints(true);
	return ;
}

void CSupportActor::ClearBuff()
{
	if (nullptr == m_pkOwner)
		return ;
	TItemTable* item_proto = ITEM_MANAGER::instance().GetTable(m_dwSummonItemVnum);
	if (nullptr == item_proto)
		return;
	for (int32_t i = 0; i < ITEM_APPLY_MAX_NUM; i++)
	{
		if (item_proto->aApplies[i].bType == APPLY_NONE)
			continue;
		m_pkOwner->ApplyPoint(item_proto->aApplies[i].bType, -item_proto->aApplies[i].lValue);
	}

	return ;
}

CSupport::CSupport(LPCHARACTER owner)
{

	m_pkOwner = owner;
	m_dwUpdatePeriod = 400;

	m_dwLastUpdateTime = 0;
}

CSupport::~CSupport()
{
	Destroy();
}

void CSupport::Destroy()
{
	for (TSupportActorMap::iterator iter = m_SupportActorMap.begin(); iter != m_SupportActorMap.end(); ++iter)
	{
		CSupportActor* SupportActor = iter->second;

		if (0 != SupportActor)
		{
			delete SupportActor;
		}
	}
	event_cancel(&m_pkSupportUpdateEvent);
	m_SupportActorMap.clear();
}

bool CSupport::Update(uint32_t deltaTime)
{
	bool bResult = true;

	uint32_t currentTime = get_dword_time();	
	
	if (m_dwUpdatePeriod > currentTime - m_dwLastUpdateTime)
		return true;
	
	std::vector <CSupportActor*> v_garbageActor;

	for (TSupportActorMap::iterator iter = m_SupportActorMap.begin(); iter != m_SupportActorMap.end(); ++iter)
	{
		CSupportActor* SupportActor = iter->second;

		if (0 != SupportActor && SupportActor->IsSummoned())
		{
			LPCHARACTER pSupport = SupportActor->GetCharacter();
			
			if (nullptr == CHARACTER_MANAGER::instance().Find(pSupport->GetVID()))
			{
				v_garbageActor.push_back(SupportActor);
			}
			else
			{
				bResult = bResult && SupportActor->Update(deltaTime);
			}
		}
	}
	for (std::vector<CSupportActor*>::iterator it = v_garbageActor.begin(); it != v_garbageActor.end(); it++)
		DeleteSupport(*it);

	m_dwLastUpdateTime = currentTime;

	return bResult;
}

void CSupport::DeleteSupport(uint32_t mobVnum)
{
	TSupportActorMap::iterator iter = m_SupportActorMap.find(mobVnum);

	if (m_SupportActorMap.end() == iter)
	{
		sys_err("[CSupport::DeleteSupport] Can't find shaman on my list (VNUM: %d)", mobVnum);
		return;
	}

	CSupportActor* SupportActor = iter->second;

	if (0 == SupportActor)
		sys_err("[CSupport::DeleteSupport] Null Pointer (SupportActor)");
	else
		delete SupportActor;

	m_SupportActorMap.erase(iter);	
}

void CSupport::DeleteSupport(CSupportActor* SupportActor)
{
	for (TSupportActorMap::iterator iter = m_SupportActorMap.begin(); iter != m_SupportActorMap.end(); ++iter)
	{
		if (iter->second == SupportActor)
		{
			delete SupportActor;
			m_SupportActorMap.erase(iter);

			return;
		}
	}

	sys_err("[CSupport::DeleteSupport] Can't find SupportActor(0x%x) on my list(size: %d) ", SupportActor, m_SupportActorMap.size());
}

void CSupport::Unsummon(uint32_t vnum, bool bDeleteFromList)
{
	CSupportActor* actor = this->GetByVnum(vnum);

	if (0 == actor)
	{
		sys_err("[CSupport::GetByVnum(%d)] Null Pointer (SupportActor)", vnum);
		return;
	}
	actor->Unsummon();

	if (true == bDeleteFromList)
		this->DeleteSupport(actor);

	bool bActive = false;
	for (TSupportActorMap::iterator it = m_SupportActorMap.begin(); it != m_SupportActorMap.end(); it++)
	{
		bActive |= it->second->IsSummoned();
	}
	if (false == bActive)
	{
		event_cancel(&m_pkSupportUpdateEvent);
		m_pkSupportUpdateEvent = nullptr;
	}
}

void CSupport::SetExp(int32_t iExp)
{
	for (TSupportActorMap::iterator iter = m_SupportActorMap.begin(); iter != m_SupportActorMap.end(); ++iter)
	{
		CSupportActor* SupportActor = iter->second;
		if (SupportActor != 0)
		{
			if (SupportActor->IsSummoned()) {
				SupportActor->SetExp(iExp);
				break;
			}
		}
	}
}

int32_t CSupport::GetLevel()
{
	for (TSupportActorMap::iterator iter = m_SupportActorMap.begin(); iter != m_SupportActorMap.end(); ++iter)
	{
		CSupportActor* SupportActor = iter->second;
		if (SupportActor != 0)
		{
			if (SupportActor->IsSummoned()) {
				return SupportActor->GetLevel();
			}
		}
	}
	return -1;
}

uint32_t CSupport::GetExp()
{
	for (TSupportActorMap::iterator iter = m_SupportActorMap.begin(); iter != m_SupportActorMap.end(); ++iter)
	{
		CSupportActor* SupportActor = iter->second;
		if (SupportActor != 0)
		{
			if (SupportActor->IsSummoned()) {
				return SupportActor->GetExp();
			}
		}
	}
	return 0;
}
bool CSupport::IsActiveSupport()
{
	bool state = false;
	for (TSupportActorMap::iterator iter = m_SupportActorMap.begin(); iter != m_SupportActorMap.end(); ++iter)
	{
		CSupportActor* SupportActor = iter->second;
		if (SupportActor != 0)
		{
			if (SupportActor->IsSummoned()) {
				state = true;
				break;
			}			
		}
	}
	return state;

}

CSupportActor* CSupport::GetActiveSupport()
{
	for (TSupportActorMap::iterator iter = m_SupportActorMap.begin(); iter != m_SupportActorMap.end(); ++iter)
	{
		CSupportActor* SupportActor = iter->second;
		if (SupportActor != 0 && SupportActor->IsSummoned())
			return SupportActor;
	}

	return nullptr;
}

CSupportActor* CSupport::Summon(uint32_t mobVnum, LPITEM pSummonItem, const char* supportName, bool bSpawnFar, uint32_t options)
{
	CSupportActor* SupportActor = this->GetByVnum(mobVnum);

	if (0 == SupportActor)
	{
		SupportActor = M2_NEW CSupportActor(m_pkOwner, mobVnum, options);
		m_SupportActorMap.insert(std::make_pair(mobVnum, SupportActor));
	}

	SupportActor->Summon(supportName, pSummonItem, bSpawnFar);

	if (nullptr == m_pkSupportUpdateEvent)
	{
		Support_event_info* info = AllocEventInfo<Support_event_info>();

		info->pSupport = this;

		m_pkSupportUpdateEvent = event_create(Support_update_event, info, PASSES_PER_SEC(1) / 4);
	}

	return SupportActor;
}

CSupportActor* CSupport::GetByVID(uint32_t vid) const
{
	CSupportActor* SupportActor = 0;

	bool bFound = false;

	for (TSupportActorMap::const_iterator iter = m_SupportActorMap.begin(); iter != m_SupportActorMap.end(); ++iter)
	{
		SupportActor = iter->second;

		if (0 == SupportActor)
		{
			sys_err("[CSupport::GetByVID(%d)] Null Pointer (SupportActor)", vid);
			continue;
		}

		bFound = SupportActor->GetVID() == vid;

		if (true == bFound)
			break;
	}

	return bFound ? SupportActor : 0;
}

CSupportActor* CSupport::GetByVnum(uint32_t vnum) const
{
	CSupportActor* SupportActor = 0;

	TSupportActorMap::const_iterator iter = m_SupportActorMap.find(vnum);

	if (m_SupportActorMap.end() != iter)
		SupportActor = iter->second;

	return SupportActor;
}

size_t CSupport::CountSummoned() const
{
	size_t count = 0;

	for (TSupportActorMap::const_iterator iter = m_SupportActorMap.begin(); iter != m_SupportActorMap.end(); ++iter)
	{
		CSupportActor* SupportActor = iter->second;

		if (0 != SupportActor)
		{
			if (SupportActor->IsSummoned())
				++count;
		}
	}

	return count;
}

void CSupport::RefreshBuff()
{
	for (TSupportActorMap::const_iterator iter = m_SupportActorMap.begin(); iter != m_SupportActorMap.end(); ++iter)
	{
		CSupportActor* SupportActor = iter->second;

		if (0 != SupportActor)
		{
			if (SupportActor->IsSummoned())
			{
				SupportActor->GiveBuff();
			}
		}
	}
}
