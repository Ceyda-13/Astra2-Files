#include "stdafx.h"
#include "config.h"
#include "constants.h"
#include "utils.h"
#include "log.h"
#include "char.h"
#include "dev_log.h"
#include "locale_service.h"
#include "item.h"
#include "item_manager.h"

#include <sstream>


#define RETURN_IF_CUBE_IS_NOT_OPENED(ch) if (!(ch)->IsCubeOpen()) return


static std::vector<CUBE_DATA*>	s_cube_proto;
static bool s_isInitializedCubeMaterialInformation = false;



enum ECubeResultCategory
{
	CUBE_CATEGORY_POTION,
	CUBE_CATEGORY_WEAPON,
	CUBE_CATEGORY_ARMOR,
	CUBE_CATEGORY_ACCESSORY,
	CUBE_CATEGORY_ETC,
};

typedef std::vector<CUBE_VALUE>	TCubeValueVector;

struct SCubeMaterialInfo
{
	SCubeMaterialInfo()
	{
		bHaveComplicateMaterial = false;
	};

	CUBE_VALUE			reward;
	TCubeValueVector	material;
	uint64_t				gold;
	TCubeValueVector	complicateMaterial;

	std::string			infoText;
	bool				bHaveComplicateMaterial;
};

struct SItemNameAndLevel
{
	SItemNameAndLevel() { level = 0; }

	std::string		name;
	int32_t				level;
};


typedef std::vector<SCubeMaterialInfo>								TCubeResultList;
typedef std::unordered_map<uint32_t, TCubeResultList>				TCubeMapByNPC;
typedef std::unordered_map<uint32_t, std::string>					TCubeResultInfoTextByNPC;

TCubeMapByNPC cube_info_map;
TCubeResultInfoTextByNPC cube_result_info_map_by_npc;

class CCubeMaterialInfoHelper
{
public:
public:
};

static bool FN_check_item_count (LPITEM *items, uint32_t item_vnum, int32_t need_count)
{
	int32_t	count = 0;

	for (int32_t i=0; i<CUBE_MAX_NUM; ++i)
	{
		if (nullptr==items[i])	continue;

		if (item_vnum==items[i]->GetVnum())
		{
			count += items[i]->GetCount();
		}
	}

	return (count>=need_count);
}

static void FN_remove_material (LPITEM *items, uint32_t item_vnum, int32_t need_count)
{
	int32_t		count	= 0;
	LPITEM	item	= nullptr;

	for (int32_t i=0; i<CUBE_MAX_NUM; ++i)
	{
		if (nullptr==items[i])	continue;

		item = items[i];
		if (item_vnum==item->GetVnum())
		{
			count += item->GetCount();

			if (count>need_count)
			{
				item->SetCount(count-need_count);
				return;
			}
			else
			{
				item->SetCount(0);
				items[i] = nullptr;
			}
		}
	}
}


static CUBE_DATA* FN_find_cube (LPITEM *items, uint16_t npc_vnum)
{
	uint32_t	i, end_index;

	if (0==npc_vnum)	return nullptr;

	end_index = s_cube_proto.size();
	for (i=0; i<end_index; ++i)
	{
		if ( s_cube_proto[i]->can_make_item(items, npc_vnum) )
			return s_cube_proto[i];
	}

	return nullptr;
}

static bool FN_check_valid_npc( uint16_t vnum )
{
	for ( std::vector<CUBE_DATA*>::iterator iter = s_cube_proto.begin(); iter != s_cube_proto.end(); iter++ )
	{
		if ( std::find((*iter)->npc_vnum.begin(), (*iter)->npc_vnum.end(), vnum) != (*iter)->npc_vnum.end() )
			return true;
	}

	return false;
}

static bool FN_check_cube_data (CUBE_DATA *cube_data)
{
	uint32_t	i = 0;
	uint32_t	end_index = 0;

	end_index = cube_data->npc_vnum.size();
	for (i=0; i<end_index; ++i)
	{
		if ( cube_data->npc_vnum[i] == 0 )	return false;
	}

	end_index = cube_data->item.size();
	for (i=0; i<end_index; ++i)
	{
		if ( cube_data->item[i].vnum == 0 )		return false;
		if ( cube_data->item[i].count == 0 )	return false;
	}

	end_index = cube_data->reward.size();
	for (i=0; i<end_index; ++i)
	{
		if ( cube_data->reward[i].vnum == 0 )	return false;
		if ( cube_data->reward[i].count == 0 )	return false;
	}
	return true;
}

