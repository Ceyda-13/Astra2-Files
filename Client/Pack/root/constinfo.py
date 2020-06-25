# option
import uiScriptLocale
import localeInfo

INPUT_IGNORE = 0

if ENABLE_DUNGEON_INFO_SYSTEM:
	dungeonInfo = []

	dungeonRanking = {
		"ranking_type" : 0,
		"ranking_list" : []
	}

	dungeonData = {
		"quest_index" : 0,
		"quest_cmd" : "",
	}


if ENABLE_MULTI_LANGUAGE:
	multiDict = {
		0 : [1254, "tr", uiScriptLocale.LANGUAGE_TR],
		1 : [1250, "ro", uiScriptLocale.LANGUAGE_RO],
		2 : [1252, "pt", uiScriptLocale.LANGUAGE_PT],
		3 : [1250, "pl", uiScriptLocale.LANGUAGE_PL],
		4 : [1252, "nl", uiScriptLocale.LANGUAGE_NL],
		5 : [1252, "it", uiScriptLocale.LANGUAGE_IT],
		6 : [1250, "hu", uiScriptLocale.LANGUAGE_HU],
		7 : [1253, "gr", uiScriptLocale.LANGUAGE_GR],
		8 : [1251, "ru", uiScriptLocale.LANGUAGE_RU],
		9 : [1252, "fr", uiScriptLocale.LANGUAGE_FR],
		10 : [1252, "es", uiScriptLocale.LANGUAGE_ES],
		11 : [1252, "en", uiScriptLocale.LANGUAGE_EN],
		12 : [1252, "dk", uiScriptLocale.LANGUAGE_DK],
		13 : [1252, "de", uiScriptLocale.LANGUAGE_DE],
		14 : [1252, "cz", uiScriptLocale.LANGUAGE_CZ],
		15 : [1256, "ae", uiScriptLocale.LANGUAGE_AE]
	}
	multiArray = [13, 15, 12, 4, 9, 3, 6, 2, 1, 8, 0, 7, 14, 11, 10, 5]

if ENABLE_REFINE_RENEWAL:
	IS_AUTO_REFINE = False
	AUTO_REFINE_TYPE = 0
	AUTO_REFINE_DATA = {
		"ITEM" : [-1, -1],
		"NPC" : [0, -1, -1, 0]
	}

if ENABLE_12ZI:
	SHOP_NO_LIMIT = []
	SHOW_BUTTON = 0

if ENABLE_GROWTH_PET_SYSTEM:
	PET_EVOLUTION = 0
	PET_LEVEL = 0
	PET_MAIN = 0
	FEEDWIND = 0
	SKILL_PET3 = 0
	SKILL_PET2 = 0
	SKILL_PET1 = 0
	LASTAFFECT_POINT = 0
	LASTAFFECT_VALUE = 0
	EVOLUTION = 0

	def GetEvolName(evol_level):
		if 1 == evol_level:
			return localeInfo.PET_INFORMATION_STAGE1
		elif 2 == evol_level:
			return localeInfo.PET_INFORMATION_STAGE2
		elif 3 == evol_level:
			return localeInfo.PET_INFORMATION_STAGE3
		elif 4 == evol_level:
			return localeInfo.PET_INFORMATION_STAGE4
			
		return localeInfo.PET_INFORMATION_STAGE1

	def GetPetType(type):
		if 1 == type:
			return localeInfo.PET_TYPE0
		elif 2 == type:
			return localeInfo.PET_TYPE1
		elif 3 == type:
			return localeInfo.PET_TYPE2
		elif 4 == type:
			return localeInfo.PET_TYPE3
		elif 5 == type:
			return localeInfo.PET_TYPE4
		elif 6 == type:
			return localeInfo.PET_TYPE5
		elif 7 == type:
			return localeInfo.PET_TYPE6
		elif 8 == type:
			return localeInfo.PET_TYPE7
			
		return "unk"

	def IS_NEW_PET(itemVnum):
		if itemVnum >= 55701 and itemVnum <= 55710:
			return 1

if ENABLE_SUPPORT_SYSTEM:
	SUPPORTLEVEL = 0
	SUPPORT_ARMOR = 0
	SUPPORT_WEAPON = 0
	SUPPORT_HAIR = 0

if ENABLE_OFFLINE_PRIVATE_SHOP:
	SHOPNAMES_RANGE = 5000
	OfflinePrivateShopTimeList = [
		0,
		1,
		3,
		5,
		12,
		18,
		24,
		30,
		36,
		48,
		60
	]
	PriceCheck = {}

if ENABLE_SEND_TARGET_INFO:
	MONSTER_INFO_DATA = {}

IN_GAME_SHOP_ENABLE = 1
CONSOLE_ENABLE = 0

PVPMODE_ENABLE = 1
PVPMODE_TEST_ENABLE = 0
PVPMODE_ACCELKEY_ENABLE = 1
PVPMODE_ACCELKEY_DELAY = 0.5
PVPMODE_PROTECTED_LEVEL = 15

