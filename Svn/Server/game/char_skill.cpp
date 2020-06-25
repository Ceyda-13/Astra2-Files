#include "stdafx.h"
#include <sstream>

#include "utils.h"
#include "config.h"
#include "vector.h"
#include "char.h"
#include "char_manager.h"
#include "battle.h"
#include "desc.h"
#include "desc_manager.h"
#include "packet.h"
#include "affect.h"
#include "item.h"
#include "sectree_manager.h"
#include "mob_manager.h"
#include "start_position.h"
#include "party.h"
#include "buffer_manager.h"
#include "guild.h"
#include "log.h"
#include "unique_item.h"
#include "questmanager.h"


static const uint32_t s_adwSubSkillVnums[] =
{
	SKILL_LEADERSHIP,
	SKILL_COMBO,
	SKILL_MINING,
	SKILL_LANGUAGE1,
	SKILL_LANGUAGE2,
	SKILL_LANGUAGE3,
	SKILL_POLYMORPH,
	SKILL_HORSE,
	SKILL_HORSE_SUMMON,
	SKILL_HORSE_WILDATTACK,
	SKILL_HORSE_CHARGE,
	SKILL_HORSE_ESCAPE,
	SKILL_HORSE_WILDATTACK_RANGE,
	SKILL_ADD_HP,
	SKILL_RESIST_PENETRATE
};

time_t CHARACTER::GetSkillNextReadTime(uint32_t dwVnum) const
{
	if (dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("vnum overflow (vnum: %u)", dwVnum);
		return 0;
	}

	return m_pSkillLevels ? m_pSkillLevels[dwVnum].tNextRead : 0;
}

void CHARACTER::SetSkillNextReadTime(uint32_t dwVnum, time_t time)
{
	if (m_pSkillLevels && dwVnum < SKILL_MAX_NUM)
		m_pSkillLevels[dwVnum].tNextRead = time;
}

bool TSkillUseInfo::HitOnce(uint32_t dwVnum)
{
	if (!bUsed)
		return false;

	sys_log(1, "__HitOnce NextUse %u current %u count %d scount %d", dwNextSkillUsableTime, get_dword_time(), iHitCount, iSplashCount);

	if (dwNextSkillUsableTime && dwNextSkillUsableTime<get_dword_time() && dwVnum != SKILL_MUYEONG && dwVnum != SKILL_HORSE_WILDATTACK)
	{
		sys_log(1, "__HitOnce can't hit");

		return false;
	}

	if (iHitCount == -1)
	{
		sys_log(1, "__HitOnce OK %d %d %d", dwNextSkillUsableTime, get_dword_time(), iHitCount);
		return true;
	}

	if (iHitCount)
	{
		sys_log(1, "__HitOnce OK %d %d %d", dwNextSkillUsableTime, get_dword_time(), iHitCount);
		iHitCount--;
		return true;
	}
	return false;
}



bool TSkillUseInfo::UseSkill(bool isGrandMaster, uint32_t vid, uint32_t dwCooltime, int32_t splashcount, int32_t hitcount, int32_t range)
{
	this->isGrandMaster = isGrandMaster;
	uint32_t dwCur = get_dword_time();

	if (bUsed && dwNextSkillUsableTime > dwCur)
	{
		sys_log(0, "cooltime is not over delta %u", dwNextSkillUsableTime - dwCur);
		iHitCount = 0;
		return false;
	}

	bUsed = true;

	if (dwCooltime)
		dwNextSkillUsableTime = dwCur + dwCooltime;
	else
		dwNextSkillUsableTime = 0;

	iRange = range;
	iMaxHitCount = iHitCount = hitcount;

	if (test_server)
		sys_log(0, "UseSkill NextUse %u  current %u cooltime %d hitcount %d/%d", dwNextSkillUsableTime, dwCur, dwCooltime, iHitCount, iMaxHitCount);

	dwVID = vid;
	iSplashCount = splashcount;
	return true;
}

int32_t CHARACTER::GetChainLightningMaxCount() const
{
	return aiChainLightningCountBySkillLevel[MIN(SKILL_MAX_LEVEL, GetSkillLevel(SKILL_CHAIN))];
}

void CHARACTER::SetAffectedEunhyung()
{
	m_dwAffectedEunhyungLevel = GetSkillPower(SKILL_EUNHYUNG);
}

void CHARACTER::SetSkillGroup(uint8_t bSkillGroup)
{
	if (bSkillGroup > 2)
		return;

	if (GetLevel() < 5)
		return;

	m_points.skill_group = bSkillGroup;

	TPacketGCChangeSkillGroup p;
	p.header = HEADER_GC_SKILL_GROUP;
	p.skill_group = m_points.skill_group;

	GetDesc()->Packet(&p, sizeof(TPacketGCChangeSkillGroup));
}

int32_t CHARACTER::ComputeCooltime(int32_t time)
{
	return CalculateDuration(GetPoint(POINT_CASTING_SPEED), time);
}

void CHARACTER::SkillLevelPacket()
{
	if (!GetDesc())
		return;

	TPacketGCSkillLevel pack;

	pack.bHeader = HEADER_GC_SKILL_LEVEL;
	memcpy(&pack.skills, m_pSkillLevels, sizeof(TPlayerSkill) * SKILL_MAX_NUM);
	GetDesc()->Packet(&pack, sizeof(TPacketGCSkillLevel));
}

void CHARACTER::SetSkillLevel(uint32_t dwVnum, uint8_t bLev)
{
	if (nullptr == m_pSkillLevels)
		return;

	if (dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("vnum overflow (vnum %u)", dwVnum);
		return;
	}

#ifdef __678TH_SKILL__
#ifdef ENABLE_WOLFMAN_CHARACTER
	if (dwVnum >= SKILL_ANTI_PALBANG && dwVnum <= SKILL_HELP_SALPOONG)
#else
	if (dwVnum >= SKILL_ANTI_PALBANG && dwVnum <= SKILL_HELP_BYEURAK)
#endif
	{
		if (!SkillCanUp(dwVnum) && bLev != 0)
			return;
	}
#endif

	m_pSkillLevels[dwVnum].bLevel = MIN(SKILL_MAX_LEVEL, bLev);
#ifdef __SKILLS_LEVEL_OVER_P__
	if (bLev >= 50)
		m_pSkillLevels[dwVnum].bMasterType = SKILL_SAGE_MASTER;
	else if (bLev >= 40)
#else
	if (bLev >= 40)
#endif
		m_pSkillLevels[dwVnum].bMasterType = SKILL_PERFECT_MASTER;
	else if (bLev >= 30)
		m_pSkillLevels[dwVnum].bMasterType = SKILL_GRAND_MASTER;
	else if (bLev >= 20)
		m_pSkillLevels[dwVnum].bMasterType = SKILL_MASTER;
	else
		m_pSkillLevels[dwVnum].bMasterType = SKILL_NORMAL;
}

bool CHARACTER::IsLearnableSkill(uint32_t dwSkillVnum) const
{
	const CSkillProto * pkSkill = CSkillManager::instance().Get(dwSkillVnum);

	if (!pkSkill)
		return false;

	if (GetSkillLevel(dwSkillVnum) >= SKILL_MAX_LEVEL)
		return false;

	if (pkSkill->dwType == 0)
	{
		if (GetSkillLevel(dwSkillVnum) >= pkSkill->bMaxLevel)
			return false;

		return true;
	}

	if (pkSkill->dwType == 5)
	{
		if (dwSkillVnum == SKILL_HORSE_WILDATTACK_RANGE && GetJob() != JOB_ASSASSIN)
			return false;

		return true;
	}

	if (GetSkillGroup() == 0)
		return false;

	if (pkSkill->dwType - 1 == GetJob())
		return true;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	if (6 == pkSkill->dwType && JOB_WOLFMAN == GetJob())
		return true;
#endif
	if (6 == pkSkill->dwType)
	{
		if (SKILL_7_A_ANTI_TANHWAN <= dwSkillVnum && dwSkillVnum <= SKILL_7_D_ANTI_YONGBI)
		{
			for (int32_t i=0 ; i < 4 ; i++)
			{
				if (uint32_t(SKILL_7_A_ANTI_TANHWAN + i) != dwSkillVnum)
				{
					if (0 != GetSkillLevel(SKILL_7_A_ANTI_TANHWAN + i))
					{
						return false;
					}
				}
			}

			return true;
		}

		if (SKILL_8_A_ANTI_GIGONGCHAM <= dwSkillVnum && dwSkillVnum <= SKILL_8_D_ANTI_BYEURAK)
		{
			for (int32_t i=0 ; i < 4 ; i++)
			{
				if (uint32_t(SKILL_8_A_ANTI_GIGONGCHAM + i) != dwSkillVnum)
				{
					if (0 != GetSkillLevel(SKILL_8_A_ANTI_GIGONGCHAM + i))
						return false;
				}
			}

			return true;
		}
	}

#ifdef __678TH_SKILL__
	if (pkSkill->dwType == 6)
	{
#ifdef ENABLE_WOLFMAN_CHARACTER
		if (dwSkillVnum >= SKILL_ANTI_PALBANG && dwSkillVnum <= SKILL_HELP_SALPOONG)
#else
		if (dwSkillVnum >= SKILL_ANTI_PALBANG && dwSkillVnum <= SKILL_HELP_BYEURAK)
#endif
		{
			if (GetSkillLevel(dwSkillVnum) != 0)
				return true;
		}
	}
#endif

	return false;
}

bool CHARACTER::LearnGrandMasterSkill(uint32_t dwSkillVnum)
{
	CSkillProto * pkSk = CSkillManager::instance().Get(dwSkillVnum);

	if (!pkSk)
		return false;

	if (!IsLearnableSkill(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("수련할 수 없는 스킬입니다."));
		return false;
	}

	sys_log(0, "learn grand master skill[%d] cur %d, next %d", dwSkillVnum, get_global_time(), GetSkillNextReadTime(dwSkillVnum));

	if (pkSk->dwType == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("그랜드 마스터 수련을 할 수 없는 스킬입니다."));
		return false;
	}

	if (GetSkillMasterType(dwSkillVnum) != SKILL_GRAND_MASTER)
	{
		if (GetSkillMasterType(dwSkillVnum) > SKILL_GRAND_MASTER)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("퍼펙트 마스터된 스킬입니다. 더 이상 수련 할 수 없습니다."));
		else
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이 스킬은 아직 그랜드 마스터 수련을 할 경지에 이르지 않았습니다."));
		return false;
	}

	std::string strTrainSkill;
	{
		std::ostringstream os;
		os << "training_grandmaster_skill.skill" << dwSkillVnum;
		strTrainSkill = os.str();
	}

	uint8_t bLastLevel = GetSkillLevel(dwSkillVnum);

	int32_t idx = MIN(9, GetSkillLevel(dwSkillVnum) - 30);

	sys_log(0, "LearnGrandMasterSkill %s table idx %d value %d", GetName(), idx, aiGrandMasterSkillBookCountForLevelUp[idx]);

	int32_t iTotalReadCount = GetQuestFlag(strTrainSkill) + 1;
	SetQuestFlag(strTrainSkill, iTotalReadCount);

	int32_t iMinReadCount = aiGrandMasterSkillBookMinCount[idx];
	int32_t iMaxReadCount = aiGrandMasterSkillBookMaxCount[idx];

	int32_t iBookCount = aiGrandMasterSkillBookCountForLevelUp[idx];
	// @correction005
	if (FindAffect(AFFECT_SKILL_BOOK_BONUS))
	{
		if (iBookCount&1)
			iBookCount = iBookCount / 2 + 1;
		else
			iBookCount = iBookCount / 2;

		RemoveAffect(AFFECT_SKILL_BOOK_BONUS);
	}

	int32_t n = number(1, iBookCount);
	sys_log(0, "Number(%d)", n);

	uint32_t nextTime = get_global_time() + number(g_dwSkillBookNextReadMin, g_dwSkillBookNextReadMax); // @correction220

	sys_log(0, "GrandMaster SkillBookCount min %d cur %d max %d (next_time=%d)", iMinReadCount, iTotalReadCount, iMaxReadCount, nextTime);

	bool bSuccess = n == 2;

	if (iTotalReadCount < iMinReadCount)
		bSuccess = false;
	if (iTotalReadCount > iMaxReadCount)
		bSuccess = true;

	if (bSuccess)
	{
		SkillLevelUp(dwSkillVnum, SKILL_UP_BY_QUEST);
	}

	SetSkillNextReadTime(dwSkillVnum, nextTime);

	if (bLastLevel == GetSkillLevel(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("크윽, 기가 역류하고 있어! 이거 설마 주화입마인가!? 젠장!"));
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("수련이 실패로 끝났습니다. 다시 도전해주시기 바랍니다."));
		LogManager::instance().CharLog(this, dwSkillVnum, "GM_READ_FAIL", "");
		return false;
	}

	ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("몸에서 뭔가 힘이 터져 나오는 기분이야!"));
	ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("뜨거운 무엇이 계속 용솟음치고 있어! 이건, 이것은!"));
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("더 높은 경지의 수련을 성공적으로 끝내셨습니다."));
	LogManager::instance().CharLog(this, dwSkillVnum, "GM_READ_SUCCESS", "");
	return true;
}

static bool FN_should_check_exp(LPCHARACTER ch)
{
	// @correction005
	return ch->GetLevel() < gPlayerMaxLevel;
}

bool CHARACTER::LearnSkillByBook(uint32_t dwSkillVnum, uint8_t bProb)
{
	const CSkillProto* pkSk = CSkillManager::instance().Get(dwSkillVnum);

	if (!pkSk)
		return false;

	if (!IsLearnableSkill(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("수련할 수 없는 스킬입니다."));
		return false;
	}

	uint32_t need_exp = 0;

	if (FN_should_check_exp(this))
	{
		need_exp = 20000;

		if ( GetExp() < need_exp )
		{
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("경험치가 부족하여 책을 읽을 수 없습니다."));
			return false;
		}
	}

	if (pkSk->dwType != 0)
	{
		if (GetSkillMasterType(dwSkillVnum) != SKILL_MASTER)
		{
			if (GetSkillMasterType(dwSkillVnum) > SKILL_MASTER)
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이 스킬은 책으로 더이상 수련할 수 없습니다."));
			else
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이 스킬은 아직 책으로 수련할 경지에 이르지 않았습니다."));
			return false;
		}
	}

	if (get_global_time() < GetSkillNextReadTime(dwSkillVnum))
	{
		if (!(test_server && quest::CQuestManager::instance().GetEventFlag("no_read_delay")))
		{
			if (FindAffect(AFFECT_SKILL_NO_BOOK_DELAY))
			{
				RemoveAffect(AFFECT_SKILL_NO_BOOK_DELAY);
				ChatPacket(CHAT_TYPE_INFO, LC_TEXT("주안술서를 통해 주화입마에서 빠져나왔습니다."));
			}
			else
			{
				SkillLearnWaitMoreTimeMessage(GetSkillNextReadTime(dwSkillVnum) - get_global_time());
				return false;
			}
		}
	}

	uint8_t bLastLevel = GetSkillLevel(dwSkillVnum);

	if (bProb != 0)
	{
		if (FindAffect(AFFECT_SKILL_BOOK_BONUS))
		{
			bProb += bProb / 2;
			RemoveAffect(AFFECT_SKILL_BOOK_BONUS);
		}

		sys_log(0, "LearnSkillByBook Pct %u prob %d", dwSkillVnum, bProb);

		if (number(1, 100) <= bProb)
		{
			if (test_server)
				sys_log(0, "LearnSkillByBook %u SUCC", dwSkillVnum);

			SkillLevelUp(dwSkillVnum, SKILL_UP_BY_BOOK);
		}
		else
		{
			if (test_server)
				sys_log(0, "LearnSkillByBook %u FAIL", dwSkillVnum);
		}
	}
	else
	{
		int32_t idx = MIN(9, GetSkillLevel(dwSkillVnum) - 20);

		sys_log(0, "LearnSkillByBook %s table idx %d value %d", GetName(), idx, aiSkillBookCountForLevelUp[idx]);

		// @correction005
		{
			int32_t need_bookcount = GetSkillLevel(dwSkillVnum) - 20;

			PointChange(POINT_EXP, -need_exp);

			quest::CQuestManager& q = quest::CQuestManager::instance();
			quest::PC* pPC = q.GetPC(GetPlayerID());

			if (pPC)
			{
				char flag[128+1];
				memset(flag, 0, sizeof(flag));
				snprintf(flag, sizeof(flag), "traning_master_skill.%u.read_count", dwSkillVnum);

				int32_t read_count = pPC->GetFlag(flag);
				int32_t percent = 65;

				if (FindAffect(AFFECT_SKILL_BOOK_BONUS))
				{
					percent = 0;
					RemoveAffect(AFFECT_SKILL_BOOK_BONUS);
				}

				if (number(1, 100) > percent)
				{
#ifdef ENABLE_MASTER_SKILLBOOK_NO_STEPS // @correction164
					if (true)
#else
					if (read_count >= need_bookcount)
#endif
					{
						SkillLevelUp(dwSkillVnum, SKILL_UP_BY_BOOK);
						pPC->SetFlag(flag, 0);

						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("책으로 더 높은 경지의 수련을 성공적으로 끝내셨습니다."));
						LogManager::instance().CharLog(this, dwSkillVnum, "READ_SUCCESS", "");
						return true;
					}
					else
					{
						pPC->SetFlag(flag, read_count + 1);

						switch (number(1, 3))
						{
							case 1:
								ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("어느정도 이 기술에 대해 이해가 되었지만 조금 부족한듯 한데.."));
								break;

							case 2:
								ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("드디어 끝이 보이는 건가...  이 기술은 이해하기가 너무 힘들어.."));
								break;

							case 3:
							default:
								ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("열심히 하는 배움을 가지는 것만이 기술을 배울수 있는 유일한 길이다.."));
								break;
						}

						ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%d 권을 더 읽어야 수련을 완료 할 수 있습니다."), need_bookcount - read_count);
						return true;
					}
				}
			}
		}
	}

	if (bLastLevel != GetSkillLevel(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("몸에서 뭔가 힘이 터져 나오는 기분이야!"));
		ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("뜨거운 무엇이 계속 용솟음치고 있어! 이건, 이것은!"));
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("책으로 더 높은 경지의 수련을 성공적으로 끝내셨습니다."));
		LogManager::instance().CharLog(this, dwSkillVnum, "READ_SUCCESS", "");
	}
	else
	{
		ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("크윽, 기가 역류하고 있어! 이거 설마 주화입마인가!? 젠장!"));
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("수련이 실패로 끝났습니다. 다시 도전해주시기 바랍니다."));
		LogManager::instance().CharLog(this, dwSkillVnum, "READ_FAIL", "");
	}

	return true;
}