CUBE_DATA::CUBE_DATA()
{
	this->percent = 0;
	this->gold = 0;
#ifdef __CUBE_COPY_SOCKET_AND_APPLY__
	this->allowCopyAttr = false;
	this->allowCopySocket = false;
#endif
}

bool CUBE_DATA::can_make_item (LPITEM *items, uint16_t npc_vnum)
{
	uint32_t	i, end_index;
	uint32_t	need_vnum;
	int32_t		need_count;
	int32_t		found_npc = false;

	end_index = this->npc_vnum.size();
	for (i=0; i<end_index; ++i)
	{
		if (npc_vnum == this->npc_vnum[i])
			found_npc = true;
	}
	if (false==found_npc)	return false;

	end_index = this->item.size();
	for (i=0; i<end_index; ++i)
	{
		need_vnum	= this->item[i].vnum;
		need_count	= this->item[i].count;

		if ( false==FN_check_item_count(items, need_vnum, need_count) )
			return false;
	}

	return true;
}

CUBE_VALUE* CUBE_DATA::reward_value ()
{
	int32_t		end_index		= 0;
	uint32_t	reward_index	= 0;

	end_index = this->reward.size();
	reward_index = number(0, end_index);
	reward_index = number(0, end_index-1);

	return &this->reward[reward_index];
}

void CUBE_DATA::remove_material (LPCHARACTER ch)
{
	uint32_t	i, end_index;
	uint32_t	need_vnum;
	int32_t		need_count;
	LPITEM	*items = ch->GetCubeItem();

	end_index = this->item.size();
	for (i=0; i<end_index; ++i)
	{
		need_vnum	= this->item[i].vnum;
		need_count	= this->item[i].count;

		FN_remove_material (items, need_vnum, need_count);
	}
}

void Cube_clean_item (LPCHARACTER ch)
{
	LPITEM	*cube_item;

	cube_item = ch->GetCubeItem();

	for (int32_t i=0; i<CUBE_MAX_NUM; ++i)
	{
		if (nullptr == cube_item[i])
			continue;

		cube_item[i] = nullptr;
	}
}

void Cube_open (LPCHARACTER ch)
{
	if (false == s_isInitializedCubeMaterialInformation)
	{
		Cube_InformationInitialize();
	}

	if (nullptr == ch)
		return;

	LPCHARACTER	npc;
	npc = ch->GetQuestNPC();
	if (nullptr==npc)
	{
		if (test_server)
			dev_log(LOG_DEB0, "cube_npc is nullptr");
		return;
	}

	if ( FN_check_valid_npc(npc->GetRaceNum()) == false )
	{
		if ( test_server == true )
		{
			dev_log(LOG_DEB0, "cube not valid NPC");
		}
		return;
	}

	if (ch->IsCubeOpen())
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("이미 제조창이 열려있습니다."));
		return;
	}
	if ( ch->GetExchange() || ch->GetMyShop() || ch->GetShopOwner() || ch->IsOpenSafebox() || ch->IsCubeOpen() )
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("다른 거래중(창고,교환,상점)에는 사용할 수 없습니다."));
		return;
	}

	int32_t distance = DISTANCE_APPROX(ch->GetX() - npc->GetX(), ch->GetY() - npc->GetY());

	if (distance >= CUBE_MAX_DISTANCE)
	{
		sys_log(1, "CUBE: TOO_FAR: %s distance %d", ch->GetName(), distance);
		return;
	}


	Cube_clean_item(ch);
	ch->SetCubeNpc(npc);
	ch->ChatPacket(CHAT_TYPE_COMMAND, "cube open %d", npc->GetRaceNum());
}

