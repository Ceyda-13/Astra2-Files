#pragma once
#include "../GameLib/ItemData.h"

struct SAffects
{
	enum
	{
		AFFECT_MAX_NUM = 32,
	};

	SAffects() : dwAffects(0) {}
	SAffects(const DWORD & c_rAffects)
	{
		__SetAffects(c_rAffects);
	}
	int operator = (const DWORD & c_rAffects)
	{
		__SetAffects(c_rAffects);
	}

	BOOL IsAffect(BYTE byIndex)
	{
		return dwAffects & (1 << byIndex);
	}

	void __SetAffects(const DWORD & c_rAffects)
	{
		dwAffects = c_rAffects;
	}

	DWORD dwAffects;
};


enum
{
	POINT_MAX_NUM = 255,
	CHARACTER_NAME_MAX_LEN = 24,
	PLAYER_NAME_MAX_LEN = 12,
};

enum EInventorys
{
	INVENTORY_NORMAL,
#ifdef ENABLE_EXTEND_INVEN_SYSTEM
	INVENTORY_UPGRADE,
	INVENTORY_BOOK,
	INVENTORY_STONE,
	INVENTORY_ATTR,
	INVENTORY_GIFTBOX,
#endif
	INVENTORY_MAX,
};

extern std::string g_strGuildSymbolPathName;

const DWORD c_Name_Max_Length = 64;
const DWORD c_FileName_Max_Length = 128;
const DWORD c_Short_Name_Max_Length = 32;

const DWORD c_Inventory_Page_Column = 5;
const DWORD c_Inventory_Page_Row = 9;
const DWORD c_Inventory_Page_Size = c_Inventory_Page_Column*c_Inventory_Page_Row; // x*y
const DWORD c_Inventory_Page_Count = 4;
const DWORD c_ItemSlot_One_Count = c_Inventory_Page_Size * c_Inventory_Page_Count;
const DWORD c_ItemSlot_Count = c_ItemSlot_One_Count * INVENTORY_MAX;
const DWORD c_Equipment_Count = 12;

const DWORD c_Equipment_Start = c_ItemSlot_Count;

const DWORD c_Equipment_Body	= c_Equipment_Start + 0;
const DWORD c_Equipment_Head	= c_Equipment_Start + 1;
const DWORD c_Equipment_Shoes	= c_Equipment_Start + 2;
const DWORD c_Equipment_Wrist	= c_Equipment_Start + 3;
const DWORD c_Equipment_Weapon	= c_Equipment_Start + 4;
const DWORD c_Equipment_Neck	= c_Equipment_Start + 5;
const DWORD c_Equipment_Ear		= c_Equipment_Start + 6;
const DWORD c_Equipment_Unique1	= c_Equipment_Start + 7;
const DWORD c_Equipment_Unique2	= c_Equipment_Start + 8;
const DWORD c_Equipment_Arrow	= c_Equipment_Start + 9;
const DWORD c_Equipment_Shield	= c_Equipment_Start + 10;

// ���� �߰��� �ű� ���� & ��Ʈ
// ������ �����ۿ� �Ҵ��� �� �ִ� ��ġ�� ���� ���, ä��� ����Ʈ ����, �ڽ�Ƭ �ý��� ������ ���ؼ� ������ �߷��ִ�.
// �̰� �� ä��� ���� ������ ����������ó�� ������ ���� ������ ��������... ����
//
// �����ϸ�, ���� ���â���� ����DB�� ������ �������� 90 ~ 102 �̰�,
// 2013�� �ʿ� ���� �߰��Ǵ� ���Ե��� 111 ~ ���� �����Ѵ�. ���� ��񿡼� �ִ�� ����� �� �ִ� ���� 121 �����̰�, 122���ʹ� ��ȥ������ ����Ѵ�.
#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	const DWORD c_New_Equipment_Start = c_Equipment_Start + 21
#ifdef ENABLE_COSTUME_MOUNT
+ 1
#endif
#ifdef ENABLE_ACCE_SYSTEM
 + 1
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
+ 1
#endif
#ifdef ENABLE_COSTUME_EFFECT
+ 2
#endif
;
	const DWORD c_New_Equipment_Count = 3