bool CHARACTER::SkillLevelDown(uint32_t dwVnum)
{
	if (nullptr == m_pSkillLevels)
		return false;

	if (g_bSkillDisable)
		return false;

	if (IsPolymorphed())
		return false;

	CSkillProto * pkSk = CSkillManager::instance().Get(dwVnum);

	if (!pkSk)
	{
		sys_err("There is no such skill by number %u", dwVnum);
		return false;
	}

	if (!IsLearnableSkill(dwVnum))
		return false;

	if (GetSkillMasterType(pkSk->dwVnum) != SKILL_NORMAL)
		return false;

	if (!GetSkillGroup())
		return false;

	if (pkSk->dwVnum >= SKILL_MAX_NUM)
		return false;

	if (m_pSkillLevels[pkSk->dwVnum].bLevel == 0)
		return false;

#ifdef __678TH_SKILL__
#ifdef ENABLE_WOLFMAN_CHARACTER
	if (pkSk->dwVnum >= SKILL_ANTI_PALBANG && pkSk->dwVnum <= SKILL_HELP_SALPOONG)
#else
	if (pkSk->dwVnum >= SKILL_ANTI_PALBANG && pkSk->dwVnum <= SKILL_HELP_BYEURAK)
#endif
	{
		if (m_pSkillLevels[pkSk->dwVnum].bLevel == 1)
			return false;
	}
#endif

	int32_t idx = POINT_SKILL;
	switch (pkSk->dwType)
	{
		case 0:
			idx = POINT_SUB_SKILL;
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 6:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		case 7:
#endif
			idx = POINT_SKILL;
			break;
		case 5:
			idx = POINT_HORSE_SKILL;
			break;
		default:
			sys_err("Wrong skill type %d skill vnum %d", pkSk->dwType, pkSk->dwVnum);
			return false;

	}

	PointChange(idx, +1);
	SetSkillLevel(pkSk->dwVnum, m_pSkillLevels[pkSk->dwVnum].bLevel - 1);

	sys_log(0, "SkillDown: %s %u %u %u type %u", GetName(), pkSk->dwVnum, m_pSkillLevels[pkSk->dwVnum].bMasterType, m_pSkillLevels[pkSk->dwVnum].bLevel, pkSk->dwType);
	Save();

	ComputePoints();
	SkillLevelPacket();
	return true;
}

void CHARACTER::SkillLevelUp(uint32_t dwVnum, uint8_t bMethod)
{
	if (nullptr == m_pSkillLevels)
		return;

	if (g_bSkillDisable)
		return;

	if (IsPolymorphed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("둔갑 중에는 능력을 올릴 수 없습니다."));
		return;
	}

#ifdef __678TH_SKILL__
#ifdef ENABLE_WOLFMAN_CHARACTER
	if (dwVnum >= SKILL_ANTI_PALBANG && dwVnum <= SKILL_HELP_SALPOONG)
#else
	if (dwVnum >= SKILL_ANTI_PALBANG && dwVnum <= SKILL_HELP_BYEURAK)
#endif
	{
		if (!SkillCanUp(dwVnum))
			return;
	}
#endif

	if (SKILL_7_A_ANTI_TANHWAN <= dwVnum && dwVnum <= SKILL_8_D_ANTI_BYEURAK)
	{
		if (0 == GetSkillLevel(dwVnum))
			return;
	}

	const CSkillProto* pkSk = CSkillManager::instance().Get(dwVnum);

	if (!pkSk)
	{
		sys_err("There is no such skill by number (vnum %u)", dwVnum);
		return;
	}

	if (pkSk->dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("Skill Vnum overflow (vnum %u)", dwVnum);
		return;
	}

	if (!IsLearnableSkill(dwVnum))
		return;

	if (pkSk->dwType != 0)
	{
		switch (GetSkillMasterType(pkSk->dwVnum))
		{
			case SKILL_GRAND_MASTER:
				if (bMethod != SKILL_UP_BY_QUEST)
					return;
				break;

			case SKILL_PERFECT_MASTER:
#ifdef __SKILLS_LEVEL_OVER_P__
				if (bMethod != SKILL_UP_BY_QUEST)
					return;
				
				break;
#else
				return;
#endif
		}
	}

	if (bMethod == SKILL_UP_BY_POINT)
	{
		if (GetSkillMasterType(pkSk->dwVnum) != SKILL_NORMAL)
			return;

		if (IS_SET(pkSk->dwFlag, SKILL_FLAG_DISABLE_BY_POINT_UP))
			return;
	}
	else if (bMethod == SKILL_UP_BY_BOOK)
	{
		if (pkSk->dwType != 0)
			if (GetSkillMasterType(pkSk->dwVnum) != SKILL_MASTER)
				return;
	}

	if (GetLevel() < pkSk->bLevelLimit)
		return;

	if (pkSk->preSkillVnum)
		if (GetSkillMasterType(pkSk->preSkillVnum) == SKILL_NORMAL &&
			GetSkillLevel(pkSk->preSkillVnum) < pkSk->preSkillLevel)
			return;

	if (!GetSkillGroup())
		return;

	if (bMethod == SKILL_UP_BY_POINT)
	{
		int32_t idx;

		switch (pkSk->dwType)
		{
			case 0:
				idx = POINT_SUB_SKILL;
				break;

			case 1:
			case 2:
			case 3:
			case 4:
			case 6:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
			case 7:
#endif
				idx = POINT_SKILL;
				break;

			case 5:
				idx = POINT_HORSE_SKILL;
				break;

			default:
				sys_err("Wrong skill type %d skill vnum %d", pkSk->dwType, pkSk->dwVnum);
				return;
		}

		if (GetPoint(idx) < 1)
			return;

		PointChange(idx, -1);
	}

	int32_t SkillPointBefore = GetSkillLevel(pkSk->dwVnum);
	SetSkillLevel(pkSk->dwVnum, m_pSkillLevels[pkSk->dwVnum].bLevel + 1);

	if (pkSk->dwType != 0)
	{
		switch (GetSkillMasterType(pkSk->dwVnum))
		{
			case SKILL_NORMAL:
				if (GetSkillLevel(pkSk->dwVnum) >= 17)
				{
#ifdef ENABLE_FORCE2MASTERSKILL // @correction161
					SetSkillLevel(pkSk->dwVnum, 20);
#else
					if (GetQuestFlag("reset_scroll.force_to_master_skill") > 0)
					{
						SetSkillLevel(pkSk->dwVnum, 20);
						SetQuestFlag("reset_scroll.force_to_master_skill", 0);
					}
					else
					{
						if (number(1, 21 - MIN(20, GetSkillLevel(pkSk->dwVnum))) == 1)
							SetSkillLevel(pkSk->dwVnum, 20);
					}
#endif
				}
				break;

			case SKILL_MASTER:
				if (GetSkillLevel(pkSk->dwVnum) >= 30)
				{
					if (number(1, 31 - MIN(30, GetSkillLevel(pkSk->dwVnum))) == 1)
						SetSkillLevel(pkSk->dwVnum, 30);
				}
				break;

			case SKILL_GRAND_MASTER:
				if (GetSkillLevel(pkSk->dwVnum) >= 40)
				{
					SetSkillLevel(pkSk->dwVnum, 40);
				}
				break;

#ifdef __SKILLS_LEVEL_OVER_P__
			case SKILL_PERFECT_MASTER:
				if (GetSkillLevel(pkSk->dwVnum) >= 49)
				{
					SetSkillLevel(pkSk->dwVnum, 49);
				}
				break;
#endif
		}
	}

	char szSkillUp[1024];

	snprintf(szSkillUp, sizeof(szSkillUp), "SkillUp: %s %u %d %d[Before:%d] type %u",
			GetName(), pkSk->dwVnum, m_pSkillLevels[pkSk->dwVnum].bMasterType, m_pSkillLevels[pkSk->dwVnum].bLevel, SkillPointBefore, pkSk->dwType);

	sys_log(0, "%s", szSkillUp);

	LogManager::instance().CharLog(this, pkSk->dwVnum, "SKILLUP", szSkillUp);
	Save();

	ComputePoints();
	SkillLevelPacket();
}

void CHARACTER::ComputeSkillPoints()
{
	if (g_bSkillDisable)
		return;
}

void CHARACTER::ResetSkill()
{
	if (nullptr == m_pSkillLevels)
		return;

	std::vector<std::pair<uint32_t, TPlayerSkill> > vec;
	size_t count = sizeof(s_adwSubSkillVnums) / sizeof(s_adwSubSkillVnums[0]);

	for (size_t i = 0; i < count; ++i)
	{
		if (s_adwSubSkillVnums[i] >= SKILL_MAX_NUM)
			continue;

		vec.push_back(std::make_pair(s_adwSubSkillVnums[i], m_pSkillLevels[s_adwSubSkillVnums[i]]));
	}

	memset(m_pSkillLevels, 0, sizeof(TPlayerSkill) * SKILL_MAX_NUM);

	std::vector<std::pair<uint32_t, TPlayerSkill> >::const_iterator iter = vec.begin();

	while (iter != vec.end())
	{
		const std::pair<uint32_t, TPlayerSkill>& pair = *(iter++);
		m_pSkillLevels[pair.first] = pair.second;
	}

	ComputePoints();
	SkillLevelPacket();
}

void CHARACTER::ComputePassiveSkill(uint32_t dwVnum)
{
	if (g_bSkillDisable)
		return;

	if (GetSkillLevel(dwVnum) == 0)
		return;

	CSkillProto * pkSk = CSkillManager::instance().Get(dwVnum);
	pkSk->SetPointVar("k", GetSkillLevel(dwVnum));
	int32_t iAmount = (int32_t) pkSk->kPointPoly.Eval();

	sys_log(2, "%s passive #%d on %d amount %d", GetName(), dwVnum, pkSk->bPointOn, iAmount);
	PointChange(pkSk->bPointOn, iAmount);
}

struct FFindNearVictim
{
	FFindNearVictim(LPCHARACTER center, LPCHARACTER attacker, const CHARACTER_SET& excepts_set = empty_set_)
		: m_pkChrCenter(center),
	m_pkChrNextTarget(nullptr),
	m_pkChrAttacker(attacker),
	m_count(0),
	m_excepts_set(excepts_set)
	{
	}

	void operator ()(LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
			return;

		LPCHARACTER pkChr = (LPCHARACTER) ent;

		if (!m_excepts_set.empty()) {
			if (m_excepts_set.find(pkChr) != m_excepts_set.end())
				return;
		}

		if (m_pkChrCenter == pkChr)
			return;

		if (!battle_is_attackable(m_pkChrAttacker, pkChr))
		{
			return;
		}

		if (abs(m_pkChrCenter->GetX() - pkChr->GetX()) > 1000 || abs(m_pkChrCenter->GetY() - pkChr->GetY()) > 1000)
			return;

		float fDist = DISTANCE_APPROX(m_pkChrCenter->GetX() - pkChr->GetX(), m_pkChrCenter->GetY() - pkChr->GetY());

		if (fDist < 1000)
		{
			++m_count;

			if ((m_count == 1) || number(1, m_count) == 1)
				m_pkChrNextTarget = pkChr;
		}
	}

	LPCHARACTER GetVictim()
	{
		return m_pkChrNextTarget;
	}

	LPCHARACTER m_pkChrCenter;
	LPCHARACTER m_pkChrNextTarget;
	LPCHARACTER m_pkChrAttacker;
	int32_t		m_count;
	const CHARACTER_SET & m_excepts_set;
private:
	static CHARACTER_SET empty_set_;
};

CHARACTER_SET FFindNearVictim::empty_set_;

EVENTINFO(chain_lightning_event_info)
{
	uint32_t			dwVictim;
	uint32_t			dwChr;

	chain_lightning_event_info()
	: dwVictim(0)
	, dwChr(0)
	{
	}
};

EVENTFUNC(ChainLightningEvent)
{
	chain_lightning_event_info * info = dynamic_cast<chain_lightning_event_info *>( event->info );

	LPCHARACTER pkChrVictim = CHARACTER_MANAGER::instance().Find(info->dwVictim);
	LPCHARACTER pkChr = CHARACTER_MANAGER::instance().Find(info->dwChr);
	LPCHARACTER pkTarget = nullptr;

	if (!pkChr || !pkChrVictim)
	{
		sys_log(1, "use chainlighting, but no character");
		return 0;
	}

	sys_log(1, "chainlighting event %s", pkChr->GetName());

	if (pkChrVictim->GetParty())
	{
		pkTarget = pkChrVictim->GetParty()->GetNextOwnership(nullptr, pkChrVictim->GetX(), pkChrVictim->GetY());
		if (pkTarget == pkChrVictim || !number(0, 2) || pkChr->GetChainLightingExcept().find(pkTarget) != pkChr->GetChainLightingExcept().end())
			pkTarget = nullptr;
	}

	if (!pkTarget)
	{
		FFindNearVictim f(pkChrVictim, pkChr, pkChr->GetChainLightingExcept());

		if (pkChrVictim->GetSectree())
		{
			pkChrVictim->GetSectree()->ForEachAround(f);
			pkTarget = f.GetVictim();
		}
	}

	if (pkTarget)
	{
		pkChrVictim->CreateFly(FLY_CHAIN_LIGHTNING, pkTarget);
		pkChr->ComputeSkill(SKILL_CHAIN, pkTarget);
		pkChr->AddChainLightningExcept(pkTarget);
	}
	else
	{
		sys_log(1, "%s use chainlighting, but find victim failed near %s", pkChr->GetName(), pkChrVictim->GetName());
	}

	return 0;
}

void SetPolyVarForAttack(LPCHARACTER ch, CSkillProto * pkSk, LPITEM pkWeapon)
{
	if (ch->IsPC())
	{
		if (pkWeapon && pkWeapon->GetType() == ITEM_WEAPON)
		{
			int32_t iWep = number(pkWeapon->GetValue(3), pkWeapon->GetValue(4));
			iWep += pkWeapon->GetValue(5);

			int32_t iMtk = number(pkWeapon->GetValue(1), pkWeapon->GetValue(2));
			iMtk += pkWeapon->GetValue(5);

			pkSk->SetPointVar("wep", iWep);
			pkSk->SetPointVar("mtk", iMtk);
			pkSk->SetPointVar("mwep", iMtk);
		}
		else
		{
			pkSk->SetPointVar("wep", 0);
			pkSk->SetPointVar("mtk", 0);
			pkSk->SetPointVar("mwep", 0);
		}
	}
	else
	{
		int32_t iWep = number(ch->GetMobDamageMin(), ch->GetMobDamageMax());
		pkSk->SetPointVar("wep", iWep);
		pkSk->SetPointVar("mwep", iWep);
		pkSk->SetPointVar("mtk", iWep);
	}
}

