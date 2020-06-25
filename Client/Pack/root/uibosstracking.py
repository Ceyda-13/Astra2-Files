import nonplayer
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
import ime
import item
import wndMgr, uiCommon, ui, time, playerSettingModule, localeInfo, snd, mouseModule, constInfo, uiScriptLocale, dbg, uiToolTip
# from uiNewShop import DropDown

# ELEMENT_NAME_LIST	= [localeInfo.BOSS_TRACKING_ELEMENT_NAME1, localeInfo.BOSS_TRACKING_ELEMENT_NAME2, localeInfo.BOSS_TRACKING_ELEMENT_NAME3, localeInfo.BOSS_TRACKING_ELEMENT_NAME4, localeInfo.BOSS_TRACKING_ELEMENT_NAME5, localeInfo.BOSS_TRACKING_ELEMENT_NAME6]
MOB_VNUM_LIST = [1192, 2206, 691, 791, 1304, 2492, 2494, 2495, 2091, 3972, 2191, 2306, 1901, 3090, 3091, 3290, 3291, 3590, 3591, 3490, 3491, 3690, 3691, 3890, 3891, 3390, 3391, 3595, 3596, 3790, 3791, 3190, 3191, 6407, 2291, 878, 879, 880]

MAP_INDEX_LIST =  \
{
	MOB_VNUM_LIST[0] : 20,
	MOB_VNUM_LIST[1] : 8,
	MOB_VNUM_LIST[2] : 10,
	MOB_VNUM_LIST[3] : 11,
	MOB_VNUM_LIST[4] : 11,
	MOB_VNUM_LIST[5] : 3,
	MOB_VNUM_LIST[6] : 3,
	MOB_VNUM_LIST[7] : 3,
	MOB_VNUM_LIST[8] : 17,
	MOB_VNUM_LIST[9] : 9,
	MOB_VNUM_LIST[10] : 9,
	MOB_VNUM_LIST[11] : 14,
	MOB_VNUM_LIST[12] : 7,
	MOB_VNUM_LIST[13] : 33,
	MOB_VNUM_LIST[14] : 33,
	MOB_VNUM_LIST[15] : 33,
	MOB_VNUM_LIST[16] : 33,
	MOB_VNUM_LIST[17] : 33,
	MOB_VNUM_LIST[18] : 33,
	MOB_VNUM_LIST[19] : 35,
	MOB_VNUM_LIST[20] : 35,
	MOB_VNUM_LIST[21] : 35,
	MOB_VNUM_LIST[22] : 34,
	MOB_VNUM_LIST[23] : 34,
	MOB_VNUM_LIST[24] : 34,
	MOB_VNUM_LIST[25] : 34,
	MOB_VNUM_LIST[26] : 34,
	MOB_VNUM_LIST[27] : 36,
	MOB_VNUM_LIST[28] : 36,
	MOB_VNUM_LIST[29] : 36,
	MOB_VNUM_LIST[30] : 36,
	MOB_VNUM_LIST[31] : 36,
	MOB_VNUM_LIST[32] : 36,
	MOB_VNUM_LIST[33] : 39,
	MOB_VNUM_LIST[34] : 8,
	MOB_VNUM_LIST[35] : 40,
	MOB_VNUM_LIST[36] : 40,
	MOB_VNUM_LIST[37] : 40,
}

