import ui
import player
import mouseModule
import net
import app
import snd
import item
import player
import chat
import grp
import uiScriptLocale
import uiRefine
import uiAttachMetin
import uiPickMoney
import uiCommon
import uiPrivateShopBuilder
import localeInfo
import constInfo
import ime
import wndMgr
import __builtin__ as selfs
import shop
import exchange
import uiToolTip

if ENABLE_ACCE_SYSTEM: # @@@correction024
	import acce

if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
	import changelook

if WJ_ELDER_ATTRIBUTE_SYSTEM: # @@@correction026
	import uiAttachBonus

ITEM_FLAG_APPLICABLE = 1 << 14

class CostumeWindow(ui.ScriptWindow):

	def __init__(self):
		import exception

		if not app.ENABLE_COSTUME_SYSTEM:
			exception.Abort("What do you do?")
			return
			 	
		ui.ScriptWindow.__init__(self)

		self.isLoaded = 0

		self.__LoadWindow()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def Show(self):
		self.__LoadWindow()
		self.RefreshCostumeSlot()

		ui.ScriptWindow.Show(self)

	def Close(self):
		self.Hide()

	def __LoadWindow(self):
		if self.isLoaded == 1:
			return

		self.isLoaded = 1

		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "UIScript/CostumeWindow.py")
		except:
			import exception
			exception.Abort("CostumeWindow.LoadWindow.LoadObject")

		try:
			wndEquip = self.GetChild("CostumeSlot")
			self.GetChild("TitleBar").SetCloseEvent(ui.__mem_func__(self.Close))

		except:
			import exception
			exception.Abort("CostumeWindow.LoadWindow.BindObject")

		## Equipment
		wndEquip.SetOverInItemEvent(ui.__mem_func__(selfs.wndInventory.OverInItem))
		wndEquip.SetOverOutItemEvent(ui.__mem_func__(selfs.wndInventory.OverOutItem))
		wndEquip.SetUnselectItemSlotEvent(ui.__mem_func__(selfs.wndInventory.UseItemSlot))
		wndEquip.SetUseSlotEvent(ui.__mem_func__(selfs.wndInventory.UseItemSlot))
		wndEquip.SetSelectEmptySlotEvent(ui.__mem_func__(selfs.wndInventory.SelectEmptySlot))
		wndEquip.SetSelectItemSlotEvent(ui.__mem_func__(selfs.wndInventory.SelectItemSlot))

		self.wndEquip = wndEquip

	def RefreshCostumeSlot(self):
		getItemVNum=player.GetItemIndex

		for i in xrange(item.COSTUME_SLOT_COUNT):
			slotNumber = item.COSTUME_SLOT_START + i
			self.wndEquip.SetItemSlot(slotNumber, getItemVNum(slotNumber), 0)

			if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
				itemTransmutedVnum = player.GetItemChangeLook(slotNumber)
				if itemTransmutedVnum:
					self.wndEquip.DisableCoverButton(slotNumber)
				else:
					self.wndEquip.EnableCoverButton(slotNumber)

		if ENABLE_NEW_EQUIPMENT_SYSTEM:
			for i in xrange(player.NEW_EQUIPMENT_SLOT_COUNT):
				slotNumber = player.NEW_EQUIPMENT_SLOT_START + i
				itemCount = player.GetItemCount(slotNumber)
				if itemCount <= 1:
					itemCount = 0
				self.wndEquip.SetItemSlot(slotNumber, getItemVNum(slotNumber), itemCount)

				if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
					itemTransmutedVnum = player.GetItemChangeLook(slotNumber)
					if itemTransmutedVnum:
						self.wndEquip.DisableCoverButton(slotNumber)
					else:
						self.wndEquip.EnableCoverButton(slotNumber)

		self.wndEquip.RefreshSlot()

class BeltInventoryWindow(ui.ScriptWindow):

	def __init__(self, wndInventory):
		import exception

		if not wndInventory:
			exception.Abort("wndInventory parameter must be set to InventoryWindow")
			return

		if not app.ENABLE_NEW_EQUIPMENT_SYSTEM:
			exception.Abort("What do you do?")
			return

		ui.ScriptWindow.__init__(self)

		self.isLoaded = 0
		self.wndInventory = wndInventory;

		self.wndBeltInventoryLayer = None
		self.wndBeltInventorySlot = None
		self.expandBtn = None
		self.minBtn = None

		self.__LoadWindow()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def Show(self, openBeltSlot = False):
		self.__LoadWindow()
		self.RefreshSlot()

		ui.ScriptWindow.Show(self)

		if openBeltSlot:
			self.OpenInventory()
		else:
			self.CloseInventory()

	def Close(self):
		self.Hide()

	def IsOpeningInventory(self):
		return self.wndBeltInventoryLayer.IsShow()

	def OpenInventory(self):
		self.wndBeltInventoryLayer.Show()
		self.expandBtn.Hide()

		if localeInfo.IsARABIC() == 0:
			self.AdjustPositionAndSize()

	def CloseInventory(self):
		self.wndBeltInventoryLayer.Hide()
		self.expandBtn.Show()

		if localeInfo.IsARABIC() == 0:
			self.AdjustPositionAndSize()

	## 현재 인벤토리 위치를 기준으로 BASE 위치를 계산, 리턴.. 숫자 하드코딩하기 정말 싫지만 방법이 없다..
	def GetBasePosition(self):
		x, y = self.wndInventory.GetGlobalPosition()
		return x - 148, y + 241

	def AdjustPositionAndSize(self):
		bx, by = self.GetBasePosition()

		if self.IsOpeningInventory():
			self.SetPosition(bx, by)
			self.SetSize(self.ORIGINAL_WIDTH, self.GetHeight())

		else:
			self.SetPosition(bx + 138, by);
			self.SetSize(10, self.GetHeight())

	def __LoadWindow(self):
		if self.isLoaded == 1:
			return

		self.isLoaded = 1

		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "UIScript/BeltInventoryWindow.py")
		except:
			import exception
			exception.Abort("CostumeWindow.LoadWindow.LoadObject")

		try:
			self.ORIGINAL_WIDTH = self.GetWidth()
			wndBeltInventorySlot = self.GetChild("BeltInventorySlot")
			self.wndBeltInventoryLayer = self.GetChild("BeltInventoryLayer")
			self.expandBtn = self.GetChild("ExpandBtn")
			self.minBtn = self.GetChild("MinimizeBtn")

			self.expandBtn.SetEvent(ui.__mem_func__(self.OpenInventory))
			self.minBtn.SetEvent(ui.__mem_func__(self.CloseInventory))

			if localeInfo.IsARABIC() :
				self.expandBtn.SetPosition(self.expandBtn.GetWidth() - 2, 15)
				self.wndBeltInventoryLayer.SetPosition(self.wndBeltInventoryLayer.GetWidth() - 5, 0)
				self.minBtn.SetPosition(self.minBtn.GetWidth() + 3, 15)

			for i in xrange(item.BELT_INVENTORY_SLOT_COUNT):
				slotNumber = item.BELT_INVENTORY_SLOT_START + i
				wndBeltInventorySlot.SetCoverButton(slotNumber,	"d:/ymir work/ui/game/quest/slot_button_01.sub",\
												"d:/ymir work/ui/game/quest/slot_button_01.sub",\
												"d:/ymir work/ui/game/quest/slot_button_01.sub",\
												"d:/ymir work/ui/game/belt_inventory/slot_disabled.tga", False, False)

		except:
			import exception
			exception.Abort("CostumeWindow.LoadWindow.BindObject")

		## Equipment
		wndBeltInventorySlot.SetOverInItemEvent(ui.__mem_func__(self.wndInventory.OverInItem))
		wndBeltInventorySlot.SetOverOutItemEvent(ui.__mem_func__(self.wndInventory.OverOutItem))
		wndBeltInventorySlot.SetUnselectItemSlotEvent(ui.__mem_func__(self.wndInventory.UseItemSlot))
		wndBeltInventorySlot.SetUseSlotEvent(ui.__mem_func__(self.wndInventory.UseItemSlot))
		wndBeltInventorySlot.SetSelectEmptySlotEvent(ui.__mem_func__(self.wndInventory.SelectEmptySlot))
		wndBeltInventorySlot.SetSelectItemSlotEvent(ui.__mem_func__(self.wndInventory.SelectItemSlot))

		self.wndBeltInventorySlot = wndBeltInventorySlot

	def RefreshSlot(self):
		getItemVNum=player.GetItemIndex

		for i in xrange(item.BELT_INVENTORY_SLOT_COUNT):
			slotNumber = item.BELT_INVENTORY_SLOT_START + i
			self.wndBeltInventorySlot.SetItemSlot(slotNumber, getItemVNum(slotNumber), player.GetItemCount(slotNumber))
			self.wndBeltInventorySlot.SetAlwaysRenderCoverButton(slotNumber, True)

			avail = "0"

			if player.IsAvailableBeltInventoryCell(slotNumber):
				self.wndBeltInventorySlot.EnableCoverButton(slotNumber)
			else:
				self.wndBeltInventorySlot.DisableCoverButton(slotNumber)

		self.wndBeltInventorySlot.RefreshSlot()

