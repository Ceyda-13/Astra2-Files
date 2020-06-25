import ui
import shop
import net
import uiCommon
import localeInfo
import constInfo
import uiPickETC
import app
import ops
import __builtin__ as selfs
import mouseModule
import player
import item
import chat
import snd

class Item(ui.ListBoxEx.Item):
	def __init__(self, text):
		ui.ListBoxEx.Item.__init__(self)
		self.canLoad = 0
		self.text = text
		self.textLine = self.__CreateTextLine(text)
	def __del__(self):
		ui.ListBoxEx.Item.__del__(self)

	def GetText(self):
		return self.text

	def SetSize(self, width, height):
		ui.ListBoxEx.Item.SetSize(self, 150, height)

	def __CreateTextLine(self, text):
		textLine = ui.TextLine()
		textLine.SetParent(self)
		textLine.SetPosition(79, 0)
		textLine.SetHorizontalAlignCenter()
		textLine.SetText(text)
		textLine.Show()
		return textLine

class OfflinePrivateShopEdit(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.NameLine = None
		self.NameSlot = None
		self.time_select_pivot_window = 0
		self.cur_time_text = 0
		self.time_select_button = 0
		self.time_bar = 0
		self.time_listbox = 0
		self.time_scrollbar = 0
		self.time_window = 0
		self.id = 0
		self.time_gauge = None
		self.time_text = None
		self.moneyText = None
		self.worthText = None
		self.questionDialog = None
		self.dlgPickMoney = None
		self.OpenShopButton = None
		self.ShopDecoButton = None
		self.ShopLogButton = None
		self.Page1Button = None
		self.Page2Button = None
		self.BackButton = None
		self.RenameButton = None
		self.ShopSearchButton = None
		self.GoldStashButton = None
		self.Log_Page = None
		self.Log_Header_Item_Name = None
		self.Log_Header_Buy_Time = None
		self.Log_Header_Item_Price = None
		self.GoldStashWrapper = None
		self.Shop_Time_Gauge_Slot = None
		self.type = 0
		self.page = 0
		self.priceInputBoard = None
		self.list = [None] * 180

		if WJ_ENABLE_TRADABLE_ICON:
			self.AddList = []

		self.__LoadWindow()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "UIScript/PrivateShopEditor.py")
		except:
			import exception
			exception.Abort("PrivateShopEditorWindow.LoadWindow.LoadObject")

		try:
			GetObject = self.GetChild
			self.NameSlot = GetObject("NameSlot")
			self.NameLine = GetObject("NameLine")
			self.itemSlot = GetObject("ItemSlot")
			self.CloseButton = GetObject("CloseButton")
			self.titleBar = GetObject("TitleBar")
			self.positionInfo = GetObject("Position")
			self.time_select_pivot_window = GetObject("time_select_pivot_window")
			self.cur_time_text = GetObject("cur_time_text")
			self.time_select_button = GetObject("time_select_button")
			self.time_gauge = GetObject("Shop_Time_Gauge")
			self.time_text = GetObject("Shop_Time_Text")
			self.time_window = GetObject("time_window")
			self.moneyText = GetObject("GoldStashValue")
			self.worthText = GetObject("ItemsNetWorthText")
			self.OpenShopButton = GetObject("OpenShopButton")
			self.ShopDecoButton = GetObject("ShopDecoButton")
			self.ShopLogButton = GetObject("ShopLogButton")
			self.Page1Button = GetObject("Page1Button")
			self.Page2Button = GetObject("Page2Button")
			self.BackButton = GetObject("BackButton")
			self.RenameButton = GetObject("RenameButton")
			self.ShopSearchButton = GetObject("ShopSearchButton")
			self.GoldStashButton = GetObject("GoldStash_Slot")
			self.Log_Page = GetObject("Log_Page")
			self.Log_Header_Item_Name = GetObject("Log_Header_Item_Name")
			self.Log_Header_Buy_Time = GetObject("Log_Header_Buy_Time")
			self.Log_Header_Item_Price = GetObject("Log_Header_Item_Price")
			self.GoldStashWrapper = GetObject("GoldStashWrapper")
			self.Shop_Time_Gauge_Slot = GetObject("Shop_Time_Gauge_Slot")
		except:
			import exception
			exception.Abort("PrivateShopBuilderWindow.LoadWindow.BindObject")

		self.titleBar.SetCloseEvent(ui.__mem_func__(self.Close))
		self.CloseButton.SetEvent(ui.__mem_func__(self.OnClose))
		self.OpenShopButton.SetEvent(ui.__mem_func__(self.OnOpenNewShop))
		self.ShopLogButton.SetEvent(ui.__mem_func__(self.LogOpen), 1)
		self.BackButton.SetEvent(ui.__mem_func__(self.LogClose), 1)
		self.Page1Button.SetEvent(ui.__mem_func__(self.SetPage), 0)
		self.Page2Button.SetEvent(ui.__mem_func__(self.SetPage), 1)
		self.RenameButton.SetEvent(ui.__mem_func__(self.Rename))
		self.ShopSearchButton.SetEvent(ui.__mem_func__(self.OpenShopSearch))
		self.GoldStashButton.SetEvent(ui.__mem_func__(self.OpenPickMoneyDialog))
		self.BackButton.Hide()
		self.Log_Page.Hide()
		self.Page1Button.Down()
		self.itemSlot.SetOverInItemEvent(ui.__mem_func__(self.OnOverInItem))
		self.itemSlot.SetOverOutItemEvent(ui.__mem_func__(self.OnOverOutItem))
		self.itemSlot.SetSelectEmptySlotEvent(ui.__mem_func__(self.OnSelectEmptySlot))
		self.itemSlot.SetSelectItemSlotEvent(ui.__mem_func__(self.OnSelectItemSlot))

		# GetObject("time_window").SetPosition(x, y + 58.85)
		x, y = self.time_select_pivot_window.GetGlobalPosition()
		self.time_bar = ui.Bar()
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

		self.time_listbox.SetSelectEvent(ui.__mem_func__(self.__SelectTime))
		self.time_listbox.Show()

		self.time_scrollbar = ui.ScrollBar()
		self.time_scrollbar.SetParent(self.time_listbox)
		self.time_scrollbar.SetScrollBarSize(94)
		self.time_scrollbar.SetPosition(144, 0)
		self.time_scrollbar.Show()
		self.time_listbox.SetScrollBar(self.time_scrollbar)

		self.time_select_button.SAFE_SetEvent(self.OnTimeSelect)

		# self.SetSize(self.GetWidth(), self.GetHeight() + 40)
		# GetObject("board").SetSize(self.GetWidth(), self.GetHeight() + 25)

		self.GetChild("TitleName").SetText("Aktif Pazarlar")
		self.cur_time_text.SetText("Aktif Pazar Bulunamadı")

		self.dlgPickMoney = uiPickETC.PickETCDialog()
		self.dlgPickMoney.LoadDialog()
		self.dlgPickMoney.Hide()

	def Destroy(self):
		self.ClearDictionary()
		self.dlgPickMoney.Destroy()
		self.dlgPickMoney = None
		self.NameLine = None
		self.NameSlot = None
		self.itemSlot = None
		self.CloseButton = None
		self.titleBar = None
		self.questionDialog = None
		self.time_gauge = None
		self.time_text = None
		self.time_window = None
		self.moneyText = None
		self.worthText = None
		self.OpenShopButton = None
		self.ShopDecoButton = None
		self.ShopLogButton = None
		self.Page1Button = None
		self.Page2Button = None
		self.BackButton = None
		self.RenameButton = None
		self.ShopSearchButton = None
		self.GoldStashButton = None
		self.Log_Header_Item_Name = None
		self.Log_Header_Buy_Time = None
		self.Log_Header_Item_Price = None
		self.GoldStashWrapper = None
		self.Shop_Time_Gauge_Slot = None
		self.priceInputBoard = None
		self.list = []
		if WJ_ENABLE_TRADABLE_ICON:
			self.AddList = []

	def Open(self):
		self.Refresh()

		self.SetCenterPosition()
		self.Show()

	def LogOpen(self, i):
		self.Log_Page.Show()
		self.ShopDecoButton.Hide()
		self.itemSlot.Hide()
		self.worthText.Hide()
		self.positionInfo.Hide()
		self.time_window.Hide()
		self.Page1Button.Hide()
		self.Page2Button.Hide()
		self.Shop_Time_Gauge_Slot.Hide()
		self.time_text.Hide()
		self.NameSlot.Hide()
		self.NameLine.Hide()
		self.RenameButton.Hide()
		self.GoldStashWrapper.Hide()
		if i == 0:
			self.BackButton.Hide()
			self.CloseButton.Hide()
			self.OpenShopButton.Show()
		else:
			self.BackButton.Show()
			self.OpenShopButton.Hide()
		self.ShopLogButton.Down()
		net.SendChatPacket("/click_offline_private_shop 7")

	def LogClose(self, i):
		self.OpenShopButton.Hide()
		self.Log_Page.Hide()
		self.CloseButton.Show()
		self.ShopDecoButton.Show()
		self.itemSlot.Show()
		self.worthText.Show()
		self.positionInfo.Show()
		self.time_window.Show()
		self.Page1Button.Show()
		self.Page2Button.Show()
		self.Shop_Time_Gauge_Slot.Show()
		self.time_text.Show()
		self.NameSlot.Show()
		self.NameLine.Show()
		self.RenameButton.Show()
		self.GoldStashWrapper.Show()
		if i == 0:
			self.BackButton.Hide()
		else:
			self.BackButton.Show()
		self.ShopLogButton.SetUp()

	def SetPage(self, i):
		self.page = i
		if i:
			self.Page1Button.SetUp()
		else:
			self.Page2Button.SetUp()

		self.Refresh()

	def Close(self):
		self.Hide()
		self.time_listbox.RemoveAllItems()
		self.time_bar.Hide()
		self.OnCloseQuestionDialog()

	def RefreshLogs(self):
		pass

	def Refresh(self):
		if ops.GetShopCount(self.type):
			self.LogClose(0)
			self.time_listbox.RemoveAllItems()
			for i in xrange(ops.GetShopCount(self.type)):
				self.time_listbox.AppendItem(Item(ops.GetSign(self.type, i)))
		else:
			self.LogOpen(0)
			return

		getitemVNum = ops.GetItemVnum
		getItemCount = ops.GetItemCount
		setitemVNum = self.itemSlot.SetItemSlot
		asd = ops.GetUnlockSlots(self.id)
		# TraceError("Count: %d" % asd)
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
				btn.SetEvent(lambda : self.UnlockSlot())
				btn.Show()
				self.list[slotNumber] = (btn)

			# TraceError("%d - %d %d %d %d" % (i, slotNumber, var, y2, x2))

			itemCount = getItemCount(self.type, self.id, slotNumber)
			if itemCount <= 1:
				itemCount = 0

			setitemVNum(i, getitemVNum(self.type, self.id, slotNumber), itemCount)
			# self.list[slotNumber] = 1

			if ENABLE_CHANGE_LOOK_SYSTEM:
				itemTransmutedVnum = ops.GetItemChangeLookVnum(self.type, self.id, slotNumber)
				if itemTransmutedVnum:
					self.itemSlot.DisableCoverButton(i)
				else:
					self.itemSlot.EnableCoverButton(i)

		self.itemSlot.RefreshSlot()

		(x, y, index) = ops.GetPosition(self.id)
		self.positionInfo.SetText("%s - (%.0f, %.0f)" % (localeInfo.MINIMAP_ZONE_NAME_DICT_BY_IDX[index], x/100, y/100))

		self.NameLine.SetText(ops.GetSign(self.type, self.id))

		self.cur_time_text.SetText(ops.GetSign(self.type, self.id))

		(start, end) = ops.GetTime(self.id)
		if start and end:
			self.time_gauge.SetPercentage(end - app.GetGlobalTimeStamp(), end - start)
			self.time_text.SetText("Kalan Zaman: %d / %d" % (end - app.GetGlobalTimeStamp(), end - start))

		self.moneyText.SetText("%s" % localeInfo.NumberToMoneyString(ops.GetPrice(self.type, self.id, 0)))

		self.worthText.SetText("Toplam Tutar: %s" % localeInfo.NumberToMoneyString(ops.GetWorth(self.id)))

	def OnOpenNewShop(self):
		self.Close()
		if selfs.wndPrivateShopBuilder.IsShow():
			selfs.wndPrivateShopBuilder.Close()
		else:
			constInfo.PriceCheck = {}
			net.SendChatPacket("/price_check")
			selfs.wndPrivateShopBuilder.Open("", 0, 30000)

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def OnOverInItem(self, slotIndex):
		if selfs.wndToolTipItem:
			slotIndex += self.page * shop.SHOP_PAGE_SIZE
			selfs.wndToolTipItem.SetEditPrivateShopItem(self.type, self.id, slotIndex)

	def OnOverOutItem(self):
		if selfs.wndToolTipItem:
			selfs.wndToolTipItem.HideToolTip()

	def OnUpdate(self):
		if self.type == 0:
			(start, end) = ops.GetTime(self.id)
			if start and end:
				self.time_gauge.SetPercentage(end - app.GetGlobalTimeStamp(), end - start)
				self.time_text.SetText("Kalan Zaman: %s / %s" % (localeInfo.SecondToColonTypeHM(end - app.GetGlobalTimeStamp()), localeInfo.SecondToColonTypeHM(end - start)))#("%s - %s" % (localeInfo.SecondToColonTypeHM(end - app.GetGlobalTimeStamp()), localeInfo.SecondToColonTypeHM(end - start)))
		else:
			pass

	def OnMoveWindow(self, x, y):
		 if self.time_bar:
			self.BarAdjustPositionAndSize()

	def BarAdjustPositionAndSize(self):
		x, y = self.time_select_pivot_window.GetGlobalPosition()
		self.time_bar.SetPosition(x, y)

	def __SelectTime(self):
		self.cur_time_text.SetText(self.time_listbox.GetSelectedItem().GetText())
		if self.time_bar:
			self.time_bar.Hide()

		self.id = self.time_listbox.GetItemIndex(self.time_listbox.GetSelectedItem())
		self.Refresh()

	def OnTimeSelect(self):
		x, y = self.time_select_pivot_window.GetGlobalPosition()
		self.time_bar.SetPosition(x, y)
		if self.time_bar:
			if self.time_bar.IsShow():
				self.time_bar.Hide()
			else:
				self.time_bar.Show()

	def OpenPickMoneyDialog(self, i = 0):
		cur = ops.GetPrice(self.type, self.id, i)

		if cur <= 0:
			return

		self.dlgPickMoney.SetTitleName(localeInfo.PICK_MONEY_TITLE)
		self.dlgPickMoney.SetAcceptEvent(ui.__mem_func__(self.PullPrice), i)
		self.dlgPickMoney.Open(cur)

		self.dlgPickMoney.SetMax(17)

	def PullPrice(self, count, i):
		# TraceError("%d" % count)
		net.SendChatPacket("/click_offline_private_shop 8 %d %d" % (ops.GetVid(self.type, self.id), count))

	def Rename(self):
		net.SendChatPacket("/click_offline_private_shop 3 %d %s" % (ops.GetVid(self.type, self.id), self.NameLine.GetText()))

	def OpenShopSearch(self):
		if selfs.wndOfflinePrivateShopSearch:
			if selfs.wndOfflinePrivateShopSearch.IsShow():
				selfs.wndOfflinePrivateShopSearch.Close()
			else:
				selfs.wndOfflinePrivateShopSearch.Open(1)

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

			(width, height) = item.GetItemSize()
			for i in xrange(height):
				if self.list[selectedSlotPos + (height - 1) * 10]:
					return

			if WJ_ENABLE_TRADABLE_ICON:
				self.AddList.append(attachedSlotPos)
				selfs.wndInventory.SetCantMouseEventSlot(attachedSlotPos)

			priceInputBoard = uiCommon.MoneyInputDialog()
			priceInputBoard.SetTitle(localeInfo.PRIVATE_SHOP_INPUT_PRICE_DIALOG_TITLE)
			priceInputBoard.SetAcceptEvent(ui.__mem_func__(self.AcceptInputPrice))
			priceInputBoard.SetCancelEvent(ui.__mem_func__(self.CancelInputPrice))
			if constInfo.PriceCheck.has_key(itemVNum):
				(min, max) = constInfo.PriceCheck[itemVNum]
				priceInputBoard.SetCheckPrices(min, max)

			priceInputBoard.Open()

			self.priceInputBoard = priceInputBoard
			self.priceInputBoard.itemVNum = itemVNum
			self.priceInputBoard.sourceWindowType = attachedInvenType
			self.priceInputBoard.sourceSlotPos = attachedSlotPos
			self.priceInputBoard.targetSlotPos = selectedSlotPos

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

		if ENABLE_MONEY_SHORTING:
			price = int(text)
		else:
			price = int(self.priceInputBoard.GetText())

		if constInfo.PriceCheck.has_key(self.priceInputBoard.itemVNum):
			(min, max) = constInfo.PriceCheck[self.priceInputBoard.itemVNum]
			count = player.GetItemCount(sourceSlotPos)
			fprice = float(float(price) / float(count))
			if fprice > float(max) or fprice < float(min):
				chat.AppendChat(chat.CHAT_TYPE_INFO, "Bu Fiyata Ekliyemezsin !")
				return

		net.SendChatPacket("/click_offline_private_shop 1 %d %d %d %d %d" % (ops.GetVid(self.type, self.id), targetSlotPos, attachedInvenType, sourceSlotPos, price))
		snd.PlaySound("sound/ui/drop.wav")

		if WJ_ENABLE_TRADABLE_ICON:
			self.AddList.remove(self.priceInputBoard.sourceSlotPos)
			selfs.wndInventory.SetCanMouseEventSlot(self.priceInputBoard.sourceSlotPos)

		self.Refresh()
		self.priceInputBoard = None
		return True

	def CancelInputPrice(self):
		if WJ_ENABLE_TRADABLE_ICON:
			self.AddList.remove(self.priceInputBoard.sourceSlotPos)
			selfs.wndInventory.SetCanMouseEventSlot(self.priceInputBoard.sourceSlotPos)

		self.priceInputBoard = None
		return True

	def OnSelectItemSlot(self, selectedSlotPos):
		selectedSlotPos += self.page * shop.SHOP_PAGE_SIZE
		isAttached = mouseModule.mouseController.isAttached()
		if isAttached:
			snd.PlaySound("sound/ui/loginfail.wav")
			mouseModule.mouseController.DeattachObject()
		else:
			# shop.DelPrivateShopItemStock(invenType, invenPos)
			net.SendChatPacket("/click_offline_private_shop 2 %d %d" % (ops.GetVid(self.type, self.id), selectedSlotPos))
			snd.PlaySound("sound/ui/drop.wav")
			self.Refresh()

	def OnClose(self):
		questionDialog = uiCommon.QuestionDialog()
		questionDialog.SetText(localeInfo.PRIVATE_SHOP_CLOSE_QUESTION)
		questionDialog.SetAcceptEvent(ui.__mem_func__(self.OnClosePrivateShop))
		questionDialog.SetCancelEvent(ui.__mem_func__(self.OnCloseQuestionDialog))
		questionDialog.Open()
		self.questionDialog = questionDialog

		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)

		return True

	def OnClosePrivateShop(self):
		# net.SendChatPacket("/click_offline_private_shop 1 %d" % ops.GetVid(self.type, self.id))
		# self.OnCloseQuestionDialog()
		# self.Close()
		# ops.Clear(self.type)
		return True

	def UnlockSlot(self):
		net.SendChatPacket("/click_offline_private_shop 6 %d" % ops.GetVid(self.type, self.id))

	def OnCloseQuestionDialog(self):
		if not self.questionDialog:
			return

		self.questionDialog.Close()
		self.questionDialog = None
		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

	def OnUpdate(self):
		if WJ_ENABLE_TRADABLE_ICON:
			for idx in self.AddList:
				selfs.wndInventory.SetCantMouseEventSlot(idx)