#ifdef ENABLE_PENDANT
+1
#endif
#ifdef ENABLE_COSTUME_PET
+1
#endif
;
	const DWORD c_Equipment_Ring1 = c_New_Equipment_Start + 0;
	const DWORD c_Equipment_Ring2 = c_New_Equipment_Start + 1;
	const DWORD c_Equipment_Belt  = c_New_Equipment_Start + 2;
#ifdef ENABLE_PENDANT
	const DWORD c_Equipment_Pendant = c_New_Equipment_Start + 3;
#endif
#ifdef ENABLE_COSTUME_PET
	const DWORD c_Equipment_Pet = c_New_Equipment_Start + 4;
#endif
#endif

enum EDragonSoulDeckType
{
	DS_DECK_1,
	DS_DECK_2,
	DS_DECK_MAX_NUM = 2,
};

enum EDragonSoulGradeTypes
{
	DRAGON_SOUL_GRADE_NORMAL,
	DRAGON_SOUL_GRADE_BRILLIANT,
	DRAGON_SOUL_GRADE_RARE,
	DRAGON_SOUL_GRADE_ANCIENT,
	DRAGON_SOUL_GRADE_LEGENDARY,
#ifdef ENABLE_DS_GRADE_MYTH
	DRAGON_SOUL_GRADE_MYTH,
#endif
	DRAGON_SOUL_GRADE_MAX,

};

enum EDragonSoulStepTypes
{
	DRAGON_SOUL_STEP_LOWEST,
	DRAGON_SOUL_STEP_LOW,
	DRAGON_SOUL_STEP_MID,
	DRAGON_SOUL_STEP_HIGH,
	DRAGON_SOUL_STEP_HIGHEST,
	DRAGON_SOUL_STEP_MAX,
};

#ifdef ENABLE_COSTUME_SYSTEM
	const DWORD c_Costume_Slot_Start	= c_Equipment_Start + 19;	// [����] ����(19) �ϵ��ڵ� ����. ���� �������� �ڽ��� ������ 19������. ���� common/length.h ������ EWearPositions ������ ����.
	const DWORD	c_Costume_Slot_Body		= c_Costume_Slot_Start + 0;
	const DWORD	c_Costume_Slot_Hair		= c_Costume_Slot_Start + 1;
#ifdef ENABLE_COSTUME_MOUNT
	const DWORD	c_Costume_Slot_Mount 	= c_Costume_Slot_Start + 2;
#endif
#ifdef ENABLE_ACCE_SYSTEM
	const DWORD	c_Costume_Slot_Acce 	= c_Costume_Slot_Start + 3;
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	const DWORD	c_Costume_Slot_Weapon 	= c_Costume_Slot_Start + 4;
#endif
#ifdef ENABLE_COSTUME_EFFECT
	const DWORD	c_Costume_Slot_Effect1 	= c_Costume_Slot_Start + 5;
	const DWORD	c_Costume_Slot_Effect2 	= c_Costume_Slot_Start + 6;
#endif
	const DWORD c_Costume_Slot_Count	= 2
#ifdef ENABLE_COSTUME_MOUNT
+ 1
#endif
#ifdef ENABLE_ACCE_SYSTEM
+ 1
#endif
#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
+ 1
#endif
#ifdef ENABLE_COSTUME_EFFECT
+ 2
#endif
;
	const DWORD c_Costume_Slot_End		= c_Costume_Slot_Start + c_Costume_Slot_Count;
#endif


// [����] ����(32) �ϵ��ڵ� ����. ���� �������� ��ȥ�� ������ 32������.
// ���� common/length.h ������ EWearPositions �������� 32���� Ȯ��� ���� �����ϰ�(32 �̻��� Ȯ�� �ϱ� ����� �Ǿ�����.),
// �� ���ĺ��͸� ��ȥ�� ���� �������� ���.
const DWORD c_Wear_Max = 32;
const DWORD c_DragonSoul_Equip_Start = c_ItemSlot_Count + c_Wear_Max;
const DWORD c_DragonSoul_Equip_Slot_Max = 6;
const DWORD c_DragonSoul_Equip_End = c_DragonSoul_Equip_Start + c_DragonSoul_Equip_Slot_Max * DS_DECK_MAX_NUM;

