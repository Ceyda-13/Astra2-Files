import ui
import localeInfo
import app
import ime
import uiScriptLocale
import chat
import net
import __builtin__ as selfs

class PopupDialog(ui.ScriptWindow):

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.__LoadDialog()
		self.acceptEvent = lambda *arg: None

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __LoadDialog(self):
		try:
			PythonScriptLoader = ui.PythonScriptLoader()
			PythonScriptLoader.LoadScriptFile(self, "UIScript/PopupDialog.py")

			self.board = self.GetChild("board")
			self.message = self.GetChild("message")
			self.accceptButton = self.GetChild("accept")
			self.accceptButton.SetEvent(ui.__mem_func__(self.Close))

		except:
			import exception
			exception.Abort("PopupDialog.LoadDialog.BindObject")

	def Open(self):
		self.SetCenterPosition()
		self.SetTop()
		self.Show()

	def Close(self):
		self.Hide()
		self.acceptEvent()

	def Destroy(self):
		self.Close()
		self.ClearDictionary()

	def SetWidth(self, width):
		height = self.GetHeight()
		self.SetSize(width, height)
		self.board.SetSize(width, height)
		self.SetCenterPosition()
		self.UpdateRect()

	def SetText(self, text):
		self.message.SetText(text)

	def SetAcceptEvent(self, event):
		self.acceptEvent = event

	def SetButtonName(self, name):
		self.accceptButton.SetText(name)

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def OnIMEReturn(self):
		self.Close()
		return True

class InputDialog(ui.ScriptWindow):

	def __init__(self):
		ui.ScriptWindow.__init__(self)

		self.__CreateDialog()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __CreateDialog(self):

		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "uiscript/inputdialog.py")

		getObject = self.GetChild
		self.board = getObject("Board")
		self.acceptButton = getObject("AcceptButton")
		self.cancelButton = getObject("CancelButton")
		self.inputSlot = getObject("InputSlot")
		self.inputValue = getObject("InputValue")

	def Open(self):
		self.inputValue.SetFocus()
		self.SetCenterPosition()
		self.SetTop()
		self.Show()

	def Close(self):
		self.ClearDictionary()
		self.board = None
		self.acceptButton = None
		self.cancelButton = None
		self.inputSlot = None
		self.inputValue = None
		self.Hide()

	def SetTitle(self, name):
		self.board.SetTitleName(name)

	def SetNumberMode(self):
		self.inputValue.SetNumberMode()

	def SetSecretMode(self):
		self.inputValue.SetSecret()

	def SetFocus(self):
		self.inputValue.SetFocus()

	def SetMaxLength(self, length):
		width = length * 6 + 10
		self.SetBoardWidth(max(width + 50, 160))
		self.SetSlotWidth(width)
		self.inputValue.SetMax(length)

	def SetSlotWidth(self, width):
		self.inputSlot.SetSize(width, self.inputSlot.GetHeight())
		self.inputValue.SetSize(width, self.inputValue.GetHeight())
		if self.IsRTL():
			self.inputValue.SetPosition(self.inputValue.GetWidth(), 0)

	def SetBoardWidth(self, width):
		self.SetSize(max(width + 50, 160), self.GetHeight())
		self.board.SetSize(max(width + 50, 160), self.GetHeight())
		if self.IsRTL():
			self.board.SetPosition(self.board.GetWidth(), 0)
		self.UpdateRect()

	def SetAcceptEvent(self, event):
		self.acceptButton.SetEvent(event)
		self.inputValue.OnIMEReturn = event

	def SetCancelEvent(self, event):
		self.board.SetCloseEvent(event)
		self.cancelButton.SetEvent(event)
		self.inputValue.OnPressEscapeKey = event

	def GetText(self):
		return self.inputValue.GetText()

