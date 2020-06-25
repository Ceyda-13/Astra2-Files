import net
import player
import item
import snd
import shop
import net
import wndMgr
import app
import chat

import ui
import uiCommon
import mouseModule
import localeInfo
import constInfo
import __builtin__ as selfs

class ShopDialog(ui.ScriptWindow):

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.xShopStart = 0
		self.yShopStart = 0
		self.questionDialog = None
		self.popup = None
		self.itemBuyQuestionDialog = None
		if ENABLE_SHOP_TAB: # @@@correction030
			self.shopPageIndex = 0

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __GetRealIndex(self, i):
		if ENABLE_SHOP_TAB: # @@@correction030
			return self.shopPageIndex * shop.SHOP_PAGE_SIZE + i
		else:
			return self.tabIdx * shop.SHOP_SLOT_COUNT + i

	def Refresh(self):
		getItemID = shop.GetItemID
		getItemCount = shop.GetItemCount
		setItemID = self.itemSlotWindow.SetItemSlot
		asd = shop.GetUnlockSlots()
		z = 1
		j = asd / 45
		if self.shopPageIndex and j:
			if j == 1:
				z = 0
			elif j == 3:
				z = 2
			asd = asd % 45
		self.list = [None] * 180

		for i in xrange(shop.SHOP_PAGE_SIZE):
			idx = self.__GetRealIndex(i)
			y = i / 10
			x = i % 10
			y2 = asd / 5
			x2 = asd % 5
			var = (y * 10) + (z * 5) + self.shopPageIndex * shop.SHOP_PAGE_SIZE
			if y2 > y and z:
				var += y2 * 10

			if y2 > y and not z:
				var += 5

			if y2 == y:
				var += x2

			if idx >= var or (self.shopPageIndex and z and not j):
				btn = ui.Button()
				btn.SetParent(self.itemSlotWindow)
				btn.SetUpVisual("d:/ymir work/ui/game/shopsearchp2p/padlock.dds")
				btn.SetOverVisual("d:/ymir work/ui/game/shopsearchp2p/padlock.dds")
				btn.SetDownVisual("d:/ymir work/ui/game/shopsearchp2p/padlock.dds")
				btn.SetPosition((x) * 32, (y) * 32)
				# btn.SetEvent(lambda : self.DeleteAll())
				btn.Show()
				self.list[idx] = (btn)

			itemCount = getItemCount(idx)
			if itemCount <= 1:
				itemCount = 0
			setItemID(i, getItemID(idx), itemCount)

		wndMgr.RefreshSlot(self.itemSlotWindow.GetWindowHandle())

	def SetItemData(self, pos, itemID, itemCount, itemPrice):
		shop.SetItemData(pos, itemID, itemCount, itemPrice)

	def LoadDialog(self):
		try:
			PythonScriptLoader = ui.PythonScriptLoader()
			PythonScriptLoader.LoadScriptFile(self, "UIScript/shopdialog.py")
		except:
			import exception
			exception.Abort("ShopDialog.LoadDialog.LoadObject")

		if not ENABLE_SHOP_TAB: # @@@correction030
			smallTab1 = None
			smallTab2 = None
			smallTab3 = None
			middleTab1 = None
			middleTab2 = None
		try:
			GetObject = self.GetChild
			self.itemSlotWindow = GetObject("ItemSlot")
			self.titleBar = GetObject("TitleBar")
			if ENABLE_SHOP_TAB: # @@@correction030
				self.Page1Button = self.GetChild("Page1Button")
				self.Page2Button = self.GetChild("Page2Button")
			else:
				self.btnBuy = GetObject("BuyButton")
				self.btnSell = GetObject("SellButton")
				self.btnClose = GetObject("CloseButton")
				middleTab1 = GetObject("MiddleTab1")
				middleTab2 = GetObject("MiddleTab2")
				smallTab1 = GetObject("SmallTab1")
				smallTab2 = GetObject("SmallTab2")
				smallTab3 = GetObject("SmallTab3")
		except:
			import exception
			exception.Abort("ShopDialog.LoadDialog.BindObject")

		self.itemSlotWindow.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
		self.itemSlotWindow.SAFE_SetButtonEvent("LEFT", "EMPTY", self.SelectEmptySlot)
		self.itemSlotWindow.SAFE_SetButtonEvent("LEFT", "EXIST", self.SelectItemSlot)
		self.itemSlotWindow.SAFE_SetButtonEvent("RIGHT", "EXIST", self.UnselectItemSlot)

		self.itemSlotWindow.SetOverInItemEvent(ui.__mem_func__(self.OverInItem))
		self.itemSlotWindow.SetOverOutItemEvent(ui.__mem_func__(self.OverOutItem))
		self.titleBar.SetCloseEvent(ui.__mem_func__(self.Close))

		if ENABLE_SHOP_TAB: # @@@correction030
			self.Page1Button.SetEvent(lambda arg = 0: self.SetShopPage(arg))
			self.Page2Button.SetEvent(lambda arg = 1: self.SetShopPage(arg))
			self.Page1Button.Down()
			self.SetShopPage(0)
		else:
			self.btnBuy.SetToggleUpEvent(ui.__mem_func__(self.CancelShopping))
			self.btnBuy.SetToggleDownEvent(ui.__mem_func__(self.OnBuy))

			self.btnSell.SetToggleUpEvent(ui.__mem_func__(self.CancelShopping))
			self.btnSell.SetToggleDownEvent(ui.__mem_func__(self.OnSell))

			self.btnClose.SetEvent(ui.__mem_func__(self.AskClosePrivateShop))

			self.smallRadioButtonGroup = ui.RadioButtonGroup.Create([[smallTab1, lambda : self.OnClickTabButton(0), None], [smallTab2, lambda : self.OnClickTabButton(1), None], [smallTab3, lambda : self.OnClickTabButton(2), None]])
			self.middleRadioButtonGroup = ui.RadioButtonGroup.Create([[middleTab1, lambda : self.OnClickTabButton(0), None], [middleTab2, lambda : self.OnClickTabButton(1), None]])

			self.__HideMiddleTabs()
			self.__HideSmallTabs()

			self.tabIdx = 0
			self.coinType = shop.SHOP_COIN_TYPE_GOLD

		self.Refresh()

	if not ENABLE_SHOP_TAB: # @@@correction030
		def __ShowBuySellButton(self):
			self.btnBuy.Show()
			self.btnSell.Show()

		def __ShowMiddleTabs(self):
			self.middleRadioButtonGroup.Show()

		def __ShowSmallTabs(self):
			self.smallRadioButtonGroup.Show()

		def __HideBuySellButton(self):
			self.btnBuy.Hide()
			self.btnSell.Hide()

		def __HideMiddleTabs(self):
			self.middleRadioButtonGroup.Hide()

		def __HideSmallTabs(self):
			self.smallRadioButtonGroup.Hide()

		def __SetTabNames(self):
			if shop.GetTabCount() == 2:
				self.middleRadioButtonGroup.SetText(0, shop.GetTabName(0))
				self.middleRadioButtonGroup.SetText(1, shop.GetTabName(1))
			elif shop.GetTabCount() == 3:
				self.smallRadioButtonGroup.SetText(0, shop.GetTabName(0))
				self.smallRadioButtonGroup.SetText(1, shop.GetTabName(1))
				self.smallRadioButtonGroup.SetText(2, shop.GetTabName(2))
	else:
		def SetShopPage(self, page):
			self.shopPageIndex = page
			if page:
				self.Page1Button.SetUp()
			else:
				self.Page2Button.SetUp()

			self.Refresh()

	def Destroy(self):
		self.Close()
		self.ClearDictionary()

		self.itemSlotWindow = 0
		self.titleBar = 0
		self.questionDialog = None
		self.popup = None
		if ENABLE_SHOP_TAB: # @@@correction030
			self.shopPageIndex = 0
			self.Page1Button = None
			self.Page2Button = None
		else:
			self.btnBuy = 0
			self.btnSell = 0
			self.btnClose = 0

	def Open(self, vid):
		isPrivateShop = False
		isMainPlayerPrivateShop = False

		if ENABLE_SHOP_TAB: # @@@correction030
			shop.Open(isPrivateShop, isMainPlayerPrivateShop)
			self.SetShopPage(0)
		else:
			import chr
			if chr.IsNPC(vid):
				isPrivateShop = False
			else:
				isPrivateShop = True

			if player.IsMainCharacterIndex(vid):
				isMainPlayerPrivateShop = True
				self.btnBuy.Hide()
				self.btnSell.Hide()
				self.btnClose.Show()
			else:
				isMainPlayerPrivateShop = False
				self.btnBuy.Show()
				self.btnSell.Show()
				self.btnClose.Hide()

			shop.Open(isPrivateShop, isMainPlayerPrivateShop)

			self.tabIdx = 0

			if isPrivateShop:
				self.__HideMiddleTabs()
				self.__HideSmallTabs()
			else:
				if shop.GetTabCount() == 1:
					self.__ShowBuySellButton()
					self.__HideMiddleTabs()
					self.__HideSmallTabs()
				elif shop.GetTabCount() == 2:
					self.__HideBuySellButton()
					self.__ShowMiddleTabs()
					self.__HideSmallTabs()
					self.__SetTabNames()
					self.middleRadioButtonGroup.OnClick(0)
				elif shop.GetTabCount() == 3:
					self.__HideBuySellButton()
					self.__HideMiddleTabs()
					self.__ShowSmallTabs()
					self.__SetTabNames()
					self.smallRadioButtonGroup.OnClick(0) # @@@correction003 (middleRadioButtonGroup(1) -> smallRadioButtonGroup(0))

		self.Refresh()
		self.SetTop()

		self.Show()

		(self.xShopStart, self.yShopStart, z) = player.GetMainCharacterPosition()

	def Close(self):
		if self.itemBuyQuestionDialog:
			self.itemBuyQuestionDialog.Close()
			self.itemBuyQuestionDialog = None
			constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)
		if self.questionDialog:
			self.OnCloseQuestionDialog()
		shop.Close()
		net.SendShopEndPacket()
		if not ENABLE_SHOP_TAB: # @@@correction030
			self.CancelShopping()
		selfs.wndToolTipItem.HideToolTip()
		self.Hide()

	if not ENABLE_SHOP_TAB: # @@@correction030
		def GetIndexFromSlotPos(self, slotPos):
			return self.tabIdx * shop.SHOP_SLOT_COUNT + slotPos

		def OnClickTabButton(self, idx):
			self.tabIdx = idx
			self.Refresh()

	def AskClosePrivateShop(self):
		questionDialog = uiCommon.QuestionDialog()
		questionDialog.SetText(localeInfo.PRIVATE_SHOP_CLOSE_QUESTION)
		questionDialog.SetAcceptEvent(ui.__mem_func__(self.OnClosePrivateShop))
		questionDialog.SetCancelEvent(ui.__mem_func__(self.OnCloseQuestionDialog))
		questionDialog.Open()
		self.questionDialog = questionDialog

		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)

		return True

	def OnClosePrivateShop(self):
		net.SendChatPacket("/close_shop")
		self.OnCloseQuestionDialog()
		return True

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def OnPressExitKey(self):
		self.Close()
		return True

	if not ENABLE_SHOP_TAB: # @@@correction030
		def OnBuy(self):
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.SHOP_BUY_INFO)
			app.SetCursor(app.BUY)
			self.btnSell.SetUp()

		def OnSell(self):
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.SHOP_SELL_INFO)
			app.SetCursor(app.SELL)
			self.btnBuy.SetUp()

		def CancelShopping(self):
			self.btnBuy.SetUp()
			self.btnSell.SetUp()
			app.SetCursor(app.NORMAL)

	def __OnClosePopupDialog(self):
		self.pop = None
		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

	## ¿ëÈ¥¼® ÆÈ¸®´Â ±â´É Ãß°¡.
	def SellAttachedItem(self):

		if shop.IsPrivateShop():
			mouseModule.mouseController.DeattachObject()
			return

		attachedSlotType = mouseModule.mouseController.GetAttachedType()
		attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
		attachedCount = mouseModule.mouseController.GetAttachedItemCount()
		attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()

		if player.SLOT_TYPE_INVENTORY == attachedSlotType or player.SLOT_TYPE_DRAGON_SOUL_INVENTORY == attachedSlotType:

			item.SelectItem(attachedItemIndex)

			if item.IsAntiFlag(item.ANTIFLAG_SELL):
				popup = uiCommon.PopupDialog()
				popup.SetText(localeInfo.SHOP_CANNOT_SELL_ITEM)
				popup.SetAcceptEvent(self.__OnClosePopupDialog)
				popup.Open()
				self.popup = popup
				return

			itemtype = player.INVENTORY

			if player.SLOT_TYPE_DRAGON_SOUL_INVENTORY == attachedSlotType:
				itemtype = player.DRAGON_SOUL_INVENTORY

			if player.IsValuableItem(itemtype, attachedSlotPos):

				itemPrice = item.GetISellItemPrice()

				if item.Is1GoldItem():
					itemPrice = attachedCount / itemPrice / 5
				else:
					itemPrice = itemPrice * max(1, attachedCount) / 5

				itemName = item.GetItemName()

				questionDialog = uiCommon.QuestionDialog()
				questionDialog.SetText(localeInfo.DO_YOU_SELL_ITEM(itemName, attachedCount, itemPrice))

				questionDialog.SetAcceptEvent(lambda arg1=attachedSlotPos, arg2=attachedCount, arg3 = itemtype: self.OnSellItem(arg1, arg2, arg3))
				questionDialog.SetCancelEvent(ui.__mem_func__(self.OnCloseQuestionDialog))
				questionDialog.Open()
				self.questionDialog = questionDialog

				constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)

			else:
				self.OnSellItem(attachedSlotPos, attachedCount, itemtype)

		else:
			snd.PlaySound("sound/ui/loginfail.wav")

		mouseModule.mouseController.DeattachObject()

	def OnSellItem(self, slotPos, count, itemtype):
		net.SendShopSellPacketNew(slotPos, count, itemtype)
		snd.PlaySound("sound/ui/money.wav")
		self.OnCloseQuestionDialog()

	def OnCloseQuestionDialog(self):
		if not self.questionDialog:
			return

		self.questionDialog.Close()
		self.questionDialog = None
		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

	def SelectEmptySlot(self, selectedSlotPos):

		isAttached = mouseModule.mouseController.isAttached()
		if isAttached:
			self.SellAttachedItem()

	def UnselectItemSlot(self, selectedSlotPos):
		if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
			return
		if shop.IsPrivateShop():
			self.AskBuyItem(selectedSlotPos)
		else:
			net.SendShopBuyPacket(self.__GetRealIndex(selectedSlotPos))

	def SelectItemSlot(self, selectedSlotPos):
		if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
			return

		isAttached = mouseModule.mouseController.isAttached()
		selectedSlotPos = self.__GetRealIndex(selectedSlotPos)
		if isAttached:
			self.SellAttachedItem()

		else:

			if True == shop.IsMainPlayerPrivateShop():
				return

			curCursorNum = app.GetCursor()
			if app.BUY == curCursorNum:
				self.AskBuyItem(selectedSlotPos)

			elif app.SELL == curCursorNum:
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.SHOP_SELL_INFO)

			else:
				selectedItemID = shop.GetItemID(selectedSlotPos)
				itemCount = shop.GetItemCount(selectedSlotPos)

				type = player.SLOT_TYPE_SHOP
				if shop.IsPrivateShop():
					type = player.SLOT_TYPE_PRIVATE_SHOP

				mouseModule.mouseController.AttachObject(self, type, selectedSlotPos, selectedItemID, itemCount)
				mouseModule.mouseController.SetCallBack("INVENTORY", ui.__mem_func__(self.DropToInventory))
				snd.PlaySound("sound/ui/pick.wav")

	def DropToInventory(self):
		attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
		self.AskBuyItem(attachedSlotPos)

	def AskBuyItem(self, slotPos):
		slotPos = self.__GetRealIndex(slotPos)

		itemIndex = shop.GetItemID(slotPos)
		itemPrice = shop.GetItemPrice(slotPos)
		itemCount = shop.GetItemCount(slotPos)

		item.SelectItem(itemIndex)
		itemName = item.GetItemName()

		itemBuyQuestionDialog = uiCommon.QuestionDialog()
		if ENABLE_MULTISHOP and shop.GetBuyWithItem(slotPos): # @@@correction032
			itemBuyQuestionDialog.SetText(localeInfo.DO_YOU_BUY_ITEM(itemName, itemCount, localeInfo.NumberToWithItemString(shop.GetBuyWithItemCount(slotPos), item.GetItemName())))
		else:
			itemBuyQuestionDialog.SetText(localeInfo.DO_YOU_BUY_ITEM(itemName, itemCount, localeInfo.NumberToMoneyString(itemPrice)))

		itemBuyQuestionDialog.SetAcceptEvent(lambda arg=True: self.AnswerBuyItem(arg))
		itemBuyQuestionDialog.SetCancelEvent(lambda arg=False: self.AnswerBuyItem(arg))
		itemBuyQuestionDialog.Open()
		itemBuyQuestionDialog.pos = slotPos
		self.itemBuyQuestionDialog = itemBuyQuestionDialog

		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)

	def AnswerBuyItem(self, flag):

		if flag:
			pos = self.itemBuyQuestionDialog.pos
			net.SendShopBuyPacket(pos)

		self.itemBuyQuestionDialog.Close()
		self.itemBuyQuestionDialog = None

		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

	def OverInItem(self, slotIndex):
		slotIndex = self.__GetRealIndex(slotIndex)
		if mouseModule.mouseController.isAttached():
			return

		if 0 != selfs.wndToolTipItem:
			if not ENABLE_SHOP_TAB: # @@@correction030
				if shop.SHOP_COIN_TYPE_GOLD == shop.GetTabCoinType(self.tabIdx):
					selfs.wndToolTipItem.SetShopItem(slotIndex)
				else:
					selfs.wndToolTipItem.SetShopItemBySecondaryCoin(slotIndex)
			else:
				selfs.wndToolTipItem.SetShopItem(slotIndex)

	def OverOutItem(self):
		if 0 != selfs.wndToolTipItem:
			selfs.wndToolTipItem.HideToolTip()

	def OnUpdate(self):

		USE_SHOP_LIMIT_RANGE = 1000

		(x, y, z) = player.GetMainCharacterPosition()
		if abs(x - self.xShopStart) > USE_SHOP_LIMIT_RANGE or abs(y - self.yShopStart) > USE_SHOP_LIMIT_RANGE:
			self.Close()


class MallPageDialog(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def Destroy(self):
		self.ClearDictionary()

	def Open(self):
		scriptLoader = ui.PythonScriptLoader()
		scriptLoader.LoadScriptFile(self, "uiscript/mallpagedialog.py")

		self.GetChild("titlebar").SetCloseEvent(ui.__mem_func__(self.Close))

		(x, y)=self.GetGlobalPosition()
		x+=10
		y+=30

		MALL_PAGE_WIDTH = 600
		MALL_PAGE_HEIGHT = 480

		app.ShowWebPage(
			"http://metin2.co.kr/08_mall/game_mall/login_fail.htm",
			(x, y, x+MALL_PAGE_WIDTH, y+MALL_PAGE_HEIGHT))

		self.Lock()
		self.Show()

	def Close(self):
		app.HideWebPage()
		self.Unlock()
		self.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True