void Cube_close (LPCHARACTER ch)
{
	RETURN_IF_CUBE_IS_NOT_OPENED(ch);
	Cube_clean_item(ch);
	ch->SetCubeNpc(nullptr);
	ch->ChatPacket(CHAT_TYPE_COMMAND, "cube close");
	dev_log(LOG_DEB0, "<CUBE> close (%s)", ch->GetName());
}

void Cube_init()
{
	CUBE_DATA * p_cube = nullptr;
	std::vector<CUBE_DATA*>::iterator iter;

	char file_name[256+1];
	snprintf(file_name, sizeof(file_name), "%s/cube.txt", LocaleService_GetBasePath().c_str());

	sys_log(0, "Cube_Init %s", file_name);

	for (iter = s_cube_proto.begin(); iter!=s_cube_proto.end(); iter++)
	{
		p_cube = *iter;
		M2_DELETE(p_cube);
	}

	s_cube_proto.clear();

	if (false == Cube_load(file_name))
		sys_err("Cube_Init failed");
}

bool Cube_load (const char *file)
{
	FILE	*fp;
	char	one_line[256];
	int64_t		value1, value2;
	const char	*delim = " \t\r\n";
	char	*v, *token_string;
	CUBE_DATA	*cube_data = nullptr;
	CUBE_VALUE	cube_value = {0,0};

	if (0 == file || 0 == file[0])
		return false;

	if ((fp = fopen(file, "r")) == 0)
		return false;

	while (fgets(one_line, 256, fp))
	{
		value1 = value2 = 0;

		if (one_line[0] == '#')
			continue;

		token_string = strtok(one_line, delim);

		if (nullptr == token_string)
			continue;

		if ((v = strtok(nullptr, delim)))
			str_to_number(value1, v);

		if ((v = strtok(nullptr, delim)))
			str_to_number(value2, v);

		TOKEN("section")
		{
			cube_data = M2_NEW CUBE_DATA;
		}
		else TOKEN("npc")
		{
			cube_data->npc_vnum.push_back((uint16_t)value1);
		}
		else TOKEN("item")
		{
			cube_value.vnum		= value1;
			cube_value.count	= value2;

			cube_data->item.push_back(cube_value);
		}
		else TOKEN("reward")
		{
			cube_value.vnum		= value1;
			cube_value.count	= value2;

			cube_data->reward.push_back(cube_value);
		}
		else TOKEN("percent")
		{
			cube_data->percent = value1;
		}
		else TOKEN("gold")
		{
			cube_data->gold = value1;
		}
#ifdef __CUBE_COPY_SOCKET_AND_APPLY__
		else TOKEN("allow_copy_attr")
		{
			cube_data->allowCopyAttr = (value1 == 1 ? true : false);
		}
		else TOKEN("allow_copy_socket")
		{
			cube_data->allowCopySocket = (value1 == 1 ? true : false);
		}
#endif
		else TOKEN("end")
		{
			if (false == FN_check_cube_data(cube_data))
			{
				dev_log(LOG_DEB0, "something wrong");
				M2_DELETE(cube_data);
				continue;
			}
			s_cube_proto.push_back(cube_data);
		}
	}

	fclose(fp);
	return true;
}

static void FN_cube_print (CUBE_DATA *data, uint32_t index)
{
	uint32_t	i;
	dev_log(LOG_DEB0, "--------------------------------");
	dev_log(LOG_DEB0, "CUBE_DATA[%d]", index);

	for (i=0; i<data->npc_vnum.size(); ++i)
	{
		dev_log(LOG_DEB0, "\tNPC_VNUM[%d] = %d", i, data->npc_vnum[i]);
	}
	for (i=0; i<data->item.size(); ++i)
	{
		dev_log(LOG_DEB0, "\tITEM[%d]   = (%d, %d)", i, data->item[i].vnum, data->item[i].count);
	}
	for (i=0; i<data->reward.size(); ++i)
	{
		dev_log(LOG_DEB0, "\tREWARD[%d] = (%d, %d)", i, data->reward[i].vnum, data->reward[i].count);
	}
	dev_log(LOG_DEB0, "\tPERCENT = %d", data->percent);
	dev_log(LOG_DEB0, "--------------------------------");
}

