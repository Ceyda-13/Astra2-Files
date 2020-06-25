import ui
import uiScriptLocale
import wndMgr
import player
import miniMap
import localeInfo
import net
import app
import colorInfo
import constInfo
import background
import __builtin__ as selfs
if ENABLE_SOME_INFOS:
	import serverInfo
	import re
	import os
	import time
	import thread

if ENABLE_OFFLINE_PRIVATE_SHOP:
	import ops

class MapTextToolTip(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)

		textLine = ui.TextLine()
		textLine.SetParent(self)
		textLine.SetHorizontalAlignCenter()
		textLine.SetOutline()
		textLine.SetHorizontalAlignRight()
		textLine.Show()
		self.textLine = textLine

	def __del__(self):
		ui.Window.__del__(self)

	def SetText(self, text):
		self.textLine.SetText(text)

	def SetTooltipPosition(self, PosX, PosY):
		if localeInfo.IsARABIC():
			w, h = self.textLine.GetTextSize()
			self.textLine.SetPosition(PosX - w - 5, PosY)
		else:
			self.textLine.SetPosition(PosX - 5, PosY)

	def SetTextColor(self, TextColor):
		self.textLine.SetPackedFontColor(TextColor)

	def GetTextSize(self):
		return self.textLine.GetTextSize()