// NOTE: 2013�� 2�� 5�� ����... ��ȥ�� ��ũ�� 2���� �����ϴµ�, ���� Ȯ�� ���ɼ��� �־ 3�� ��ũ �������� �Ҵ� �� ��. �� �� ������ ��Ʈ �κ��丮�� ���
const DWORD c_DragonSoul_Equip_Reserved_Count = c_DragonSoul_Equip_Slot_Max * 3;

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	// ��Ʈ �������� �����ϴ� �κ��丮
	const DWORD c_Belt_Inventory_Slot_Start = c_DragonSoul_Equip_End + c_DragonSoul_Equip_Reserved_Count;
	const DWORD c_Belt_Inventory_Width = 4;
	const DWORD c_Belt_Inventory_Height= 4;
	const DWORD c_Belt_Inventory_Slot_Count = c_Belt_Inventory_Width * c_Belt_Inventory_Height;
	const DWORD c_Belt_Inventory_Slot_End = c_Belt_Inventory_Slot_Start + c_Belt_Inventory_Slot_Count;

	const DWORD c_Inventory_Count	= c_Belt_Inventory_Slot_End;
#else
	const DWORD c_Inventory_Count	= c_DragonSoul_Equip_End;
#endif

// ��ȥ�� ���� �κ��丮
const DWORD c_DragonSoul_Inventory_Start = 0;
const DWORD c_DragonSoul_Inventory_Box_Size = 32;
const DWORD c_DragonSoul_Inventory_Count = CItemData::DS_SLOT_NUM_TYPES * DRAGON_SOUL_GRADE_MAX * c_DragonSoul_Inventory_Box_Size;
const DWORD c_DragonSoul_Inventory_End = c_DragonSoul_Inventory_Start + c_DragonSoul_Inventory_Count;

enum ESlotType
{
	SLOT_TYPE_NONE,
	SLOT_TYPE_INVENTORY,
	SLOT_TYPE_SKILL,
	SLOT_TYPE_EMOTION,
	SLOT_TYPE_SHOP,
	SLOT_TYPE_EXCHANGE_OWNER,
	SLOT_TYPE_EXCHANGE_TARGET,
	SLOT_TYPE_QUICK_SLOT,
	SLOT_TYPE_SAFEBOX,
	SLOT_TYPE_PRIVATE_SHOP,
	SLOT_TYPE_MALL,
	SLOT_TYPE_DRAGON_SOUL_INVENTORY,
#ifdef ENABLE_FISH_EVENT
	SLOT_TYPE_FISH_EVENT,
#endif
#ifdef ENABLE_SWITCHBOT
	SLOT_TYPE_SWITCHBOT,
#endif
	SLOT_TYPE_MAX,
};

enum EWindows
{
	RESERVED_WINDOW,
	INVENTORY,				// �⺻ �κ��丮. (45ĭ ¥���� 2������ ���� = 90ĭ)
	EQUIPMENT,
	SAFEBOX,
	MALL,
	DRAGON_SOUL_INVENTORY,
	BELT_INVENTORY,			// NOTE: W2.1 ������ ���� �߰��Ǵ� ��Ʈ ���� �������� �����ϴ� ��Ʈ �κ��丮
#ifdef ENABLE_SWITCHBOT
	SWITCHBOT,
#endif
	GROUND,					// NOTE: 2013�� 2��5�� ������� unused.. �� �ִ°���???
	WINDOW_TYPE_MAX,
};

enum EDSInventoryMaxNum
{
	DS_INVENTORY_MAX_NUM = c_DragonSoul_Inventory_Count,
	DS_REFINE_WINDOW_MAX_NUM = 15,
};

#ifdef ENABLE_MINI_GAME
enum EMiniGameTypes
{
#ifdef ENABLE_MINI_GAME_RUMI
	MINIGAME_TYPE_RUMI,
#endif
#if defined(ENABLE_MONSTER_BACK) || defined(ENABLE_CARNIVAL2016)
	MINIGAME_ATTENDANCE,
	MINIGAME_MONSTERBACK,
#endif
#ifdef ENABLE_FISH_EVENT
	MINIGAME_FISH,
#endif
	MINIGAME_YUTNORI,
	MINIGAME_CATCHKING,
	MINIGAME_TYPE_MAX,
};
#endif

#if defined(ENABLE_MONSTER_BACK) || defined(ENABLE_CARNIVAL2016)
enum EAttenDanceDataTypes
{
	ATTENDANCE_DATA_TYPE_DAY,
	ATTENDANCE_DATA_TYPE_MISSION_CLEAR,
	ATTENDANCE_DATA_TYPE_GET_REWARD,
	ATTENDANCE_DATA_TYPE_SHOW_MAX,
};
#endif