void Cube_print ()
{
	for (uint32_t i=0; i<s_cube_proto.size(); ++i)
	{
		FN_cube_print(s_cube_proto[i], i);
	}
}


static bool FN_update_cube_status(LPCHARACTER ch)
{
	if (nullptr == ch)
		return false;

	if (!ch->IsCubeOpen())
		return false;

	LPCHARACTER	npc = ch->GetQuestNPC();
	if (nullptr == npc)
		return false;

	CUBE_DATA* cube = FN_find_cube(ch->GetCubeItem(), npc->GetRaceNum());

	if (nullptr == cube)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "cube info 0 0 0");
		return false;
	}

	ch->ChatPacket(CHAT_TYPE_COMMAND, "cube info %llu %d %d", cube->gold, 0, 0);
	return true;
}

bool Cube_make (LPCHARACTER ch)
{

	LPCHARACTER	npc;
	int32_t			percent_number = 0;
	CUBE_DATA	*cube_proto;
	LPITEM	*items;
	LPITEM	new_item;
#ifdef __CUBE_COPY_SOCKET_AND_APPLY__
	uint32_t	copyAttr[ITEM_ATTRIBUTE_MAX_NUM][2];
	uint32_t	copySocket[ITEM_SOCKET_MAX_NUM];
#endif

	if (!(ch)->IsCubeOpen())
	{
		(ch)->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("제조창이 열려있지 않습니다"));
		return false;
	}

	npc = ch->GetQuestNPC();
	if (nullptr == npc)
	{
		return false;
	}

	items = ch->GetCubeItem();
	cube_proto = FN_find_cube(items, npc->GetRaceNum());

	if (nullptr == cube_proto)
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("제조 재료가 부족합니다"));
		return false;
	}
// @correction141 BEGIN
	uint64_t cube_gold = cube_proto->gold;
	if ((int64_t)cube_gold < 0 || ch->GetGold() < cube_gold)
// @correction141 END
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("돈이 부족하거나 아이템이 제자리에 없습니다."));
		return false;
	}

	CUBE_VALUE	*reward_value = cube_proto->reward_value();

#ifdef __CUBE_COPY_SOCKET_AND_APPLY__
	for (int32_t i = 0; i < CUBE_MAX_NUM; ++i)
	{
		if (nullptr == items[i])	continue;

		if (items[i]->GetType() == ITEM_WEAPON || items[i]->GetType() == ITEM_ARMOR || items[i]->GetType() == ITEM_UNIQUE || items[i]->GetType() == ITEM_COSTUME)
		{
			bool hasElement = false;
			for (uint32_t j = 0; j < cube_proto->item.size(); ++j)
			{
				if (cube_proto->item[j].vnum == items[i]->GetVnum())
				{
					hasElement = true;
					break;
				}
			}

			if (hasElement == false)
				continue;

			for (int32_t a = 0; a < ITEM_ATTRIBUTE_MAX_NUM; a++)
			{
				copyAttr[a][0] = items[i]->GetAttributeType(a);
				copyAttr[a][1] = items[i]->GetAttributeValue(a);
			}

			for (int32_t a = 0; a < ITEM_SOCKET_MAX_NUM; a++)
				copySocket[a] = items[i]->GetSocket(a);

			break;
		}

		continue;
	}
#endif

	cube_proto->remove_material (ch);

	if (0 < cube_proto->gold)
		ch->ChangeGold(false, (cube_proto->gold), false);

	percent_number = number(1,100);
	if ( percent_number<=cube_proto->percent)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "cube success %d %d", reward_value->vnum, reward_value->count);
		new_item = ch->AutoGiveItem(reward_value->vnum, reward_value->count);

#ifdef __CUBE_COPY_SOCKET_AND_APPLY__
		if (cube_proto->allowCopyAttr == true && copyAttr != nullptr)
		{
			new_item->ClearAttribute();

			for (int32_t a = 0; a < ITEM_ATTRIBUTE_MAX_NUM; a++)
			{
				new_item->SetForceAttribute(a, copyAttr[a][0], copyAttr[a][1]);
			}
		}

		if (cube_proto->allowCopySocket == true && copySocket != nullptr)
		{
			for (int32_t i = 0; i < ITEM_SOCKET_MAX_NUM; i++)
				new_item->SetSocket(i, copySocket[i]);
		}
