import ui
import chr

class Render(ui.Window):
	def __init__(self):
		ui.Window.__init__(self)
		self.Board = None
		self.ModelWindow = None
		self.CacheData = (0, 0, 0, 0, 0, chr.MOTION_MODE_GENERAL)
		self.Init()

	def __del__(self):
		ui.Window.__del__(self)

	def Init(self):
		self.Board = ui.ThinBoard()
		self.Board.SetSize(500, 670)
		self.Board.SetCenterPosition()
		# self.Board.AddFlag('movable')
		self.Board.AddFlag('float')
		# self.board.SetTitleName('Render Target')
		# self.board.SetCloseEvent(self.Close)

		self.ModelWindow = ui.RenderTarget()
		self.ModelWindow.SetParent(self.Board)
		self.ModelWindow.SetPosition(20, 20)
		self.ModelWindow.SetSize(460, 610)
		self.ModelWindow.SetRenderTarget(2)
		self.ModelWindow.SetAutoRotation(1.0)
		self.ModelWindow.SetRenderTargetData(0, 12, 100, 4)
		self.ModelWindow.Show()

		self.button = ui.Button()
		self.button.SetParent(self.Board)
		self.button.SetPosition(219.5, 640)
		self.button.SetUpVisual('d:/ymir work/ui/public/middle_button_01.sub')
		self.button.SetOverVisual('d:/ymir work/ui/public/middle_button_02.sub')
		self.button.SetDownVisual('d:/ymir work/ui/public/middle_button_03.sub')
		self.button.SetText("Kapat")
		self.button.SetEvent(self.Close)
		self.button.Show()

	def Open(self):
		self.Board.Show()
		self.Show()

	def Close(self):
		self.Board.Hide()
		self.Hide()

	def Destroy(self):
		self.Board = None
		self.ModelWindow = None

	def AppendModel(self, race, armor = 0, weapon = 0, hair = 0, acce = 0, motion = chr.MOTION_MODE_GENERAL):
		if self.CacheData != (race, armor, weapon, hair, acce, motion):
			self.ModelWindow.SetRenderTargetData(race, armor, weapon, hair, acce, motion)
			self.CacheData = (race, armor, weapon, hair, acce, motion)

		# if not self.Board.IsShow():
			# self.Board.Show()

	def DeleteModel(self):
		if self.Board.IsShow():
			self.Close()

		self.CacheData = (0, 0, 0, 0, 0, chr.MOTION_MODE_GENERAL)

	def ModelWindowProcess(self):
		if self.Board.IsShow():
			self.Close()
		else:
			if self.CacheData != (0, 0, 0, 0, 0, chr.MOTION_MODE_GENERAL):
				self.Open()

	def OnPressEscapeKey(self):
		self.Close()
		return True