#ifdef ENABLE_FISH_EVENT
enum EFishEventInfo
{
	FISH_EVENT_SHAPE_NONE,
	FISH_EVENT_SHAPE_1,
	FISH_EVENT_SHAPE_2,
	FISH_EVENT_SHAPE_3,
	FISH_EVENT_SHAPE_4,
	FISH_EVENT_SHAPE_5,
	FISH_EVENT_SHAPE_6,
	FISH_EVENT_SHAPE_7,
	FISH_EVENT_SHAPE_MAX_NUM,
};
#endif

#ifdef ENABLE_SWITCHBOT
enum ESwitchbotValues
{
	SWITCHBOT_SLOT_COUNT = 5,
	SWITCHBOT_ALTERNATIVE_COUNT = 2,
	MAX_NORM_ATTR_NUM = 5,
};

enum EAttributeSet
{
	ATTRIBUTE_SET_WEAPON,
	ATTRIBUTE_SET_BODY,
	ATTRIBUTE_SET_WRIST,
	ATTRIBUTE_SET_FOOTS,
	ATTRIBUTE_SET_NECK,
	ATTRIBUTE_SET_HEAD,
	ATTRIBUTE_SET_SHIELD,
	ATTRIBUTE_SET_EAR,
	ATTRIBUTE_SET_MAX_NUM,
};

#endif
#pragma pack (push, 1)
#define WORD_MAX 0xffff

typedef struct SItemPos
{
	BYTE window_type;
	WORD cell;
    SItemPos ()
    {
		window_type =     INVENTORY;
		cell = WORD_MAX;
    }
	SItemPos (BYTE _window_type, WORD _cell)
    {
        window_type = _window_type;
        cell = _cell;
    }

	// ������ cell�� ���� ���� BYTE�� ��κ�������, oi
	// � �κ��� int, � �κ��� WORD�� �Ǿ��־�,
	// ���� ū �ڷ����� int�� �޴´�.
  //  int operator=(const int _cell)
  //  {
		//window_type = INVENTORY;
  //      cell = _cell;
  //      return cell;
  //  }
	bool IsValidCell()
	{
		switch (window_type)
		{
		case INVENTORY:
			return cell < c_Inventory_Count;
			break;
		case EQUIPMENT:
			return cell < c_DragonSoul_Equip_End;
			break;
		case DRAGON_SOUL_INVENTORY:
			return cell < (DS_INVENTORY_MAX_NUM);
			break;
#ifdef ENABLE_SWITCHBOT
		case SWITCHBOT:
			return cell < SWITCHBOT_SLOT_COUNT;
			break;
#endif
		default:
			return false;
		}
	}
	bool IsEquipCell()
	{
		switch (window_type)
		{
		case INVENTORY:
		case EQUIPMENT:
			return (c_Equipment_Start + c_Wear_Max > cell) && (c_Equipment_Start <= cell);
			break;

		case BELT_INVENTORY:
		case DRAGON_SOUL_INVENTORY:
			return false;
			break;

		default:
			return false;
		}
	}

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	bool IsBeltInventoryCell()
	{
		bool bResult = c_Belt_Inventory_Slot_Start <= cell && c_Belt_Inventory_Slot_End > cell;
		return bResult;
	}
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
	bool IsInventoryCell(BYTE bType)
	{
		bool bResult = INVENTORY == window_type && cell >= c_ItemSlot_One_Count * bType && cell < c_ItemSlot_One_Count * (bType + 1);
		return bResult;
	}
#endif

	bool operator==(const struct SItemPos& rhs) const
	{
		return (window_type == rhs.window_type) && (cell == rhs.cell);
	}

	bool operator<(const struct SItemPos& rhs) const
	{
		return (window_type < rhs.window_type) || ((window_type == rhs.window_type) && (cell < rhs.cell));
	}
} TItemPos;
#pragma pack(pop)

const DWORD c_QuickBar_Line_Count = 3;
const DWORD c_QuickBar_Slot_Count = 12;

const float c_Idle_WaitTime = 5.0f;

const int c_Monster_Race_Start_Number = 6;
const int c_Monster_Model_Start_Number = 20001;

