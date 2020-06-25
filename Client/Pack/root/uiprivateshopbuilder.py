import ui
import snd
import shop
import mouseModule
import player
import chr
import net
import uiCommon
import localeInfo
import chat
import item
import systemSetting #±èÁØÈ£
import player #±èÁØÈ£
import __builtin__ as selfs
if ENABLE_OFFLINE_PRIVATE_SHOP:
	import constInfo
	import uiPrivateShopEditor
	import chat

g_isBuildingPrivateShop = False

g_itemPriceDict={}

g_privateShopAdvertisementBoardDict={}

def Clear():
	global g_itemPriceDict
	global g_isBuildingPrivateShop
	g_itemPriceDict={}
	g_isBuildingPrivateShop = False
	# @@@correction007 BEGIN
	global g_privateShopAdvertisementBoardDict
	g_privateShopAdvertisementBoardDict={}
	# @@@correction007 END

def IsPrivateShopItemPriceList():
	global g_itemPriceDict
	if g_itemPriceDict:
		return True
	else:
		return False

def IsBuildingPrivateShop():
	global g_isBuildingPrivateShop
	if player.IsOpenPrivateShop() or g_isBuildingPrivateShop:
		return True
	else:
		return False

def SetPrivateShopItemPrice(itemVNum, itemPrice):
	global g_itemPriceDict
	g_itemPriceDict[int(itemVNum)]=itemPrice

def GetPrivateShopItemPrice(itemVNum):
	try:
		global g_itemPriceDict
		return g_itemPriceDict[itemVNum]
	except KeyError:
		return 0

def UpdateADBoard():
	for key in g_privateShopAdvertisementBoardDict.keys():
		g_privateShopAdvertisementBoardDict[key].Show()

def DeleteADBoard(vid):
	if not g_privateShopAdvertisementBoardDict.has_key(vid):
		return

	del g_privateShopAdvertisementBoardDict[vid]


class PrivateShopAdvertisementBoard(ui.ThinBoard):
	def __init__(self):
		ui.ThinBoard.__init__(self, "UI_BOTTOM")
		self.vid = None
		self.__MakeTextLine()

	def __del__(self):
		ui.ThinBoard.__del__(self)

	def __MakeTextLine(self):
		self.textLine = ui.TextLine()
		self.textLine.SetParent(self)
		self.textLine.SetWindowHorizontalAlignCenter()
		self.textLine.SetWindowVerticalAlignCenter()
		self.textLine.SetHorizontalAlignCenter()
		self.textLine.SetVerticalAlignCenter()
		self.textLine.Show()

	def Open(self, vid, text):
		self.vid = vid

		self.textLine.SetText(text)
		self.textLine.UpdateRect()
		self.SetSize(len(text)*6 + 10*2, 20)
		self.Show()

		g_privateShopAdvertisementBoardDict[vid] = self

	def OnMouseLeftButtonUp(self):
		if not self.vid:
			return
		net.SendOnClickPacket(self.vid)

		return True

	def OnUpdate(self):
		if not self.vid:
			return

		if systemSetting.IsShowSalesText():
			self.Show()
			(x, y) = chr.GetProjectPosition(self.vid, 220)
			self.SetPosition(x - self.GetWidth() / 2, y - self.GetHeight() / 2)
		else:
			for key in g_privateShopAdvertisementBoardDict.keys():
				if player.GetMainCharacterIndex() == key:
					g_privateShopAdvertisementBoardDict[key].Show()
					x, y = chr.GetProjectPosition(player.GetMainCharacterIndex(), 220)
					g_privateShopAdvertisementBoardDict[key].SetPosition(x - self.GetWidth()/2, y - self.GetHeight()/2)
				else:
					g_privateShopAdvertisementBoardDict[key].Hide()

