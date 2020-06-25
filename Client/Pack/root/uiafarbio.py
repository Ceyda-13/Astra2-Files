import ui
import app
import chr
import chrmgr
import player
import net
import background
import chat
import textTail
import event
import effect
import systemSetting
import quest
import guild
import skill
import messenger
import exchange
import ime
import item
import wndMgr, uiCommon, time, playerSettingModule, localeInfo, snd, mouseModule, constInfo, uiScriptLocale, dbg, uiToolTip

class BiyologEkran(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.__Initialize()
		self.__Load()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __Initialize(self):
		self.titleBar = 0
		self.secim = 0
		self.seviye = 0
		self.secimbutonlari = []

	def Destroy(self):
		self.ClearDictionary()

		self.__Initialize()
	
	def __Load_LoadScript(self, fileName):
		try:
			pyScriptLoader = ui.PythonScriptLoader()
			pyScriptLoader.LoadScriptFile(self, fileName)
		except:
			import exception
			exception.Abort("OptionDialog.__Load_LoadScript")

	def __Load_BindObject(self):
		try:
			GetObject = self.GetChild
			self.titleBar = GetObject("titlebar")
			self.baslik = GetObject("titlename")
			self.af1 = GetObject("afv1")
			self.af2 = GetObject("afv2")
			self.af3 = GetObject("afv3")
			self.odulver = GetObject("odulver")
			self.secimbutonlari.append(GetObject("odul1"))
			self.secimbutonlari.append(GetObject("odul2"))
			self.secimbutonlari.append(GetObject("odul3"))
			self.secimbutonlari[0].SAFE_SetEvent(self.__odulsec,0)
			self.secimbutonlari[1].SAFE_SetEvent(self.__odulsec,1)	
			self.secimbutonlari[2].SAFE_SetEvent(self.__odulsec,2)			
			self.odulver.SAFE_SetEvent(self.vergitsin)	
		
		except:
			import exception
			exception.Abort("OptionDialog.__Load_BindObject")

	def __Load(self):
		self.__Load_LoadScript("uiscript/afarbio.py")
		self.__Load_BindObject()
		self.SetCenterPosition()
		self.titleBar.SetCloseEvent(ui.__mem_func__(self.Close))

	def vergitsin(self):
		net.SendChatPacket("/afarbio " + str(self.seviye) + " " + str(self.secim))
	
	def __odulsec(self, odul):
		if int(odul) == 0:
			self.secimbutonlari[0].Down()
			self.secimbutonlari[1].SetUp()
			self.secimbutonlari[2].SetUp()
			self.secim = 1
		elif int(odul) == 1:
			self.secimbutonlari[1].Down()
			self.secimbutonlari[0].SetUp()
			self.secimbutonlari[2].SetUp()
			self.secim = 2
		else:
			self.secimbutonlari[2].Down()
			self.secimbutonlari[1].SetUp()
			self.secimbutonlari[0].SetUp()
			self.secim = 3
		return

	def OnPressEscapeKey(self):
		self.Close()
		return True
		
	def SetBaslik(self, baslik):
		self.baslik.SetText(str(baslik)+ " Görevi Ödülü")
		self.seviye = int(baslik)
		
	def SetOdul(self, aff1, affv1, aff2, affv2, aff3, affv3):
		self.af1.SetText("Max HP +"+str(affv1))
		self.af2.SetText("Savunma Deðeri +"+str(affv2))
		self.af3.SetText("Saldýrý Deðeri +"+str(affv3))

	def Show(self):
		ui.ScriptWindow.Show(self)

	def Close(self):
		self.Hide()

class BiologWnd(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.Initialize()
		self.LoadWindow()

	def Initialize(self):
		self.wndBioSlot = None
		self.tooltipItem = uiToolTip.ItemToolTip()
		self.tooltipItem.Hide()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "UIScript/afarbiowindow.py")
		except:
			import exception
			exception.Abort("BiologWnd.LoadWindow.LoadObject")
		try:
			self.titleBar = self.GetChild("titlebar")
			wndBioSlot = self.GetChild("BeltInventorySlot")
			self.counttext = self.GetChild("count")
			self.sure = self.GetChild("time")
			self.verbutton = self.GetChild("biobutton")

		except:
			import exception
			exception.Abort("BiologWnd.LoadWindow.BindObject")

		self.titleBar.SetCloseEvent(ui.__mem_func__(self.Close))
		self.verbutton.SetEvent(ui.__mem_func__(self.vergitsin))
		wndBioSlot.SetOverInItemEvent(ui.__mem_func__(self.OverInItem2))
		wndBioSlot.SetOverOutItemEvent(ui.__mem_func__(self.OverOutItem2))
		self.wndBioSlot = wndBioSlot

	def Destroy(self):
		self.ClearDictionary()

	def Open(self):
		self.SetCenterPosition()
		self.SetTop()
		self.Show()
		
	def vergitsin(self):
		net.SendChatPacket("/afarbio")
	
	def SetBiyolog(self, bioitem, verilen, toplam, kalansure):
		constInfo.gerekli = 0
		constInfo.kalan = 0
		constInfo.verilen = 0
		constInfo.toplam = 0
	
		constInfo.gerekli = int(bioitem)
		constInfo.kalan = int(kalansure)
		constInfo.verilen = int(verilen)
		constInfo.toplam = int(toplam)
		self.RefreshSlot()

	def OverInItem2(self):
		itemIndex = int(constInfo.gerekli)
		item.SelectItem(itemIndex)
		
		taslarr = [player.GetItemMetinSocket(999, i) for i in xrange(player.METIN_SOCKET_MAX_NUM)]
		efsunlarr = [player.GetItemAttribute(999, i) for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]

		self.tooltipItem.ClearToolTip()
		self.tooltipItem.AddRefineItemData(itemIndex, taslarr, efsunlarr)
		self.tooltipItem.Show()

	def OverOutItem2(self):
		self.tooltipItem.ClearToolTip()
		self.tooltipItem.Hide()

	def RefreshSlot(self):
		self.counttext.SetText(str(constInfo.verilen)+"/"+str(constInfo.toplam))
		getItemVNum= constInfo.gerekli

		if getItemVNum > 0:
			self.wndBioSlot.SetItemSlot(999, getItemVNum)
		else:	
			self.wndBioSlot.ClearSlot(999)
			return
		self.wndBioSlot.RefreshSlot()		
		
		
	def OnUpdate(self):
		self.RefreshSlot()
		import time
		current_milli_time = int(app.GetGlobalTimeStamp())
		if int(constInfo.kalan) == 0:
			self.sure.SetText("Verilebilir!")
		elif (int(constInfo.kalan)-current_milli_time <= 0):
			self.sure.SetText("Verilebilir!")
		else:
			self.sure.SetText(str(localeInfo.SecondToDHM(int(constInfo.kalan)-current_milli_time)))
			
		
	def Close(self):
		self.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True
