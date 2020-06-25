import os
import app
import dbg
import grp
import item
import background
import chr
import chrmgr
import player
import snd
import chat
import textTail
import snd
import net
import effect
import wndMgr
import fly
import systemSetting
import quest
import guild
import skill
import messenger
import constInfo
import exchange
import ime
import event

import ui
import uiCommon
import uiCharacter
import uiChat
import uiPrivateShopBuilder
import uiWhisper
import uiGuild
import uiQuest
import uiScriptLocale

import mouseModule
import localeInfo

import playerSettingModule

import musicInfo
import debugInfo
import stringCommander

from _weakref import proxy

import __builtin__ as selfs

# TEXTTAIL_LIVINGTIME_CONTROL
#if localeInfo.IsJAPAN():
#	app.SetTextTailLivingTime(8.0)
# END_OF_TEXTTAIL_LIVINGTIME_CONTROL

# SCREENSHOT_CWDSAVE
SCREENSHOT_CWDSAVE = False
SCREENSHOT_DIR = None

if localeInfo.IsEUROPE():
	SCREENSHOT_CWDSAVE = True

if localeInfo.IsCIBN10():
	SCREENSHOT_CWDSAVE = False
	SCREENSHOT_DIR = "YT2W"

cameraDistance = 1550.0
cameraPitch = 27.0
cameraRotation = 0.0
cameraHeight = 100.0

testAlignment = 0