class PrivateShopBuilder(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)

		self.itemStock = {}
		self.priceInputBoard = None
		if not ENABLE_OFFLINE_PRIVATE_SHOP:
			self.title = ""
		else:
			self.time_select_pivot_window = 0
			self.cur_time_text = 0
			self.time_select_button = 0
			self.time_bar = 0
			self.time_listbox = 0
			self.time_scrollbar = 0
			self.Page1Button = None
			self.Page2Button = None
			self.page = 0
			self.list = [None] * 180
			self.TitleType = 0
			self.PolyVnum = 30000

		self.__LoadWindow()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "UIScript/PrivateShopBuilder.py")
		except:
			import exception
			exception.Abort("PrivateShopBuilderWindow.LoadWindow.LoadObject")

		try:
			GetObject = self.GetChild
			self.nameLine = GetObject("NameLine")
			self.itemSlot = GetObject("ItemSlot")
			self.btnOk = GetObject("OkButton")
			self.btnClose = GetObject("CloseButton")
			self.titleBar = GetObject("TitleBar")
			if ENABLE_OFFLINE_PRIVATE_SHOP:
				self.positionInfo = GetObject("Position")
				self.time_select_pivot_window = GetObject("time_select_pivot_window")
				self.cur_time_text = GetObject("cur_time_text")
				self.time_select_button = GetObject("time_select_button")
				self.Page1Button = GetObject("Page1Button")
				self.Page2Button = GetObject("Page2Button")
		except:
			import exception
			exception.Abort("PrivateShopBuilderWindow.LoadWindow.BindObject")

		self.btnOk.SetEvent(ui.__mem_func__(self.OnOk))
		self.btnClose.SetEvent(ui.__mem_func__(self.OnClose))
		self.titleBar.SetCloseEvent(ui.__mem_func__(self.OnClose))

		self.itemSlot.SetSelectEmptySlotEvent(ui.__mem_func__(self.OnSelectEmptySlot))
		self.itemSlot.SetSelectItemSlotEvent(ui.__mem_func__(self.OnSelectItemSlot))
		self.itemSlot.SetOverInItemEvent(ui.__mem_func__(self.OnOverInItem))
		self.itemSlot.SetOverOutItemEvent(ui.__mem_func__(self.OnOverOutItem))

		if ENABLE_OFFLINE_PRIVATE_SHOP:
			self.Page1Button.SetEvent(ui.__mem_func__(self.SetPage), 0)
			self.Page2Button.SetEvent(ui.__mem_func__(self.SetPage), 1)
			self.Page1Button.Down()
			import uiScriptLocale
			self.cur_time_text.SetText(localeInfo.TOOLTIP_ITEM_NORMAL + " " + uiScriptLocale.SHOP_TITLE)

			self.time_bar = ui.Bar()
			x, y = self.time_select_pivot_window.GetGlobalPosition()
			self.time_bar.SetPosition(x, y)
			self.time_bar.SetSize(159, 94)
			self.time_bar.SetColor(0xFF000000)
			self.time_bar.AddFlag("float")
			self.time_bar.Hide()

			self.time_listbox = ui.ListBoxEx()
			self.time_listbox.SetParent(self.time_bar)
			self.time_listbox.SetPosition(0, 0)
			self.time_listbox.SetSize(159, 94)
			self.time_listbox.SetViewItemCount(5)
			for i in xrange(len(constInfo.OfflinePrivateShopTimeList)):
				if not i:
					self.time_listbox.AppendItem(uiPrivateShopEditor.Item(localeInfo.TOOLTIP_ITEM_NORMAL + " " + uiScriptLocale.SHOP_TITLE))
				else:
					self.time_listbox.AppendItem(uiPrivateShopEditor.Item("%d " % constInfo.OfflinePrivateShopTimeList[i] + uiScriptLocale.PET_INFORMATION_LIFE_TIME))

			self.time_listbox.SetSelectEvent(ui.__mem_func__(self.__SelectTime))
			self.time_listbox.Show()

			self.time_scrollbar = ui.ScrollBar()
			self.time_scrollbar.SetParent(self.time_listbox)
			self.time_scrollbar.SetScrollBarSize(94)
			self.time_scrollbar.SetPosition(144, 0)
			self.time_scrollbar.Show()
			self.time_listbox.SetScrollBar(self.time_scrollbar)

			self.time_select_button.SAFE_SetEvent(self.OnTimeSelect)

	def Destroy(self):
		self.ClearDictionary()

		self.nameLine = None
		self.itemSlot = None
		self.btnOk = None
		self.btnClose = None
		self.titleBar = None
		self.priceInputBoard = None
		if ENABLE_OFFLINE_PRIVATE_SHOP:
			self.time_select_pivot_window = 0
			self.cur_time_text = 0
			self.time_select_button = 0
			self.time_bar = 0
			self.time_listbox = 0
			self.time_scrollbar = 0
			self.Page1Button = None
			self.Page2Button = None
			self.page = 0
			self.list = []
			self.TitleType = 0
			self.PolyVnum = 30000

	def Open(self, title, TitleType, PolyVnum):
		if not ENABLE_OFFLINE_PRIVATE_SHOP:
			self.title = title
			if len(title) > 25:
				title = title[:22] + "..."

			self.nameLine.SetText(title)
		else:
			self.TitleType = TitleType
			self.PolyVnum = PolyVnum

		self.itemStock = {}
		shop.ClearPrivateShopStock()
		self.SetCenterPosition()
		self.Refresh()
		self.Show()

		global g_isBuildingPrivateShop
		g_isBuildingPrivateShop = True
		if WJ_ENABLE_TRADABLE_ICON:
			if selfs.wndInventory:
				selfs.wndInventory.RefreshItemSlot()

	def Close(self):
		global g_isBuildingPrivateShop
		g_isBuildingPrivateShop = False

		if not ENABLE_OFFLINE_PRIVATE_SHOP:
			self.title = ""

		self.itemStock = {}
		shop.ClearPrivateShopStock()
		self.Hide()

		if ENABLE_OFFLINE_PRIVATE_SHOP:
			self.time_bar.Hide()

		if WJ_ENABLE_TRADABLE_ICON:
			for i in xrange(shop.SHOP_PAGE_SIZE):
				if self.itemStock.has_key(i):
					invenType, invenPos = self.itemStock[i]
					self.SetCanMouseEventSlot(invenType, invenPos)

			if selfs.wndInventory:
				selfs.wndInventory.RefreshItemSlot()

	def Refresh(self):
		getitemVNum=player.GetItemIndex
		getItemCount=player.GetItemCount
		setitemVNum=self.itemSlot.SetItemSlot
		delItem=self.itemSlot.ClearSlot
		if ENABLE_OFFLINE_PRIVATE_SHOP:
			asd = 0
			z = 1
			j = asd / 45
			if self.page and j:
				if j == 1:
					z = 0
				elif j == 3:
					z = 2
				asd = asd % 45
			self.list = [None] * 180

		for i in xrange(shop.SHOP_PAGE_SIZE):
			slotNumber = self.page * shop.SHOP_PAGE_SIZE + i
			if ENABLE_OFFLINE_PRIVATE_SHOP:
				y = i / 10
				x = i % 10
				y2 = asd / 5
				x2 = asd % 5
				var = (y * 10) + (z * 5) + self.page * shop.SHOP_PAGE_SIZE
				if y2 > y and z:
					var += y2 * 10

				if y2 > y and not z:
					var += 5

				if y2 == y:
					var += x2

				if slotNumber >= var or (self.page and z and not j):
					btn = ui.Button()
					btn.SetParent(self.itemSlot)
					btn.SetUpVisual("d:/ymir work/ui/game/shopsearchp2p/padlock.dds")
					btn.SetOverVisual("d:/ymir work/ui/game/shopsearchp2p/padlock.dds")
					btn.SetDownVisual("d:/ymir work/ui/game/shopsearchp2p/padlock.dds")
					btn.SetPosition((x) * 32, (y) * 32)
					# btn.SetEvent(lambda : self.DeleteAll())
					btn.Show()
					self.list[slotNumber] = (btn)

			slot = self.itemSlot
			# TraceError("%d - %d" % (i, slotNumber))

			if not self.itemStock.has_key(slotNumber):
				delItem(i)
				continue

			pos = self.itemStock[slotNumber]

			itemCount = getItemCount(*pos)
			if itemCount <= 1:
				itemCount = 0
			setitemVNum(i, getitemVNum(*pos), itemCount)

			# if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
				# itemTransmutedVnum = player.GetItemChangeLook(invenType, invenPos)
				# if itemTransmutedVnum:
					# slot.DisableCoverButton(i)
				# else:
					# slot.EnableCoverButton(i)

		self.itemSlot.RefreshSlot()

	def OnSelectEmptySlot(self, selectedSlotPos):
		selectedSlotPos += self.page * shop.SHOP_PAGE_SIZE
		isAttached = mouseModule.mouseController.isAttached()
		if isAttached:
			attachedSlotType = mouseModule.mouseController.GetAttachedType()
			attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
			mouseModule.mouseController.DeattachObject()

			if player.SLOT_TYPE_INVENTORY != attachedSlotType and player.SLOT_TYPE_DRAGON_SOUL_INVENTORY != attachedSlotType:
				return
			attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)

			itemVNum = player.GetItemIndex(attachedInvenType, attachedSlotPos)
			item.SelectItem(itemVNum)

			if item.IsAntiFlag(item.ANTIFLAG_GIVE) or item.IsAntiFlag(item.ANTIFLAG_MYSHOP):
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.PRIVATE_SHOP_CANNOT_SELL_ITEM)
				return

			if ENABLE_OFFLINE_PRIVATE_SHOP:
				(width, height) = item.GetItemSize()
				for i in xrange(height):
					if self.list[selectedSlotPos + (height - 1) * 10]:
						return

			if WJ_ENABLE_TRADABLE_ICON:
				self.SetCanMouseEventSlot(attachedInvenType, attachedSlotPos)

			priceInputBoard = uiCommon.MoneyInputDialog()
			priceInputBoard.SetTitle(localeInfo.PRIVATE_SHOP_INPUT_PRICE_DIALOG_TITLE)
			priceInputBoard.SetAcceptEvent(ui.__mem_func__(self.AcceptInputPrice))
			priceInputBoard.SetCancelEvent(ui.__mem_func__(self.CancelInputPrice))
			if ENABLE_OFFLINE_PRIVATE_SHOP and constInfo.PriceCheck.has_key(itemVNum):
				(min, max) = constInfo.PriceCheck[itemVNum]
				priceInputBoard.SetCheckPrices(min, max)

			priceInputBoard.Open()

			itemPrice=GetPrivateShopItemPrice(itemVNum)
			if itemPrice>0:
				priceInputBoard.SetValue(itemPrice)

			self.priceInputBoard = priceInputBoard
			self.priceInputBoard.itemVNum = itemVNum
			self.priceInputBoard.sourceWindowType = attachedInvenType
			self.priceInputBoard.sourceSlotPos = attachedSlotPos
			self.priceInputBoard.targetSlotPos = selectedSlotPos

	def OnSelectItemSlot(self, selectedSlotPos):
		selectedSlotPos += self.page * shop.SHOP_PAGE_SIZE
		isAttached = mouseModule.mouseController.isAttached()
		if isAttached:
			snd.PlaySound("sound/ui/loginfail.wav")
			mouseModule.mouseController.DeattachObject()

		else:
			if not selectedSlotPos in self.itemStock:
				return

			invenType, invenPos = self.itemStock[selectedSlotPos]
			shop.DelPrivateShopItemStock(invenType, invenPos)
			snd.PlaySound("sound/ui/drop.wav")

			del self.itemStock[selectedSlotPos]

			if WJ_ENABLE_TRADABLE_ICON:
				self.SetCanMouseEventSlot(invenType, invenPos)

			self.Refresh()

	def AcceptInputPrice(self):

		if not self.priceInputBoard:
			return True

		text = self.priceInputBoard.GetText()
		if ENABLE_MONEY_SHORTING:
			text = text.replace("k", "000")

		if not text:
			return True

		if not text.isdigit():
			return True

		if int(text) <= 0:
			return True

		attachedInvenType = self.priceInputBoard.sourceWindowType
		sourceSlotPos = self.priceInputBoard.sourceSlotPos
		targetSlotPos = self.priceInputBoard.targetSlotPos

		for privatePos, (itemWindowType, itemSlotIndex) in self.itemStock.items():
			if itemWindowType == attachedInvenType and itemSlotIndex == sourceSlotPos:
				shop.DelPrivateShopItemStock(itemWindowType, itemSlotIndex)
				del self.itemStock[privatePos]

		if ENABLE_MONEY_SHORTING:
			price = long(text)
		else:
			price = int(self.priceInputBoard.GetText())

		if ENABLE_OFFLINE_PRIVATE_SHOP and constInfo.PriceCheck.has_key(self.priceInputBoard.itemVNum):
			(min, max) = constInfo.PriceCheck[self.priceInputBoard.itemVNum]
			count = player.GetItemCount(sourceSlotPos)
			fprice = float(float(price) / float(count))
			if fprice > float(max) or fprice < float(min):
				chat.AppendChat(chat.CHAT_TYPE_INFO, "Bu Fiyata Ekliyemezsin !")
				return

		if IsPrivateShopItemPriceList():
			SetPrivateShopItemPrice(self.priceInputBoard.itemVNum, price)

		shop.AddPrivateShopItemStock(attachedInvenType, sourceSlotPos, targetSlotPos, price)
		self.itemStock[targetSlotPos] = (attachedInvenType, sourceSlotPos)

		if WJ_ENABLE_TRADABLE_ICON:
			self.SetCantMouseEventSlot(attachedInvenType, sourceSlotPos)

		snd.PlaySound("sound/ui/drop.wav")

		self.Refresh()

		#####

		self.priceInputBoard = None
		return True

	def CancelInputPrice(self):
		self.priceInputBoard = None
		return True

	def OnOk(self):
		if not ENABLE_OFFLINE_PRIVATE_SHOP and not self.title:
			return
		else:
			if not len(self.nameLine.GetText()):
				return

		if 0 == len(self.itemStock):
			return

		if ENABLE_OFFLINE_PRIVATE_SHOP:
			shop.BuildPrivateShop(self.nameLine.GetText(), 3600 * constInfo.OfflinePrivateShopTimeList[self.time_listbox.GetItemIndex(self.time_listbox.GetSelectedItem())], self.TitleType, self.PolyVnum)
		else:
			shop.BuildPrivateShop(self.title)

		self.Close()

	def OnClose(self):
		self.Close()

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def OnOverInItem(self, slotIndex):
		slotIndex += self.page * shop.SHOP_PAGE_SIZE
		if selfs.wndToolTipItem:
			if self.itemStock.has_key(slotIndex):
				selfs.wndToolTipItem.SetPrivateShopBuilderItem(*self.itemStock[slotIndex] + (slotIndex,))

	def OnOverOutItem(self):
		if selfs.wndToolTipItem:
			selfs.wndToolTipItem.HideToolTip()

	def OnUpdate(self):
		if WJ_ENABLE_TRADABLE_ICON:
			for i in xrange(shop.SHOP_PAGE_SIZE):
				if self.itemStock.has_key(i):
					invenType, invenPos = self.itemStock[i]
					self.SetCantMouseEventSlot(invenType, invenPos)

		if ENABLE_OFFLINE_PRIVATE_SHOP:
			(x, y, z) = player.GetMainCharacterPosition()
			self.positionInfo.SetText("%s - (%.0f, %.0f)" % (localeInfo.MINIMAP_ZONE_NAME_DICT[selfs.wndMiniMap.GetMapName()], x/100, y/100))

	def OnMoveWindow(self, x, y):
		if ENABLE_OFFLINE_PRIVATE_SHOP:
			 if self.time_bar:
				self.BarAdjustPositionAndSize()

	if ENABLE_OFFLINE_PRIVATE_SHOP:
		def BarAdjustPositionAndSize(self):
			x, y = self.time_select_pivot_window.GetGlobalPosition()
			self.time_bar.SetPosition(x, y)

		def __SelectTime(self):
			self.cur_time_text.SetText(self.time_listbox.GetSelectedItem().GetText())
			if self.time_bar:
				self.time_bar.Hide()

		def OnTimeSelect(self):
			x, y = self.time_select_pivot_window.GetGlobalPosition()
			self.time_bar.SetPosition(x, y)
			if self.time_bar:
				if self.time_bar.IsShow():
					self.time_bar.Hide()
				else:
					self.time_bar.Show()

		def SetPage(self, i):
			self.page = i
			if i:
				self.Page1Button.SetUp()
			else:
				self.Page2Button.SetUp()

			self.Refresh()

	if WJ_ENABLE_TRADABLE_ICON:
		def SetCantMouseEventSlot(self, inven_type, idx):
			if inven_type == player.INVENTORY:
				selfs.wndInventory.SetCantMouseEventSlot(idx)

		def SetCanMouseEventSlot(self, inven_type, idx):
			if inven_type == player.INVENTORY:
				selfs.wndInventory.SetCanMouseEventSlot(idx)


