import chat, net, player, item, wndMgr, mouseModule, ui, localeInfo
import __builtin__ as selfs

class ChestDropWindow(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		
		self.currentChest = 0
		self.currentPage = 1
		self.openAmount = 1
		self.ItemVnum = -1
		self.InvType = -1
		self.InvPos = -1
		
		self.chestDrop = {}

		self.__LoadWindow()

	def __del__(self):
		ui.ScriptWindow.__del__(self)
		
	def __LoadWindow(self):
		try:
			PythonScriptLoader = ui.PythonScriptLoader()
			PythonScriptLoader.LoadScriptFile(self, "UIScript/chestdropwindow.py")
		except:
			import exception
			exception.Abort("ChestDropWindow.__LoadWindow.LoadObject")

		try:
			self.titleBar = self.GetChild("TitleBar")

			self.openItemSlot = self.GetChild("OpenItemSlot")
			# self.openCount = self.GetChild("CountValue")
			# self.openChestButtonSingle = self.GetChild("OpenChestButtonSingle")
			# self.openChestButtonMultiple = self.GetChild("OpenChestButtonMultiple")
			
			self.prevButton = self.GetChild("prev_button")
			self.nextButton = self.GetChild("next_button")
			self.currentPageBack = self.GetChild("CurrentPageBack")
			self.currentPageText = self.GetChild("CurrentPage")
		except:
			import exception
			exception.Abort("ChestDropWindow.__LoadWindow.BindObject")
			
		self.titleBar.SetCloseEvent(ui.__mem_func__(self.Close))
		
		# self.openChestButtonSingle.SetEvent(ui.__mem_func__(self.OnClickOpenChest))
		# self.openChestButtonMultiple.SetEvent(ui.__mem_func__(self.OnClickOpenChest))
		
		# self.SetOpenAmount(1)
		
		self.prevButton.SetEvent(ui.__mem_func__(self.OnClickPrevPage))
		self.nextButton.SetEvent(ui.__mem_func__(self.OnClickNextPage))
		
		self.currentPageText.SetText(str(self.currentPage))

		wndItem = ui.GridSlotWindow()
		wndItem.SetParent(self)
		wndItem.SetPosition(8, 35)
		wndItem.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
		wndItem.SetOverInItemEvent(ui.__mem_func__(self.OverInItem))
		wndItem.SetOverOutItemEvent(ui.__mem_func__(self.OverOutItem))
		wndItem.ArrangeSlot(0, 15, 5, 32, 32, 0, 0)
		wndItem.RefreshSlot()
		wndItem.SetSlotBaseImage("d:/ymir work/ui/public/Slot_Base.sub", 1.0, 1.0, 1.0, 1.0)
		wndItem.Show()
		
		self.wndItem = wndItem
		
	def Close(self):
		self.ItemVnum = -1
		self.InvType = -1
		self.InvPos = -1
		self.Hide()
		
	def Destroy(self):
		self.ClearDictionary()
		
		self.wndItem = None
		
		self.currentChest = 0
		self.currentPage = 1
		self.openAmount = 1
		self.ItemVnum = -1
		self.InvType = -1
		self.InvPos = -1
		self.chestDrop = {}
		
	# def SetOpenAmount(self, amount):
		# self.openAmount = amount
		# self.openChestButtonSingle.SetText(localeInfo.CHEST_DROP)
		# self.openChestButtonMultiple.SetText(localeInfo.CHEST_DROP)
		
	def Open(self, invType = player.INVENTORY, invItemPos = -1):
		self.currentChest = 0
		self.currentPage = 1
		# self.SetOpenAmount(1)
		self.chestDrop = {}
		self.RefreshItemSlot()
		
		self.SetInvItemSlot(invType, invItemPos)
	
		self.SetTop()
		self.SetCenterPosition()
		self.Show()
			
	def AddChestDropItem(self, invType, invItemPos, pageIndex, slotIndex, itemVnum, itemCount):
		chestVnum = player.GetItemIndex(invType, invItemPos)
		if not self.chestDrop.has_key(chestVnum):
			self.chestDrop[chestVnum] = {}
			
		if not self.chestDrop[chestVnum].has_key(pageIndex):
			self.chestDrop[chestVnum][pageIndex] = {}
	
		if self.chestDrop[chestVnum].has_key(pageIndex):
			if self.chestDrop[chestVnum][pageIndex].has_key(slotIndex):
				if self.chestDrop[chestVnum][pageIndex][slotIndex][0] == itemVnum and self.chestDrop[chestVnum][pageIndex][slotIndex][1] == itemCount:
					return

		self.chestDrop[chestVnum][pageIndex][slotIndex] = [itemVnum, itemCount]

	# def OnClickOpenChest(self):
		# if self.ItemVnum == -1:
			# return
		# self.openAmount = int(self.openCount.GetText())
		# UseCount = 0
		# Pos = 0
		# count = player.GetItemCountByVnum(self.ItemVnum)
		# if player.GetItemTypeBySlot(self.InvType, player.GetItemPosByVnum(self.InvType, self.ItemVnum, 0)) == item.ITEM_TYPE_GACHA:
			# for i in xrange(count):
				# Pos = player.GetItemPosByVnum(self.InvType, self.ItemVnum, i)
				# gacha = player.GetItemMetinSocket(self.InvType, Pos, 0)
				# if self.openAmount - UseCount < gacha:
					# for a in xrange(self.openAmount - UseCount):
						# net.SendItemUsePacket(self.InvType, Pos)
						# UseCount += 1
				# else:
					# for a in xrange(gacha):
						# net.SendItemUsePacket(self.InvType, Pos)
						# UseCount += 1

				# if UseCount >= self.openAmount:
					# break
		# else:
		# for i in xrange(count):
			# Pos = player.GetItemPosByVnum(self.InvType, self.ItemVnum, i)
			# icount = player.GetItemCount(self.InvType, Pos)
			# for i in xrange(icount):
				# net.SendItemUsePacket(self.InvType, Pos)
				# UseCount += 1

				# if UseCount >= self.openAmount:
					# break

			# if UseCount >= self.openAmount:
				# break

		# self.OnPressEscapeKey()
		# self.openCount.SetText("1")

	def OnClickPrevPage(self):
		if not self.chestDrop.has_key(self.currentChest):
			return
			
		if self.chestDrop[self.currentChest].has_key(self.currentPage - 1):
			self.currentPage = self.currentPage - 1
			self.currentPageText.SetText(str(self.currentPage))
			self.RefreshItemSlot()
		
	def OnClickNextPage(self):
		if not self.chestDrop.has_key(self.currentChest):
			return
			
		if self.chestDrop[self.currentChest].has_key(self.currentPage + 1):
			self.currentPage = self.currentPage + 1
			self.currentPageText.SetText(str(self.currentPage))
			self.RefreshItemSlot()
			
	def EnableMultiPage(self):
		# self.openChestButtonSingle.Hide()
		# self.openChestButtonMultiple.Show()
		
		self.prevButton.Show()
		self.nextButton.Show()
		self.currentPageBack.Show()
		
	def EnableSinglePage(self):
		# self.openChestButtonSingle.Show()
		# self.openChestButtonMultiple.Hide()
		
		self.prevButton.Hide()
		self.nextButton.Hide()
		self.currentPageBack.Hide()	
		
	def SetInvItemSlot(self, invType, invItemPos):		
		self.ItemVnum = player.GetItemIndex(invType, invItemPos)
		self.InvType = invType
		self.InvPos = invItemPos
		# itemCount = player.GetItemCountByVnum(self.ItemVnum)
		# if player.GetItemTypeBySlot(self.InvType, player.GetItemPosByVnum(self.InvType, self.ItemVnum, 0)) == item.ITEM_TYPE_GACHA:
			# count = itemCount
			# itemCount = 0
			# for i in xrange(count):
				# Pos = player.GetItemPosByVnum(self.InvType, self.ItemVnum, i)
				# itemCount = itemCount + player.GetItemMetinSocket(self.InvType, Pos, 0)

		if self.ItemVnum:
			self.openItemSlot.SetItemSlot(0, self.ItemVnum, 0)
			
	# def GetInvItemSlot(self):
		# return self.ItemVnum
			
	def RefreshItems(self, invType, invItemPos):
		chestVnum = player.GetItemIndex(invType, invItemPos)
		if chestVnum:
			self.currentChest = chestVnum
			
		if not self.chestDrop.has_key(self.currentChest):
			chat.AppendChat(chat.CHAT_TYPE_INFO, "Sandýk bilgisi bulunamadý !")
			self.Close()
			return
			
		# if self.chestDrop[self.currentChest].has_key(2):
			# self.EnableMultiPage()
		# else:
		self.EnableSinglePage()
			
		self.RefreshItemSlot()

	def RefreshItemSlot(self):
		for i in xrange(15 * 5):
			self.wndItem.ClearSlot(i)
			
		if not self.chestDrop.has_key(self.currentChest):
			return
			
		if not self.chestDrop[self.currentChest].has_key(self.currentPage):
			return
			
		for key, value in self.chestDrop[self.currentChest][self.currentPage].iteritems():
			itemVnum = value[0]
			itemCount = value[1]

			if itemCount <= 1:
				itemCount = 0
			
			self.wndItem.SetItemSlot(key, itemVnum, itemCount)
			
		wndMgr.RefreshSlot(self.wndItem.GetWindowHandle())
			
	def OverInItem(self, slotIndex):
		if mouseModule.mouseController.isAttached():
			return
			
		if not self.chestDrop.has_key(self.currentChest):
			return
			
		if not self.chestDrop[self.currentChest].has_key(self.currentPage):
			return
    
		if 0 != selfs.wndToolTipItem:
			selfs.wndToolTipItem.SetItemToolTip(self.chestDrop[self.currentChest][self.currentPage][slotIndex][0])
    
	def OverOutItem(self):
		if 0 != selfs.wndToolTipItem:
			selfs.wndToolTipItem.HideToolTip()
								
	def OnPressEscapeKey(self):
		self.Close()
		return True
