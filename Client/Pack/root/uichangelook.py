import ui
import player
import uiCommon
import changelook
import localeInfo
import mouseModule
import uiToolTip
import app
import __builtin__ as selfs

class Window(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.isLoaded = 0
		self.PositionOut = 0
		self.PositionStartX = 0
		self.PositionStartY = 0
		self.dialog = None
		self.ChangeLookToolTIpButton = None
		self.ChangeLookToolTipList = [
			localeInfo.CHANGE_TOOLTIP_LINE1, 
			localeInfo.CHANGE_TOOLTIP_LINE2,
			localeInfo.CHANGE_TOOLTIP_LINE3,
			localeInfo.CHANGE_TOOLTIP_LINE4
		]
		if ENABLE_CHANGE_LOOK_ITEM_SYSTEM: # @@@correction025
			self.ChangeLookToolTipList.append(localeInfo.CHANGE_TOOLTIP_LINE5)
			self.ChangeLookToolTipList.append(localeInfo.CHANGE_TOOLTIP_LINE6)

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def Destroy(self):
		self.ClearDictionary()
		self.titleBar = None
		self.titleName = None
		self.accept = None
		self.cancel = None
		self.slot = None
		self.cost = None
		self.PositionOut = 0
		self.PositionStartX = 0
		self.PositionStartY = 0
		self.dialog = None
		self.ChangeLookToolTIpButton = None

	def LoadWindow(self):
		if self.isLoaded:
			return
		
		self.isLoaded = 1
		
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/changelookwindow.py")
			
		except:
			import exception
			exception.Abort("ChangeLookWindow.LoadDialog.LoadScript")
		
		try:
			self.titleBar = self.GetChild("TitleBar")
			self.titleName = self.GetChild("TitleName")
			self.accept = self.GetChild("AcceptButton")
			self.cancel = self.GetChild("CancelButton")
			self.slot = self.GetChild("ChangeLookSlot")
			self.cost = self.GetChild("Cost")
		except:
			import exception
			exception.Abort("ChangeLookWindow.LoadDialog.BindObject")
		
		self.titleBar.SetCloseEvent(ui.__mem_func__(self.OnClose))
		self.titleName.SetText(localeInfo.CHANGE_LOOK_TITLE)
		self.cancel.SetEvent(ui.__mem_func__(self.OnClose))
		self.accept.SetEvent(ui.__mem_func__(self.OnPressAccept))
		self.cost.SetText(localeInfo.CHANGE_LOOK_COST % (localeInfo.NumberToMoneyString(changelook.GetCost())))
		self.slot.SetSelectEmptySlotEvent(ui.__mem_func__(self.OnSelectEmptySlot))
		self.slot.SetUnselectItemSlotEvent(ui.__mem_func__(self.OnSelectItemSlot))
		self.slot.SetUseSlotEvent(ui.__mem_func__(self.OnSelectItemSlot))
		self.slot.SetOverInItemEvent(ui.__mem_func__(self.OnOverInItem))
		self.slot.SetOverOutItemEvent(ui.__mem_func__(self.OnOverOutItem))

		self.ChangeLookToolTIpButton = self.GetChild("ChangeLookToolTIpButton")
		self.ChangeLookToolTip = self.__CreateGameTypeToolTip(localeInfo.CHANGE_TOOLTIP_TITLE, self.ChangeLookToolTipList)
		self.ChangeLookToolTip.SetTop()
		self.ChangeLookToolTIpButton.SetToolTipWindow(self.ChangeLookToolTip)

	def __CreateGameTypeToolTip(self, title, descList):
		toolTip = uiToolTip.ToolTip()
		toolTip.SetTitle(title)
		if ENABLE_CHANGE_LOOK_ITEM_SYSTEM: # @@@correction025
			toolTip.AppendSpace(7)
		else:
			toolTip.AppendSpace(5)

		for desc in descList:
			toolTip.AutoAppendTextLine(desc)

		toolTip.AlignHorizonalCenter()
		toolTip.SetTop()
		return toolTip

	def IsOpened(self):
		if self.IsShow() and self.isLoaded:
			return True
		
		return False

	def Open(self):
		self.PositionOut = 0
		(self.PositionStartX, self.PositionStartY, z) = player.GetMainCharacterPosition()
		self.cost.SetText(localeInfo.CHANGE_LOOK_COST % (localeInfo.NumberToMoneyString(changelook.GetCost())))
		for i in xrange(changelook.WINDOW_MAX_MATERIALS):
			self.slot.ClearSlot(i)
		
		self.SetCenterPosition()
		self.Show()

	def Close(self):
		if selfs.wndToolTipItem:
			selfs.wndToolTipItem.HideToolTip()
		
		self.OnCancelAccept()
		self.Hide()

	def OnClose(self):
		changelook.SendCloseRequest()

	def OnPressEscapeKey(self):
		self.OnClose()
		return True

	def OnPressAccept(self):
		(isHere, iCell) = changelook.GetAttachedItem(1)
		if not isHere:
			return
		
		dialog = uiCommon.QuestionDialog()
		dialog.SetText(localeInfo.CHANGE_LOOK_CHANGE_ITEM)
		dialog.SetAcceptEvent(ui.__mem_func__(self.OnAccept))
		dialog.SetCancelEvent(ui.__mem_func__(self.OnCancelAccept))
		dialog.Open()
		self.dialog = dialog

	def OnAccept(self):
		changelook.SendRefineRequest()
		self.OnCancelAccept()

	def OnCancelAccept(self):
		if self.dialog:
			self.dialog.Close()
		
		self.dialog = None
		return True

	def OnUpdate(self):
		LIMIT_RANGE = changelook.LIMIT_RANGE
		(x, y, z) = player.GetMainCharacterPosition()
		if abs(x - self.PositionStartX) >= LIMIT_RANGE or abs(y - self.PositionStartY) >= LIMIT_RANGE:
			if not self.PositionOut:
				self.PositionOut += 1
				self.OnClose()

	def OnSelectEmptySlot(self, selectedSlotPos):
		isAttached = mouseModule.mouseController.isAttached()
		if not isAttached:
			return
		
		attachedSlotType = mouseModule.mouseController.GetAttachedType()
		attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
		attachedInvenType = player.SlotTypeToInvenType(attachedSlotType)
		mouseModule.mouseController.DeattachObject()
		if attachedSlotType == player.SLOT_TYPE_INVENTORY and attachedInvenType == player.INVENTORY:
			changelook.Add(attachedInvenType, attachedSlotPos, selectedSlotPos)
			if ENABLE_CHANGE_LOOK_ITEM_SYSTEM: # @@@correction025
				if selectedSlotPos == 2:
					self.cost.SetText(localeInfo.CHANGE_LOOK_COST % (localeInfo.NumberToMoneyString("0")))

	def OnSelectItemSlot(self, selectedSlotPos):
		mouseModule.mouseController.DeattachObject()
		changelook.Remove(selectedSlotPos)
		if ENABLE_CHANGE_LOOK_ITEM_SYSTEM: # @@@correction025
			self.cost.SetText(localeInfo.CHANGE_LOOK_COST % (localeInfo.NumberToMoneyString(changelook.GetCost())))

	def OnOverInItem(self, selectedSlotPos):
		self.slot.SetUsableItem(False)
		if selfs.wndToolTipItem:
			(isHere, iCell) = changelook.GetAttachedItem(selectedSlotPos)
			if isHere:
				selfs.wndToolTipItem.SetInventoryItem(iCell)

	def OnOverOutItem(self):
		self.slot.SetUsableItem(False)
		if selfs.wndToolTipItem:
			selfs.wndToolTipItem.HideToolTip()

	def Refresh(self):
		for i in xrange(changelook.WINDOW_MAX_MATERIALS):
			self.slot.ClearSlot(i)
			(isHere, iCell) = changelook.GetAttachedItem(i)
			if isHere:
				self.slot.SetItemSlot(i, player.GetItemIndex(iCell), 1)

	if ENABLE_CHANGE_LOOK_ITEM_SYSTEM: # @@@correction025
		def ChangeMoney(self):
			self.cost.SetText(localeInfo.CHANGE_LOOK_COST % (localeInfo.NumberToMoneyString("0")))