ELEMENT_LIST =  \
{
	MOB_VNUM_LIST[0] : localeInfo.BOSS_TRACKING_ELEMENT_NAME3, # buz elementi
	MOB_VNUM_LIST[1] : localeInfo.BOSS_TRACKING_ELEMENT_NAME2, # ateþ elementi
	MOB_VNUM_LIST[2] : localeInfo.BOSS_TRACKING_ELEMENT_NAME4,
	MOB_VNUM_LIST[3] : localeInfo.BOSS_TRACKING_ELEMENT_NAME4,
	MOB_VNUM_LIST[4] : localeInfo.BOSS_TRACKING_ELEMENT_NAME4,
	MOB_VNUM_LIST[5] : localeInfo.BOSS_TRACKING_ELEMENT_NAME8,
	MOB_VNUM_LIST[6] : localeInfo.BOSS_TRACKING_ELEMENT_NAME8,
	MOB_VNUM_LIST[7] : localeInfo.BOSS_TRACKING_ELEMENT_NAME8,
	MOB_VNUM_LIST[8] : localeInfo.BOSS_TRACKING_ELEMENT_NAME4,
	MOB_VNUM_LIST[9] : localeInfo.BOSS_TRACKING_ELEMENT_NAME5,
	MOB_VNUM_LIST[10] : localeInfo.BOSS_TRACKING_ELEMENT_NAME4,
	MOB_VNUM_LIST[11] : localeInfo.BOSS_TRACKING_ELEMENT_NAME5,
	MOB_VNUM_LIST[12] : localeInfo.BOSS_TRACKING_ELEMENT_NAME3,
	MOB_VNUM_LIST[13] : localeInfo.BOSS_TRACKING_ELEMENT_NAME5,
	MOB_VNUM_LIST[14] : localeInfo.BOSS_TRACKING_ELEMENT_NAME5,
	MOB_VNUM_LIST[15] : localeInfo.BOSS_TRACKING_ELEMENT_NAME4,
	MOB_VNUM_LIST[16] : localeInfo.BOSS_TRACKING_ELEMENT_NAME4,
	MOB_VNUM_LIST[17] : localeInfo.BOSS_TRACKING_ELEMENT_NAME2,
	MOB_VNUM_LIST[18] : localeInfo.BOSS_TRACKING_ELEMENT_NAME2,
	MOB_VNUM_LIST[19] : localeInfo.BOSS_TRACKING_ELEMENT_NAME8,
	MOB_VNUM_LIST[20] : localeInfo.BOSS_TRACKING_ELEMENT_NAME8,
	MOB_VNUM_LIST[21] : localeInfo.BOSS_TRACKING_ELEMENT_NAME3,
	MOB_VNUM_LIST[22] : localeInfo.BOSS_TRACKING_ELEMENT_NAME3,
	MOB_VNUM_LIST[23] : localeInfo.BOSS_TRACKING_ELEMENT_NAME5,
	MOB_VNUM_LIST[24] : localeInfo.BOSS_TRACKING_ELEMENT_NAME5,
	MOB_VNUM_LIST[25] : localeInfo.BOSS_TRACKING_ELEMENT_NAME8,
	MOB_VNUM_LIST[26] : localeInfo.BOSS_TRACKING_ELEMENT_NAME8,
	MOB_VNUM_LIST[27] : localeInfo.BOSS_TRACKING_ELEMENT_NAME2,
	MOB_VNUM_LIST[28] : localeInfo.BOSS_TRACKING_ELEMENT_NAME2,
	MOB_VNUM_LIST[29] : localeInfo.BOSS_TRACKING_ELEMENT_NAME5,
	MOB_VNUM_LIST[30] : localeInfo.BOSS_TRACKING_ELEMENT_NAME5,
	MOB_VNUM_LIST[31] : localeInfo.BOSS_TRACKING_ELEMENT_NAME5,
	MOB_VNUM_LIST[32] : localeInfo.BOSS_TRACKING_ELEMENT_NAME5,
	MOB_VNUM_LIST[33] : localeInfo.BOSS_TRACKING_ELEMENT_NAME4,
	MOB_VNUM_LIST[34] : localeInfo.BOSS_TRACKING_ELEMENT_NAME2,
	MOB_VNUM_LIST[35] : localeInfo.BOSS_TRACKING_ELEMENT_NAME4,
	MOB_VNUM_LIST[36] : localeInfo.BOSS_TRACKING_ELEMENT_NAME4,
	MOB_VNUM_LIST[37] : localeInfo.BOSS_TRACKING_ELEMENT_NAME4,
}