class AtlasWindow(ui.ScriptWindow):

	class AtlasRenderer(ui.Window):
		def __init__(self):
			ui.Window.__init__(self)
			self.AddFlag("not_pick")

		def OnUpdate(self):
			miniMap.UpdateAtlas()

		def OnRender(self):
			(x, y) = self.GetGlobalPosition()
			fx = float(x)
			fy = float(y)
			miniMap.RenderAtlas(fx, fy)

		def HideAtlas(self):
			miniMap.HideAtlas()

		def ShowAtlas(self):
			miniMap.ShowAtlas()

	def __init__(self):
		self.tooltipInfo = MapTextToolTip()
		self.tooltipInfo.Hide()
		self.infoGuildMark = ui.MarkBox()
		self.infoGuildMark.Hide()
		self.AtlasMainWindow = None
		self.mapName = ""
		self.board = 0

		ui.ScriptWindow.__init__(self)

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def SetMapName(self, mapName):
		if 949==app.GetDefaultCodePage():
			try:
				self.board.SetTitleName(localeInfo.MINIMAP_ZONE_NAME_DICT[mapName])
			except:
				pass

	def LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "UIScript/AtlasWindow.py")
		except:
			import exception
			exception.Abort("AtlasWindow.LoadWindow.LoadScript")

		try:
			self.board = self.GetChild("board")

		except:
			import exception
			exception.Abort("AtlasWindow.LoadWindow.BindObject")

		self.AtlasMainWindow = self.AtlasRenderer()
		self.board.SetCloseEvent(self.Hide)
		self.AtlasMainWindow.SetParent(self.board)
		self.AtlasMainWindow.SetPosition(7, 30)
		self.tooltipInfo.SetParent(self.board)
		self.infoGuildMark.SetParent(self.board)
		self.SetPosition(wndMgr.GetScreenWidth() - 136 - 256 - 10, 0)
		if ENABLE_ATLAS_WARPING:
			self.board.SetOnMouseLeftButtonUpEvent(ui.__mem_func__(self.OnMouseLeftButtonUpEvent))

		self.Hide()

		miniMap.RegisterAtlasWindow(self)

	def Destroy(self):
		miniMap.UnregisterAtlasWindow()
		self.ClearDictionary()
		self.AtlasMainWindow = None
		self.tooltipAtlasClose = 0
		self.tooltipInfo = None
		self.infoGuildMark = None
		self.board = None

	def OnUpdate(self):

		if not self.tooltipInfo:
			return

		if not self.infoGuildMark:
			return

		self.infoGuildMark.Hide()
		self.tooltipInfo.Hide()

		if False == self.board.IsIn():
			return

		(mouseX, mouseY) = wndMgr.GetMousePosition()
		(bFind, sName, iPosX, iPosY, dwTextColor, dwGuildID) = miniMap.GetAtlasInfo(mouseX, mouseY)

		if False == bFind:
			return

		if "empty_guild_area" == sName:
			sName = localeInfo.GUILD_EMPTY_AREA

		if localeInfo.IsARABIC() and sName[-1].isalnum():
			self.tooltipInfo.SetText("(%s)%d, %d" % (sName, iPosX, iPosY))
		else:
			self.tooltipInfo.SetText("%s(%d, %d)" % (sName, iPosX, iPosY))

		(x, y) = self.GetGlobalPosition()
		self.tooltipInfo.SetTooltipPosition(mouseX - x, mouseY - y)
		self.tooltipInfo.SetTextColor(dwTextColor)
		self.tooltipInfo.Show()
		self.tooltipInfo.SetTop()

		if 0 != dwGuildID:
			textWidth, textHeight = self.tooltipInfo.GetTextSize()
			self.infoGuildMark.SetIndex(dwGuildID)
			self.infoGuildMark.SetPosition(mouseX - x - textWidth - 18 - 5, mouseY - y)
			self.infoGuildMark.Show()

	def Hide(self):
		if self.AtlasMainWindow:
			self.AtlasMainWindow.HideAtlas()
			self.AtlasMainWindow.Hide()
		ui.ScriptWindow.Hide(self)

	def Show(self):
		if self.AtlasMainWindow:
			(bGet, iSizeX, iSizeY) = miniMap.GetAtlasSize()
			if bGet:
				self.SetSize(iSizeX + 15, iSizeY + 38)

				if localeInfo.IsARABIC():
					self.board.SetPosition(iSizeX+15, 0)

				self.board.SetSize(iSizeX + 15, iSizeY + 38)
				#self.AtlasMainWindow.SetSize(iSizeX, iSizeY)
				self.AtlasMainWindow.ShowAtlas()
				self.AtlasMainWindow.Show()
		ui.ScriptWindow.Show(self)

	def SetCenterPositionAdjust(self, x, y):
		self.SetPosition((wndMgr.GetScreenWidth() - self.GetWidth()) / 2 + x, (wndMgr.GetScreenHeight() - self.GetHeight()) / 2 + y)

	def OnPressEscapeKey(self):
		self.Hide()
		return True

	if ENABLE_ATLAS_WARPING:
		def OnMouseLeftButtonUpEvent(self):
			(mouseX, mouseY) = wndMgr.GetMousePosition()
			(bFind, sName, iPosX, iPosY, dwTextColor, dwGuildID) = miniMap.GetAtlasInfo(mouseX, mouseY)
			net.SendChatPacket("/go %s %s" % (str(iPosX), str(iPosY)))

def __RegisterMiniMapColor(type, rgb):
	miniMap.RegisterColor(type, rgb[0], rgb[1], rgb[2])

