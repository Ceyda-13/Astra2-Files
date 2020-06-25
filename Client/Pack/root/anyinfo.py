import __builtin__ as selfs
import app
import _winreg

########################## - DEFINES - ##########################
selfs.ENABLE_ENERGY_SYSTEM = app.ENABLE_ENERGY_SYSTEM
selfs.ENABLE_DRAGON_SOUL_SYSTEM = app.ENABLE_DRAGON_SOUL_SYSTEM
selfs.ENABLE_COSTUME_SYSTEM = app.ENABLE_COSTUME_SYSTEM
selfs.ENABLE_NEW_EQUIPMENT_SYSTEM = app.ENABLE_NEW_EQUIPMENT_SYSTEM
selfs.ENABLE_NEW_LEVELSKILL_SYSTEM = 0 # @@@correction016
selfs.ENABLE_CLEAN_DATA_IF_FAIL_LOGIN = 0
selfs.ENABLE_PASTE_FEATURE = 1 # @@@correction017
selfs.ENABLE_FULLSTONE_DETAILS = 1 # @@@correction022
selfs.ENABLE_REFINE_PCT = 1 # @@@correction020
selfs.ENABLE_678TH_SKILL = app.ENABLE_678TH_SKILL # @@@correction014
selfs.ENABLE_WOLFMAN_CHARACTER = app.ENABLE_WOLFMAN_CHARACTER
selfs.ENABLE_SEQUENCE_SYSTEM = app.ENABLE_SEQUENCE_SYSTEM # @@@correction023
selfs.ENABLE_LVL115_ARMOR_EFFECT = app.ENABLE_LVL115_ARMOR_EFFECT # @@@correction015
selfs.ENABLE_LEVEL_IN_TRADE = app.ENABLE_LEVEL_IN_TRADE # @@@correction018
selfs.ENABLE_ACCE_SYSTEM = app.ENABLE_ACCE_SYSTEM # @@@correction024
selfs.ENABLE_CHANGE_LOOK_SYSTEM = app.ENABLE_CHANGE_LOOK_SYSTEM # @@@correction025
selfs.ENABLE_CHANGE_LOOK_ITEM_SYSTEM = app.ENABLE_CHANGE_LOOK_ITEM_SYSTEM # @@@correction025
selfs.WJ_ELDER_ATTRIBUTE_SYSTEM = 1 # @@@correction026
selfs.ENABLE_FAST_NPC = 1 # @@@correction027
selfs.ENABLE_FAST_STAT = 1 # @@@correction028
selfs.ENABLE_FAST_EQUIP = 0 # @@@correction029
selfs.ENABLE_SHOP_TAB = app.ENABLE_SHOP_TAB # @@@correction030
selfs.ENABLE_MULTISHOP = app.ENABLE_MULTISHOP # @@@correction032
selfs.ENABLE_VIEW_TARGET_DECIMAL_HP = app.ENABLE_VIEW_TARGET_DECIMAL_HP # @@@correction033
selfs.ENABLE_VIEW_TARGET_PLAYER_HP = app.ENABLE_VIEW_TARGET_PLAYER_HP # @@@correction033
selfs.ENABLE_GUILDWAR_KILL_COUNTER = 0 # @@@correction034
selfs.ENABLE_DISCORD_RPC = app.ENABLE_DISCORD_RPC # @@@correction035
selfs.ENABLE_SOULBIND_SYSTEM = app.ENABLE_SOULBIND_SYSTEM # @@@correction036
selfs.ENABLE_QUIVER_SYSTEM = app.ENABLE_QUIVER_SYSTEM # @@@correction037
selfs.ENABLE_WEAPON_COSTUME_SYSTEM = app.ENABLE_WEAPON_COSTUME_SYSTEM # @@@correction038
selfs.ENABLE_COSTUME_EFFECT = app.ENABLE_COSTUME_EFFECT # @@@correction039
selfs.ENABLE_BLOCK_EQUIPMENT = 1  # @@@correction042
selfs.ENABLE_SOME_INFOS = 1  # @@@correction046
selfs.ENABLE_EXPANDED_TASKBAR = 1  # @@@correction047
selfs.agh = app.agh  # @@@correction049
selfs.ENABLE_PLAYER_PER_ACCOUNT5 = app.ENABLE_PLAYER_PER_ACCOUNT5  # @@@correction057
selfs.ENABLE_NEW_MINIMAP = 1  # @@@correction059
selfs.ENABLE_SEND_TARGET_INFO = app.ENABLE_SEND_TARGET_INFO
selfs.ENABLE_RENDER_TARGET = app.ENABLE_RENDER_TARGET
selfs.ENABLE_TOOLTIP_SHOW_MODEL_SYSTEM = 0
selfs.ENABLE_FAST_SELL = 1
selfs.ENABLE_OFFLINE_PRIVATE_SHOP = app.ENABLE_OFFLINE_PRIVATE_SHOP
selfs.UNLIMITED_POTIONS = 1
selfs.ENABLE_ELEMENT_ADD = app.ENABLE_ELEMENT_ADD
selfs.ENABLE_PENDANT = app.ENABLE_PENDANT
selfs.ENABLE_MOVE_CHANNEL = 1
selfs.ENABLE_COSTUME_MOUNT = app.ENABLE_COSTUME_MOUNT
selfs.ENABLE_COSTUME_PET = app.ENABLE_COSTUME_PET
selfs.ENABLE_NEW_BONUSES = app.ENABLE_NEW_BONUSES
selfs.ENABLE_ALIGN_RENEWAL = app.ENABLE_ALIGN_RENEWAL
selfs.ENABLE_GUILD_DRAGONLAIR_SYSTEM = app.ENABLE_GUILD_DRAGONLAIR_SYSTEM
selfs.ENABLE_TEMPLE_OCHAO = app.ENABLE_TEMPLE_OCHAO
selfs.ENABLE_SUPPORT_SYSTEM = app.ENABLE_SUPPORT_SYSTEM
selfs.ENABLE_GROWTH_PET_SYSTEM = app.ENABLE_GROWTH_PET_SYSTEM
selfs.ENABLE_BOSS_TRACKING = app.ENABLE_BOSS_TRACKING
selfs.ENABLE_DEFENSE_WAVE = app.ENABLE_DEFENSE_WAVE
selfs.ENABLE_SHOW_CHEST_DROP = app.ENABLE_SHOW_CHEST_DROP
selfs.ENABLE_DETAILS_UI = app.ENABLE_DETAILS_UI
selfs.ENABLE_QUEST_CATEGORY = 1
selfs.ENABLE_EXTEND_INVEN_SYSTEM = app.ENABLE_EXTEND_INVEN_SYSTEM
selfs.ENABLE_SORT_ITEMS = 1
selfs.ENABLE_SPILT_ITEMS = 1
selfs.ENABLE_MALLBUTTON_RENEWAL = 1
selfs.ENABLE_MINI_GAME = app.ENABLE_MINI_GAME
selfs.ENABLE_MINI_GAME_RUMI = app.ENABLE_MINI_GAME_RUMI
selfs.ENABLE_MINI_GAME_OKEY_NORMAL = app.ENABLE_MINI_GAME_OKEY_NORMAL
selfs.ENABLE_FISH_EVENT = app.ENABLE_FISH_EVENT
selfs.FISH_EVENT_SHAPE_7 = app.FISH_EVENT_SHAPE_7
selfs.FISH_EVENT_SHAPE_6 = app.FISH_EVENT_SHAPE_6
selfs.FISH_EVENT_SHAPE_5 = app.FISH_EVENT_SHAPE_5
selfs.FISH_EVENT_SHAPE_4 = app.FISH_EVENT_SHAPE_4
selfs.FISH_EVENT_SHAPE_3 = app.FISH_EVENT_SHAPE_3
selfs.FISH_EVENT_SHAPE_2 = app.FISH_EVENT_SHAPE_2
selfs.FISH_EVENT_SHAPE_1 = app.FISH_EVENT_SHAPE_1
selfs.ENABLE_MONSTER_BACK = app.ENABLE_MONSTER_BACK
selfs.ENABLE_CARNIVAL2016 = app.ENABLE_CARNIVAL2016
selfs.ENABLE_ACCUMULATE_DAMAGE_DISPLAY = app.ENABLE_ACCUMULATE_DAMAGE_DISPLAY
selfs.ENABLE_MINI_GAME_CATCH_KING = app.ENABLE_MINI_GAME_CATCH_KING
selfs.ENABLE_ANTI_RESIST_MAGIC_BONUS_SYSTEM = app.ENABLE_ANTI_RESIST_MAGIC_BONUS_SYSTEM
selfs.ENABLE_MOVE_COSTUME_ATTR = app.ENABLE_MOVE_COSTUME_ATTR
selfs.ENABLE_SPECIAL_GACHA = app.ENABLE_SPECIAL_GACHA
selfs.ENABLE_12ZI = app.ENABLE_12ZI
selfs.ENABLE_RITUEL_STONE = app.ENABLE_RITUEL_STONE
selfs.ENABLE_GUILD_GENERAL_AND_LEADER = app.ENABLE_GUILD_GENERAL_AND_LEADER
selfs.ENABLE_REFINE_RENEWAL = 1
selfs.ENABLE_LOADING_RENEWAL = 0
selfs.ENABLE_ATLAS_WARPING = app.ENABLE_ATLAS_WARPING
selfs.ENABLE_DISABLE_SOFTWARE_TILING = app.ENABLE_DISABLE_SOFTWARE_TILING
selfs.WJ_SHOW_MOB_INFO = app.WJ_SHOW_MOB_INFO
selfs.ENABLE_GRAPHIC_ON_OFF = app.ENABLE_GRAPHIC_ON_OFF
selfs.ENABLE_FOG_FIX = app.ENABLE_FOG_FIX
selfs.ENABLE_ENVIRONMENT_EFFECT_OPTION = app.ENABLE_ENVIRONMENT_EFFECT_OPTION
selfs.ENABLE_MULTI_LANGUAGE = app.ENABLE_MULTI_LANGUAGE
selfs.ENABLE_DUNGEON_INFO_SYSTEM = 1
selfs.WJ_ENABLE_TRADABLE_ICON = 1
selfs.ENABLE_DS_GRADE_MYTH = app.ENABLE_DS_GRADE_MYTH
selfs.ENABLE_MONEY_SHORTING = 1
selfs.ENABLE_SWITCHBOT = app.ENABLE_SWITCHBOT
selfs.ENABLE_FAST_RUHBK = 1
selfs.ENABLE_AFAR_BIOLOG = 1
selfs.ENABLE_AUTO_DS_REFINE = 1
selfs.ENABLE_DOG_MODE = app.ENABLE_DOG_MODE
selfs.ENABLE_DS_REFINE_MASTER = app.ENABLE_DS_REFINE_MASTER
selfs.ENABLE_ARRAGNE_AFFECTS = 1
selfs.ENABLE_NEW_BUTTONS = 1
selfs.ENABLE_SHOW_MAP_BOSS_AND_STONE = app.ENABLE_SHOW_MAP_BOSS_AND_STONE
selfs.ENABLE_FAST_USE_GIFTBOX = app.ENABLE_FAST_USE_GIFTBOX
selfs.ENABLE_YANG_INFO = app.ENABLE_YANG_INFO
selfs.ENABLE_AUTO_PICKUP = app.ENABLE_AUTO_PICKUP
selfs.ENABLE_BLOCK_EXP = 1
selfs.ENABLE_SOUL_SYSTEM = app.ENABLE_SOUL_SYSTEM
selfs.WJ_SECURITY_SYSTEM = 1#app.WJ_SECURITY_SYSTEM
selfs.NEW_TARGET_BUTTONS = 1
selfs.ENABLE_SKILLS_LEVEL_OVER_P = app.ENABLE_SKILLS_LEVEL_OVER_P
selfs.ENABLE_ADDON_ATTR = 1
########################## - DEFINES - ##########################