MOB_REBORN_TIME =  \
{
	MOB_VNUM_LIST[0] : 14400,# Buz cadýsý çýkýþ süresi 4 saat
	MOB_VNUM_LIST[1] : 900,# Alev Kral 15 dakika
	MOB_VNUM_LIST[2] : 1100,
	MOB_VNUM_LIST[3] : 7200,
	MOB_VNUM_LIST[4] : 7200,
	MOB_VNUM_LIST[5] : 1800,
	MOB_VNUM_LIST[6] : 1800,
	MOB_VNUM_LIST[7] : 1800,
	MOB_VNUM_LIST[8] : 1200,
	MOB_VNUM_LIST[9] : 21600,
	MOB_VNUM_LIST[10] : 1200,
	MOB_VNUM_LIST[11] : 1000,
	MOB_VNUM_LIST[12] : 1200,
	MOB_VNUM_LIST[13] : 1800,
	MOB_VNUM_LIST[14] : 1800,
	MOB_VNUM_LIST[15] : 1800,
	MOB_VNUM_LIST[16] : 1800,
	MOB_VNUM_LIST[17] : 1200,
	MOB_VNUM_LIST[18] : 2400,
	MOB_VNUM_LIST[19] : 1800,
	MOB_VNUM_LIST[20] : 1800,
	MOB_VNUM_LIST[21] : 1800,
	MOB_VNUM_LIST[22] : 1800,
	MOB_VNUM_LIST[23] : 1800,
	MOB_VNUM_LIST[24] : 1800,
	MOB_VNUM_LIST[25] : 1800,
	MOB_VNUM_LIST[26] : 1800,
	MOB_VNUM_LIST[27] : 1800,
	MOB_VNUM_LIST[28] : 1800,
	MOB_VNUM_LIST[29] : 1800,
	MOB_VNUM_LIST[30] : 1800,
	MOB_VNUM_LIST[31] : 1800,
	MOB_VNUM_LIST[32] : 1800,
	MOB_VNUM_LIST[33] : 600,
	MOB_VNUM_LIST[34] : 21600,
	MOB_VNUM_LIST[35] : 3600,
	MOB_VNUM_LIST[36] : 3600,
	MOB_VNUM_LIST[37] : 3600,
}

def SecondToHM(time):
	second = int(time % 60)
	minute = int((time / 60) % 60)
	hour = int((time / 60) / 60) % 24

	if hour <= 0:
		return "%d Dakika %02d Saniye" % (minute, second)
	else:
		return "%d Saat %02d Dakika %02d Saniye" % (hour, minute, second)

def GetRebornTime(time):
	second = int(time % 60)
	minute = int((time / 60) % 60)
	hour = int((time / 60) / 60) % 24
	
	if hour <= 0 and minute <= 0:
		return "%d saniye'de" % (second)
	elif hour <= 0:
		return "%d dakika'da" % (minute)
	else:
		return "%d saat'de" % (hour)