#endif

		LogManager::instance().CubeLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(),
				reward_value->vnum, new_item->GetID(), reward_value->count, 1);
		return true;
	}
	else
	{
		ch->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("제조에 실패하였습니다."));
		ch->ChatPacket(CHAT_TYPE_COMMAND, "cube fail");
		LogManager::instance().CubeLog(ch->GetPlayerID(), ch->GetX(), ch->GetY(),
				reward_value->vnum, 0, 0, 0);
		return false;
	}

	return false;
}


void Cube_show_list (LPCHARACTER ch)
{
	LPITEM	*cube_item;
	LPITEM	item;

	RETURN_IF_CUBE_IS_NOT_OPENED(ch);

	cube_item = ch->GetCubeItem();

	for (int32_t i=0; i<CUBE_MAX_NUM; ++i)
	{
		item = cube_item[i];
		if (nullptr==item)	continue;

		ch->ChatPacket(CHAT_TYPE_INFO, "cube[%d]: inventory[%u]: %s",
				i, item->GetCell(), item->GetName());
	}
}


void Cube_add_item (LPCHARACTER ch, int32_t cube_index, int32_t inven_index)
{
	LPITEM	item;
	LPITEM	*cube_item;

	RETURN_IF_CUBE_IS_NOT_OPENED(ch);

	if (inven_index<0 || INVENTORY_MAX_NUM<=inven_index)
		return;
	if (cube_index<0 || CUBE_MAX_NUM<=cube_index)
		return;

	item = ch->GetInventoryItem(inven_index);

	if (nullptr==item)	return;

	cube_item = ch->GetCubeItem();

	for (int32_t i=0; i<CUBE_MAX_NUM; ++i)
	{
		if (item==cube_item[i])
		{
			cube_item[i] = nullptr;
			break;
		}
	}

	cube_item[cube_index] = item;

	if (test_server)
		ch->ChatPacket(CHAT_TYPE_INFO, "cube[%d]: inventory[%d]: %s added",
									cube_index, inven_index, item->GetName());

	FN_update_cube_status(ch);

	return;
}

void Cube_delete_item (LPCHARACTER ch, int32_t cube_index)
{
	LPITEM	item;
	LPITEM	*cube_item;

	RETURN_IF_CUBE_IS_NOT_OPENED(ch);

	if (cube_index<0 || CUBE_MAX_NUM<=cube_index)	return;

	cube_item = ch->GetCubeItem();

	if ( nullptr== cube_item[cube_index] )	return;

	item = cube_item[cube_index];
	cube_item[cube_index] = nullptr;

	if (test_server)
		ch->ChatPacket(CHAT_TYPE_INFO, "cube[%d]: cube[%d]: %s deleted",
				cube_index, item->GetCell(), item->GetName());

	FN_update_cube_status(ch);

	return;
}

SItemNameAndLevel SplitItemNameAndLevelFromName(const std::string& name)
{
	int32_t level = 0;
	SItemNameAndLevel info;
	info.name = name;

	size_t pos = name.find("+");

	if (std::string::npos != pos)
	{
		const std::string levelStr = name.substr(pos + 1, name.size() - pos - 1);
		str_to_number(level, levelStr.c_str());

		info.name = name.substr(0, pos);
	}

	info.level = level;

	return info;
};

bool FIsEqualCubeValue(const CUBE_VALUE& a, const CUBE_VALUE& b)
{
	return (a.vnum == b.vnum) && (a.count == b.count);
}

bool FIsLessCubeValue(const CUBE_VALUE& a, const CUBE_VALUE& b)
{
	return a.vnum < b.vnum;
}

