import dbg
import ui
import snd
import systemSetting
import net
import chat
import app
import localeInfo
import constInfo
import chrmgr
import player
import musicInfo

import uiSelectMusic
import background

MUSIC_FILENAME_MAX_LEN = 25
if ENABLE_GRAPHIC_ON_OFF:
	GRAPHIC_LEVEL_MAX_NUM = 5

blockMode = 0

class OptionDialog(ui.ScriptWindow):
	if ENABLE_MULTI_LANGUAGE:
		class Item(ui.ListBoxEx.Item):
			def __init__(self, text):
				ui.ListBoxEx.Item.__init__(self)
				self.canLoad = 0
				self.text = text
				self.textLine = self.__CreateTextLine(text)
			def __del__(self):
				ui.ListBoxEx.Item.__del__(self)

			def GetText(self):
				return self.text

			def SetSize(self, width, height):
				ui.ListBoxEx.Item.SetSize(self, 200, height)

			def __CreateTextLine(self, text):
				textLine = ui.TextLine()
				textLine.SetParent(self)
				textLine.SetPosition(85, 0)
				textLine.SetText(text)
				textLine.Show()
				return textLine

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.__Initialize()
		self.__Load()
		if ENABLE_GRAPHIC_ON_OFF:
			self.RefreshPetStatus()
			self.RefreshNpcNameStatus()
		if ENABLE_ENVIRONMENT_EFFECT_OPTION:
			import uiPhaseCurtain
			self.curtain = uiPhaseCurtain.PhaseCurtain()
			self.curtain.speed = 0.03
			self.curtain.Hide()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __Initialize(self):
		self.titleBar = 0
		self.changeMusicButton = 0
		self.selectMusicFile = 0
		self.ctrlMusicVolume = 0
		self.ctrlSoundVolume = 0
		self.musicListDlg = 0
		self.cameraModeButtonList = []
		self.ctrlShadowQuality = 0
		if not ENABLE_DISABLE_SOFTWARE_TILING:
			self.tilingMode = 0
			self.tilingApplyButton = 0
			self.tilingModeButtonList = []

		if ENABLE_ENVIRONMENT_EFFECT_OPTION:
			self.nightModeButtonList = []
			self.snowModeButtonList = []
			self.snowTextureModeButtonList = []
			self.curtain = None

		if ENABLE_FOG_FIX:
			self.fogButtonList = []
		else:
			self.fogModeButtonList = []

		if ENABLE_GRAPHIC_ON_OFF:
			self.effectLevel = 0
			self.effectLevelApplyButton = 0
			self.effectLevelButtonList = []

			self.privateShopLevel = 0
			self.privateShopLevelApplyButton = 0
			self.privateShopLevelButtonList = []

			self.dropItemLevel = 0
			self.dropItemLevelApplyButton = 0
			self.dropItemLevelButtonList = []

			self.petStatusButtonList = []
			self.npcNameStatusButtonList = []

		if ENABLE_MULTI_LANGUAGE:
			self.language_select_pivot_window = 0
			self.cur_language_text = 0
			self.language_select_button = 0
			self.language_change_button = 0
			self.multi_language_bar = 0
			self.multi_language_listbox = 0
			self.multi_language_scrollbar = 0

	def Destroy(self):
		self.ClearDictionary()
		self.__Initialize()

	def __Load_LoadScript(self, fileName):
		try:
			pyScriptLoader = ui.PythonScriptLoader()
			pyScriptLoader.LoadScriptFile(self, fileName)
		except:
			import exception
			exception.Abort("System.OptionDialog.__Load_LoadScript")

	def __Load_BindObject(self):
		try:
			GetObject = self.GetChild
			self.titleBar = GetObject("titlebar")
			self.selectMusicFile = GetObject("bgm_file")
			self.changeMusicButton = GetObject("bgm_button")
			self.ctrlMusicVolume = GetObject("music_volume_controller")
			self.ctrlSoundVolume = GetObject("sound_volume_controller")
			self.cameraModeButtonList.append(GetObject("camera_short"))
			self.cameraModeButtonList.append(GetObject("camera_long"))

			if ENABLE_FOG_FIX:
				self.fogButtonList.append(GetObject("fog_off"))
				self.fogButtonList.append(GetObject("fog_on"))
			else:
				self.fogModeButtonList.append(GetObject("fog_level0"))
				self.fogModeButtonList.append(GetObject("fog_level1"))
				self.fogModeButtonList.append(GetObject("fog_level2"))

			if not ENABLE_DISABLE_SOFTWARE_TILING:
				self.tilingModeButtonList.append(GetObject("tiling_cpu"))
				self.tilingModeButtonList.append(GetObject("tiling_gpu"))
				self.tilingApplyButton = GetObject("tiling_apply")
			self.ctrlShadowQuality=GetObject("shadow_bar")

			if ENABLE_ENVIRONMENT_EFFECT_OPTION:
				self.nightModeButtonList.append(GetObject("night_mode_off"))
				self.nightModeButtonList.append(GetObject("night_mode_on"))
				self.snowModeButtonList.append(GetObject("snow_mode_off"))
				self.snowModeButtonList.append(GetObject("snow_mode_on"))
				self.snowTextureModeButtonList.append(GetObject("snow_texture_mode_off"))
				self.snowTextureModeButtonList.append(GetObject("snow_texture_mode_on"))

			if ENABLE_GRAPHIC_ON_OFF:
				for i in xrange(1, GRAPHIC_LEVEL_MAX_NUM + 1):
					self.effectLevelButtonList.append(GetObject("effect_level%d" % i))
					self.privateShopLevelButtonList.append(GetObject("privateShop_level%d" % i))
					self.dropItemLevelButtonList.append(GetObject("dropItem_level%d" % i))

				self.effectLevelApplyButton=GetObject("effect_apply")
				self.privateShopLevelApplyButton=GetObject("privateShop_apply")
				self.dropItemLevelApplyButton=GetObject("dropItem_apply")

				self.petStatusButtonList.append(GetObject("pet_on"))
				self.petStatusButtonList.append(GetObject("pet_off"))

				self.npcNameStatusButtonList.append(GetObject("npcName_on"))
				self.npcNameStatusButtonList.append(GetObject("npcName_off"))

			if ENABLE_MULTI_LANGUAGE:
				self.language_select_pivot_window = GetObject("language_select_pivot_window")
				self.cur_language_text = GetObject("cur_language_text")
				self.language_select_button = GetObject("language_select_button")
				self.language_change_button = GetObject("language_change_button")
		except:
			import exception
			exception.Abort("OptionDialog.__Load_BindObject")

	def __Load(self):
		self.__Load_LoadScript("uiscript/systemoptiondialog.py")
		self.__Load_BindObject()

		self.SetCenterPosition()

		self.titleBar.SetCloseEvent(ui.__mem_func__(self.Close))

		self.ctrlMusicVolume.SetSliderPos(float(systemSetting.GetMusicVolume()))
		self.ctrlMusicVolume.SetEvent(ui.__mem_func__(self.OnChangeMusicVolume))

		self.ctrlSoundVolume.SetSliderPos(float(systemSetting.GetSoundVolume()) / 5.0)
		self.ctrlSoundVolume.SetEvent(ui.__mem_func__(self.OnChangeSoundVolume))

		self.ctrlShadowQuality.SetSliderPos(float(systemSetting.GetShadowLevel()) / 5.0)
		self.ctrlShadowQuality.SetEvent(ui.__mem_func__(self.OnChangeShadowQuality))

		self.changeMusicButton.SAFE_SetEvent(self.__OnClickChangeMusicButton)

		self.cameraModeButtonList[0].SAFE_SetEvent(self.__OnClickCameraModeShortButton)
		self.cameraModeButtonList[1].SAFE_SetEvent(self.__OnClickCameraModeLongButton)

		if ENABLE_FOG_FIX:
			self.fogButtonList[0].SAFE_SetEvent(self.__OnClickFogModeOffButton)
			self.fogButtonList[1].SAFE_SetEvent(self.__OnClickFogModeOnButton)
		else:
			self.fogModeButtonList[0].SAFE_SetEvent(self.__OnClickFogModeLevel0Button)
			self.fogModeButtonList[1].SAFE_SetEvent(self.__OnClickFogModeLevel1Button)
			self.fogModeButtonList[2].SAFE_SetEvent(self.__OnClickFogModeLevel2Button)

		if not ENABLE_DISABLE_SOFTWARE_TILING:
			self.tilingModeButtonList[0].SAFE_SetEvent(self.__OnClickTilingModeCPUButton)
			self.tilingModeButtonList[1].SAFE_SetEvent(self.__OnClickTilingModeGPUButton)
			self.tilingApplyButton.SAFE_SetEvent(self.__OnClickTilingApplyButton)
			self.__SetCurTilingMode()

		if ENABLE_ENVIRONMENT_EFFECT_OPTION:			
			self.nightModeButtonList[0].SAFE_SetEvent(self.__OnClickNightModeOffButton)
			self.nightModeButtonList[1].SAFE_SetEvent(self.__OnClickNightModeOnButton)
			self.__InitNightModeOption()
			
			self.snowModeButtonList[0].SAFE_SetEvent(self.__OnClickSnowModeOffButton)
			self.snowModeButtonList[1].SAFE_SetEvent(self.__OnClickSnowModeOnButton)
			self.__InitSnowModeOption()
			
			self.snowTextureModeButtonList[0].SAFE_SetEvent(self.__OnClickSnowTextureModeOffButton)
			self.snowTextureModeButtonList[1].SAFE_SetEvent(self.__OnClickSnowTextureModeOnButton)		
			self.__InitSnowTextureModeOption()

		if ENABLE_GRAPHIC_ON_OFF:
			for i in xrange(GRAPHIC_LEVEL_MAX_NUM):
				self.effectLevelButtonList[i].SAFE_SetEvent(self.__OnClickEffectLevelButton, i)
				self.privateShopLevelButtonList[i].SAFE_SetEvent(self.__OnClickPrivateShopLevelButton, i)
				self.dropItemLevelButtonList[i].SAFE_SetEvent(self.__OnClickDropItemLevelButton, i)

			for i in xrange(2):
				self.petStatusButtonList[i].SAFE_SetEvent(self.__OnClickPetStatusButton, bool(not i))
				self.npcNameStatusButtonList[i].SAFE_SetEvent(self.__OnClickNpcNameStatusButton, bool(not i))

			self.effectLevelApplyButton.SAFE_SetEvent(self.__OnClickEffectApplyButton)
			self.privateShopLevelApplyButton.SAFE_SetEvent(self.__OnClickPrivateShopApplyButton)
			self.dropItemLevelApplyButton.SAFE_SetEvent(self.__OnClickDropItemApplyButton)

		if ENABLE_MULTI_LANGUAGE:
			Region = ""
			file = open("locale.cfg")
			lines = file.readlines()

			if len(lines) > 0:
				tokens = lines[0].split()
				Region = tokens[2]

			self.cur_language_text.SetText(constInfo.multiDict[int(Region)][2])

			self.multi_language_bar = ui.Bar()
			x, y = self.language_select_pivot_window.GetGlobalPosition()
			self.multi_language_bar.SetPosition(x, y)
			self.multi_language_bar.SetSize(210, 94)
			self.multi_language_bar.SetColor(0xFF000000)
			#self.multi_language_bar.AddFlag("float")
			self.multi_language_bar.Hide()

			self.multi_language_listbox = ui.ListBoxEx()
			self.multi_language_listbox.SetParent(self.multi_language_bar)
			self.multi_language_listbox.SetPosition(0, 0)
			self.multi_language_listbox.SetSize(210, 94)
			self.multi_language_listbox.SetViewItemCount(5)
			for i in xrange(16):
				self.multi_language_listbox.AppendItem(self.Item(constInfo.multiDict[constInfo.multiArray[i]][2]))

			self.multi_language_listbox.SetSelectEvent(ui.__mem_func__(self.__SelectMulti))
			self.multi_language_listbox.Show()

			self.multi_language_scrollbar = ui.ScrollBar()
			self.multi_language_scrollbar.SetParent(self.multi_language_listbox)
			self.multi_language_scrollbar.SetScrollBarSize(94)
			self.multi_language_scrollbar.SetPosition(195, 0)
			self.multi_language_scrollbar.Show()
			self.multi_language_listbox.SetScrollBar(self.multi_language_scrollbar)

			self.language_select_button.SAFE_SetEvent(self.OnLanguageSelect)

			self.language_change_button.SAFE_SetEvent(self.OnLanguageChange)

		if ENABLE_FOG_FIX:
			self.__ClickRadioButton(self.fogButtonList, systemSetting.GetFogMode())
			background.SetFogMode(systemSetting.GetFogMode())
		else:
			self.__ClickRadioButton(self.fogModeButtonList, constInfo.GET_FOG_LEVEL_INDEX())

		self.__ClickRadioButton(self.cameraModeButtonList, constInfo.GET_CAMERA_MAX_DISTANCE_INDEX())

		if ENABLE_GRAPHIC_ON_OFF:
			self.__ClickRadioButton(self.effectLevelButtonList, systemSetting.GetEffectLevel())
			self.__ClickRadioButton(self.privateShopLevelButtonList, systemSetting.GetPrivateShopLevel())
			self.__ClickRadioButton(self.dropItemLevelButtonList, systemSetting.GetDropItemLevel())

		if musicInfo.fieldMusic == musicInfo.METIN2THEMA:
			self.selectMusicFile.SetText(uiSelectMusic.DEFAULT_THEMA)
		else:
			self.selectMusicFile.SetText(musicInfo.fieldMusic[:MUSIC_FILENAME_MAX_LEN])

	if not ENABLE_DISABLE_SOFTWARE_TILING:
		def __OnClickTilingModeCPUButton(self):
			self.__NotifyChatLine(localeInfo.SYSTEM_OPTION_CPU_TILING_1)
			self.__NotifyChatLine(localeInfo.SYSTEM_OPTION_CPU_TILING_2)
			self.__NotifyChatLine(localeInfo.SYSTEM_OPTION_CPU_TILING_3)
			self.__SetTilingMode(0)

		def __OnClickTilingModeGPUButton(self):
			self.__NotifyChatLine(localeInfo.SYSTEM_OPTION_GPU_TILING_1)
			self.__NotifyChatLine(localeInfo.SYSTEM_OPTION_GPU_TILING_2)
			self.__NotifyChatLine(localeInfo.SYSTEM_OPTION_GPU_TILING_3)
			self.__SetTilingMode(1)

		def __OnClickTilingApplyButton(self):
			self.__NotifyChatLine(localeInfo.SYSTEM_OPTION_TILING_EXIT)
			if 0==self.tilingMode:
				background.EnableSoftwareTiling(1)
			else:
				background.EnableSoftwareTiling(0)

			net.ExitGame()

	def __OnClickChangeMusicButton(self):
		if not self.musicListDlg:

			self.musicListDlg=uiSelectMusic.FileListDialog()
			self.musicListDlg.SAFE_SetSelectEvent(self.__OnChangeMusic)

		self.musicListDlg.Open()


	def __ClickRadioButton(self, buttonList, buttonIndex):
		try:
			selButton=buttonList[buttonIndex]
		except IndexError:
			return

		for eachButton in buttonList:
			eachButton.SetUp()

		selButton.Down()

	if not ENABLE_DISABLE_SOFTWARE_TILING:
		def __SetTilingMode(self, index):
			self.__ClickRadioButton(self.tilingModeButtonList, index)
			self.tilingMode=index

	def __SetCameraMode(self, index):
		constInfo.SET_CAMERA_MAX_DISTANCE_INDEX(index)
		self.__ClickRadioButton(self.cameraModeButtonList, index)

	if not ENABLE_FOG_FIX:
		def __SetFogLevel(self, index):
			constInfo.SET_FOG_LEVEL_INDEX(index)
			self.__ClickRadioButton(self.fogModeButtonList, index)

	def __OnClickCameraModeShortButton(self):
		self.__SetCameraMode(0)

	def __OnClickCameraModeLongButton(self):
		self.__SetCameraMode(1)

	if ENABLE_FOG_FIX:
		def __OnClickFogModeOnButton(self):
			background.SetFogMode(True)
			systemSetting.SetFogMode(True)
			self.__ClickRadioButton(self.fogButtonList, 1)
		def __OnClickFogModeOffButton(self):
			background.SetFogMode(False)
			systemSetting.SetFogMode(False)
			self.__ClickRadioButton(self.fogButtonList, 0)
	else:
		def __OnClickFogModeLevel0Button(self):
			self.__SetFogLevel(0)

		def __OnClickFogModeLevel1Button(self):
			self.__SetFogLevel(1)

		def __OnClickFogModeLevel2Button(self):
			self.__SetFogLevel(2)


	def __OnChangeMusic(self, fileName):
		self.selectMusicFile.SetText(fileName[:MUSIC_FILENAME_MAX_LEN])

		if musicInfo.fieldMusic != "":
			snd.FadeOutMusic("BGM/"+ musicInfo.fieldMusic)

		if fileName==uiSelectMusic.DEFAULT_THEMA:
			musicInfo.fieldMusic=musicInfo.METIN2THEMA
		else:
			musicInfo.fieldMusic=fileName

		musicInfo.SaveLastPlayFieldMusic()

		if musicInfo.fieldMusic != "":
			snd.FadeInMusic("BGM/" + musicInfo.fieldMusic)

	def OnChangeMusicVolume(self):
		pos = self.ctrlMusicVolume.GetSliderPos()
		snd.SetMusicVolume(pos * net.GetFieldMusicVolume())
		systemSetting.SetMusicVolume(pos)

	def OnChangeSoundVolume(self):
		pos = self.ctrlSoundVolume.GetSliderPos()
		snd.SetSoundVolumef(pos)
		systemSetting.SetSoundVolumef(pos)

	def OnChangeShadowQuality(self):
		pos = self.ctrlShadowQuality.GetSliderPos()
		systemSetting.SetShadowLevel(int(pos / 0.2))

	def OnCloseInputDialog(self):
		self.inputDialog.Close()
		self.inputDialog = None
		return True

	def OnCloseQuestionDialog(self):
		self.questionDialog.Close()
		self.questionDialog = None
		return True

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def Show(self):
		ui.ScriptWindow.Show(self)

	def Close(self):
		if not ENABLE_DISABLE_SOFTWARE_TILING:
			self.__SetCurTilingMode()
		self.Hide()
		if ENABLE_MULTI_LANGUAGE:
			self.multi_language_bar.Hide()

	if not ENABLE_DISABLE_SOFTWARE_TILING:
		def __SetCurTilingMode(self):
			if background.IsSoftwareTiling():
				self.__SetTilingMode(0)
			else:
				self.__SetTilingMode(1)

	def __NotifyChatLine(self, text):
		chat.AppendChat(chat.CHAT_TYPE_INFO, text)

	def Refresh(self):
		if ENABLE_GRAPHIC_ON_OFF:
			self.RefreshPetStatus()
			self.RefreshNpcNameStatus()

	def OnMoveWindow(self, x, y):
		if ENABLE_MULTI_LANGUAGE:
			 if self.multi_language_bar:
				self.BarAdjustPositionAndSize()

	if ENABLE_ENVIRONMENT_EFFECT_OPTION:
		def __InitNightModeOption(self):
			self.NightModeOn = systemSetting.GetNightModeOption()
			self.__ClickRadioButton(self.nightModeButtonList, self.NightModeOn)
			self.__SetNightMode(self.NightModeOn)
			
		def __InitSnowModeOption(self):
			self.SnowModeOn	= systemSetting.GetSnowModeOption()
			self.__ClickRadioButton(self.snowModeButtonList, self.SnowModeOn)
			self.__SetSnowMode(self.SnowModeOn)
	
		def __InitSnowTextureModeOption(self):
			self.SnowTextureModeOn = systemSetting.GetSnowTextureModeOption()
			self.__ClickRadioButton(self.snowTextureModeButtonList, self.SnowTextureModeOn)
			self.__SetSnowTextureMode(self.SnowTextureModeOn)
				
		def __OnClickNightModeOffButton(self):
			self.__ClickRadioButton(self.nightModeButtonList, 0)
			self.__SetNightMode(0)
			
		def __OnClickNightModeOnButton(self):
			self.__ClickRadioButton(self.nightModeButtonList, 1)
			self.__SetNightMode(1)
			
		def __OnClickSnowModeOffButton(self):
			self.__ClickRadioButton(self.snowModeButtonList, 0)
			self.__SetSnowMode(0)
			
		def __OnClickSnowModeOnButton(self):
			self.__ClickRadioButton(self.snowModeButtonList, 1)
			self.__SetSnowMode(1)
			
		def __OnClickSnowTextureModeOffButton(self):
			self.__ClickRadioButton(self.snowTextureModeButtonList, 0)
			self.__SetSnowTextureMode(0)
			
		def __OnClickSnowTextureModeOnButton(self):
			self.__ClickRadioButton(self.snowTextureModeButtonList, 1)
			self.__SetSnowTextureMode(1)

		def __SetSnowMode(self, index):
			systemSetting.SetSnowModeOption(index)
			background.EnableSnow(index)

		def __SetSnowTextureMode(self, index):
			systemSetting.SetSnowTextureModeOption(index)
			# background.EnableSnowTextureMode()

		def __SetNightMode(self, index):
			systemSetting.SetNightModeOption(index)	
			if index:
				if self.curtain:
					self.curtain.SAFE_FadeOut(self.__DayMode_OnCompleteChangeToDark)
				else:
					self.__DayMode_OnCompleteChangeToDark()
			else:
				if self.curtain:
					self.curtain.SAFE_FadeOut(self.__DayMode_OnCompleteChangeToLight)
				else:
					self.__DayMode_OnCompleteChangeToLight()

		def __DayMode_OnCompleteChangeToLight(self):
			background.SetEnvironmentData(0)
			if self.curtain:
				self.curtain.FadeIn()

		def __DayMode_OnCompleteChangeToDark(self):		
			background.RegisterEnvironmentData(1, constInfo.ENVIRONMENT_NIGHT)
			background.SetEnvironmentData(1)
			if self.curtain:
				self.curtain.FadeIn()

	if ENABLE_GRAPHIC_ON_OFF:
		def __OnClickEffectLevelButton(self, index):
			self.__ClickRadioButton(self.effectLevelButtonList, index)
			self.effectLevel=index

		def __OnClickEffectApplyButton(self):
			systemSetting.SetEffectLevel(self.effectLevel)

		def __OnClickPrivateShopLevelButton(self, index):
			self.__ClickRadioButton(self.privateShopLevelButtonList, index)
			self.privateShopLevel=index

		def __OnClickPrivateShopApplyButton(self):
			systemSetting.SetPrivateShopLevel(self.privateShopLevel)

		def __OnClickDropItemLevelButton(self, index):
			self.__ClickRadioButton(self.dropItemLevelButtonList, index)
			self.dropItemLevel=index

		def __OnClickDropItemApplyButton(self):
			systemSetting.SetDropItemLevel(self.dropItemLevel)

		def __OnClickPetStatusButton(self, flag):
			systemSetting.SetPetStatusFlag(flag)
			self.RefreshPetStatus()

		def RefreshPetStatus(self):
			if systemSetting.IsPetStatus():
				self.petStatusButtonList[0].SetUp()
				self.petStatusButtonList[1].Down()
			else:
				self.petStatusButtonList[0].Down()
				self.petStatusButtonList[1].SetUp()

		def __OnClickNpcNameStatusButton(self, flag):
			systemSetting.SetNpcNameStatusFlag(flag)
			self.RefreshNpcNameStatus()

		def RefreshNpcNameStatus(self):
			if systemSetting.IsNpcNameStatus():
				self.npcNameStatusButtonList[0].SetUp()
				self.npcNameStatusButtonList[1].Down()
			else:
				self.npcNameStatusButtonList[0].Down()
				self.npcNameStatusButtonList[1].SetUp()

	if ENABLE_MULTI_LANGUAGE:
		def BarAdjustPositionAndSize(self):
			x, y = self.language_select_pivot_window.GetGlobalPosition()
			self.multi_language_bar.SetPosition(x, y)

		def __SelectMulti(self):
			self.cur_language_text.SetText(self.multi_language_listbox.GetSelectedItem().GetText())
			self.multi_language_bar.Hide()

		def OnLanguageChange(self):
			selItem = self.multi_language_listbox.GetSelectedItem()
			arr = constInfo.multiDict[constInfo.multiArray[self.multi_language_listbox.GetItemIndex(selItem)]]
			file = open("locale.cfg", "w")
			file.write("%d %s %d" % (arr[0], arr[1], self.multi_language_listbox.GetItemIndex(selItem)))
			file.close()
			app.Exit()
			import os
			os.system('start Astra2.bin')

		def OnLanguageSelect(self):
			if self.multi_language_bar.IsShow():
				self.multi_language_bar.Hide()
			else:
				self.multi_language_bar.Show()