class DropDown(ui.Window):
	dropped  = 0
	dropstat = 0
	last = 0
	lastS = 0
	maxh = 95
	tt = ""
	OnChange = None
	class Item(ui.ListBoxEx.Item):
		def __init__(self,parent, text,value=0):
			ui.ListBoxEx.Item.__init__(self)

			self.textBox=ui.TextLine()
			self.textBox.SetParent(self)
			self.textBox.SetText(text)
			# self.textBox.SetLimitWidth(parent.GetWidth()-132)
			self.textBox.Show()
			self.value = value
		def GetValue(self):
			return self.value
		def __del__(self):
			ui.ListBoxEx.Item.__del__(self)
			
	def __init__(self,parent,tt = "",down=1):
		ui.Window.__init__(self,"TOP_MOST")
		self.tt=tt
		self.down = down
		self.SetParentProxy(parent)
		self.bg = ui.Bar("TOP_MOST")
		self.bg.SetParent(self)
		self.bg.SetPosition(0,0)
		self.bg.SetColor(0xc0000000)
		self.bg.OnMouseOverIn = self.bgMouseIn
		self.bg.OnMouseOverOut = self.bgMouseOut
		self.bg.OnMouseLeftButtonDown = self.ExpandMe
		self.bg.Show()
		self.act = ui.TextLine()
		self.act.SetParent(self.bg)
		self.act.SetPosition(4,2)
		self.act.SetText(self.tt)
		self.act.Show()
		self.GetText = self.act.GetText
		
		self.Drop = ui.Bar("TOP_MOST")
		self.Drop.SetParent(self.GetParentProxy())
		self.Drop.SetPosition(0,21)
		# self.Drop.SetSize(150,95)
		self.Drop.SetSize(150,0)
		# self.Drop.SetColor(0xc00a0a0a)
		self.Drop.SetColor(0xff0a0a0a)
		
		
		self.ScrollBar = ui.ThinScrollBar()
		self.ScrollBar.SetParent(self.Drop)
		self.ScrollBar.SetPosition(132,0)
		# self.ScrollBar.SetScrollBarSize(95)
		self.ScrollBar.SetScrollBarSize(0)
		# self.ScrollBar.Show()
		
		self.DropList = ui.ListBoxEx()
		self.DropList.SetParent(self.Drop)
		self.DropList.itemHeight = 12
		self.DropList.itemStep = 13
		self.DropList.SetPosition(0,0)
		# self.DropList.SetSize(132,self.maxh)
		self.DropList.SetSize(132,13) 
		self.DropList.SetScrollBar(self.ScrollBar)
		self.DropList.SetSelectEvent(self.SetTitle)
		self.DropList.SetViewItemCount(0)
		self.DropList.Show()
		if self.tt != "":
			self.AppendItemAndSelect(self.tt)
		self.selected = self.DropList.GetSelectedItem()
		
			
		self.SetSize(120,20)
	def __del__(self): 
		ui.Window.__del__(self)
	c = 1
	def AppendItem(self,text,value=0):
		self.c+=1   
		self.DropList.AppendItem(self.Item(self,text,value))
		self.maxh = min(95,13*self.c)
		if self.c > 2:
			self.ScrollBar.Show()
			
		
	def AppendItemAndSelect(self,text,value=0):
		self.DropList.AppendItem(self.Item(self,text,value))
		self.DropList.SelectIndex(len(self.DropList.itemList)-1)
		
		
	def ClearItems(self):
		self.DropList.RemoveAllItems()
		self.AppendItemAndSelect(self.tt)
		self.act.SetText(self.tt)
		
	def Clear(self):
		self.DropList.SelectIndex(0)
		self.ScrollBar.SetScrollBarSize(0)
		self.c = 1
		
	def SelectByAffectId(self,id):
		for x in self.DropList.itemList:
			if x.value == id:
				self.DropList.SelectItem(x)
				break
				
	def SetTitle(self,item):
		self.act.SetText(str(item.textBox.GetText()))
		self.last = self.DropList.basePos
		self.lastS = self.ScrollBar.GetPos()
		self.dropped = 0
		self.selected = item
		if self.OnChange:
			self.OnChange()
		# self.Drop.Hide()
		
	def SetPosition(self,w,h):
		ui.Window.SetPosition(self,w,h)
		if self.down == 1:
			self.Drop.SetPosition(w,h+21)
		else:
			self.Drop.SetPosition(w,h-self.Drop.GetHeight())
		
	def SetSize(self,w,h):
		ui.Window.SetSize(self,w,h)
		self.bg.SetSize(w,h)
		self.Drop.SetSize(w,0)
		self.DropList.SetSize(w-18,self.maxh)
		for x in self.DropList.itemList:
			x.SetSize(w-18,12)
		self.ScrollBar.SetPosition(w-18,0)
		
		
	def ExpandMe(self):
		if self.dropped == 1:
			# self.Drop.Hide()
			self.dropped = 0
		else:
			# self.Drop.Show()
			self.dropped = 1
			
	def OnUpdate(self):
		iter = 6
		if self.Drop.GetHeight() < 50:
			self.ScrollBar.Hide()
		else:
			self.ScrollBar.Show()
			
		if self.dropped == 0 and self.dropstat == 1:
			if self.Drop.GetHeight() <=0:
				self.dropstat = 0
				self.Drop.SetSize(self.Drop.GetWidth(),0)
				self.ScrollBar.SetScrollBarSize(self.Drop.GetHeight())
				self.Drop.Hide()
			else:
				if self.Drop.GetHeight()-iter < 0:
					self.Drop.SetSize(self.Drop.GetWidth(),0)
				else:
					self.Drop.SetSize(self.Drop.GetWidth(),self.Drop.GetHeight()-iter)
					(w,h) = self.GetLocalPosition()
					self.SetPosition(w,h)
						
					
				self.ScrollBar.SetScrollBarSize(self.Drop.GetHeight())
			self.DropList.SetViewItemCount(int(self.Drop.GetHeight()/13))
			self.DropList.SetBasePos(self.last+1)
			self.DropList.SetBasePos(self.last)
		elif self.dropped == 1 and self.dropstat == 0:
			self.Drop.Show()
			self.SetTop()
			if self.Drop.GetHeight() >=self.maxh:
				self.Drop.SetSize(self.Drop.GetWidth(),self.maxh)
				self.ScrollBar.SetScrollBarSize(self.maxh)
				self.dropstat = 1
				self.DropList.SetViewItemCount(7)
				self.ScrollBar.SetPos(self.lastS)
			else:
				self.ScrollBar.SetScrollBarSize(self.Drop.GetHeight()+iter)
				self.Drop.SetSize(self.Drop.GetWidth(),self.Drop.GetHeight()+iter)
				(w,h) = self.GetLocalPosition()
				self.SetPosition(w,h)
			self.DropList.SetViewItemCount(int(self.Drop.GetHeight()/13))
			self.DropList.SetBasePos(self.last+1)
			self.DropList.SetBasePos(self.last)
		
	## BG Hover
	def bgMouseIn(self):
		self.bg.SetColor(0xc00a0a0a)
	def bgMouseOut(self):
		self.bg.SetColor(0xc0000000)