class InputDialogWithDescription(InputDialog):

	def __init__(self):
		ui.ScriptWindow.__init__(self)

		self.__CreateDialog()

	def __del__(self):
		InputDialog.__del__(self)

	def __CreateDialog(self):

		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "uiscript/inputdialogwithdescription.py")

		try:
			getObject = self.GetChild
			self.board = getObject("Board")
			self.acceptButton = getObject("AcceptButton")
			self.cancelButton = getObject("CancelButton")
			self.inputSlot = getObject("InputSlot")
			self.inputValue = getObject("InputValue")
			self.description = getObject("Description")

		except:
			import exception
			exception.Abort("InputDialogWithDescription.LoadBoardDialog.BindObject")

	def SetDescription(self, text):
		self.description.SetText(text)

class InputDialogWithDescription2(InputDialog):

	def __init__(self):
		ui.ScriptWindow.__init__(self)

		self.__CreateDialog()

	def __del__(self):
		InputDialog.__del__(self)

	def __CreateDialog(self):

		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "uiscript/inputdialogwithdescription2.py")

		try:
			getObject = self.GetChild
			self.board = getObject("Board")
			self.acceptButton = getObject("AcceptButton")
			self.cancelButton = getObject("CancelButton")
			self.inputSlot = getObject("InputSlot")
			self.inputValue = getObject("InputValue")
			self.description1 = getObject("Description1")
			self.description2 = getObject("Description2")

		except:
			import exception
			exception.Abort("InputDialogWithDescription.LoadBoardDialog.BindObject")

	def SetDescription1(self, text):
		self.description1.SetText(text)

	def SetDescription2(self, text):
		self.description2.SetText(text)

class QuestionDialog(ui.ScriptWindow):

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.__CreateDialog()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __CreateDialog(self):
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "uiscript/questiondialog.py")

		self.board = self.GetChild("board")
		self.textLine = self.GetChild("message")
		self.acceptButton = self.GetChild("accept")
		self.cancelButton = self.GetChild("cancel")

	def Open(self):
		self.SetCenterPosition()
		self.SetTop()
		self.Show()

	def Close(self):
		self.Hide()

	def SetWidth(self, width):
		height = self.GetHeight()
		self.SetSize(width, height)
		self.board.SetSize(width, height)
		self.SetCenterPosition()
		self.UpdateRect()

	def SAFE_SetAcceptEvent(self, event):
		self.acceptButton.SAFE_SetEvent(event)

	def SAFE_SetCancelEvent(self, event):
		self.cancelButton.SAFE_SetEvent(event)

	def SetAcceptEvent(self, event):
		self.acceptButton.SetEvent(event)

	def SetCancelEvent(self, event):
		self.cancelButton.SetEvent(event)

	def SetText(self, text):
		self.textLine.SetText(text)

	def SetAcceptText(self, text):
		self.acceptButton.SetText(text)

	def SetCancelText(self, text):
		self.cancelButton.SetText(text)

	def OnPressEscapeKey(self):
		self.Close()
		return True

class QuestionDialogItem(ui.ScriptWindow): # @@@correction045
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.__CreateDialog()
	def __del__(self):
		ui.ScriptWindow.__del__(self)
	def __CreateDialog(self):
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "uiscript/questiondialogitem.py")
		self.board = self.GetChild("board")
		self.textLine = self.GetChild("message")
		self.acceptButton = self.GetChild("accept")
		self.destroyButton = self.GetChild("destroy")
		self.cancelButton = self.GetChild("cancel")
	def Open(self):
		self.SetCenterPosition()
		self.SetTop()
		self.Show()
	def Close(self):
		self.Hide()
	def SetWidth(self, width):
		height = self.GetHeight()
		self.SetSize(width, height)
		self.board.SetSize(width, height)
		self.SetCenterPosition()
		self.UpdateRect()
	def SAFE_SetAcceptEvent(self, event):
		self.acceptButton.SAFE_SetEvent(event)
	def SAFE_SetCancelEvent(self, event):
		self.cancelButton.SAFE_SetEvent(event)
	def SetAcceptEvent(self, event):
		self.acceptButton.SetEvent(event)
	def SetDestroyEvent(self, event):
		self.destroyButton.SetEvent(event)
	def SetCancelEvent(self, event):
		self.cancelButton.SetEvent(event)
	def SetText(self, text):
		self.textLine.SetText(text)
	def SetAcceptText(self, text):
		self.acceptButton.SetText(text)
	def SetCancelText(self, text):
		self.cancelButton.SetText(text)
	def OnPressEscapeKey(self):
		self.Close()
		return True