struct FuncSplashDamage
{
	FuncSplashDamage(int32_t x, int32_t y, CSkillProto * pkSk, LPCHARACTER pkChr, int32_t iAmount, int32_t iAG, int32_t iMaxHit, LPITEM pkWeapon, bool bDisableCooltime, TSkillUseInfo* pInfo, uint8_t bUseSkillPower)
		:
		m_x(x), m_y(y), m_pkSk(pkSk), m_pkChr(pkChr), m_iAmount(iAmount), m_iAG(iAG), m_iCount(0), m_iMaxHit(iMaxHit), m_pkWeapon(pkWeapon), m_bDisableCooltime(bDisableCooltime), m_pInfo(pInfo), m_bUseSkillPower(bUseSkillPower)
		{
		}

	void operator () (LPENTITY ent)
	{
		if (!ent->IsType(ENTITY_CHARACTER))
		{
			return;
		}

		LPCHARACTER pkChrVictim = (LPCHARACTER) ent;

		if (DISTANCE_APPROX(m_x - pkChrVictim->GetX(), m_y - pkChrVictim->GetY()) > m_pkSk->iSplashRange)
		{
			if(test_server)
				sys_log(0, "XXX target too far %s", m_pkChr->GetName());
			return;
		}

		if (!battle_is_attackable(m_pkChr, pkChrVictim))
		{
			if(test_server)
				sys_log(0, "XXX target not attackable %s", m_pkChr->GetName());
			return;
		}

		if (m_pkChr->IsPC())
			if (!(m_pkSk->dwVnum >= GUILD_SKILL_START && m_pkSk->dwVnum <= GUILD_SKILL_END))
				if (!m_bDisableCooltime && m_pInfo && !m_pInfo->HitOnce(m_pkSk->dwVnum) && m_pkSk->dwVnum != SKILL_MUYEONG)
				{
					if(test_server)
						sys_log(0, "check guild skill %s", m_pkChr->GetName());
					return;
				}

		++m_iCount;

		int32_t iDam;

		m_pkSk->SetPointVar("k", 1.0 * m_bUseSkillPower * m_pkSk->bMaxLevel / 100);
		m_pkSk->SetPointVar("lv", m_pkChr->GetLevel());
		m_pkSk->SetPointVar("iq", m_pkChr->GetPoint(POINT_IQ));
		m_pkSk->SetPointVar("str", m_pkChr->GetPoint(POINT_ST));
		m_pkSk->SetPointVar("dex", m_pkChr->GetPoint(POINT_DX));
		m_pkSk->SetPointVar("con", m_pkChr->GetPoint(POINT_HT));
		m_pkSk->SetPointVar("def", m_pkChr->GetPoint(POINT_DEF_GRADE));
		m_pkSk->SetPointVar("odef", m_pkChr->GetPoint(POINT_DEF_GRADE) - m_pkChr->GetPoint(POINT_DEF_GRADE_BONUS));
		m_pkSk->SetPointVar("horse_level", m_pkChr->GetHorseLevel());

		bool bIgnoreDefense = false;

		if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_PENETRATE))
		{
			int32_t iPenetratePct = (int32_t) m_pkSk->kPointPoly2.Eval();

			if (number(1, 100) <= iPenetratePct)
				bIgnoreDefense = true;
		}

		bool bIgnoreTargetRating = false;

		if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_IGNORE_TARGET_RATING))
		{
			int32_t iPct = (int32_t) m_pkSk->kPointPoly2.Eval();

			if (number(1, 100) <= iPct)
				bIgnoreTargetRating = true;
		}

		m_pkSk->SetPointVar("ar", CalcAttackRating(m_pkChr, pkChrVictim, bIgnoreTargetRating));

		if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_USE_MELEE_DAMAGE))
			m_pkSk->SetPointVar("atk", CalcMeleeDamage(m_pkChr, pkChrVictim, true, bIgnoreTargetRating));
		else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_USE_ARROW_DAMAGE))
		{
			LPITEM pkBow, pkArrow;

			if (1 == m_pkChr->GetArrowAndBow(&pkBow, &pkArrow, 1))
				m_pkSk->SetPointVar("atk", CalcArrowDamage(m_pkChr, pkChrVictim, pkBow, pkArrow, true));
			else
				m_pkSk->SetPointVar("atk", 0);
		}

		if (m_pkSk->bPointOn == POINT_MOV_SPEED)
			m_pkSk->kPointPoly.SetVar("maxv", pkChrVictim->GetLimitPoint(POINT_MOV_SPEED));

		m_pkSk->SetPointVar("maxhp", pkChrVictim->GetMaxHP());
		m_pkSk->SetPointVar("maxsp", pkChrVictim->GetMaxSP());

		m_pkSk->SetPointVar("chain", m_pkChr->GetChainLightningIndex());
		m_pkChr->IncChainLightningIndex();

		bool bUnderEunhyung = m_pkChr->GetAffectedEunhyung() > 0;

		m_pkSk->SetPointVar("ek", m_pkChr->GetAffectedEunhyung()*1./100);
		SetPolyVarForAttack(m_pkChr, m_pkSk, m_pkWeapon);

		int32_t iAmount = 0;

		if (m_pkChr->GetUsedSkillMasterType(m_pkSk->dwVnum) >= SKILL_GRAND_MASTER)
		{
			iAmount = (int32_t) m_pkSk->kMasterBonusPoly.Eval();
		}
		else
		{
			iAmount = (int32_t) m_pkSk->kPointPoly.Eval();
		}

		if (test_server && iAmount == 0 && m_pkSk->bPointOn != POINT_NONE)
		{
			m_pkChr->ChatPacket(CHAT_TYPE_INFO, "효과가 없습니다. 스킬 공식을 확인하세요");
		}
		iAmount = -iAmount;

		if (m_pkSk->dwVnum == SKILL_AMSEOP)
		{
			float fDelta = GetDegreeDelta(m_pkChr->GetRotation(), pkChrVictim->GetRotation());
			float adjust;

			if (fDelta < 35.0f)
			{
				adjust = 1.5f;

				if (bUnderEunhyung)
					adjust += 0.5f;

				if (m_pkChr->GetWear(WEAR_WEAPON) && m_pkChr->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_DAGGER)
				{
					// @correction005
					adjust += 0.5f;
				}
			}
			else
			{
				adjust = 1.0f;

				// @correction005
				if (bUnderEunhyung)
					adjust += 0.5f;

				if (m_pkChr->GetWear(WEAR_WEAPON) && m_pkChr->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_DAGGER)
					adjust += 0.5f;
			}

			iAmount = (int32_t) (iAmount * adjust);
		}
		else if (m_pkSk->dwVnum == SKILL_GUNGSIN)
		{
			float adjust = 1.0;

			if (m_pkChr->GetWear(WEAR_WEAPON) && m_pkChr->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_DAGGER)
			{
				// @correction005
				adjust = 1.35f;
			}

			iAmount = (int32_t) (iAmount * adjust);
		}
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		else if (m_pkSk->dwVnum == SKILL_GONGDAB)
		{
			float adjust = 1.0;

			if (m_pkChr->GetWear(WEAR_WEAPON) && m_pkChr->GetWear(WEAR_WEAPON)->GetSubType() == WEAPON_CLAW)
			{
				adjust = 1.35f;
			}

			iAmount = (int32_t)(iAmount * adjust);
		}
#endif

		iDam = CalcBattleDamage(iAmount, m_pkChr->GetLevel(), pkChrVictim->GetLevel());

		if (m_pkChr->IsPC() && m_pkChr->m_SkillUseInfo[m_pkSk->dwVnum].GetMainTargetVID() != (uint32_t) pkChrVictim->GetVID())
		{
			iDam = (int32_t) (iDam * m_pkSk->kSplashAroundDamageAdjustPoly.Eval());
		}

		EDamageType dt = DAMAGE_TYPE_NONE;

		switch (m_pkSk->bSkillAttrType)
		{
			case SKILL_ATTR_TYPE_NORMAL:
				break;

			case SKILL_ATTR_TYPE_MELEE:
				{
					dt = DAMAGE_TYPE_MELEE;

					LPITEM pkWeapon = m_pkChr->GetWear(WEAR_WEAPON);

					if (pkWeapon)
						switch (pkWeapon->GetSubType())
						{
							case WEAPON_SWORD:
								iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_SWORD)) / 100;
								break;

							case WEAPON_TWO_HANDED:
								iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_TWOHAND)) / 100;

								break;

							case WEAPON_DAGGER:
								iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_DAGGER)) / 100;
								break;

							case WEAPON_BELL:
								iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_BELL)) / 100;
								break;

							case WEAPON_FAN:
								iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_FAN)) / 100;
								break;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
							case WEAPON_CLAW:
#ifdef USE_ITEM_CLAW_AS_DAGGER
								iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_DAGGER)) / 100;
#else
								iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_CLAW)) / 100;
#endif
								break;
#endif
						}

					if (!bIgnoreDefense)
						iDam -= pkChrVictim->GetPoint(POINT_DEF_GRADE);
				}
				break;

			case SKILL_ATTR_TYPE_RANGE:
				dt = DAMAGE_TYPE_RANGE;
				iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_BOW)) / 100;
				break;

			case SKILL_ATTR_TYPE_MAGIC:
			{
				dt = DAMAGE_TYPE_MAGIC;
				iDam = CalcAttBonus(m_pkChr, pkChrVictim, iDam);
				int32_t reduce_resist_magic = pkChrVictim->GetPoint(POINT_RESIST_MAGIC);
#ifdef __ANTI_RESIST_MAGIC_REDUCTION__
				if (m_pkChr->GetPoint(POINT_RESIST_MAGIC_REDUCTION) > 0)
				{
					// Fix Magic Resistance : If player have more than 100% magic resistance, will be modified to 100%;
					int32_t fix_magic_resistance = (pkChrVictim->GetPoint(POINT_RESIST_MAGIC) > 100) ? (int32_t)(100) : (int32_t)(pkChrVictim->GetPoint(POINT_RESIST_MAGIC));
					// End Fix;

					reduce_resist_magic = fix_magic_resistance - m_pkChr->GetPoint(POINT_RESIST_MAGIC_REDUCTION);
					if (reduce_resist_magic < 1)
						reduce_resist_magic = 0;
				}
#endif
				iDam = iDam * (100 - reduce_resist_magic) / 100;
			}
			break;

			default:
				sys_err("Unknown skill attr type %u vnum %u", m_pkSk->bSkillAttrType, m_pkSk->dwVnum);
				break;
		}

		if (pkChrVictim->IsNPC())
		{
			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_WIND))
			{
				iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_WIND)) / 100;
			}

			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_ELEC))
			{
				iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_ELEC)) / 100;
			}

			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_FIRE))
			{
				iDam = iDam * (100 - pkChrVictim->GetPoint(POINT_RESIST_FIRE)) / 100;
			}
		}

		if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_COMPUTE_MAGIC_DAMAGE))
			dt = DAMAGE_TYPE_MAGIC;

		if (pkChrVictim->CanBeginFight())
			pkChrVictim->BeginFight(m_pkChr);

		if (m_pkSk->dwVnum == SKILL_CHAIN)
			sys_log(0, "%s CHAIN INDEX %d DAM %d DT %d", m_pkChr->GetName(), m_pkChr->GetChainLightningIndex() - 1, iDam, dt);

#ifdef __678TH_SKILL__
		{
			uint8_t HELP_SKILL_ID = 0;
			switch (m_pkSk->dwVnum)
			{
			case SKILL_PALBANG:
				HELP_SKILL_ID = SKILL_HELP_PALBANG;
				break;
			case SKILL_AMSEOP:
				HELP_SKILL_ID = SKILL_HELP_AMSEOP;
				break;
			case SKILL_SWAERYUNG:
				HELP_SKILL_ID = SKILL_HELP_SWAERYUNG;
				break;
			case SKILL_YONGBI:
				HELP_SKILL_ID = SKILL_HELP_YONGBI;
				break;
			case SKILL_GIGONGCHAM:
				HELP_SKILL_ID = SKILL_HELP_GIGONGCHAM;
				break;
			case SKILL_HWAJO:
				HELP_SKILL_ID = SKILL_HELP_HWAJO;
				break;
			case SKILL_MARYUNG:
				HELP_SKILL_ID = SKILL_HELP_MARYUNG;
				break;
			case SKILL_BYEURAK:
				HELP_SKILL_ID = SKILL_HELP_BYEURAK;
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case SKILL_SALPOONG:
				HELP_SKILL_ID = SKILL_HELP_SALPOONG;
				break;
#endif
			default:
				break;
			}

			if (HELP_SKILL_ID != 0)
			{
				uint8_t HELP_SKILL_LV = m_pkChr->GetSkillLevel(HELP_SKILL_ID);
				if (HELP_SKILL_LV != 0)
				{
					CSkillProto* pkSk = CSkillManager::instance().Get(HELP_SKILL_ID);
					if (!pkSk)
						sys_err("Can't find %d skill in skill_proto.", HELP_SKILL_ID);
					else
					{
						pkSk->SetPointVar("k", 1.0f * m_pkChr->GetSkillPower(HELP_SKILL_ID) * pkSk->bMaxLevel / 100);

						double IncreaseAmount = pkSk->kPointPoly.Eval();
						sys_log(0, "HELP_SKILL: increase amount: %lf, normal damage: %d, increased damage: %d.", IncreaseAmount, iDam, int32_t(iDam * (IncreaseAmount / 100.0)));
						iDam += iDam * (IncreaseAmount / 100.0);
					}
				}
			}
		}

		{
			uint8_t ANTI_SKILL_ID = 0;
			switch (m_pkSk->dwVnum)
			{
			case SKILL_PALBANG:
				ANTI_SKILL_ID = SKILL_ANTI_PALBANG;
				break;
			case SKILL_AMSEOP:
				ANTI_SKILL_ID = SKILL_ANTI_AMSEOP;
				break;
			case SKILL_SWAERYUNG:
				ANTI_SKILL_ID = SKILL_ANTI_SWAERYUNG;
				break;
			case SKILL_YONGBI:
				ANTI_SKILL_ID = SKILL_ANTI_YONGBI;
				break;
			case SKILL_GIGONGCHAM:
				ANTI_SKILL_ID = SKILL_ANTI_GIGONGCHAM;
				break;
			case SKILL_HWAJO:
				ANTI_SKILL_ID = SKILL_ANTI_HWAJO;
				break;
			case SKILL_MARYUNG:
				ANTI_SKILL_ID = SKILL_ANTI_MARYUNG;
				break;
			case SKILL_BYEURAK:
				ANTI_SKILL_ID = SKILL_ANTI_BYEURAK;
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case SKILL_SALPOONG:
				ANTI_SKILL_ID = SKILL_ANTI_SALPOONG;
				break;
#endif
			default:
				break;
			}

			if (ANTI_SKILL_ID != 0)
			{
				uint8_t ANTI_SKILL_LV = pkChrVictim->GetSkillLevel(ANTI_SKILL_ID);
				if (ANTI_SKILL_LV != 0)
				{
					CSkillProto* pkSk = CSkillManager::instance().Get(ANTI_SKILL_ID);
					if (!pkSk)
						sys_err("Can't find %d skill in skill_proto.", ANTI_SKILL_ID);
					else
					{
						pkSk->SetPointVar("k", 1.0f * pkChrVictim->GetSkillPower(ANTI_SKILL_ID) * pkSk->bMaxLevel / 100);

						double ResistAmount = pkSk->kPointPoly.Eval();
						sys_log(0, "ANTI_SKILL: resist amount: %lf, normal damage: %d, reduced damage: %d.", ResistAmount, iDam, int32_t(iDam * (ResistAmount / 100.0)));
						iDam -= iDam * (ResistAmount / 100.0);
					}
				}
			}
		}