if ENABLE_AFAR_BIOLOG:
	class BioWindow(ui.ScriptWindow):
		def __init__(self):					 
			ui.ScriptWindow.__init__(self)

			self.isLoaded = 0
			self.kalan = 0
			self.gerekli = 0
			
			self.wndBioLayer = None
			self.wndBioSlot = None
			self.expandBtn = None
			self.minBtn = None
			
			self.tooltipItem = uiToolTip.ItemToolTip()
			self.tooltipItem.Hide()

			self.__LoadWindow()

		def __del__(self):
			ui.ScriptWindow.__del__(self)

		def Show(self, openBeltSlot = FALSE):
			self.__LoadWindow()
			self.RefreshSlot()

			ui.ScriptWindow.Show(self)
			
			if openBeltSlot:
				self.OpenInventory()
			else:
				self.CloseInventory()

		def Close(self):
			self.Hide()

		def IsOpeningInventory(self):
			return self.wndBioLayer.IsShow()
			
		def OpenInventory(self):
			self.wndBioLayer.Show()
			self.expandBtn.Hide()

			if localeInfo.IsARABIC() == 0:
				self.AdjustPositionAndSize()
					
		def CloseInventory(self):
			self.wndBioLayer.Hide()
			self.expandBtn.Show()
			
			if localeInfo.IsARABIC() == 0:
				self.AdjustPositionAndSize()

		def GetBasePosition(self):
			x, y = selfs.wndInventory.GetGlobalPosition()
			return x - 148, y + 241+90+30+5+9
			
		def AdjustPositionAndSize(self):
			bx, by = self.GetBasePosition()
			
			if self.IsOpeningInventory():			
				self.SetPosition(bx, by)
				self.SetSize(self.ORIGINAL_WIDTH, self.GetHeight())
			else:
				self.SetPosition(bx + 138, by);
				self.SetSize(10, self.GetHeight())
			
		def __LoadWindow(self):
			if self.isLoaded == 1:
				return

			self.isLoaded = 1

			try:
				pyScrLoader = ui.PythonScriptLoader()
				pyScrLoader.LoadScriptFile(self, "UIScript/afarbiowindow2.py")
			except:
				import exception
				exception.Abort("CostumeWindow.LoadWindow.LoadObject")

			try:
				self.ORIGINAL_WIDTH = self.GetWidth()
				wndBioSlot = self.GetChild("BeltInventorySlot")
				self.wndBioLayer = self.GetChild("BeltInventoryLayer")
				self.expandBtn = self.GetChild("ExpandBtn")
				self.minBtn = self.GetChild("MinimizeBtn")
				self.counttext = self.GetChild("count")
				self.sure = self.GetChild("time")
				self.verbutton = self.GetChild("biobutton")
				
				self.expandBtn.SetEvent(ui.__mem_func__(self.OpenInventory))
				self.minBtn.SetEvent(ui.__mem_func__(self.CloseInventory))
				self.verbutton.SetEvent(ui.__mem_func__(self.Send))
				
				if localeInfo.IsARABIC() :
					self.expandBtn.SetPosition(self.expandBtn.GetWidth() - 2, 15)
					self.wndBioLayer.SetPosition(self.wndBioLayer.GetWidth() - 5, 0)
					self.minBtn.SetPosition(self.minBtn.GetWidth() + 3, 15)			
		
				for i in xrange(item.BELT_INVENTORY_SLOT_COUNT):
					slotNumber = item.BELT_INVENTORY_SLOT_START + i							
					wndBioSlot.SetCoverButton(slotNumber,	"d:/ymir work/ui/game/quest/slot_button_01.sub",\
													"d:/ymir work/ui/game/quest/slot_button_01.sub",\
													"d:/ymir work/ui/game/quest/slot_button_01.sub",\
													"d:/ymir work/ui/game/belt_inventory/slot_disabled.tga", FALSE, FALSE)									
				
			except:
				import exception
				exception.Abort("CostumeWindow.LoadWindow.BindObject")

			wndBioSlot.SetOverInItemEvent(ui.__mem_func__(self.OverInItem2))
			wndBioSlot.SetOverOutItemEvent(ui.__mem_func__(self.OverOutItem2))
			self.wndBioSlot = wndBioSlot
			
		def Send(self):
			net.SendChatPacket("/afarbio")
		
		def SetBiyolog(self, bioitem, verilen, toplam, kalansure):
			self.gerekli = int(bioitem)
			self.kalan = int(kalansure)
			self.counttext.SetText(str(verilen)+"/"+str(toplam))
			self.RefreshSlot()
		
		def OverInItem2(self):
			itemIndex = int(self.gerekli)
			item.SelectItem(itemIndex)
			
			taslarr = [player.GetItemMetinSocket(999, i) for i in xrange(player.METIN_SOCKET_MAX_NUM)]
			efsunlarr = [player.GetItemAttribute(999, i) for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]

			self.tooltipItem.ClearToolTip()
			self.tooltipItem.AddRefineItemData(itemIndex, taslarr, efsunlarr)
			self.tooltipItem.Show()

		def OverOutItem2(self):
			self.tooltipItem.ClearToolTip()
			self.tooltipItem.Hide()
			
		def OnUpdate(self):
			import time
			current_milli_time = int(app.GetGlobalTimeStamp())
			if int(self.kalan) == 0:
				self.sure.SetText("Verilebilir!")
			elif (int(self.kalan)-current_milli_time <= 0):
				self.sure.SetText("Verilebilir!")
			else:
				self.sure.SetText(str(localeInfo.SecondToDHM(int(self.kalan)-current_milli_time)))
				
		def RefreshSlot(self):
			getItemVNum= int (self.gerekli)
			
			if getItemVNum > 0:
				self.wndBioSlot.SetItemSlot(999, getItemVNum)
			else:	
				self.wndBioSlot.ClearSlot(999)
				return
				
			self.wndBioSlot.RefreshSlot()		