class QuestionDialog2(QuestionDialog):

	def __init__(self):
		QuestionDialog.__init__(self)
		self.__CreateDialog()

	def __del__(self):
		QuestionDialog.__del__(self)

	def __CreateDialog(self):
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "uiscript/questiondialog2.py")

		self.board = self.GetChild("board")
		self.textLine1 = self.GetChild("message1")
		self.textLine2 = self.GetChild("message2")
		self.acceptButton = self.GetChild("accept")
		self.cancelButton = self.GetChild("cancel")

	def SetText1(self, text):
		self.textLine1.SetText(text)

	def SetText2(self, text):
		self.textLine2.SetText(text)

class QuestionDialogWithTimeLimit(QuestionDialog2):
	def __init__(self):
		ui.ScriptWindow.__init__(self)

		self.__CreateDialog()
		self.endTime = 0
		# @@@correction045 BEGIN
		self.timeOverMsg = 0
		self.timeOverEvent = None
		self.timeOverEventArgs = None
		# @@@correction045 END

	def __del__(self):
		QuestionDialog2.__del__(self)

	def __CreateDialog(self):
		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "uiscript/questiondialog2.py")

		self.board = self.GetChild("board")
		self.textLine1 = self.GetChild("message1")
		self.textLine2 = self.GetChild("message2")
		self.acceptButton = self.GetChild("accept")
		self.cancelButton = self.GetChild("cancel")

	def Open(self, timeout):
		self.SetCenterPosition()
		self.SetTop()
		self.Show()

		self.endTime = app.GetTime() + timeout

# @@@correction045 BEGIN
	def SetTimeOverEvent(self, event, *args):
		self.timeOverEvent = event
		self.timeOverEventArgs = args

	def SetTimeOverMsg(self, msg):
		self.timeOverMsg = msg

	def OnTimeOver(self):
		if self.timeOverEvent:
			apply(self.timeOverEvent, self.timeOverEventArgs)
		if self.timeOverMsg:
			chat.AppendChat(chat.CHAT_TYPE_INFO, self.timeOverMsg)
# @@@correction045 END

	def OnUpdate(self):
		leftTime = max(0, self.endTime - app.GetTime())
		self.SetText2(localeInfo.UI_LEFT_TIME % (leftTime))
		if leftTime <= 0: # @@@correction045 BEGIN
			self.OnTimeOver()

