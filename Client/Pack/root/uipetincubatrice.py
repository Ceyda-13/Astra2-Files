import ui
import player
import chat
import __builtin__ as selfs

class PetSystemIncubator(ui.ScriptWindow):
	
	def __init__(self, cell, new_pet):
		ui.ScriptWindow.__init__(self)
		self.cell = cell
		self.__LoadWindow()
		self.LoadPetIncubatorImg(new_pet)

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def Show(self):
		ui.ScriptWindow.Show(self)

	def Close(self):
		self.Hide()
		if WJ_ENABLE_TRADABLE_ICON:
			selfs.wndInventory.SetCanMouseEventSlot(self.cell)

	def __LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/PetHatchingWindow.py")
		except:
			import exception
			exception.Abort("PetHatchingWindow.LoadWindow.LoadObject")

		try:
			self.board = self.GetChild("board")
			self.boardtitle = self.GetChild("PetHatching_TitleBar")

			self.petimg = self.GetChild("HatchingItemSlot")
			self.petname = self.GetChild("pet_name")
			self.HatchingButton = self.GetChild("HatchingButton")

			self.boardtitle.SetCloseEvent(ui.__mem_func__(self.Close))
			self.HatchingButton.SetEvent(ui.__mem_func__(self.RequestHatching))

		except:
			import exception
			exception.Abort("PetHatchingWindow.LoadWindow.BindObject")
			
	def LoadPetIncubatorImg(self, new_pet):
		petarryname = ["Maymun Yumurtasý", "Örümcek Yumurtasý", "Razadör Yumurtasý", "Nemere Yumurtasý", "Mavi Ejderha Yumurtasý", "Kýzýl Ejderha Yumurtasý", "Azrailcik Yumurtasý", "Tombik cellatck Yumurtasý", "Baashido Yavrusu Yumurtasý", "Nessie Yumurtasý"]
		petarryimg = [55701, 55702, 55703, 55704, 55705, 55706, 55707, 55708, 55709, 55710]
		chat.AppendChat(chat.CHAT_TYPE_INFO, "[Pet-Kulucka] "+petarryname[int(new_pet)]+".")
		self.petimg.SetItemSlot(0, petarryimg[int(new_pet)], 0)
		self.petimg.SetAlwaysRenderCoverButton(0, True)
		if WJ_ENABLE_TRADABLE_ICON:
			selfs.wndInventory.SetCantMouseEventSlot(self.cell)

	def RequestHatching(self):
		if self.petname.GetText() == "" or len(self.petname.GetText()) < 4:
			chat.AppendChat(chat.CHAT_TYPE_INFO, "[Pet-Kulucka] Pet ismi Gecerli degil!")
			return
			
		if player.GetElk() < 100000:
			return
			
		chat.AppendChat(chat.CHAT_TYPE_INFO, "[Pet-Kulucka]Pet yumurtadan Cikti.")
		import chr
		chr.RequestPetName(self.petname.GetText())
		self.Close()

	def OnUpdate(self):
		if WJ_ENABLE_TRADABLE_ICON:
			selfs.wndInventory.SetCantMouseEventSlot(self.cell)