#endif

		{
			uint8_t AntiSkillID = 0;

			switch (m_pkSk->dwVnum)
			{
				case SKILL_TANHWAN:		AntiSkillID = SKILL_7_A_ANTI_TANHWAN;		break;
				case SKILL_AMSEOP:		AntiSkillID = SKILL_7_B_ANTI_AMSEOP;		break;
				case SKILL_SWAERYUNG:	AntiSkillID = SKILL_7_C_ANTI_SWAERYUNG;		break;
				case SKILL_YONGBI:		AntiSkillID = SKILL_7_D_ANTI_YONGBI;		break;
				case SKILL_GIGONGCHAM:	AntiSkillID = SKILL_8_A_ANTI_GIGONGCHAM;	break;
				case SKILL_YEONSA:		AntiSkillID = SKILL_8_B_ANTI_YEONSA;		break;
				case SKILL_MAHWAN:		AntiSkillID = SKILL_8_C_ANTI_MAHWAN;		break;
				case SKILL_BYEURAK:		AntiSkillID = SKILL_8_D_ANTI_BYEURAK;		break;
			}

			if (0 != AntiSkillID)
			{
				uint8_t AntiSkillLevel = pkChrVictim->GetSkillLevel(AntiSkillID);

				if (0 != AntiSkillLevel)
				{
					CSkillProto* pkSk = CSkillManager::instance().Get(AntiSkillID);
					if (!pkSk)
					{
						sys_err ("There is no anti skill(%d) in skill proto", AntiSkillID);
					}
					else
					{
						pkSk->SetPointVar("k", 1.0f * pkChrVictim->GetSkillPower(AntiSkillID) * pkSk->bMaxLevel / 100);

						double ResistAmount = pkSk->kPointPoly.Eval();

						sys_log(0, "ANTI_SKILL: Resist(%lf) Orig(%d) Reduce(%d)", ResistAmount, iDam, int32_t(iDam * (ResistAmount/100.0)));

						iDam -= iDam * (ResistAmount/100.0);
					}
				}
			}
		}

		if (!pkChrVictim->Damage(m_pkChr, iDam, dt) && !pkChrVictim->IsStun())
		{
			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_REMOVE_GOOD_AFFECT))
			{
#ifdef ENABLE_NULLIFYAFFECT_LIMIT // @correction163
				int32_t iLevel = m_pkChr->GetLevel();
				int32_t yLevel = pkChrVictim->GetLevel();
				int32_t iDifLev = 9;
				if ((iLevel-iDifLev <= yLevel) && (iLevel+iDifLev >= yLevel))
#endif
				{
					int32_t iAmount2 = (int32_t) m_pkSk->kPointPoly2.Eval();
					int32_t iDur2 = (int32_t) m_pkSk->kDurationPoly2.Eval();
					iDur2 += m_pkChr->GetPoint(POINT_PARTY_BUFFER_BONUS);

					if (number(1, 100) <= iAmount2)
					{
						pkChrVictim->RemoveGoodAffect(true);
						pkChrVictim->AddAffect(m_pkSk->dwVnum, POINT_NONE, 0, AFF_PABEOP, iDur2, 0, true);
					}
				}
			}
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_SLOW | SKILL_FLAG_STUN | SKILL_FLAG_FIRE_CONT | SKILL_FLAG_POISON | SKILL_FLAG_BLEEDING))
#else
			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_SLOW | SKILL_FLAG_STUN | SKILL_FLAG_FIRE_CONT | SKILL_FLAG_POISON))
#endif
			{
				int32_t iPct = (int32_t) m_pkSk->kPointPoly2.Eval();
				int32_t iDur = (int32_t) m_pkSk->kDurationPoly2.Eval();

				iDur += m_pkChr->GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_STUN))
				{
					SkillAttackAffect(pkChrVictim, iPct, IMMUNE_STUN, AFFECT_STUN, POINT_NONE, 0, AFF_STUN, iDur, m_pkSk->szName);
				}
				else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_SLOW))
				{
					SkillAttackAffect(pkChrVictim, iPct, IMMUNE_SLOW, AFFECT_SLOW, POINT_MOV_SPEED, -30, AFF_SLOW, iDur, m_pkSk->szName);
				}
				else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_FIRE_CONT))
				{
					m_pkSk->SetDurationVar("k", 1.0 * m_bUseSkillPower * m_pkSk->bMaxLevel / 100);
					m_pkSk->SetDurationVar("iq", m_pkChr->GetPoint(POINT_IQ));

					iDur = (int32_t)m_pkSk->kDurationPoly2.Eval();
					int32_t bonus = m_pkChr->GetPoint(POINT_PARTY_BUFFER_BONUS);

					if (bonus != 0)
					{
						iDur += bonus / 2;
					}

					if (number(1, 100) <= iDur)
					{
						pkChrVictim->AttackedByFire(m_pkChr, iPct, 5);
					}
				}
				else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_POISON))
				{
					if (number(1, 100) <= iPct)
						pkChrVictim->AttackedByPoison(m_pkChr);
				}
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
				else if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_BLEEDING))
				{
					if (number(1, 100) <= iPct)
						pkChrVictim->AttackedByBleeding(m_pkChr);
				}
#endif
			}

			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_CRUSH | SKILL_FLAG_CRUSH_LONG) &&
				!IS_SET(pkChrVictim->GetAIFlag(), AIFLAG_NOMOVE))
			{
				float fCrushSlidingLength = 200;

				if (m_pkChr->IsNPC())
					fCrushSlidingLength = 400;

				if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_CRUSH_LONG))
					fCrushSlidingLength *= 2;

				float fx, fy;
				float degree = GetDegreeFromPositionXY(m_pkChr->GetX(), m_pkChr->GetY(), pkChrVictim->GetX(), pkChrVictim->GetY());

				if (m_pkSk->dwVnum == SKILL_HORSE_WILDATTACK)
				{
					degree -= m_pkChr->GetRotation();
					degree = fmod(degree, 360.0f) - 180.0f;

					if (degree > 0)
						degree = m_pkChr->GetRotation() + 90.0f;
					else
						degree = m_pkChr->GetRotation() - 90.0f;
				}

				GetDeltaByDegree(degree, fCrushSlidingLength, &fx, &fy);
				sys_log(0, "CRUSH! %s -> %s (%d %d) -> (%d %d)", m_pkChr->GetName(), pkChrVictim->GetName(), pkChrVictim->GetX(), pkChrVictim->GetY(), (int32_t)(pkChrVictim->GetX()+fx), (int32_t)(pkChrVictim->GetY()+fy));
				int32_t tx = (int32_t)(pkChrVictim->GetX()+fx);
				int32_t ty = (int32_t)(pkChrVictim->GetY()+fy);
				// @correction081 BEGIN
				while (pkChrVictim->GetSectree()->GetAttribute(tx, ty) & (ATTR_BLOCK | ATTR_OBJECT) && fCrushSlidingLength > 0)
				{
					if (fCrushSlidingLength >= 10)
						fCrushSlidingLength -= 10;
					else
						fCrushSlidingLength = 0;

					GetDeltaByDegree(degree, fCrushSlidingLength, &fx, &fy);
					tx = (int32_t)(pkChrVictim->GetX() + fx);
					ty = (int32_t)(pkChrVictim->GetY() + fy);
				}
				// @correction081 END

				pkChrVictim->Sync(tx, ty);
				pkChrVictim->Goto(tx, ty);
				pkChrVictim->CalculateMoveDuration();

				if (m_pkChr->IsPC() && m_pkChr->m_SkillUseInfo[m_pkSk->dwVnum].GetMainTargetVID() == (uint32_t) pkChrVictim->GetVID())
				{
					// @correction005
					SkillAttackAffect(pkChrVictim, 1000, IMMUNE_STUN, m_pkSk->dwVnum, POINT_NONE, 0, AFF_STUN, 4, m_pkSk->szName);
				}
				else
				{
					pkChrVictim->SyncPacket();
				}
			}
		}

		if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_HP_ABSORB))
		{
			int32_t iPct = (int32_t) m_pkSk->kPointPoly2.Eval();
			m_pkChr->PointChange(POINT_HP, iDam * iPct / 100);
		}

		if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_SP_ABSORB))
		{
			int32_t iPct = (int32_t) m_pkSk->kPointPoly2.Eval();
			m_pkChr->PointChange(POINT_SP, iDam * iPct / 100);
		}

		if (m_pkSk->dwVnum == SKILL_CHAIN && m_pkChr->GetChainLightningIndex() < m_pkChr->GetChainLightningMaxCount())
		{
			chain_lightning_event_info* info = AllocEventInfo<chain_lightning_event_info>();

			info->dwVictim = pkChrVictim->GetVID();
			info->dwChr = m_pkChr->GetVID();

			event_create(ChainLightningEvent, info, passes_per_sec / 5);
		}
		if(test_server)
			sys_log(0, "FuncSplashDamage End :%s ", m_pkChr->GetName());
	}

	int32_t		m_x;
	int32_t		m_y;
	CSkillProto * m_pkSk;
	LPCHARACTER	m_pkChr;
	int32_t		m_iAmount;
	int32_t		m_iAG;
	int32_t		m_iCount;
	int32_t		m_iMaxHit;
	LPITEM	m_pkWeapon;
	bool m_bDisableCooltime;
	TSkillUseInfo* m_pInfo;
	uint8_t m_bUseSkillPower;
};

struct FuncSplashAffect
{
	FuncSplashAffect(LPCHARACTER ch, int32_t x, int32_t y, int32_t iDist, uint32_t dwVnum, uint8_t bPointOn, int32_t iAmount, uint32_t dwAffectFlag, int32_t iDuration, int32_t iSPCost, bool bOverride, int32_t iMaxHit)
	{
		m_x = x;
		m_y = y;
		m_iDist = iDist;
		m_dwVnum = dwVnum;
		m_bPointOn = bPointOn;
		m_iAmount = iAmount;
		m_dwAffectFlag = dwAffectFlag;
		m_iDuration = iDuration;
		m_iSPCost = iSPCost;
		m_bOverride = bOverride;
		m_pkChrAttacker = ch;
		m_iMaxHit = iMaxHit;
		m_iCount = 0;
	}

	void operator () (LPENTITY ent)
	{
		if (m_iMaxHit && m_iMaxHit <= m_iCount)
			return;

		if (ent->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pkChr = (LPCHARACTER) ent;

			if (test_server)
				sys_log(0, "FuncSplashAffect step 1 : name:%s vnum:%d iDur:%d", pkChr->GetName(), m_dwVnum, m_iDuration);
			if (DISTANCE_APPROX(m_x - pkChr->GetX(), m_y - pkChr->GetY()) < m_iDist)
			{
				if (test_server)
					sys_log(0, "FuncSplashAffect step 2 : name:%s vnum:%d iDur:%d", pkChr->GetName(), m_dwVnum, m_iDuration);
				if (m_dwVnum == SKILL_TUSOK)
					if (pkChr->CanBeginFight())
						pkChr->BeginFight(m_pkChrAttacker);

				if (pkChr->IsPC() && m_dwVnum == SKILL_TUSOK)
					pkChr->AddAffect(m_dwVnum, m_bPointOn, m_iAmount, m_dwAffectFlag, m_iDuration/3, m_iSPCost, m_bOverride);
				else
					pkChr->AddAffect(m_dwVnum, m_bPointOn, m_iAmount, m_dwAffectFlag, m_iDuration, m_iSPCost, m_bOverride);

				m_iCount ++;
			}
		}
	}

	LPCHARACTER m_pkChrAttacker;
	int32_t		m_x;
	int32_t		m_y;
	int32_t		m_iDist;
	uint32_t	m_dwVnum;
	uint8_t	m_bPointOn;
	int32_t		m_iAmount;
	uint32_t	m_dwAffectFlag;
	int32_t		m_iDuration;
	int32_t		m_iSPCost;
	bool	m_bOverride;
	int32_t         m_iMaxHit;
	int32_t         m_iCount;
};

EVENTINFO(skill_gwihwan_info)
{
	uint32_t pid;
	uint8_t bsklv;

	skill_gwihwan_info()
	: pid( 0 )
	, bsklv( 0 )
	{
	}
};

EVENTFUNC(skill_gwihwan_event)
{
	skill_gwihwan_info* info = dynamic_cast<skill_gwihwan_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "skill_gwihwan_event> <Factor> Null pointer" );
		return 0;
	}

	uint32_t pid = info->pid;
	uint8_t sklv= info->bsklv;
	LPCHARACTER ch = CHARACTER_MANAGER::instance().FindByPID(pid);

	if (!ch)
		return 0;

	int32_t percent = 20 * sklv - 1;

	if (number(1, 100) <= percent)
	{
		PIXEL_POSITION pos;

		if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos))
		{
			sys_log(1, "Recall: %s %d %d -> %d %d", ch->GetName(), ch->GetX(), ch->GetY(), pos.x, pos.y);
			ch->WarpSet(pos.x, pos.y);
		}
		else
		{
			sys_err("CHARACTER::UseItem : cannot find spawn position (name %s, %d x %d)", ch->GetName(), ch->GetX(), ch->GetY());
			ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("귀환에 실패하였습니다."));
	}
	return 0;
}