class MoneyInputDialog(ui.ScriptWindow):

	def __init__(self):
		ui.ScriptWindow.__init__(self)

		self.moneyHeaderText = localeInfo.MONEY_INPUT_DIALOG_SELLPRICE
		self.__CreateDialog()
		self.SetMaxLength(17)

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __CreateDialog(self):

		pyScrLoader = ui.PythonScriptLoader()
		pyScrLoader.LoadScriptFile(self, "uiscript/moneyinputdialog.py")

		getObject = self.GetChild
		self.board = self.GetChild("board")
		self.acceptButton = getObject("AcceptButton")
		self.cancelButton = getObject("CancelButton")
		self.inputValue = getObject("InputValue")
		if not ENABLE_MONEY_SHORTING:
			self.inputValue.SetNumberMode()
		self.inputValue.OnIMEUpdate = ui.__mem_func__(self.__OnValueUpdate)
		self.moneyText = getObject("MoneyValue")
		if ENABLE_OFFLINE_PRIVATE_SHOP:
			self.minText = getObject("minText")
			self.maxText = getObject("maxText")

	def Open(self):
		self.inputValue.SetText("")
		self.inputValue.SetFocus()
		self.__OnValueUpdate()
		self.SetCenterPosition()
		self.SetTop()
		self.Show()

	def Close(self):
		self.ClearDictionary()
		self.board = None
		self.acceptButton = None
		self.cancelButton = None
		self.inputValue = None
		self.Hide()

	def SetTitle(self, name):
		self.board.SetTitleName(name)

	def SetFocus(self):
		self.inputValue.SetFocus()

	def SetMaxLength(self, length):
		# length = max(9, length)
		self.inputValue.SetMax(length)

	def SetMoneyHeaderText(self, text):
		self.moneyHeaderText = text

	def SetAcceptEvent(self, event):
		self.acceptButton.SetEvent(event)
		self.inputValue.OnIMEReturn = event

	def SetCancelEvent(self, event):
		self.board.SetCloseEvent(event)
		self.cancelButton.SetEvent(event)
		self.inputValue.OnPressEscapeKey = event

	def SetValue(self, value):
		value=str(value)
		self.inputValue.SetText(value)
		self.__OnValueUpdate()
		ime.SetCursorPosition(len(value))


	def GetText(self):
		return self.inputValue.GetText()

	def __OnValueUpdate(self):
		ui.EditLine.OnIMEUpdate(self.inputValue)

		text = self.inputValue.GetText()
		if ENABLE_MONEY_SHORTING:
			text = text.replace("k", "000")

		money = 0
		if text and text.isdigit():
			try:
				money = int(text)
			except ValueError:
				money = 199999999

		self.moneyText.SetText(self.moneyHeaderText + localeInfo.NumberToMoneyString(money))

	if ENABLE_OFFLINE_PRIVATE_SHOP:
		def SetCheckPrices(self, min, max):
			self.minText.SetText("Min: " + localeInfo.NumberToMoneyString(min))
			self.maxText.SetText("Max: " + localeInfo.NumberToMoneyString(max))