if not ENABLE_FOG_FIX:
	FOG_LEVEL0 = 4800.0
	FOG_LEVEL1 = 9600.0
	FOG_LEVEL2 = 12800.0
	FOG_LEVEL = FOG_LEVEL0
	FOG_LEVEL_LIST = [FOG_LEVEL0, FOG_LEVEL1, FOG_LEVEL2]

CAMERA_MAX_DISTANCE_SHORT = 2500.0
CAMERA_MAX_DISTANCE_LONG = 3500.0
CAMERA_MAX_DISTANCE_LIST=[CAMERA_MAX_DISTANCE_SHORT, CAMERA_MAX_DISTANCE_LONG]
CAMERA_MAX_DISTANCE = CAMERA_MAX_DISTANCE_SHORT

CHRNAME_COLOR_INDEX = 0

ENVIRONMENT_NIGHT="d:/ymir work/environment/moonlight04.msenv"

# constant
HIGH_PRICE = 500000
MIDDLE_PRICE = 50000
ERROR_METIN_STONE = 28960
SUB2_LOADING_ENABLE = 1
EXPANDED_COMBO_ENABLE = 1
CONVERT_EMPIRE_LANGUAGE_ENABLE = 0
USE_ITEM_WEAPON_TABLE_ATTACK_BONUS = 0
ADD_DEF_BONUS_ENABLE = 0
LOGIN_COUNT_LIMIT_ENABLE = 0

USE_SKILL_EFFECT_UPGRADE_ENABLE = 1

VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD = 1
GUILD_MONEY_PER_GSP = 100
GUILD_WAR_TYPE_SELECT_ENABLE = 1
TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE = 10

HAIR_COLOR_ENABLE = 1
ARMOR_SPECULAR_ENABLE = 1
WEAPON_SPECULAR_ENABLE = 1
KEEP_ACCOUNT_CONNETION_ENABLE = 1
MINIMAP_POSITIONINFO_ENABLE = 1

isItemQuestionDialog = 0

def GET_ITEM_QUESTION_DIALOG_STATUS():
	global isItemQuestionDialog
	return isItemQuestionDialog

def SET_ITEM_QUESTION_DIALOG_STATUS(flag):
	global isItemQuestionDialog
	isItemQuestionDialog = flag

import app
import net

########################

if not ENABLE_FOG_FIX:
	def SET_DEFAULT_FOG_LEVEL():
		global FOG_LEVEL
		app.SetMinFog(FOG_LEVEL)

	def SET_FOG_LEVEL_INDEX(index):
		global FOG_LEVEL
		global FOG_LEVEL_LIST
		try:
			FOG_LEVEL=FOG_LEVEL_LIST[index]
		except IndexError:
			FOG_LEVEL=FOG_LEVEL_LIST[0]
		app.SetMinFog(FOG_LEVEL)

	def GET_FOG_LEVEL_INDEX():
		global FOG_LEVEL
		global FOG_LEVEL_LIST
		return FOG_LEVEL_LIST.index(FOG_LEVEL)

########################

def SET_DEFAULT_CAMERA_MAX_DISTANCE():
	global CAMERA_MAX_DISTANCE
	app.SetCameraMaxDistance(CAMERA_MAX_DISTANCE)

def SET_CAMERA_MAX_DISTANCE_INDEX(index):
	global CAMERA_MAX_DISTANCE
	global CAMERA_MAX_DISTANCE_LIST
	try:
		CAMERA_MAX_DISTANCE=CAMERA_MAX_DISTANCE_LIST[index]
	except:
		CAMERA_MAX_DISTANCE=CAMERA_MAX_DISTANCE_LIST[0]

	app.SetCameraMaxDistance(CAMERA_MAX_DISTANCE)

def GET_CAMERA_MAX_DISTANCE_INDEX():
	global CAMERA_MAX_DISTANCE
	global CAMERA_MAX_DISTANCE_LIST
	return CAMERA_MAX_DISTANCE_LIST.index(CAMERA_MAX_DISTANCE)

########################

import chrmgr
import player
import app

def SET_DEFAULT_CHRNAME_COLOR():
	global CHRNAME_COLOR_INDEX
	chrmgr.SetEmpireNameMode(CHRNAME_COLOR_INDEX)

def SET_CHRNAME_COLOR_INDEX(index):
	global CHRNAME_COLOR_INDEX
	CHRNAME_COLOR_INDEX=index
	chrmgr.SetEmpireNameMode(index)

def GET_CHRNAME_COLOR_INDEX():
	global CHRNAME_COLOR_INDEX
	return CHRNAME_COLOR_INDEX

def SET_VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD(index):
	global VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD
	VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD = index

def GET_VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD():
	global VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD
	return VIEW_OTHER_EMPIRE_PLAYER_TARGET_BOARD

def SET_DEFAULT_CONVERT_EMPIRE_LANGUAGE_ENABLE():
	global CONVERT_EMPIRE_LANGUAGE_ENABLE
	net.SetEmpireLanguageMode(CONVERT_EMPIRE_LANGUAGE_ENABLE)