int32_t CHARACTER::ComputeSkillAtPosition(uint32_t dwVnum, const PIXEL_POSITION& posTarget, uint8_t bSkillLevel)
{
	if (GetMountVnum())
		return BATTLE_NONE;

	if (IsPolymorphed())
		return BATTLE_NONE;

	if (g_bSkillDisable)
		return BATTLE_NONE;

	CSkillProto * pkSk = CSkillManager::instance().Get(dwVnum);

	if (!pkSk)
		return BATTLE_NONE;

	if (test_server)
	{
		sys_log(0, "ComputeSkillAtPosition %s vnum %d x %d y %d level %d",
				GetName(), dwVnum, posTarget.x, posTarget.y, bSkillLevel);
	}


	if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
		return BATTLE_NONE;

	if (0 == bSkillLevel)
	{
		if ((bSkillLevel = GetSkillLevel(pkSk->dwVnum)) == 0)
		{
			return BATTLE_NONE;
		}
	}

	const float k = 1.0 * GetSkillPower(pkSk->dwVnum, bSkillLevel) * pkSk->bMaxLevel / 100;

	pkSk->SetPointVar("k", k);
	pkSk->kSplashAroundDamageAdjustPoly.SetVar("k", k);

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_MELEE_DAMAGE))
	{
		pkSk->SetPointVar("atk", CalcMeleeDamage(this, this, true, false));
	}
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_MAGIC_DAMAGE))
	{
		pkSk->SetPointVar("atk", CalcMagicDamage(this, this));
	}
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_ARROW_DAMAGE))
	{
		LPITEM pkBow, pkArrow;
		if (1 == GetArrowAndBow(&pkBow, &pkArrow, 1))
		{
			pkSk->SetPointVar("atk", CalcArrowDamage(this, this, pkBow, pkArrow, true));
		}
		else
		{
			pkSk->SetPointVar("atk", 0);
		}
	}

	if (pkSk->bPointOn == POINT_MOV_SPEED)
	{
		pkSk->SetPointVar("maxv", this->GetLimitPoint(POINT_MOV_SPEED));
	}

	pkSk->SetPointVar("lv", GetLevel());
	pkSk->SetPointVar("iq", GetPoint(POINT_IQ));
	pkSk->SetPointVar("str", GetPoint(POINT_ST));
	pkSk->SetPointVar("dex", GetPoint(POINT_DX));
	pkSk->SetPointVar("con", GetPoint(POINT_HT));
	pkSk->SetPointVar("maxhp", this->GetMaxHP());
	pkSk->SetPointVar("maxsp", this->GetMaxSP());
	pkSk->SetPointVar("chain", 0);
	pkSk->SetPointVar("ar", CalcAttackRating(this, this));
	pkSk->SetPointVar("def", GetPoint(POINT_DEF_GRADE));
	pkSk->SetPointVar("odef", GetPoint(POINT_DEF_GRADE) - GetPoint(POINT_DEF_GRADE_BONUS));
	pkSk->SetPointVar("horse_level", GetHorseLevel());

	if (pkSk->bSkillAttrType != SKILL_ATTR_TYPE_NORMAL)
		OnMove(true);

	LPITEM pkWeapon = GetWear(WEAR_WEAPON);

	SetPolyVarForAttack(this, pkSk, pkWeapon);

	pkSk->SetDurationVar("k", k);

	int32_t iAmount = (int32_t) pkSk->kPointPoly.Eval();
	int32_t iAmount2 = (int32_t) pkSk->kPointPoly2.Eval();

	int32_t iAmount3 = (int32_t) pkSk->kPointPoly3.Eval();

	if (GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER)
	{
		iAmount = (int32_t) pkSk->kMasterBonusPoly.Eval();
	}

	if (test_server && iAmount == 0 && pkSk->bPointOn != POINT_NONE)
	{
		ChatPacket(CHAT_TYPE_INFO, "효과가 없습니다. 스킬 공식을 확인하세요");
	}

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_REMOVE_BAD_AFFECT))
	{
		if (number(1, 100) <= iAmount2)
		{
			RemoveBadAffect();
		}
	}

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_ATTACK | SKILL_FLAG_USE_MELEE_DAMAGE | SKILL_FLAG_USE_MAGIC_DAMAGE))
	{
		bool bAdded = false;

		if (pkSk->bPointOn == POINT_HP && iAmount < 0)
		{
			int32_t iAG = 0;

			FuncSplashDamage f(posTarget.x, posTarget.y, pkSk, this, iAmount, iAG, pkSk->lMaxHit, pkWeapon, m_bDisableCooltime, IsPC()?&m_SkillUseInfo[dwVnum]:nullptr, GetSkillPower(dwVnum, bSkillLevel));

			if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
			{
				if (GetSectree())
					GetSectree()->ForEachAround(f);
			}
			else
			{
				f(this);
			}
		}
		else
		{
			int32_t iDur = (int32_t) pkSk->kDurationPoly.Eval();

			if (IsPC())
				if (!(dwVnum >= GUILD_SKILL_START && dwVnum <= GUILD_SKILL_END))
					if (!m_bDisableCooltime && !m_SkillUseInfo[dwVnum].HitOnce(dwVnum) && dwVnum != SKILL_MUYEONG)
					{
						return BATTLE_NONE;
					}


			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					AddAffect(pkSk->dwVnum, pkSk->bPointOn, iAmount, pkSk->dwAffectFlag, iDur, 0, true);
				else
				{
					if (GetSectree())
					{
						FuncSplashAffect f(this, posTarget.x, posTarget.y, pkSk->iSplashRange, pkSk->dwVnum, pkSk->bPointOn, iAmount, pkSk->dwAffectFlag, iDur, 0, true, pkSk->lMaxHit);
						GetSectree()->ForEachAround(f);
					}
				}
				bAdded = true;
			}
		}

		if (pkSk->bPointOn2 != POINT_NONE)
		{
			int32_t iDur = (int32_t) pkSk->kDurationPoly2.Eval();

			sys_log(1, "try second %u %d %d", pkSk->dwVnum, pkSk->bPointOn2, iDur);

			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					AddAffect(pkSk->dwVnum, pkSk->bPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur, 0, !bAdded);
				else
				{
					if (GetSectree())
					{
						FuncSplashAffect f(this, posTarget.x, posTarget.y, pkSk->iSplashRange, pkSk->dwVnum, pkSk->bPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur, 0, !bAdded, pkSk->lMaxHit);
						GetSectree()->ForEachAround(f);
					}
				}
				bAdded = true;
			}
			else
			{
				PointChange(pkSk->bPointOn2, iAmount2);
			}
		}

		if (GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER && pkSk->bPointOn3 != POINT_NONE)
		{
			int32_t iDur = (int32_t) pkSk->kDurationPoly3.Eval();

			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					AddAffect(pkSk->dwVnum, pkSk->bPointOn3, iAmount3, 0, iDur, 0, !bAdded);
				else
				{
					if (GetSectree())
					{
						FuncSplashAffect f(this, posTarget.x, posTarget.y, pkSk->iSplashRange, pkSk->dwVnum, pkSk->bPointOn3, iAmount3, 0, iDur, 0, !bAdded, pkSk->lMaxHit);
						GetSectree()->ForEachAround(f);
					}
				}
			}
			else
			{
				PointChange(pkSk->bPointOn3, iAmount3);
			}
		}

		return BATTLE_DAMAGE;
	}
	else
	{
		bool bAdded = false;
		int32_t iDur = (int32_t) pkSk->kDurationPoly.Eval();

		if (iDur > 0)
		{
			iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
			pkSk->kDurationSPCostPoly.SetVar("k", k);

			AddAffect(pkSk->dwVnum,
					  pkSk->bPointOn,
					  iAmount,
					  pkSk->dwAffectFlag,
					  iDur,
					  (int32_t) pkSk->kDurationSPCostPoly.Eval(),
					  !bAdded);

			bAdded = true;
		}
		else
		{
			PointChange(pkSk->bPointOn, iAmount);
		}

		if (pkSk->bPointOn2 != POINT_NONE)
		{
			int32_t iDur = (int32_t) pkSk->kDurationPoly2.Eval();

			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
				AddAffect(pkSk->dwVnum, pkSk->bPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur, 0, !bAdded);
				bAdded = true;
			}
			else
			{
				PointChange(pkSk->bPointOn2, iAmount2);
			}
		}

		if (GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER && pkSk->bPointOn3 != POINT_NONE)
		{
			int32_t iDur = (int32_t) pkSk->kDurationPoly3.Eval();

			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
				AddAffect(pkSk->dwVnum, pkSk->bPointOn3, iAmount3, 0, iDur, 0, !bAdded);
			}
			else
			{
				PointChange(pkSk->bPointOn3, iAmount3);
			}
		}

		return BATTLE_NONE;
	}
}

#if defined(ENABLE_WOLFMAN_CHARACTER) || defined(ENABLE_PARTY_FLAG) // @correction116 | @correction190
struct FComputeSkillParty
{
	FComputeSkillParty(uint32_t dwVnum, LPCHARACTER pkAttacker, uint8_t bSkillLevel = 0)
		: m_dwVnum(dwVnum), m_pkAttacker(pkAttacker), m_bSkillLevel(bSkillLevel)
		{
		}

	void operator () (LPCHARACTER ch)
	{
		m_pkAttacker->ComputeSkill(m_dwVnum, ch, m_bSkillLevel);
	}

	uint32_t m_dwVnum;
	LPCHARACTER m_pkAttacker;
	uint8_t m_bSkillLevel;
};

int32_t CHARACTER::ComputeSkillParty(uint32_t dwVnum, LPCHARACTER pkVictim, uint8_t bSkillLevel)
{
	FComputeSkillParty f(dwVnum, pkVictim, bSkillLevel);
	if (GetParty() && GetParty()->GetNearMemberCount())
		GetParty()->ForEachNearMember(f);
	else
		f(this);

	return BATTLE_NONE;
}
#endif

int32_t CHARACTER::ComputeSkill(uint32_t dwVnum, LPCHARACTER pkVictim, uint8_t bSkillLevel)
{
	const bool bCanUseHorseSkill = CanUseHorseSkill();

	if (dwVnum != SKILL_MUYEONG && false == bCanUseHorseSkill && true == IsRiding()) // @correction026
		return BATTLE_NONE;

	if (IsPolymorphed())
		return BATTLE_NONE;

	if (g_bSkillDisable)
		return BATTLE_NONE;

	CSkillProto* pkSk = CSkillManager::instance().Get(dwVnum);

	if (!pkSk)
		return BATTLE_NONE;

	if (dwVnum != SKILL_MUYEONG && bCanUseHorseSkill && pkSk->dwType != SKILL_TYPE_HORSE) // @correction026
		return BATTLE_NONE;

	if (dwVnum != SKILL_MUYEONG && !bCanUseHorseSkill && pkSk->dwType == SKILL_TYPE_HORSE) // @correction026
		return BATTLE_NONE;


	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY))
		pkVictim = this;
#if defined(ENABLE_WOLFMAN_CHARACTER) || defined(ENABLE_PARTY_FLAG) // @correction116 | @correction190
	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY) && !GetParty() && !pkVictim)
		pkVictim = this;
#endif

	if (!pkVictim)
	{
		if (test_server)
			sys_log(0, "ComputeSkill: %s Victim == null, skill %d", GetName(), dwVnum);

		return BATTLE_NONE;
	}

	if (pkSk->dwTargetRange && DISTANCE_SQRT(GetX() - pkVictim->GetX(), GetY() - pkVictim->GetY()) >= pkSk->dwTargetRange + 50)
	{
		if (test_server)
			sys_log(0, "ComputeSkill: Victim too far, skill %d : %s to %s (distance %u limit %u)",
					dwVnum,
					GetName(),
					pkVictim->GetName(),
					(int32_t)DISTANCE_SQRT(GetX() - pkVictim->GetX(), GetY() - pkVictim->GetY()),
					pkSk->dwTargetRange);

		return BATTLE_NONE;
	}

	if (0 == bSkillLevel)
	{
		if ((bSkillLevel = GetSkillLevel(pkSk->dwVnum)) == 0)
		{
			if (test_server)
				sys_log(0, "ComputeSkill : name:%s vnum:%d  skillLevelBySkill : %d ", GetName(), pkSk->dwVnum, bSkillLevel);
			return BATTLE_NONE;
		}
	}

	if (pkVictim->IsAffectFlag(AFF_PABEOP) && pkVictim->IsGoodAffect(dwVnum))
	{
		return BATTLE_NONE;
	}

	const float k = 1.0 * GetSkillPower(pkSk->dwVnum, bSkillLevel) * pkSk->bMaxLevel / 100;

	pkSk->SetPointVar("k", k);
	pkSk->kSplashAroundDamageAdjustPoly.SetVar("k", k);

	if (pkSk->dwType == SKILL_TYPE_HORSE)
	{
		LPITEM pkBow, pkArrow;
		if (1 == GetArrowAndBow(&pkBow, &pkArrow, 1))
		{
			pkSk->SetPointVar("atk", CalcArrowDamage(this, pkVictim, pkBow, pkArrow, true));
		}
		else
		{
			pkSk->SetPointVar("atk", CalcMeleeDamage(this, pkVictim, true, false));
		}
	}
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_MELEE_DAMAGE))
	{
		pkSk->SetPointVar("atk", CalcMeleeDamage(this, pkVictim, true, false));
	}
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_MAGIC_DAMAGE))
	{
		pkSk->SetPointVar("atk", CalcMagicDamage(this, pkVictim));
	}
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_ARROW_DAMAGE))
	{
		LPITEM pkBow, pkArrow;
		if (1 == GetArrowAndBow(&pkBow, &pkArrow, 1))
		{
			pkSk->SetPointVar("atk", CalcArrowDamage(this, pkVictim, pkBow, pkArrow, true));
		}
		else
		{
			pkSk->SetPointVar("atk", 0);
		}
	}

	if (pkSk->bPointOn == POINT_MOV_SPEED)
	{
		pkSk->SetPointVar("maxv", pkVictim->GetLimitPoint(POINT_MOV_SPEED));
	}

	pkSk->SetPointVar("lv", GetLevel());
	pkSk->SetPointVar("iq", GetPoint(POINT_IQ));
	pkSk->SetPointVar("str", GetPoint(POINT_ST));
	pkSk->SetPointVar("dex", GetPoint(POINT_DX));
	pkSk->SetPointVar("con", GetPoint(POINT_HT));
	pkSk->SetPointVar("maxhp", pkVictim->GetMaxHP());
	pkSk->SetPointVar("maxsp", pkVictim->GetMaxSP());
	pkSk->SetPointVar("chain", 0);
	pkSk->SetPointVar("ar", CalcAttackRating(this, pkVictim));
	pkSk->SetPointVar("def", GetPoint(POINT_DEF_GRADE));
	pkSk->SetPointVar("odef", GetPoint(POINT_DEF_GRADE) - GetPoint(POINT_DEF_GRADE_BONUS));
	pkSk->SetPointVar("horse_level", GetHorseLevel());

	if (pkSk->bSkillAttrType != SKILL_ATTR_TYPE_NORMAL)
		OnMove(true);

	LPITEM pkWeapon = GetWear(WEAR_WEAPON);

	SetPolyVarForAttack(this, pkSk, pkWeapon);

	pkSk->kDurationPoly.SetVar("k", k);
	pkSk->kDurationPoly2.SetVar("k", k);

	int32_t iAmount = (int32_t) pkSk->kPointPoly.Eval();
	int32_t iAmount2 = (int32_t) pkSk->kPointPoly2.Eval();
	int32_t iAmount3 = (int32_t) pkSk->kPointPoly3.Eval();

	if (test_server && IsPC())
		sys_log(0, "iAmount: %d %d %d , atk:%f skLevel:%f k:%f GetSkillPower(%d) MaxLevel:%d Per:%f",
				iAmount, iAmount2, iAmount3,
				pkSk->kPointPoly.GetVar("atk"),
				pkSk->kPointPoly.GetVar("k"),
				k,
				GetSkillPower(pkSk->dwVnum, bSkillLevel),
				pkSk->bMaxLevel,
				pkSk->bMaxLevel/100
				);

	if (GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER)
	{
		iAmount = (int32_t) pkSk->kMasterBonusPoly.Eval();
	}

	if (test_server && iAmount == 0 && pkSk->bPointOn != POINT_NONE)
	{
		ChatPacket(CHAT_TYPE_INFO, "효과가 없습니다. 스킬 공식을 확인하세요");
	}


	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_REMOVE_BAD_AFFECT))
	{
		if (number(1, 100) <= iAmount2)
		{
			pkVictim->RemoveBadAffect();
		}
	}

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_ATTACK | SKILL_FLAG_USE_MELEE_DAMAGE | SKILL_FLAG_USE_MAGIC_DAMAGE) &&
		!(pkSk->dwVnum == SKILL_MUYEONG && pkVictim == this) && !(pkSk->IsChargeSkill() && pkVictim == this))
	{
		bool bAdded = false;

		if (pkSk->bPointOn == POINT_HP && iAmount < 0)
		{
			int32_t iAG = 0;


			FuncSplashDamage f(pkVictim->GetX(), pkVictim->GetY(), pkSk, this, iAmount, iAG, pkSk->lMaxHit, pkWeapon, m_bDisableCooltime, IsPC()?&m_SkillUseInfo[dwVnum]:nullptr, GetSkillPower(dwVnum, bSkillLevel));
			if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
			{
				if (pkVictim->GetSectree())
					pkVictim->GetSectree()->ForEachAround(f);
			}
			else
			{
				f(pkVictim);
			}
		}
		else
		{
			pkSk->kDurationPoly.SetVar("k", k);
			int32_t iDur = (int32_t) pkSk->kDurationPoly.Eval();


			if (IsPC())
				if (!(dwVnum >= GUILD_SKILL_START && dwVnum <= GUILD_SKILL_END))
					if (!m_bDisableCooltime && !m_SkillUseInfo[dwVnum].HitOnce(dwVnum) && dwVnum != SKILL_MUYEONG)
					{
						return BATTLE_NONE;
					}

			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn, iAmount, pkSk->dwAffectFlag, iDur, 0, true);
				else
				{
					if (pkVictim->GetSectree())
					{
						FuncSplashAffect f(this, pkVictim->GetX(), pkVictim->GetY(), pkSk->iSplashRange, pkSk->dwVnum, pkSk->bPointOn, iAmount, pkSk->dwAffectFlag, iDur, 0, true, pkSk->lMaxHit);
						pkVictim->GetSectree()->ForEachAround(f);
					}
				}
				bAdded = true;
			}
		}

		if (pkSk->bPointOn2 != POINT_NONE && !pkSk->IsChargeSkill())
		{
			pkSk->kDurationPoly2.SetVar("k", k);
			int32_t iDur = (int32_t) pkSk->kDurationPoly2.Eval();

			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur, 0, !bAdded);
				else
				{
					if (pkVictim->GetSectree())
					{
						FuncSplashAffect f(this, pkVictim->GetX(), pkVictim->GetY(), pkSk->iSplashRange, pkSk->dwVnum, pkSk->bPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur, 0, !bAdded, pkSk->lMaxHit);
						pkVictim->GetSectree()->ForEachAround(f);
					}
				}

				bAdded = true;
			}
			else
			{
				pkVictim->PointChange(pkSk->bPointOn2, iAmount2);
			}
		}

		if (pkSk->bPointOn3 != POINT_NONE && !pkSk->IsChargeSkill() && GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER)
		{
			pkSk->kDurationPoly3.SetVar("k", k);
			int32_t iDur = (int32_t) pkSk->kDurationPoly3.Eval();


			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn3, iAmount3,0, iDur, 0, !bAdded);
				else
				{
					if (pkVictim->GetSectree())
					{
						FuncSplashAffect f(this, pkVictim->GetX(), pkVictim->GetY(), pkSk->iSplashRange, pkSk->dwVnum, pkSk->bPointOn3, iAmount3, 0, iDur, 0, !bAdded, pkSk->lMaxHit);
						pkVictim->GetSectree()->ForEachAround(f);
					}
				}

				bAdded = true;
			}
			else
			{
				pkVictim->PointChange(pkSk->bPointOn3, iAmount3);
			}
		}

		return BATTLE_DAMAGE;
	}
	else
	{
		if (dwVnum == SKILL_MUYEONG)
		{
			pkSk->kDurationPoly.SetVar("k", k);
			pkSk->kDurationSPCostPoly.SetVar("k", k);

			int32_t iDur = (int32_t) pkSk->kDurationPoly.Eval();
			iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

			if (pkVictim == this)
				AddAffect(dwVnum,
						POINT_NONE, 0,
						AFF_MUYEONG,
						iDur,
						(int32_t) pkSk->kDurationSPCostPoly.Eval(),
						true);

			return BATTLE_NONE;
		}

		bool bAdded = false;
		pkSk->kDurationPoly.SetVar("k", k);
		int32_t iDur = (int32_t) pkSk->kDurationPoly.Eval();

		if (iDur > 0)
		{
			iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);
			pkSk->kDurationSPCostPoly.SetVar("k", k);

			if (pkSk->bPointOn2 != POINT_NONE)
			{
				pkVictim->RemoveAffect(pkSk->dwVnum);

				int32_t iDur2 = (int32_t) pkSk->kDurationPoly2.Eval();

				if (iDur2 > 0)
				{
					if (test_server)
						sys_log(0, "SKILL_AFFECT: %s %s Dur:%d To:%d Amount:%d",
								GetName(),
								pkSk->szName,
								iDur2,
								pkSk->bPointOn2,
								iAmount2);

					iDur2 += GetPoint(POINT_PARTY_BUFFER_BONUS);
					pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur2, 0, false);
				}
				else
				{
					pkVictim->PointChange(pkSk->bPointOn2, iAmount2);
				}

				uint32_t affact_flag = pkSk->dwAffectFlag;

				// @correction005
				if ((pkSk->dwVnum == SKILL_CHUNKEON && GetUsedSkillMasterType(pkSk->dwVnum) < SKILL_GRAND_MASTER))
					affact_flag = AFF_CHEONGEUN_WITH_FALL;

				pkVictim->AddAffect(pkSk->dwVnum,
						pkSk->bPointOn,
						iAmount,
						affact_flag,
						iDur,
						(int32_t) pkSk->kDurationSPCostPoly.Eval(),
						false);
			}
			else
			{
				if (test_server)
					sys_log(0, "SKILL_AFFECT: %s %s Dur:%d To:%d Amount:%d",
							GetName(),
							pkSk->szName,
							iDur,
							pkSk->bPointOn,
							iAmount);

				pkVictim->AddAffect(pkSk->dwVnum,
						pkSk->bPointOn,
						iAmount,
						pkSk->dwAffectFlag,
						iDur,
						(int32_t) pkSk->kDurationSPCostPoly.Eval(),
						!bAdded);
			}

			bAdded = true;
		}
		else
		{
			if (!pkSk->IsChargeSkill())
				pkVictim->PointChange(pkSk->bPointOn, iAmount);

			if (pkSk->bPointOn2 != POINT_NONE)
			{
				pkVictim->RemoveAffect(pkSk->dwVnum);

				int32_t iDur2 = (int32_t) pkSk->kDurationPoly2.Eval();

				if (iDur2 > 0)
				{
					iDur2 += GetPoint(POINT_PARTY_BUFFER_BONUS);

					if (pkSk->IsChargeSkill())
						pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn2, iAmount2, AFF_TANHWAN_DASH, iDur2, 0, false);
					else
						pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn2, iAmount2, pkSk->dwAffectFlag2, iDur2, 0, false);
				}
				else
				{
					pkVictim->PointChange(pkSk->bPointOn2, iAmount2);
				}

			}
		}

		if (pkSk->bPointOn3 != POINT_NONE && !pkSk->IsChargeSkill() && GetUsedSkillMasterType(pkSk->dwVnum) >= SKILL_GRAND_MASTER)
		{

			pkSk->kDurationPoly3.SetVar("k", k);
			int32_t iDur = (int32_t) pkSk->kDurationPoly3.Eval();

			sys_log(0, "try third %u %d %d %d 1894", pkSk->dwVnum, pkSk->bPointOn3, iDur, iAmount3);

			if (iDur > 0)
			{
				iDur += GetPoint(POINT_PARTY_BUFFER_BONUS);

				if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_SPLASH))
					pkVictim->AddAffect(pkSk->dwVnum, pkSk->bPointOn3, iAmount3, 0, iDur, 0, !bAdded);
				else
				{
					if (pkVictim->GetSectree())
					{
						FuncSplashAffect f(this, pkVictim->GetX(), pkVictim->GetY(), pkSk->iSplashRange, pkSk->dwVnum, pkSk->bPointOn3, iAmount3, 0, iDur, 0, !bAdded, pkSk->lMaxHit);
						pkVictim->GetSectree()->ForEachAround(f);
					}
				}

				bAdded = true;
			}
			else
			{
				pkVictim->PointChange(pkSk->bPointOn3, iAmount3);
			}
		}

		return BATTLE_NONE;
	}
}