const float c_fAttack_Delay_Time = 0.2f;
const float c_fHit_Delay_Time = 0.1f;
const float c_fCrash_Wave_Time = 0.2f;
const float c_fCrash_Wave_Distance = 3.0f;

const float c_fHeight_Step_Distance = 50.0f;

enum
{
	DISTANCE_TYPE_FOUR_WAY,
	DISTANCE_TYPE_EIGHT_WAY,
	DISTANCE_TYPE_ONE_WAY,
	DISTANCE_TYPE_MAX_NUM,
};

const float c_fMagic_Script_Version = 1.0f;
const float c_fSkill_Script_Version = 1.0f;
const float c_fMagicSoundInformation_Version = 1.0f;
const float c_fBattleCommand_Script_Version = 1.0f;
const float c_fEmotionCommand_Script_Version = 1.0f;
const float c_fActive_Script_Version = 1.0f;
const float c_fPassive_Script_Version = 1.0f;

// Used by PushMove
const float c_fWalkDistance = 175.0f;
const float c_fRunDistance = 310.0f;

#define FILE_MAX_LEN 128

enum
{
	ITEM_SOCKET_SLOT_MAX_NUM = 6,
	// refactored attribute slot begin
	ITEM_ATTRIBUTE_SLOT_NORM_NUM	= 5,
	ITEM_ATTRIBUTE_SLOT_RARE_NUM	= 2,

	ITEM_ATTRIBUTE_SLOT_NORM_START	= 0,
	ITEM_ATTRIBUTE_SLOT_NORM_END	= ITEM_ATTRIBUTE_SLOT_NORM_START + ITEM_ATTRIBUTE_SLOT_NORM_NUM,

	ITEM_ATTRIBUTE_SLOT_RARE_START	= ITEM_ATTRIBUTE_SLOT_NORM_END,
	ITEM_ATTRIBUTE_SLOT_RARE_END	= ITEM_ATTRIBUTE_SLOT_RARE_START + ITEM_ATTRIBUTE_SLOT_RARE_NUM,

	ITEM_ATTRIBUTE_SLOT_MAX_NUM		= ITEM_ATTRIBUTE_SLOT_RARE_END, // 7
	// refactored attribute slot end
};

#pragma pack(push)
#pragma pack(1)

typedef struct SQuickSlot
{
	BYTE Type;
	BYTE Position;
} TQuickSlot;

typedef struct TPlayerItemAttribute
{
    BYTE        bType;
    short       sValue;
} TPlayerItemAttribute;

typedef struct packet_item
{
    DWORD       vnum;
    WORD        count;
	DWORD		flags;
	DWORD		anti_flags;
	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_SOULBIND_SYSTEM
	long		sealbind;
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	DWORD		changelook;
#endif
} TItemData;

typedef struct packet_shop_item
{
    DWORD       vnum;
    uint64_t	price;
    WORD        count;
	BYTE		display_pos;
	long		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	DWORD		changelook;
#endif
#ifdef ENABLE_MULTISHOP
	DWORD		wPriceVnum;
#endif
} TShopItemData;

#ifdef ENABLE_MINI_GAME_RUMI
typedef struct SMiniGameRumiMoveCard
{
	BYTE bPos;
	BYTE bIndex;
	BYTE bColor;
	BYTE bNumber;
} TMiniGameRumiMoveCard;

typedef struct SMiniGameRumiMove
{
	TMiniGameRumiMoveCard src;
	TMiniGameRumiMoveCard dst;
} TMiniGameRumiMove;

typedef struct SMiniGameRumiScore
{
	BYTE bScore;
	short sTotalScore;
} TMiniGameRumiScore;
#endif
#pragma pack(pop)

inline float GetSqrtDistance(int ix1, int iy1, int ix2, int iy2) // By sqrt
{
	float dx, dy;

	dx = float(ix1 - ix2);
	dy = float(iy1 - iy2);

	return sqrtf(dx*dx + dy*dy);
}

// DEFAULT_FONT
void DefaultFont_Startup();
void DefaultFont_Cleanup();
void DefaultFont_SetName(const char * c_szFontName);
CResource* DefaultFont_GetResource();
CResource* DefaultItalicFont_GetResource();
// END_OF_DEFAULT_FONT

void SetGuildSymbolPath(const char * c_szPathName);
const char * GetGuildSymbolFileName(DWORD dwGuildID);
BYTE SlotTypeToInvenType(BYTE bSlotType);
