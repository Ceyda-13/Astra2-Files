import ui
import uiScriptLocale
import wndMgr
import player
import localeInfo
import net
import app

if ENABLE_MINI_GAME_RUMI:
	import uiMiniGameRumi
	MINIGAME_TYPE_RUMI = player.MINIGAME_TYPE_RUMI
	RUMI_ROOT = "d:/ymir work/ui/minigame/rumi/"

if ENABLE_MONSTER_BACK or ENABLE_CARNIVAL2016:
	import uiMiniGameAttendance
	MINIGAME_ATTENDANCE = player.MINIGAME_ATTENDANCE

if ENABLE_MONSTER_BACK:
	MINIGAME_MONSTERBACK = player.MINIGAME_MONSTERBACK

if ENABLE_FISH_EVENT:
	import uiMiniGameFishEvent
	MINIGAME_FISH = player.MINIGAME_FISH

if ENABLE_MINI_GAME_CATCH_KING:
	import uiMiniGameCatchKing
	MINIGAME_CATCHKING = player.MINIGAME_CATCHKING

MINIGAME_TYPE_MAX = player.MINIGAME_TYPE_MAX

button_gap = 10
button_height = 25
	
class MiniGameDialog(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.isLoaded = 0

		self.board = None
		self.close_button = None

		self.button_dict = {}

		self.__LoadWindow()

	def __del__(self):
		ui.ScriptWindow.__del__(self)
		self.Destroy()

	def Destroy(self):
		self.isLoaded = 0

		self.board = None
		self.close_button = None

		self.button_dict = {}

	def Show(self):
		self.__LoadWindow()
		ui.ScriptWindow.Show(self)

	def Close(self):
		self.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def __LoadWindow(self):
		if self.isLoaded == 1:
			return

		self.isLoaded = 1
			
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "UIScript/MiniGameDialog.py")

			self.board = self.GetChild("board")
			self.close_button = self.GetChild("close_button")
			self.close_button.SetEvent(ui.__mem_func__(self.Close))

		except:
			import exception
			exception.Abort("MiniGameDialog.LoadWindow.BindObject")

		self.Hide()

	def AppendButton(self, type, name, func):
		if self.button_dict.has_key(type):
			return

		button = ui.Button()
		button.SetParent(self.board)
		button_count = len(self.button_dict)
		pos_y = (button_gap * (button_count + 1)) + button_count * button_height
		button.SetPosition(10, pos_y)
		button.SetUpVisual("d:/ymir work/ui/public/XLarge_Button_01.sub")
		button.SetOverVisual("d:/ymir work/ui/public/XLarge_Button_02.sub")
		button.SetDownVisual("d:/ymir work/ui/public/XLarge_Button_03.sub")

		if name:
			button.SetText(name)

		if func: 
			button.SetEvent(ui.__mem_func__(func))

		button.Show()
		self.button_dict[type] = button

	def DeleteButton(self, type):
		if not self.button_dict.has_key(type):
			return

		self.button_dict[type].Hide()
		del self.button_dict[type]

	def DeleteAllButton(self):
		for button in self.button_dict.values():
			button.Hide()
			del button

		self.button_dict.clear()

	def RefreshDialog(self):
		total_len = len(self.button_dict) + 1
		board_height = (button_height * total_len) + (button_gap * (total_len + 1))
		self.board.SetSize(200, board_height)
		self.SetSize(200, board_height)

		dict_len = len(self.button_dict)
		pos_y = (button_gap * (dict_len + 1)) + dict_len * button_height

		if localeInfo.IsARABIC():
			(lx,ly) = self.close_button.GetLocalPosition()
			self.close_button.SetPosition( lx, pos_y )
		else:
			self.close_button.SetPosition( 10, pos_y )