if ENABLE_OFFLINE_PRIVATE_SHOP:
	class PrivateShopTitleBar(ui.ShopDecoTitle):
		def __init__(self, type):
			ui.ShopDecoTitle.__init__(self, type-1, "UI_BOTTOM")
				
			self.Show()
			
			self.type = type
			self.vid = None
			self.__MakeTextLine()
			self.__CalcFontColor()

		def __del__(self):
			ui.ShopDecoTitle.__del__(self)
			self.textLine = None
			
		def __CalcFontColor(self) :	
			if self.type != 0 :
				import uiMyShopDecoration
				COLOR = int(uiMyShopDecoration.DECO_SHOP_TITLE_LIST[self.type-1][2], 16)
				
				RED		= (COLOR & 0xFF0000) >> 16
				GREEN	= (COLOR & 0x00FF00) >> 8
				BLUE	= COLOR & 0x0000FF
				
				self.textLine.SetFontColor(float(RED)/255.0, float(GREEN)/255.0, float(BLUE)/255.0)

		def __MakeTextLine(self):
			self.textLine = ui.TextLine()
			# self.textLine.DisableEnterToken()
			self.textLine.SetParent(self)
			self.textLine.SetWindowHorizontalAlignCenter()
			self.textLine.SetWindowVerticalAlignCenter()
			self.textLine.SetHorizontalAlignCenter()
			self.textLine.SetVerticalAlignCenter()
			self.textLine.Show()

		def Open(self, vid, text):
			self.vid = vid

			self.textLine.SetText(text)
			width = max(32, len(text)*6 + 20*2)
			self.SetSize(width, 32)
			
			self.Show() 
					
			g_privateShopAdvertisementBoardDict[vid] = self
			
		def OnMouseLeftButtonUp(self):
			if not self.vid:
				return
			net.SendOnClickPacket(self.vid)
			
			return True
			
		def OnUpdate(self):
			if not self.vid:
				return

			if systemSetting.IsShowSalesText():
				self.Show()
				x, y = chr.GetProjectPosition(self.vid, 220)
				width = x - self.GetWidth()/2
				height = y - self.GetHeight()/2
				self.SetPosition(width, height)
			else:
				for key in g_privateShopAdvertisementBoardDict.keys():
					if  player.GetMainCharacterIndex() == key:
						title_bar = g_privateShopAdvertisementBoardDict[key]
						
						title_bar.Show()
						
						x, y = chr.GetProjectPosition(player.GetMainCharacterIndex(), 220)
						
						width = x - title_bar.GetWidth()/2
						height = y - title_bar.GetHeight()/2
						
						title_bar.SetPosition(width, height)
					else:
						title_bar = g_privateShopAdvertisementBoardDict[key]
						title_bar.Hide()

