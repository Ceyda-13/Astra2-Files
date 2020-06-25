import ui
import app
import grp
import time
import chat
import item
import systemSetting
import playerSettingModule
import dbg
import __builtin__ as selfs
FACE_IMAGE_DICT = {
	playerSettingModule.RACE_WARRIOR_M	: "icon/face/warrior_m.tga",
	playerSettingModule.RACE_WARRIOR_W	: "icon/face/warrior_w.tga",
	playerSettingModule.RACE_ASSASSIN_M	: "icon/face/assassin_m.tga",
	playerSettingModule.RACE_ASSASSIN_W	: "icon/face/assassin_w.tga",
	playerSettingModule.RACE_SURA_M		: "icon/face/sura_m.tga",
	playerSettingModule.RACE_SURA_W		: "icon/face/sura_w.tga",
	playerSettingModule.RACE_SHAMAN_M	: "icon/face/shaman_m.tga",
	playerSettingModule.RACE_SHAMAN_W	: "icon/face/shaman_w.tga",
}

DROP_MESSAGE_TIME = 4
DROP_MAX_STACK = 13

class Gui(ui.Window):
	TAB_TEXTS = [
		"Oyuncu Ýsmi",
		"Ölme Sayýsý",
		"Öldürme Sayýsý",
	]
	TAB_TEXTS_MAX = len(TAB_TEXTS)

	def __init__(self):
		ui.Window.__init__(self)
		self.__Initialize()
		self.__LoadWindow()

	def __del__(self):
		ui.Window.__del__(self)	

	def __Initialize(self):
		self.Board = None
		self.Bar = None
		self.TabTexts = []
		self.TextsDict = {}
		self.PlayerCount = 0

	def __LoadWindow(self):
		self.Board = ui.Bar()
		self.Board.SetSize(465, 280)
		self.Board.SetCenterPosition()
		self.Board.AddFlag('movable')
		self.Board.AddFlag('float')
		self.Board.SetColor(0x77000000)

		self.titleBar = ui.TitleBar()
		self.titleBar.SetParent(self.Board)
		self.titleBar.MakeTitleBar(465, "red")
		self.titleBar.SetPosition(0, 1.5)
		self.titleBar.Show()

		self.titleName = ui.TextLine()
		self.titleName.SetParent(self.titleBar)
		self.titleName.SetPosition(0, 4.5)
		self.titleName.SetWindowHorizontalAlignCenter()
		self.titleName.SetHorizontalAlignCenter()
		self.titleName.SetText("Lonca Savaþý Skor Tablosu")
		self.titleName.Show()

		self.Bar = ui.ExpandedImageBox()
		self.Bar.SetParent(self.Board)
		self.Bar.LoadImage("guild/bar.tga")
		self.Bar.SetPosition(1, 20)
		self.Bar.SetScale(CalcScale(404, 59, True), 1.0)
		self.Bar.Show()

		self.Bar2 = ui.Line()
		self.Bar2.SetParent(self.Board)
		self.Bar2.SetSize(465, 0)
		self.Bar2.SetPosition(-1, 0)
		self.Bar2.Show()

		self.Bar3 = ui.Line()
		self.Bar3.SetParent(self.Board)
		self.Bar3.SetSize(0, 280)
		self.Bar3.SetPosition(0, 0)
		self.Bar3.Show()

		self.Bar4 = ui.Line()
		self.Bar4.SetParent(self.Board)
		self.Bar4.SetSize(465, 0)
		self.Bar4.SetPosition(0, 280)
		self.Bar4.Show()

		self.Bar5 = ui.Line()
		self.Bar5.SetParent(self.Board)
		self.Bar5.SetSize(0, 280)
		self.Bar5.SetPosition(465, 0)
		self.Bar5.Show()

		# self.Bar6 = ui.Line()
		# self.Bar6.SetParent(self.Board)
		# self.Bar6.SetSize(0, 245)
		# self.Bar6.SetPosition(150, 40)
		# self.Bar6.Show()

		# self.Bar7 = ui.Line()
		# self.Bar7.SetParent(self.Board)
		# self.Bar7.SetSize(0, 245)
		# self.Bar7.SetPosition(300, 40)
		# self.Bar7.Show()

		for i in xrange(self.TAB_TEXTS_MAX):
			text = ui.TextLine()
			text.SetParent(self.Bar)
			text.SetText(self.TAB_TEXTS[i])
			text.SetPosition(10 + i * 190, 5)
			text.Show()
			self.TabTexts.append(text)

		self.PlayerCount += 1

	def Open(self):
		if self.Board.IsShow():
			self.Board.Hide()
		else:
			self.Board.Show()

	def Close(self):
		self.Board.Hide()

	def OnPressEscapeKey(self):
		self.Hide()
		return True	

	def OnUpdatePlayer(self, killer_name, killer_race, victim_name, victim_race):
		if not self.TextsDict.has_key(killer_name):
			texts = []
			for i in xrange(self.TAB_TEXTS_MAX):
				text = ui.TextLine()
				text.SetParent(self.Bar)
				if i == 0:
					text.SetText(killer_name)
				elif i == 1:
					text.SetText("0")
				elif i == 2:
					text.SetText("1")
				text.SetPosition(10 + i * 210, self.PlayerCount * 20 + 5)
				text.Show()
				texts.append(text)

			self.TextsDict.update({
				killer_name : {
					"race" : killer_race,
					"point" : 1,
					"point2" : 0,
					"text" : texts,
				},
			})
			# face = ui.ExpandedImageBox()
			# face.SetParent(self.Bar)
			global FACE_IMAGE_DICT
			# face.LoadImage(FACE_IMAGE_DICT[killer_race])
			# face.SetPosition(100, self.PlayerCount * 20 + 5)
			# face.SetScale(0.4, 0.4)
			# face.Show()
			# texts.append(face)
			self.PlayerCount += 1
		else:
			self.TextsDict[killer_name]["point"] += 1
			a = int(self.TextsDict[killer_name]["text"][2].GetText())
			self.TextsDict[killer_name]["text"][2].SetText("%s" % str(a + 1))

		if not self.TextsDict.has_key(victim_name):
			texts = []
			for i in xrange(self.TAB_TEXTS_MAX):
				text = ui.TextLine()
				text.SetParent(self.Bar)
				if i == 0:
					text.SetText(victim_name)
				elif i == 1:
					text.SetText("1")
				elif i == 2:
					text.SetText("0")
				text.SetPosition(10 + i * 210, self.PlayerCount * 20 + 5)
				text.Show()
				texts.append(text)
			# face = ui.ExpandedImageBox()
			# face.SetParent(self.Bar)
			# global FACE_IMAGE_DICT
			# face.LoadImage(FACE_IMAGE_DICT[victim_race])
			# face.SetPosition(100, self.PlayerCount * 20 + 5)
			# face.SetScale(0.4, 0.4)
			# face.Show()
			# texts.append(face)
			self.TextsDict.update({
				victim_name : {
					"race" : victim_race,
					"point" : 0,
					"point2" : 1,
					"text" : texts,
				},
			})
			self.PlayerCount += 1
		else:
			self.TextsDict[victim_name]["point2"] += 1
			a = int(self.TextsDict[victim_name]["text"][1].GetText())
			self.TextsDict[victim_name]["text"][1].SetText("%s" % str(a + 1))