class BossTrackingInfoWindow(ui.ScriptWindow):
	SELECT_MOB_TIMER = 0
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.data = []
		self.__LoadWindow()
		
	def __del__(self):
		ui.ScriptWindow.__del__(self)
		

	def Destroy(self):
		self.Close()
		self.ClearDictionary()
		
	def __LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "UIScript/BossTrackingInfoWindow.py")
		except:
			import exception
			exception.Abort("BossTrackingInfoWindow.__LoadWindow.LoadScriptFile")
			
		try:
			self.backGround 		= self.GetChild("Background")
			self.avatarImage		= self.GetChild("AvatarImage")
			self.info1				= self.GetChild("Info1")
			#self.info2				= self.GetChild("Info2")
			self.respawnInfos		= [self.GetChild("CH%dRespawnInfo" % int(i+1)) for i in xrange(6)]
			self.channelInfo		= self.GetChild("ChannelInfo")
			self.elementInfo		= self.GetChild("ElementInfo")
			self.mapInfo			= self.GetChild("MapInfo")
			self.mobLevelInfo		= self.GetChild("MobLevelInfo")
			self.btnToolTip				= self.GetChild("ToolTipButton")
		except:
			import exception
			exception.Abort("BossTrackingInfoWindow.__LoadWindow.BindObject")
			
		try:
			self.dropDown		= DropDown(self.backGround, "Bir patron seçiniz. ")
			self.dropDown.SetPosition(136, 14)
			self.dropDown.SetSize(137, 21)
			self.dropDown.SetTop()
			self.dropDown.OnChange = self.__OnDropdownChange
			self.dropDown.ClearItems()
			self.dropDown.Show()
			self.GetChild("board").SetCloseEvent(ui.__mem_func__(self.ButtonCloseEvent))
			self.btnToolTip.SetToolTipWindow(self.__CreateToolTip(localeInfo.BOSS_TRACKING_INFO_WINDOW_TITLE, [localeInfo.BOSS_TRACKING_INFO_WINDOW_TOOLTIP1, localeInfo.BOSS_TRACKING_INFO_WINDOW_TOOLTIP2, localeInfo.BOSS_TRACKING_INFO_WINDOW_TOOLTIP3]))			
		except:		
			import exception
			exception.Abort("BossTrackingInfoWindow.__LoadWindow.LoadingObject")
			
	def __CreateToolTip(self, title, descList):
		toolTip = uiToolTip.ToolTip()
		toolTip.SetTitle(title)
		toolTip.AppendSpace(5)

		for desc in descList:
			toolTip.AutoAppendTextLine(desc)

		toolTip.AlignHorizonalCenter()
		return toolTip			
			
	def __OnDropdownChange(self):
		if app.GetTime() > self.SELECT_MOB_TIMER:
			if (0 == self.dropDown.DropList.GetSelectedItem().value):
				self.ClearGui()
				
			self.data = []
			net.SendChatPacket("/open_boss_tracking %d" % int(self.dropDown.DropList.GetSelectedItem().value))
			#self.SELECT_MOB_TIMER = app.GetTime() + 2
		else:
			NewGetTime = self.SELECT_MOB_TIMER - app.GetTime()
			chat.AppendChat(chat.CHAT_TYPE_INFO, "Tekrar seçim yapabilmek için %d saniye beklemelisin." % (NewGetTime))
			
	def SetData(self, b_kill_time, b_start_time, b_channel, b_mob_vnum, map_index):
		data_tuple = (b_kill_time,b_start_time,b_channel,b_mob_vnum, map_index)
		self.data.append(data_tuple)
		
	def ConvertToTimeInfo(self):
		(arg1, arg2, arg3, arg4, arg5) = self.data[0]
		if (int(arg2) <= 0):
			convert_time = GetRebornTime(int(MOB_REBORN_TIME[int(arg4)]))
		else:
			convert_time = GetRebornTime(int(arg2))

		return convert_time

	def GetMapIndex(self):
		(arg1, arg2, arg3, arg4, arg5) = self.data[0]
		if (int(arg5) <= 0):
			return MAP_INDEX_LIST[int(arg4)]
		else:
			return int(arg5)
		
	def GetElementName(self, arg4):
		return ELEMENT_LIST[int(arg4)]
		
	def ClearGui(self):
		self.avatarImage.LoadImage("d:/ymir work/ui/boss_follow/none.tga")
		self.data = []
		self.mapInfo.SetText("Patron seçimi yapýlmadý.")
		#self.info2.SetText("Patron seçimi yapýlmadý.")
		self.info1.SetText("Patron seçimi yapýlmadý.")
		self.mobLevelInfo.SetText("Patron seçimi yapýlmadý.")
		self.channelInfo.SetText("Patron seçimi yapýlmadý.")
		self.elementInfo.SetText("Patron seçimi yapýlmadý.")
		for i in xrange(6):
			self.respawnInfos[i-1].SetText("Bir patron seçiniz.")

	def OnUpdate(self):
		if int(len(self.data)) != 0:
			for key in xrange(len(self.data)):
				(arg1, arg2, arg3, arg4, arg5) = self.data[key]
				format_time = (int(arg2)+int(arg1))-app.GetGlobalTimeStamp()
	
				CHANNEL_COUNT = 4
	
				if CHANNEL_COUNT < 5:
					self.respawnInfos[6-1].SetText("CH6: Bu kanal devredýþý.")
					self.respawnInfos[5-1].SetText("CH5: Bu kanal devredýþý.")
				
				if (int(arg4) == self.dropDown.DropList.GetSelectedItem().value):
					self.avatarImage.LoadImage("d:/ymir work/ui/boss_follow/%d.tga" % (arg4))
				if int(arg1) == 0:
					self.respawnInfos[arg3-1].SetText("CH%d: Boss þuanda yaþýyor" % (int(arg3)))
				elif (format_time < 0):
					self.respawnInfos[arg3-1].SetText("CH%d: Boss þuanda yaþýyor" % (int(arg3)))
				else:
					self.respawnInfos[arg3-1].SetText("CH%d: %s" % (int(arg3), SecondToHM(format_time)))
					
				if CHANNEL_COUNT < 5:
					self.respawnInfos[6-1].SetText("CH6: Bu kanal devredýþý.")
					self.respawnInfos[5-1].SetText("CH5: Bu kanal devredýþý.")
			
				self.mapInfo.SetText(localeInfo.MINIMAP_ZONE_NAME_DICT_BY_IDX[int(self.GetMapIndex())])
				self.info1.SetText(localeInfo.BOSS_TRACKING_INFO_REBORN_TIME % (nonplayer.GetMonsterName(int(arg4)), self.ConvertToTimeInfo()))
				self.mobLevelInfo.SetText(localeInfo.BOSS_TRACKING_INFO_LEVEL % (nonplayer.GetMonsterLevel(int(arg4))))
				self.channelInfo.SetText(uiScriptLocale.BOSS_TRACKING_INFO_CHANNEL)
				self.elementInfo.SetText(str(self.GetElementName(int(arg4))))

	def Close(self):
		self.data = []
		self.dropDown.ClearItems()
		self.dropDown.Clear()
		self.Hide()
		
	def ButtonCloseEvent(self):
		net.SendChatPacket("/close_boss_tracking")
		self.data = []
		self.dropDown.ClearItems()
		self.dropDown.Clear()
		self.Hide()
		
	def OnPressEscapeKey(self):
		self.ButtonCloseEvent()
		return True
		
	def Open(self):
		self.ClearGui()
		self.dropDown.ClearItems()
		self.dropDown.Clear()
		for i in xrange(len(MOB_VNUM_LIST)):
			self.dropDown.AppendItem(nonplayer.GetMonsterName(int(MOB_VNUM_LIST[i])), int(MOB_VNUM_LIST[i]))
		self.SetCenterPosition()
		self.Show()
		self.SetTop()