class GameWindow(ui.ScriptWindow):
	def __init__(self, stream):
		selfs.wndGame = self
		ui.ScriptWindow.__init__(self, "GAME")
		self.SetWindowName("game")
		net.SetPhaseWindow(net.PHASE_WINDOW_GAME, self)
		player.SetGameWindow(self)

		self.itemDropQuestionDialog = None

		selfs.wndStream = stream
		# selfs.wndInterFace = interfaceModule.Interface()
		systemSetting.SetInterfaceHandler(self)

		self.IsQBHide = 0
		self.windowOpenPosition = 0
		self.dlgWhisperWithoutTarget = None
		self.listGMName = {}
		self.privateShopAdvertisementBoardDict = {}
		self.guildScoreBoardDict = {}
		self.equipmentDialogDict = {}

		event.SetInterfaceWindow(self)

		# selfs.wndInterFace.MakeInterface()
		import uiMessenger
		selfs.wndMessenger = uiMessenger.MessengerWindow()

		from _weakref import proxy
		selfs.wndMessenger.SetWhisperButtonEvent(lambda n, i = proxy(self) : i.OpenWhisperDialog(n))
		selfs.wndMessenger.SetGuildButtonEvent(ui.__mem_func__(self.ToggleGuildWindow))

		selfs.wndGuild = uiGuild.GuildWindow()

		selfs.wndChat = uiChat.ChatWindow()
		selfs.wndChat.SetSize(selfs.wndChat.CHAT_WINDOW_WIDTH, 0)
		selfs.wndChat.SetPosition(wndMgr.GetScreenWidth()/2 - selfs.wndChat.CHAT_WINDOW_WIDTH/2, wndMgr.GetScreenHeight() - selfs.wndChat.EDIT_LINE_HEIGHT - 37)
		selfs.wndChat.SetHeight(200)
		selfs.wndChat.Refresh()
		selfs.wndChat.Show()
		selfs.wndChat.SetSendWhisperEvent(ui.__mem_func__(self.OpenWhisperDialogWithoutTarget))
		selfs.wndChat.SetOpenChatLogEvent(ui.__mem_func__(self.ToggleChatLogWindow))

		import uiParty
		selfs.wndParty = uiParty.PartyWindow()
		selfs.wndParty.Hide()

		import uiCharacter
		selfs.wndCharacter = uiCharacter.CharacterWindow()

		import uiInventory
		selfs.wndInventory = uiInventory.InventoryWindow()

		if ENABLE_DRAGON_SOUL_SYSTEM:
			import uiDragonSoul
			selfs.wndDragonSoul = uiDragonSoul.DragonSoulWindow()
			selfs.wndDragonSoulRefine = uiDragonSoul.DragonSoulRefineWindow()

		import uiMiniMap
		selfs.wndMiniMap = uiMiniMap.MiniMap()

		import uiSafebox
		selfs.wndSafebox = uiSafebox.SafeboxWindow()
		selfs.wndMall = uiSafebox.MallWindow()
		selfs.wndChatLog = uiChat.ChatLogWindow()

		import uiExchange
		selfs.wndDialogExchange = uiExchange.ExchangeDialog()
		selfs.wndDialogExchange.LoadDialog()
		selfs.wndDialogExchange.SetCenterPosition()
		selfs.wndDialogExchange.Hide()

		import uiPointReset
		selfs.wndDialogPointReset = uiPointReset.PointResetDialog()
		selfs.wndDialogPointReset.LoadDialog()
		selfs.wndDialogPointReset.Hide()

		import uiShop
		selfs.wndDialogShop = uiShop.ShopDialog()
		selfs.wndDialogShop.LoadDialog()
		selfs.wndDialogShop.Hide()

		import uiRestart
		selfs.wndDialogRestart = uiRestart.RestartDialog()
		selfs.wndDialogRestart.LoadDialog()
		selfs.wndDialogRestart.Hide()

		import uiSystem
		selfs.wndDialogSystem = uiSystem.SystemDialog()
		selfs.wndDialogSystem.LoadDialog()
		selfs.wndDialogSystem.SetOpenHelpWindowEvent(ui.__mem_func__(self.OpenHelpWindow))
		selfs.wndDialogSystem.Hide()

		import uiSafebox
		selfs.wndDialogPassword = uiSafebox.PasswordDialog()
		selfs.wndDialogPassword.Hide()

		import uiToolTip
		selfs.wndHyperLinkItemToolTip = uiToolTip.HyperlinkItemToolTip()
		selfs.wndHyperLinkItemToolTip.Hide()

		selfs.wndToolTipItem = uiToolTip.ItemToolTip()
		selfs.wndToolTipItem.Hide()

		selfs.wndToolTipSkill = uiToolTip.SkillToolTip()
		selfs.wndToolTipSkill.Hide()

		selfs.wndPrivateShopBuilder = uiPrivateShopBuilder.PrivateShopBuilder()
		selfs.wndPrivateShopBuilder.Hide()

		import uiRefine
		selfs.wndDialogRefineNew = uiRefine.RefineDialogNew()
		selfs.wndDialogRefineNew.Hide()

		selfs.wndUICurtain = ui.Bar("TOP_MOST")
		selfs.wndUICurtain.SetSize(wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())
		selfs.wndUICurtain.SetColor(0x77000000)
		selfs.wndUICurtain.Hide()

		import uiTaskBar
		selfs.wndTaskBar = uiTaskBar.TaskBar()
		selfs.wndTaskBar.LoadWindow()
		selfs.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_CHARACTER, ui.__mem_func__(self.ToggleCharacterWindowStatusPage))
		selfs.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_INVENTORY, ui.__mem_func__(self.ToggleInventoryWindow))
		selfs.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_MESSENGER, ui.__mem_func__(self.ToggleMessenger))
		selfs.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_SYSTEM, ui.__mem_func__(self.OnPressExitKey))
		selfs.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_EXPAND_MONEY, ui.__mem_func__(self.ToggleExpandedMoneyButton)) # @@@correction047
		if uiTaskBar.TaskBar.IS_EXPANDED:
			selfs.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_EXPAND, ui.__mem_func__(self.ToggleExpandedButton))
			selfs.wndExpandedTaskBar = uiTaskBar.ExpandedTaskBar()
			selfs.wndExpandedTaskBar.LoadWindow()
			selfs.wndExpandedTaskBar.SetToggleButtonEvent(uiTaskBar.ExpandedTaskBar.BUTTON_DRAGON_SOUL, ui.__mem_func__(self.ToggleDragonSoulWindow))
		else:
			selfs.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_CHAT, ui.__mem_func__(self.ToggleChat))
		if ENABLE_FAST_EQUIP: # @@@correction029
			selfs.wndTaskBar.SetToggleButtonEvent(uiTaskBar.TaskBar.BUTTON_EKIPMAN, ui.__mem_func__(self.ToggleEkipmanWindowStatusPage))

		if ENABLE_ENERGY_SYSTEM:
			selfs.wndEnergyBar = uiTaskBar.EnergyBar()
			selfs.wndEnergyBar.LoadWindow()

		import uiGameButton
		selfs.wndGameButton = uiGameButton.GameButtonWindow()
		selfs.wndGameButton.SetTop()
		selfs.wndGameButton.Show()
		selfs.wndGameButton.SetButtonEvent("STATUS", ui.__mem_func__(self.__OnClickStatusPlusButton))
		selfs.wndGameButton.SetButtonEvent("SKILL", ui.__mem_func__(self.__OnClickSkillPlusButton))
		selfs.wndGameButton.SetButtonEvent("QUEST", ui.__mem_func__(self.__OnClickQuestButton))
		selfs.wndGameButton.SetButtonEvent("HELP", ui.__mem_func__(self.__OnClickHelpButton))
		selfs.wndGameButton.SetButtonEvent("BUILD", ui.__mem_func__(self.__OnClickBuildButton))

		import uiHelp
		selfs.wndHelp = uiHelp.HelpWindow()
		selfs.wndHelp.LoadDialog()
		selfs.wndHelp.SetCloseEvent(ui.__mem_func__(self.CloseHelpWindow))
		selfs.wndHelp.Hide()

		import uiTip
		selfs.wndTipBoard = uiTip.TipBoard()
		selfs.wndTipBoard.Hide()

		selfs.wndBigBoard = uiTip.BigBoard()
		selfs.wndBigBoard.Hide()

		if constInfo.IN_GAME_SHOP_ENABLE:
			import uiWeb
			selfs.wndWeb = uiWeb.WebWindow()
			selfs.wndWeb.LoadWindow()
			selfs.wndWeb.Hide()

		import uiCube
		selfs.wndCube = uiCube.CubeWindow()
		selfs.wndCube.LoadWindow()
		selfs.wndCube.Hide()

		selfs.wndCubeResult = uiCube.CubeResultWindow()
		selfs.wndCubeResult.LoadWindow()
		selfs.wndCubeResult.Hide()

		import uiSelectItem
		selfs.wndItemSelect = uiSelectItem.SelectItemWindow()
		selfs.wndItemSelect.Hide()

		if ENABLE_EXPANDED_TASKBAR: # @@@correction047
			import uiTaskBar
			selfs.wndExpandedMoneyTaskBar = uiTaskBar.ExpandedMoneyTaskBar()

		if ENABLE_ACCE_SYSTEM: # @@@correction024
			import uiAcce
			selfs.wndAcceCombine = uiAcce.CombineWindow()
			selfs.wndAcceCombine.LoadWindow()
			selfs.wndAcceCombine.Hide()
			
			selfs.wndAcceAbsorption = uiAcce.AbsorbWindow()
			selfs.wndAcceAbsorption.LoadWindow()
			selfs.wndAcceAbsorption.Hide()

		if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
			import uiChangeLook
			selfs.wndChangeLook = uiChangeLook.Window()
			selfs.wndChangeLook.LoadWindow()
			selfs.wndChangeLook.Hide()

		self.questButtonList = []
		self.whisperButtonList = []
		self.whisperDialogDict = {}
		self.privateShopAdvertisementBoardDict = {}

		chat.InitWhisper(self)

		self.DRAGON_SOUL_IS_QUALIFIED = False

		# selfs.wndInterFace.ShowDefaultWindows()

		selfs.wndTaskBar.Show()
		selfs.wndMiniMap.Show()
		selfs.wndMiniMap.ShowMiniMap()
		if selfs.wndEnergyBar:
			selfs.wndEnergyBar.Show()

		import uiPhaseCurtain
		selfs.wndCurtain = uiPhaseCurtain.PhaseCurtain()
		selfs.wndCurtain.speed = 0.03
		selfs.wndCurtain.Hide()

		import uiTarget
		selfs.wndTargetBoard = uiTarget.TargetBoard()
		selfs.wndTargetBoard.SetWhisperEvent(ui.__mem_func__(self.OpenWhisperDialog))
		selfs.wndTargetBoard.Hide()

		import consoleModule
		self.console = consoleModule.ConsoleWindow()
		self.console.BindGameClass(self)
		self.console.SetConsoleSize(wndMgr.GetScreenWidth(), 200)
		self.console.Hide()

		import uiMapNameShower
		selfs.wndMapNameShower = uiMapNameShower.MapNameShower()

		import uiAffectShower
		selfs.wndAffectShower = uiAffectShower.AffectShower()

		import uiPlayerGauge
		selfs.wndPlayerGauge = uiPlayerGauge.PlayerGauge(self)
		selfs.wndPlayerGauge.Hide()

		if ENABLE_GUILDWAR_KILL_COUNTER: # @@@correction034
			import uiguildwarkillcounter
			selfs.wndGuildWarKillCounter = uiguildwarkillcounter.MessageQueue()
			selfs.wndGuildWarScore = uiguildwarkillcounter.Gui()

		if ENABLE_TOOLTIP_SHOW_MODEL_SYSTEM:
			import uiRender
			selfs.wndRender = uiRender.Render()

		if ENABLE_OFFLINE_PRIVATE_SHOP:
			import uiPrivateShopEditor
			import uiPrivateShopSearch
			import uiMyShopDecoration
			selfs.wndOfflinePrivateShopEdit = uiPrivateShopEditor.OfflinePrivateShopEdit()
			selfs.wndOfflinePrivateShopSearch = uiPrivateShopSearch.OfflinePrivateShopSearch()
			selfs.wndMyShopDeco = uiMyShopDecoration.MyShopDecoration()

		if ENABLE_GUILD_DRAGONLAIR_SYSTEM:
			import uiDragonLairRanking
			selfs.wndMeleyRanking = uiDragonLairRanking.Window()
			selfs.wndMeleyRanking.LoadWindow()
			selfs.wndMeleyRanking.Hide()

		if ENABLE_SUPPORT_SYSTEM:
			import uiSupportSystem
			selfs.wndSupportPG = uiSupportSystem.SupportMainGui()
			selfs.wndSupportPG.Close()

		if ENABLE_GROWTH_PET_SYSTEM:
			import uipetsystem
			selfs.wndPetMain = uipetsystem.PetSystemMain()
			selfs.wndPetMini = uipetsystem.PetSystemMini()

		if ENABLE_BOSS_TRACKING:
			import uiBossTracking
			selfs.wndBossTracking = uiBossTracking.BossTrackingInfoWindow()
			selfs.wndBossTracking.Hide()

		if ENABLE_SHOW_CHEST_DROP:
			import uiChestDrop
			selfs.wndChestDrop = uiChestDrop.ChestDropWindow()

		if ENABLE_MINI_GAME:
			import uiMiniGame
			selfs.wndMiniGame = uiMiniGame.MiniGameWindow()
			selfs.wndMiniGame.IntegrationMiniGame(False)
			selfs.wndMiniGame.show_mini_game_dialog()

		if ENABLE_12ZI:
			selfs.wndMissionBoard = uiTip.MissionBoard()
			selfs.wndMissionBoard.Hide()
			import ui12zi

			selfs.wnd12ziTimer = ui12zi.FloorLimitTimeWindow()
			selfs.wnd12ziTimer.Hide()

			selfs.wndBead = ui12zi.BeadWindow()
			selfs.wndBead.Show()

			selfs.wnd12ziReward = ui12zi.Reward12ziWindow()
			selfs.wnd12ziReward.Hide()

		if ENABLE_DUNGEON_INFO_SYSTEM:
			import uiDungeonInfo
			selfs.wndDungeonInfo = uiDungeonInfo.DungeonInfo()
			selfs.wndDungeonInfo.Hide()

		if ENABLE_SWITCHBOT:
			import uiSwitchbot
			selfs.wndSwitchbot = uiSwitchbot.SwitchbotWindow()
			selfs.wndSwitchbot.Hide()

		if ENABLE_FAST_RUHBK:
			import uiFastRuh
			import uiFastBK

			selfs.wndFastRuh = uiFastRuh.FastRuh()
			selfs.wndFastRuh.Hide()

			selfs.wndFastBK = uiFastBK.FastBK()
			selfs.wndFastBK.Hide()

		if ENABLE_AFAR_BIOLOG:
			import uiAfarBio
			selfs.wndBio = uiAfarBio.BiyologEkran()
			selfs.wndBio.Hide()

			selfs.wndBio2 = uiAfarBio.BiologWnd()
			selfs.wndBio2.Hide()

		if ENABLE_FAST_NPC:
			import uiNpc
			selfs.wndFastNpc = uiNpc.Gui()
			selfs.wndFastNpc.Hide()

		self.__SetQuickSlotMode()

		self.__ServerCommand_Build()
		self.__ProcessPreservedServerCommand()

	def __del__(self):
		systemSetting.DestroyInterfaceHandler()
		event.SetInterfaceWindow(None)
		player.SetGameWindow(0)
		net.ClearPhaseWindow(net.PHASE_WINDOW_GAME, self)
		ui.ScriptWindow.__del__(self)

	def Open(self):
		app.SetFrameSkip(1)

		self.SetSize(wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())

		selfs.quickSlotPageIndex = 0

		self.PickingCharacterIndex = -1
		self.PickingItemIndex = -1
		self.consoleEnable = False
		self.isShowDebugInfo = False
		self.ShowNameFlag = False

		self.enableXMasBoom = False
		self.startTimeXMasBoom = 0.0
		self.indexXMasBoom = 0

		global cameraDistance, cameraPitch, cameraRotation, cameraHeight

		app.SetCamera(cameraDistance, cameraPitch, cameraRotation, cameraHeight)

		constInfo.SET_DEFAULT_CAMERA_MAX_DISTANCE()
		constInfo.SET_DEFAULT_CHRNAME_COLOR()
		if not ENABLE_FOG_FIX:
			constInfo.SET_DEFAULT_FOG_LEVEL()
		constInfo.SET_DEFAULT_CONVERT_EMPIRE_LANGUAGE_ENABLE()
		constInfo.SET_DEFAULT_USE_ITEM_WEAPON_TABLE_ATTACK_BONUS()
		constInfo.SET_DEFAULT_USE_SKILL_EFFECT_ENABLE()
		constInfo.SET_TWO_HANDED_WEAPON_ATT_SPEED_DECREASE_VALUE()

		import event
		event.SetLeftTimeString(localeInfo.UI_LEFT_TIME)

		textTail.EnablePKTitle(constInfo.PVPMODE_ENABLE)

		if constInfo.PVPMODE_TEST_ENABLE:
			self.testPKMode = ui.TextLine()
			self.testPKMode.SetFontName(localeInfo.UI_DEF_FONT)
			self.testPKMode.SetPosition(0, 15)
			self.testPKMode.SetWindowHorizontalAlignCenter()
			self.testPKMode.SetHorizontalAlignCenter()
			self.testPKMode.SetFeather()
			self.testPKMode.SetOutline()
			self.testPKMode.Show()

			self.testAlignment = ui.TextLine()
			self.testAlignment.SetFontName(localeInfo.UI_DEF_FONT)
			self.testAlignment.SetPosition(0, 35)
			self.testAlignment.SetWindowHorizontalAlignCenter()
			self.testAlignment.SetHorizontalAlignCenter()
			self.testAlignment.SetFeather()
			self.testAlignment.SetOutline()
			self.testAlignment.Show()

		self.__BuildKeyDict()
		self.__BuildDebugInfo()
		uiPrivateShopBuilder.Clear()
		exchange.InitTrading()

		if debugInfo.IsDebugMode():
			self.ToggleDebugInfo()

		snd.SetMusicVolume(systemSetting.GetMusicVolume()*net.GetFieldMusicVolume())
		snd.SetSoundVolume(systemSetting.GetSoundVolume())

		netFieldMusicFileName = net.GetFieldMusicFileName()
		if netFieldMusicFileName:
			snd.FadeInMusic("BGM/" + netFieldMusicFileName)
		elif musicInfo.fieldMusic != "":
			snd.FadeInMusic("BGM/" + musicInfo.fieldMusic)

		self.__SetQuickSlotMode()
		self.__SelectQuickPage(selfs.quickSlotPageIndex)

		self.SetFocus()
		self.Show()
		app.ShowCursor()

		net.SendEnterGamePacket()
		try:
			self.StartGame()
		except:
			import exception
			exception.Abort("GameWindow.Open")
		
		self.cubeInformation = {}
		self.currentCubeNPC = 0

		if ENABLE_DISCORD_RPC: # @@@correction035
			app.SetDiscordInformations(0)

		selfs.wndDialogSystem.RefreshConfigs()

	def Close(self):
		self.Hide()

		global cameraDistance, cameraPitch, cameraRotation, cameraHeight
		(cameraDistance, cameraPitch, cameraRotation, cameraHeight) = app.GetCamera()

		if musicInfo.fieldMusic != "":
			snd.FadeOutMusic("BGM/"+ musicInfo.fieldMusic)

		self.onPressKeyDict = None
		self.onClickKeyDict = None

		chat.Close()
		snd.StopAllSound()
		grp.InitScreenEffect()
		chr.Destroy()
		textTail.Clear()
		quest.Clear()
		background.Destroy()
		guild.Destroy()
		messenger.Destroy()
		skill.ClearSkillData()
		wndMgr.Unlock()
		mouseModule.mouseController.DeattachObject()

		self.guildNameBoard = None
		self.partyRequestQuestionDialog = None
		self.partyInviteQuestionDialog = None
		self.guildInviteQuestionDialog = None
		self.messengerAddFriendQuestion = None
		self.itemDropQuestionDialog = None
		self.confirmDialog = None
		self.OpenLinkQuestionDialog = None

		self.PrintCoord = None
		self.FrameRate = None
		self.Pitch = None
		self.Splat = None
		self.TextureNum = None
		self.ObjectNum = None
		self.ViewDistance = None
		self.PrintMousePos = None

		self.ClearDictionary()

		if selfs.GuildWarQuestionDialog:
			selfs.GuildWarQuestionDialog.Close()
			selfs.GuildWarQuestionDialog.Destroy()
			del selfs.GuildWarQuestionDialog
			selfs.GuildWarQuestionDialog = None

		if self.console:
			self.console.BindGameClass(0)
			self.console.Close()
			self.console = None

		if selfs.wndMapNameShower:
			selfs.wndMapNameShower.Destroy()
			del selfs.wndMapNameShower
			selfs.wndMapNameShower = None

		if selfs.wndPlayerGauge:
			selfs.wndPlayerGauge.Destroy()
			del selfs.wndPlayerGauge
			selfs.wndPlayerGauge = None

		if selfs.wndAffectShower:
			selfs.wndAffectShower.Destroy()
			del selfs.wndAffectShower
			selfs.wndAffectShower = None

		if selfs.wndTargetBoard:
			selfs.wndTargetBoard.Destroy()
			del selfs.wndTargetBoard
			selfs.wndTargetBoard = None

		self.HideAllWindows()
		# selfs.wndInterFace.Close()
		if self.dlgWhisperWithoutTarget:
			self.dlgWhisperWithoutTarget.Destroy()
			del self.dlgWhisperWithoutTarget

		if uiQuest.QuestDialog.__dict__.has_key("QuestCurtain"):
			uiQuest.QuestDialog.QuestCurtain.Close()

		if selfs.wndQuestWindow:
			for key, eachQuestWindow in selfs.wndQuestWindow.items():
				eachQuestWindow.nextCurtainMode = -1
				eachQuestWindow.CloseSelf()
				del eachQuestWindow
				eachQuestWindow = None

			del selfs.wndQuestWindow
			selfs.wndQuestWindow = {}

		if selfs.wndChat:
			selfs.wndChat.Destroy()
			del selfs.wndChat
			selfs.wndChat = None

		if selfs.wndTaskBar:
			selfs.wndTaskBar.Destroy()
			del selfs.wndTaskBar
			selfs.wndTaskBar = None

		if selfs.wndExpandedTaskBar:
			selfs.wndExpandedTaskBar.Destroy()
			del selfs.wndExpandedTaskBar
			selfs.wndExpandedTaskBar = None

		if selfs.wndEnergyBar:
			selfs.wndEnergyBar.Destroy()
			del selfs.wndEnergyBar
			selfs.wndEnergyBar = None

		if selfs.wndCharacter:
			selfs.wndCharacter.Destroy()
			del selfs.wndCharacter
			selfs.wndCharacter = None

		if selfs.wndInventory:
			selfs.wndInventory.Destroy()
			del selfs.wndInventory
			selfs.wndInventory = None

		if selfs.wndDragonSoul:
			selfs.wndDragonSoul.Destroy()
			del selfs.wndDragonSoul
			selfs.wndDragonSoul = None

		if selfs.wndDragonSoulRefine:
			selfs.wndDragonSoulRefine.Destroy()
			del selfs.wndDragonSoulRefine
			selfs.wndDragonSoulRefine = None

		if selfs.wndDialogExchange:
			selfs.wndDialogExchange.Destroy()
			del selfs.wndDialogExchange
			selfs.wndDialogExchange = None

		if selfs.wndDialogPointReset:
			selfs.wndDialogPointReset.Destroy()
			del selfs.wndDialogPointReset
			selfs.wndDialogPointReset = None

		if selfs.wndDialogShop:
			selfs.wndDialogShop.Destroy()
			del selfs.wndDialogShop
			selfs.wndDialogShop = None

		if selfs.wndDialogRestart:
			selfs.wndDialogRestart.Destroy()
			del selfs.wndDialogRestart
			selfs.wndDialogRestart = None

		if selfs.wndDialogSystem:
			selfs.wndDialogSystem.Destroy()
			del selfs.wndDialogSystem
			selfs.wndDialogSystem = None

		if selfs.wndDialogPassword:
			selfs.wndDialogPassword.Destroy()
			del selfs.wndDialogPassword
			selfs.wndDialogPassword = None

		if selfs.wndMiniMap:
			selfs.wndMiniMap.Hide()
			selfs.wndMiniMap.Destroy()
			del selfs.wndMiniMap
			selfs.wndMiniMap = None

		if selfs.wndSafebox:
			selfs.wndSafebox.Destroy()
			del selfs.wndSafebox
			selfs.wndSafebox = None

		if selfs.wndWeb:
			selfs.wndWeb.Destroy()
			del selfs.wndWeb
			selfs.wndWeb = None

		if selfs.wndMall:
			selfs.wndMall.Destroy()
			del selfs.wndMall
			selfs.wndMall = None

		if selfs.wndParty:
			selfs.wndParty.Destroy()
			del selfs.wndParty
			selfs.wndParty = None

		if selfs.wndHelp:
			selfs.wndHelp.Destroy()
			del selfs.wndHelp
			selfs.wndHelp = None

		if selfs.wndCube:
			selfs.wndCube.Destroy()
			del selfs.wndCube
			selfs.wndCube = None

		if selfs.wndCubeResult:
			selfs.wndCubeResult.Destroy()
			del selfs.wndCubeResult
			selfs.wndCubeResult = None

		if selfs.wndMessenger:
			selfs.wndMessenger.Destroy()
			del selfs.wndMessenger
			selfs.wndMessenger = None

		if selfs.wndGuild:
			selfs.wndGuild.Destroy()
			del selfs.wndGuild
			selfs.wndGuild = None

		if selfs.wndPrivateShopBuilder:
			selfs.wndPrivateShopBuilder.Destroy()
			del selfs.wndPrivateShopBuilder
			selfs.wndPrivateShopBuilder = None

		if selfs.wndDialogRefineNew:
			selfs.wndDialogRefineNew.Destroy()
			del selfs.wndDialogRefineNew
			selfs.wndDialogRefineNew = None

		if selfs.wndGuildBuilding:
			selfs.wndGuildBuilding.Destroy()
			del selfs.wndGuildBuilding
			selfs.wndGuildBuilding = None

		if selfs.wndGameButton:
			selfs.wndGameButton.Destroy()
			del selfs.wndGameButton
			selfs.wndGameButton = None

		if selfs.wndMallPageDialog:
			selfs.wndMallPageDialog.Destroy()
			del selfs.wndMallPageDialog
			selfs.wndMallPageDialog = None

		if selfs.wndItemSelect:
			selfs.wndItemSelect.Destroy()
			del selfs.wndItemSelect
			selfs.wndItemSelect = None

		if selfs.wndChatLog:
			selfs.wndChatLog.Destroy()
			del selfs.wndChatLog
			selfs.wndChatLog = None

		if selfs.wndUICurtain:
			selfs.wndUICurtain.Destroy()
			del selfs.wndUICurtain
			selfs.wndUICurtain = None

		if selfs.wndHyperLinkItemToolTip:
			selfs.wndHyperLinkItemToolTip.Destroy()
			del selfs.wndHyperLinkItemToolTip
			selfs.wndHyperLinkItemToolTip = None

		if selfs.wndToolTipItem:
			selfs.wndToolTipItem.Destroy()
			del selfs.wndToolTipItem
			selfs.wndToolTipItem = None

		if selfs.wndToolTipSkill:
			selfs.wndToolTipSkill.Destroy()
			del selfs.wndToolTipSkill
			selfs.wndToolTipSkill = None

		if selfs.wndInputDialog:
			selfs.wndInputDialog.Destroy()
			del selfs.wndInputDialog
			selfs.wndInputDialog = None

		if selfs.wndTipBoard:
			selfs.wndTipBoard.Destroy()
			del selfs.wndTipBoard
			selfs.wndTipBoard = None

		if selfs.wndBigBoard:
			selfs.wndBigBoard.Destroy()
			del selfs.wndBigBoard
			selfs.wndBigBoard = None

		if selfs.wndCurtain:
			selfs.wndCurtain.Destroy()
			del selfs.wndCurtain
			selfs.wndCurtain = None

		if selfs.wndPopupDialog:
			selfs.wndPopupDialog.Destroy()
			del selfs.wndPopupDialog
			selfs.wndPopupDialog = None

		if selfs.pressNumber:
			del selfs.pressNumber
			selfs.pressNumber = None

		if ENABLE_EXPANDED_TASKBAR and selfs.wndExpandedMoneyTaskBar: # @@@correction047
			selfs.wndExpandedMoneyTaskBar.Destroy()
			del selfs.wndExpandedMoneyTaskBar
			selfs.wndExpandedMoneyTaskBar = None

		if ENABLE_ACCE_SYSTEM: # @@@correction024
			if selfs.wndAcceCombine:
				selfs.wndAcceCombine.Destroy()
				del selfs.wndAcceCombine
				selfs.wndAcceCombine = None

			if selfs.wndAcceAbsorption:
				selfs.wndAcceAbsorption.Destroy()
				del selfs.wndAcceAbsorption
				selfs.wndAcceAbsorption = None

		if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
			if selfs.wndChangeLook:
				selfs.wndChangeLook.Destroy()
				del selfs.wndChangeLook
				selfs.wndChangeLook = None

		if ENABLE_GUILDWAR_KILL_COUNTER: # @@@correction034
			if selfs.wndGuildWarKillCounter:
				selfs.wndGuildWarKillCounter.Destroy()
				del selfs.wndGuildWarKillCounter
				selfs.wndGuildWarKillCounter = None

			if selfs.wndGuildWarScore:
				selfs.wndGuildWarScore.Destroy()
				del selfs.wndGuildWarScore
				selfs.wndGuildWarScore = None

		if ENABLE_TOOLTIP_SHOW_MODEL_SYSTEM:
			if selfs.wndRender:
				selfs.wndRender.Destroy()
				del selfs.wndRender
				selfs.wndRender = None

		if ENABLE_OFFLINE_PRIVATE_SHOP:
			if selfs.wndOfflinePrivateShopEdit:
				selfs.wndOfflinePrivateShopEdit.Destroy()
				del selfs.wndOfflinePrivateShopEdit
				selfs.wndOfflinePrivateShopEdit = None

			if selfs.wndOfflinePrivateShopSearch:
				selfs.wndOfflinePrivateShopSearch.Destroy()
				del selfs.wndOfflinePrivateShopSearch
				selfs.wndOfflinePrivateShopSearch = None

			if selfs.wndMyShopDeco:
				selfs.wndMyShopDeco.Destroy()
				del selfs.wndMyShopDeco
				selfs.wndMyShopDeco = None

		if ENABLE_GUILD_DRAGONLAIR_SYSTEM:
			if selfs.wndMeleyRanking:
				selfs.wndMeleyRanking.Destroy()
				del selfs.wndMeleyRanking
				selfs.wndMeleyRanking = None

		if ENABLE_SUPPORT_SYSTEM:
			if selfs.wndSupportPG:
				selfs.wndSupportPG.Destroy()
				selfs.wndSupportPG.Close()
				del selfs.wndSupportPG
				selfs.wndSupportPG = None

		if ENABLE_GROWTH_PET_SYSTEM:
			if selfs.wndPetMain:
				selfs.wndPetMain.Close()
				del selfs.wndPetMain
				selfs.wndPetMain = None
			if selfs.wndPetMini:
				selfs.wndPetMini.Close()
				del selfs.wndPetMini
				selfs.wndPetMini = None

		if ENABLE_BOSS_TRACKING:
			if selfs.wndBossTracking:
				selfs.wndBossTracking.Close()
				del selfs.wndBossTracking
				selfs.wndBossTracking = None

		if ENABLE_SHOW_CHEST_DROP:
			if selfs.wndChestDrop:
				selfs.wndChestDrop.Destroy()
				del selfs.wndChestDrop
				selfs.wndChestDrop = None

		if ENABLE_MINI_GAME:
			if selfs.wndMiniGame:
				selfs.wndMiniGame.Destroy()
				del selfs.wndMiniGame
				selfs.wndMiniGame = None

		if ENABLE_12ZI:
			if selfs.wndBead:
				selfs.wndBead.Hide()
				selfs.wndBead.Destroy()
				del selfs.wndBead
				selfs.wndBead = None

			if selfs.wnd12ziTimer:
				selfs.wnd12ziTimer.Hide()
				selfs.wnd12ziTimer.Destroy()
				del selfs.wnd12ziTimer
				selfs.wnd12ziTimer = None

			if selfs.wnd12ziReward:
				selfs.wnd12ziReward.Hide()
				selfs.wnd12ziReward.Destroy()
				del selfs.wnd12ziReward
				selfs.wnd12ziReward = None

			del selfs.wndMissionBoard
			selfs.wndMissionBoard = None

		if ENABLE_DUNGEON_INFO_SYSTEM:
			if selfs.wndDungeonInfo:
				selfs.wndDungeonInfo.Hide()
				selfs.wndDungeonInfo.Destroy()
				del selfs.wndDungeonInfo
				selfs.wndDungeonInfo = None

		if ENABLE_SWITCHBOT:
			if selfs.wndSwitchbot:
				selfs.wndSwitchbot.Hide()
				selfs.wndSwitchbot.Destroy()
				del selfs.wndSwitchbot
				selfs.wndSwitchbot = None

		if ENABLE_FAST_RUHBK:
			if selfs.wndFastRuh:
				selfs.wndFastRuh.Hide()
				selfs.wndFastRuh.Destroy()
				del selfs.wndFastRuh
				selfs.wndFastRuh = None

			if selfs.wndFastBK:
				selfs.wndFastBK.Hide()
				selfs.wndFastBK.Destroy()
				del selfs.wndFastBK
				selfs.wndFastBK = None

		if ENABLE_AFAR_BIOLOG:
			if selfs.wndBio:
				selfs.wndBio.Hide()
				selfs.wndBio.Destroy()
				del selfs.wndBio
				selfs.wndBio = None

			if selfs.wndBio2:
				selfs.wndBio2.Hide()
				selfs.wndBio2.Destroy()
				del selfs.wndBio2
				selfs.wndBio2 = None

		if ENABLE_FAST_NPC:
			if selfs.wndFastNpc:
				selfs.wndFastNpc.Hide()
				selfs.wndFastNpc.Destroy()
				del selfs.wndFastNpc
				selfs.wndFastNpc = None

		for btn in self.questButtonList:
			btn.SetEvent(0)
		for btn in self.whisperButtonList:
			btn.SetEvent(0)
		for dlg in self.whisperDialogDict.itervalues():
			dlg.Destroy()
		for brd in self.guildScoreBoardDict.itervalues():
			brd.Destroy()
		for dlg in self.equipmentDialogDict.itervalues():
			dlg.Destroy()

		self.questButtonList = []
		self.whisperButtonList = []
		self.whisperDialogDict = {}
		self.privateShopAdvertisementBoardDict = {}
		self.guildScoreBoardDict = {}
		self.equipmentDialogDict = {}

		uiChat.DestroyChatInputSetWindow()
		# selfs.wndInterFace = None

		player.ClearSkillDict()
		player.ResetCameraRotation()

		self.KillFocus()
		app.HideCursor()

	def ShowAllWindows(self):
		if selfs.wndTaskBar:
			selfs.wndTaskBar.Show()

		if selfs.wndCharacter:
			selfs.wndCharacter.Show()

		if selfs.wndInventory:
			selfs.wndInventory.Show()

		if ENABLE_DRAGON_SOUL_SYSTEM:
			if selfs.wndDragonSoul:
				selfs.wndDragonSoul.Show()

			if selfs.wndDragonSoulRefine:
				selfs.wndDragonSoulRefine.Show()

		if selfs.wndChat:
			selfs.wndChat.Show()

		if selfs.wndMiniMap:
			selfs.wndMiniMap.Show()

		if selfs.wndEnergyBar:
			selfs.wndEnergyBar.Show()

		if selfs.wndExpandedTaskBar:
			selfs.wndExpandedTaskBar.Show()
			selfs.wndExpandedTaskBar.SetTop()

		if ENABLE_EXPANDED_TASKBAR: # @@@correction047
			if selfs.wndExpandedMoneyTaskBar:
				selfs.wndExpandedMoneyTaskBar.Show()

		if ENABLE_12ZI:
			if selfs.wndBead:
				selfs.wndBead.Show()
			# if selfs.wnd12ziTimer:
				# selfs.wnd12ziTimer.Show()
			# if selfs.wnd12ziReward:
				# selfs.wnd12ziReward.Show()

	def HideAllWindows(self):
		if selfs.wndParty:
			selfs.wndParty.Hide()

		if selfs.wndTaskBar:
			selfs.wndTaskBar.Hide()

		if selfs.wndGameButton:
			selfs.wndGameButton.Hide()

		if selfs.wndEnergyBar:
			selfs.wndEnergyBar.Hide()

		if ENABLE_DETAILS_UI:
			if selfs.wndCharacter:
				selfs.wndCharacter.Close()
		else:
			if selfs.wndCharacter:
				selfs.wndCharacter.Hide()

		if selfs.wndInventory:
			selfs.wndInventory.Hide()

		if ENABLE_DRAGON_SOUL_SYSTEM:
			if selfs.wndDragonSoul:
				selfs.wndDragonSoul.Hide()

			if selfs.wndDragonSoulRefine:
				selfs.wndDragonSoulRefine.Hide()

		if selfs.wndChat:
			selfs.wndChat.Hide()

		if selfs.wndMiniMap:
			selfs.wndMiniMap.Hide()

		if selfs.wndMessenger:
			selfs.wndMessenger.Hide()

		if selfs.wndGuild:
			selfs.wndGuild.Hide()

		if selfs.wndExpandedTaskBar:
			selfs.wndExpandedTaskBar.Hide()

		if ENABLE_EXPANDED_TASKBAR: # @@@correction047
			if selfs.wndExpandedMoneyTaskBar:
				selfs.wndExpandedMoneyTaskBar.Hide()

		if ENABLE_OFFLINE_PRIVATE_SHOP:
			if selfs.wndOfflinePrivateShopEdit:
				selfs.wndOfflinePrivateShopEdit.Hide()

			if selfs.wndOfflinePrivateShopSearch:
				selfs.wndOfflinePrivateShopSearch.Hide()

			if selfs.wndMyShopDeco:
				selfs.wndMyShopDeco.Hide()

		if ENABLE_MINI_GAME:
			if selfs.wndMiniGame:
				selfs.wndMiniGame.hide_mini_game_dialog()

		if ENABLE_12ZI:
			if selfs.wndBead:
				selfs.wndBead.Hide()
			if selfs.wnd12ziTimer:
				selfs.wnd12ziTimer.Hide()
			if selfs.wnd12ziReward:
				selfs.wnd12ziReward.Hide()

		if ENABLE_DUNGEON_INFO_SYSTEM:
			if selfs.wndDungeonInfo:
				selfs.wndDungeonInfo.Hide()

		if ENABLE_SWITCHBOT:
			if selfs.wndSwitchbot:
				selfs.wndSwitchbot.Hide()

		if ENABLE_FAST_RUHBK:
			if selfs.wndFastRuh:
				selfs.wndFastRuh.Hide()

			if selfs.wndFastBK:
				selfs.wndFastBK.Hide()

		if ENABLE_AFAR_BIOLOG:
			if selfs.wndBio:
				selfs.wndBio.Hide()

			if selfs.wndBio2:
				selfs.wndBio2.Hide()

		if ENABLE_FAST_NPC:
			if selfs.wndFastNpc:
				selfs.wndFastNpc.Hide()

	def __HideWindows(self):
		hideWindows = selfs.wndTaskBar,\
						selfs.wndCharacter,\
						selfs.wndInventory,\
						selfs.wndMiniMap,\
						selfs.wndGuild,\
						selfs.wndMessenger,\
						selfs.wndChat,\
						selfs.wndParty,\
						selfs.wndGameButton,\

		if selfs.wndEnergyBar:
			hideWindows += selfs.wndEnergyBar,

		if selfs.wndExpandedTaskBar:
			hideWindows += selfs.wndExpandedTaskBar,

		if ENABLE_DRAGON_SOUL_SYSTEM:
			hideWindows += selfs.wndDragonSoul,\
						selfs.wndDragonSoulRefine,

		if ENABLE_EXPANDED_TASKBAR: # @@@correction047
			hideWindows += selfs.wndExpandedMoneyTaskBar,

		if ENABLE_MINI_GAME:
			if selfs.wndMiniGame:
				hideWindows += selfs.wndMiniGame,

		if ENABLE_12ZI:
			if selfs.wndBead:
				hideWindows += selfs.wndBead,
			if selfs.wnd12ziTimer:
				hideWindows += selfs.wnd12ziTimer,
			if selfs.wnd12ziReward:
				hideWindows += selfs.wnd12ziReward,

		if ENABLE_DUNGEON_INFO_SYSTEM:
			if selfs.wndDungeonInfo:
				hideWindows += selfs.wndDungeonInfo,

		if ENABLE_SWITCHBOT:
			if selfs.wndSwitchbot:
				hideWindows += selfs.wndSwitchbot,

		if ENABLE_FAST_RUHBK:
			if selfs.wndFastRuh:
				hideWindows += selfs.wndFastRuh,

			if selfs.wndFastBK:
				hideWindows += selfs.wndFastBK,

		if ENABLE_AFAR_BIOLOG:
			if selfs.wndBio:
				hideWindows += selfs.wndBio,

			if selfs.wndBio2:
				hideWindows += selfs.wndBio2,

		if ENABLE_FAST_NPC:
			if selfs.wndFastNpc:
				hideWindows += selfs.wndFastNpc,

		hideWindows = filter(lambda x:x.IsShow(), hideWindows)
		map(lambda x:x.Hide(), hideWindows)
		import sys

		self.HideAllQuestButton()
		self.HideAllWhisperButton()

		if selfs.wndChat.IsEditMode():
			selfs.wndChat.CloseChat()

		return hideWindows

	def __ShowWindows(self, wnds):
		import sys
		map(lambda x:x.Show(), wnds)
		if not self.IsQBHide:
			self.ShowAllQuestButton()
		else:
			self.HideAllQuestButton()

		self.ShowAllWhisperButton()

	def __BuildKeyDict(self):
		onPressKeyDict = {}
		onPressKeyDict[app.DIK_1]	= lambda : self.__PressNumKey(1)
		onPressKeyDict[app.DIK_2]	= lambda : self.__PressNumKey(2)
		onPressKeyDict[app.DIK_3]	= lambda : self.__PressNumKey(3)
		onPressKeyDict[app.DIK_4]	= lambda : self.__PressNumKey(4)
		onPressKeyDict[app.DIK_5]	= lambda : self.__PressNumKey(5)
		onPressKeyDict[app.DIK_6]	= lambda : self.__PressNumKey(6)
		onPressKeyDict[app.DIK_7]	= lambda : self.__PressNumKey(7)
		onPressKeyDict[app.DIK_8]	= lambda : self.__PressNumKey(8)
		onPressKeyDict[app.DIK_9]	= lambda : self.__PressNumKey(9)
		onPressKeyDict[app.DIK_F1]	= lambda : self.__PressQuickSlot(4)
		onPressKeyDict[app.DIK_F2]	= lambda : self.__PressQuickSlot(5)
		onPressKeyDict[app.DIK_F3]	= lambda : self.__PressQuickSlot(6)
		onPressKeyDict[app.DIK_F4]	= lambda : self.__PressQuickSlot(7)
		if ENABLE_FAST_NPC: # @@@correction027
			onPressKeyDict[app.DIK_F5]	= lambda : self.OpenNpc()
		if ENABLE_SWITCHBOT:
			onPressKeyDict[app.DIK_F6]	= lambda : self.ToggleSwitchbotWindow()
		if ENABLE_SUPPORT_SYSTEM:
			onPressKeyDict[app.DIK_F9]	= lambda : self.OpenSupportGui()
		if ENABLE_DUNGEON_INFO_SYSTEM:
			onPressKeyDict[app.DIK_F7]	= lambda : self.OpenDungeonInfoWindow()
		if ENABLE_OFFLINE_PRIVATE_SHOP:
			onPressKeyDict[app.DIK_J]	= lambda : self.OpenShopWindow()

		onPressKeyDict[app.DIK_LALT]		= lambda : self.ShowName()
		onPressKeyDict[app.DIK_LCONTROL]	= lambda : self.ShowMouseImage()
		onPressKeyDict[app.DIK_SYSRQ]		= lambda : self.SaveScreen()
		onPressKeyDict[app.DIK_SPACE]		= lambda : self.StartAttack()

		onPressKeyDict[app.DIK_UP]			= lambda : self.MoveUp()
		onPressKeyDict[app.DIK_DOWN]		= lambda : self.MoveDown()
		onPressKeyDict[app.DIK_LEFT]		= lambda : self.MoveLeft()
		onPressKeyDict[app.DIK_RIGHT]		= lambda : self.MoveRight()
		onPressKeyDict[app.DIK_W]			= lambda : self.MoveUp()
		onPressKeyDict[app.DIK_S]			= lambda : self.MoveDown()
		onPressKeyDict[app.DIK_A]			= lambda : self.MoveLeft()
		onPressKeyDict[app.DIK_D]			= lambda : self.MoveRight()

		onPressKeyDict[app.DIK_E]			= lambda: app.RotateCamera(app.CAMERA_TO_POSITIVE)
		onPressKeyDict[app.DIK_R]			= lambda: app.ZoomCamera(app.CAMERA_TO_NEGATIVE)
		#onPressKeyDict[app.DIK_F]			= lambda: app.ZoomCamera(app.CAMERA_TO_POSITIVE)
		onPressKeyDict[app.DIK_T]			= lambda: app.PitchCamera(app.CAMERA_TO_NEGATIVE)
		onPressKeyDict[app.DIK_G]			= self.__PressGKey
		onPressKeyDict[app.DIK_Q]			= self.__PressQKey

		onPressKeyDict[app.DIK_NUMPAD9]		= lambda: app.MovieResetCamera()
		onPressKeyDict[app.DIK_NUMPAD4]		= lambda: app.MovieRotateCamera(app.CAMERA_TO_NEGATIVE)
		onPressKeyDict[app.DIK_NUMPAD6]		= lambda: app.MovieRotateCamera(app.CAMERA_TO_POSITIVE)
		onPressKeyDict[app.DIK_PGUP]		= lambda: app.MovieZoomCamera(app.CAMERA_TO_NEGATIVE)
		onPressKeyDict[app.DIK_PGDN]		= lambda: app.MovieZoomCamera(app.CAMERA_TO_POSITIVE)
		onPressKeyDict[app.DIK_NUMPAD8]		= lambda: app.MoviePitchCamera(app.CAMERA_TO_NEGATIVE)
		onPressKeyDict[app.DIK_NUMPAD2]		= lambda: app.MoviePitchCamera(app.CAMERA_TO_POSITIVE)
		onPressKeyDict[app.DIK_GRAVE]		= lambda : self.PickUpItem()
		onPressKeyDict[app.DIK_Z]			= lambda : self.PickUpItem()
		onPressKeyDict[app.DIK_C]			= lambda state = "STATUS": self.ToggleCharacterWindow(state)
		onPressKeyDict[app.DIK_V]			= lambda state = "SKILL": self.ToggleCharacterWindow(state)
		#onPressKeyDict[app.DIK_B]			= lambda state = "EMOTICON": self.ToggleCharacterWindow(state)
		onPressKeyDict[app.DIK_N]			= lambda state = "QUEST": self.ToggleCharacterWindow(state)
		onPressKeyDict[app.DIK_I]			= lambda : self.ToggleInventoryWindow()
		onPressKeyDict[app.DIK_O]			= lambda : self.ToggleDragonSoulWindowWithNoInfo()
		onPressKeyDict[app.DIK_M]			= lambda : self.__PressMKey()
		#onPressKeyDict[app.DIK_H]			= lambda : self.OpenHelpWindow()
		onPressKeyDict[app.DIK_ADD]			= lambda : self.MiniMapScaleUp()
		onPressKeyDict[app.DIK_SUBTRACT]	= lambda : self.MiniMapScaleDown()
		onPressKeyDict[app.DIK_L]			= lambda : self.ToggleChatLogWindow()
		onPressKeyDict[app.DIK_COMMA]		= lambda : self.ShowConsole()		# "`" key
		onPressKeyDict[app.DIK_LSHIFT]		= lambda : self.__SetQuickPageMode()

		# onPressKeyDict[app.DIK_J]			= lambda : self.__PressJKey()
		onPressKeyDict[app.DIK_H]			= lambda : self.__PressHKey()
		onPressKeyDict[app.DIK_B]			= lambda : self.__PressBKey()
		onPressKeyDict[app.DIK_F]			= lambda : self.__PressFKey()

		#onPressKeyDict[app.DIK_K]			= lambda : self.OpenCubeWindow()

		self.onPressKeyDict = onPressKeyDict

		onClickKeyDict = {}
		onClickKeyDict[app.DIK_UP] = lambda : self.StopUp()
		onClickKeyDict[app.DIK_DOWN] = lambda : self.StopDown()
		onClickKeyDict[app.DIK_LEFT] = lambda : self.StopLeft()
		onClickKeyDict[app.DIK_RIGHT] = lambda : self.StopRight()
		onClickKeyDict[app.DIK_SPACE] = lambda : self.EndAttack()

		onClickKeyDict[app.DIK_W] = lambda : self.StopUp()
		onClickKeyDict[app.DIK_S] = lambda : self.StopDown()
		onClickKeyDict[app.DIK_A] = lambda : self.StopLeft()
		onClickKeyDict[app.DIK_D] = lambda : self.StopRight()
		onClickKeyDict[app.DIK_Q] = lambda: app.RotateCamera(app.CAMERA_STOP)
		onClickKeyDict[app.DIK_E] = lambda: app.RotateCamera(app.CAMERA_STOP)
		onClickKeyDict[app.DIK_R] = lambda: app.ZoomCamera(app.CAMERA_STOP)
		onClickKeyDict[app.DIK_F] = lambda: app.ZoomCamera(app.CAMERA_STOP)
		onClickKeyDict[app.DIK_T] = lambda: app.PitchCamera(app.CAMERA_STOP)
		onClickKeyDict[app.DIK_G] = lambda: self.__ReleaseGKey()
		onClickKeyDict[app.DIK_NUMPAD4] = lambda: app.MovieRotateCamera(app.CAMERA_STOP)
		onClickKeyDict[app.DIK_NUMPAD6] = lambda: app.MovieRotateCamera(app.CAMERA_STOP)
		onClickKeyDict[app.DIK_PGUP] = lambda: app.MovieZoomCamera(app.CAMERA_STOP)
		onClickKeyDict[app.DIK_PGDN] = lambda: app.MovieZoomCamera(app.CAMERA_STOP)
		onClickKeyDict[app.DIK_NUMPAD8] = lambda: app.MoviePitchCamera(app.CAMERA_STOP)
		onClickKeyDict[app.DIK_NUMPAD2] = lambda: app.MoviePitchCamera(app.CAMERA_STOP)
		onClickKeyDict[app.DIK_LALT] = lambda: self.HideName()
		onClickKeyDict[app.DIK_LCONTROL] = lambda: self.HideMouseImage()
		onClickKeyDict[app.DIK_LSHIFT] = lambda: self.__SetQuickSlotMode()
		if ENABLE_GUILDWAR_KILL_COUNTER: # @@@correction034
			onPressKeyDict[app.DIK_TAB]	= lambda : self.GuildScore()
			onClickKeyDict[app.DIK_TAB]	= lambda : self.GuildScore()
		if ENABLE_GROWTH_PET_SYSTEM:
			onClickKeyDict[app.DIK_P] = lambda: self.OpenPetMainGui()
		#if constInfo.PVPMODE_ACCELKEY_ENABLE:
		#	onClickKeyDict[app.DIK_B] = lambda: self.ChangePKMode()

		self.onClickKeyDict=onClickKeyDict

	def __PressNumKey(self,num):
		if app.IsPressed(app.DIK_LCONTROL) or app.IsPressed(app.DIK_RCONTROL):
			if num >= 1 and num <= 9:
				if(chrmgr.IsPossibleEmoticon(-1)):
					chrmgr.SetEmoticon(-1,int(num)-1)
					net.SendEmoticon(int(num)-1)
		else:
			if num >= 1 and num <= 4:
				selfs.pressNumber(num-1)

	def __ClickBKey(self):
		if app.IsPressed(app.DIK_LCONTROL) or app.IsPressed(app.DIK_RCONTROL):
			return
		else:
			if constInfo.PVPMODE_ACCELKEY_ENABLE:
				self.ChangePKMode()


	def	__PressJKey(self):
		if app.IsPressed(app.DIK_LCONTROL) or app.IsPressed(app.DIK_RCONTROL):
			if player.IsMountingHorse():
				net.SendChatPacket("/unmount")
			else:
				#net.SendChatPacket("/user_horse_ride")
				if not uiPrivateShopBuilder.IsBuildingPrivateShop():
					for i in xrange(player.INVENTORY_PAGE_SIZE * player.INVENTORY_PAGE_COUNT): # @@@correction012
						if player.GetItemIndex(i) in (71114, 71116, 71118, 71120):
							net.SendItemUsePacket(i)
							break
	def	__PressHKey(self):
		if app.IsPressed(app.DIK_LCONTROL) or app.IsPressed(app.DIK_RCONTROL):
			net.SendChatPacket("/user_horse_ride")
		else:
			self.OpenHelpWindow()

	def	__PressBKey(self):
		if app.IsPressed(app.DIK_LCONTROL) or app.IsPressed(app.DIK_RCONTROL):
			net.SendChatPacket("/user_horse_back")
		else:
			state = "EMOTICON"
			self.ToggleCharacterWindow(state)

	def	__PressFKey(self):
		if app.IsPressed(app.DIK_LCONTROL) or app.IsPressed(app.DIK_RCONTROL):
			net.SendChatPacket("/user_horse_feed")
		else:
			app.ZoomCamera(app.CAMERA_TO_POSITIVE)

	def __PressGKey(self):
		if app.IsPressed(app.DIK_LCONTROL) or app.IsPressed(app.DIK_RCONTROL):
			net.SendChatPacket("/ride")
		else:
			if self.ShowNameFlag:
				self.ToggleGuildWindow()
			else:
				app.PitchCamera(app.CAMERA_TO_POSITIVE)

	def	__ReleaseGKey(self):
		app.PitchCamera(app.CAMERA_STOP)

	def __PressQKey(self):
		if app.IsPressed(app.DIK_LCONTROL) or app.IsPressed(app.DIK_RCONTROL):
			if 0 == self.IsQBHide:
				self.IsQBHide = 1
				self.HideAllQuestButton()
			else:
				self.IsQBHide = 0
				self.ShowAllQuestButton()
		else:
			app.RotateCamera(app.CAMERA_TO_NEGATIVE)

	def __PressMKey(self):
		if app.IsPressed(app.DIK_LALT) or app.IsPressed(app.DIK_RALT):
			self.ToggleMessenger()
		else:
			self.ToggleMiniMap()

	def __SetQuickSlotMode(self):
		selfs.pressNumber=ui.__mem_func__(self.__PressQuickSlot)

	def __SetQuickPageMode(self):
		selfs.pressNumber=ui.__mem_func__(self.__SelectQuickPage)

	def __PressQuickSlot(self, localSlotIndex):
		if localeInfo.IsARABIC():
			if 0 <= localSlotIndex and localSlotIndex < 4:
				player.RequestUseLocalQuickSlot(3-localSlotIndex)
			else:
				player.RequestUseLocalQuickSlot(11-localSlotIndex)
		else:
			player.RequestUseLocalQuickSlot(localSlotIndex)

	def __SelectQuickPage(self, pageIndex):
		selfs.quickSlotPageIndex = pageIndex
		player.SetQuickPage(pageIndex)

	def ToggleDebugInfo(self):
		self.isShowDebugInfo = not self.isShowDebugInfo

		if self.isShowDebugInfo:
			self.PrintCoord.Show()
			self.FrameRate.Show()
			self.Pitch.Show()
			self.Splat.Show()
			self.TextureNum.Show()
			self.ObjectNum.Show()
			self.ViewDistance.Show()
			self.PrintMousePos.Show()
		else:
			self.PrintCoord.Hide()
			self.FrameRate.Hide()
			self.Pitch.Hide()
			self.Splat.Hide()
			self.TextureNum.Hide()
			self.ObjectNum.Hide()
			self.ViewDistance.Hide()
			self.PrintMousePos.Hide()

	def __BuildDebugInfo(self):
		self.PrintCoord = ui.TextLine()
		self.PrintCoord.SetFontName(localeInfo.UI_DEF_FONT)
		self.PrintCoord.SetPosition(wndMgr.GetScreenWidth() - 270, 0)

		self.FrameRate = ui.TextLine()
		self.FrameRate.SetFontName(localeInfo.UI_DEF_FONT)
		self.FrameRate.SetPosition(wndMgr.GetScreenWidth() - 270, 20)

		self.Pitch = ui.TextLine()
		self.Pitch.SetFontName(localeInfo.UI_DEF_FONT)
		self.Pitch.SetPosition(wndMgr.GetScreenWidth() - 270, 40)

		self.Splat = ui.TextLine()
		self.Splat.SetFontName(localeInfo.UI_DEF_FONT)
		self.Splat.SetPosition(wndMgr.GetScreenWidth() - 270, 60)

		self.PrintMousePos = ui.TextLine()
		self.PrintMousePos.SetFontName(localeInfo.UI_DEF_FONT)
		self.PrintMousePos.SetPosition(wndMgr.GetScreenWidth() - 270, 80)

		self.TextureNum = ui.TextLine()
		self.TextureNum.SetFontName(localeInfo.UI_DEF_FONT)
		self.TextureNum.SetPosition(wndMgr.GetScreenWidth() - 270, 100)

		self.ObjectNum = ui.TextLine()
		self.ObjectNum.SetFontName(localeInfo.UI_DEF_FONT)
		self.ObjectNum.SetPosition(wndMgr.GetScreenWidth() - 270, 120)

		self.ViewDistance = ui.TextLine()
		self.ViewDistance.SetFontName(localeInfo.UI_DEF_FONT)
		self.ViewDistance.SetPosition(0, 0)

	def __NotifyError(self, msg):
		chat.AppendChat(chat.CHAT_TYPE_INFO, msg)

	def ChangePKMode(self):
		if not app.IsPressed(app.DIK_LCONTROL):
			return

		if player.GetStatus(player.LEVEL)<constInfo.PVPMODE_PROTECTED_LEVEL:
			self.__NotifyError(localeInfo.OPTION_PVPMODE_PROTECT % (constInfo.PVPMODE_PROTECTED_LEVEL))
			return

		curTime = app.GetTime()
		if curTime - selfs.lastPKModeSendedTime < constInfo.PVPMODE_ACCELKEY_DELAY:
			return

		selfs.lastPKModeSendedTime = curTime
		curPKMode = player.GetPKMode()
		nextPKMode = curPKMode + 1

		if nextPKMode == player.PK_MODE_PROTECT:
			if 0 == player.GetGuildID():
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.OPTION_PVPMODE_CANNOT_SET_GUILD_MODE)
				nextPKMode = 0
			else:
				nextPKMode = player.PK_MODE_GUILD

		elif nextPKMode == player.PK_MODE_MAX_NUM:
			nextPKMode = 0

		net.SendChatPacket("/PKMode " + str(nextPKMode))
		print "/PKMode " + str(nextPKMode)

	def OnChangePKMode(self):
		selfs.wndCharacter.RefreshAlignment()
		selfs.wndDialogSystem.OnChangePKMode()
		try:
			self.__NotifyError(localeInfo.OPTION_PVPMODE_MESSAGE_DICT[player.GetPKMode()])
		except KeyError:
			print "UNKNOWN PVPMode[%d]" % (player.GetPKMode())

		if constInfo.PVPMODE_TEST_ENABLE:
			curPKMode = player.GetPKMode()
			alignment, grade = chr.testGetPKData()
			self.pkModeNameDict = { 0 : "PEACE", 1 : "REVENGE", 2 : "FREE", 3 : "PROTECT", }
			self.testPKMode.SetText("Current PK Mode : " + self.pkModeNameDict.get(curPKMode, "UNKNOWN"))
			self.testAlignment.SetText("Current Alignment : " + str(alignment) + " (" + localeInfo.TITLE_NAME_LIST[grade] + ")")

	def StartGame(self):
		self.RefreshInventory()
		self.RefreshEquipment()
		self.RefreshCharacter()
		self.RefreshSkill()

	def CheckGameButton(self):
		if selfs.wndGameButton:
			selfs.wndGameButton.CheckGameButton()

	def RefreshAlignment(self):
		selfs.wndCharacter.RefreshAlignment()

	def RefreshStatus(self):
		self.CheckGameButton()

		if selfs.wndTaskBar:
			selfs.wndTaskBar.RefreshStatus()
		if selfs.wndCharacter:
			selfs.wndCharacter.RefreshStatus()

		if ENABLE_EXPANDED_TASKBAR and selfs.wndExpandedMoneyTaskBar: # @@@correction047
			selfs.wndExpandedMoneyTaskBar.RefreshStatus()
		else:
			if selfs.wndInventory:
				selfs.wndInventory.RefreshStatus()

		if selfs.wndEnergyBar:
			selfs.wndEnergyBar.RefreshStatus()

		if ENABLE_DRAGON_SOUL_SYSTEM:
			if selfs.wndDragonSoul:
				selfs.wndDragonSoul.RefreshStatus()

		if selfs.wndPlayerGauge:
			if selfs.wndPlayerGauge:
				selfs.wndPlayerGauge.RefreshGauge()

	def RefreshStamina(self):
		selfs.wndTaskBar.RefreshStamina()

	def RefreshSkill(self):
		self.CheckGameButton()
		if selfs.wndCharacter:
			selfs.wndCharacter.RefreshSkill()
		if selfs.wndTaskBar:
			selfs.wndTaskBar.RefreshSkill()

	if not ENABLE_QUEST_CATEGORY:
		def RefreshQuest(self):
			if selfs.wndCharacter:
				selfs.wndCharacter.RefreshQuest()

	def RefreshMessenger(self):
		if selfs.wndMessenger:
			selfs.wndMessenger.RefreshMessenger()

	def RefreshGuildInfoPage(self):
		if selfs.wndGuild:
			selfs.wndGuild.RefreshGuildInfoPage()

	def RefreshGuildBoardPage(self):
		if selfs.wndGuild:
			selfs.wndGuild.RefreshGuildBoardPage()

	def RefreshGuildMemberPage(self):
		if selfs.wndGuild:
			selfs.wndGuild.RefreshGuildMemberPage()

	def RefreshGuildMemberPageGradeComboBox(self):
		if selfs.wndGuild:
			selfs.wndGuild.RefreshGuildMemberPageGradeComboBox()

	def RefreshGuildSkillPage(self):
		if selfs.wndGuild:
			selfs.wndGuild.RefreshGuildSkillPage()

	def RefreshGuildGradePage(self):
		if selfs.wndGuild:
			selfs.wndGuild.RefreshGuildGradePage()

	def RefreshMobile(self):
		if selfs.wndDialogSystem:
			self.wndDialogSystem.RefreshMobile()

	def OnMobileAuthority(self):
		if selfs.wndDialogSystem:
			selfs.wndDialogSystem.OnMobileAuthority()

	def OnBlockMode(self, mode):
		if selfs.wndDialogSystem:
			selfs.wndDialogSystem.OnBlockMode(mode)

	def OpenQuestWindow(self, skin, idx):
		if constInfo.INPUT_IGNORE == 1:
			return

		wnds = ()
		q = uiQuest.QuestDialog(skin, idx)
		q.SetWindowName("QuestWindow" + str(idx))
		q.Show()
		if skin:
			q.Lock()
			wnds = self.__HideWindows()
			q.AddOnDoneEvent(lambda tmp_self, args=wnds: self.__ShowWindows(args))
		if skin:
			q.AddOnCloseEvent(q.Unlock)
		q.AddOnCloseEvent(lambda key = selfs.wndQuestWindowNewKey:ui.__mem_func__(self.RemoveQuestDialog)(key))
		selfs.wndQuestWindow[selfs.wndQuestWindowNewKey] = q

		selfs.wndQuestWindowNewKey = selfs.wndQuestWindowNewKey + 1

	def RemoveQuestDialog(self, key):
		del selfs.wndQuestWindow[key]

	def BINARY_ClearQuest(self, index):
		btn = self.__FindQuestButton(index)
		if 0 != btn:
			self.__DestroyQuestButton(btn)

	def RecvQuest(self, index, name):
		if ENABLE_QUEST_CATEGORY:
			selfs.wndCharacter.questCategory.RecvQuest(self.BINARY_RecvQuest, index, name)
		else:
			self.BINARY_RecvQuest(index, name, "file", localeInfo.GetLetterImageName())

	def BINARY_RecvQuest(self, index, name, iconType, iconName):
		btn = self.__FindQuestButton(index)
		if 0 != btn:
			self.__DestroyQuestButton(btn)

		btn = uiWhisper.WhisperButton()
		import item
		if "item"==iconType:
			item.SelectItem(int(iconName))
			buttonImageFileName=item.GetIconImageFileName()
		else:
			buttonImageFileName=iconName

		if localeInfo.IsEUROPE():
			if "highlight" == iconType:
				btn.SetUpVisual("locale/ymir_ui/highlighted_quest.tga")
				btn.SetOverVisual("locale/ymir_ui/highlighted_quest_r.tga")
				btn.SetDownVisual("locale/ymir_ui/highlighted_quest_r.tga")
			else:
				btn.SetUpVisual(localeInfo.GetLetterCloseImageName())
				btn.SetOverVisual(localeInfo.GetLetterOpenImageName())
				btn.SetDownVisual(localeInfo.GetLetterOpenImageName())
		else:
			btn.SetUpVisual(buttonImageFileName)
			btn.SetOverVisual(buttonImageFileName)
			btn.SetDownVisual(buttonImageFileName)
			btn.Flash()

		if localeInfo.IsARABIC():
			btn.SetToolTipText(name, 0, 35)
			btn.ToolTipText.SetHorizontalAlignCenter()
		else:
			btn.SetToolTipText(name, -20, 35)
			btn.ToolTipText.SetHorizontalAlignLeft()

		btn.SetEvent(ui.__mem_func__(self.__StartQuest), btn)
		btn.Show()

		btn.index = index
		btn.name = name

		self.questButtonList.insert(0, btn)
		self.__ArrangeQuestButton()

	def __ArrangeQuestButton(self):
		screenWidth = wndMgr.GetScreenWidth()
		screenHeight = wndMgr.GetScreenHeight()

		if selfs.wndParty.IsShow():
			xPos = 100 + 30
		else:
			xPos = 20

		if localeInfo.IsARABIC():
			xPos = xPos + 15

		yPos = 170 * screenHeight / 600
		yCount = (screenHeight - 330) / 63

		count = 0
		for btn in self.questButtonList:

			btn.SetPosition(xPos + (int(count/yCount) * 100), yPos + (count%yCount * 63))
			count += 1
			if self.IsQBHide:
				btn.Hide()
			else:
				btn.Show()

	def __StartQuest(self, btn):
		event.QuestButtonClick(btn.index)
		self.__DestroyQuestButton(btn)

	def __FindQuestButton(self, index):
		for btn in self.questButtonList:
			if btn.index == index:
				return btn

		return 0

	def __DestroyQuestButton(self, btn):
		btn.SetEvent(0)
		self.questButtonList.remove(btn)
		self.__ArrangeQuestButton()

	def HideAllQuestButton(self):
		for btn in self.questButtonList:
			btn.Hide()

	def ShowAllQuestButton(self):
		for btn in self.questButtonList:
			btn.Show()

	def AskGuildName(self):
		guildNameBoard = uiCommon.InputDialog()
		guildNameBoard.SetTitle(localeInfo.GUILD_NAME)
		guildNameBoard.SetAcceptEvent(ui.__mem_func__(self.ConfirmGuildName))
		guildNameBoard.SetCancelEvent(ui.__mem_func__(self.CancelGuildName))
		guildNameBoard.Open()

		self.guildNameBoard = guildNameBoard

	def ConfirmGuildName(self):
		guildName = self.guildNameBoard.GetText()
		if not guildName:
			return

		if net.IsInsultIn(guildName):
			self.PopupMessage(localeInfo.GUILD_CREATE_ERROR_INSULT_NAME)
			return

		net.SendAnswerMakeGuildPacket(guildName)
		self.guildNameBoard.Close()
		self.guildNameBoard = None
		return True

	def CancelGuildName(self):
		self.guildNameBoard.Close()
		self.guildNameBoard = None
		return True

	def PopupMessage(self, msg):
		selfs.wndStream.popupWindow.Close()
		selfs.wndStream.popupWindow.Open(msg, 0, localeInfo.UI_OK)

	def OpenRefineDialog(self, targetItemPos, nextGradeItemVnum, cost, prob, type = 0):
		selfs.wndDialogRefineNew.Open(targetItemPos, nextGradeItemVnum, cost, prob, type)

	def AppendMaterialToRefineDialog(self, vnum, count):
		selfs.wndDialogRefineNew.AppendMaterial(vnum, count)

	def RunUseSkillEvent(self, slotIndex, coolTime):
		selfs.wndCharacter.OnUseSkill(slotIndex, coolTime)
		selfs.wndTaskBar.OnUseSkill(slotIndex, coolTime)
		selfs.wndGuild.OnUseSkill(slotIndex, coolTime)

	def ClearAffects(self):
		selfs.wndAffectShower.ClearAffects()

	def SetAffect(self, affect):
		selfs.wndAffectShower.SetAffect(affect)

	def ResetAffect(self, affect):
		selfs.wndAffectShower.ResetAffect(affect)

	def BINARY_NEW_AddAffect(self, type, pointIdx, value, duration):
		selfs.wndAffectShower.BINARY_NEW_AddAffect(type, pointIdx, value, duration)
		if chr.NEW_AFFECT_DRAGON_SOUL_DECK1 == type or chr.NEW_AFFECT_DRAGON_SOUL_DECK2 == type:
			if ENABLE_DRAGON_SOUL_SYSTEM:
				selfs.wndDragonSoul.ActivateDragonSoulByExtern(type - chr.NEW_AFFECT_DRAGON_SOUL_DECK1)
		elif chr.NEW_AFFECT_DRAGON_SOUL_QUALIFIED == type:
			self.BINARY_DragonSoulGiveQuilification()

	def BINARY_NEW_RemoveAffect(self, type, pointIdx):
		selfs.wndAffectShower.BINARY_NEW_RemoveAffect(type, pointIdx)
		if chr.NEW_AFFECT_DRAGON_SOUL_DECK1 == type or chr.NEW_AFFECT_DRAGON_SOUL_DECK2 == type:
			if ENABLE_DRAGON_SOUL_SYSTEM:
				selfs.wndDragonSoul.DeactivateDragonSoul()

	def ActivateSkillSlot(self, slotIndex):
		selfs.wndCharacter.OnActivateSkill(slotIndex)
		selfs.wndTaskBar.OnActivateSkill(slotIndex)

	def DeactivateSkillSlot(self, slotIndex):
		selfs.wndCharacter.OnDeactivateSkill(slotIndex)
		selfs.wndTaskBar.OnDeactivateSkill(slotIndex)

	def RefreshEquipment(self):
		self.RefreshInventory()

	def RefreshInventory(self):
		if selfs.wndTaskBar:
			selfs.wndTaskBar.RefreshQuickSlot()
		if selfs.wndInventory:
			selfs.wndInventory.RefreshItemSlot()
		if ENABLE_DRAGON_SOUL_SYSTEM:
			if selfs.wndDragonSoul:
				selfs.wndDragonSoul.RefreshItemSlot()

	def RefreshCharacter(self):
		if selfs.wndCharacter:
			selfs.wndCharacter.RefreshCharacter()
		if selfs.wndTaskBar:
			selfs.wndTaskBar.RefreshQuickSlot()

	def OnGameOver(self):
		self.CloseTargetBoard()
		self.OpenRestartDialog()

	def OpenRestartDialog(self):
		if selfs.wndDialogRestart:
			selfs.wndDialogRestart.OpenDialog()
			selfs.wndDialogRestart.SetTop()

	def ChangeCurrentSkill(self, skillSlotNumber):
		if selfs.wndTaskBar:
			selfs.wndTaskBar.OnChangeCurrentSkill(skillSlotNumber)

	def SetPCTargetBoard(self, vid, name):
		if selfs.wndTargetBoard:
			selfs.wndTargetBoard.Open(vid, name)
		if app.IsPressed(app.DIK_LCONTROL):
			if not player.IsSameEmpire(vid):
				return

			if player.IsMainCharacterIndex(vid):
				return
			elif chr.INSTANCE_TYPE_BUILDING == chr.GetInstanceType(vid):
				return

			self.OpenWhisperDialog(name)

	def RefreshTargetBoardByVID(self, vid):
		if selfs.wndTargetBoard:
			selfs.wndTargetBoard.RefreshByVID(vid)

	def RefreshTargetBoardByName(self, name):
		if selfs.wndTargetBoard:
			selfs.wndTargetBoard.RefreshByName(name)

	def __RefreshTargetBoard(self):
		if selfs.wndTargetBoard:
			selfs.wndTargetBoard.Refresh()

	if ENABLE_VIEW_TARGET_DECIMAL_HP: # @@@correction033
		def SetHPTargetBoard(self, vid, hpPercentage, iMinHP, iMaxHP):
			if selfs.wndTargetBoard:
				if vid != selfs.wndTargetBoard.GetTargetVID():
					selfs.wndTargetBoard.ResetTargetBoard()
					selfs.wndTargetBoard.SetEnemyVID(vid)
				
				selfs.wndTargetBoard.SetHP(hpPercentage, iMinHP, iMaxHP)
				selfs.wndTargetBoard.Show()
	else:
		def SetHPTargetBoard(self, vid, hpPercentage):
			if selfs.wndTargetBoard:
				if vid != selfs.wndTargetBoard.GetTargetVID():
					selfs.wndTargetBoard.ResetTargetBoard()
					selfs.wndTargetBoard.SetEnemyVID(vid)

				selfs.wndTargetBoard.SetHP(hpPercentage)
				selfs.wndTargetBoard.Show()

	def CloseTargetBoardIfDifferent(self, vid):
		if selfs.wndTargetBoard:
			if vid != selfs.wndTargetBoard.GetTargetVID():
				selfs.wndTargetBoard.Close()

	def CloseTargetBoard(self):
		if selfs.wndTargetBoard:
			selfs.wndTargetBoard.Close()

	def OpenEquipmentDialog(self, vid):
		import uiEquipmentDialog
		dlg = uiEquipmentDialog.EquipmentDialog()
		dlg.SetCloseEvent(ui.__mem_func__(self.CloseEquipmentDialog))
		dlg.Open(vid)

		self.equipmentDialogDict[vid] = dlg

	def SetEquipmentDialogItem(self, vid, slotIndex, vnum, count):
		if not vid in self.equipmentDialogDict:
			return

		self.equipmentDialogDict[vid].SetEquipmentDialogItem(slotIndex, vnum, count)

	def SetEquipmentDialogSocket(self, vid, slotIndex, socketIndex, value):
		if not vid in self.equipmentDialogDict:
			return

		self.equipmentDialogDict[vid].SetEquipmentDialogSocket(slotIndex, socketIndex, value)

	def SetEquipmentDialogAttr(self, vid, slotIndex, attrIndex, type, value):
		if not vid in self.equipmentDialogDict:
			return

		self.equipmentDialogDict[vid].SetEquipmentDialogAttr(slotIndex, attrIndex, type, value)

	def ShowMapName(self, mapName, x, y):
		if selfs.wndMapNameShower:
			selfs.wndMapNameShower.ShowMapName(mapName, x, y)

		if selfs.wndMiniMap:
			selfs.wndMiniMap.SetMapName(mapName)

	def BINARY_OpenAtlasWindow(self):
		if selfs.wndMiniMap:
			selfs.wndMiniMap.ShowAtlas()

	def OnRecvWhisper(self, mode, name, line, empire, language):
		if mode == chat.WHISPER_TYPE_GM and not self.listGMName.has_key(name):
			self.listGMName[name] = "GM"
		# TraceError(line)
		chat.AppendWhisper(mode, name, line)
		self.RecvWhisper(name, empire, language)

	def OnRecvWhisperSystemMessage(self, mode, name, line):
		chat.AppendWhisper(chat.WHISPER_TYPE_SYSTEM, name, line)
		self.RecvWhisper(name)

	def OnRecvWhisperError(self, mode, name, line):
		if localeInfo.WHISPER_ERROR.has_key(mode):
			chat.AppendWhisper(chat.WHISPER_TYPE_SYSTEM, name, localeInfo.WHISPER_ERROR[mode](name))
		else:
			chat.AppendWhisper(chat.WHISPER_TYPE_SYSTEM, name, "Whisper Unknown Error(mode=%d, name=%s)" % (mode, name))
		self.RecvWhisper(name)

	def OnPickMoney(self, money):
		chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.GAME_PICK_MONEY % (money))

	def OnShopError(self, type):
		try:
			self.PopupMessage(localeInfo.SHOP_ERROR_DICT[type])
		except KeyError:
			self.PopupMessage(localeInfo.SHOP_ERROR_UNKNOWN % (type))

	def OnSafeBoxError(self):
		self.PopupMessage(localeInfo.SAFEBOX_ERROR)

	def OnFishingSuccess(self, isFish, fishName):
		chat.AppendChatWithDelay(chat.CHAT_TYPE_INFO, localeInfo.FISHING_SUCCESS(isFish, fishName), 2000)

	def OnFishingNotifyUnknown(self):
		chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.FISHING_UNKNOWN)

	def OnFishingWrongPlace(self):
		chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.FISHING_WRONG_PLACE)

	def OnFishingNotify(self, isFish, fishName):
		chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.FISHING_NOTIFY(isFish, fishName))

	def OnFishingFailure(self):
		chat.AppendChatWithDelay(chat.CHAT_TYPE_INFO, localeInfo.FISHING_FAILURE, 2000)

	def OnCannotPickItem(self):
		chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.GAME_CANNOT_PICK_ITEM)

	def OnCannotMining(self):
		chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.GAME_CANNOT_MINING)

	def OnCannotUseSkill(self, vid, type):
		if localeInfo.USE_SKILL_ERROR_TAIL_DICT.has_key(type):
			textTail.RegisterInfoTail(vid, localeInfo.USE_SKILL_ERROR_TAIL_DICT[type])

		if localeInfo.USE_SKILL_ERROR_CHAT_DICT.has_key(type):
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.USE_SKILL_ERROR_CHAT_DICT[type])

	def	OnCannotShotError(self, vid, type):
		textTail.RegisterInfoTail(vid, localeInfo.SHOT_ERROR_TAIL_DICT.get(type, localeInfo.SHOT_ERROR_UNKNOWN % (type)))

	def StartPointReset(self):
		selfs.wndDialogPointReset.Show()
		selfs.wndDialogPointReset.SetTop()

	def StartShop(self, vid):
		selfs.wndInventory.Show()
		selfs.wndInventory.SetTop()
		selfs.wndDialogShop.Open(vid)
		selfs.wndDialogShop.SetTop()
		if ENABLE_EXPANDED_TASKBAR and selfs.wndExpandedMoneyTaskBar: # @@@correction047
			selfs.wndExpandedMoneyTaskBar.Show()

	def EndShop(self):
		selfs.wndDialogShop.Close()

	def RefreshShop(self):
		selfs.wndDialogShop.Refresh()

	def SetShopSellingPrice(self, Price):
		pass

	def StartExchange(self):
		selfs.wndDialogExchange.OpenDialog()
		selfs.wndDialogExchange.Refresh()

	def EndExchange(self):
		selfs.wndDialogExchange.CloseDialog()

	def RefreshExchange(self):
		selfs.wndDialogExchange.Refresh()

	def RecvPartyInviteQuestion(self, leaderVID, leaderName):
		# @@@correction045 BEGIN
		partyInviteQuestionDialog = uiCommon.QuestionDialogWithTimeLimit()
		partyInviteQuestionDialog.SetText1(leaderName + localeInfo.PARTY_DO_YOU_JOIN)
		partyInviteQuestionDialog.SetTimeOverMsg(localeInfo.PARTY_ANSWER_TIMEOVER)
		partyInviteQuestionDialog.SetTimeOverEvent(self.AnswerPartyInvite, False)
		partyInviteQuestionDialog.SetAcceptEvent(lambda arg=True: self.AnswerPartyInvite(arg))
		partyInviteQuestionDialog.SetCancelEvent(lambda arg=False: self.AnswerPartyInvite(arg))
		partyInviteQuestionDialog.Open(10)
		# @@@correction045 END
		partyInviteQuestionDialog.partyLeaderVID = leaderVID
		self.partyInviteQuestionDialog = partyInviteQuestionDialog

	def AnswerPartyInvite(self, answer):
		if not self.partyInviteQuestionDialog:
			return

		partyLeaderVID = self.partyInviteQuestionDialog.partyLeaderVID

		distance = player.GetCharacterDistance(partyLeaderVID)
		if distance < 0.0 or distance > 5000:
			answer = False

		net.SendPartyInviteAnswerPacket(partyLeaderVID, answer)

		self.partyInviteQuestionDialog.Close()
		self.partyInviteQuestionDialog = None

	def AddPartyMember(self, pid, name):
		selfs.wndParty.AddPartyMember(pid, name)
		self.__ArrangeQuestButton()

	def UpdatePartyMemberInfo(self, pid):
		selfs.wndParty.UpdatePartyMemberInfo(pid)

	def RemovePartyMember(self, pid):
		selfs.wndParty.RemovePartyMember(pid)
		self.__ArrangeQuestButton()
		self.__RefreshTargetBoard()

	def LinkPartyMember(self, pid, vid):
		selfs.wndParty.LinkPartyMember(pid, vid)

	def UnlinkPartyMember(self, pid):
		selfs.wndParty.UnlinkPartyMember(pid)

	def UnlinkAllPartyMember(self):
		selfs.wndParty.UnlinkAllPartyMember()

	def ExitParty(self):
		selfs.wndParty.ExitParty()
		self.__ArrangeQuestButton()
		self.RefreshTargetBoardByVID(selfs.wndTargetBoard.GetTargetVID())

	def ChangePartyParameter(self, distributionMode):
		selfs.wndParty.ChangePartyParameter(distributionMode)

	def OnMessengerAddFriendQuestion(self, name):
		# @@@correction045 BEGIN
		messengerAddFriendQuestion = uiCommon.QuestionDialogWithTimeLimit()
		messengerAddFriendQuestion.SetText1(localeInfo.MESSENGER_DO_YOU_ACCEPT_ADD_FRIEND % (name))
		messengerAddFriendQuestion.SetTimeOverMsg(localeInfo.MESSENGER_ADD_FRIEND_ANSWER_TIMEOVER)
		messengerAddFriendQuestion.SetTimeOverEvent(self.OnDenyAddFriend)
		messengerAddFriendQuestion.SetAcceptEvent(ui.__mem_func__(self.OnAcceptAddFriend))
		messengerAddFriendQuestion.SetCancelEvent(ui.__mem_func__(self.OnDenyAddFriend))
		messengerAddFriendQuestion.Open(10)
		# @@@correction045 END
		messengerAddFriendQuestion.name = name
		self.messengerAddFriendQuestion = messengerAddFriendQuestion

	def OnAcceptAddFriend(self):
		name = self.messengerAddFriendQuestion.name
		net.SendChatPacket("/messenger_auth y " + name)
		self.OnCloseAddFriendQuestionDialog()
		return True

	def OnDenyAddFriend(self):
		name = self.messengerAddFriendQuestion.name
		net.SendChatPacket("/messenger_auth n " + name)
		self.OnCloseAddFriendQuestionDialog()
		return True

	def OnCloseAddFriendQuestionDialog(self):
		self.messengerAddFriendQuestion.Close()
		self.messengerAddFriendQuestion = None
		return True

	def OpenSafeboxWindow(self, size):
		selfs.wndDialogPassword.CloseDialog()
		selfs.wndSafebox.ShowWindow(size)

	def RefreshSafebox(self):
		selfs.wndSafebox.RefreshSafebox()

	def RefreshSafeboxMoney(self):
		selfs.wndSafebox.RefreshSafeboxMoney()

	def OpenMallWindow(self, size):
		selfs.wndDialogPassword.CloseDialog()
		selfs.wndMall.ShowWindow(size)

	def RefreshMall(self):
		selfs.wndMall.RefreshMall()

	def RecvGuildInviteQuestion(self, guildID, guildName):
		guildInviteQuestionDialog = uiCommon.QuestionDialog()
		guildInviteQuestionDialog.SetText(guildName + localeInfo.GUILD_DO_YOU_JOIN)
		guildInviteQuestionDialog.SetAcceptEvent(lambda arg=True: self.AnswerGuildInvite(arg))
		guildInviteQuestionDialog.SetCancelEvent(lambda arg=False: self.AnswerGuildInvite(arg))
		guildInviteQuestionDialog.Open()
		guildInviteQuestionDialog.guildID = guildID
		self.guildInviteQuestionDialog = guildInviteQuestionDialog

	def AnswerGuildInvite(self, answer):

		if not self.guildInviteQuestionDialog:
			return

		guildLeaderVID = self.guildInviteQuestionDialog.guildID
		net.SendGuildInviteAnswerPacket(guildLeaderVID, answer)

		self.guildInviteQuestionDialog.Close()
		self.guildInviteQuestionDialog = None

	def DeleteGuild(self):
		selfs.wndMessenger.ClearGuildMember()
		selfs.wndGuild.DeleteGuild()

	def BINARY_ActEmotion(self, emotionIndex):
		if selfs.wndCharacter:
			selfs.wndCharacter.ActEmotion(emotionIndex)

	def CheckFocus(self):
		if False == self.IsFocus():
			if True == selfs.wndChat.IsEditMode():
				self.ToggleChat()

			self.SetFocus()

	def SaveScreen(self):
		if SCREENSHOT_CWDSAVE:
			if not os.path.exists(os.getcwd()+os.sep+"screenshot"):
				os.mkdir(os.getcwd()+os.sep+"screenshot")

			(succeeded, name) = grp.SaveScreenShotToPath(os.getcwd()+os.sep+"screenshot"+os.sep)
		elif SCREENSHOT_DIR:
			(succeeded, name) = grp.SaveScreenShot(SCREENSHOT_DIR)
		else:
			(succeeded, name) = grp.SaveScreenShot()

		if succeeded:
			pass
			"""
			chat.AppendChat(chat.CHAT_TYPE_INFO, name + localeInfo.SCREENSHOT_SAVE1)
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.SCREENSHOT_SAVE2)
			"""
		else:
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.SCREENSHOT_SAVE_FAILURE)

	def ShowConsole(self):
		if debugInfo.IsDebugMode() or True == self.consoleEnable:
			player.EndKeyWalkingImmediately()
			self.console.OpenWindow()

	def ShowName(self):
		self.ShowNameFlag = True
		selfs.wndPlayerGauge.EnableShowAlways()
		player.SetQuickPage(selfs.quickSlotPageIndex+1)

	def __IsShowName(self):
		if systemSetting.IsAlwaysShowName():
			return True

		if self.ShowNameFlag:
			return True

		return False
	
	def HideName(self):
		self.ShowNameFlag = False
		selfs.wndPlayerGauge.DisableShowAlways()
		player.SetQuickPage(selfs.quickSlotPageIndex)

	def ShowMouseImage(self):
		selfs.wndTaskBar.ShowMouseImage()

	def HideMouseImage(self):
		selfs.wndTaskBar.HideMouseImage()

	def StartAttack(self):
		player.SetAttackKeyState(True)

	def EndAttack(self):
		player.SetAttackKeyState(False)

	def MoveUp(self):
		player.SetSingleDIKKeyState(app.DIK_UP, True)

	def MoveDown(self):
		player.SetSingleDIKKeyState(app.DIK_DOWN, True)

	def MoveLeft(self):
		player.SetSingleDIKKeyState(app.DIK_LEFT, True)

	def MoveRight(self):
		player.SetSingleDIKKeyState(app.DIK_RIGHT, True)

	def StopUp(self):
		player.SetSingleDIKKeyState(app.DIK_UP, False)

	def StopDown(self):
		player.SetSingleDIKKeyState(app.DIK_DOWN, False)

	def StopLeft(self):
		player.SetSingleDIKKeyState(app.DIK_LEFT, False)

	def StopRight(self):
		player.SetSingleDIKKeyState(app.DIK_RIGHT, False)

	def PickUpItem(self):
		player.PickCloseItem()

	def OnKeyDown(self, key):
		if selfs.wndWeb and selfs.wndWeb.IsShow():
			return

		if key == app.DIK_ESC: # @@@correction043
			self.RequestDropItem(False)
			constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

		try:
			self.onPressKeyDict[key]()
		except KeyError:
			pass
		except:
			raise

		return True

	def OnKeyUp(self, key):
		try:
			if self.onClickKeyDict:
				self.onClickKeyDict[key]()
		except KeyError:
			pass
		except:
			raise

		return True

	def OnMouseLeftButtonDown(self):
		if self.BUILD_OnMouseLeftButtonDown():
			return

		if mouseModule.mouseController.isAttached():
			self.CheckFocus()
		else:
			hyperlink = ui.GetHyperlink()
			if hyperlink:
				return
			else:
				self.CheckFocus()
				player.SetMouseState(player.MBT_LEFT, player.MBS_PRESS);

		return True

	def OnMouseLeftButtonUp(self):
		if self.BUILD_OnMouseLeftButtonUp():
			return

		if mouseModule.mouseController.isAttached():
			attachedType = mouseModule.mouseController.GetAttachedType()
			attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()
			attachedItemSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
			attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()

			if player.SLOT_TYPE_QUICK_SLOT == attachedType:
				player.RequestDeleteGlobalQuickSlot(attachedItemSlotPos)

			elif player.SLOT_TYPE_INVENTORY == attachedType:

				if player.ITEM_MONEY == attachedItemIndex:
					self.__PutMoney(attachedType, attachedItemCount, self.PickingCharacterIndex)
				else:
					self.__PutItem(attachedType, attachedItemIndex, attachedItemSlotPos, attachedItemCount, self.PickingCharacterIndex)

			elif player.SLOT_TYPE_DRAGON_SOUL_INVENTORY == attachedType:
				self.__PutItem(attachedType, attachedItemIndex, attachedItemSlotPos, attachedItemCount, self.PickingCharacterIndex)

			mouseModule.mouseController.DeattachObject()

		else:
			hyperlink = ui.GetHyperlink()
			if hyperlink:
				if app.IsPressed(app.DIK_LALT):
					link = chat.GetLinkFromHyperlink(hyperlink)
					ime.PasteString(link)
				else:
					self.MakeHyperlinkTooltip(hyperlink)
				return
			else:
				player.SetMouseState(player.MBT_LEFT, player.MBS_CLICK)
		return True

	def __PutItem(self, attachedType, attachedItemIndex, attachedItemSlotPos, attachedItemCount, dstChrID):
		if player.SLOT_TYPE_INVENTORY == attachedType or player.SLOT_TYPE_DRAGON_SOUL_INVENTORY == attachedType:
			attachedInvenType = player.SlotTypeToInvenType(attachedType)
			if True == chr.HasInstance(self.PickingCharacterIndex) and player.GetMainCharacterIndex() != dstChrID:
				if player.IsEquipmentSlot(attachedItemSlotPos) and player.SLOT_TYPE_DRAGON_SOUL_INVENTORY != attachedType: # @@@correction044
					selfs.wndStream.popupWindow.Close()
					selfs.wndStream.popupWindow.Open(localeInfo.EXCHANGE_FAILURE_EQUIP_ITEM, 0, localeInfo.UI_OK)
				else:
					if chr.IsNPC(dstChrID):
						if ENABLE_REFINE_RENEWAL:
							constInfo.AUTO_REFINE_TYPE = 2
							constInfo.AUTO_REFINE_DATA["NPC"][0] = dstChrID
							constInfo.AUTO_REFINE_DATA["NPC"][1] = attachedInvenType
							constInfo.AUTO_REFINE_DATA["NPC"][2] = attachedItemSlotPos
							constInfo.AUTO_REFINE_DATA["NPC"][3] = attachedItemCount
						net.SendGiveItemPacket(dstChrID, attachedInvenType, attachedItemSlotPos, attachedItemCount)
					else:
						if ENABLE_GUILD_DRAGONLAIR_SYSTEM:
							if chr.IsStone(dstChrID):
								net.SendGiveItemPacket(dstChrID, attachedInvenType, attachedItemSlotPos, attachedItemCount)
							else:
								net.SendExchangeStartPacket(dstChrID)
								# chat.AppendChat(chat.CHAT_TYPE_INFO, "Bu Sistem Bakýmdadýr !")
								net.SendExchangeItemAddPacket(attachedInvenType, attachedItemSlotPos, 0)
						else:
							# chat.AppendChat(chat.CHAT_TYPE_INFO, "Bu Sistem Bakýmdadýr !")
							net.SendExchangeStartPacket(dstChrID)
							net.SendExchangeItemAddPacket(attachedInvenType, attachedItemSlotPos, 0)
			else:
				self.__DropItem(attachedType, attachedItemIndex, attachedItemSlotPos, attachedItemCount)

	def __PutMoney(self, attachedType, attachedMoney, dstChrID):
		if True == chr.HasInstance(dstChrID) and player.GetMainCharacterIndex() != dstChrID:
			# chat.AppendChat(chat.CHAT_TYPE_INFO, "Bu Sistem Bakýmdadýr !")
			net.SendExchangeStartPacket(dstChrID)
			net.SendExchangeElkAddPacket(attachedMoney)
		else:
			self.__DropMoney(attachedType, attachedMoney)

	def __DropMoney(self, attachedType, attachedMoney):
		if uiPrivateShopBuilder.IsBuildingPrivateShop():
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.DROP_ITEM_FAILURE_PRIVATE_SHOP)
			return

		if attachedMoney>=1000:
			selfs.wndStream.popupWindow.Close()
			selfs.wndStream.popupWindow.Open(localeInfo.DROP_MONEY_FAILURE_1000_OVER, 0, localeInfo.UI_OK)
			return

		itemDropQuestionDialog = uiCommon.QuestionDialog()
		itemDropQuestionDialog.SetText(localeInfo.DO_YOU_DROP_MONEY % (attachedMoney))
		itemDropQuestionDialog.SetAcceptEvent(lambda arg=True: self.RequestDropItem(arg))
		itemDropQuestionDialog.SetCancelEvent(lambda arg=False: self.RequestDropItem(arg))
		itemDropQuestionDialog.Open()
		itemDropQuestionDialog.dropType = attachedType
		itemDropQuestionDialog.dropCount = attachedMoney
		itemDropQuestionDialog.dropNumber = player.ITEM_MONEY
		self.itemDropQuestionDialog = itemDropQuestionDialog

	def __DropItem(self, attachedType, attachedItemIndex, attachedItemSlotPos, attachedItemCount):
		if uiPrivateShopBuilder.IsBuildingPrivateShop():
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.DROP_ITEM_FAILURE_PRIVATE_SHOP)
			return

		if player.SLOT_TYPE_INVENTORY == attachedType and player.IsEquipmentSlot(attachedItemSlotPos):
			selfs.wndStream.popupWindow.Close()
			selfs.wndStream.popupWindow.Open(localeInfo.DROP_ITEM_FAILURE_EQUIP_ITEM, 0, localeInfo.UI_OK)

		else:
			if player.SLOT_TYPE_INVENTORY == attachedType:
				dropItemIndex = player.GetItemIndex(attachedItemSlotPos)

				item.SelectItem(dropItemIndex)
				dropItemName = item.GetItemName()

				questionText = localeInfo.HOW_MANY_ITEM_DO_YOU_DROP(dropItemName, attachedItemCount)

				itemDropQuestionDialog = uiCommon.QuestionDialogItem() # @@@correction045
				itemDropQuestionDialog.SetText(questionText)
				itemDropQuestionDialog.SetAcceptEvent(lambda arg=True: self.RequestDropItem(arg))
				itemDropQuestionDialog.SetCancelEvent(lambda arg=False: self.RequestDropItem(arg))
				itemDropQuestionDialog.SetDestroyEvent(lambda arg=True: self.RequestDestroyItem(arg)) # @@@correction045
				itemDropQuestionDialog.Open()
				itemDropQuestionDialog.dropType = attachedType
				itemDropQuestionDialog.dropNumber = attachedItemSlotPos
				itemDropQuestionDialog.dropCount = attachedItemCount
				self.itemDropQuestionDialog = itemDropQuestionDialog

				constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)
			elif player.SLOT_TYPE_DRAGON_SOUL_INVENTORY == attachedType:
				dropItemIndex = player.GetItemIndex(player.DRAGON_SOUL_INVENTORY, attachedItemSlotPos)

				item.SelectItem(dropItemIndex)
				dropItemName = item.GetItemName()

				questionText = localeInfo.HOW_MANY_ITEM_DO_YOU_DROP(dropItemName, attachedItemCount)

				itemDropQuestionDialog = uiCommon.QuestionDialogItem() # @@@correction045
				itemDropQuestionDialog.SetText(questionText)
				itemDropQuestionDialog.SetAcceptEvent(lambda arg=True: self.RequestDropItem(arg))
				itemDropQuestionDialog.SetCancelEvent(lambda arg=False: self.RequestDropItem(arg))
				itemDropQuestionDialog.SetDestroyEvent(lambda arg=True: self.RequestDestroyItem(arg)) # @@@correction045
				itemDropQuestionDialog.Open()
				itemDropQuestionDialog.dropType = attachedType
				itemDropQuestionDialog.dropNumber = attachedItemSlotPos
				itemDropQuestionDialog.dropCount = attachedItemCount
				self.itemDropQuestionDialog = itemDropQuestionDialog

				constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)

	def RequestDropItem(self, answer):
		if not self.itemDropQuestionDialog:
			return

		if answer:
			dropType = self.itemDropQuestionDialog.dropType
			dropCount = self.itemDropQuestionDialog.dropCount
			dropNumber = self.itemDropQuestionDialog.dropNumber

			if player.SLOT_TYPE_INVENTORY == dropType:
				if dropNumber == player.ITEM_MONEY:
					net.SendGoldDropPacketNew(dropCount)
					snd.PlaySound("sound/ui/money.wav")
				else:
					self.__SendDropItemPacket(dropNumber, dropCount)
			elif player.SLOT_TYPE_DRAGON_SOUL_INVENTORY == dropType:
					self.__SendDropItemPacket(dropNumber, dropCount, player.DRAGON_SOUL_INVENTORY)

		self.itemDropQuestionDialog.Close()
		self.itemDropQuestionDialog = None

		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

	def RequestDestroyItem(self, answer): # @@@correction045
		if not self.itemDropQuestionDialog:
			return
		if answer:
			dropType = self.itemDropQuestionDialog.dropType
			dropNumber = self.itemDropQuestionDialog.dropNumber
			if player.SLOT_TYPE_INVENTORY == dropType:
				if dropNumber == player.ITEM_MONEY:
					return
				else:
					self.__SendDestroyItemPacket(dropNumber)

		self.itemDropQuestionDialog.Close()
		self.itemDropQuestionDialog = None
		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

	def __SendDropItemPacket(self, itemVNum, itemCount, itemInvenType = player.INVENTORY):
		if uiPrivateShopBuilder.IsBuildingPrivateShop():
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.DROP_ITEM_FAILURE_PRIVATE_SHOP)
			return

		chat.AppendChat(chat.CHAT_TYPE_INFO, "Yere item atamazsýn !")

		# net.SendItemDropPacketNew(itemInvenType, itemVNum, itemCount)

	def __SendDestroyItemPacket(self, itemVNum, itemInvenType = player.INVENTORY): # @@@correction045
		if uiPrivateShopBuilder.IsBuildingPrivateShop():
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.DROP_ITEM_FAILURE_PRIVATE_SHOP)
			return

		net.SendItemDestroyPacket(itemVNum, 1)

	def OnMouseRightButtonDown(self):

		self.CheckFocus()

		if True == mouseModule.mouseController.isAttached():
			mouseModule.mouseController.DeattachObject()

		else:
			player.SetMouseState(player.MBT_RIGHT, player.MBS_PRESS)

		return True

	def OnMouseRightButtonUp(self):
		if True == mouseModule.mouseController.isAttached():
			return True

		player.SetMouseState(player.MBT_RIGHT, player.MBS_CLICK)
		return True

	def OnMouseMiddleButtonDown(self):
		player.SetMouseMiddleButtonState(player.MBS_PRESS)

	def OnMouseMiddleButtonUp(self):
		player.SetMouseMiddleButtonState(player.MBS_CLICK)

	def OnUpdate(self):
		app.UpdateGame()

		if selfs.wndMapNameShower.IsShow():
			selfs.wndMapNameShower.Update()

		if self.isShowDebugInfo:
			self.UpdateDebugInfo()

		if self.enableXMasBoom:
			self.__XMasBoom_Update()

		self.BUILD_OnUpdate()

	def UpdateDebugInfo(self):
		(x, y, z) = player.GetMainCharacterPosition()
		nUpdateTime = app.GetUpdateTime()
		nUpdateFPS = app.GetUpdateFPS()
		nRenderFPS = app.GetRenderFPS()
		nFaceCount = app.GetFaceCount()
		fFaceSpeed = app.GetFaceSpeed()
		nST=background.GetRenderShadowTime()
		(fAveRT, nCurRT) =  app.GetRenderTime()
		(iNum, fFogStart, fFogEnd, fFarCilp) = background.GetDistanceSetInfo()
		(iPatch, iSplat, fSplatRatio, sTextureNum) = background.GetRenderedSplatNum()
		if iPatch == 0:
			iPatch = 1

		#(dwRenderedThing, dwRenderedCRC) = background.GetRenderedGraphicThingInstanceNum()

		self.PrintCoord.SetText("Coordinate: %.2f %.2f %.2f ATM: %d" % (x, y, z, app.GetAvailableTextureMemory()/(1024*1024)))
		xMouse, yMouse = wndMgr.GetMousePosition()
		self.PrintMousePos.SetText("MousePosition: %d %d" % (xMouse, yMouse))

		self.FrameRate.SetText("UFPS: %3d UT: %3d FS %.2f" % (nUpdateFPS, nUpdateTime, fFaceSpeed))

		if fAveRT>1.0:
			self.Pitch.SetText("RFPS: %3d RT:%.2f(%3d) FC: %d(%.2f) " % (nRenderFPS, fAveRT, nCurRT, nFaceCount, nFaceCount/fAveRT))

		self.Splat.SetText("PATCH: %d SPLAT: %d BAD(%.2f)" % (iPatch, iSplat, fSplatRatio))
		#self.Pitch.SetText("Pitch: %.2f" % (app.GetCameraPitch())
		#self.TextureNum.SetText("TN : %s" % (sTextureNum))
		#self.ObjectNum.SetText("GTI : %d, CRC : %d" % (dwRenderedThing, dwRenderedCRC))
		self.ViewDistance.SetText("Num : %d, FS : %f, FE : %f, FC : %f" % (iNum, fFogStart, fFogEnd, fFarCilp))

	def OnRender(self):
		app.RenderGame()

		if self.console.Console.collision:
			background.RenderCollision()
			chr.RenderCollision()

		(x, y) = app.GetCursorPosition()
		textTail.UpdateAllTextTail()

		if True == wndMgr.IsPickedWindow(self.hWnd):

			self.PickingCharacterIndex = chr.Pick()

			if -1 != self.PickingCharacterIndex:
				textTail.ShowCharacterTextTail(self.PickingCharacterIndex)
			if 0 != selfs.wndTargetBoard.GetTargetVID():
				textTail.ShowCharacterTextTail(selfs.wndTargetBoard.GetTargetVID())

			if not self.__IsShowName():
				self.PickingItemIndex = item.Pick()
				if -1 != self.PickingItemIndex:
					textTail.ShowItemTextTail(self.PickingItemIndex)

		if self.__IsShowName():
			textTail.ShowAllTextTail()
			self.PickingItemIndex = textTail.Pick(x, y)

		textTail.UpdateShowingTextTail()
		textTail.ArrangeTextTail()
		if -1 != self.PickingItemIndex:
			textTail.SelectItemName(self.PickingItemIndex)

		grp.PopState()
		grp.SetInterfaceRenderState()

		textTail.Render()
		textTail.HideAllTextTail()

	def OnPressEscapeKey(self):
		if app.TARGET == app.GetCursor():
			app.SetCursor(app.NORMAL)

		elif True == mouseModule.mouseController.isAttached():
			mouseModule.mouseController.DeattachObject()

		else:
			selfs.wndDialogSystem.OpenDialog()
			selfs.wndDialogSystem.SetTop()

		return True

	def OnIMEReturn(self):
		if app.IsPressed(app.DIK_LSHIFT):
			self.OpenWhisperDialogWithoutTarget()
		else:
			self.ToggleChat()
		return True

	def OnPressExitKey(self):
		if False == selfs.wndDialogSystem.IsShow():
			selfs.wndDialogSystem.OpenDialog()
			selfs.wndDialogSystem.SetTop()
		else:
			selfs.wndDialogSystem.Close()

		return True

	def BINARY_LoverInfo(self, name, lovePoint):
		if selfs.wndMessenger:
			selfs.wndMessenger.OnAddLover(name, lovePoint)
		if selfs.wndAffectShower:
			selfs.wndAffectShower.SetLoverInfo(name, lovePoint)

	def BINARY_UpdateLovePoint(self, lovePoint):
		if selfs.wndMessenger:
			selfs.wndMessenger.OnUpdateLovePoint(lovePoint)
		if selfs.wndAffectShower:
			selfs.wndAffectShower.OnUpdateLovePoint(lovePoint)
	
	def BINARY_OnQuestConfirm(self, msg, timeout, pid):
		confirmDialog = uiCommon.QuestionDialogWithTimeLimit()
		# @@@correction045 BEGIN
		confirmDialog.SetText1(msg)
		confirmDialog.Open(timeout)
		# @@@correction045 END
		confirmDialog.SetAcceptEvent(lambda answer=True, pid=pid: net.SendQuestConfirmPacket(answer, pid) or self.confirmDialog.Hide())
		confirmDialog.SetCancelEvent(lambda answer=False, pid=pid: net.SendQuestConfirmPacket(answer, pid) or self.confirmDialog.Hide())
		self.confirmDialog = confirmDialog

	def Gift_Show(self):
		selfs.wndTaskBar.ShowGift()

	def BINARY_Cube_Open(self, npcVNUM):
		self.currentCubeNPC = npcVNUM

		selfs.wndCube.Open()

		if False == selfs.wndInventory.IsShow():
			selfs.wndInventory.Show()
			if ENABLE_EXPANDED_TASKBAR and selfs.wndExpandedMoneyTaskBar: # @@@correction047
				selfs.wndExpandedMoneyTaskBar.Show()

		if npcVNUM not in self.cubeInformation:
			net.SendChatPacket("/cube r_info")
		else:
			cubeInfoList = self.cubeInformation[npcVNUM]

			i = 0
			for cubeInfo in cubeInfoList:
				selfs.wndCube.AddCubeResultItem(cubeInfo["vnum"], cubeInfo["count"])

				j = 0
				for materialList in cubeInfo["materialList"]:
					for materialInfo in materialList:
						itemVnum, itemCount = materialInfo
						selfs.wndCube.AddMaterialInfo(i, j, itemVnum, itemCount)
					j = j + 1

				i = i + 1

			selfs.wndCube.Refresh()

	def BINARY_Cube_Close(self):
		selfs.wndCube.Close()

	def BINARY_Cube_UpdateInfo(self, gold, itemVnum, count):
		selfs.wndCube.UpdateInfo(gold, itemVnum, count)

	def BINARY_Cube_Succeed(self, itemVnum, count):
		selfs.wndCube.Clear()
		if 0:
			selfs.wndCubeResult.SetPosition(*selfs.wndCube.GetGlobalPosition())
			selfs.wndCubeResult.SetCubeResultItem(itemVnum, count)
			selfs.wndCubeResult.Open()
			selfs.wndCubeResult.SetTop()

	def BINARY_Cube_Failed(self):
		selfs.wndCube.Refresh()

	def BINARY_Cube_ResultList(self, npcVNUM, listText):
		if npcVNUM == 0:
			npcVNUM = self.currentCubeNPC

		self.cubeInformation[npcVNUM] = []

		try:
			for eachInfoText in listText.split("/"):
				eachInfo = eachInfoText.split(",")
				itemVnum	= int(eachInfo[0])
				itemCount	= int(eachInfo[1])

				self.cubeInformation[npcVNUM].append({"vnum": itemVnum, "count": itemCount})
				selfs.wndCube.AddCubeResultItem(itemVnum, itemCount)

			resultCount = len(self.cubeInformation[npcVNUM])
			requestCount = 7
			modCount = resultCount % requestCount
			splitCount = resultCount / requestCount
			for i in xrange(splitCount):
				net.SendChatPacket("/cube r_info %d %d" % (i * requestCount, requestCount))

			if 0 < modCount:
				net.SendChatPacket("/cube r_info %d %d" % (splitCount * requestCount, modCount))

		except RuntimeError, msg:
			dbg.TraceError(msg)
			return 0

		pass

	def BINARY_Cube_MaterialInfo(self, startIndex, listCount, listText):
		try:
			if 3 > len(listText):
				dbg.TraceError("Wrong Cube Material Infomation")
				return 0

			eachResultList = listText.split("@")

			cubeInfo = self.cubeInformation[self.currentCubeNPC]

			itemIndex = 0
			for eachResultText in eachResultList:
				cubeInfo[startIndex + itemIndex]["materialList"] = [[], [], [], [], []]
				materialList = cubeInfo[startIndex + itemIndex]["materialList"]

				gold = 0
				splitResult = eachResultText.split("/")
				if 1 < len(splitResult):
					gold = int(splitResult[1])

				eachMaterialList = splitResult[0].split("&")

				i = 0
				for eachMaterialText in eachMaterialList:
					complicatedList = eachMaterialText.split("|")

					if 0 < len(complicatedList):
						for complicatedText in complicatedList:
							(itemVnum, itemCount) = complicatedText.split(",")
							itemVnum = int(itemVnum)
							itemCount = int(itemCount)
							selfs.wndCube.AddMaterialInfo(itemIndex + startIndex, i, itemVnum, itemCount)

							materialList[i].append((itemVnum, itemCount))

					else:
						itemVnum, itemCount = eachMaterialText.split(",")
						itemVnum = int(itemVnum)
						itemCount = int(itemCount)
						selfs.wndCube.AddMaterialInfo(itemIndex + startIndex, i, itemVnum, itemCount)

						materialList[i].append((itemVnum, itemCount))

					i = i + 1



				itemIndex = itemIndex + 1

			selfs.wndCube.Refresh()


		except RuntimeError, msg:
			dbg.TraceError(msg)
			return 0

		pass

	def BINARY_Highlight_Item(self, inven_type, inven_pos):
		if player.DRAGON_SOUL_INVENTORY == inven_type:
			if ENABLE_DRAGON_SOUL_SYSTEM and selfs.wndDragonSoul: # @@@correction004
				selfs.wndDragonSoul.HighlightSlot(inven_pos)

	def BINARY_DragonSoulGiveQuilification(self):
		self.DRAGON_SOUL_IS_QUALIFIED = True
		import uiTaskBar
		selfs.wndExpandedTaskBar.SetToolTipText(uiTaskBar.ExpandedTaskBar.BUTTON_DRAGON_SOUL, uiScriptLocale.TASKBAR_DRAGON_SOUL)

	def BINARY_DragonSoulRefineWindow_Open(self):
		if False == player.IsObserverMode():
			if ENABLE_DRAGON_SOUL_SYSTEM:
				if False == selfs.wndDragonSoulRefine.IsShow():
					selfs.wndDragonSoulRefine.Show()
					if None != selfs.wndDragonSoul:
						if False == selfs.wndDragonSoul.IsShow():
							selfs.wndDragonSoul.Show()

	def BINARY_DragonSoulRefineWindow_RefineFail(self, reason, inven_type, inven_pos):
		if False == player.IsObserverMode():
			if ENABLE_DRAGON_SOUL_SYSTEM:
				if True == selfs.wndDragonSoulRefine.IsShow():
					selfs.wndDragonSoulRefine.RefineFail(reason, inven_type, inven_pos)

	def BINARY_DragonSoulRefineWindow_RefineSucceed(self, inven_type, inven_pos):
		if False == player.IsObserverMode():
			if ENABLE_DRAGON_SOUL_SYSTEM:
				if True == selfs.wndDragonSoulRefine.IsShow():
					selfs.wndDragonSoulRefine.RefineSucceed(inven_type, inven_pos)

	def BINARY_SetBigMessage(self, message):
		if selfs.wndBigBoard:
			selfs.wndBigBoard.SetTip(message)

	def BINARY_SetTipMessage(self, message):
		if selfs.wndTipBoard:
			selfs.wndTipBoard.SetTip(message)

	def BINARY_AppendNotifyMessage(self, type):
		if not type in localeInfo.NOTIFY_MESSAGE:
			return
		chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.NOTIFY_MESSAGE[type])

	def BINARY_Guild_EnterGuildArea(self, areaID):
		mainCharacterName = player.GetMainCharacterName()
		import guild
		masterName = guild.GetGuildMasterName()

		if mainCharacterName != masterName:
			return

		if areaID != player.GetGuildID():
			return

		selfs.wndGameButton.ShowBuildButton()

	def BINARY_Guild_ExitGuildArea(self, areaID):
		selfs.wndGameButton.HideBuildButton()

	def BINARY_GuildWar_OnSendDeclare(self, guildID):
		pass

	def BINARY_GuildWar_OnRecvDeclare(self, guildID, warType):
		mainCharacterName = player.GetMainCharacterName()
		masterName = guild.GetGuildMasterName()
		if mainCharacterName == masterName:
			self.__GuildWar_OpenAskDialog(guildID, warType)

	def BINARY_GuildWar_OnRecvPoint(self, gainGuildID, opponentGuildID, point):
		key = uiGuild.GetGVGKey(gainGuildID, opponentGuildID)
		if not self.guildScoreBoardDict.has_key(key):
			return

		guildBoard = self.guildScoreBoardDict[key]
		guildBoard.SetScore(gainGuildID, opponentGuildID, point)

	def BINARY_GuildWar_OnStart(self, guildSelf, guildOpp):
		selfs.wndGuild.OnStartGuildWar(guildSelf, guildOpp)

		guildWarScoreBoard = uiGuild.GuildWarScoreBoard()
		guildWarScoreBoard.Open(guildSelf, guildOpp)
		guildWarScoreBoard.Show()
		self.guildScoreBoardDict[uiGuild.GetGVGKey(guildSelf, guildOpp)] = guildWarScoreBoard

	def BINARY_GuildWar_OnEnd(self, guildSelf, guildOpp):
		selfs.wndGuild.OnEndGuildWar(guildSelf, guildOpp)

		key = uiGuild.GetGVGKey(guildSelf, guildOpp)

		if not self.guildScoreBoardDict.has_key(key):
			return

		self.guildScoreBoardDict[key].Destroy()
		del self.guildScoreBoardDict[key]

	def BINARY_BettingGuildWar_SetObserverMode(self, isEnable):
		selfs.wndGameButton.SetObserverMode(isEnable)

	def BINARY_BettingGuildWar_UpdateObserverCount(self, observerCount):
		selfs.wndMiniMap.UpdateObserverCount(observerCount)

	def __GuildWar_UpdateMemberCount(self, guildID1, memberCount1, guildID2, memberCount2, observerCount):
		guildID1 = int(guildID1)
		guildID2 = int(guildID2)
		memberCount1 = int(memberCount1)
		memberCount2 = int(memberCount2)
		observerCount = int(observerCount)

		key = uiGuild.GetGVGKey(gulidID1, guildID2)
		if self.guildScoreBoardDict.has_key(key):
			self.guildScoreBoardDict[key].UpdateMemberCount(gulidID1, memberCount1, guildID2, memberCount2)

		selfs.wndMiniMap.UpdateObserverCount(observerCount)

	def __GuildWar_OpenAskDialog(self, guildID, warType):
		guildName = guild.GetGuildName(guildID)

		if "Noname" == guildName:
			return

		questionDialog = uiGuild.AcceptGuildWarDialog()
		questionDialog.SAFE_SetAcceptEvent(self.__GuildWar_OnAccept)
		questionDialog.SAFE_SetCancelEvent(self.__GuildWar_OnDecline)
		questionDialog.Open(guildName, warType)

		selfs.GuildWarQuestionDialog = questionDialog

	def __GuildWar_CloseAskDialog(self):
		selfs.GuildWarQuestionDialog.Close()
		selfs.GuildWarQuestionDialog.Destroy()
		del selfs.GuildWarQuestionDialog
		selfs.GuildWarQuestionDialog = None

	def __GuildWar_OnAccept(self):
		guildName = selfs.GuildWarQuestionDialog.GetGuildName()

		net.SendChatPacket("/war " + guildName)
		self.__GuildWar_CloseAskDialog()

		return 1

	def __GuildWar_OnDecline(self):
		guildName = selfs.GuildWarQuestionDialog.GetGuildName()
		net.SendChatPacket("/nowar " + guildName)
		self.__GuildWar_CloseAskDialog()
		return 1

	def __ServerCommand_Build(self):
		serverCommandList={
			"ConsoleEnable"			: self.__Console_Enable,
			"DayMode"				: self.__DayMode_Update,
			"PRESERVE_DayMode"		: self.__PRESERVE_DayMode_Update,
			"CloseRestartWindow"	: self.__RestartDialog_Close,
			"OpenPrivateShop"		: self.__PrivateShop_Open,
			"PartyHealReady"		: self.PartyHealReady,
			"ShowMeSafeboxPassword"	: self.AskSafeboxPassword,
			"CloseSafebox"			: self.CommandCloseSafebox,

			"CloseMall"				: self.CommandCloseMall,
			"ShowMeMallPassword"	: self.AskMallPassword,
			"item_mall"				: self.__ItemMall_Open,

			"RefineSuceeded"		: self.RefineSuceededMessage,
			"RefineFailed"			: self.RefineFailedMessage,
			"xmas_snow"				: self.__XMasSnow_Enable,
			"xmas_boom"				: self.__XMasBoom_Enable,
			"xmas_song"				: self.__XMasSong_Enable,
			"xmas_tree"				: self.__XMasTree_Enable,
			"newyear_boom"			: self.__XMasBoom_Enable,
			"PartyRequest"			: self.__PartyRequestQuestion,
			"PartyRequestDenied"	: self.__PartyRequestDenied,
			"horse_state"			: self.__Horse_UpdateState,
			"hide_horse_state"		: self.__Horse_HideState,
			"WarUC"					: self.__GuildWar_UpdateMemberCount,
			"test_server"			: self.__EnableTestServerFlag,
			"mall"			: self.__InGameShop_Show,

			"lover_login"			: self.__LoginLover,
			"lover_logout"			: self.__LogoutLover,
			"lover_near"			: self.__LoverNear,
			"lover_far"				: self.__LoverFar,
			"lover_divorce"			: self.__LoverDivorce,
			"PlayMusic"				: self.__PlayMusic,

			"MyShopPriceList"		: self.__PrivateShop_PriceList,
		}

		if ENABLE_FAST_NPC: # @@@correction027
			serverCommandList["OpenNpc"] = self.OpenNpc

		if ENABLE_GUILDWAR_KILL_COUNTER: # @@@correction034
			serverCommandList["GW_Kill_Update"] = self.GuildWar_ProcessKillInput

		if ENABLE_OFFLINE_PRIVATE_SHOP:
			serverCommandList["OpenMyShopDecoWnd"] = self.OpenMyShopDecoWnd
			serverCommandList["PriceCheck"] = self.PriceCheck

		if ENABLE_MOVE_CHANNEL:
			serverCommandList["Update"] = self.Update

		if ENABLE_GUILD_DRAGONLAIR_SYSTEM:
			serverCommandList["meley_open"] = self.OpenMeleyRanking
			serverCommandList["meley_rank"] = self.AddRankMeleyRanking

		if ENABLE_SUPPORT_SYSTEM:
			serverCommandList["samanboard"] = self.OpenSupportGui
			serverCommandList["SupportLv"] = self.SetSupportLevel
			serverCommandList["SupportName"] = self.SetSupportName
			serverCommandList["SupportLeave"] = self.SupportLeave
			serverCommandList["SupporInt"] = self.SupportInt
			serverCommandList["SupportExp"] = self.SetSupportExp
			serverCommandList["SupportIcon"] = self.SetSupportIcon
			serverCommandList["SupportSpeciality"] = self.SetSupportSpeciality
			serverCommandList["SupportArmor"] = self.SetSupportArmor
			serverCommandList["SupportWeapon"] = self.SetSupportWeapon
			serverCommandList["SupportHair"] = self.SetSupportHair

		if ENABLE_GROWTH_PET_SYSTEM:
			serverCommandList["PetEvolution"] = self.SetPetEvolution
			serverCommandList["PetName"] = self.SetPetName
			serverCommandList["PetLevel"] = self.SetPetLevel
			serverCommandList["PetDuration"] = self.SetPetDuration
			serverCommandList["PetBonus"] = self.SetPetBonus
			serverCommandList["PetSkill"] = self.SetPetskill
			serverCommandList["PetIcon"] = self.SetPetIcon
			serverCommandList["PetExp"] = self.SetPetExp
			serverCommandList["PetUnsummon"] = self.PetUnsummon
			serverCommandList["OpenPetIncubator"] = self.OpenPetIncubator
			serverCommandList["ResultAttr"] = self.ResultAttr
			serverCommandList["PetType"] = self.PetType

		if ENABLE_DEFENSE_WAVE:
			serverCommandList["BINARY_Update_Mast_HP"] = self.BINARY_Update_Mast_HP
			serverCommandList["BINARY_Update_Mast_Window"] = self.BINARY_Update_Mast_Window

		if ENABLE_MINI_GAME:
			serverCommandList["SetMiniGameEvent"] = self.SetMiniGameEvent

		if ENABLE_MINI_GAME_OKEY_NORMAL:
			serverCommandList["mini_game_okey_normal"] = self.__MiniGameOkeyNormal

		if ENABLE_ACCUMULATE_DAMAGE_DISPLAY:
			serverCommandList["RefreshAccumulateCount"] = self.RefreshAccumulateCount

		if ENABLE_12ZI:
			serverCommandList["Show_Button_Index"] = self.Show_Button_Index
			serverCommandList["Show12ziJumpButton"] = self.Show12ziJumpButton
			serverCommandList["RefreshShopItemToolTip"] = self.RefreshShopItemToolTip
			serverCommandList["SetBeadCount"] = self.SetBeadCount
			serverCommandList["NextBeadUpdateTime"] = self.NextBeadUpdateTime
			serverCommandList["OpenZodiacReward"] = self.OpenZodiacReward
			serverCommandList["Refresh12ziTimer"] = self.Refresh12ziTimer
			serverCommandList["OpenReviveDialog"] = self.OpenReviveDialog

		if ENABLE_DUNGEON_INFO_SYSTEM:
			serverCommandList["DungeonInfo"] = self.DungeonInfo
			serverCommandList["CleanDungeonInfo"] = self.CleanDungeonInfo
			serverCommandList["CleanDungeonRanking"] = self.CleanDungeonRanking
			serverCommandList["GetDungeonInfo"] = self.GetDungeonInfo
			serverCommandList["UpdateDungeonInfo"] = self.UpdateDungeonInfo
			serverCommandList["UpdateDungeonRanking"] = self.UpdateDungeonRanking
			serverCommandList["OpenDungeonRanking"] = self.OpenDungeonRanking

		if ENABLE_FAST_RUHBK:
			serverCommandList["fastruh"] = self.OpenFastRuh
			serverCommandList["fastbk"] = self.OpenFastBK

		if ENABLE_AFAR_BIOLOG:
			serverCommandList["OpenBiologWindow"] = self.OpenBiologWindow
			serverCommandList["OpenBiologWnd"] = self.CMDBiyologPenceresiAc
			serverCommandList["biyologodul"] = self.biyoodulac
			serverCommandList["biyologekrankapa"] = self.biyologekrankapa
			serverCommandList["biyolog"] = self.biyolog

		serverCommandList["getinputbegin"] = self.getinputbegin
		serverCommandList["getinputend"] = self.getinputend

		if WJ_SECURITY_SYSTEM:
			serverCommandList["captcha"] = self.captcha

		if ENABLE_BLOCK_EXP:
			serverCommandList["ExpBlock"] = self.ExpBlock

		self.serverCommander = stringCommander.Analyzer()
		for serverCommandItem in serverCommandList.items():
			self.serverCommander.SAFE_RegisterCallBack(serverCommandItem[0], serverCommandItem[1])

	def BINARY_ServerCommand_Run(self, line):
		try:
			return self.serverCommander.Run(line)
		except RuntimeError, msg:
			dbg.TraceError(msg)
			return 0

	def __ProcessPreservedServerCommand(self):
		try:
			command = net.GetPreservedServerCommand()
			while command:
				print " __ProcessPreservedServerCommand", command
				self.serverCommander.Run(command)
				command = net.GetPreservedServerCommand()
		except RuntimeError, msg:
			dbg.TraceError(msg)
			return 0

	def PartyHealReady(self):
		selfs.wndParty.PartyHealReady()

	def AskSafeboxPassword(self):
		if selfs.wndSafebox.IsShow():
			return

		selfs.wndDialogPassword.SetTitle(localeInfo.PASSWORD_TITLE)
		selfs.wndDialogPassword.SetSendMessage("/safebox_password ")
		selfs.wndDialogPassword.ShowDialog()

	def AskMallPassword(self):
		if selfs.wndMall.IsShow():
			return

		selfs.wndDialogPassword.SetTitle(localeInfo.MALL_PASSWORD_TITLE)
		selfs.wndDialogPassword.SetSendMessage("/mall_password ")
		selfs.wndDialogPassword.ShowDialog()

	def __ItemMall_Open(self):
		if not selfs.wndMallPageDialog:
			import uiShop
			selfs.wndMallPageDialog = uiShop.MallPageDialog()

		selfs.wndMallPageDialog.Open()

	def CommandCloseMall(self):
		selfs.wndMall.CommandCloseMall()

	def RefineSuceededMessage(self):
		snd.PlaySound("sound/ui/make_soket.wav")
		self.PopupMessage(localeInfo.REFINE_SUCCESS)
		if ENABLE_REFINE_RENEWAL and selfs.wndDialogRefineNew:
			selfs.wndDialogRefineNew.CheckRefine(False)

	def RefineFailedMessage(self):
		snd.PlaySound("sound/ui/jaeryun_fail.wav")
		self.PopupMessage(localeInfo.REFINE_FAILURE)
		if ENABLE_REFINE_RENEWAL and selfs.wndDialogRefineNew:
			selfs.wndDialogRefineNew.CheckRefine(True)

	def CommandCloseSafebox(self):
		selfs.wndSafebox.CommandCloseSafebox()

	def __PrivateShop_PriceList(self, itemVNum, itemPrice):
		uiPrivateShopBuilder.SetPrivateShopItemPrice(itemVNum, itemPrice)

	def __Horse_HideState(self):
		selfs.wndAffectShower.SetHorseState(0, 0, 0)

	def __Horse_UpdateState(self, level, health, battery):
		selfs.wndAffectShower.SetHorseState(int(level), int(health), int(battery))

	def __IsXMasMap(self):
		mapDict = ( "metin2_map_n_flame_01",
					"metin2_map_n_desert_01",
					"metin2_map_spiderdungeon",
					"metin2_map_deviltower1", )

		if background.GetCurrentMapName() in mapDict:
			return False

		return True

	def __XMasSnow_Enable(self, mode):
		self.__XMasSong_Enable(mode)
		if "1"==mode:
			if not self.__IsXMasMap():
				return
			background.EnableSnow(1)
		else:
			background.EnableSnow(0)

	def __XMasBoom_Enable(self, mode):
		if "1"==mode:
			if not self.__IsXMasMap():
				return
			self.__DayMode_Update("dark")
			self.enableXMasBoom = True
			self.startTimeXMasBoom = app.GetTime()
		else:
			self.__DayMode_Update("light")
			self.enableXMasBoom = False

	def __XMasTree_Enable(self, grade):

		print "XMAS_TREE ", grade
		background.SetXMasTree(int(grade))

	def __XMasSong_Enable(self, mode):
		if "1"==mode:
			XMAS_BGM = "xmas.mp3"

			if app.IsExistFile("BGM/" + XMAS_BGM)==1:
				if musicInfo.fieldMusic != "":
					snd.FadeOutMusic("BGM/" + musicInfo.fieldMusic)

				musicInfo.fieldMusic=XMAS_BGM
				snd.FadeInMusic("BGM/" + musicInfo.fieldMusic)

		else:
			if musicInfo.fieldMusic != "":
				snd.FadeOutMusic("BGM/" + musicInfo.fieldMusic)

			musicInfo.fieldMusic=musicInfo.METIN2THEMA
			snd.FadeInMusic("BGM/" + musicInfo.fieldMusic)

	def __RestartDialog_Close(self):
		selfs.wndDialogRestart.Close()

	def __Console_Enable(self):
		constInfo.CONSOLE_ENABLE = True
		self.consoleEnable = True
		app.EnableSpecialCameraMode()
		ui.EnablePaste(True)

	if ENABLE_OFFLINE_PRIVATE_SHOP:
		def BINARY_OfflinePrivateShopEditOpen(self):
			selfs.wndOfflinePrivateShopEdit.Refresh()

		def __PrivateShop_Open(self):
			if not selfs.wndPrivateShopBuilder.IsShow():
				constInfo.PriceCheck = {}
				net.SendChatPacket("/price_check")
				selfs.wndPrivateShopBuilder.Open("", 0, 30000)

		def OpenPShopSearchDialog(self):
			selfs.wndOfflinePrivateShopSearch.Open(0)

		def OpenPShopSearchDialogCash(self):
			selfs.wndOfflinePrivateShopSearch.Open(1)

		def RefreshPShopSearchDialog(self):
			selfs.wndOfflinePrivateShopSearch.RefreshList()

		def RefreshPShopLogs(self):
			selfs.wndOfflinePrivateShopEdit.RefreshLogs()

		def BINARY_PrivateShop_Appear(self, vid, text, type):
			if type == 0:
				board = uiPrivateShopBuilder.PrivateShopAdvertisementBoard()
				board.Open(vid, text)
			else:
				board = uiPrivateShopBuilder.PrivateShopTitleBar(type)
				board.Open(vid, text)

		def OpenMyShopDecoWnd(self):
			if selfs.wndInputDialog:
				return

			if selfs.wndPrivateShopBuilder.IsShow():
				return

			if selfs.wndMyShopDeco:
				selfs.wndMyShopDeco.Open()

		def PriceCheck(self, vnum, max, min):
			constInfo.PriceCheck[int(vnum)] = (int(max), int(min))
	else:
		def __PrivateShop_Open(self):
			#if player.IsInSafeArea():
			#	chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.CANNOT_OPEN_PRIVATE_SHOP_IN_SAFE_AREA)
			#	return

			inputDialog = uiCommon.InputDialog()
			inputDialog.SetTitle(localeInfo.PRIVATE_SHOP_INPUT_NAME_DIALOG_TITLE)
			inputDialog.SetMaxLength(32)
			inputDialog.SetAcceptEvent(ui.__mem_func__(self.OpenPrivateShopBuilder))
			inputDialog.SetCancelEvent(ui.__mem_func__(self.ClosePrivateShopInputNameDialog))
			inputDialog.Open()
			selfs.wndInputDialog = inputDialog

		def BINARY_PrivateShop_Appear(self, vid, text):
			board = uiPrivateShopBuilder.PrivateShopAdvertisementBoard()
			board.Open(vid, text)

			self.privateShopAdvertisementBoardDict[vid] = board

	def BINARY_PrivateShop_Disappear(self, vid):
		if not self.privateShopAdvertisementBoardDict.has_key(vid):
			return

		del self.privateShopAdvertisementBoardDict[vid]
		uiPrivateShopBuilder.DeleteADBoard(vid)

	def __PRESERVE_DayMode_Update(self, mode):
		if "light"==mode:
			background.SetEnvironmentData(0)
		elif "dark"==mode:

			if not self.__IsXMasMap():
				return

			background.RegisterEnvironmentData(1, constInfo.ENVIRONMENT_NIGHT)
			background.SetEnvironmentData(1)

	def __DayMode_Update(self, mode):
		if "light"==mode:
			if selfs.wndCurtain:
				selfs.wndCurtain.SAFE_FadeOut(self.__DayMode_OnCompleteChangeToLight)
		elif "dark"==mode:

			if not self.__IsXMasMap():
				return

			if selfs.wndCurtain:
				selfs.wndCurtain.SAFE_FadeOut(self.__DayMode_OnCompleteChangeToDark)

	def __DayMode_OnCompleteChangeToLight(self):
		# if ENABLE_12ZI:
			# background.SetEnvironmentData(background.DAY_MODE_LIGHT)
		# else:
		background.SetEnvironmentData(0)
		if selfs.wndCurtain:
			selfs.wndCurtain.FadeIn()

	def __DayMode_OnCompleteChangeToDark(self):
		background.RegisterEnvironmentData(1, constInfo.ENVIRONMENT_NIGHT)
		background.SetEnvironmentData(1)
		if selfs.wndCurtain:
			selfs.wndCurtain.FadeIn()

	def __XMasBoom_Update(self):
		self.BOOM_DATA_LIST = ( (2, 5), (5, 2), (7, 3), (10, 3), (20, 5) )
		if self.indexXMasBoom >= len(self.BOOM_DATA_LIST):
			return

		boomTime = self.BOOM_DATA_LIST[self.indexXMasBoom][0]
		boomCount = self.BOOM_DATA_LIST[self.indexXMasBoom][1]

		if app.GetTime() - self.startTimeXMasBoom > boomTime:
			self.indexXMasBoom += 1

			for i in xrange(boomCount):
				self.__XMasBoom_Boom()

	def __XMasBoom_Boom(self):
		x, y, z = player.GetMainCharacterPosition()
		randX = app.GetRandom(-150, 150)
		randY = app.GetRandom(-150, 150)

		snd.PlaySound3D(x+randX, -y+randY, z, "sound/common/etc/salute.mp3")

	def __PartyRequestQuestion(self, vid):
		vid = int(vid)
		partyRequestQuestionDialog = uiCommon.QuestionDialog()
		partyRequestQuestionDialog.SetText(chr.GetNameByVID(vid) + localeInfo.PARTY_DO_YOU_ACCEPT)
		partyRequestQuestionDialog.SetAcceptText(localeInfo.UI_ACCEPT)
		partyRequestQuestionDialog.SetCancelText(localeInfo.UI_DENY)
		partyRequestQuestionDialog.SetAcceptEvent(lambda arg=True: self.__AnswerPartyRequest(arg))
		partyRequestQuestionDialog.SetCancelEvent(lambda arg=False: self.__AnswerPartyRequest(arg))
		partyRequestQuestionDialog.Open()
		partyRequestQuestionDialog.vid = vid
		self.partyRequestQuestionDialog = partyRequestQuestionDialog

	def __AnswerPartyRequest(self, answer):
		if not self.partyRequestQuestionDialog:
			return

		vid = self.partyRequestQuestionDialog.vid

		if answer:
			net.SendChatPacket("/party_request_accept " + str(vid))
		else:
			net.SendChatPacket("/party_request_deny " + str(vid))

		self.partyRequestQuestionDialog.Close()
		self.partyRequestQuestionDialog = None

	def __PartyRequestDenied(self):
		self.PopupMessage(localeInfo.PARTY_REQUEST_DENIED)

	def __EnableTestServerFlag(self):
		app.EnableTestServerFlag()

	def __InGameShop_Show(self, url):
		if constInfo.IN_GAME_SHOP_ENABLE:
			selfs.wndWeb.Open(url)
			selfs.wndChat.CloseChat()

	def __LoginLover(self):
		if selfs.wndMessenger:
			selfs.wndMessenger.OnLoginLover()

	def __LogoutLover(self):
		if selfs.wndMessenger:
			selfs.wndMessenger.OnLogoutLover()
		if selfs.wndAffectShower:
			selfs.wndAffectShower.HideLoverState()

	def __LoverNear(self):
		if selfs.wndAffectShower:
			selfs.wndAffectShower.ShowLoverState()

	def __LoverFar(self):
		if selfs.wndAffectShower:
			selfs.wndAffectShower.HideLoverState()

	def __LoverDivorce(self):
		if selfs.wndMessenger:
			selfs.wndMessenger.ClearLoverInfo()
		if selfs.wndAffectShower:
			selfs.wndAffectShower.ClearLoverState()

	def __PlayMusic(self, flag, filename):
		flag = int(flag)
		if flag:
			snd.FadeOutAllMusic()
			musicInfo.SaveLastPlayFieldMusic()
			snd.FadeInMusic("BGM/" + filename)
		else:
			snd.FadeOutAllMusic()
			musicInfo.LoadLastPlayFieldMusic()
			snd.FadeInMusic("BGM/" + musicInfo.fieldMusic)

	if ENABLE_ACCE_SYSTEM: # @@@correction024
		def ActAcce(self, iAct, bWindow):
			if iAct == 1:
				if bWindow == True:
					if not selfs.wndAcceCombine.IsOpened():
						selfs.wndAcceCombine.Open()
					
					if not selfs.wndInventory.IsShow():
						selfs.wndInventory.Show()
				else:
					if not selfs.wndAcceAbsorption.IsOpened():
						selfs.wndAcceAbsorption.Open()
					
					if not selfs.wndInventory.IsShow():
						selfs.wndInventory.Show()
				
				selfs.wndInventory.RefreshBagSlotWindow()
			elif iAct == 2:
				if bWindow == True:
					if selfs.wndAcceCombine.IsOpened():
						selfs.wndAcceCombine.Close()
				else:
					if selfs.wndAcceAbsorption.IsOpened():
						selfs.wndAcceAbsorption.Close()
				
				selfs.wndInventory.RefreshBagSlotWindow()
			elif iAct == 3 or iAct == 4:
				if bWindow == True:
					if selfs.wndAcceCombine.IsOpened():
						selfs.wndAcceCombine.Refresh(iAct)
				else:
					if selfs.wndAcceAbsorption.IsOpened():
						selfs.wndAcceAbsorption.Refresh(iAct)
				
				selfs.wndInventory.RefreshBagSlotWindow()

		def AlertAcce(self, bWindow):
			snd.PlaySound("sound/ui/make_soket.wav")
			if bWindow:
				self.PopupMessage(localeInfo.ACCE_DEL_SERVEITEM)
			else:
				self.PopupMessage(localeInfo.ACCE_DEL_ABSORDITEM)

	if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
		def ActChangeLook(self, iAct):
			if iAct == 1:
				if not selfs.wndChangeLook.IsOpened():
					selfs.wndChangeLook.Open()
				
				if not selfs.wndInventory.IsShow():
					selfs.wndInventory.Show()
				
				selfs.wndInventory.RefreshBagSlotWindow()
			elif iAct == 2:
				if selfs.wndChangeLook.IsOpened():
					selfs.wndChangeLook.Close()
				
				selfs.wndInventory.RefreshBagSlotWindow()
			elif iAct == 3 or iAct == 4:
				if selfs.wndChangeLook.IsOpened():
					selfs.wndChangeLook.Refresh()
				
				selfs.wndInventory.RefreshBagSlotWindow()

		def AlertChangeLook(self):
			self.PopupMessage(localeInfo.CHANGE_LOOK_DEL_ITEM)

	if ENABLE_FAST_NPC: # @@@correction027
		def OpenNpc(self):
			if selfs.wndFastNpc:
				if selfs.wndFastNpc.IsShow():
					selfs.wndFastNpc.Close()
				else:
					selfs.wndFastNpc.Open()

	if ENABLE_GUILDWAR_KILL_COUNTER: # @@@correction034
		def GuildWar_ProcessKillInput(self, killerName, killerRace, victimName, victimRace, weapon):
			selfs.wndGuildWarKillCounter.OnMessage(killerName, killerRace, victimName, victimRace, weapon)
			selfs.wndGuildWarScore.OnUpdatePlayer(killerName, killerRace, victimName, victimRace)

		def GuildScore(self):
			if background.GetCurrentMapName() == "metin2_map_t3":
				if selfs.wndGuildWarScore:
					selfs.wndGuildWarScore.Open()

	def MakeHyperlinkTooltip(self, hyperlink):
		tokens = hyperlink.split(":")
		if tokens and len(tokens):
			type = tokens[0]
			if "item" == type:
				selfs.wndHyperLinkItemToolTip.SetHyperlinkItem(tokens)
			elif "whisper" == type:
				if player.GetName() == str(tokens[1]):
					chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.WHISPER_CANNOT_SEND_YOURSELF)
					return
				self.OpenWhisperDialog(str(tokens[1]))
			elif "web" == type and tokens[1].startswith("httpXxX") or "web" == type and tokens[1].startswith("httpsXxX"):
					OpenLinkQuestionDialog = uiCommon.QuestionDialog2()
					OpenLinkQuestionDialog.SetText1(localeInfo.CHAT_OPEN_LINK_DANGER)
					OpenLinkQuestionDialog.SetText2(localeInfo.CHAT_OPEN_LINK)
					link = "start " + tokens[1].replace("XxX", "://").replace("&","^&")
					OpenLinkQuestionDialog.SetAcceptEvent(lambda arg=TRUE: self.AnswerOpenLink(arg, link))
					OpenLinkQuestionDialog.SetCancelEvent(lambda arg=FALSE: self.AnswerOpenLink(arg, link))
					OpenLinkQuestionDialog.Open()
					self.OpenLinkQuestionDialog = OpenLinkQuestionDialog
			elif "sysweb" == type:
				os.system("start " + tokens[1].replace("XxX", "://"))

	def AnswerOpenLink(self, answer, link):
		if not self.OpenLinkQuestionDialog:
			return

		self.OpenLinkQuestionDialog.Close()
		self.OpenLinkQuestionDialog = None

		if not answer:
			return

		# link = constInfo.link
		os.system(link)	

	def SelectMouseButtonEvent(self, dir, event):
		selfs.wndTaskBar.SelectMouseButtonEvent(dir, event)

	def ClosePointResetDialog(self):
		selfs.wndDialogPointReset.Close()

	def OpenCharacterWindowQuestPage(self):
		selfs.wndCharacter.Show()
		selfs.wndCharacter.SetState("QUEST")

	def ToggleChat(self):
		if True == selfs.wndChat.IsEditMode():
			selfs.wndChat.CloseChat()
		else:
			if selfs.wndWeb and selfs.wndWeb.IsShow():
				pass
			else:
				selfs.wndChat.OpenChat()

	def SetChatFocus(self):
		selfs.wndChat.SetChatFocus()

	def ToggleMessenger(self):
		if selfs.wndMessenger.IsShow():
			selfs.wndMessenger.Hide()
		else:
			selfs.wndMessenger.SetTop()
			selfs.wndMessenger.Show()

	def ToggleMiniMap(self):
		if app.IsPressed(app.DIK_LSHIFT) or app.IsPressed(app.DIK_RSHIFT):
			if False == selfs.wndMiniMap.isShowMiniMap():
				selfs.wndMiniMap.ShowMiniMap()
				selfs.wndMiniMap.SetTop()
			else:
				selfs.wndMiniMap.HideMiniMap()

		else:
			selfs.wndMiniMap.ToggleAtlasWindow()

	def MiniMapScaleUp(self):
		selfs.wndMiniMap.ScaleUp()

	def MiniMapScaleDown(self):
		selfs.wndMiniMap.ScaleDown()

	def ToggleCharacterWindow(self, state):
		if False == player.IsObserverMode():
			if False == selfs.wndCharacter.IsShow():
				if False == player.IsObserverMode():
					selfs.wndCharacter.SetState(state)
					selfs.wndCharacter.Show()
					selfs.wndCharacter.SetTop()
			else:
				if state == selfs.wndCharacter.GetState():
					selfs.wndCharacter.OverOutItem()
					if ENABLE_DETAILS_UI:
						selfs.wndCharacter.Close()
					else:
						selfs.wndCharacter.Hide()
				else:
					selfs.wndCharacter.SetState(state)

	def ToggleCharacterWindowStatusPage(self):
		self.ToggleCharacterWindow("STATUS")

	def ToggleInventoryWindow(self):
		if False == player.IsObserverMode():
			if False == selfs.wndInventory.IsShow():
				selfs.wndInventory.Show()
				selfs.wndInventory.SetTop()
				if ENABLE_EXPANDED_TASKBAR and selfs.wndExpandedMoneyTaskBar: # @@@correction047
					selfs.wndExpandedMoneyTaskBar.Show()
			else:
				selfs.wndInventory.OverOutItem()
				selfs.wndInventory.Close()
				if ENABLE_EXPANDED_TASKBAR and selfs.wndExpandedMoneyTaskBar: # @@@correction047
					selfs.wndExpandedMoneyTaskBar.Hide()

	def ToggleExpandedButton(self):
		if False == player.IsObserverMode():
			if False == selfs.wndExpandedTaskBar.IsShow():
				selfs.wndExpandedTaskBar.Show()
				selfs.wndExpandedTaskBar.SetTop()
			else:
				selfs.wndExpandedTaskBar.Close()

	def ToggleDragonSoulWindow(self):
		if False == player.IsObserverMode():
			if ENABLE_DRAGON_SOUL_SYSTEM:
				if False == selfs.wndDragonSoul.IsShow():
					if self.DRAGON_SOUL_IS_QUALIFIED:
						selfs.wndDragonSoul.Show()
					else:
						try:
							selfs.wndPopupDialog.SetText(localeInfo.DRAGON_SOUL_UNQUALIFIED)
							selfs.wndPopupDialog.Open()
						except:
							selfs.wndPopupDialog = uiCommon.PopupDialog()
							selfs.wndPopupDialog.SetText(localeInfo.DRAGON_SOUL_UNQUALIFIED)
							selfs.wndPopupDialog.Open()
				else:
					selfs.wndDragonSoul.Close()

	def ToggleDragonSoulWindowWithNoInfo(self):
		if False == player.IsObserverMode():
			if ENABLE_DRAGON_SOUL_SYSTEM:
				if False == selfs.wndDragonSoul.IsShow():
					if self.DRAGON_SOUL_IS_QUALIFIED:
						selfs.wndDragonSoul.Show()
				else:
					selfs.wndDragonSoul.Close()

	def ToggleGuildWindow(self):
		if not selfs.wndGuild.IsShow():
			if selfs.wndGuild.CanOpen():
				selfs.wndGuild.Open()
			else:
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.GUILD_YOU_DO_NOT_JOIN)
		else:
			selfs.wndGuild.OverOutItem()
			selfs.wndGuild.Hide()

	def ToggleChatLogWindow(self):
		if selfs.wndChatLog.IsShow():
			selfs.wndChatLog.Hide()
		else:
			selfs.wndChatLog.Show()

	def __OnClickStatusPlusButton(self):
		self.ToggleCharacterWindow("STATUS")

	def __OnClickSkillPlusButton(self):
		self.ToggleCharacterWindow("SKILL")

	def __OnClickQuestButton(self):
		self.ToggleCharacterWindow("QUEST")

	def __OnClickHelpButton(self):
		player.SetPlayTime(1)
		self.CheckGameButton()
		self.OpenHelpWindow()

	def __OnClickBuildButton(self):
		self.BUILD_OpenWindow()

	def OpenHelpWindow(self):
		selfs.wndUICurtain.Show()
		selfs.wndHelp.Open()

	def CloseHelpWindow(self):
		selfs.wndUICurtain.Hide()
		selfs.wndHelp.Close()

	def BINARY_OpenSelectItemWindow(self):
		selfs.wndItemSelect.Open()

	def ClosePrivateShopInputNameDialog(self):
		selfs.wndInputDialog = None
		return True

	def OpenPrivateShopBuilder(self):
		if not selfs.wndInputDialog:
			return True

		if not len(selfs.wndInputDialog.GetText()):
			return True

		selfs.wndPrivateShopBuilder.Open(selfs.wndInputDialog.GetText())
		self.ClosePrivateShopInputNameDialog()
		return True

	def CloseEquipmentDialog(self, vid):
		if not vid in self.equipmentDialogDict:
			return

		del self.equipmentDialogDict[vid]

	def OpenWhisperDialogWithoutTarget(self):
		if not self.dlgWhisperWithoutTarget:
			dlgWhisper = uiWhisper.WhisperDialog(self.MinimizeWhisperDialog, self.CloseWhisperDialog)
			dlgWhisper.LoadDialog()
			dlgWhisper.OpenWithoutTarget(self.RegisterTemporaryWhisperDialog)
			dlgWhisper.SetPosition(self.windowOpenPosition*30,self.windowOpenPosition*30)
			dlgWhisper.Show()
			self.dlgWhisperWithoutTarget = dlgWhisper

			self.windowOpenPosition = (self.windowOpenPosition+1) % 5

		else:
			self.dlgWhisperWithoutTarget.SetTop()
			self.dlgWhisperWithoutTarget.OpenWithoutTarget(self.RegisterTemporaryWhisperDialog)

	def RegisterTemporaryWhisperDialog(self, name):
		if not self.dlgWhisperWithoutTarget:
			return

		btn = self.__FindWhisperButton(name)
		if 0 != btn:
			self.DestroyWhisperButton(btn)

		elif self.whisperDialogDict.has_key(name):
			oldDialog = self.whisperDialogDict[name]
			oldDialog.Destroy()
			del self.whisperDialogDict[name]

		self.whisperDialogDict[name] = self.dlgWhisperWithoutTarget
		self.dlgWhisperWithoutTarget.OpenWithTarget(name)
		self.dlgWhisperWithoutTarget = None
		self.__CheckGameMaster(name)

	def OpenWhisperDialog(self, name, empire = "", language = ""):
		if not self.whisperDialogDict.has_key(name):
			dlg = self.__MakeWhisperDialog(name)
			dlg.OpenWithTarget(name)
			if ENABLE_MULTI_LANGUAGE:
				if language != "" and empire != "":
					dlg.SetFlag(empire, language)
			else:
				if empire != "":
					dlg.SetFlag(empire)
			dlg.chatLine.SetFocus()
			dlg.Show()

			self.__CheckGameMaster(name)
			btn = self.__FindWhisperButton(name)
			if 0 != btn:
				if ENABLE_MULTI_LANGUAGE:
					dlg.SetFlag(btn.empireID, btn.languageID)
				self.DestroyWhisperButton(btn)

	def RecvWhisper(self, name, empire = "", language = ""):
		if not self.whisperDialogDict.has_key(name):
			btn = self.__FindWhisperButton(name)
			if 0 == btn:
				if ENABLE_MULTI_LANGUAGE:
					btn = self.__MakeWhisperButton(name, empire, language)
				else:
					btn = self.__MakeWhisperButton(name, empire)
				btn.Flash()
				chat.AppendChat(chat.CHAT_TYPE_NOTICE, localeInfo.RECEIVE_MESSAGE % (name))
			else:
				if ENABLE_MULTI_LANGUAGE:
					if language != "":
						btn.languageID = language
				if empire != "":
					btn.empireID = empire
				btn.Flash()
		elif self.IsGameMasterName(name):
			dlg = self.whisperDialogDict[name]
			dlg.SetGameMasterLook()
			if ENABLE_MULTI_LANGUAGE:
				if language != "" and empire != "":
					dlg.SetFlag(empire, language)
			else:
				if empire != "":
					dlg.SetFlag(empire)

	def MakeWhisperButton(self, name):
		self.__MakeWhisperButton(name)

	def ShowWhisperDialog(self, btn):
		try:
			self.__MakeWhisperDialog(btn.name)
			dlgWhisper = self.whisperDialogDict[btn.name]
			dlgWhisper.OpenWithTarget(btn.name)
			if ENABLE_MULTI_LANGUAGE:
				if btn.languageID != "" and btn.empireID != "":
					dlgWhisper.SetFlag(btn.empireID, btn.languageID)
			else:
				if btn.empireID != "":
					dlgWhisper.SetFlag(btn.empireID)
			dlgWhisper.Show()
			self.__CheckGameMaster(btn.name)
		except:
			import dbg
			dbg.TraceError("interface.ShowWhisperDialog - Failed to find key")

		self.DestroyWhisperButton(btn)

	def MinimizeWhisperDialog(self, name, empireID, languageID = None):
		if 0 != name:
			if ENABLE_MULTI_LANGUAGE:
				self.__MakeWhisperButton(name, empireID, languageID)
			else:
				self.__MakeWhisperButton(name, empireID)

		self.CloseWhisperDialog(name)

	def CloseWhisperDialog(self, name):
		if 0 == name:

			if self.dlgWhisperWithoutTarget:
				self.dlgWhisperWithoutTarget.Destroy()
				self.dlgWhisperWithoutTarget = None

			return

		try:
			dlgWhisper = self.whisperDialogDict[name]
			dlgWhisper.Destroy()
			del self.whisperDialogDict[name]
		except:
			import dbg
			dbg.TraceError("interface.CloseWhisperDialog - Failed to find key")

	def __ArrangeWhisperButton(self):
		screenWidth = wndMgr.GetScreenWidth()
		screenHeight = wndMgr.GetScreenHeight()

		xPos = screenWidth - 70
		yPos = 170 * screenHeight / 600
		yCount = (screenHeight - 330) / 63

		count = 0
		for button in self.whisperButtonList:

			button.SetPosition(xPos + (int(count/yCount) * -50), yPos + (count%yCount * 63))
			count += 1

	def __FindWhisperButton(self, name):
		for button in self.whisperButtonList:
			if button.name == name:
				return button

		return 0

	def __MakeWhisperDialog(self, name):
		dlgWhisper = uiWhisper.WhisperDialog(self.MinimizeWhisperDialog, self.CloseWhisperDialog)
		dlgWhisper.LoadDialog()
		dlgWhisper.SetPosition(self.windowOpenPosition*30,self.windowOpenPosition*30)
		self.whisperDialogDict[name] = dlgWhisper
		self.windowOpenPosition = (self.windowOpenPosition+1) % 5
		return dlgWhisper

	def __MakeWhisperButton(self, name, empireID = "", languageID = ""):
		whisperButton = uiWhisper.WhisperButton()
		whisperButton.SetUpVisual("d:/ymir work/ui/game/windows/btn_mail_up.sub")
		whisperButton.SetOverVisual("d:/ymir work/ui/game/windows/btn_mail_up.sub")
		whisperButton.SetDownVisual("d:/ymir work/ui/game/windows/btn_mail_up.sub")
		if self.IsGameMasterName(name):
			whisperButton.SetToolTipTextWithColor(name, 0xffffa200)
		else:
			whisperButton.SetToolTipText(name)
		whisperButton.ToolTipText.SetHorizontalAlignCenter()
		whisperButton.SetEvent(ui.__mem_func__(self.ShowWhisperDialog), whisperButton)
		whisperButton.Show()
		whisperButton.name = name
		whisperButton.empireID = empireID
		if ENABLE_MULTI_LANGUAGE:
			whisperButton.languageID = languageID

		self.whisperButtonList.insert(0, whisperButton)
		self.__ArrangeWhisperButton()
		return whisperButton

	def DestroyWhisperButton(self, button):
		button.SetEvent(0)
		self.whisperButtonList.remove(button)
		self.__ArrangeWhisperButton()

	def HideAllWhisperButton(self):
		for btn in self.whisperButtonList:
			btn.Hide()

	def ShowAllWhisperButton(self):
		for btn in self.whisperButtonList:
			btn.Show()

	def __CheckGameMaster(self, name):
		if not self.listGMName.has_key(name):
			return
		if self.whisperDialogDict.has_key(name):
			dlg = self.whisperDialogDict[name]
			dlg.SetGameMasterLook()

	def IsGameMasterName(self, name):
		if self.listGMName.has_key(name):
			return True
		else:
			return False

	def BUILD_OpenWindow(self):
		selfs.wndGuildBuilding = uiGuild.BuildGuildBuildingWindow()
		selfs.wndGuildBuilding.Open()
		selfs.wndGuildBuilding.wnds = self.__HideWindows()
		selfs.wndGuildBuilding.SetCloseEvent(ui.__mem_func__(self.BUILD_CloseWindow))

	def BUILD_CloseWindow(self):
		self.__ShowWindows(selfs.wndGuildBuilding.wnds)
		selfs.wndGuildBuilding = None

	def BUILD_OnUpdate(self):
		if not selfs.wndGuildBuilding:
			return

		if selfs.wndGuildBuilding.IsPositioningMode():
			import background
			x, y, z = background.GetPickingPoint()
			selfs.wndGuildBuilding.SetBuildingPosition(x, y, z)

	def BUILD_OnMouseLeftButtonDown(self):
		if not selfs.wndGuildBuilding:
			return

		if selfs.wndGuildBuilding.IsPositioningMode():
			selfs.wndGuildBuilding.SettleCurrentPosition()
			return True
		elif selfs.wndGuildBuilding.IsPreviewMode():
			pass
		else:
			return True
		return False

	def BUILD_OnMouseLeftButtonUp(self):
		if not selfs.wndGuildBuilding:
			return

		if not selfs.wndGuildBuilding.IsPreviewMode():
			return True

		return False

	def IsEditLineFocus(self):
		if self.ChatWindow.chatLine.IsFocus():
			return 1

		if self.ChatWindow.chatToLine.IsFocus():
			return 1

		return 0

	if ENABLE_FAST_EQUIP: # @@@correction029
		def ToggleEkipmanWindowStatusPage(self):
			import uifastequip
			self.wnd = uifastequip.changeequip()
			if selfs.FAST_EQUIP == 0:
				selfs.FAST_EQUIP = 1
				self.wnd.Show()
			else:
				selfs.FAST_EQUIP = 2

	if ENABLE_EXPANDED_TASKBAR: # @@@correction047
		def ToggleExpandedMoneyButton(self):
			if selfs.wndExpandedMoneyTaskBar.IsShow():
				selfs.wndExpandedMoneyTaskBar.Close()
			else:
				selfs.wndExpandedMoneyTaskBar.Open()

	if ENABLE_SEND_TARGET_INFO:
		def BINARY_AddTargetMonsterDropInfo(self, raceNum, itemVnum, itemCount):
			if not raceNum in constInfo.MONSTER_INFO_DATA:
				constInfo.MONSTER_INFO_DATA.update({raceNum : {}})
				constInfo.MONSTER_INFO_DATA[raceNum].update({"items" : []})
			curList = constInfo.MONSTER_INFO_DATA[raceNum]["items"]

			isUpgradeable = False
			isMetin = False
			item.SelectItem(itemVnum)
			if item.GetItemType() == item.ITEM_TYPE_WEAPON or item.GetItemType() == item.ITEM_TYPE_ARMOR:
				isUpgradeable = True
			elif item.GetItemType() == item.ITEM_TYPE_METIN:
				isMetin = True

			for curItem in curList:
				if isUpgradeable:
					if curItem.has_key("vnum_list") and curItem["vnum_list"][0] / 10 * 10 == itemVnum / 10 * 10:
						if not (itemVnum in curItem["vnum_list"]):
							curItem["vnum_list"].append(itemVnum)
						return
				elif isMetin:
					if curItem.has_key("vnum_list"):
						baseVnum = curItem["vnum_list"][0]
					if curItem.has_key("vnum_list") and (baseVnum - baseVnum%1000) == (itemVnum - itemVnum%1000):
						if not (itemVnum in curItem["vnum_list"]):
							curItem["vnum_list"].append(itemVnum)
						return
				else:
					if curItem.has_key("vnum") and curItem["vnum"] == itemVnum and curItem["count"] == itemCount:
						return

			if isUpgradeable or isMetin:
				curList.append({"vnum_list":[itemVnum], "count":itemCount})
			else:
				curList.append({"vnum":itemVnum, "count":itemCount})

		def BINARY_RefreshTargetMonsterDropInfo(self, raceNum):
			selfs.wndTargetBoard.RefreshMonsterInfoBoard()

	if ENABLE_MOVE_CHANNEL:
		def Update(self, ch):
			selfs.CHANNEL = ch
			import serverInfo
			net.SetServerInfo("%s, Kanal %s" % (serverInfo.REGION_DICT[0][SERVER]["name"], ch))

	if ENABLE_GUILD_DRAGONLAIR_SYSTEM:
		def OpenMeleyRanking(self):
			if selfs.wndInterFace:
				selfs.wndInterFace.OpenMeleyRanking()

		def AddRankMeleyRanking(self, data):
			if selfs.wndInterFace:
				line = int(data.split("#")[1])
				name = str(data.split("#")[2])
				members = int(data.split("#")[3])
				seconds = int(data.split("#")[4])
				minutes = seconds // 60
				seconds %= 60
				if seconds > 0:
					time = localeInfo.TIME_MIN_SEC % (minutes, seconds)
				else:
					time = localeInfo.TIME_MIN % (minutes)
				
				selfs.wndInterFace.RankMeleyRanking(line, name, members, time)

	if ENABLE_SUPPORT_SYSTEM:
		def OpenSupportGui(self):
			if selfs.wndSupportPG.IsShow():
				selfs.wndSupportPG.Close()
			else:
				selfs.wndSupportPG.Show()
				selfs.wndSupportPG.SetTop()
			
		def SetSupportArmor(self, armor):
			selfs.wndSupportPG.SetArmor(armor)
			constInfo.SUPPORT_ARMOR = int(armor)

		def SetSupportWeapon(self, weapon):
			selfs.wndSupportPG.SetWeapon(weapon)
			constInfo.SUPPORT_WEAPON = int(weapon)

		def SetSupportHair(self, hair):
			selfs.wndSupportPG.SetHair(hair)
			constInfo.SUPPORT_HAIR = int(hair)

		def SetSupportLevel(self, level):
			selfs.wndSupportPG.SetLevel(level)
			constInfo.SUPPORTLEVEL = int(level)
			
		def SetSupportName(self, name, name2):
			selfs.wndSupportPG.SetName(name, name2)	
			
		def SupportLeave(self):
			selfs.wndSupportPG.SetDefaultInfo()
			selfs.wndSupportPG.Close()
		
		def SupportInt(self , ints):
			selfs.wndSupportPG.SetInt(ints)
			
		def SetSupportExp(self, exp, exptot):
			if int(exptot) > 0:
				selfs.wndSupportPG.SetExperience(exp, exptot)
			selfs.wndSupportPG.SetExperience(exp, exptot)
			
		def SetSupportIcon(self, vnum):
			if int(vnum) > 0:
				selfs.wndSupportPG.SetImageSlot(vnum)
			selfs.wndSupportPG.SetImageSlot(vnum)
			
		def SetSupportSpeciality(self,speciality):
			selfs.wndSupportPG.SetSpeciality(speciality)

	if ENABLE_GROWTH_PET_SYSTEM:
		def SetPetEvolution(self, evo):
			selfs.wndPetMain.SetEvolveName(int(evo))
		
		def SetPetName(self, name):
			if len(name) > 1 and name != "":
				selfs.wndPetMini.Show()
			selfs.wndPetMain.SetName(name)
		
		def SetPetLevel(self, level):
			selfs.wndPetMain.SetLevel(level)
		
		def SetPetDuration(self, dur, durt, dura):
			if int(durt) > 0:
				selfs.wndPetMini.SetDuration(dur, durt, dura)
			selfs.wndPetMain.SetDuration(dur, durt, dura)
		
		def SetPetBonus(self, hp, dif, sp):
			selfs.wndPetMain.SetHp(hp)
			selfs.wndPetMain.SetDef(dif)
			selfs.wndPetMain.SetSp(sp)
			
		def SetPetskill(self, slot, idx, lv):
			selfs.wndPetMini.SetSkill(slot, idx, lv)
			selfs.wndPetMain.SetSkill(slot, idx, lv)
			selfs.wndAffectShower.BINARY_NEW_AddAffect(5400 + int(idx), int(constInfo.LASTAFFECT_POINT) + 1, int(constInfo.LASTAFFECT_VALUE) + 1, 0)
			if int(slot) == 0:
				constInfo.SKILL_PET1 = 5400 + int(idx)
			if int(slot) == 1:
				constInfo.SKILL_PET2 = 5400 + int(idx)
			if int(slot) == 2:
				constInfo.SKILL_PET3 = 5400 + int(idx)

		def SetPetIcon(self, vnum):
			if int(vnum) > 0:
				selfs.wndPetMini.SetImageSlot(vnum)
			selfs.wndPetMain.SetImageSlot(vnum)
			
		def SetPetExp(self, exp, expi, exptot):
			if int(exptot) > 0:
				selfs.wndPetMini.SetExperience(exp, expi, exptot)
			selfs.wndPetMain.SetExperience(exp, expi, exptot)
			
		def PetUnsummon(self):
			selfs.wndPetMini.SetDefaultInfo()
			selfs.wndPetMini.Close()
			selfs.wndPetMain.SetDefaultInfo()
			selfs.wndAffectShower.BINARY_NEW_RemoveAffect(int(constInfo.SKILL_PET1),0)
			selfs.wndAffectShower.BINARY_NEW_RemoveAffect(int(constInfo.SKILL_PET2),0)
			selfs.wndAffectShower.BINARY_NEW_RemoveAffect(int(constInfo.SKILL_PET3),0)
			constInfo.SKILL_PET1 = 0
			constInfo.SKILL_PET2 = 0
			constInfo.SKILL_PET3 = 0
		
		def OpenPetMainGui(self):
			selfs.wndPetMain.Show()
			selfs.wndPetMain.SetTop()

		def OpenPetIncubator(self, cell = -1, pet_new = 0):
			import uipetincubatrice
			# cell = int(cell) % 45
			self.petinc = uipetincubatrice.PetSystemIncubator(int(cell), int(pet_new))
			self.petinc.Show()
			self.petinc.SetTop()

		def OpenPetMini(self):
			selfs.wndPetMini.Show()
			selfs.wndPetMini.SetTop()

		def OpenPetFeed(self):
			self.feedwind = uipetfeed.PetFeedWindow()
			self.feedwind.Show()
			self.feedwind.SetTop()

		def Gift_Show(self):
			if constInfo.PET_MAIN == 0:
				selfs.wndPetMain.Show()
				constInfo.PET_MAIN =1
				selfs.wndPetMain.SetTop()
			else:
				selfs.wndPetMain.Hide()
				constInfo.PET_MAIN =0

		def ResultAttr(self, a, b, c, type, pos):
			if selfs.wndPetMain:
				selfs.wndPetMain.ResultAttr(int(a), int(b), int(c), int(type), int(pos))

		def PetType(self, type):
			if selfs.wndPetMain:
				selfs.wndPetMain.PetTypee(type)

	if ENABLE_BOSS_TRACKING:
		def BINARY_BOSS_TRACKING(self, kill_time, start_time, channel, mob_vnum, map_index):
			selfs.wndBossTracking.SetData(kill_time, start_time, channel, mob_vnum, map_index)

	if ENABLE_DEFENSE_WAVE:
		def BINARY_Update_Mast_HP(self, hp):
			if selfs.wndMiniMap:
				selfs.wndMiniMap.SetMastHP(int(hp))

		def BINARY_Update_Mast_Window(self, i):
			if selfs.wndMiniMap:
				selfs.wndMiniMap.SetMastWindow(int(i))

	if ENABLE_SHOW_CHEST_DROP:
		def BINARY_AddChestDropInfo(self, invType, invItemPos, pageIndex, slotIndex, itemVnum, itemCount):
			if selfs.wndChestDrop:
				selfs.wndChestDrop.AddChestDropItem(int(invType), int(invItemPos), int(pageIndex), int(slotIndex), int(itemVnum), int(itemCount))
						
		def BINARY_RefreshChestDropInfo(self, invType, invItemPos):
			if selfs.wndChestDrop:
				selfs.wndChestDrop.RefreshItems(invType, invItemPos)

	if ENABLE_MINI_GAME:
		def SetMiniGameEvent(self, Type, IsOpen):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.SetEvent(Type, IsOpen)

	if ENABLE_MINI_GAME_RUMI:
		def MiniGameRumiStart(self):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameRumiStart()

		def MiniGameRumiSetDeckCount(self, deck_card_count):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameRumiSetDeckCount(deck_card_count)

		def MiniGameRumiMoveCard(self, src_pos, src_index, src_color, src_number, dst_pos, dst_index, dst_color, dst_number):
			i = src_index
			if src_pos == 1:
				i = max(0, src_index / 8 - 1)
			srcCard = (src_pos, i, src_color, src_number)
			dstCard = (dst_pos, dst_index, dst_color, dst_number)
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameRumiMoveCard(srcCard, dstCard)

		def MiniGameRumiIncreaseScore(self, score, total_score):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameRumiIncreaseScore(score, total_score)

		def MiniGameRumiEnd(self):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameRumiEnd()

	if ENABLE_MINI_GAME_OKEY_NORMAL:
		def __MiniGameOkeyNormal(self, enable):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.SetOkeyNormalBG()

	if ENABLE_ACCUMULATE_DAMAGE_DISPLAY:
		def RefreshAccumulateCount(self, vid):
			if selfs.wndTargetBoard and vid == selfs.wndTargetBoard.GetTargetVID():
				selfs.wndTargetBoard.RefreshAccumulateCount(vid)

	if ENABLE_MONSTER_BACK or ENABLE_CARNIVAL2016:
		def MiniGameAttendanceSetData(self, type, value):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameAttendanceSetData(type, value)

		def MiniGameAttendanceRequestRewardList(self):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameAttendanceRequestRewardList()

	if ENABLE_FISH_EVENT:
		def MiniGameFishUse(self, window, shape):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameFishUse(window, shape)
			
		def MiniGameFishAdd(self, pos, shape):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameFishAdd(pos, shape)
			
		def MiniGameFishReward(self, vnum):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameFishReward(vnum)
			
		def MiniGameFishCount(self, count):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameFishCount(count)	

	if ENABLE_MINI_GAME_CATCH_KING:
		def MiniGameCatchKingEventStart(self, bigScore):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameCatchKingEventStart(bigScore)
					
		def MiniGameCatchKingSetHandCard(self, cardNumber):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameCatchKingSetHandCard(cardNumber)

		def MiniGameCatchKingResultField(self, score, rowType, cardPos, cardValue, keepFieldCard, destroyHandCard, getReward, isFiveNear):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameCatchKingResultField(score, rowType, cardPos, cardValue, keepFieldCard, destroyHandCard, getReward, isFiveNear)
					
		def MiniGameCatchKingSetEndCard(self, cardPos, cardNumber):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameCatchKingSetEndCard(cardPos, cardNumber)
					
		def MiniGameCatchKingReward(self, rewardCode):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.MiniGameCatchKingReward(rewardCode)

	if ENABLE_12ZI:			
		def Refresh12ziTimer(self, currentFloor, jumpCount, limitTime, elapseTime, isUp):
			if selfs.wndMiniMap:
				selfs.wndMiniMap.Hide()
				
			if not selfs.wnd12ziTimer:
				selfs.wnd12ziTimer = ui12zi.FloorLimitTimeWindow()
				
			selfs.wnd12ziTimer.Refresh12ziTimer(int(currentFloor), int(jumpCount), int(limitTime), int(elapseTime), int(isUp))
			selfs.wnd12ziTimer.Open()

		def Show12ziJumpButton(self):
			if selfs.wnd12ziTimer:
				selfs.wnd12ziTimer.Show12ziJumpButton()
			
		def Hide12ziTimer(self):
			if selfs.wnd12ziTimer:
				selfs.wnd12ziTimer.Hide()

		def OpenReviveDialog(self, vid, itemcount):
			selfs.wndTargetBoard.OpenReviveDialog(int(vid), int(itemcount))

		def RefreshShopItemToolTip(self, vnum):
			constInfo.SHOP_NO_LIMIT.append(int(vnum))
			if selfs.wndToolTipItem:
				selfs.wndToolTipItem.RefreshShopToolTip()

		def SetBeadCount(self, value):
			if selfs.wndBead:
				selfs.wndBead.SetBeadCount(int(value))
			
		def NextBeadUpdateTime(self, value):
			if selfs.wndBead:
				selfs.wndBead.NextBeadUpdateTime(int(value))

		def BINARY_SetMissionMessage(self, message):
			selfs.wndMissionBoard.SetMission(message)
			
		def BINARY_SetSubMissionMessage(self, message):
			selfs.wndMissionBoard.SetSubMission(message)
			
		def BINARY_CleanMissionMessage(self):
			selfs.wndMissionBoard.CleanMission()

		def BINARY_SetEnvironment(self, idx):
			self.indexEnv = idx
			selfs.wndCurtain.SAFE_FadeOut(self.__SetEnvironment)
		
		def __SetEnvironment(self):
			background.SetEnvironmentData(self.indexEnv)
			selfs.wndCurtain.FadeIn()
	
		def OpenZodiacReward(self, yellowmark, greenmark, yellowreward, greenreward, goldreward):
			if not selfs.wnd12ziReward:
				selfs.wnd12ziReward = ui12zi.Reward12ziWindow()

			selfs.wnd12ziReward.Open(int(yellowmark), int(greenmark), int(yellowreward), int(greenreward), int(goldreward))

		def Show_Button_Index(self, id):
			constInfo.SHOW_BUTTON = int(id)


	if ENABLE_DUNGEON_INFO_SYSTEM:
		def OpenDungeonInfoWindow(self):
			if selfs.wndMiniMap:
				selfs.wndMiniMap.OpenDungeonInfo()

	if ENABLE_OFFLINE_PRIVATE_SHOP:
		def OpenShopWindow(self):
			if selfs.wndMiniMap:
				selfs.wndMiniMap.ShopOpen()

		def DungeonInfo(self, questindex):
			constInfo.dungeonData["quest_index"] = questindex

		def CleanDungeonInfo(self):
			constInfo.dungeonInfo = []

		def CleanDungeonRanking(self):
			constInfo.dungeonRanking["ranking_list"] = []

		def GetDungeonInfo(self, cmd):
			cmd = cmd.split("#")

			if cmd[0] == "INPUT":
				constInfo.INPUT_IGNORE = int(cmd[1])

			elif cmd[0] == "CMD":
				net.SendQuestInputStringPacket(constInfo.dungeonData["quest_cmd"])
				constInfo.dungeonData["quest_cmd"] = ""

			else:
				pass

		def UpdateDungeonInfo(self, type, organization, levelLimit, partyMembers, mapName, mapIndex, mapCoordX, mapCoordY, cooldown, duration, entranceMapName, strengthBonusName, resistanceBonusName, itemVnum, finished, fastestTime, highestDamage,reptime):
			type = int(type)
			organization = int(organization)
			levelLimit = int(levelLimit)
			partyMembers = int(partyMembers)
			mapName = str(mapName).replace("_", " ")
			mapIndex = int(mapIndex)
			mapCoordX = int(mapCoordX)
			mapCoordY = int(mapCoordY)
			cooldown = int(cooldown)
			duration = int(duration)
			entranceMapName = str(entranceMapName).replace("_", " ")
			strengthBonusName = str(strengthBonusName).replace("_", " ")
			resistanceBonusName = str(resistanceBonusName).replace("_", " ")
			itemVnum = int(itemVnum)
			finished = int(finished)
			fastestTime = int(fastestTime)
			highestDamage = int(highestDamage)
			reptimex = int(reptime)

			constInfo.dungeonInfo.append(\
				{
					"type" : type,\
					"organization" : organization,\
					"level_limit" : levelLimit,\
					"party_members" : partyMembers,\
					"map" : mapName,\
					"map_index" : mapIndex,\
					"map_coord_x" : mapCoordX,\
					"map_coord_y" : mapCoordY,\
					"cooldown" : cooldown,\
					"duration" : duration,\
					"entrance_map" : entranceMapName,\
					"strength_bonus" : strengthBonusName,\
					"resistance_bonus" : resistanceBonusName,\
					"item_vnum" : itemVnum,
					"finished" : finished,
					"fastest_time" : fastestTime,
					"highest_damage" : highestDamage,
					"reptime" : reptimex,
				},
			)

		def UpdateDungeonRanking(self, name, level, pointType):
			name = str(name)
			level = int(level)
			pointType = int(pointType)
			constInfo.dungeonRanking["ranking_list"].append([name, level, pointType],)

		def OpenDungeonRanking(self):
			self.DungeonRank = uiDungeonInfo.DungeonRank()
			self.DungeonRank.Open()
			
	if ENABLE_SWITCHBOT:
		def ToggleSwitchbotWindow(self):
			if selfs.wndSwitchbot.IsShow():
				selfs.wndSwitchbot.Close()
			else:
				selfs.wndSwitchbot.Open()
				
		def RefreshSwitchbotWindow(self):
			if selfs.wndSwitchbot and selfs.wndSwitchbot.IsShow():
				selfs.wndSwitchbot.RefreshSwitchbotWindow()

		def RefreshSwitchbotItem(self, slot):
			if selfs.wndSwitchbot and selfs.wndSwitchbot.IsShow():
				selfs.wndSwitchbot.RefreshSwitchbotItem(slot)

	if ENABLE_FAST_RUHBK:
		def OpenFastRuh(self):
			selfs.wndFastRuh.Show()

		def OpenFastBK(self):
			selfs.wndFastBK.Show()

	if ENABLE_AFAR_BIOLOG:
		def biyoodulac(self, gorev, af1, afv1, af2, afv2, af3, afv3):
			selfs.wndBio.Show()	
			selfs.wndBio.SetBaslik(str(gorev))
			selfs.wndBio.SetOdul(af1,afv1,af2,afv2,af3,afv3)

		def biyologekrankapa(self):
			selfs.wndBio.Close()
			
		def biyolog(self, bioitem, verilen, toplam, kalansure):
			selfs.wndBio2.SetBiyolog(bioitem, verilen, toplam, kalansure)
			selfs.wndInventory.SetBiyolog(bioitem, verilen, toplam, kalansure)

		def CMDBiyologPenceresiAc(self):
			self.OpenBiologWindow()

		def OpenBiologWindow(self):
			selfs.wndBio2.Show()
			
	def getinputbegin(self):
		constInfo.INPUT_IGNORE = 1
	
	def getinputend(self):
		constInfo.INPUT_IGNORE = 0

	if ENABLE_GROWTH_PET_SYSTEM:
		def OpenInputNameDialogPet(self, slot):
			inputDialog = uiCommon.InputDialogName()
			inputDialog.SetTitle(localeInfo.PET_SYSTEM_CHANGE_NAME)
			inputDialog.SetAcceptEvent(ui.__mem_func__(self.ChangePetName))
			inputDialog.SetCancelEvent(ui.__mem_func__(self.ClosePrivateShopInputNameDialog))
			inputDialog.Open()
			inputDialog.slot = slot	
			selfs.wndInputDialog = inputDialog	
			
		def ChangePetName(self):
			net.SendChatPacket("/growth_change_namee %s %s" % (str(selfs.wndInputDialog.slot), str(selfs.wndInputDialog.GetText())))
			self.ClosePrivateShopInputNameDialog()

	if WJ_SECURITY_SYSTEM:
		def captcha(self, sayi1, sayi2, sayi3, sayi4, sayi5):
			captchatest = uiCommon.CaptchaEkran()
			captchatest.SetText1("|cffffff00Bot Engel Sistemi")
			captchatest.SetText3("Oyuna devam edebilmek için üstteki kodu seç.")
			captchatest.SetTimeOverMsg("Soruyu zamanynda cevaplamadýðýn için oyundan atýlýyorsun.")
			captchatest.SetTimeOverEvent(self.captchacevap)
			captchatest.Open(25, sayi1, sayi2, sayi3, sayi4, sayi5)
			selfs.wndCaptcha = captchatest

		def captchacevap(self):
			if not selfs.wndCaptcha:
				return

			net.SendChatPacket("/cp_a 0")
			selfs.wndCaptcha.Close()
			selfs.wndCaptcha = None

	if ENABLE_BLOCK_EXP:
		def ExpBlock(self, i):
			selfs.wndDialogSystem.ExpBlock(int(i))