class MessageObject(ui.Bar):
	def __init__(self):
		ui.Bar.__init__(self)

		# self.__killWeapon = None

		self.__killerName = None
		self.__killerRace = None

		self.__victimName = None
		self.__victimRace = None

		self.endTime = time.clock() + DROP_MESSAGE_TIME

		self.SetSize(250, 25)
		self.SetColor(0x77000000)

	def __del__(self):
		ui.Bar.__del__(self)


	# def SetKillWeapon(self, weaponVnum):
		# item.SelectItem(weaponVnum)

		# self.__killWeapon = ui.ExpandedImageBox()
		# self.__killWeapon.SetParent(self)		
		# try:
			# self.__killWeapon.LoadImage(item.GetIconImageFileName())
		# except:
			# self.__killWeapon.Hide()
		# self.__killWeapon.SetPosition(110, -5)
		# self.__killWeapon.SetScale(0.6, 0.6)

	def SetKillerInfo(self, killerName, killerRace):
		self.__killerName = ui.TextLine()
		self.__killerName.SetParent(self)
		self.__killerName.SetPosition(25, 3)
		self.__killerName.SetText(killerName)
		self.__killerName.SetOutline()
		self.__killerName.SetFeather(False)
		self.__killerName.SetPackedFontColor(0xff00e5ee)

		self.__killerRace = ui.ExpandedImageBox()
		self.__killerRace.SetParent(self)		
		try:
			self.__killerRace.LoadImage(FACE_IMAGE_DICT[int(killerRace)])
		except:
			self.__killerRace.Hide()
		self.__killerRace.SetPosition(0, 5)
		self.__killerRace.SetScale(0.4, 0.4)
		
	def SetVictimInfo(self, victimName, victimRace):
		self.__victimName = ui.TextLine()
		self.__victimName.SetParent(self)
		self.__victimName.SetPosition(155, 3)
		self.__victimName.SetText(victimName)
		self.__victimName.SetOutline()
		self.__victimName.SetFeather(False)
		self.__victimName.SetPackedFontColor(0xffff0000)


		self.__victimRace = ui.ExpandedImageBox()
		self.__victimRace.SetParent(self)		
		try:
			self.__victimRace.LoadImage(FACE_IMAGE_DICT[int(victimRace)])
		except:
			self.__victimRace.Hide()
		self.__victimRace.SetPosition(135, 5)
		self.__victimRace.SetScale(0.4, 0.4)

	def ShowInfos(self):
		self.__killerName.Show()
		self.__killerRace.Show()

		self.__victimName.Show()
		self.__victimRace.Show()

		# self.__killWeapon.Show()

	def isTimeout(self):
		lastTime = max(0, self.endTime - time.clock())
		if 0 == lastTime:
			return True
		return False