bool CHARACTER::UseSkill(uint32_t dwVnum, LPCHARACTER pkVictim, bool bUseGrandMaster)
{
	if (pkVictim && pkVictim->IsPet())
		return false;

#ifdef __MOUNT__
	if (pkVictim && pkVictim->IsMountSystem())
		return false;
#endif

#ifdef __SUPPORT__
	if (pkVictim && pkVictim->IsSupport())
		return false;
#endif

#ifdef __GROWTH_PET__
	if (pkVictim && pkVictim->IsNewPet())
		return false;
#endif

	// @correction082 BEGIN
	if ((dwVnum == SKILL_GEOMKYUNG || dwVnum == SKILL_GWIGEOM) && !GetWear(WEAR_WEAPON))
		return false;
	// @correction082 END

	if (false == CanUseSkill(dwVnum))
		return false;

	if (test_server)
	{
		if (quest::CQuestManager::instance().GetEventFlag("no_grand_master"))
		{
			bUseGrandMaster = false;
		}
	}

	if (g_bSkillDisable)
		return false;

	if (IsObserverMode())
		return false;

	if (!CanMove())
		return false;

	if (IsPolymorphed())
		return false;

	const bool bCanUseHorseSkill = CanUseHorseSkill();


	if (dwVnum == SKILL_HORSE_SUMMON)
	{
		if (GetSkillLevel(dwVnum) == 0)
			return false;

		if (GetHorseLevel() <= 0)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말이 없습니다. 마굿간 경비병을 찾아가세요."));
		else
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("말 소환 아이템을 사용하세요."));

		return true;
	}

	if (false == bCanUseHorseSkill && true == IsRiding())
		return false;

	CSkillProto * pkSk = CSkillManager::instance().Get(dwVnum);
	sys_log(0, "%s: USE_SKILL: %d pkVictim %p", GetName(), dwVnum, get_pointer(pkVictim));

	if (!pkSk)
		return false;

	if (bCanUseHorseSkill && pkSk->dwType != SKILL_TYPE_HORSE)
		return BATTLE_NONE;

	if (!bCanUseHorseSkill && pkSk->dwType == SKILL_TYPE_HORSE)
		return BATTLE_NONE;

	if (GetSkillLevel(dwVnum) == 0)
		return false;


	if (GetSkillMasterType(dwVnum) < SKILL_GRAND_MASTER)
		bUseGrandMaster = false;

	if (GetWear(WEAR_WEAPON) && (GetWear(WEAR_WEAPON)->GetType() == ITEM_ROD || GetWear(WEAR_WEAPON)->GetType() == ITEM_PICK))
		return false;

	m_SkillUseInfo[dwVnum].TargetVIDMap.clear();

	if (pkSk->IsChargeSkill())
	{
		if ((IsAffectFlag(AFF_TANHWAN_DASH)) || (pkVictim && (pkVictim != this)))
		{
			if (!pkVictim)
				return false;

			if (!IsAffectFlag(AFF_TANHWAN_DASH))
			{
				if (!UseSkill(dwVnum, this))
					return false;
			}

			m_SkillUseInfo[dwVnum].SetMainTargetVID(pkVictim->GetVID());
			ComputeSkill(dwVnum, pkVictim);
			RemoveAffect(dwVnum);
			return true;
		}
	}

	if (dwVnum == SKILL_COMBO)
	{
		if (m_bComboIndex)
			m_bComboIndex = 0;
		else
			m_bComboIndex = GetSkillLevel(SKILL_COMBO);

		ChatPacket(CHAT_TYPE_COMMAND, "combo %d", m_bComboIndex);
		return true;
	}

	if ((0 != pkSk->dwAffectFlag || pkSk->dwVnum == SKILL_MUYEONG) && (pkSk->dwFlag & SKILL_FLAG_TOGGLE) && RemoveAffect(pkSk->dwVnum))
	{
		return true;
	}

	if (IsAffectFlag(AFF_REVIVE_INVISIBLE))
		RemoveAffect(AFFECT_REVIVE_INVISIBLE);

	const float k = 1.0 * GetSkillPower(pkSk->dwVnum) * pkSk->bMaxLevel / 100;

	pkSk->SetPointVar("k", k);
	pkSk->kSplashAroundDamageAdjustPoly.SetVar("k", k);

	pkSk->kCooldownPoly.SetVar("k", k);
	int32_t iCooltime = (int32_t) pkSk->kCooldownPoly.Eval();
	int32_t lMaxHit = pkSk->lMaxHit ? pkSk->lMaxHit : -1;

	pkSk->SetSPCostVar("k", k);

	uint32_t dwCur = get_dword_time();

	if (dwVnum == SKILL_TERROR && m_SkillUseInfo[dwVnum].bUsed && m_SkillUseInfo[dwVnum].dwNextSkillUsableTime > dwCur )
	{
		sys_log(0, " SKILL_TERROR's Cooltime is not delta over %u", m_SkillUseInfo[dwVnum].dwNextSkillUsableTime  - dwCur );
		return false;
	}

	int32_t iNeededSP = 0;

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_USE_HP_AS_COST))
	{
		pkSk->SetSPCostVar("maxhp", GetMaxHP());
		pkSk->SetSPCostVar("v", GetHP());
		iNeededSP = (int32_t) pkSk->kSPCostPoly.Eval();

		if (GetSkillMasterType(dwVnum) >= SKILL_GRAND_MASTER && bUseGrandMaster)
		{
			iNeededSP = (int32_t) pkSk->kGrandMasterAddSPCostPoly.Eval();
		}

		if (GetHP() < iNeededSP)
			return false;

		PointChange(POINT_HP, -iNeededSP);
	}
	else
	{
		pkSk->SetSPCostVar("maxhp", GetMaxHP());
		pkSk->SetSPCostVar("maxv", GetMaxSP());
		pkSk->SetSPCostVar("v", GetSP());

		iNeededSP = (int32_t) pkSk->kSPCostPoly.Eval();

		if (GetSkillMasterType(dwVnum) >= SKILL_GRAND_MASTER && bUseGrandMaster)
		{
			iNeededSP = (int32_t) pkSk->kGrandMasterAddSPCostPoly.Eval();
		}

		if (GetSP() < iNeededSP)
			return false;

		if (test_server)
			ChatPacket(CHAT_TYPE_INFO, LC_TEXT("%s SP소모: %d"), pkSk->szName, iNeededSP);

		PointChange(POINT_SP, -iNeededSP);
	}

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY))
		pkVictim = this;
#if defined(ENABLE_WOLFMAN_CHARACTER) || defined(ENABLE_PARTY_FLAG) // @correction116 | @correction190
	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY) && !GetParty() && !pkVictim)
		pkVictim = this;
#endif

	if ((pkSk->dwVnum == SKILL_MUYEONG) || (pkSk->IsChargeSkill() && !IsAffectFlag(AFF_TANHWAN_DASH) && !pkVictim))
	{
		pkVictim = this;
	}

	int32_t iSplashCount = 1;

	if (false == m_bDisableCooltime)
	{
		if (false ==
				m_SkillUseInfo[dwVnum].UseSkill(
					bUseGrandMaster,
				   	(nullptr != pkVictim && SKILL_HORSE_WILDATTACK != dwVnum) ? pkVictim->GetVID() : 0,
				   	ComputeCooltime(iCooltime * 1000),
				   	iSplashCount,
				   	lMaxHit))
		{
			if (test_server)
				ChatPacket(CHAT_TYPE_NOTICE, "cooltime not finished %s %d", pkSk->szName, iCooltime);

			return false;
		}
	}

#if defined(ENABLE_WOLFMAN_CHARACTER) || defined(ENABLE_PARTY_FLAG) // @correction116 | @correction190
	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY) && !GetParty() && !pkVictim)
		ComputeSkill(dwVnum, this);
	else if (IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY) && GetParty())
		ComputeSkillParty(dwVnum, this);
#endif

	if (dwVnum == SKILL_CHAIN)
	{
		ResetChainLightningIndex();
		AddChainLightningExcept(pkVictim);
	}

	if (IS_SET(pkSk->dwFlag, SKILL_FLAG_SELFONLY))
		ComputeSkill(dwVnum, this);
	else if (!IS_SET(pkSk->dwFlag, SKILL_FLAG_ATTACK))
		ComputeSkill(dwVnum, pkVictim);
	else if (dwVnum == SKILL_BYEURAK)
		ComputeSkill(dwVnum, pkVictim);
	else if (dwVnum == SKILL_MUYEONG || pkSk->IsChargeSkill())
		ComputeSkill(dwVnum, pkVictim);

	m_dwLastSkillTime = get_dword_time();

	return true;
}

int32_t CHARACTER::GetUsedSkillMasterType(uint32_t dwVnum)
{
	const TSkillUseInfo& rInfo = m_SkillUseInfo[dwVnum];

	if (GetSkillMasterType(dwVnum) < SKILL_GRAND_MASTER)
		return GetSkillMasterType(dwVnum);

	if (rInfo.isGrandMaster)
		return GetSkillMasterType(dwVnum);

	return MIN(GetSkillMasterType(dwVnum), SKILL_MASTER);
}

int32_t CHARACTER::GetSkillMasterType(uint32_t dwVnum) const
{
	if (!IsPC())
		return 0;

	if (dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("%s skill vnum overflow %u", GetName(), dwVnum);
		return 0;
	}

	return m_pSkillLevels ? m_pSkillLevels[dwVnum].bMasterType : (int32_t)SKILL_NORMAL;
}

int32_t CHARACTER::GetSkillPower(uint32_t dwVnum, uint8_t bLevel) const
{
	if (dwVnum >= SKILL_LANGUAGE1 && dwVnum <= SKILL_LANGUAGE3 && IsEquipUniqueGroup(UNIQUE_GROUP_RING_OF_LANGUAGE))
	{
		return 100;
	}

	if (dwVnum >= GUILD_SKILL_START && dwVnum <= GUILD_SKILL_END)
	{
		if (GetGuild())
			return 100 * GetGuild()->GetSkillLevel(dwVnum) / 7 / 7;
		else
			return 0;
	}

	if (bLevel)
	{
		return GetSkillPowerByLevel(bLevel, true);
	}

	if (dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("%s skill vnum overflow %u", GetName(), dwVnum);
		return 0;
	}

	return GetSkillPowerByLevel(GetSkillLevel(dwVnum));
}

int32_t CHARACTER::GetSkillLevel(uint32_t dwVnum) const
{
	if (dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("%s skill vnum overflow %u", GetName(), dwVnum);
		sys_log(0, "%s skill vnum overflow %u", GetName(), dwVnum);
		return 0;
	}

	return MIN(SKILL_MAX_LEVEL, m_pSkillLevels ? m_pSkillLevels[dwVnum].bLevel : 0);
}

EVENTFUNC(skill_muyoung_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "skill_muyoung_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER	ch = info->ch;

	if (ch == nullptr) {
		return 0;
	}

	if (!ch->IsAffectFlag(AFF_MUYEONG))
	{
		ch->StopMuyeongEvent();
		return 0;
	}

	FFindNearVictim f(ch, ch);
	if (ch->GetSectree())
	{
		ch->GetSectree()->ForEachAround(f);
		if (f.GetVictim())
		{
			ch->CreateFly(FLY_SKILL_MUYEONG, f.GetVictim());
			ch->ComputeSkill(SKILL_MUYEONG, f.GetVictim());
		}
	}

	return PASSES_PER_SEC(3);
}

void CHARACTER::StartMuyeongEvent()
{
	if (m_pkMuyeongEvent)
		return;

	char_event_info* info = AllocEventInfo<char_event_info>();

	info->ch = this;
	m_pkMuyeongEvent = event_create(skill_muyoung_event, info, PASSES_PER_SEC(1));
}