void Cube_MakeCubeInformationText()
{
	for (TCubeMapByNPC::iterator iter = cube_info_map.begin(); cube_info_map.end() != iter; ++iter)
	{
		TCubeResultList& resultList = iter->second;

		for (TCubeResultList::iterator resultIter = resultList.begin(); resultList.end() != resultIter; ++resultIter)
		{
			SCubeMaterialInfo& materialInfo = *resultIter;
			std::string& infoText = materialInfo.infoText;


			if (0 < materialInfo.complicateMaterial.size())
			{
				std::sort(materialInfo.complicateMaterial.begin(), materialInfo.complicateMaterial.end(), FIsLessCubeValue);
				std::sort(materialInfo.material.begin(), materialInfo.material.end(), FIsLessCubeValue);

				for (TCubeValueVector::iterator iter = materialInfo.complicateMaterial.begin(); materialInfo.complicateMaterial.end() != iter; ++iter)
				{
					for (TCubeValueVector::iterator targetIter = materialInfo.material.begin(); materialInfo.material.end() != targetIter; ++targetIter)
					{
						if (*targetIter == *iter)
						{
							targetIter = materialInfo.material.erase(targetIter);
						}
					}
				}

				for (TCubeValueVector::iterator iter = materialInfo.complicateMaterial.begin(); materialInfo.complicateMaterial.end() != iter; ++iter)
				{
					char tempBuffer[128];
					sprintf(tempBuffer, "%d,%d|", iter->vnum, iter->count);

					infoText += std::string(tempBuffer);
				}

				infoText.erase(infoText.size() - 1);

				if (0 < materialInfo.material.size())
					infoText.push_back('&');
			}

			for (TCubeValueVector::iterator iter = materialInfo.material.begin(); materialInfo.material.end() != iter; ++iter)
			{
				char tempBuffer[128];
				sprintf(tempBuffer, "%d,%d&", iter->vnum, iter->count);
				infoText += std::string(tempBuffer);
			}

			infoText.erase(infoText.size() - 1);

			if (0 < materialInfo.gold)
			{
				char temp[128];
				sprintf(temp, "%llu", materialInfo.gold);
				infoText += std::string("/") + temp;
			}

		}
	}
}

void Cube_InformationInitialize()
{
	for (size_t i = 0; i < s_cube_proto.size(); ++i)
	{
		CUBE_DATA* cubeData = s_cube_proto[i];

		const std::vector<CUBE_VALUE>& rewards = cubeData->reward;

		if (1 != rewards.size())
		{
			sys_err("[CubeInfo] WARNING! Does not support multiple rewards (count: %d)", rewards.size());
			continue;
		}

		const CUBE_VALUE& reward = rewards.at(0);
		const uint16_t& npcVNUM = cubeData->npc_vnum.at(0);
		bool bComplicate = false;

		TCubeMapByNPC& cubeMap = cube_info_map;
		TCubeResultList& resultList = cubeMap[npcVNUM];
		SCubeMaterialInfo materialInfo;

		materialInfo.reward = reward;
		materialInfo.gold = cubeData->gold;
		materialInfo.material = cubeData->item;

		for (TCubeResultList::iterator iter = resultList.begin(); resultList.end() != iter; ++iter)
		{
			SCubeMaterialInfo& existInfo = *iter;

			if (reward.vnum == existInfo.reward.vnum)
			{
				for (TCubeValueVector::iterator existMaterialIter = existInfo.material.begin(); existInfo.material.end() != existMaterialIter; ++existMaterialIter)
				{
					TItemTable* existMaterialProto = ITEM_MANAGER::Instance().GetTable(existMaterialIter->vnum);
					SItemNameAndLevel existItemInfo = SplitItemNameAndLevelFromName(existMaterialProto->szName);

					if (0 < existItemInfo.level)
					{
						for (TCubeValueVector::iterator currentMaterialIter = materialInfo.material.begin(); materialInfo.material.end() != currentMaterialIter; ++currentMaterialIter)
						{
							TItemTable* currentMaterialProto = ITEM_MANAGER::Instance().GetTable(currentMaterialIter->vnum);
							SItemNameAndLevel currentItemInfo = SplitItemNameAndLevelFromName(currentMaterialProto->szName);

							if (currentItemInfo.name == existItemInfo.name)
							{
								bComplicate = true;
								existInfo.complicateMaterial.push_back(*currentMaterialIter);

								if (std::find(existInfo.complicateMaterial.begin(), existInfo.complicateMaterial.end(), *existMaterialIter) == existInfo.complicateMaterial.end())
									existInfo.complicateMaterial.push_back(*existMaterialIter);


								break;
							}
						}
					}
				}
			}

		}

		if (false == bComplicate)
			resultList.push_back(materialInfo);
	}

	Cube_MakeCubeInformationText();

	s_isInitializedCubeMaterialInformation = true;
}