class MessageQueue(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)

		self.stack = []

		self.lastClock = 0
		self.timeDiff = 0.5
		self.nextY = 0

		self.__Reset()

	def __del__(self):
		ui.Window.__del__(self)

		self.stack = None

	def OnMessage(self, killerName, killerRace, victimName, victimRace, killWeaponVnum):
		message = MessageObject()

		message.SetParent(self)
		# if killWeaponVnum:
			# message.SetKillWeapon(killWeaponVnum)
		message.SetKillerInfo(killerName, killerRace)
		message.SetVictimInfo(victimName, victimRace)

		message.Hide()

		count = len(self.stack)

		if count == DROP_MAX_STACK:
			self.stack.remove(self.stack[0])

		self.stack.append(message)
		self.__Render()

	def __Reset(self):
		#self.SetSize(250, 426)
		self.SetPosition(systemSetting.GetWidth()- 250, -200)

		self.Show()

	def __Render(self):
		for it in self.stack:
			if it.isTimeout():
				self.stack.remove(it)

		stack = list(self.stack)
		stack.reverse()

		self.nextY = 408

		for it in stack:
			it.SetPosition(0, self.nextY)

			if not it.IsShow():
				it.Show()
				it.ShowInfos()

			self.nextY += 27

	def OnUpdate(self):
		if len(self.stack) > 0:
			if (app.GetTime() - self.lastClock) >= self.timeDiff:
				self.lastClock = app.GetTime()
				self.__Render()

class WindowTestUI(ui.BoardWithTitleBar):
	def __init__(self):
		ui.BoardWithTitleBar.__init__(self)

		self.wnd = MessageQueue()
		self.BuildUI()
	
	def __del__(self):
		ui.BoardWithTitleBar.__del__(self)
	
	def BuildUI(self):
		self.AddFlag('movable')
		self.AddFlag('float')
		self.SetSize(50, 50)
		self.SetTitleName("")
		self.SetCloseEvent(self.Close)
		self.SetCenterPosition()
		self.Show()

		self.CikartButton = ui.Button()
		self.CikartButton.SetParent(self)
		self.CikartButton.SetPosition(8, 28)
		self.CikartButton.SetUpVisual('d:/ymir work/ui/public/xsmall_button_01.sub')
		self.CikartButton.SetOverVisual('d:/ymir work/ui/public/xsmall_button_02.sub')
		self.CikartButton.SetDownVisual('d:/ymir work/ui/public/xsmall_button_03.sub')
		self.CikartButton.SetText("")
		self.CikartButton.SetEvent(self.Load)
		self.CikartButton.Show()

	def Load(self):
		self.wnd.OnMessage("KILLER_NAME_12", 2, "VICTIM_NAME_12", 4)
		selfs.wndGuildWarScore.OnUpdatePlayer("KILLER_NAME_12", 2, "VICTIM_NAME_12", 4)

	def ShowWindow(self):
		self.Show()
	
	def Close(self):
		self.Hide()
		return True
	
	def OnPressEscapeKey(self):
		self.Close()
		return True

# WindowTestUI()