void CHARACTER::StopMuyeongEvent()
{
	event_cancel(&m_pkMuyeongEvent);
}

void CHARACTER::SkillLearnWaitMoreTimeMessage(uint32_t ms)
{
	if (ms < 3 * 60)
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("몸 속이 뜨겁군. 하지만 아주 편안해. 이대로 기를 안정시키자."));
	else if (ms < 5 * 60)
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("그래, 천천히. 좀더 천천히, 그러나 막힘 없이 빠르게!"));
	else if (ms < 10 * 60)
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("그래, 이 느낌이야. 체내에 기가 아주 충만해."));
	else if (ms < 30 * 60)
	{
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("다 읽었다! 이제 비급에 적혀있는 대로 전신에 기를 돌리기만 하면,"));
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("그것으로 수련은 끝난 거야!"));
	}
	else if (ms < 1 * 3600)
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("이제 책의 마지막 장이야! 수련의 끝이 눈에 보이고 있어!"));
	else if (ms < 2 * 3600)
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("얼마 안 남았어! 조금만 더!"));
	else if (ms < 3 * 3600)
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("좋았어! 조금만 더 읽으면 끝이다!"));
	else if (ms < 6 * 3600)
	{
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("책장도 이제 얼마 남지 않았군."));
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("뭔가 몸 안에 힘이 생기는 기분인 걸."));
	}
	else if (ms < 12 * 3600)
	{
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("이제 좀 슬슬 가닥이 잡히는 것 같은데."));
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("좋아, 이 기세로 계속 나간다!"));
	}
	else if (ms < 18 * 3600)
	{
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("아니 어떻게 된 게 종일 읽어도 머리에 안 들어오냐."));
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("공부하기 싫어지네."));
	}
	else
	{
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("생각만큼 읽기가 쉽지가 않군. 이해도 어렵고 내용도 난해해."));
		ChatPacket(CHAT_TYPE_TALKING, "%s", LC_TEXT("이래서야 공부가 안된다구."));
	}
}

void CHARACTER::DisableCooltime()
{
	m_bDisableCooltime = true;
}

bool CHARACTER::HasMobSkill() const
{
	return CountMobSkill() > 0;
}

size_t CHARACTER::CountMobSkill() const
{
	if (!m_pkMobData)
		return 0;

	size_t c = 0;

	for (size_t i = 0; i < MOB_SKILL_MAX_NUM; ++i)
		if (m_pkMobData->m_table.Skills[i].dwVnum)
			++c;

	return c;
}

const TMobSkillInfo* CHARACTER::GetMobSkill(uint32_t idx) const
{
	if (idx >= MOB_SKILL_MAX_NUM)
		return nullptr;

	if (!m_pkMobData)
		return nullptr;

	if (0 == m_pkMobData->m_table.Skills[idx].dwVnum)
		return nullptr;

	return &m_pkMobData->m_mobSkillInfo[idx];
}

bool CHARACTER::CanUseMobSkill(uint32_t idx) const
{
	const TMobSkillInfo* pInfo = GetMobSkill(idx);

	if (!pInfo)
		return false;

	if (m_adwMobSkillCooltime[idx] > get_dword_time())
		return false;

	if (number(0, 1))
		return false;

	return true;
}

EVENTINFO(mob_skill_event_info)
{
	DynamicCharacterPtr ch;
	PIXEL_POSITION pos;
	uint32_t vnum;
	int32_t index;
	uint8_t level;

	mob_skill_event_info()
	: ch()
	, pos()
	, vnum(0)
	, index(0)
	, level(0)
	{
	}
};

EVENTFUNC(mob_skill_hit_event)
{
	mob_skill_event_info * info = dynamic_cast<mob_skill_event_info *>( event->info );

	if ( info == nullptr )
	{
		sys_err( "mob_skill_event_info> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = info->ch;
	if (ch == nullptr) {
		return 0;
	}

	ch->ComputeSkillAtPosition(info->vnum, info->pos, info->level);
	ch->m_mapMobSkillEvent.erase(info->index);

	return 0;
}

#ifdef __TEMPLE_OCHAO__
struct FHealerParty
{
	FHealerParty(LPCHARACTER pkHealer) : m_pkHealer(pkHealer) {}
	
	void operator () (LPCHARACTER ch)
	{
		int32_t iRevive = (int32_t)(m_pkHealer->GetMaxHP() / 100 * 15);
		int32_t iHP = (ch->GetMaxHP() >= ch->GetHP() + iRevive) ? (int32_t)(ch->GetHP() + iRevive) : (int32_t)(ch->GetMaxHP());
		ch->SetHP(iHP);
		ch->EffectPacket(SE_EFFECT_HEALER);
		sys_log(0, "FHealerParty: %s (pointer: %p) heal the HP of %s (pointer: %p) with %d (new HP: %d).", m_pkHealer->GetName(), get_pointer(m_pkHealer), ch->GetName(), get_pointer(ch), iRevive, ch->GetHP());
	}
	
	LPCHARACTER	m_pkHealer;
};
#endif

bool CHARACTER::UseMobSkill(uint32_t idx)
{
	if (IsPC())
		return false;

	const TMobSkillInfo* pInfo = GetMobSkill(idx);

	if (!pInfo)
		return false;

	uint32_t dwVnum = pInfo->dwSkillVnum;
	CSkillProto * pkSk = CSkillManager::instance().Get(dwVnum);

	if (!pkSk)
		return false;

	const float k = 1.0 * GetSkillPower(pkSk->dwVnum, pInfo->bSkillLevel) * pkSk->bMaxLevel / 100;

	pkSk->kCooldownPoly.SetVar("k", k);
	int32_t iCooltime = (int32_t) (pkSk->kCooldownPoly.Eval() * 1000);

	m_adwMobSkillCooltime[idx] = get_dword_time() + iCooltime;

	sys_log(0, "USE_MOB_SKILL: %s idx %d vnum %u cooltime %d", GetName(), idx, dwVnum, iCooltime);

#ifdef __TEMPLE_OCHAO__
	#define HEALING_SKILL_VNUM 265
	if ((IsMonster()) && (pkSk->dwVnum == HEALING_SKILL_VNUM))
	{
		LPPARTY pkParty = GetParty();
		if ((pkParty) && (IS_SET(pkSk->dwFlag, SKILL_FLAG_PARTY)))
		{
			FHealerParty f(this);
			pkParty->ForEachMemberPtr(f);
		}
		else
		{
			int32_t iRevive = (int32_t)(GetMaxHP() / 100 * 15);
			int32_t iHP = (GetMaxHP() >= GetHP() + iRevive) ? (int32_t)(GetHP() + iRevive) : (int32_t)(GetMaxHP());
			SetHP(iHP);
			EffectPacket(SE_EFFECT_HEALER);
			sys_log(0, "FHealer: %s (pointer: %p) heal their HP with %d (new HP: %d).", GetName(), get_pointer(this), iRevive, GetHP());
		}
		
		return true;
	}
#endif

	if (m_pkMobData->m_mobSkillInfo[idx].vecSplashAttack.empty())
	{
		sys_err("No skill hit data for mob %s index %d", GetName(), idx);
		return false;
	}

	for (size_t i = 0; i < m_pkMobData->m_mobSkillInfo[idx].vecSplashAttack.size(); i++)
	{
		PIXEL_POSITION pos = GetXYZ();
		const TMobSplashAttackInfo& rInfo = m_pkMobData->m_mobSkillInfo[idx].vecSplashAttack[i];

		if (rInfo.dwHitDistance)
		{
			float fx, fy;
			GetDeltaByDegree(GetRotation(), rInfo.dwHitDistance, &fx, &fy);
			pos.x += (int32_t) fx;
			pos.y += (int32_t) fy;
		}

		if (rInfo.dwTiming)
		{
			if (test_server)
				sys_log(0, "               timing %ums", rInfo.dwTiming);

			mob_skill_event_info* info = AllocEventInfo<mob_skill_event_info>();

			info->ch = this;
			info->pos = pos;
			info->level = pInfo->bSkillLevel;
			info->vnum = dwVnum;
			info->index = i;

			itertype(m_mapMobSkillEvent) it = m_mapMobSkillEvent.find(i);
			if (it != m_mapMobSkillEvent.end()) {
				LPEVENT existing = it->second;
				event_cancel(&existing);
				m_mapMobSkillEvent.erase(it);
			}

			m_mapMobSkillEvent.insert(std::make_pair(i, event_create(mob_skill_hit_event, info, PASSES_PER_SEC(rInfo.dwTiming) / 1000)));
		}
		else
		{
			ComputeSkillAtPosition(dwVnum, pos, pInfo->bSkillLevel);
		}
	}

	return true;
}

void CHARACTER::ResetMobSkillCooltime()
{
	memset(m_adwMobSkillCooltime, 0, sizeof(m_adwMobSkillCooltime));
}

bool CHARACTER::IsUsableSkillMotion(uint32_t dwMotionIndex) const
{
	uint32_t selfJobGroup = (GetJob()+1) * 10 + GetSkillGroup();
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	const uint32_t SKILL_NUM = 176;
#else
	const uint32_t SKILL_NUM = 158;
#endif
	static uint32_t s_anSkill2JobGroup[SKILL_NUM] = {
		0,
		11,
		11,
		11,
		11,
		11,
		11,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		12,
		12,
		12,
		12,
		12,
		12,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		21,
		21,
		21,
		21,
		21,
		21,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		22,
		22,
		22,
		22,
		22,
		22,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		31,
		31,
		31,
		31,
		31,
		31,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		32,
		32,
		32,
		32,
		32,
		32,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		41,
		41,
		41,
		41,
		41,
		41,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		42,
		42,
		42,
		42,
		42,
		42,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		51,
		51,
		51,
		51,
		51,
		51,
#endif
	};

	const uint32_t MOTION_MAX_NUM 	= 124;
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	const uint32_t SKILL_LIST_MAX_COUNT	= 6;
#else
	const uint32_t SKILL_LIST_MAX_COUNT	= 5;
#endif
	static uint32_t s_anMotion2SkillVnumList[MOTION_MAX_NUM][SKILL_LIST_MAX_COUNT] =
	{
		{   0,		0,			0,			0,			0		},

#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		{   5,		1,			31,			61,			91,	170		},
		{   5,		2,			32,			62,			92,	171		},
		{   5,		3,			33,			63,			93,	172		},
		{   5,		4,			34,			64,			94,	173		},
		{   5,		5,			35,			65,			95,	174		},
		{   5,		6,			36,			66,			96,	175		},
#else
		{   4,		1,			31,			61,			91		},
		{   4,		2,			32,			62,			92		},
		{   4,		3,			33,			63,			93		},
		{   4,		4,			34,			64,			94		},
		{   4,		5,			35,			65,			95		},
		{   4,		6,			36,			66,			96		},
#endif
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   4,		16,			46,			76,			106		},
		{   4,		17,			47,			77,			107		},
		{   4,		18,			48,			78,			108		},
		{   4,		19,			49,			79,			109		},
		{   4,		20,			50,			80,			110		},
		{   4,		21,			51,			81,			111		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		{   5,		1,			31,			61,			91,	170		},
		{   5,		2,			32,			62,			92,	171		},
		{   5,		3,			33,			63,			93,	172		},
		{   5,		4,			34,			64,			94,	173		},
		{   5,		5,			35,			65,			95,	174		},
		{   5,		6,			36,			66,			96,	175		},
#else
		{   4,		1,			31,			61,			91		},
		{   4,		2,			32,			62,			92		},
		{   4,		3,			33,			63,			93		},
		{   4,		4,			34,			64,			94		},
		{   4,		5,			35,			65,			95		},
		{   4,		6,			36,			66,			96		},
#endif
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   4,		16,			46,			76,			106		},
		{   4,		17,			47,			77,			107		},
		{   4,		18,			48,			78,			108		},
		{   4,		19,			49,			79,			109		},
		{   4,		20,			50,			80,			110		},
		{   4,		21,			51,			81,			111		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		{   5,		1,			31,			61,			91,	170		},
		{   5,		2,			32,			62,			92,	171		},
		{   5,		3,			33,			63,			93,	172		},
		{   5,		4,			34,			64,			94,	173		},
		{   5,		5,			35,			65,			95,	174		},
		{   5,		6,			36,			66,			96,	175		},
#else
		{   4,		1,			31,			61,			91		},
		{   4,		2,			32,			62,			92		},
		{   4,		3,			33,			63,			93		},
		{   4,		4,			34,			64,			94		},
		{   4,		5,			35,			65,			95		},
		{   4,		6,			36,			66,			96		},
#endif
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   4,		16,			46,			76,			106		},
		{   4,		17,			47,			77,			107		},
		{   4,		18,			48,			78,			108		},
		{   4,		19,			49,			79,			109		},
		{   4,		20,			50,			80,			110		},
		{   4,		21,			51,			81,			111		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
		{   5,		1,			31,			61,			91,	170		},
		{   5,		2,			32,			62,			92,	171		},
		{   5,		3,			33,			63,			93,	172		},
		{   5,		4,			34,			64,			94,	173		},
		{   5,		5,			35,			65,			95,	174		},
		{   5,		6,			36,			66,			96,	175		},
#else
		{   4,		1,			31,			61,			91		},
		{   4,		2,			32,			62,			92		},
		{   4,		3,			33,			63,			93		},
		{   4,		4,			34,			64,			94		},
		{   4,		5,			35,			65,			95		},
		{   4,		6,			36,			66,			96		},
#endif
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   4,		16,			46,			76,			106		},
		{   4,		17,			47,			77,			107		},
		{   4,		18,			48,			78,			108		},
		{   4,		19,			49,			79,			109		},
		{   4,		20,			50,			80,			110		},
		{   4,		21,			51,			81,			111		},
		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   0,		0,			0,			0,			0		},
		{   0,		0,			0,			0,			0		},

		{   1,  152,    0,    0,    0},
		{   1,  153,    0,    0,    0},
		{   1,  154,    0,    0,    0},
		{   1,  155,    0,    0,    0},
		{   1,  156,    0,    0,    0},
		{   1,  157,    0,    0,    0},

		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},
		{   0,    0,    0,    0,    0},

		{   2,  137,  140,    0,    0},
		{   1,  138,    0,    0,    0},
		{   1,  139,    0,    0,    0},
	};

	if (dwMotionIndex >= MOTION_MAX_NUM)
	{
		sys_err("OUT_OF_MOTION_VNUM: name=%s, motion=%d/%d", GetName(), dwMotionIndex, MOTION_MAX_NUM);
		return false;
	}

	uint32_t* skillVNums = s_anMotion2SkillVnumList[dwMotionIndex];

	uint32_t skillCount = *skillVNums++;
	if (skillCount >= SKILL_LIST_MAX_COUNT)
	{
		sys_err("OUT_OF_SKILL_LIST: name=%s, count=%d/%d", GetName(), skillCount, SKILL_LIST_MAX_COUNT);
		return false;
	}

	for (uint32_t skillIndex = 0; skillIndex != skillCount; ++skillIndex)
	{
		if (skillIndex >= SKILL_MAX_NUM)
		{
			sys_err("OUT_OF_SKILL_VNUM: name=%s, skill=%d/%d", GetName(), skillIndex, SKILL_MAX_NUM);
			return false;
		}

		uint32_t eachSkillVNum = skillVNums[skillIndex];
		if ( eachSkillVNum != 0 )
		{
			uint32_t eachJobGroup = s_anSkill2JobGroup[eachSkillVNum];

			if (0 == eachJobGroup || eachJobGroup == selfJobGroup)
			{
				uint32_t eachSkillLevel = 0;

				if (eachSkillVNum >= GUILD_SKILL_START && eachSkillVNum <= GUILD_SKILL_END)
				{
					if (GetGuild())
						eachSkillLevel = GetGuild()->GetSkillLevel(eachSkillVNum);
					else
						eachSkillLevel = 0;
				}
				else
				{
					eachSkillLevel = GetSkillLevel(eachSkillVNum);
				}

				if (eachSkillLevel > 0)
				{
					return true;
				}
			}
		}
	}

	return false;
}

void CHARACTER::ClearSkill()
{
	PointChange(POINT_SKILL, 4 + (GetLevel() - 5) - GetPoint(POINT_SKILL));

	ResetSkill();
}