class InventoryWindow(ui.ScriptWindow):

	USE_TYPE_TUPLE = ["USE_CLEAN_SOCKET", "USE_CHANGE_ATTRIBUTE", "USE_ADD_ATTRIBUTE", "USE_ADD_ATTRIBUTE2", "USE_ADD_ACCESSORY_SOCKET", "USE_PUT_INTO_ACCESSORY_SOCKET", "USE_PUT_INTO_BELT_SOCKET", "USE_PUT_INTO_RING_SOCKET"]
	if ENABLE_MOVE_COSTUME_ATTR:
		USE_TYPE_TUPLE += ["USE_ADD_COSTUME_ATTRIBUTE", "USE_RESET_COSTUME_ATTR"]

	questionDialog = None
	tooltipItem = None
	wndCostume = None
	wndBelt = None

	dlgPickMoney = None
	if ENABLE_FAST_USE_GIFTBOX:
		dlgUseItem = None

	sellingSlotNumber = -1
	isLoaded = 0
	isOpenedCostumeWindowWhenClosingInventory = 0		# 인벤토리 닫을 때 코스츔이 열려있었는지 여부-_-; 네이밍 ㅈㅅ
	isOpenedBeltWindowWhenClosingInventory = 0		# 인벤토리 닫을 때 벨트 인벤토리가 열려있었는지 여부-_-; 네이밍 ㅈㅅ

	if ENABLE_MALLBUTTON_RENEWAL:
		choix = None
		EntrepotIs = None
		Magasinier = None

	if WJ_ENABLE_TRADABLE_ICON:
		listUnusableSlot = []

	if ENABLE_AFAR_BIOLOG:
		wndBio = None
		isOpenedBioInventory = 0

	if ENABLE_NEW_BUTTONS:
		newButtonList = []

	def __init__(self):
		ui.ScriptWindow.__init__(self)

		self.isOpenedBeltWindowWhenClosingInventory = 0		# 인벤토리 닫을 때 벨트 인벤토리가 열려있었는지 여부-_-; 네이밍 ㅈㅅ

		self.inventoryPageIndex = 0 # @@@correction052

		if ENABLE_EXTEND_INVEN_SYSTEM:
			self.categoryTab = []
			self.categoryPageIndex = 0

		if ENABLE_AFAR_BIOLOG:
			self.isOpenedBioInventory = 0

		if ENABLE_NEW_BUTTONS:
			self.newButtonList = []

		self.__LoadWindow()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def Show(self):
		self.__LoadWindow()

		ui.ScriptWindow.Show(self)

		# 인벤토리를 닫을 때 코스츔이 열려있었다면 인벤토리를 열 때 코스츔도 같이 열도록 함.
		if self.isOpenedCostumeWindowWhenClosingInventory and self.wndCostume:
			self.wndCostume.Show()

		# 인벤토리를 닫을 때 벨트 인벤토리가 열려있었다면 같이 열도록 함.
		if self.wndBelt:
			self.wndBelt.Show(self.isOpenedBeltWindowWhenClosingInventory)

		if ENABLE_AFAR_BIOLOG:
			if self.wndBio:
				self.wndBio.Show(self.isOpenedBioInventory)

	def __LoadWindow(self):
		if self.isLoaded == 1:
			return

		self.isLoaded = 1

		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/InventoryWindow.py")
		except:
			import exception
			exception.Abort("InventoryWindow.LoadWindow.LoadObject")

		try:
			wndItem = self.GetChild("ItemSlot")
			wndEquip = self.GetChild("EquipmentSlot")
			self.GetChild("TitleBar").SetCloseEvent(ui.__mem_func__(self.Close))
			self.mallButton = self.GetChild2("MallButton")
			self.DSSButton = self.GetChild2("DSSButton")
			self.costumeButton = self.GetChild2("CostumeButton")

			self.inventoryTab = []
			for i in xrange(player.INVENTORY_PAGE_COUNT): # @@@correction052
				self.inventoryTab.append(self.GetChild("Inventory_Tab_%02d" % (i+1)))

			self.equipmentTab = []
			self.equipmentTab.append(self.GetChild("Equipment_Tab_01"))
			self.equipmentTab.append(self.GetChild("Equipment_Tab_02"))

			if self.costumeButton and not app.ENABLE_COSTUME_SYSTEM:
				self.costumeButton.Hide()
				self.costumeButton.Destroy()
				self.costumeButton = 0

			# Belt Inventory Window
			self.wndBelt = None

			if ENABLE_NEW_EQUIPMENT_SYSTEM:
				self.wndBelt = BeltInventoryWindow(self)

			if ENABLE_EXTEND_INVEN_SYSTEM:
				for i in xrange(player.INVENTORY_MAX):
					self.categoryTab.append(self.GetChild("Category_Tab_%d" % i))

			if ENABLE_SORT_ITEMS:
				self.yenilebutton = self.GetChild2("YenileButton")

			self.wndBio = None
			if ENABLE_AFAR_BIOLOG:
				self.wndBio = BioWindow()

			if ENABLE_NEW_BUTTONS:
				for i in xrange(3):
					self.newButtonList.append(self.GetChild("New_Button_%d" % i))

		except:
			import exception
			exception.Abort("InventoryWindow.LoadWindow.BindObject")

		## Item
		wndItem.SetSelectEmptySlotEvent(ui.__mem_func__(self.SelectEmptySlot))
		wndItem.SetSelectItemSlotEvent(ui.__mem_func__(self.SelectItemSlot))
		wndItem.SetUnselectItemSlotEvent(ui.__mem_func__(self.UseItemSlot))
		wndItem.SetUseSlotEvent(ui.__mem_func__(self.UseItemSlot))
		wndItem.SetOverInItemEvent(ui.__mem_func__(self.OverInItem))
		wndItem.SetOverOutItemEvent(ui.__mem_func__(self.OverOutItem))

		## Equipment
		wndEquip.SetSelectEmptySlotEvent(ui.__mem_func__(self.SelectEmptySlot))
		wndEquip.SetSelectItemSlotEvent(ui.__mem_func__(self.SelectItemSlot))
		wndEquip.SetUnselectItemSlotEvent(ui.__mem_func__(self.UseItemSlot))
		wndEquip.SetUseSlotEvent(ui.__mem_func__(self.UseItemSlot))
		wndEquip.SetOverInItemEvent(ui.__mem_func__(self.OverInItem))
		wndEquip.SetOverOutItemEvent(ui.__mem_func__(self.OverOutItem))

		## PickMoneyDialog
		dlgPickMoney = None
		if ENABLE_SPILT_ITEMS:
			import uiPickItem
			dlgPickMoney = uiPickItem.PickItemDialog()
		else:
			dlgPickMoney = uiPickMoney.PickMoneyDialog()
		dlgPickMoney.LoadDialog()
		dlgPickMoney.Hide()

		dlgUseItem = None
		if ENABLE_FAST_USE_GIFTBOX:
			dlgUseItem = uiPickMoney.PickMoneyDialog()
			dlgUseItem.LoadDialog()
			dlgUseItem.Hide()

		## RefineDialog
		self.refineDialog = uiRefine.RefineDialog()
		self.refineDialog.Hide()

		if WJ_ELDER_ATTRIBUTE_SYSTEM: # @@@correction026
			self.attachBonusDialog = uiAttachBonus.AttachBonusDialog()
			self.attachBonusDialog.Hide()

		## AttachMetinDialog
		self.attachMetinDialog = uiAttachMetin.AttachMetinDialog()
		self.attachMetinDialog.Hide()

		if not ENABLE_EXPANDED_TASKBAR: # @@@correction047
			## MoneySlot
			self.wndMoneySlot.SetEvent(ui.__mem_func__(self.OpenPickMoneyDialog))

		for i in xrange(player.INVENTORY_PAGE_COUNT): # @@@correction052
			self.inventoryTab[i].SetEvent(lambda arg=i: self.SetInventoryPage(arg))
		self.inventoryTab[0].Down()

		self.equipmentTab[0].SetEvent(lambda arg=0: self.SetEquipmentPage(arg))
		self.equipmentTab[1].SetEvent(lambda arg=1: self.SetEquipmentPage(arg))
		self.equipmentTab[0].Down()
		self.equipmentTab[0].Hide()
		self.equipmentTab[1].Hide()

		self.wndItem = wndItem
		self.wndEquip = wndEquip
		self.dlgPickMoney = dlgPickMoney
		if ENABLE_FAST_USE_GIFTBOX:
			self.dlgUseItem = dlgUseItem

		# MallButton
		if self.mallButton:
			self.mallButton.SetEvent(ui.__mem_func__(self.ClickMallButton))

		if self.DSSButton:
			self.DSSButton.SetEvent(ui.__mem_func__(self.ClickDSSButton))

		# Costume Button
		if self.costumeButton:
			self.costumeButton.SetEvent(ui.__mem_func__(self.ClickCostumeButton))

		self.wndCostume = None

		if ENABLE_EXTEND_INVEN_SYSTEM:
			for i in xrange(player.INVENTORY_MAX):
				self.categoryTab[i].SetEvent(lambda arg = i: self.SetCategoryPage(arg))

			self.categoryTab[0].Down()

		if ENABLE_NEW_BUTTONS:
			for i in xrange(3):
				self.newButtonList[i].SetEvent(lambda arg = i: self.SetNewButtons(arg))

		if ENABLE_SORT_ITEMS:
			if self.yenilebutton:
				self.yenilebutton.SetEvent(ui.__mem_func__(self.ClickYenileButton))

		## Refresh
		self.SetInventoryPage(0)
		self.SetEquipmentPage(0)
		self.RefreshItemSlot()
		if not ENABLE_EXPANDED_TASKBAR: # @@@correction047
			self.RefreshStatus()

		if ENABLE_MALLBUTTON_RENEWAL:
			self.choix = ui.BoardWithTitleBar()
			self.choix.SetSize(210, 80)
			self.choix.SetCenterPosition()
			self.choix.AddFlag('float')
			self.choix.AddFlag('movable')
			self.choix.SetTitleName(uiScriptLocale.PASSWORD_DESC_1)
			self.choix.Hide()

			self.EntrepotIs = ui.Button()
			self.EntrepotIs.SetEvent(self.OpenIs)
			self.EntrepotIs.SetParent(self.choix)
			self.EntrepotIs.SetPosition(35, 40)
			self.EntrepotIs.SetUpVisual("d:/ymir work/ui/public/middle_button_01.sub")
			self.EntrepotIs.SetOverVisual("d:/ymir work/ui/public/middle_button_02.sub")
			self.EntrepotIs.SetDownVisual("d:/ymir work/ui/public/middle_button_03.sub")
			self.EntrepotIs.SetText("N.Market")
			self.EntrepotIs.SetToolTipText("Nesne Market")
			self.EntrepotIs.Show()

			self.Magasinier = ui.Button()
			self.Magasinier.SetEvent(self._normal_mall)
			self.Magasinier.SetParent(self.choix)
			self.Magasinier.SetPosition(105, 40)
			self.Magasinier.SetUpVisual("d:/ymir work/ui/public/middle_button_01.sub")
			self.Magasinier.SetOverVisual("d:/ymir work/ui/public/middle_button_02.sub")
			self.Magasinier.SetDownVisual("d:/ymir work/ui/public/middle_button_03.sub")
			self.Magasinier.SetText(uiScriptLocale.SAFE_TITLE)
			self.Magasinier.SetToolTipText("Depo")
			self.Magasinier.Show()

		if WJ_ENABLE_TRADABLE_ICON:
			self.listUnusableSlot = []

	def Destroy(self):
		self.ClearDictionary()

		self.dlgPickMoney.Destroy()
		self.dlgPickMoney = 0

		self.refineDialog.Destroy()
		self.refineDialog = 0

		self.attachMetinDialog.Destroy()
		self.attachMetinDialog = 0

		if WJ_ELDER_ATTRIBUTE_SYSTEM: # @@@correction026
			self.attachBonusDialog.Destroy()
			self.attachBonusDialog = 0

		self.wndItem = 0
		self.wndEquip = 0
		if not ENABLE_EXPANDED_TASKBAR: # @@@correction047
			self.wndMoney = 0
			self.wndMoneySlot = 0
		self.questionDialog = None
		self.mallButton = None

		self.DSSButton = None

		if self.wndCostume:
			self.wndCostume.Destroy()
			self.wndCostume = 0

		if self.wndBelt:
			self.wndBelt.Destroy()
			self.wndBelt = None

		if ENABLE_AFAR_BIOLOG:
			if self.wndBio:
				self.wndBio.Destroy()
				self.wndBio = None

		self.inventoryTab = []
		self.equipmentTab = []

		if ENABLE_EXTEND_INVEN_SYSTEM:
			self.categoryTab = []
			self.categoryPageIndex = 0

		if ENABLE_FAST_USE_GIFTBOX:
			self.dlgUseItem.Destroy()
			self.dlgUseItem = 0

	def Hide(self):
		if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS():
			self.OnCloseQuestionDialog()
			return
		if None != selfs.wndToolTipItem:
			selfs.wndToolTipItem.HideToolTip()

		if self.wndCostume:
			self.isOpenedCostumeWindowWhenClosingInventory = self.wndCostume.IsShow()			# 인벤토리 창이 닫힐 때 코스츔이 열려 있었는가?
			self.wndCostume.Close()

		if self.wndBelt:
			self.isOpenedBeltWindowWhenClosingInventory = self.wndBelt.IsOpeningInventory()		# 인벤토리 창이 닫힐 때 벨트 인벤토리도 열려 있었는가?
			print "Is Opening Belt Inven?? ", self.isOpenedBeltWindowWhenClosingInventory
			self.wndBelt.Close()

		if ENABLE_AFAR_BIOLOG:
			if self.wndBio:
				self.isOpenedBioInventory = self.wndBio.IsOpeningInventory()		
				print "Is Opening bio Inven ", self.isOpenedBioInventory
				self.wndBio.Close()

		if self.dlgPickMoney:
			self.dlgPickMoney.Close()

		if ENABLE_FAST_USE_GIFTBOX:
			if self.dlgUseItem:
				self.dlgUseItem.Close()

		wndMgr.Hide(self.hWnd)

	def Close(self):
		self.Hide()

	def SetInventoryPage(self, page):
		self.inventoryPageIndex = page
		for i in xrange(player.INVENTORY_PAGE_COUNT): # @@@correction052
			if i!=page:
				self.inventoryTab[i].SetUp()
		self.RefreshBagSlotWindow()

	def SetEquipmentPage(self, page):
		self.equipmentPageIndex = page
		self.equipmentTab[1-page].SetUp()
		self.RefreshEquipSlotWindow()

	def ClickMallButton(self):
		if ENABLE_MALLBUTTON_RENEWAL:
			self.choix.Show()
		else:
			net.SendChatPacket("/click_mall")

	if ENABLE_MALLBUTTON_RENEWAL:
		def OpenIs(self):
			self.choix.Hide()
			net.SendChatPacket("/click_mall")
	   
		def _normal_mall(self):
			self.choix.Hide()
			net.SendChatPacket("/click_safebox")

	# DSSButton
	def ClickDSSButton(self):
		print "click_dss_button"
		selfs.wndGame.ToggleDragonSoulWindow()

	def ClickCostumeButton(self):
		print "Click Costume Button"
		if self.wndCostume:
			if self.wndCostume.IsShow():
				self.wndCostume.Hide()
			else:
				self.wndCostume.Show()
		else:
			self.wndCostume = CostumeWindow()
			self.wndCostume.Show()

	def OpenPickMoneyDialog(self):

		if mouseModule.mouseController.isAttached():

			attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
			if player.SLOT_TYPE_SAFEBOX == mouseModule.mouseController.GetAttachedType():

				if player.ITEM_MONEY == mouseModule.mouseController.GetAttachedItemIndex():
					net.SendSafeboxWithdrawMoneyPacket(mouseModule.mouseController.GetAttachedItemCount())
					snd.PlaySound("sound/ui/money.wav")

			mouseModule.mouseController.DeattachObject()

		else:
			curMoney = player.GetElk()

			if curMoney <= 0:
				return

			self.dlgPickMoney.SetTitleName(localeInfo.PICK_MONEY_TITLE)
			self.dlgPickMoney.SetAcceptEvent(ui.__mem_func__(self.OnPickMoney))
			self.dlgPickMoney.Open(curMoney)
			self.dlgPickMoney.SetMax(9) # 인벤토리 990000 제한 버그 수정

	def OnPickMoney(self, money):
		mouseModule.mouseController.AttachMoney(self, player.SLOT_TYPE_INVENTORY, money)

	def OnPickItem(self, count):
		itemSlotIndex = self.dlgPickMoney.itemGlobalSlotIndex
		selectedItemVNum = player.GetItemIndex(itemSlotIndex)
		mouseModule.mouseController.AttachObject(self, player.SLOT_TYPE_INVENTORY, itemSlotIndex, selectedItemVNum, count)

	def __InventoryLocalSlotPosToGlobalSlotPos(self, local):
		if player.IsEquipmentSlot(local) or player.IsCostumeSlot(local) or (app.ENABLE_NEW_EQUIPMENT_SYSTEM and player.IsBeltInventorySlot(local)):
			return local

		if ENABLE_EXTEND_INVEN_SYSTEM:
			return (self.inventoryPageIndex * player.INVENTORY_PAGE_SIZE) + (self.categoryPageIndex * player.ITEM_SLOT_ONE_COUNT) + local
		else:
			return (self.inventoryPageIndex * player.INVENTORY_PAGE_SIZE) + local

	def RefreshBagSlotWindow(self):
		getItemVNum=player.GetItemIndex
		getItemCount=player.GetItemCount
		setItemVNum=self.wndItem.SetItemSlot

		for i in xrange(player.INVENTORY_PAGE_SIZE):
			slotNumber = self.__InventoryLocalSlotPosToGlobalSlotPos(i)

			itemCount = getItemCount(slotNumber)
			# itemCount == 0이면 소켓을 비운다.
			if 0 == itemCount:
				self.wndItem.ClearSlot(i)
				continue
			elif 1 == itemCount:
				itemCount = 0

			itemVnum = getItemVNum(slotNumber)
			setItemVNum(i, itemVnum, itemCount)

			if WJ_ENABLE_TRADABLE_ICON:
				if ((shop.IsOpen() and item.IsAntiFlag(item.ANTIFLAG_SELL)) or\
				(exchange.isTrading() and item.IsAntiFlag(item.ANTIFLAG_GIVE)) or\
				(selfs.wndPrivateShopBuilder and selfs.wndPrivateShopBuilder.IsShow() and item.IsAntiFlag(item.ITEM_ANTIFLAG_MYSHOP)) or\
				(selfs.wndSafebox and selfs.wndSafebox.IsShow() and item.IsAntiFlag(item.ANTIFLAG_SAFEBOX))):
					self.wndItem.SetUnusableSlotOnTopWnd(i)
					self.listUnusableSlot.append(i)
				elif slotNumber in self.listUnusableSlot and\
				(not shop.IsOpen() and item.IsAntiFlag(item.ANTIFLAG_SELL) or\
				(not exchange.isTrading() and item.IsAntiFlag(item.ANTIFLAG_GIVE)) or\
				(selfs.wndPrivateShopBuilder and not selfs.wndPrivateShopBuilder.IsShow() and item.IsAntiFlag(item.ITEM_ANTIFLAG_MYSHOP)) or\
				(selfs.wndSafebox and not selfs.wndSafebox.IsShow() and item.IsAntiFlag(item.ANTIFLAG_SAFEBOX))):
					self.wndItem.SetUsableSlotOnTopWnd(i)
					self.listUnusableSlot.remove(i)

			if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
				itemTransmutedVnum = player.GetItemChangeLook(slotNumber)
				if itemTransmutedVnum:
					self.wndItem.DisableCoverButton(i)
				else:
					self.wndItem.EnableCoverButton(i)

			## 자동물약 (HP: #72723 ~ #72726, SP: #72727 ~ #72730) 특수처리 - 아이템인데도 슬롯에 활성화/비활성화 표시를 위한 작업임 - [hyo]
			if constInfo.IS_AUTO_POTION(itemVnum):
				# metinSocket - [0] : 활성화 여부, [1] : 사용한 양, [2] : 최대 용량
				metinSocket = [player.GetItemMetinSocket(slotNumber, j) for j in xrange(player.METIN_SOCKET_MAX_NUM)]

				# if slotNumber >= player.INVENTORY_PAGE_SIZE:
					# slotNumber -= player.INVENTORY_PAGE_SIZE

				isActivated = 0 != metinSocket[0]

				if isActivated:
					self.wndItem.ActivateSlot(i)
					potionType = 0;
					if constInfo.IS_AUTO_POTION_HP(itemVnum):
						potionType = player.AUTO_POTION_TYPE_HP
					elif constInfo.IS_AUTO_POTION_SP(itemVnum):
						potionType = player.AUTO_POTION_TYPE_SP

					usedAmount = int(metinSocket[1])
					totalAmount = int(metinSocket[2])
					player.SetAutoPotionInfo(potionType, isActivated, (totalAmount - usedAmount), totalAmount, self.__InventoryLocalSlotPosToGlobalSlotPos(i))

				else:
					self.wndItem.DeactivateSlot(i)

			if UNLIMITED_POTIONS:
				item.SelectItem(itemVnum)
				if item.GetValue(3) == 4248 or item.GetValue(3) == 42480:
					metinSocket = [player.GetItemMetinSocket(slotNumber, j) for j in xrange(player.METIN_SOCKET_MAX_NUM)]
					isActivated = 0 != metinSocket[0]
					if isActivated:
						self.wndItem.ActivateSlot(i)
					else:
						self.wndItem.DeactivateSlot(i)

			# if ENABLE_GROWTH_PET_SYSTEM and item.ITEM_TYPE_PET == item.GetItemType() and item.PET_UPBRINGING == item.GetItemSubType():
				# item.SelectItem(itemVnum)
				# attrs = [player.GetItemAttribute(slotNumber, i) for i in xrange(player.ATTRIBUTE_SLOT_MAX_NUM)]
				# if attrs[4][1]:
					# self.wndItem.ActivateSlot(i)
				# else:
					# self.wndItem.DeactivateSlot(i)

		self.wndItem.RefreshSlot()

		if self.wndBelt:
			self.wndBelt.RefreshSlot()

		if ENABLE_AFAR_BIOLOG:
			if self.wndBio:
				self.wndBio.RefreshSlot()

	def RefreshEquipSlotWindow(self):
		getItemVNum=player.GetItemIndex
		getItemCount=player.GetItemCount
		setItemVNum=self.wndEquip.SetItemSlot
		for i in xrange(player.EQUIPMENT_PAGE_COUNT):
			slotNumber = player.EQUIPMENT_SLOT_START + i
			itemCount = getItemCount(slotNumber)
			if itemCount <= 1:
				itemCount = 0
			setItemVNum(slotNumber, getItemVNum(slotNumber), itemCount)
			# TraceError("%d %d" % (slotNumber, getItemVNum(slotNumber)))

			if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
				itemTransmutedVnum = player.GetItemChangeLook(slotNumber)
				if itemTransmutedVnum:
					self.wndEquip.DisableCoverButton(slotNumber)
				else:
					self.wndEquip.EnableCoverButton(slotNumber)

		if ENABLE_NEW_EQUIPMENT_SYSTEM:
			for i in xrange(player.NEW_EQUIPMENT_SLOT_COUNT):
				slotNumber = player.NEW_EQUIPMENT_SLOT_START + i
				itemCount = getItemCount(slotNumber)
				if itemCount <= 1:
					itemCount = 0
				setItemVNum(slotNumber, getItemVNum(slotNumber), itemCount)

				if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
					itemTransmutedVnum = player.GetItemChangeLook(slotNumber)
					if itemTransmutedVnum:
						self.wndEquip.DisableCoverButton(slotNumber)
					else:
						self.wndEquip.EnableCoverButton(slotNumber)

				# print "ENABLE_NEW_EQUIPMENT_SYSTEM", slotNumber, itemCount, getItemVNum(slotNumber)

		self.wndEquip.RefreshSlot()

		if self.wndCostume:
			self.wndCostume.RefreshCostumeSlot()

	def RefreshItemSlot(self):
		self.RefreshBagSlotWindow()
		self.RefreshEquipSlotWindow()

	if not ENABLE_EXPANDED_TASKBAR: # @@@correction047
		def RefreshStatus(self):
			money = player.GetElk()
			self.wndMoney.SetText(localeInfo.NumberToMoneyString(money))

	def SellItem(self):
		if self.sellingSlotitemIndex == player.GetItemIndex(self.sellingSlotNumber):
			if self.sellingSlotitemCount == player.GetItemCount(self.sellingSlotNumber):
				## 용혼석도 팔리게 하는 기능 추가하면서 인자 type 추가
				net.SendShopSellPacketNew(self.sellingSlotNumber, self.questionDialog.count, player.INVENTORY)
				snd.PlaySound("sound/ui/money.wav")
		self.OnCloseQuestionDialog()

	def OnDetachMetinFromItem(self):
		if None == self.questionDialog:
			return

		#net.SendItemUseToItemPacket(self.questionDialog.sourcePos, self.questionDialog.targetPos)
		self.__SendUseItemToItemPacket(self.questionDialog.sourcePos, self.questionDialog.targetPos)
		self.OnCloseQuestionDialog()

	def OnCloseQuestionDialog(self):
		if not self.questionDialog:
			return

		self.questionDialog.Close()
		self.questionDialog = None
		constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(0)

	## Slot Event
	def SelectEmptySlot(self, selectedSlotPos):
		if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
			return

		selectedSlotPos = self.__InventoryLocalSlotPosToGlobalSlotPos(selectedSlotPos)

		if mouseModule.mouseController.isAttached():

			attachedSlotType = mouseModule.mouseController.GetAttachedType()
			attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
			attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()
			attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()

			if player.SLOT_TYPE_INVENTORY == attachedSlotType:
				# @@@correction010 BEGIN (block ds equip)
				attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)
				if player.IsDSEquipmentSlot(attachedInvenType, attachedSlotPos):
					mouseModule.mouseController.DeattachObject()
					return
				# @@@correction010 END

				itemCount = player.GetItemCount(attachedSlotPos)
				attachedCount = mouseModule.mouseController.GetAttachedItemCount()
				if ENABLE_SPILT_ITEMS and self.dlgPickMoney and self.dlgPickMoney.IsSplitAll():
					net.SendChatPacket("/split_items %d %d %d" % (attachedSlotPos, attachedCount, selectedSlotPos))
					self.dlgPickMoney.SplitClear()
				else:
					self.__SendMoveItemPacket(attachedSlotPos, selectedSlotPos, attachedCount)

				if item.IsRefineScroll(attachedItemIndex):
					self.wndItem.SetUseMode(False)
					
			elif ENABLE_SWITCHBOT and player.SLOT_TYPE_SWITCHBOT == attachedSlotType:
				attachedCount = mouseModule.mouseController.GetAttachedItemCount()
				net.SendItemMovePacket(player.SWITCHBOT, attachedSlotPos, player.INVENTORY, selectedSlotPos, attachedCount)

			elif player.SLOT_TYPE_PRIVATE_SHOP == attachedSlotType:
				mouseModule.mouseController.RunCallBack("INVENTORY")

			elif player.SLOT_TYPE_SHOP == attachedSlotType:
				net.SendShopBuyPacket(attachedSlotPos)

			elif player.SLOT_TYPE_SAFEBOX == attachedSlotType:

				if player.ITEM_MONEY == attachedItemIndex:
					net.SendSafeboxWithdrawMoneyPacket(mouseModule.mouseController.GetAttachedItemCount())
					snd.PlaySound("sound/ui/money.wav")

				else:
					net.SendSafeboxCheckoutPacket(attachedSlotPos, selectedSlotPos)

			elif player.SLOT_TYPE_MALL == attachedSlotType:
				net.SendMallCheckoutPacket(attachedSlotPos, selectedSlotPos)

			mouseModule.mouseController.DeattachObject()

	def SelectItemSlot(self, itemSlotIndex):
		if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
			return

		itemSlotIndex = self.__InventoryLocalSlotPosToGlobalSlotPos(itemSlotIndex)

		if mouseModule.mouseController.isAttached():
			attachedSlotType = mouseModule.mouseController.GetAttachedType()
			attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
			attachedItemVID = mouseModule.mouseController.GetAttachedItemIndex()

			if player.SLOT_TYPE_INVENTORY == attachedSlotType:
				# @@@correction010 BEGIN (block ds equip)
				attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)
				if player.IsDSEquipmentSlot(attachedInvenType, attachedSlotPos):
					mouseModule.mouseController.DeattachObject()
					return
				# @@@correction010 END
				self.__DropSrcItemToDestItemInInventory(attachedItemVID, attachedSlotPos, itemSlotIndex)

			mouseModule.mouseController.DeattachObject()

		else:

			curCursorNum = app.GetCursor()
			if app.SELL == curCursorNum:
				self.__SellItem(itemSlotIndex)

			elif app.BUY == curCursorNum:
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.SHOP_BUY_INFO)

			elif app.IsPressed(app.DIK_LALT):
				link = player.GetItemLink(itemSlotIndex)
				ime.PasteString(link)

			elif app.IsPressed(app.DIK_LSHIFT):
				itemCount = player.GetItemCount(itemSlotIndex)

				if itemCount > 1:
					self.dlgPickMoney.SetTitleName(localeInfo.PICK_ITEM_TITLE)
					self.dlgPickMoney.SetAcceptEvent(ui.__mem_func__(self.OnPickItem))
					self.dlgPickMoney.Open(itemCount)
					self.dlgPickMoney.itemGlobalSlotIndex = itemSlotIndex
				#else:
					#selectedItemVNum = player.GetItemIndex(itemSlotIndex)
					#mouseModule.mouseController.AttachObject(self, player.SLOT_TYPE_INVENTORY, itemSlotIndex, selectedItemVNum)

			elif app.IsPressed(app.DIK_LCONTROL):
				itemIndex = player.GetItemIndex(itemSlotIndex)

				if True == item.CanAddToQuickSlotItem(itemIndex):
					player.RequestAddToEmptyLocalQuickSlot(player.SLOT_TYPE_INVENTORY, itemSlotIndex)
				else:
					chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.QUICKSLOT_REGISTER_DISABLE_ITEM)

			else:
				selectedItemVNum = player.GetItemIndex(itemSlotIndex)
				itemCount = player.GetItemCount(itemSlotIndex)
				mouseModule.mouseController.AttachObject(self, player.SLOT_TYPE_INVENTORY, itemSlotIndex, selectedItemVNum, itemCount)

				if self.__IsUsableItemToItem(selectedItemVNum, itemSlotIndex):
					self.wndItem.SetUseMode(True)
				else:
					self.wndItem.SetUseMode(False)

				snd.PlaySound("sound/ui/pick.wav")

	def __DropSrcItemToDestItemInInventory(self, srcItemVID, srcItemSlotPos, dstItemSlotPos):
		if srcItemSlotPos == dstItemSlotPos:
			return

		if ENABLE_GROWTH_PET_SYSTEM:
			if constInfo.IS_NEW_PET(srcItemVID) and player.GetItemIndex(dstItemSlotPos) == 55002:
				self.questionDialog = uiCommon.QuestionDialog()
				self.questionDialog.SetText(localeInfo.PET_SYSTEM_ADD_BOX_PET)
				self.questionDialog.SetAcceptEvent(ui.__mem_func__(self.UseTransportBox))
				self.questionDialog.SetCancelEvent(ui.__mem_func__(self.OnCloseQuestionDialog))
				self.questionDialog.Open()
				self.questionDialog.src = srcItemSlotPos
				self.questionDialog.dst = dstItemSlotPos
				
			if srcItemVID == 55001 and constInfo.IS_NEW_PET(player.GetItemIndex(dstItemSlotPos)):
				self.questionDialog = uiCommon.QuestionDialog()
				self.questionDialog.SetText(localeInfo.PET_SYSTEM_FEED_PROTEIN)
				self.questionDialog.SetAcceptEvent(ui.__mem_func__(self.UseProtein))
				self.questionDialog.SetCancelEvent(ui.__mem_func__(self.OnCloseQuestionDialog))
				self.questionDialog.Open()
				self.questionDialog.src = srcItemSlotPos
				self.questionDialog.dst = dstItemSlotPos

			if constInfo.IS_NEW_PET(player.GetItemIndex(dstItemSlotPos)) and srcItemVID == 55030:
				selfs.wndGame.OpenInputNameDialogPet(dstItemSlotPos)
				return

		if item.IsRefineScroll(srcItemVID):
			self.RefineItem(srcItemSlotPos, dstItemSlotPos)
			self.wndItem.SetUseMode(False)

		elif item.IsMetin(srcItemVID):
			self.AttachMetinToItem(srcItemSlotPos, dstItemSlotPos)

		elif item.IsDetachScroll(srcItemVID):
			self.DetachMetinFromItem(srcItemSlotPos, dstItemSlotPos)

		elif item.IsKey(srcItemVID):
			self.__SendUseItemToItemPacket(srcItemSlotPos, dstItemSlotPos)

		elif (player.GetItemFlags(srcItemSlotPos) & ITEM_FLAG_APPLICABLE) == ITEM_FLAG_APPLICABLE:
			self.__SendUseItemToItemPacket(srcItemSlotPos, dstItemSlotPos)

		elif item.GetUseType(srcItemVID) in self.USE_TYPE_TUPLE:
			self.__SendUseItemToItemPacket(srcItemSlotPos, dstItemSlotPos)

		elif WJ_ELDER_ATTRIBUTE_SYSTEM and player.GetItemIndex(srcItemSlotPos) == 71051: # @@@correction026
			self.AttachBonusToItem(srcItemSlotPos, dstItemSlotPos)
		elif WJ_ELDER_ATTRIBUTE_SYSTEM and player.GetItemIndex(srcItemSlotPos) == 71052: # @@@correction026
			self.__SendUseItemToItemPacket(srcItemSlotPos, dstItemSlotPos)
		elif ENABLE_CHANGE_LOOK_SYSTEM and player.GetItemIndex(srcItemSlotPos) == 72325: # @@@correction025
			self.__SendUseItemToItemPacket(srcItemSlotPos, dstItemSlotPos)
		elif ENABLE_SOULBIND_SYSTEM and (item.IsSealScroll(srcItemVID) or item.IsUnSealScroll(srcItemVID)): # @@@correction036
			self.__SendUseItemToItemPacket(srcItemSlotPos, dstItemSlotPos)
		elif player.GetItemIndex(srcItemSlotPos) >= 90060 and player.GetItemIndex(srcItemSlotPos) <= 90062: # @@@correction053
			self.__SendUseItemToItemPacket(srcItemSlotPos, dstItemSlotPos)
		elif player.GetItemIndex(srcItemSlotPos) >= 19983 and player.GetItemIndex(srcItemSlotPos) <= 19984:
			self.__SendUseItemToItemPacket(srcItemSlotPos, dstItemSlotPos)
		elif ENABLE_ADDON_ATTR and player.GetItemIndex(srcItemSlotPos) >= 71984 and player.GetItemIndex(srcItemSlotPos) <= 71985:
			self.__SendUseItemToItemPacket(srcItemSlotPos, dstItemSlotPos)
		else:
			#snd.PlaySound("sound/ui/drop.wav")

			## 이동시킨 곳이 장착 슬롯일 경우 아이템을 사용해서 장착 시킨다 - [levites]
			if player.IsEquipmentSlot(dstItemSlotPos):

				## 들고 있는 아이템이 장비일때만
				if item.IsEquipmentVID(srcItemVID):
					self.__UseItem(srcItemSlotPos)

			else:
				self.__SendMoveItemPacket(srcItemSlotPos, dstItemSlotPos, 0)
				#net.SendItemMovePacket(srcItemSlotPos, dstItemSlotPos, 0)

	def __SellItem(self, itemSlotPos):
		if not player.IsEquipmentSlot(itemSlotPos):
			self.sellingSlotNumber = itemSlotPos
			itemIndex = player.GetItemIndex(itemSlotPos)
			itemCount = player.GetItemCount(itemSlotPos)


			self.sellingSlotitemIndex = itemIndex
			self.sellingSlotitemCount = itemCount

			item.SelectItem(itemIndex)
			## 안티 플레그 검사 빠져서 추가
			## 20140220
			if item.IsAntiFlag(item.ANTIFLAG_SELL):
				popup = uiCommon.PopupDialog()
				popup.SetText(localeInfo.SHOP_CANNOT_SELL_ITEM)
				popup.SetAcceptEvent(self.__OnClosePopupDialog)
				popup.Open()
				self.popup = popup
				return

			itemPrice = item.GetISellItemPrice()

			if item.Is1GoldItem():
				itemPrice = itemCount / itemPrice / 5
			else:
				itemPrice = itemPrice * itemCount / 5

			item.GetItemName(itemIndex)
			itemName = item.GetItemName()

			self.questionDialog = uiCommon.QuestionDialog()
			self.questionDialog.SetText(localeInfo.DO_YOU_SELL_ITEM(itemName, itemCount, itemPrice))
			self.questionDialog.SetAcceptEvent(ui.__mem_func__(self.SellItem))
			self.questionDialog.SetCancelEvent(ui.__mem_func__(self.OnCloseQuestionDialog))
			self.questionDialog.Open()
			self.questionDialog.count = itemCount

			constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)

	def __OnClosePopupDialog(self):
		self.pop = None

	def RefineItem(self, scrollSlotPos, targetSlotPos):

		scrollIndex = player.GetItemIndex(scrollSlotPos)
		targetIndex = player.GetItemIndex(targetSlotPos)

		if player.REFINE_OK != player.CanRefine(scrollIndex, targetSlotPos):
			return

		if ENABLE_REFINE_RENEWAL:
			constInfo.AUTO_REFINE_TYPE = 1
			constInfo.AUTO_REFINE_DATA["ITEM"][0] = scrollSlotPos
			constInfo.AUTO_REFINE_DATA["ITEM"][1] = targetSlotPos

		###########################################################
		self.__SendUseItemToItemPacket(scrollSlotPos, targetSlotPos)
		#net.SendItemUseToItemPacket(scrollSlotPos, targetSlotPos)
		return
		###########################################################

		###########################################################
		#net.SendRequestRefineInfoPacket(targetSlotPos)
		#return
		###########################################################

		result = player.CanRefine(scrollIndex, targetSlotPos)

		if player.REFINE_ALREADY_MAX_SOCKET_COUNT == result:
			#snd.PlaySound("sound/ui/jaeryun_fail.wav")
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_NO_MORE_SOCKET)

		elif player.REFINE_NEED_MORE_GOOD_SCROLL == result:
			#snd.PlaySound("sound/ui/jaeryun_fail.wav")
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_NEED_BETTER_SCROLL)

		elif player.REFINE_CANT_MAKE_SOCKET_ITEM == result:
			#snd.PlaySound("sound/ui/jaeryun_fail.wav")
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_SOCKET_DISABLE_ITEM)

		elif player.REFINE_NOT_NEXT_GRADE_ITEM == result:
			#snd.PlaySound("sound/ui/jaeryun_fail.wav")
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_UPGRADE_DISABLE_ITEM)

		elif player.REFINE_CANT_REFINE_METIN_TO_EQUIPMENT == result:
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_EQUIP_ITEM)

		if player.REFINE_OK != result:
			return

		self.refineDialog.Open(scrollSlotPos, targetSlotPos)

	def DetachMetinFromItem(self, scrollSlotPos, targetSlotPos):
		scrollIndex = player.GetItemIndex(scrollSlotPos)
		targetIndex = player.GetItemIndex(targetSlotPos)
		if not player.CanDetach(scrollIndex, targetSlotPos):
			item.SelectItem(scrollIndex)
			if ENABLE_ACCE_SYSTEM: # @@@correction024
				if item.GetValue(0) == acce.CLEAN_ATTR_VALUE0:
					chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.ACCE_FAILURE_CLEAN)
			elif ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
				if item.GetValue(0) == changelook.CLEAN_ATTR_VALUE0:
					chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.CHANGE_LOOK_FAILURE_CLEAN)
			else:
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_METIN_INSEPARABLE_ITEM)
			return
		
		self.questionDialog = uiCommon.QuestionDialog()
		self.questionDialog.SetText(localeInfo.REFINE_DO_YOU_SEPARATE_METIN)
		if ENABLE_ACCE_SYSTEM: # @@@correction024
			item.SelectItem(targetIndex)
			if item.GetItemType() == item.ITEM_TYPE_COSTUME and item.GetItemSubType() == item.COSTUME_TYPE_ACCE:
				item.SelectItem(scrollIndex)
				if item.GetValue(0) == acce.CLEAN_ATTR_VALUE0:
					self.questionDialog.SetText(localeInfo.ACCE_DO_YOU_CLEAN)

		if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
			item.SelectItem(targetIndex)
			if item.GetItemType() == item.ITEM_TYPE_WEAPON or item.GetItemType() == item.ITEM_TYPE_ARMOR or item.GetItemType() == item.ITEM_TYPE_COSTUME:
				item.SelectItem(scrollIndex)
				if item.GetValue(0) == changelook.CLEAN_ATTR_VALUE0:
					self.questionDialog.SetText(localeInfo.CHANGE_LOOK_DO_YOU_CLEAN)

		self.questionDialog.SetAcceptEvent(ui.__mem_func__(self.OnDetachMetinFromItem))
		self.questionDialog.SetCancelEvent(ui.__mem_func__(self.OnCloseQuestionDialog))
		self.questionDialog.Open()
		self.questionDialog.sourcePos = scrollSlotPos
		self.questionDialog.targetPos = targetSlotPos

	def AttachMetinToItem(self, metinSlotPos, targetSlotPos):
		metinIndex = player.GetItemIndex(metinSlotPos)
		targetIndex = player.GetItemIndex(targetSlotPos)

		item.SelectItem(metinIndex)
		itemName = item.GetItemName()

		result = player.CanAttachMetin(metinIndex, targetSlotPos)

		if player.ATTACH_METIN_NOT_MATCHABLE_ITEM == result:
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_CAN_NOT_ATTACH(itemName))

		if player.ATTACH_METIN_NO_MATCHABLE_SOCKET == result:
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_NO_SOCKET(itemName))

		elif player.ATTACH_METIN_NOT_EXIST_GOLD_SOCKET == result:
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_NO_GOLD_SOCKET(itemName))

		elif player.ATTACH_METIN_CANT_ATTACH_TO_EQUIPMENT == result:
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.REFINE_FAILURE_EQUIP_ITEM)

		if player.ATTACH_METIN_OK != result:
			return

		self.attachMetinDialog.Open(metinSlotPos, targetSlotPos)

	def OverOutItem(self):
		self.wndItem.SetUsableItem(False)
		if None != selfs.wndToolTipItem:
			selfs.wndToolTipItem.HideToolTip()

	def OverInItem(self, overSlotPos):
		overSlotPos = self.__InventoryLocalSlotPosToGlobalSlotPos(overSlotPos)
		self.wndItem.SetUsableItem(False)

		if mouseModule.mouseController.isAttached():
			attachedItemType = mouseModule.mouseController.GetAttachedType()
			if player.SLOT_TYPE_INVENTORY == attachedItemType:

				attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
				attachedItemVNum = mouseModule.mouseController.GetAttachedItemIndex()

				if attachedItemVNum == player.ITEM_MONEY: # @@@correction006
					pass
				elif self.__CanUseSrcItemToDstItem(attachedItemVNum, attachedSlotPos, overSlotPos):
					self.wndItem.SetUsableItem(True)
					self.ShowToolTip(overSlotPos)
					return

		self.ShowToolTip(overSlotPos)


	def __IsUsableItemToItem(self, srcItemVNum, srcSlotPos):
		if ENABLE_GROWTH_PET_SYSTEM:
			if constInfo.IS_NEW_PET(srcItemVNum):
				return True
			
			if srcItemVNum == 55001:
				return True

		if item.IsRefineScroll(srcItemVNum):
			return True
		elif item.IsMetin(srcItemVNum):
			return True
		elif item.IsDetachScroll(srcItemVNum):
			return True
		elif item.IsKey(srcItemVNum):
			return True
		elif (player.GetItemFlags(srcSlotPos) & ITEM_FLAG_APPLICABLE) == ITEM_FLAG_APPLICABLE:
			return True
		elif WJ_ELDER_ATTRIBUTE_SYSTEM and player.GetItemIndex(srcSlotPos) == 71051 or player.GetItemIndex(srcSlotPos) == 71052: # @@@correction026
			return True
		elif ENABLE_CHANGE_LOOK_SYSTEM and player.GetItemIndex(srcSlotPos) == 72325:
			return True
		elif ENABLE_SOULBIND_SYSTEM and (item.IsSealScroll(player.GetItemIndex(srcSlotPos)) or item.IsUnSealScroll(player.GetItemIndex(srcSlotPos))):
			return True
		elif player.GetItemIndex(srcSlotPos) >= 90060 and player.GetItemIndex(srcSlotPos) <= 90062: # @@@correction053
			return True
		elif player.GetItemIndex(srcSlotPos) >= 19983 and player.GetItemIndex(srcSlotPos) <= 19984:
			return True
		elif ENABLE_ADDON_ATTR and player.GetItemIndex(srcSlotPos) >= 71984 and player.GetItemIndex(srcSlotPos) <= 71985:
			return True
		else:
			if item.GetUseType(srcItemVNum) in self.USE_TYPE_TUPLE:
				return True

		return False

	def __CanUseSrcItemToDstItem(self, srcItemVNum, srcSlotPos, dstSlotPos):
		if srcSlotPos == dstSlotPos:
			return False

		if ENABLE_GROWTH_PET_SYSTEM:
			if constInfo.IS_NEW_PET(srcItemVNum) and player.GetItemIndex(dstSlotPos) == 55002:			
				return True		
			
			if srcItemVNum == 55001 and constInfo.IS_NEW_PET(player.GetItemIndex(dstSlotPos)):
				return True

		if item.IsRefineScroll(srcItemVNum):
			if player.REFINE_OK == player.CanRefine(srcItemVNum, dstSlotPos):
				return True
		elif item.IsMetin(srcItemVNum):
			if player.ATTACH_METIN_OK == player.CanAttachMetin(srcItemVNum, dstSlotPos):
				return True
		elif item.IsDetachScroll(srcItemVNum):
			if player.DETACH_METIN_OK == player.CanDetach(srcItemVNum, dstSlotPos):
				return True
		elif item.IsKey(srcItemVNum):
			if player.CanUnlock(srcItemVNum, dstSlotPos):
				return True
		elif (player.GetItemFlags(srcSlotPos) & ITEM_FLAG_APPLICABLE) == ITEM_FLAG_APPLICABLE:
			return True
		elif ENABLE_SOULBIND_SYSTEM and item.IsSealScroll(srcItemVNum): # @@@correction036
			if (item.IsSealScroll(srcItemVNum) and player.CanSealItem(dstSlotPos)) or (item.IsUnSealScroll(srcItemVNum) and player.CanUnSealItem(dstSlotPos)):
				return True
		elif WJ_ELDER_ATTRIBUTE_SYSTEM and player.GetItemIndex(srcSlotPos) == 71051 or player.GetItemIndex(srcSlotPos) == 71052: # @@@correction026
			if self.__CanPutNewAttribute(dstSlotPos):
				return True
		elif ENABLE_ADDON_ATTR and player.GetItemIndex(srcSlotPos) >= 71984 and player.GetItemIndex(srcSlotPos) <= 71985:
			if self.__CanChangeItemAttrList(dstSlotPos):
				return True
		else:
			useType=item.GetUseType(srcItemVNum)

			if ENABLE_MOVE_COSTUME_ATTR:
				if useType == "USE_ADD_COSTUME_ATTRIBUTE" or useType == "USE_RESET_COSTUME_ATTR":				
					dstItemVNum = player.GetItemIndex(dstSlotPos)
					item.SelectItem(dstItemVNum)
					if item.GetItemType() == item.ITEM_TYPE_COSTUME:
						return True

			if "USE_CLEAN_SOCKET" == useType:
				if self.__CanCleanBrokenMetinStone(dstSlotPos):
					return True
			elif "USE_CHANGE_ATTRIBUTE" == useType:
				if self.__CanChangeItemAttrList(dstSlotPos):
					return True
			elif "USE_ADD_ATTRIBUTE" == useType:
				if self.__CanAddItemAttr(dstSlotPos):
					return True
			elif "USE_ADD_ATTRIBUTE2" == useType:
				if self.__CanAddItemAttr(dstSlotPos):
					return True
			elif "USE_ADD_ACCESSORY_SOCKET" == useType:
				if self.__CanAddAccessorySocket(dstSlotPos):
					return True
			elif "USE_PUT_INTO_ACCESSORY_SOCKET" == useType:
				if self.__CanPutAccessorySocket(dstSlotPos, srcItemVNum):
					return True;
			elif "USE_PUT_INTO_BELT_SOCKET" == useType:
				dstItemVNum = player.GetItemIndex(dstSlotPos)
				print "USE_PUT_INTO_BELT_SOCKET", srcItemVNum, dstItemVNum

				item.SelectItem(dstItemVNum)

				if item.ITEM_TYPE_BELT == item.GetItemType():
					return True

		return False

	def __CanCleanBrokenMetinStone(self, dstSlotPos):
		dstItemVNum = player.GetItemIndex(dstSlotPos)
		if dstItemVNum == 0:
			return False

		item.SelectItem(dstItemVNum)

		if item.ITEM_TYPE_WEAPON != item.GetItemType():
			return False

		for i in xrange(player.METIN_SOCKET_MAX_NUM):
			if player.GetItemMetinSocket(dstSlotPos, i) == constInfo.ERROR_METIN_STONE:
				return True

		return False

	def __CanChangeItemAttrList(self, dstSlotPos):
		dstItemVNum = player.GetItemIndex(dstSlotPos)
		if dstItemVNum == 0:
			return False

		item.SelectItem(dstItemVNum)

		if not item.GetItemType() in (item.ITEM_TYPE_WEAPON, item.ITEM_TYPE_ARMOR):	
			return False

		for i in xrange(player.METIN_SOCKET_MAX_NUM):
			if player.GetItemAttribute(dstSlotPos, i) != 0:
				return True

		return False

	def __CanPutAccessorySocket(self, dstSlotPos, mtrlVnum):
		dstItemVNum = player.GetItemIndex(dstSlotPos)
		if dstItemVNum == 0:
			return False

		item.SelectItem(dstItemVNum)

		if item.GetItemType() != item.ITEM_TYPE_ARMOR:
			return False

		if not item.GetItemSubType() in (item.ARMOR_WRIST, item.ARMOR_NECK, item.ARMOR_EAR):
			return False

		curCount = player.GetItemMetinSocket(dstSlotPos, 0)
		maxCount = player.GetItemMetinSocket(dstSlotPos, 1)

		tmpmtrlVnum = constInfo.GET_ACCESSORY_MATERIAL_VNUM(dstItemVNum, item.GetItemSubType())

		if mtrlVnum != tmpmtrlVnum and mtrlVnum != constInfo.JewelAccessoryInfosdict[tmpmtrlVnum]:
			return False

		if curCount>=maxCount:
			return False

		return True

	def __CanAddAccessorySocket(self, dstSlotPos):
		dstItemVNum = player.GetItemIndex(dstSlotPos)
		if dstItemVNum == 0:
			return False

		item.SelectItem(dstItemVNum)

		if item.GetItemType() != item.ITEM_TYPE_ARMOR:
			return False

		if not item.GetItemSubType() in (item.ARMOR_WRIST, item.ARMOR_NECK, item.ARMOR_EAR):
			return False

		curCount = player.GetItemMetinSocket(dstSlotPos, 0)
		maxCount = player.GetItemMetinSocket(dstSlotPos, 1)

		ACCESSORY_SOCKET_MAX_SIZE = 3
		if maxCount >= ACCESSORY_SOCKET_MAX_SIZE:
			return False

		return True

	def __CanAddItemAttr(self, dstSlotPos):
		dstItemVNum = player.GetItemIndex(dstSlotPos)
		if dstItemVNum == 0:
			return False

		item.SelectItem(dstItemVNum)

		if not item.GetItemType() in (item.ITEM_TYPE_WEAPON, item.ITEM_TYPE_ARMOR):
			return False

		attrCount = 0
		for i in xrange(player.METIN_SOCKET_MAX_NUM):
			if player.GetItemAttribute(dstSlotPos, i) != 0:
				attrCount += 1

		if attrCount<4:
			return True

		return False

	def ShowToolTip(self, slotIndex):
		if None != selfs.wndToolTipItem:
			selfs.wndToolTipItem.SetInventoryItem(slotIndex)

	def OnTop(self):
		if None != selfs.wndToolTipItem:
			selfs.wndToolTipItem.SetTop()

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def UseItemSlot(self, slotIndex):
		curCursorNum = app.GetCursor()
		if app.SELL == curCursorNum:
			return

		if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS():
			return

		slotIndex = self.__InventoryLocalSlotPosToGlobalSlotPos(slotIndex)

		if ENABLE_DRAGON_SOUL_SYSTEM:
			if selfs.wndDragonSoulRefine.IsShow():
				selfs.wndDragonSoulRefine.AutoSetItem((player.INVENTORY, slotIndex), 1)
				return

		if ENABLE_ACCE_SYSTEM: # @@@correction024
			if self.isShowAcceWindow():
				acce.Add(player.INVENTORY, slotIndex, 255)
				return

		if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
			if self.isShowChangeLookWindow():
				if ENABLE_CHANGE_LOOK_ITEM_SYSTEM: # @@@correction025
					ItemVnum = player.GetItemIndex(slotIndex)
					if ItemVnum == 72326 or ItemVnum == 72341:
						changelook.Add(player.INVENTORY, slotIndex, 2)
						selfs.wndChangeLook.ChangeMoney()
					else:
						changelook.Add(player.INVENTORY, slotIndex, 255)
				else:
					changelook.Add(player.INVENTORY, slotIndex, 255)

				return

		if ENABLE_FAST_SELL and app.IsPressed(app.DIK_LCONTROL) and app.IsPressed(app.DIK_X):
			if uiPrivateShopBuilder.IsBuildingPrivateShop():
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.DROP_ITEM_FAILURE_PRIVATE_SHOP)
				return

			net.SendItemDestroyPacket(slotIndex, 0)
			snd.PlaySound("sound/ui/money.wav")
			return

		if ENABLE_FAST_USE_GIFTBOX and app.IsPressed(app.DIK_LALT):
			if uiPrivateShopBuilder.IsBuildingPrivateShop():
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.DROP_ITEM_FAILURE_PRIVATE_SHOP)
				return

			if not self.dlgUseItem:
				self.dlgUseItem = uiPickMoney.PickMoneyDialog()

			self.dlgUseItem.SetTitleName(localeInfo.PICK_ITEM_TITLE)
			self.dlgUseItem.SetAcceptEvent(ui.__mem_func__(self.OnUseItem))
			self.dlgUseItem.Open(player.GetItemCount(slotIndex))
			self.dlgUseItem.itemGlobalSlotIndex = slotIndex
			return

		if ENABLE_SHOW_CHEST_DROP and (player.GetItemTypeBySlot(slotIndex) == item.ITEM_TYPE_GIFTBOX or (ENABLE_SPECIAL_GACHA and player.GetItemTypeBySlot(slotIndex) == item.ITEM_TYPE_GACHA)) and app.IsPressed(app.DIK_LCONTROL):
			if selfs.wndChestDrop:
				if selfs.wndChestDrop.IsShow():
					selfs.wndChestDrop.Close()
				else:
					selfs.wndChestDrop.Open(slotIndex)
					net.SendChestDropInfo(slotIndex)

				return

		if ENABLE_TOOLTIP_SHOW_MODEL_SYSTEM and app.IsPressed(app.DIK_LCONTROL):
			selfs.wndRender.ModelWindowProcess()
			return

		self.__UseItem(slotIndex)
		mouseModule.mouseController.DeattachObject()
		self.OverOutItem()

	def __UseItem(self, slotIndex):
		ItemVNum = player.GetItemIndex(slotIndex)
		item.SelectItem(ItemVNum)

		if item.IsFlag(item.ITEM_FLAG_CONFIRM_WHEN_USE):
			self.questionDialog = uiCommon.QuestionDialog()
			self.questionDialog.SetText(localeInfo.INVENTORY_REALLY_USE_ITEM)
			self.questionDialog.SetAcceptEvent(ui.__mem_func__(self.__UseItemQuestionDialog_OnAccept))
			self.questionDialog.SetCancelEvent(ui.__mem_func__(self.__UseItemQuestionDialog_OnCancel))
			self.questionDialog.Open()
			self.questionDialog.slotIndex = slotIndex
			constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)
		else:
			self.__SendUseItemPacket(slotIndex)
			#net.SendItemUsePacket(slotIndex)

	def __UseItemQuestionDialog_OnCancel(self):
		self.OnCloseQuestionDialog()

	def __UseItemQuestionDialog_OnAccept(self):
		self.__SendUseItemPacket(self.questionDialog.slotIndex)
		self.OnCloseQuestionDialog()

	def __SendUseItemToItemPacket(self, srcSlotPos, dstSlotPos):
		# 개인상점 열고 있는 동안 아이템 사용 방지
		if uiPrivateShopBuilder.IsBuildingPrivateShop():
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.USE_ITEM_FAILURE_PRIVATE_SHOP)
			return

		net.SendItemUseToItemPacket(srcSlotPos, dstSlotPos)

	def __SendUseItemPacket(self, slotPos):
		# 개인상점 열고 있는 동안 아이템 사용 방지
		if uiPrivateShopBuilder.IsBuildingPrivateShop():
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.USE_ITEM_FAILURE_PRIVATE_SHOP)
			return

		net.SendItemUsePacket(slotPos)

	def __SendMoveItemPacket(self, srcSlotPos, dstSlotPos, srcItemCount):
		# 개인상점 열고 있는 동안 아이템 사용 방지
		if uiPrivateShopBuilder.IsBuildingPrivateShop():
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.MOVE_ITEM_FAILURE_PRIVATE_SHOP)
			return

		net.SendItemMovePacket(srcSlotPos, dstSlotPos, srcItemCount)

	def OnMoveWindow(self, x, y):
		if self.wndBelt:
			self.wndBelt.AdjustPositionAndSize()
		if ENABLE_AFAR_BIOLOG:
			if self.wndBio:
				self.wndBio.AdjustPositionAndSize()

	if ENABLE_ACCE_SYSTEM: # @@@correction024
		def isShowAcceWindow(self):
			if wndAcceCombine:
				if wndAcceCombine.IsShow():
					return 1

			if wndAcceAbsorption:
				if wndAcceAbsorption.IsShow():
					return 1

			return 0

	if ENABLE_CHANGE_LOOK_SYSTEM: # @@@correction025
		def isShowChangeLookWindow(self):
			if selfs.wndChangeLook:
				if selfs.wndChangeLook.IsShow():
					return 1

			return 0

	def OnUpdate(self):
		if WJ_ELDER_ATTRIBUTE_SYSTEM: # @@@correction026
			if self.attachBonusDialog:
				if self.attachBonusDialog.IsShow():
					self.attachBonusDialog.Update()

		if WJ_ENABLE_TRADABLE_ICON:
			if ENABLE_ACCE_SYSTEM:
				for j in xrange(acce.WINDOW_MAX_MATERIALS):
					(isHere, iCell) = acce.GetAttachedItem(j)
					if isHere:
						self.SetCantMouseEventSlot(iCell)
					else:
						self.SetCanMouseEventSlot(iCell)

			if ENABLE_CHANGE_LOOK_SYSTEM:
				for j in xrange(changelook.WINDOW_MAX_MATERIALS):
					(isHere, iCell) = changelook.GetAttachedItem(j)
					if isHere:
						self.SetCantMouseEventSlot(iCell)
					else:
						self.SetCanMouseEventSlot(iCell)

	if WJ_ELDER_ATTRIBUTE_SYSTEM: # @@@correction026
		def AttachBonusToItem(self, sourceSlotPos, targetSlotPos):
			targetIndex = player.GetItemIndex(targetSlotPos)
			item.SelectItem(targetIndex)
			if item.GetItemType() not in (item.ITEM_TYPE_WEAPON, item.ITEM_TYPE_ARMOR):
				return False
			self.attachBonusDialog.Open(sourceSlotPos, targetSlotPos)

		def __CanPutNewAttribute(self, dstSlotPos):
			dstItemVNum = player.GetItemIndex(dstSlotPos)
			if dstItemVNum == 0:
				return False
			item.SelectItem(dstItemVNum)
			if item.GetItemType() not in (item.ITEM_TYPE_WEAPON, item.ITEM_TYPE_ARMOR):
				return False
			return True

	if ENABLE_GROWTH_PET_SYSTEM:
		def UseTransportBox(self):
			self.__SendUseItemToItemPacket(self.questionDialog.src, self.questionDialog.dst)
			self.OnCloseQuestionDialog()
		
		def UseProtein(self):
			self.__SendUseItemToItemPacket(self.questionDialog.src, self.questionDialog.dst)
			self.OnCloseQuestionDialog()

	if ENABLE_EXTEND_INVEN_SYSTEM:
		def SetCategoryPage(self, page):
			self.categoryTab[self.categoryPageIndex].SetUp()
			self.categoryPageIndex = page
			self.categoryTab[self.categoryPageIndex].Down()
			
			# self.titleName.SetText(self.WINDOW_NAMES[self.categoryPageIndex])
			self.SetInventoryPage(0)
			self.inventoryTab[0].Down()
			self.RefreshBagSlotWindow()

	if ENABLE_SORT_ITEMS:
		def ClickYenileButton(self):
			net.SendChatPacket("/click_sort_inventory_items %d" % self.categoryPageIndex)

	def GetInventoryPageIndex(self):
		return self.inventoryPageIndex

	def SetCanMouseEventSlot(self, inven_slot_index):
		inven_slot_index = inven_slot_index % 180
		page = self.GetInventoryPageIndex()
		if (page * player.INVENTORY_PAGE_SIZE) <= inven_slot_index < ((page + 1) * player.INVENTORY_PAGE_SIZE):
			if inven_slot_index >= player.INVENTORY_PAGE_SIZE:
				inven_slot_index -= (page * player.INVENTORY_PAGE_SIZE)
			if self.wndItem:
				self.wndItem.SetCanMouseEventSlot(inven_slot_index)
			
	def SetCantMouseEventSlot(self, inven_slot_index):
		inven_slot_index = inven_slot_index % 180
		page = self.GetInventoryPageIndex()
		if (page * player.INVENTORY_PAGE_SIZE) <= inven_slot_index < ((page + 1) * player.INVENTORY_PAGE_SIZE):
			if inven_slot_index >= player.INVENTORY_PAGE_SIZE:
				inven_slot_index -= (page * player.INVENTORY_PAGE_SIZE)
			if self.wndItem:
				self.wndItem.SetCantMouseEventSlot(inven_slot_index)

	if ENABLE_AFAR_BIOLOG:
		def SetBiyolog(self, bioitem, verilen, toplam, kalansure):
			if self.wndBio:
				self.wndBio.SetBiyolog(bioitem, verilen, toplam, kalansure)

	if ENABLE_NEW_BUTTONS:
		def SetNewButtons(self, page):
			if page == 0:
				selfs.wndGame.ToggleSwitchbotWindow()
			if page == 1:
				selfs.wndGame.OpenSupportGui()
			if page == 2:
				selfs.wndGame.OpenNpc()

	if ENABLE_FAST_USE_GIFTBOX:
		def OnUseItem(self, count):
			# selectedItemVNum = player.GetItemIndex(itemSlotIndex)
			if count > 200:
				count = 200
			net.SendItemUsePacket(player.INVENTORY, self.dlgUseItem.itemGlobalSlotIndex, count)

