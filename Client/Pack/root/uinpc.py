import ui, app, net
import __builtin__ as selfs

class Gui(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.__Initialize()
		self.__LoadWindow()

	def __del__(self):
		ui.ScriptWindow.__del__(self)	

	def __Initialize(self):
		self.ButonList = []
		self.NPCList = [21, 11, 1, 26, 421, 20471, 6000, 20406, 51, 20503, 20504, 41]
		self.btnTexts = ["Satýcý", "Zýrh Satýcýsý", "Silah Satýcýsý", "Huahn-So", "Biyolog Yardýmcýsý", "Eþya Tüccarý", "Yýl Dönümü Tüccarý", "Theowahdan", "Simyacý", "Metin Gaya Maðazasý", "Boss Gaya Maðazasý", "Balýkçý"]

	def __LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "UIScript/MoveChannelDialog.py")
		except:
			import exception
			exception.Abort("MoveChannelDialog.__LoadDialog")

		self.ParentBoard = self.GetChild("MoveChannelBoard")
		self.ChildBoard = self.GetChild("BlackBoard")
		self.GetChild("MoveChannelTitle").SetCloseEvent(ui.__mem_func__(self.OnPressEscapeKey))
		self.GetChild("TitleName").SetText("Uzaktan Market")

		self.DlgWidht = 190
		self.BlackBoardHeight = 23 * len(self.NPCList) + 5 * (len(self.NPCList) - 1) + 13
		self.DlgHeight = self.BlackBoardHeight + 50

		for i in xrange(len(self.NPCList)):
			btn = ui.Button()
			btn.SetParent(self.ChildBoard)
			btn.SetPosition(8, 6 + i*28)
			btn.SetUpVisual("d:/ymir work/ui/game/myshop_deco/select_btn_01.sub")
			btn.SetOverVisual("d:/ymir work/ui/game/myshop_deco/select_btn_02.sub")
			btn.SetDownVisual("d:/ymir work/ui/game/myshop_deco/select_btn_03.sub")
			btn.SetToolTipText("")
			btn.SetEvent(ui.__mem_func__(self.NPC_Open), self.NPCList[i])
			btn.SetText(self.btnTexts[i])
			btn.Show()
			self.ButonList.append(btn)
		
		self.ParentBoard.SetSize(self.DlgWidht, self.DlgHeight)
		self.ChildBoard.SetSize(self.DlgWidht - 26, self.BlackBoardHeight)
		self.SetSize(self.DlgWidht, self.DlgHeight)
		
		self.UpdateRect()

	def NPC_Open(self, vnum):
		net.SendChatPacket("/npc_open %d" % vnum)
	
	def Open(self):
		if self.IsShow():
			self.Hide()
		else:
			self.Show()

	def Close(self):
		self.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True