def SET_DEFAULT_USE_ITEM_WEAPON_TABLE_ATTACK_BONUS():
	global USE_ITEM_WEAPON_TABLE_ATTACK_BONUS
	player.SetWeaponAttackBonusFlag(USE_ITEM_WEAPON_TABLE_ATTACK_BONUS)

def SET_DEFAULT_USE_SKILL_EFFECT_ENABLE():
	global USE_SKILL_EFFECT_UPGRADE_ENABLE
	app.SetSkillEffectUpgradeEnable(USE_SKILL_EFFECT_UPGRADE_ENABLE)

def SET_TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE():
	global TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE
	app.SetTwoHandedWeaponAttSpeedDecreaseValue(TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE)

########################
import item

ACCESSORY_MATERIAL_LIST = [50623, 50624, 50625, 50626, 50627, 50628, 50629, 50630, 50631, 50632, 50633, 50634, 50635, 50636, 50637, 50638, 50639, 50641, 50642, 50643 , 50644, 50645, 50646, 50647, 50650, 50657]
#ACCESSORY_MATERIAL_LIST = [50623, 50623, 50624, 50624, 50625, 50625, 50626, 50627, 50628, 50629, 50630, 50631, 50632, 50633, 
#			    50623, 50623, 50624, 50624, ]
JewelAccessoryInfos = [
		# cevher		bileklik	kolye	kupe
		[ 50634,	14220,	16220,	17220 ],
		[ 50635,	14500,	16500,	17500 ],
		[ 50636,	14520,	16520,	17520 ],
		[ 50637,	14540,	16540,	17540 ],
		[ 50638,	14560,	16560,	17560 ],
		[ 50639,	14570,	16570,	17570 ],
		[ 50640,	14230,	16230,	17230 ],
		[ 50635,	24500,	19500,	18500 ],
		[ 50636,	24520,	19520,	18520 ],
		[ 50637,	24540,	19540,	18540 ],
		[ 50638,	24560,	19560,	18560 ],
		[ 50639,	24570,	19570,	18570 ],
        [ 50650,	14240,	16240,	17240 ],
	]

JewelAccessoryInfosdict =	{
	50634:	50641, #OK
	50635:	50642, #OK
	50636:	50643, #OK
	50637:	50644, #OK
	50638:	50645, #OK
	50639:	50646, #OK
	50640:	50647, #OK
	18900:	18901, #OK
	50650:	50657, #OK
	50633:	50633,
}

def GET_ACCESSORY_MATERIAL_VNUM(vnum, subType):
	ret = vnum
	item_base = (vnum / 10) * 10
	for info in JewelAccessoryInfos:
		if item.ARMOR_WRIST == subType:
			if info[1] == item_base:
				return info[0]
		elif item.ARMOR_NECK == subType:
			if info[2] == item_base:
				return info[0]
		elif item.ARMOR_EAR == subType:
			if info[3] == item_base:
				return info[0]

	if vnum >= 16210 and vnum <= 16219:
		return 50625

	if item.ARMOR_WRIST == subType:
		WRIST_ITEM_VNUM_BASE = 14000
		ret -= WRIST_ITEM_VNUM_BASE
	elif item.ARMOR_NECK == subType:
		NECK_ITEM_VNUM_BASE = 16000
		ret -= NECK_ITEM_VNUM_BASE
	elif item.ARMOR_EAR == subType:
		EAR_ITEM_VNUM_BASE = 17000
		ret -= EAR_ITEM_VNUM_BASE

	type = ret/20

	if type<0 or type>=len(ACCESSORY_MATERIAL_LIST):
		type = (ret-170) / 20
		if type<0 or type>=len(ACCESSORY_MATERIAL_LIST):
			return 0

	return ACCESSORY_MATERIAL_LIST[type]

##################################################################
## ?? ??? '??' ??? ???, ??? ??? ?? ??? ??..
## ??? ?????? ????? ???? ???, ? ???? ?? ?????? ????? ? ??? ??..

def GET_BELT_MATERIAL_VNUM(vnum, subType = 0):
	# ??? ?? ???? ??? ???(#18900)? ?? ??
	return 18900

##################################################################
## ???? (HP: #72723 ~ #72726, SP: #72727 ~ #72730)

# ?? vnum? ???????
def IS_AUTO_POTION(itemVnum):
	return IS_AUTO_POTION_HP(itemVnum) or IS_AUTO_POTION_SP(itemVnum)

# ?? vnum? HP ???????
def IS_AUTO_POTION_HP(itemVnum):
	if 72723 <= itemVnum and 72726 >= itemVnum:
		return 1
	elif itemVnum >= 76021 and itemVnum <= 76022:		## ?? ??? ??? ??? ??
		return 1
	elif itemVnum == 79012:
		return 1

	return 0

# ?? vnum? SP ???????
def IS_AUTO_POTION_SP(itemVnum):
	if 72727 <= itemVnum and 72730 >= itemVnum:
		return 1
	elif itemVnum >= 76004 and itemVnum <= 76005:		## ?? ??? ??? ??? ??
		return 1
	elif itemVnum == 79013:
		return 1

	return 0