if ENABLE_GROWTH_PET_SYSTEM:
	class InputDialogName(ui.ScriptWindow):

		def __init__(self):
			ui.ScriptWindow.__init__(self)

			self.__CreateDialog()

		def __del__(self):
			ui.ScriptWindow.__del__(self)

		def __CreateDialog(self):

			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/inputdialog_name.py")

			getObject = self.GetChild
			self.board = getObject("Board")
			self.acceptButton = getObject("AcceptButton")
			self.cancelButton = getObject("CancelButton")
			self.inputSlot = getObject("InputSlot")
			self.inputValue = getObject("InputValue")

		def Open(self):
			self.inputValue.SetFocus()
			self.SetCenterPosition()
			self.SetTop()
			self.Show()

		def Close(self):
			self.ClearDictionary()
			self.board = None
			self.acceptButton = None
			self.cancelButton = None
			self.inputSlot = None
			self.inputValue = None
			self.Hide()

		def SetTitle(self, name):
			self.board.SetTitleName(name)

		def SetNumberMode(self):
			self.inputValue.SetNumberMode()

		def SetSecretMode(self):
			self.inputValue.SetSecret()

		def SetFocus(self):
			self.inputValue.SetFocus()

		def SetMaxLength(self, length):
			width = length * 6 + 10
			self.SetBoardWidth(max(width + 50, 160))
			self.SetSlotWidth(width)
			self.inputValue.SetMax(length)

		def SetSlotWidth(self, width):
			self.inputSlot.SetSize(width, self.inputSlot.GetHeight())
			self.inputValue.SetSize(width, self.inputValue.GetHeight())
			if self.IsRTL():
				self.inputValue.SetPosition(self.inputValue.GetWidth(), 0)

		def SetBoardWidth(self, width):
			self.SetSize(max(width + 50, 160), self.GetHeight())
			self.board.SetSize(max(width + 50, 160), self.GetHeight())	
			if self.IsRTL():
				self.board.SetPosition(self.board.GetWidth(), 0)
			self.UpdateRect()

		def SetAcceptEvent(self, event):
			self.acceptButton.SetEvent(event)
			self.inputValue.OnIMEReturn = event

		def SetCancelEvent(self, event):
			self.board.SetCloseEvent(event)
			self.cancelButton.SetEvent(event)
			self.inputValue.OnPressEscapeKey = event

		def GetText(self):
			return self.inputValue.GetText()	
			
	if WJ_SECURITY_SYSTEM:
		class CaptchaEkran(QuestionDialog2):
			def __init__(self):
				ui.ScriptWindow.__init__(self)
				self.btns = []
				self.__CreateDialog()
				self.endTime = 0
				self.timeOverMsg = 0
				self.timeOverEvent = None
				self.timeOverEventArgs = None

			def __del__(self):
				QuestionDialog2.__del__(self)

			def __CreateDialog(self):
				pyScrLoader = ui.PythonScriptLoader()
				pyScrLoader.LoadScriptFile(self, "uiscript/captcha.py")

				self.board = self.GetChild("board")
				self.textLine1 = self.GetChild("message1")
				self.textLine2 = self.GetChild("message2")
				self.textLine3 = self.GetChild("message3")
				self.textLine3.Hide()
				self.cp1 = self.GetChild("cp1")
				self.cp2 = self.GetChild("cp2")
				self.cp3 = self.GetChild("cp3")
				self.cp4 = self.GetChild("cp4")
				self.cp5 = self.GetChild("cp5")
				for i in xrange(6):
					self.btns.append(self.GetChild("b%d" % i))
					self.btns[i].SetEvent(ui.__mem_func__(self.Select), i)

			def Select(self, i):
				net.SendChatPacket("/cp_a " + self.btns[i].GetText())
				selfs.wndCaptcha.Close()
				selfs.wndCaptcha = None

			def Open(self, timeout, cpp1, cpp2, cpp3, cpp4, cpp5):
				self.SetCenterPosition()
				self.SetTop()
				self.cp1.LoadImage("captcha/"+str(cpp1)+".tga")
				self.cp1.Show()
				self.cp2.LoadImage("captcha/"+str(cpp2)+".tga")
				self.cp2.Show()
				self.cp3.LoadImage("captcha/"+str(cpp3)+".tga")
				self.cp3.Show()
				self.cp4.LoadImage("captcha/"+str(cpp4)+".tga")
				self.cp4.Show()
				self.cp5.LoadImage("captcha/"+str(cpp5)+".tga")
				self.cp5.Show()
				self.Show()
				rand = app.GetRandom(0, 5)
				self.btns[rand].SetText(str(cpp1) + str(cpp2) + str(cpp3) + str(cpp4) + str(cpp5))

				for i in xrange(6):
					if i != rand:
						self.btns[i].SetText(str(app.GetRandom(1, 9)) + str(app.GetRandom(1, 9)) + str(app.GetRandom(1, 9)) + str(app.GetRandom(1, 9)) + str(app.GetRandom(1, 9)))

				self.endTime = app.GetTime() + timeout

			def SetTimeOverEvent(self, event, *args):
				self.timeOverEvent = event
				self.timeOverEventArgs = args

			def SetTimeOverMsg(self, msg):
				self.timeOverMsg = msg
		
			def SetText1(self, text):
				self.textLine1.SetText(text)
				self.textLine1.Show()

			def SetText2(self, text):
				self.textLine2.SetText(text)
				self.textLine2.Show()
		
			def SetText3(self, text):
				self.textLine3.SetText(text)
				self.textLine3.Show()
		
			def GetInput(self):
				return ""
				# return self.InputValue.GetText()
		
			def Temizle(self):
				# self.InputValue.SetText("")
				pass

			def OnTimeOver(self):
				if self.timeOverEvent:
					apply(self.timeOverEvent, self.timeOverEventArgs)
				if self.timeOverMsg:
					chat.AppendChat(chat.CHAT_TYPE_INFO, self.timeOverMsg)

			def OnUpdate(self):
				leftTime = max(0, self.endTime - app.GetTime())
				self.SetText2(localeInfo.UI_LEFT_TIME % (leftTime))
				if leftTime <= 0:
					self.OnTimeOver()