void Cube_request_result_list(LPCHARACTER ch)
{
	RETURN_IF_CUBE_IS_NOT_OPENED(ch);

	LPCHARACTER	npc = ch->GetQuestNPC();
	if (nullptr == npc)
		return;

	uint32_t npcVNUM = npc->GetRaceNum();

	if (!FN_check_valid_npc(npcVNUM)) // @correction048
	{
		if (test_server)
			dev_log(LOG_DEB0, "cube not valid NPC");
		return;
	}

	size_t resultCount = 0;

	std::string& resultText = cube_result_info_map_by_npc[npcVNUM];

	if (resultText.length() == 0)
	{
		resultText.clear();

		const TCubeResultList& resultList = cube_info_map[npcVNUM];
		for (TCubeResultList::const_iterator iter = resultList.begin(); resultList.end() != iter; ++iter)
		{
			const SCubeMaterialInfo& materialInfo = *iter;
			char temp[128];
			sprintf(temp, "%d,%d", materialInfo.reward.vnum, materialInfo.reward.count);

			resultText += std::string(temp) + "/";
		}

		resultCount = resultList.size();

		if (resultText.size() != 0) // @correction048
			resultText.erase(resultText.size() - 1);

		// @correction083 BEGIN
		size_t resultTextSize = resultText.size() < 20 ? 20 - resultText.size() : resultText.size() - 20;
		if (resultTextSize >= CHAT_MAX_LEN)
		// @correction083 END
		{
			sys_err("[CubeInfo] Too int32_t cube result list text. (NPC: %d, length: %d)", npcVNUM, resultText.size());
			resultText.clear();
			resultCount = 0;
		}
	}


	ch->ChatPacket(CHAT_TYPE_COMMAND, "cube r_list %d %d %s", npcVNUM, resultCount, resultText.c_str());
}

void Cube_request_material_info(LPCHARACTER ch, int32_t requestStartIndex, int32_t requestCount)
{
	RETURN_IF_CUBE_IS_NOT_OPENED(ch);

	LPCHARACTER	npc = ch->GetQuestNPC();
	if (nullptr == npc)
		return;

	uint32_t npcVNUM = npc->GetRaceNum();

	if (!FN_check_valid_npc(npcVNUM)) // @correction048
	{
		if (test_server)
			dev_log(LOG_DEB0, "cube not valid NPC");
		return;
	}

	std::string materialInfoText = "";

	int32_t index = 0;
	bool bCatchInfo = false;

	const TCubeResultList& resultList = cube_info_map[npcVNUM];
	for (TCubeResultList::const_iterator iter = resultList.begin(); resultList.end() != iter; ++iter)
	{
		const SCubeMaterialInfo& materialInfo = *iter;

		if (index++ == requestStartIndex)
		{
			bCatchInfo = true;
		}

		if (bCatchInfo)
		{
			materialInfoText += materialInfo.infoText + "@";
		}

		if (index >= requestStartIndex + requestCount)
			break;
	}

	if (false == bCatchInfo)
	{
		sys_err("[CubeInfo] Can't find matched material info (NPC: %d, index: %d, request count: %d)", npcVNUM, requestStartIndex, requestCount);
		return;
	}

	if (materialInfoText.size() != 0) // @correction048
		materialInfoText.erase(materialInfoText.size() - 1);

	if (materialInfoText.size() - 20 >= CHAT_MAX_LEN)
	{
		sys_err("[CubeInfo] Too int32_t material info. (NPC: %d, requestStart: %d, requestCount: %d, length: %d)", npcVNUM, requestStartIndex, requestCount, materialInfoText.size());
	}

	ch->ChatPacket(CHAT_TYPE_COMMAND, "cube m_info %d %d %s", requestStartIndex, requestCount, materialInfoText.c_str());


}