########################## - CLASSES - ##########################
selfs.wndGame = None
selfs.wndStream = None
selfs.wndCurtain = None
selfs.wndTargetBoard = None
selfs.wndMapNameShower = None
selfs.wndAffectShower = None
selfs.wndPlayerGauge = None
selfs.wndInputDialog = None
selfs.wndTipBoard = None
selfs.wndBigBoard = None
selfs.wndMallPageDialog = None
selfs.wndWeb = None
selfs.wndTaskBar = None
selfs.wndCharacter = None
selfs.wndInventory = None
selfs.wndExpandedTaskBar = None
selfs.wndDragonSoul = None
selfs.wndDragonSoulRefine = None
selfs.wndChat = None
selfs.wndMessenger = None
selfs.wndMiniMap = None
selfs.wndGuild = None
selfs.wndGuildBuilding = None
selfs.wndUICurtain = None
selfs.wndQuestWindow = {}
selfs.wndQuestWindowNewKey = 0
selfs.wndEnergyBar = None
selfs.wndParty = None
selfs.wndGameButton = None
selfs.wndSafebox = None
selfs.wndMall = None
selfs.wndHelp = None
selfs.wndCube = None
selfs.wndCubeResult = None
selfs.wndItemSelect = None
selfs.wndPopupDialog = None
selfs.wndDialogExchange = None
selfs.wndDialogPointReset = None
selfs.wndDialogShop = None
selfs.wndDialogRestart = None
selfs.wndDialogSystem = None
selfs.wndDialogPassword = None
selfs.wndDialogRefineNew = None
selfs.wndHyperLinkItemToolTip = None
selfs.wndToolTipItem = None
selfs.wndToolTipSkill = None
selfs.wndPrivateShopBuilder = None
selfs.quickSlotPageIndex = 0
selfs.lastPKModeSendedTime = 0
selfs.pressNumber = None
selfs.GuildWarQuestionDialog = None
if ENABLE_EXPANDED_TASKBAR: # @@@correction047
	selfs.wndExpandedMoneyTaskBar = None