class MiniGameWindow(ui.ScriptWindow):
	def __init__(self):
		self.isLoaded = 0
		self.main_game = None
		if ENABLE_MINI_GAME_RUMI:
			self.rumi_game = None
		if ENABLE_MONSTER_BACK or ENABLE_CARNIVAL2016:
			self.attendance_game = None
		if ENABLE_FISH_EVENT:
			self.fish_game = None
		if ENABLE_MINI_GAME_CATCH_KING:
			self.catchking_game = None

		self.mini_game_dialog = None
		self.isshow_mini_game_dialog = False

		self.game_type = MINIGAME_TYPE_MAX

		ui.ScriptWindow.__init__(self)

		self.__LoadWindow()
		
	def __del__(self):
		ui.ScriptWindow.__del__(self)
		
	def Show(self):
		self.__LoadWindow()
		ui.ScriptWindow.Show(self)
		if self.mini_game_dialog and self.isshow_mini_game_dialog:
			self.mini_game_dialog.Show()

	def Close(self):
		self.Hide()

	def Hide(self):
		if self.mini_game_dialog:
			self.isshow_mini_game_dialog = self.mini_game_dialog.IsShow()
			self.mini_game_dialog.Hide()

		wndMgr.Hide(self.hWnd)

	def Destroy(self):
		self.isLoaded = 0

		self.main_game = None		

		if ENABLE_MINI_GAME_RUMI:
			if self.rumi_game:
				self.rumi_game.Destroy()
				self.rumi_game = None

		if ENABLE_MONSTER_BACK or ENABLE_CARNIVAL2016:
			if self.attendance_game:
				self.attendance_game.Destroy()
				self.attendance_game = None

		if ENABLE_FISH_EVENT:
			if self.fish_game:
				self.fish_game.Destroy()
				self.fish_game = None

		if ENABLE_MINI_GAME_CATCH_KING:
			if self.catchking_game:
				self.catchking_game.Destroy()
				self.catchking_game = None

		if self.mini_game_dialog:
			self.mini_game_dialog.Destroy()
			self.mini_game_dialog = None

		self.game_type = MINIGAME_TYPE_MAX

	def __LoadWindow(self):
		if self.isLoaded == 1:
			return

		self.isLoaded = 1

		try:
			self.__LoadScript("UIScript/MiniGameWindow.py")
		except:
			import exception
			exception.Abort("MiniGameWindow.LoadWindow.LoadObject")

		try:
			mini_game_window = self.GetChild("mini_game_window")
			self.event_banner_button = ui.Button()
			self.event_banner_button.SetParent(mini_game_window)
			self.event_banner_button.SetPosition(0, 0)
			self.event_banner_button.SetUpVisual("d:/ymir work/ui/minigame/banner.sub")
			self.event_banner_button.SetOverVisual("d:/ymir work/ui/minigame/banner.sub")
			self.event_banner_button.SetDownVisual("d:/ymir work/ui/minigame/banner.sub")
			self.event_banner_button.SetEvent(ui.__mem_func__(self.ClickIntegrationEventBannerButton))
			self.event_banner_button.Hide()
			self.event_banner_button_enable = False
		except:
			import exception
			exception.Abort("MiniGameWindow.LoadWindow.EventBannerButton.BindObject")

		try:
			self.mini_game_dialog = MiniGameDialog()
			self.mini_game_dialog.Hide()
		except:
			import exception
			exception.Abort("MiniGameWindow.LoadWindow.MiniGameDialog")

		self.Show()

	def __LoadScript(self, fileName):
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, fileName)			

	def ClickIntegrationEventBannerButton(self):
		if not self.mini_game_dialog:
			return

		if self.mini_game_dialog.IsShow():
			self.mini_game_dialog.Close()
		else:			
			self.mini_game_dialog.Show()

	def IntegrationMiniGame(self, enable):
		if enable:
			self.event_banner_button.Show()	
			self.event_banner_button_enable = True
		else:
			self.event_banner_button.Hide()
			self.event_banner_button_enable = False

		if ENABLE_MINI_GAME_RUMI:
			if self.rumi_game:
				self.rumi_game.Destroy()
				self.rumi_game = None

		if ENABLE_MONSTER_BACK or ENABLE_CARNIVAL2016:
			if self.attendance_game:
				self.attendance_game.Destroy()
				self.attendance_game = None

		if ENABLE_FISH_EVENT:
			if self.fish_game:
				self.fish_game.Destroy()
				self.fish_game = None

		if ENABLE_MINI_GAME_CATCH_KING:
			if self.catchking_game:
				self.catchking_game.Destroy()
				self.catchking_game = None

		if self.mini_game_dialog:
			self.mini_game_dialog.DeleteAllButton()

			if False == enable:
				self.mini_game_dialog.Hide()

			self.mini_game_dialog.RefreshDialog()

			self.game_type = MINIGAME_TYPE_MAX
			self.main_game = None

	def __CloseAll(self, except_game = MINIGAME_TYPE_MAX):
		if ENABLE_MINI_GAME_RUMI:
			if self.rumi_game and except_game != MINIGAME_TYPE_RUMI:
				self.rumi_game.Close()

		if ENABLE_MONSTER_BACK or ENABLE_CARNIVAL2016:
			if self.attendance_game and except_game != MINIGAME_ATTENDANCE:
				self.attendance_game.Close()

		if ENABLE_FISH_EVENT:
			if self.fish_game and except_game != MINIGAME_FISH:
				self.fish_game.Close()

		if ENABLE_MINI_GAME_CATCH_KING:
			if self.catchking_game and except_game != MINIGAME_CATCHKING:
				self.catchking_game.Close()

	def hide_mini_game_dialog(self):
		if self.event_banner_button:
			if self.event_banner_button.IsShow():
				self.event_banner_button.Hide()

		if self.mini_game_dialog:
			if self.mini_game_dialog.IsShow():
				self.mini_game_dialog.Hide()

	def show_mini_game_dialog(self):
		if self.event_banner_button:
			if self.event_banner_button_enable:
				self.event_banner_button.Show()		

	def SetEvent(self, Type, IsOpen):
		if ENABLE_MINI_GAME_RUMI and Type == MINIGAME_TYPE_RUMI:
			if IsOpen:
				self.mini_game_dialog.AppendButton(MINIGAME_TYPE_RUMI, uiScriptLocale.BANNER_OKEY_BUTTON, self.__ClickRumiButton)
			else:
				self.mini_game_dialog.DeleteButton(MINIGAME_TYPE_RUMI)

		if (ENABLE_MONSTER_BACK or ENABLE_CARNIVAL2016) and Type == MINIGAME_ATTENDANCE:
			if IsOpen:
				self.mini_game_dialog.AppendButton(MINIGAME_ATTENDANCE, uiScriptLocale.BANNER_ATTENDANCE_BUTTON, self.__ClickAttendanceButton)
			else:
				self.mini_game_dialog.DeleteButton(MINIGAME_ATTENDANCE)

		if ENABLE_MONSTER_BACK and ENABLE_ACCUMULATE_DAMAGE_DISPLAY and Type == MINIGAME_MONSTERBACK:
			if IsOpen:
				self.mini_game_dialog.AppendButton(MINIGAME_MONSTERBACK, uiScriptLocale.BANNER_MONSTERBACK_BUTTON, self.__ClickMonsterBackButton)
			else:
				self.mini_game_dialog.DeleteButton(MINIGAME_MONSTERBACK)

		if ENABLE_FISH_EVENT:
			if IsOpen:
				self.mini_game_dialog.AppendButton(MINIGAME_FISH, uiScriptLocale.BANNER_FISH_BUTTON, self.__ClickFishEventButton)
			else:
				self.mini_game_dialog.DeleteButton(MINIGAME_FISH)

		if ENABLE_MINI_GAME_CATCH_KING:
			if IsOpen:
				self.mini_game_dialog.AppendButton(MINIGAME_CATCHKING, uiScriptLocale.BANNER_CATCHKING_BUTTON, self.__ClickCatchKingButton)
			else:
				self.mini_game_dialog.DeleteButton(MINIGAME_CATCHKING)

		self.mini_game_dialog.RefreshDialog()

	if ENABLE_MINI_GAME_OKEY_NORMAL:
		def SetOkeyNormalBG(self):
			if not self.rumi_game:
				return
				
			self.rumi_game.SetOkeyNormalBG()

	if ENABLE_MINI_GAME_RUMI:
		def __ClickRumiButton(self):
			self.__CloseAll(MINIGAME_TYPE_RUMI)

			if self.mini_game_dialog:
				self.mini_game_dialog.Close()

			if not self.rumi_game:
				self.rumi_game = uiMiniGameRumi.MiniGameRumi()
			
			self.main_game = self.rumi_game
			self.game_type = MINIGAME_TYPE_RUMI
			self.main_game.Open()

		def MiniGameRumiStart(self):
			if not self.main_game:
				return

			self.main_game.GameStart()

		def MiniGameRumiSetDeckCount(self, deck_card_count):
			if MINIGAME_TYPE_RUMI != self.game_type:
				return
				
			self.main_game.SetDeckCount(deck_card_count)

		def MiniGameRumiMoveCard(self, srcCard, dstCard):
			if MINIGAME_TYPE_RUMI != self.game_type:
				return
				
			self.main_game.MoveCard(srcCard, dstCard)

		def MiniGameRumiIncreaseScore(self, score, total_score):
			if MINIGAME_TYPE_RUMI != self.game_type:
				return
				
			self.main_game.IncreaseScore(score, total_score)

		def	MiniGameRumiEnd(self):
			if not self.main_game:
				return

			self.main_game.GameEnd()

	if ENABLE_MONSTER_BACK or ENABLE_CARNIVAL2016:
		def MiniGameAttendance(self, enable):
			if enable:
				if self.attendance_button:
					self.attendance_button.Show()
			else:
				if self.attendance_button:
					self.attendance_button.Hide()
				
			if self.attendance_game:
				self.attendance_game.Destroy()
				self.attendance_game = None
				
			if self.game_type == MINIGAME_ATTENDANCE:
				self.main_game = None
			
		def __ClickAttendanceButton(self):
			self.__CloseAll(MINIGAME_ATTENDANCE)

			if self.mini_game_dialog:
				self.mini_game_dialog.Close()
		
			if not self.attendance_game:
				self.attendance_game = uiMiniGameAttendance.MiniGameAttendance()

			self.main_game = self.attendance_game
			self.game_type = MINIGAME_ATTENDANCE
			self.main_game.Open()
			
		def MiniGameAttendanceSetData(self, type, value):
			if MINIGAME_ATTENDANCE != self.game_type:
				return
				
			if not self.main_game:
				return

			if type == player.ATTENDANCE_DATA_TYPE_DAY:
				self.main_game.MiniGameAttendanceSetDay(value)
			elif type == player.ATTENDANCE_DATA_TYPE_MISSION_CLEAR:
				self.main_game.MiniGameAttendanceSetMissionClear(value)
			elif type == player.ATTENDANCE_DATA_TYPE_GET_REWARD:
				self.main_game.MiniGameAttendanceSetReward(value)
			elif type == player.ATTENDANCE_DATA_TYPE_SHOW_MAX:
				self.main_game.MiniGameAttendanceSetShowMax(value)

			self.main_game.RefreshAttendanceBoard()

		def MiniGameAttendanceRequestRewardList(self):
			if MINIGAME_ATTENDANCE != self.game_type:
				return
				
			if not self.main_game:
				return
				
			self.main_game.MiniGameAttendanceRequestRewardList()

	if ENABLE_MONSTER_BACK:
		def __ClickMonsterBackButton(self):
			self.__CloseAll()
			if self.mini_game_dialog:
				self.mini_game_dialog.Close()
				
			net.SendRequestEventQuest("e_monsterback")
			self.main_game = None
			self.game_type = MINIGAME_TYPE_MAX

	if ENABLE_FISH_EVENT:
		def CantFishEventSlot(self, InvenSlot):
			ItemVnum = player.GetItemIndex(InvenSlot)
			if ItemVnum in [25106, 25107]:
				return False

			return True

		def __ClickFishEventButton(self):
			self.__CloseAll(MINIGAME_FISH)
				
			if self.mini_game_dialog:
				self.mini_game_dialog.Close()

			if not self.fish_game:
				self.fish_game = uiMiniGameFishEvent.MiniGameFish()

			self.main_game = self.fish_game
			self.game_type = MINIGAME_FISH
			self.main_game.Open()

		def MiniGameFishUse(self, window, shape):			
			if MINIGAME_FISH != self.game_type:
				return

			if not self.fish_game:
				return

			self.fish_game.MiniGameFishUse(window, shape)

		def MiniGameFishAdd(self, pos, shape):
			if MINIGAME_FISH != self.game_type:
				return

			if not self.fish_game:
				return

			self.fish_game.MiniGameFishAdd(pos, shape)
			
		def MiniGameFishReward(self, vnum):
			if MINIGAME_FISH != self.game_type:
				return

			if not self.fish_game:
				return

			self.fish_game.MiniGameFishReward(vnum)

		def MiniGameFishCount(self, count):
			if MINIGAME_FISH != self.game_type:
				return

			if not self.fish_game:
				return

			self.fish_game.MiniGameFishCount(count)

	if ENABLE_MINI_GAME_CATCH_KING:
		def __ClickCatchKingButton(self):
			self.__CloseAll(MINIGAME_CATCHKING)
				
			if self.mini_game_dialog:
				self.mini_game_dialog.Close()

			if not self.catchking_game:
				self.catchking_game = uiMiniGameCatchKing.MiniGameCatchKing()

			self.main_game = self.catchking_game
			self.game_type = MINIGAME_CATCHKING
			self.main_game.Open()

		def MiniGameCatchKingEventStart(self, bigScore):
			if MINIGAME_CATCHKING != self.game_type:
				return

			if not self.catchking_game:
				return

			self.catchking_game.GameStart(bigScore)
				
		def MiniGameCatchKingSetHandCard(self, cardNumber):
			if MINIGAME_CATCHKING != self.game_type:
				return

			if not self.catchking_game:
				return

			self.catchking_game.CatchKingSetHandCard(cardNumber)
				
		def MiniGameCatchKingResultField(self, score, rowType, cardPos, cardValue, keepFieldCard, destroyHandCard, getReward, isFiveNear):
			if MINIGAME_CATCHKING != self.game_type:
				return

			if not self.catchking_game:
				return

			self.catchking_game.CatchKingResultField(score, rowType, cardPos, cardValue, keepFieldCard, destroyHandCard, getReward, isFiveNear)
				
		def MiniGameCatchKingSetEndCard(self, cardPos, cardNumber):
			if MINIGAME_CATCHKING != self.game_type:
				return

			if not self.catchking_game:
				return

			self.catchking_game.CatchKingSetEndCard(cardPos, cardNumber)
				
		def MiniGameCatchKingReward(self, rewardCode):
			if MINIGAME_CATCHKING != self.game_type:
				return

			if not self.catchking_game:
				return

			self.catchking_game.CatchKingReward(rewardCode)

