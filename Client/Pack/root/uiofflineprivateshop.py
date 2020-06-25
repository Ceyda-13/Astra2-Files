import ui
import localeInfo
import net
import __builtin__ as selfs
import app

class OfflinePrivateShop(ui.ScriptWindow):
	btnTexts = ["Aktif Pazarlar", "Kapanan Pazarlar", "Yeni Pazar Aç"]
	cnt = 3
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.__LoadDialog()
		
		self.IsShow = False
		self.time = [0, 0, 0]
		
	def __del__(self):
		ui.ScriptWindow.__del__(self)
		
	def __LoadDialog(self):	
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "UIScript/MoveChannelDialog.py")
		except:
			import exception
			exception.Abort("MoveChannelDialog.__LoadDialog")

		self.ParentBoard = self.GetChild("MoveChannelBoard")
		self.ChildBoard = self.GetChild("BlackBoard")
		self.GetChild("MoveChannelTitle").SetCloseEvent(ui.__mem_func__(self.OnPressEscapeKey))
		self.GetChild("TitleName").SetText("Çevrim Dýþý Pazar Menüsü")
		
		self.ChannelList = []
		
		self.DlgWidht = 190
		self.BlackBoardHeight = 23 * self.cnt + 5 * (self.cnt - 1) + 13
		self.DlgHeight = self.BlackBoardHeight + 50

		for i in xrange(self.cnt):
			btn = ui.ToggleButton()
			btn.SetParent(self.ChildBoard)
			btn.SetPosition(8, 6 + i*28)
			btn.SetUpVisual("d:/ymir work/ui/game/myshop_deco/select_btn_01.sub")
			btn.SetOverVisual("d:/ymir work/ui/game/myshop_deco/select_btn_02.sub")
			btn.SetDownVisual("d:/ymir work/ui/game/myshop_deco/select_btn_03.sub")
			btn.SetToolTipText("")
			btn.SetToggleUpEvent(ui.__mem_func__(self.__SelectChannel), i+1)
			btn.SetToggleDownEvent(ui.__mem_func__(self.__SelectChannel), i+1)
			btn.SetText(self.btnTexts[i])
			btn.Show()
			self.ChannelList.append(btn)
		
		self.ParentBoard.SetSize(self.DlgWidht, self.DlgHeight)
		self.ChildBoard.SetSize(self.DlgWidht - 26, self.BlackBoardHeight)
		self.SetSize(self.DlgWidht, self.DlgHeight)
		
		self.UpdateRect()
		
	def __SelectChannel(self, idx):
		if idx == 1:
			if selfs.wndOfflinePrivateShopEdit.IsShow():
				selfs.wndOfflinePrivateShopEdit.Close()
				self.ChannelList[idx-1].SetUp()
			else:
				if self.time[idx] <= app.GetGlobalTimeStamp():
					net.SendChatPacket("/click_offline_private_shop 0")
					self.time[idx] = app.GetGlobalTimeStamp() + 10
				else:
					selfs.wndOfflinePrivateShopEdit.Open()

				self.ChannelList[idx-1].Down()
		elif idx == 2:
			if selfs.wndOfflinePrivateShopNotSell.IsShow():
				selfs.wndOfflinePrivateShopNotSell.Close()
				self.ChannelList[idx-1].SetUp()
			else:
				if self.time[idx] <= app.GetGlobalTimeStamp():
					net.SendChatPacket("/click_offline_private_shop 1")
					self.time[idx] = app.GetGlobalTimeStamp() + 10
				else:
					selfs.wndOfflinePrivateShopNotSell.Open()

				self.ChannelList[idx-1].Down()
		elif idx == 3:
			if selfs.wndPrivateShopBuilder.IsShow():
				selfs.wndPrivateShopBuilder.Close()
				self.ChannelList[idx-1].SetUp()
			else:
				selfs.wndPrivateShopBuilder.Open("")
				self.ChannelList[idx-1].Down()
					
	def SetTime(self, idx, time = 0):
		self.time[idx] = time

	def OnPressEscapeKey(self):
		self.Hide()
		return True
		
	def SetButton(self, idx, status = 0):
		if status:
			self.ChannelList[idx].Down()
		else:
			self.ChannelList[idx].SetUp()