if ENABLE_ACCE_SYSTEM: # @@@correction024
	selfs.wndAcceCombine = None
	selfs.wndAcceAbsorption = None
if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
	selfs.wndChangeLook = None
if ENABLE_GUILDWAR_KILL_COUNTER: # @@@correction034
	selfs.wndGuildWarKillCounter = None
	selfs.wndGuildWarScore = None
if ENABLE_TOOLTIP_SHOW_MODEL_SYSTEM:
	selfs.wndRender = None
if ENABLE_OFFLINE_PRIVATE_SHOP:
	selfs.wndOfflinePrivateShopEdit = None
	selfs.wndOfflinePrivateShopSearch = None
	selfs.wndMyShopDeco = None
if ENABLE_GUILD_DRAGONLAIR_SYSTEM:
	selfs.wndMeleyRanking = None
if ENABLE_SUPPORT_SYSTEM:
	selfs.wndSupportPG = None
if ENABLE_GROWTH_PET_SYSTEM:
	selfs.wndPetMain = None
	selfs.wndPetMini = None
if ENABLE_BOSS_TRACKING:
	selfs.wndBossTracking = None
if ENABLE_SHOW_CHEST_DROP:
	selfs.wndChestDrop = None
if ENABLE_MINI_GAME:
	selfs.wndMiniGame = None