void CHARACTER::ClearSubSkill()
{
	PointChange(POINT_SUB_SKILL, GetLevel() < 10 ? 0 : (GetLevel() - 9) - GetPoint(POINT_SUB_SKILL));

	if (m_pSkillLevels == nullptr)
	{
		sys_err("m_pSkillLevels nil (name: %s)", GetName());
		return;
	}

	TPlayerSkill CleanSkill;
	memset(&CleanSkill, 0, sizeof(TPlayerSkill));

	size_t count = sizeof(s_adwSubSkillVnums) / sizeof(s_adwSubSkillVnums[0]);

	for (size_t i = 0; i < count; ++i)
	{
		if (s_adwSubSkillVnums[i] >= SKILL_MAX_NUM)
			continue;

		m_pSkillLevels[s_adwSubSkillVnums[i]] = CleanSkill;
	}

	ComputePoints();
	SkillLevelPacket();
}

bool CHARACTER::ResetOneSkill(uint32_t dwVnum)
{
	if (nullptr == m_pSkillLevels)
	{
		sys_err("m_pSkillLevels nil (name %s, vnum %u)", GetName(), dwVnum);
		return false;
	}

	if (dwVnum >= SKILL_MAX_NUM)
	{
		sys_err("vnum overflow (name %s, vnum %u)", GetName(), dwVnum);
		return false;
	}

	uint8_t level = m_pSkillLevels[dwVnum].bLevel;

	m_pSkillLevels[dwVnum].bLevel = 0;
	m_pSkillLevels[dwVnum].bMasterType = 0;
	m_pSkillLevels[dwVnum].tNextRead = 0;

	if (level > 17)
		level = 17;

	PointChange(POINT_SKILL, level);

	LogManager::instance().CharLog(this, dwVnum, "ONE_SKILL_RESET_BY_SCROLL", "");

	ComputePoints();
	SkillLevelPacket();

	return true;
}

#ifdef ENABLE_MOUNT_SKILL_ATTACK_CHECK // @correction162
eMountType GetMountLevelByVnum(uint32_t dwMountVnum, bool IsNew)
{
	if (!dwMountVnum)
		return MOUNT_TYPE_NONE;

	switch (dwMountVnum)
	{
		// @correction040 begin
		case 20107:
		case 20108:
		case 20109:
			if (IsNew)
				return MOUNT_TYPE_NONE;
		// @correction040 end
		case 20110:
		case 20111:
		case 20112:
		case 20113:
		case 20114:
		case 20115:
		case 20116:
		case 20117:
		case 20118:
		case 20205:
		case 20206:
		case 20207:
		case 20208:
		case 20120:
		case 20121:
		case 20122:
		case 20123:
		case 20124:
		case 20125:
		case 20209:
		case 20210:
		case 20211:
		case 20212:
		case 20215:
		case 20218:
		case 20225:
		case 20230:
			return MOUNT_TYPE_MILITARY;
			break;
		// @correction040 begin
		case 20104:
		case 20105:
		case 20106:
			if (IsNew)
				return MOUNT_TYPE_NONE;
		// @correction040 end
		case 20119:
		case 20214:
		case 20217:
		case 20219:
		case 20220:
		case 20221:
		case 20222:
		case 20224:
		case 20226:
		case 20227:
		case 20229:
		case 20231:
		case 20232:
			return MOUNT_TYPE_COMBAT;
			break;
		// @correction040 begin
		case 20101:
		case 20102:
		case 20103:
			if (IsNew)
				return MOUNT_TYPE_NONE;
		// @correction040 end
		case 20213:
		case 20216:
		case 20201:
		case 20202:
		case 20203:
		case 20204:
		case 20223:
		case 20228:
			return MOUNT_TYPE_NORMAL;
			break;
		default:
			return MOUNT_TYPE_NONE;
			break;
	}
}
#endif

// @correction124 BEGIN
const int32_t SKILL_COUNT = 6;
static const uint32_t SkillList[JOB_MAX_NUM][SKILL_GROUP_MAX_NUM][SKILL_COUNT] =
{
	{ {	1,	2,	3,	4,	5,	6	}, {	16,	17,	18,	19,	20,	21	} },
	{ {	31,	32,	33,	34,	35,	36	}, {	46,	47,	48,	49,	50,	51	} },
	{ {	61,	62,	63,	64,	65,	66	}, {	76,	77,	78,	79,	80,	81	} },
	{ {	91,	92,	93,	94,	95,	96	}, {	106,107,108,109,110,111	} },
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	{ {	170,171,172,173,174,175	}, {	0,	0,	0,	0,	0,	0	} },
#endif
};

const uint32_t GetRandomSkillVnum(uint8_t bJob)
{
	uint32_t dwSkillVnum = 0;
	do
	{
		uint32_t tmpJob = (bJob != JOB_MAX_NUM) ? MINMAX(0, bJob, JOB_MAX_NUM - 1) : number(0, JOB_MAX_NUM - 1);
		uint32_t tmpSkillGroup = number(0, SKILL_GROUP_MAX_NUM - 1);
		uint32_t tmpSkillCount = number(0, SKILL_COUNT - 1);
		dwSkillVnum = SkillList[tmpJob][tmpSkillGroup][tmpSkillCount];

#if defined(ENABLE_WOLFMAN_CHARACTER) && !defined(USE_WOLFMAN_BOOKS) // @correction190
		if (tmpJob == JOB_WOLFMAN)
			continue;
#endif

		if (dwSkillVnum != 0 && nullptr != CSkillManager::instance().Get(dwSkillVnum))
			break;
	} while (true);
	return dwSkillVnum;
}
// @correction124 END

bool CHARACTER::CanUseSkill(uint32_t dwSkillVnum) const
{
	if (0 == dwSkillVnum) return false;

	if (0 < GetSkillGroup())
	{
		const uint32_t* pSkill = SkillList[ GetJob() ][ GetSkillGroup()-1 ];

		for (int32_t i=0 ; i < SKILL_COUNT ; ++i)
		{
			if (pSkill[i] == dwSkillVnum) return true;
		}
	}


	if (true == IsRiding())
	{
#ifdef ENABLE_MOUNT_SKILL_ATTACK_CHECK // @correction162
		eMountType eIsMount = GetMountLevelByVnum(GetMountVnum(), false);
		if (eIsMount != MOUNT_TYPE_MILITARY)
		{
			if (test_server)
				sys_log(0, "CanUseSkill: Mount can't skill. vnum(%u) type(%d)", GetMountVnum(), static_cast<int32_t>(eIsMount));
			return false;
		}
#endif
		switch(dwSkillVnum)
		{
			case SKILL_HORSE_WILDATTACK:
			case SKILL_HORSE_CHARGE:
			case SKILL_HORSE_ESCAPE:
			case SKILL_HORSE_WILDATTACK_RANGE:
				return true;
		}
	}

	switch( dwSkillVnum )
	{
		case 121: case 122: case 124: case 126: case 127: case 128: case 129: case 130:
		case 131:
		case 151: case 152: case 153: case 154: case 155: case 156: case 157: case 158: case 159:
			return true;
	}

	return false;
}

bool CHARACTER::CheckSkillHitCount(const uint8_t SkillID, const VID TargetVID)
{
	std::map<int32_t, TSkillUseInfo>::iterator iter = m_SkillUseInfo.find(SkillID);

	if (iter == m_SkillUseInfo.end())
	{
		sys_log(0, "SkillHack: Skill(%u) is not in container", SkillID);
		return false;
	}

	TSkillUseInfo& rSkillUseInfo = iter->second;

	if (false == rSkillUseInfo.bUsed)
	{
		sys_log(0, "SkillHack: not used skill(%u)", SkillID);
		return false;
	}

	switch (SkillID)
	{
		case SKILL_YONGKWON:
		case SKILL_HWAYEOMPOK:
		case SKILL_DAEJINGAK:
		case SKILL_PAERYONG:
			sys_log(0, "SkillHack: cannot use attack packet for skill(%u)", SkillID);
			return false;
	}

	std::unordered_map<VID, size_t>::iterator iterTargetMap = rSkillUseInfo.TargetVIDMap.find(TargetVID);

	if (rSkillUseInfo.TargetVIDMap.end() != iterTargetMap)
	{
		size_t MaxAttackCountPerTarget = 1;

		switch (SkillID)
		{
			case SKILL_SAMYEON:
			case SKILL_CHARYUN:
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
			case SKILL_CHAYEOL:
#endif
				MaxAttackCountPerTarget = 3;
				break;

			case SKILL_HORSE_WILDATTACK_RANGE:
				MaxAttackCountPerTarget = 5;
				break;

			case SKILL_YEONSA:
				MaxAttackCountPerTarget = 7;
				break;

			case SKILL_HORSE_ESCAPE:
				MaxAttackCountPerTarget = 10;
				break;
		}

		if (iterTargetMap->second >= MaxAttackCountPerTarget)
		{
			sys_log(0, "SkillHack: Too Many Hit count from SkillID(%u) count(%u)", SkillID, iterTargetMap->second);
			return false;
		}

		iterTargetMap->second++;
	}
	else
	{
		rSkillUseInfo.TargetVIDMap.insert( std::make_pair(TargetVID, 1) );
	}

	return true;
}

#ifdef __678TH_SKILL__
bool CHARACTER::SkillCanUp(uint32_t dwVnum)
{
	bool canLevelUP = false;
	switch (dwVnum)
	{
	case SKILL_ANTI_PALBANG:
	{
		if (GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
			GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 && GetSkillLevel(SKILL_ANTI_HWAJO) == 0 && GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 && GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_ANTI_AMSEOP:
	{
		if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
			GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
			GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
			GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_ANTI_SWAERYUNG:
	{
		if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
			GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
			GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
			GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_ANTI_YONGBI:
	{
		if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
			GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
			GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_ANTI_GIGONGCHAM:
	{
		if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
			GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
			GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
			GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_ANTI_HWAJO:
	{
		if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
			GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
			GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_ANTI_MARYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_ANTI_MARYUNG:
	{
		if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
			GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
			GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
			GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_ANTI_BYEURAK:
	{
		if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
			GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
			GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
			GetSkillLevel(SKILL_ANTI_MARYUNG) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_ANTI_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
#ifdef ENABLE_WOLFMAN_CHARACTER
	case SKILL_ANTI_SALPOONG:
	{
		if (GetSkillLevel(SKILL_ANTI_PALBANG) == 0 &&
			GetSkillLevel(SKILL_ANTI_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_ANTI_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_ANTI_YONGBI) == 0 &&
			GetSkillLevel(SKILL_ANTI_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_ANTI_HWAJO) == 0 &&
			GetSkillLevel(SKILL_ANTI_MARYUNG) == 0
			&& GetSkillLevel(SKILL_ANTI_BYEURAK) == 0
			)
			canLevelUP = true;

		break;
	}
#endif
	case SKILL_HELP_PALBANG:
	{
		if (GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
			GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
			GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_HELP_AMSEOP:
	{
		if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
			GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
			GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
			GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_HELP_SWAERYUNG:
	{
		if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
			GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
			GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
			GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_HELP_YONGBI:
	{
		if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
			GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
			GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_HELP_GIGONGCHAM:
	{
		if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
			GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
			GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
			GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_HELP_HWAJO:
	{
		if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
			GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
			GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_HELP_MARYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_HELP_MARYUNG:
	{
		if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
			GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
			GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
			GetSkillLevel(SKILL_HELP_BYEURAK) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
	case SKILL_HELP_BYEURAK:
	{
		if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
			GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
			GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
			GetSkillLevel(SKILL_HELP_MARYUNG) == 0
#ifdef ENABLE_WOLFMAN_CHARACTER
			&& GetSkillLevel(SKILL_HELP_SALPOONG) == 0
#endif
			)
			canLevelUP = true;

		break;
	}
#ifdef ENABLE_WOLFMAN_CHARACTER
	case SKILL_HELP_SALPOONG:
	{
		if (GetSkillLevel(SKILL_HELP_PALBANG) == 0 &&
			GetSkillLevel(SKILL_HELP_AMSEOP) == 0 &&
			GetSkillLevel(SKILL_HELP_SWAERYUNG) == 0 &&
			GetSkillLevel(SKILL_HELP_YONGBI) == 0 &&
			GetSkillLevel(SKILL_HELP_GIGONGCHAM) == 0 &&
			GetSkillLevel(SKILL_HELP_HWAJO) == 0 &&
			GetSkillLevel(SKILL_HELP_MARYUNG) == 0
			&& GetSkillLevel(SKILL_HELP_BYEURAK) == 0
			)
			canLevelUP = true;

		break;
	}
#endif
	default:
		break;
	}

	return canLevelUP;
}
#endif

#ifdef __SKILLS_LEVEL_OVER_P__
bool CHARACTER::LearnSageMasterSkill(uint32_t dwSkillVnum)
{
	CSkillProto * pkSk = CSkillManager::instance().Get(dwSkillVnum);
	if (!pkSk)
		return false;
	
	if (!IsLearnableSkill(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("수련할 수 없는 스킬입니다."));
		return false;
	}
	
	sys_log(0, "<Skill> Learn skill:%d to sage master, cur %d, next %d", dwSkillVnum, get_global_time(), GetSkillNextReadTime(dwSkillVnum));
	if (pkSk->dwType == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("그랜드 마스터 수련을 할 수 없는 스킬입니다."));
		return false;
	}
	
	if (GetSkillMasterType(dwSkillVnum) != SKILL_PERFECT_MASTER)
		return false;
	
	const int aiSageMasterSkillBookMaxCount[10] = {5,  7,  9, 11, 13, 15, 20, 25, 30, 35};
	const int aiSageMasterSkillBookMinCount[10] = {1, 1, 1, 2,  2,  3,  3,  4,  5,  6};
	const int aiSageMasterSkillBookCountForLevelUp[10] = {3, 3, 5, 5, 7, 7, 10, 10, 10, 20,};
	
	std::string strTrainSkill;
	{
		std::ostringstream os;
		os << "training_sagemaster_skill.skill" << dwSkillVnum;
		strTrainSkill = os.str();
	}
	
	uint8_t bLastLevel = GetSkillLevel(dwSkillVnum);
	int idx = MIN(9, GetSkillLevel(dwSkillVnum) - 40);
	sys_log(0, "<Skill> LearnSageMasterSkill: %s table idx %d value %d", GetName(), idx, aiSageMasterSkillBookCountForLevelUp[idx]);
	int iTotalReadCount = GetQuestFlag(strTrainSkill) + 1;
	SetQuestFlag(strTrainSkill, iTotalReadCount);
	
	int iMinReadCount = aiSageMasterSkillBookMinCount[idx];
	int iMaxReadCount = aiSageMasterSkillBookMaxCount[idx];
	int iBookCount = aiSageMasterSkillBookCountForLevelUp[idx];
	if (FindAffect(AFFECT_SKILL_BOOK_BONUS))
	{
		if (iBookCount&1)
			iBookCount = iBookCount / 2 + 1;
		else
			iBookCount = iBookCount / 2;
		
		RemoveAffect(AFFECT_SKILL_BOOK_BONUS);
	}
	
	int n = number(1, iBookCount);
	uint32_t nextTime = get_global_time() + number(28800, 43200);
	sys_log(0, "<Skill> SageMaster [skill books count]: minim %d, currently %d, maximum %d (next_time %d).", iMinReadCount, iTotalReadCount, iMaxReadCount, nextTime);
	
	bool bSuccess = n == 2;
	if (iTotalReadCount < iMinReadCount)
		bSuccess = false;
	if (iTotalReadCount > iMaxReadCount)
		bSuccess = true;
	
	if (bSuccess)
		SkillLevelUp(dwSkillVnum, SKILL_UP_BY_QUEST);
	
	SetSkillNextReadTime(dwSkillVnum, nextTime);
	if (GetSkillLevel(dwSkillVnum) == bLastLevel)
	{
		ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("크윽, 기가 역류하고 있어! 이거 설마 주화입마인가!? 젠장!"));
		ChatPacket(CHAT_TYPE_INFO, LC_TEXT("수련이 실패로 끝났습니다. 다시 도전해주시기 바랍니다."));
		LogManager::instance().CharLog(this, dwSkillVnum, "GM_READ_FAIL", "");
		return false;
	}
	
	ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("몸에서 뭔가 힘이 터져 나오는 기분이야!"));
	ChatPacket(CHAT_TYPE_TALKING, LC_TEXT("뜨거운 무엇이 계속 용솟음치고 있어! 이건, 이것은!"));
	ChatPacket(CHAT_TYPE_INFO, LC_TEXT("더 높은 경지의 수련을 성공적으로 끝내셨습니다."));
	LogManager::instance().CharLog(this, dwSkillVnum, "GM_READ_SUCCESS", "");
	return true;
}
#endif