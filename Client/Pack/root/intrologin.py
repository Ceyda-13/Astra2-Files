import app
import net
import ui
import snd
import wndMgr
import musicInfo
import serverInfo
import systemSetting
import localeInfo
import constInfo
import ime
import uiScriptLocale
import ServerStateChecker
import dbg
import binascii
import __builtin__ as selfs

class LoginWindow(ui.ScriptWindow):
	def __init__(self, stream):
		ui.ScriptWindow.__init__(self)
		
		net.SetPhaseWindow(net.PHASE_WINDOW_LOGIN, self)
		net.SetAccountConnectorHandler(self)

		self.stream = stream
		self.channelButton = None
		self.channelChTxt = None

	def __del__(self):
		ui.ScriptWindow.__del__(self)
		
		net.ClearPhaseWindow(net.PHASE_WINDOW_LOGIN, self)
		net.SetAccountConnectorHandler(0)

	def Open(self):
		ServerStateChecker.Create(self)
		self.loginFailureMsgDict={
			"ALREADY"	: localeInfo.LOGIN_FAILURE_ALREAY,
			"NOID"		: localeInfo.LOGIN_FAILURE_NOT_EXIST_ID,
			"WRONGPWD"	: localeInfo.LOGIN_FAILURE_WRONG_PASSWORD,
			"FULL"		: localeInfo.LOGIN_FAILURE_TOO_MANY_USER,
			"SHUTDOWN"	: localeInfo.LOGIN_FAILURE_SHUTDOWN,
			"REPAIR"	: localeInfo.LOGIN_FAILURE_REPAIR_ID,
			"BLOCK"		: localeInfo.LOGIN_FAILURE_BLOCK_ID,
			"WRONGMAT"	: localeInfo.LOGIN_FAILURE_WRONG_MATRIX_CARD_NUMBER,
			"QUIT"		: localeInfo.LOGIN_FAILURE_WRONG_MATRIX_CARD_NUMBER_TRIPLE,
			"BESAMEKEY"	: localeInfo.LOGIN_FAILURE_BE_SAME_KEY,
			"NOTAVAIL"	: localeInfo.LOGIN_FAILURE_NOT_AVAIL,
			"NOBILL"	: localeInfo.LOGIN_FAILURE_NOBILL,
			"BLKLOGIN"	: localeInfo.LOGIN_FAILURE_BLOCK_LOGIN,
			"WEBBLK"	: localeInfo.LOGIN_FAILURE_WEB_BLOCK,
		}

		self.loginFailureFuncDict = {
			"WRONGPWD"	: localeInfo.LOGIN_FAILURE_WRONG_PASSWORD,
			"WRONGMAT"	: localeInfo.LOGIN_FAILURE_WRONG_MATRIX_CARD_NUMBER,
			"QUIT"		: app.Exit,
		}

		self.SetSize(wndMgr.GetScreenWidth(), wndMgr.GetScreenHeight())
		self.SetWindowName("LoginWindow")

		self.__LoadScript("uiscript/LoginWindow.py")

		if musicInfo.loginMusic != "":
			snd.SetMusicVolume(systemSetting.GetMusicVolume())
			snd.FadeInMusic("BGM/" + musicInfo.loginMusic)

		snd.SetSoundVolume(systemSetting.GetSoundVolume())

		ime.AddExceptKey(91)
		ime.AddExceptKey(93)
		
		self.SetChannel(1)
		self.SetServ(1)
		self.idEditLine.SetFocus()
		
		self.Show()
		app.ShowCursor()		
		self.__RequestServerStateList()
		self.__RefreshServerStateList()
		
	def Close(self):
		ServerStateChecker.Initialize(self)

		if musicInfo.loginMusic != "" and musicInfo.selectMusic != "":
			snd.FadeOutMusic("BGM/"+musicInfo.loginMusic)

		if self.stream.popupWindow:
			self.stream.popupWindow.Close()

		self.Hide()
		app.HideCursor()
		ime.ClearExceptKey()

	def OnConnectFailure(self):
		snd.PlaySound("sound/ui/loginfail.wav")
		self.PopupNotifyMessage(localeInfo.LOGIN_CONNECT_FAILURE, self.EmptyFunc)

	def OnHandShake(self):
		snd.PlaySound("sound/ui/loginok.wav")
		self.PopupDisplayMessage(localeInfo.LOGIN_CONNECT_SUCCESS)

	def OnLoginStart(self):
		self.PopupDisplayMessage(localeInfo.LOGIN_PROCESSING)

	def OnLoginFailure(self, error):
		try:
			loginFailureMsg = self.loginFailureMsgDict[error]
		except KeyError:
		
			loginFailureMsg = localeInfo.LOGIN_FAILURE_UNKNOWN  + error

		loginFailureFunc = self.loginFailureFuncDict.get(error, self.EmptyFunc)

		self.PopupNotifyMessage(loginFailureMsg, loginFailureFunc)

		snd.PlaySound("sound/ui/loginfail.wav")

	def __LoadScript(self, fileName):
		try:
			ui.PythonScriptLoader().LoadScriptFile(self, fileName)

			self.idEditLine = self.GetChild("UserEditLine")
			self.pwdEditLine = self.GetChild("PwEditLine")
			self.loginButton = self.GetChild("LoginBtn")
			
			self.channelButton = {
				1 : self.GetChild("ChannelBtn1"),
				2 :	self.GetChild("ChannelBtn2"),
				3 : self.GetChild("ChannelBtn3"),
				4 : self.GetChild("ChannelBtn4"),
				5 : self.GetChild("ChannelBtn5"),
				6 : self.GetChild("ChannelBtn6")
			}

			self.servButton = {
				1 : self.GetChild("SvButton1"),
				2 :	self.GetChild("SvButton2")
			}
			
			self.channelChTxt = []
			for i in xrange(6):
				self.channelChTxt.append(self.GetChild("ChannelState%d" % (i + 1)))
			
			self.panelButton = {
				"EXIT"	:	self.GetChild("ExitBtn")
			}
				
			for (action, button) in self.panelButton.items():
				button.SetEvent(ui.__mem_func__(self.MainBoard), action)
				# button.SetText("Çikis")

			# self.GetChild("serv1t").SetText(serverInfo.SRVS[1]["name"])
			# self.GetChild("serv2t").SetText(serverInfo.SRVS[2]["name"])

		except:
			import exception
			exception.Abort("LoginWindow.__LoadScript.BindObject") 

		for (channelID, channelButtons) in self.channelButton.items():
			channelButtons.SetEvent(ui.__mem_func__(self.SetChannel), channelID)

		for (servID, servButtons) in self.servButton.items():
			servButtons.SetEvent(ui.__mem_func__(self.SetServ), servID)

		self.loginButton.SetEvent(ui.__mem_func__(self.__OnClickLoginButton))
		# self.loginButton.SetText("Giris Yap")

		self.idEditLine.SetReturnEvent(ui.__mem_func__(self.pwdEditLine.SetFocus))
		self.idEditLine.SetTabEvent(ui.__mem_func__(self.pwdEditLine.SetFocus))

		self.pwdEditLine.SetReturnEvent(ui.__mem_func__(self.__OnClickLoginButton))
		self.pwdEditLine.SetTabEvent(ui.__mem_func__(self.idEditLine.SetFocus))

		self.AccButtons = []
		a = 38.83333333333333
		for i in xrange(14):
			btn = ui.Button()
			btn.SetParent(self.GetChild("LoginDialog"))
			btn.SetPosition(511 + 2 + (i % 2 * 143), 87 + (i / 2 * a))
			btn.SetUpVisual("d:/ymir work/ui/login_new/acc_btn_default.dds")
			btn.SetOverVisual("d:/ymir work/ui/login_new/acc_btn_over.dds")
			btn.SetDownVisual("d:/ymir work/ui/login_new/acc_btn_down.dds")
			btn.SetEvent(ui.__mem_func__(self.WriteAndReadAccount), i)
			btn.Show()

			text = ui.TextLine()
			text.SetFontName("Arial:16")
			if GetReg("id%d" % i) and GetReg("pwd%d" % i):
				text.SetText(binascii.a2b_base64("%s" % GetReg("id%d" % i)))
			else:
				text.SetText("Empty Slot")
			text.SetParent(btn)
			text.SetVerticalAlignCenter()
			text.SetWindowVerticalAlignCenter()
			text.SetPosition(10, 0)
			text.SetPackedFontColor(0xFFB88E7D)
			text.Show()

			btn2 = ui.Button()
			btn2.SetParent(btn)
			btn2.SetWindowVerticalAlignCenter()
			btn2.SetPosition(120, -5.5)
			btn2.SetUpVisual("d:/ymir work/ui/login_new/acc_x_btn.dds")
			btn2.SetOverVisual("d:/ymir work/ui/login_new/acc_x_btn.dds")
			btn2.SetDownVisual("d:/ymir work/ui/login_new/acc_x_btn.dds")
			btn2.SetEvent(ui.__mem_func__(self.DeleteAccount), i)
			btn2.Show()
			self.AccButtons.append([btn, text, btn2])

		for i in xrange(16):
			button = self.GetChild("Lang%d" % i)
			if i == 10:
				button.Down()

	def MainBoard(self, action):
		if action == "EXIT":
			self.stream.SetPhaseWindow(0)
		
	def SetChannel(self, ch):
		selfs.CHANNEL = int(ch)
		for (channelID, channelButtons) in self.channelButton.items():
			if ch != channelID:
				channelButtons.SetUp()
				channelButtons.Enable()
			else:
				channelButtons.Down()
				channelButtons.Disable()

		self.SetInfos()

	def SetServ(self, sv):
		selfs.SERVER = int(sv)
		for (servID, servButtons) in self.servButton.items():
			if sv != servID:
				servButtons.SetUp()
				servButtons.Enable()
			else:
				servButtons.Down()
				servButtons.Disable()
		self.SetInfos()
		self.__RequestServerStateList()
		self.__RefreshServerStateList()

	def SetInfos(self):
		channelDict = serverInfo.REGION_DICT[0][SERVER]["channel"]
		authDict = serverInfo.REGION_AUTH_SERVER_DICT[0][SERVER]
		markDict = serverInfo.MARKADDR_DICT[SERVER * 10]
		self.stream.SetConnectInfo(channelDict[CHANNEL]["ip"], channelDict[CHANNEL]["tcp_port"], authDict["ip"], authDict["port"])
		net.SetMarkServer(markDict["ip"], markDict["tcp_port"])
		app.SetGuildMarkPath(markDict["mark"])
		app.SetGuildSymbolPath(markDict["symbol_path"])
		net.SetServerInfo("%s, %s" % (serverInfo.REGION_DICT[0][SERVER]["name"], channelDict[CHANNEL]["name"]))

	def Connect(self, id, pwd):
		if ENABLE_SEQUENCE_SYSTEM: # @@@correction023
			net.SetPacketSequenceMode()

		self.SetInfos()

		self.stream.popupWindow.Close()
		self.stream.popupWindow.Open(localeInfo.LOGIN_CONNETING, self.EmptyFunc, localeInfo.UI_CANCEL)

		self.stream.SetLoginInfo(id, pwd)
		self.stream.Connect()
		
	def PopupDisplayMessage(self, msg):
		self.stream.popupWindow.Close()
		self.stream.popupWindow.Open(msg)

	def PopupNotifyMessage(self, msg, func=0):
		if not func:
			func = self.EmptyFunc

		self.stream.popupWindow.Close()
		self.stream.popupWindow.Open(msg, func, localeInfo.UI_OK)

	def OnPressExitKey(self):
		if self.stream.popupWindow:
			self.stream.popupWindow.Close()
		self.stream.SetPhaseWindow(0)
		return TRUE

	def OnUpdate(self):
		ServerStateChecker.Update()

	def EmptyFunc(self):
		pass

	def __OnClickLoginButton(self):
		id = self.idEditLine.GetText()
		pwd = self.pwdEditLine.GetText()

		if len(id)==0:
			self.PopupNotifyMessage(localeInfo.LOGIN_INPUT_ID, self.EmptyFunc)
			return

		if len(pwd)==0:
			self.PopupNotifyMessage(localeInfo.LOGIN_INPUT_PASSWORD, self.EmptyFunc)
			return

		self.Connect(id, pwd)
		
	def WriteAndReadAccount(self, i):
		# if self.AccButtons[i][1] == "Empty Slot":
		if not GetReg("id%d" % i) and not GetReg("pwd%d" % i):		
			if self.idEditLine.GetText() == "" or self.pwdEditLine.GetText() == "":
				self.PopupNotifyMessage("Lütfen Id ve Sifre Girin", self.EmptyFunc)
				return

			SetReg("id%d" % i, str(binascii.b2a_base64(self.idEditLine.GetText())))
			SetReg("pwd%d" % i, str(binascii.b2a_base64(self.pwdEditLine.GetText())))
			self.AccButtons[i][1].SetText(self.idEditLine.GetText())
			self.PopupNotifyMessage("Hesap Kaydedildi !", self.EmptyFunc)
		else:
			self.idEditLine.SetText("%s" % binascii.a2b_base64("%s" % GetReg("id%d" % i)))
			self.pwdEditLine.SetText("%s" % binascii.a2b_base64("%s" % GetReg("pwd%d" % i)))
			self.__OnClickLoginButton()

	def DeleteAccount(self, i):
		SetReg("id%d" % i, "")
		SetReg("pwd%d" % i, "")
		self.AccButtons[i][1].SetText("Empty Slot")

	def __RequestServerStateList(self):
		try:
			channelDict = serverInfo.REGION_DICT[0][SERVER]["channel"]
		except:
			return

		ServerStateChecker.Initialize();
		for id, channelDataDict in channelDict.items():
			key = channelDataDict["key"]
			ip = channelDataDict["ip"]
			udp_port = channelDataDict["udp_port"]
			ServerStateChecker.AddChannel(key, ip, udp_port)
			
		ServerStateChecker.Request()

	def __RefreshServerStateList(self):
		try:
			channelDict = serverInfo.REGION_DICT[0][SERVER]["channel"]
		except:
			return

		for channelID, channelDataDict in channelDict.items():
			channelName = channelDataDict["name"]
			channelState = channelDataDict["state"]
			if channelState == serverInfo.STATE_NONE:
				self.channelChTxt[int(channelID) - 1].SetText(channelState)
			else:
				self.channelChTxt[int(channelID) - 1].SetText(channelState)

	def NotifyChannelState(self, addrKey, state):
		try:
			stateName = serverInfo.STATE_DICT[state]
		except:
			stateName = serverInfo.STATE_NONE

		regionID = int(addrKey / 1000)
		serverID = int(addrKey / 10) % 100
		channelID = addrKey % 10

		try:
			serverInfo.REGION_DICT[regionID][serverID]["channel"][channelID]["state"] = stateName		
			self.__RefreshServerStateList()
		except:
			import exception
			exception.Abort(localeInfo.CHANNEL_NOT_FIND_INFO)