class MiniMap(ui.ScriptWindow):

	CANNOT_SEE_INFO_MAP_DICT = {
		"metin2_map_monkeydungeon" : False,
		"metin2_map_monkeydungeon_02" : False,
		"metin2_map_monkeydungeon_03" : False,
		"metin2_map_devilsCatacomb" : False,
	}

	if ENABLE_12ZI:
		CANNOT_SEE_INFO_MAP_DICT.update({
			"metin2_12zi_stage" : False,
		})

	def __init__(self):
		ui.ScriptWindow.__init__(self)

		self.__Initialize()

		miniMap.Create()
		miniMap.SetScale(2.0)

		self.AtlasWindow = AtlasWindow()
		self.AtlasWindow.LoadWindow()
		self.AtlasWindow.Hide()

		self.tooltipMiniMapOpen = MapTextToolTip()
		self.tooltipMiniMapOpen.SetText(localeInfo.MINIMAP)
		self.tooltipMiniMapOpen.Show()
		self.tooltipMiniMapClose = MapTextToolTip()
		self.tooltipMiniMapClose.SetText(localeInfo.UI_CLOSE)
		self.tooltipMiniMapClose.Show()
		self.tooltipScaleUp = MapTextToolTip()
		self.tooltipScaleUp.SetText(localeInfo.MINIMAP_INC_SCALE)
		self.tooltipScaleUp.Show()
		self.tooltipScaleDown = MapTextToolTip()
		self.tooltipScaleDown.SetText(localeInfo.MINIMAP_DEC_SCALE)
		self.tooltipScaleDown.Show()
		self.tooltipAtlasOpen = MapTextToolTip()
		self.tooltipAtlasOpen.SetText(localeInfo.MINIMAP_SHOW_AREAMAP)
		self.tooltipAtlasOpen.Show()
		self.tooltipInfo = MapTextToolTip()
		self.tooltipInfo.Show()

		if miniMap.IsAtlas():
			self.tooltipAtlasOpen.SetText(localeInfo.MINIMAP_SHOW_AREAMAP)
		else:
			self.tooltipAtlasOpen.SetText(localeInfo.MINIMAP_CAN_NOT_SHOW_AREAMAP)

		self.tooltipInfo = MapTextToolTip()
		self.tooltipInfo.Show()

		self.mapName = ""

		self.isLoaded = 0
		self.canSeeInfo = True

		# AUTOBAN
		self.imprisonmentDuration = 0
		self.imprisonmentEndTime = 0
		self.imprisonmentEndTimeText = ""
		# END_OF_AUTOBAN

	def __del__(self):
		miniMap.Destroy()
		ui.ScriptWindow.__del__(self)

	def __Initialize(self):
		self.positionInfo = 0
		self.observerCount = 0

		self.OpenWindow = 0
		self.CloseWindow = 0
		self.ScaleUpButton = 0
		self.ScaleDownButton = 0
		self.MiniMapHideButton = 0
		self.MiniMapShowButton = 0
		self.AtlasShowButton = 0

		self.tooltipMiniMapOpen = 0
		self.tooltipMiniMapClose = 0
		self.tooltipScaleUp = 0
		self.tooltipScaleDown = 0
		self.tooltipAtlasOpen = 0
		self.tooltipInfo = None
		self.serverInfo = None

		if ENABLE_SOME_INFOS:
			self.FPS = 0
			# self.Ping = 0
			self.Day = 0
			self.Time = 0

		if ENABLE_OFFLINE_PRIVATE_SHOP:
			self.ShopSearchButton = None
			self.time = 0

		if ENABLE_DEFENSE_WAVE:
			self.MastHp = 0

		if ENABLE_MINI_GAME:
			self.MiniGame = None

		if ENABLE_DUNGEON_INFO_SYSTEM:
			self.DungeonInfo = None

	def SetMapName(self, mapName):
		self.mapName=mapName
		self.AtlasWindow.SetMapName(mapName)

		if self.CANNOT_SEE_INFO_MAP_DICT.has_key(mapName):
			self.canSeeInfo = False
			self.HideMiniMap()
			self.tooltipMiniMapOpen.SetText(localeInfo.MINIMAP_CANNOT_SEE)
		else:
			self.canSeeInfo = True
			self.ShowMiniMap()
			self.tooltipMiniMapOpen.SetText(localeInfo.MINIMAP)

	# AUTOBAN
	def SetImprisonmentDuration(self, duration):
		self.imprisonmentDuration = duration
		self.imprisonmentEndTime = app.GetGlobalTimeStamp() + duration

		self.__UpdateImprisonmentDurationText()

	def __UpdateImprisonmentDurationText(self):
		restTime = max(self.imprisonmentEndTime - app.GetGlobalTimeStamp(), 0)

		imprisonmentEndTimeText = localeInfo.SecondToDHM(restTime)
		if imprisonmentEndTimeText != self.imprisonmentEndTimeText:
			self.imprisonmentEndTimeText = imprisonmentEndTimeText
			self.serverInfo.SetText("%s: %s" % (uiScriptLocale.AUTOBAN_QUIZ_REST_TIME, self.imprisonmentEndTimeText))
	# END_OF_AUTOBAN

	def Show(self):
		self.__LoadWindow()

		ui.ScriptWindow.Show(self)

	def __LoadWindow(self):
		if self.isLoaded == 1:
			return

		self.isLoaded = 1

		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/Minimap.py")
		except:
			import exception
			exception.Abort("MiniMap.LoadWindow.LoadScript")

		try:
			self.OpenWindow = self.GetChild("OpenWindow")
			self.MiniMapWindow = self.GetChild("MiniMapWindow")
			self.ScaleUpButton = self.GetChild("ScaleUpButton")
			if ENABLE_NEW_MINIMAP:
				self.GetChild("OpenWindowBGI2").Hide()
				self.GetChild("ScaleUpButton2").Hide()
				self.GetChild("ScaleUpButton2").SetEvent(ui.__mem_func__(self.ScaleUp))
				self.GetChild("ScaleDownButton2").Hide()
				self.GetChild("ScaleDownButton2").SetEvent(ui.__mem_func__(self.ScaleDown))
				self.GetChild("MiniMapHideButton2").Hide()
				self.GetChild("MiniMapHideButton2").SetEvent(ui.__mem_func__(self.HideMiniMap))
				self.GetChild("AtlasShowButton2").Hide()
				if miniMap.IsAtlas():
					self.GetChild("AtlasShowButton2").SetEvent(ui.__mem_func__(self.ShowAtlas))
			self.ScaleDownButton = self.GetChild("ScaleDownButton")
			self.MiniMapHideButton = self.GetChild("MiniMapHideButton")
			self.AtlasShowButton = self.GetChild("AtlasShowButton")
			self.CloseWindow = self.GetChild("CloseWindow")
			self.MiniMapShowButton = self.GetChild("MiniMapShowButton")
			self.positionInfo = self.GetChild("PositionInfo")
			self.observerCount = self.GetChild("ObserverCount")
			self.serverInfo = self.GetChild("ServerInfo")
			if ENABLE_SOME_INFOS:
				self.FPS = self.GetChild("FPS")
				# self.Ping = self.GetChild("Ping")
				self.Day = self.GetChild("Day")
				self.Clock = self.GetChild("Clock")
			if ENABLE_OFFLINE_PRIVATE_SHOP:
				self.ShopSearchButton = self.GetChild("ShopSearchButton")
			if ENABLE_BOSS_TRACKING:
				self.BossTracking = self.GetChild("BossTrackingButton")
			if ENABLE_DEFENSE_WAVE:
				self.MastHp = self.GetChild("MastHp")
				self.MastWindow = self.GetChild("MastWindow")
				self.MastHp.OnMouseOverIn = ui.__mem_func__(self.MastHp.ShowToolTip)
				self.MastHp.OnMouseOverOut = ui.__mem_func__(self.MastHp.HideToolTip)
			if ENABLE_MINI_GAME:
				self.MiniGame = self.GetChild("minigame_button")
			if ENABLE_DUNGEON_INFO_SYSTEM:
				self.DungeonInfo = self.GetChild("DungeonInfoButton")
		except:
			import exception
			exception.Abort("MiniMap.LoadWindow.Bind")

		if constInfo.MINIMAP_POSITIONINFO_ENABLE==0:
			self.positionInfo.Hide()

		if ENABLE_DEFENSE_WAVE:
			self.MastHp.SetPercentage(5000000, 5000000)
			self.MastHp.SetToolTipText("HP:  5000000 /5000000")
			self.MastWindow.Hide()

		self.serverInfo.SetText(net.GetServerInfo())
		self.ScaleUpButton.SetEvent(ui.__mem_func__(self.ScaleUp))
		self.ScaleDownButton.SetEvent(ui.__mem_func__(self.ScaleDown))
		self.MiniMapHideButton.SetEvent(ui.__mem_func__(self.HideMiniMap))
		self.MiniMapShowButton.SetEvent(ui.__mem_func__(self.ShowMiniMap))
		if ENABLE_OFFLINE_PRIVATE_SHOP:
			self.ShopSearchButton.SetEvent(ui.__mem_func__(self.ShopOpen))

		if miniMap.IsAtlas():
			self.AtlasShowButton.SetEvent(ui.__mem_func__(self.ShowAtlas))

		if ENABLE_BOSS_TRACKING:
			self.BossTracking.SetEvent(ui.__mem_func__(self.OpenBossTracking))

		if ENABLE_MINI_GAME:
			self.MiniGame.SetEvent(ui.__mem_func__(self.OpenMiniGame))

		if ENABLE_DUNGEON_INFO_SYSTEM:
			self.DungeonInfo.SetEvent(ui.__mem_func__(self.OpenDungeonInfo))

		(ButtonPosX, ButtonPosY) = self.MiniMapShowButton.GetGlobalPosition()
		self.tooltipMiniMapOpen.SetTooltipPosition(ButtonPosX, ButtonPosY)

		(ButtonPosX, ButtonPosY) = self.MiniMapHideButton.GetGlobalPosition()
		self.tooltipMiniMapClose.SetTooltipPosition(ButtonPosX, ButtonPosY)

		(ButtonPosX, ButtonPosY) = self.ScaleUpButton.GetGlobalPosition()
		self.tooltipScaleUp.SetTooltipPosition(ButtonPosX, ButtonPosY)

		(ButtonPosX, ButtonPosY) = self.ScaleDownButton.GetGlobalPosition()
		self.tooltipScaleDown.SetTooltipPosition(ButtonPosX, ButtonPosY)

		(ButtonPosX, ButtonPosY) = self.AtlasShowButton.GetGlobalPosition()
		self.tooltipAtlasOpen.SetTooltipPosition(ButtonPosX, ButtonPosY)

		self.ShowMiniMap()
		# if ENABLE_SOME_INFOS:
			# def Ping(self):
				# while 1:
					# ping = int(re.findall("[0-9][0-9]", str(os.popen("%WINDIR%/system32/ping -n 1 " + serverInfo.SRVS[SERVER]["HOST"]).readlines()[-1]))[0])
					# if self.Ping:
						# self.Ping.SetText("Ping: %d" % (ping))
					# time.sleep(1)

			# thread.start_new_thread(Ping, (self,))

	def Destroy(self):
		self.HideMiniMap()

		self.AtlasWindow.Destroy()
		self.AtlasWindow = None

		self.ClearDictionary()

		self.__Initialize()

	def GetMapName(self):
		return self.mapName

	def UpdateObserverCount(self, observerCount):
		if observerCount>0:
			self.observerCount.Show()
		elif observerCount<=0:
			self.observerCount.Hide()

		self.observerCount.SetText(localeInfo.MINIMAP_OBSERVER_COUNT % observerCount)

	def OnUpdate(self):
		if ENABLE_SOME_INFOS:
			self.FPS.SetText("FPS: %3d" % (app.GetUpdateFPS()))
			self.Day.SetText(time.strftime("%a, %d %b %Y"))
			self.Clock.SetText(time.strftime("[%X]"))

		(x, y, z) = player.GetMainCharacterPosition()
		miniMap.Update(x, y)

		self.positionInfo.SetText("(%.0f, %.0f)" % (x/100, y/100))

		if self.tooltipInfo:
			if True == self.MiniMapWindow.IsIn():
				(mouseX, mouseY) = wndMgr.GetMousePosition()
				(bFind, sName, iPosX, iPosY, dwTextColor) = miniMap.GetInfo(mouseX, mouseY)
				if bFind == 0:
					self.tooltipInfo.Hide()
				elif not self.canSeeInfo:
					self.tooltipInfo.SetText("%s(%s)" % (sName, localeInfo.UI_POS_UNKNOWN))
					self.tooltipInfo.SetTooltipPosition(mouseX - 5, mouseY)
					self.tooltipInfo.SetTextColor(dwTextColor)
					self.tooltipInfo.Show()
				else:
					if localeInfo.IsARABIC() and sName[-1].isalnum():
						self.tooltipInfo.SetText("(%s)%d, %d" % (sName, iPosX, iPosY))
					else:
						self.tooltipInfo.SetText("%s(%d, %d)" % (sName, iPosX, iPosY))
					self.tooltipInfo.SetTooltipPosition(mouseX - 5, mouseY)
					self.tooltipInfo.SetTextColor(dwTextColor)
					self.tooltipInfo.Show()
			else:
				self.tooltipInfo.Hide()

			# AUTOBAN
			if self.imprisonmentDuration:
				self.__UpdateImprisonmentDurationText()
			# END_OF_AUTOBAN

		if True == self.MiniMapShowButton.IsIn():
			self.tooltipMiniMapOpen.Show()
		else:
			self.tooltipMiniMapOpen.Hide()

		if True == self.MiniMapHideButton.IsIn():
			self.tooltipMiniMapClose.Show()
		else:
			self.tooltipMiniMapClose.Hide()

		if True == self.ScaleUpButton.IsIn():
			self.tooltipScaleUp.Show()
		else:
			self.tooltipScaleUp.Hide()

		if True == self.ScaleDownButton.IsIn():
			self.tooltipScaleDown.Show()
		else:
			self.tooltipScaleDown.Hide()

		if True == self.AtlasShowButton.IsIn():
			self.tooltipAtlasOpen.Show()
		else:
			self.tooltipAtlasOpen.Hide()

	def OnRender(self):
		(x, y) = self.GetGlobalPosition()
		fx = float(x)
		fy = float(y)
		if ENABLE_NEW_MINIMAP and MINIMAP:
			miniMap.Render(fx + 38.0, fy + 26.5)
		else:
			if ENABLE_12ZI and not localeInfo.IsARABIC():
				miniMap.Render(fx + 19.0, fy + 5.0)
			else:
				miniMap.Render(fx + 4.0, fy + 5.0)

	def Close(self):
		self.HideMiniMap()

	def HideMiniMap(self):
		miniMap.Hide()
		self.OpenWindow.Hide()
		self.CloseWindow.Show()

	def ShowMiniMap(self):
		if not self.canSeeInfo:
			return

		miniMap.Show()
		self.OpenWindow.Show()
		self.CloseWindow.Hide()

	def isShowMiniMap(self):
		return miniMap.isShow()

	def ScaleUp(self):
		miniMap.ScaleUp()

	def ScaleDown(self):
		miniMap.ScaleDown()

	def ShowAtlas(self):
		if not miniMap.IsAtlas():
			return
		if not self.AtlasWindow.IsShow():
			self.AtlasWindow.Show()

	def ToggleAtlasWindow(self):
		if not miniMap.IsAtlas():
			return
		if self.AtlasWindow.IsShow():
			self.AtlasWindow.Hide()
		else:
			self.AtlasWindow.Show()

	if ENABLE_NEW_MINIMAP:
		def ChangeMiniMap(self, i):
			if i:
				(a, b) = self.GetLocalPosition()
				self.SetPosition(a - 36, b)
				self.GetChild("OpenWindowBGI").Hide()
				self.GetChild("OpenWindowBGI2").Show()
				self.MiniMapWindow.SetPosition(56, 5)
				self.ScaleUpButton.Hide()
				self.GetChild("ScaleUpButton2").Show()
				self.ScaleDownButton.Hide()
				self.GetChild("ScaleDownButton2").Show()
				self.MiniMapHideButton.Hide()
				self.GetChild("MiniMapHideButton2").Show()
				self.AtlasShowButton.Hide()
				self.GetChild("AtlasShowButton2").Show()
				self.serverInfo.SetPosition(56 + 52, 5.8)
				self.positionInfo.SetPosition(52 + 52, 157)
				self.observerCount.SetPosition(70, 200)
				if ENABLE_OFFLINE_PRIVATE_SHOP:
					self.ShopSearchButton.SetPosition(-27 + 52, 120)
				if ENABLE_BOSS_TRACKING:
					self.BossTracking.SetPosition(-5 + 52, 139)
				if ENABLE_DUNGEON_INFO_SYSTEM:
					self.DungeonInfo.SetPosition(-33 + 52, 98)
				self.Day.SetPosition(52 + 52, 180 + 6)
				self.Clock.SetPosition(52 + 52, 192 + 6)
				if ENABLE_MINI_GAME:
					selfs.wndMiniGame.SetPosition(6 + 9, 28 + 16)
				# if ENABLE_12ZI:
					# selfs.wndBead.SetPos(-8‬, 134 - 39)
			else:
				(a, b) = self.GetLocalPosition()
				self.SetPosition(a + 36, b)
				self.GetChild("OpenWindowBGI").Show()
				self.GetChild("OpenWindowBGI2").Hide()
				self.MiniMapWindow.SetPosition(18, 5)
				self.ScaleUpButton.Show()
				self.GetChild("ScaleUpButton2").Hide()
				self.ScaleDownButton.Show()
				self.GetChild("ScaleDownButton2").Hide()
				self.MiniMapHideButton.Show()
				self.GetChild("MiniMapHideButton2").Hide()
				self.AtlasShowButton.Show()
				self.GetChild("AtlasShowButton2").Hide()
				self.serverInfo.SetPosition(84, 140)
				self.positionInfo.SetPosition(84, 160)
				self.observerCount.SetPosition(84, 180)
				if ENABLE_OFFLINE_PRIVATE_SHOP:
					self.ShopSearchButton.SetPosition(16, 104)
				if ENABLE_BOSS_TRACKING:
					self.BossTracking.SetPosition(39, 119)
				if ENABLE_DUNGEON_INFO_SYSTEM:
					self.DungeonInfo.SetPosition(1 + 4, 59 + 22)
				self.Day.SetPosition(84, 175)
				self.Clock.SetPosition(84, 187)
				if ENABLE_MINI_GAME:
					selfs.wndMiniGame.SetPosition(6, 28)
				# if ENABLE_12ZI:
					# selfs.wndBead.SetPos(0, 134)

	if ENABLE_OFFLINE_PRIVATE_SHOP:
		def ShopOpen(self):
			if selfs.wndOfflinePrivateShopEdit:
				if selfs.wndOfflinePrivateShopEdit.IsShow():
					selfs.wndOfflinePrivateShopEdit.Close()
				else:
					selfs.wndOfflinePrivateShopEdit.Open()
					if self.time <= app.GetGlobalTimeStamp():
						constInfo.PriceCheck = {}
						net.SendChatPacket("/price_check")
						ops.Clear(0)
						net.SendChatPacket("/click_offline_private_shop 0")
						self.time = app.GetGlobalTimeStamp() + 10

	if ENABLE_BOSS_TRACKING:
		def OpenBossTracking(self):
			if selfs.wndBossTracking:
				if selfs.wndBossTracking.IsShow():
					selfs.wndBossTracking.Close()
				else:
					selfs.wndBossTracking.Open()

	if ENABLE_DEFENSE_WAVE:
		def SetMastHP(self, hp):
			self.MastHp.SetPercentage(hp, 5000000)
			self.MastHp.SetToolTipText("HP:  %d /5000000" % hp)

		def SetMastWindow(self, i):
			if i:
				self.MastWindow.Show()
			else:
				self.MastWindow.Hide()

	if ENABLE_MINI_GAME:
		def OpenMiniGame(self):
			if selfs.wndMiniGame:
				selfs.wndMiniGame.ClickIntegrationEventBannerButton()

	if ENABLE_DUNGEON_INFO_SYSTEM:
		def OpenDungeonInfo(self):
			if selfs.wndDungeonInfo:
				if selfs.wndDungeonInfo.IsShow():
					selfs.wndDungeonInfo.Close()
				else:
					selfs.wndDungeonInfo.Open()
