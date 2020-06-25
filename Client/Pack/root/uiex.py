import app
import wndMgr
import systemSetting
import mouseModule
import ui

class Window(ui.Window):
	def __init__(self, skinFileName = ""):
		ui.Window.__init__(self, "UI")

		self.children = []
		self.childDict = {}

		self.__LoadSkin(skinFileName)

		self.Show()

	def __del__(self):
		ui.Window.__del__(self)

	def ClearDictionary(self):
		self.children	= []
		self.childDict	= {}

	def InsertChild(self, name, child):
		self.childDict[name] = child

	def IsChild(self, name):
		return name in self.childDict

	def GetChild(self, name):
		return self.childDict[name]

	def __LoadSkin(self, fileName):
		loader = ui.PythonScriptLoader()
		loader.LoadScriptFile(self, fileName)