if ENABLE_12ZI:
	selfs.wndMissionBoard = None
	selfs.wndBead = None
	selfs.wnd12ziTimer = None
	selfs.wnd12ziReward = None
if ENABLE_SWITCHBOT:
	selfs.wndSwitchbot = None
if ENABLE_FAST_RUHBK:
	selfs.wndFastRuh = None
	selfs.wndFastBK = None
if ENABLE_AFAR_BIOLOG:
	selfs.wndBio = None
	selfs.wndBio2 = None
if ENABLE_FAST_NPC:
	selfs.wndFastNpc = None
if WJ_SECURITY_SYSTEM:
	selfs.wndCaptcha = None
########################## - CLASSES - ##########################

########################## - CONSTS - ##########################
selfs.SERVER = 1
selfs.CHANNEL = 1
selfs.RomaNumers = ["I", "II", "III", "IV", "V", "VI"]
if ENABLE_FAST_EQUIP: # @@@correction029
	selfs.FAST_EQUIP = 0
	selfs.FAST_PAGE = 1
REG_PATH = r"SOFTWARE\Savies" # @@@correction056
selfs.MINIMAP = 0  # @@@correction059
########################## - CONSTS - ##########################

########################## - FUNCTIONS - ##########################
import dbg
selfs.TraceError = dbg.TraceError
def CalcScale(src, ch, dir):
	change = float(ch) / float(src)
	return (1.0 - change, 1.0 + change)[dir]
selfs.CalcScale = CalcScale

# @@@correction056 BEGIN
def SetReg(name, value):
	try:
		_winreg.CreateKey(_winreg.HKEY_CURRENT_USER, REG_PATH)
		registry_key = _winreg.OpenKey(_winreg.HKEY_CURRENT_USER, REG_PATH, 0, _winreg.KEY_WRITE)
		_winreg.SetValueEx(registry_key, name, 0, _winreg.REG_SZ, value)
		_winreg.CloseKey(registry_key)
		return True
	except WindowsError:
		return False

def GetReg(name):
	try:
		registry_key = _winreg.OpenKey(_winreg.HKEY_CURRENT_USER, REG_PATH, 0, _winreg.KEY_READ)
		value, regtype = _winreg.QueryValueEx(registry_key, name)
		_winreg.CloseKey(registry_key)
		return value
	except WindowsError:
		return None

selfs.SetReg = SetReg
selfs.GetReg = GetReg
# @@@correction056 END

if ENABLE_TOOLTIP_SHOW_MODEL_SYSTEM:
	def GetS(race):
		if race == 0 or race == 2 or race == 5 or race == 7 or race == 8:
			return 0
		else:
			return 1

	def GetRaceS(race, flag):
		if race < 8 and flag != GetS(race):
			if race < 4:
				return race + 4
			else:
				return race - 4
		else:
			return race

	selfs.GetRaceS = GetRaceS
	selfs.GetS = GetS
########################## - FUNCTIONS - ##########################
