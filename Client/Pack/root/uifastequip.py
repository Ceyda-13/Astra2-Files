import os
import ui
import app
import ime
import grp
import net
import snd
import item
import chat
import player
import locale
import grpText
import uiRefine
import uiToolTip
import mouseModule
import uiAttachMetin
import uiScriptLocale
import __builtin__ as selfs

class CostumeEquipmentDialog(ui.ScriptWindow):
	def __init__(self, wndEquip):
		if not wndEquip:
			import exception
			exception.Abort("wndEquipment parameter must be set to CostumeEquipmentDialog")
			return

		ui.ScriptWindow.__init__(self)

		self.isLoaded = 0
		self.wndEquip = wndEquip;

		self.wndCostumeEquipmentLayer = None
		self.wndCostumeEquipmentSlot = None
		self.expandBtn = None
		self.minBtn = None
		self.tokens = None

		self.__LoadWindow()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def Show2(self):
		self.__LoadWindow()
		ui.ScriptWindow.Show(self)

		self.CloseCostumeEquipment()

	def Close2(self):
		self.Hide()

	def IsOpenedCostumeEquipment(self):
		return self.wndCostumeEquipmentLayer.IsShow()

	def OpenCostumeEquipment(self):
		self.wndCostumeEquipmentLayer.Show()
		self.expandBtn.Hide()

		self.AdjustPositionAndSize()

	def CloseCostumeEquipment(self):
		self.wndCostumeEquipmentLayer.Hide()
		self.expandBtn.Show()

		self.AdjustPositionAndSize()

	def GetBasePosition(self):
		x, y = self.wndEquip.GetGlobalPosition()
		return x - 139, y + 30

	def AdjustPositionAndSize(self):
		bx, by = self.GetBasePosition()

		if self.IsOpenedCostumeEquipment():
			self.SetPosition(bx, by)
			self.SetSize(self.ORIGINAL_WIDTH, self.GetHeight())

		else:
			self.SetPosition(bx + 129, by);
			self.SetSize(10, self.GetHeight())

	def __LoadWindow(self):
		if self.isLoaded == 1:
			return

		self.isLoaded = 1

		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/CostumeEquipmentDialog.py")
		except:
			import exception
			exception.Abort("CostumeEquipmentDialog.LoadWindow.LoadObject")

		try:
			self.ORIGINAL_WIDTH = self.GetWidth()
			wndCostumeEquipmentSlot = self.GetChild("CostumeEquipmentSlot")
			self.wndCostumeEquipmentLayer = self.GetChild("CostumeEquipmentLayer")
			self.expandBtn = self.GetChild("ExpandButton")
			self.minBtn = self.GetChild("MinimizeButton")

			self.expandBtn.SetEvent(ui.__mem_func__(self.OpenCostumeEquipment))
			self.minBtn.SetEvent(ui.__mem_func__(self.CloseCostumeEquipment))

		except:
			import exception
			exception.Abort("CostumeEquipmentDialog.LoadWindow.BindObject")

		wndCostumeEquipmentSlot.SetOverInItemEvent(ui.__mem_func__(self.wndEquip.OverInItem))
		wndCostumeEquipmentSlot.SetOverOutItemEvent(ui.__mem_func__(self.wndEquip.OnOverOutItem))
		wndCostumeEquipmentSlot.SetSelectEmptySlotEvent(ui.__mem_func__(self.wndEquip.SelectItemSlot))
		wndCostumeEquipmentSlot.SetSelectItemSlotEvent(ui.__mem_func__(self.wndEquip.SelectItemSlot))
		self.wndCostumeEquipmentSlot = wndCostumeEquipmentSlot
		
class changeequip(ui.ScriptWindow):
	def __init__(self):
		import exception
		ui.ScriptWindow.__init__(self)
		self.wndEquip = None
		self.tokens = None
		self.tooltipItem = uiToolTip.ItemToolTip()
		self.tooltipItem.Hide()
		self.wndCostumeEquipment = CostumeEquipmentDialog(self)
		if selfs.FAST_PAGE == 1:
			self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage1.pyc"
		elif selfs.FAST_PAGE == 2:
			self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage2.pyc"
		elif selfs.FAST_PAGE == 3:
			self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage3.pyc"
		elif selfs.FAST_PAGE == 4:
			self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage4.pyc"
		elif selfs.FAST_PAGE == 5:
			self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage5.pyc"
		elif selfs.FAST_PAGE == 6:
			self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage6.pyc"

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def Show(self):
		self.__LoadWindow()
		ui.ScriptWindow.Show(self)
		
		if self.wndCostumeEquipment:
			self.wndCostumeEquipment.Show2()

	def Close(self):
		selfs.FAST_EQUIP = 0
		#snd.PlaySound("sound/ui/click.wav")
		self.Hide()
		self.wndCostumeEquipment.Close2()
		
			
	def __LoadWindow(self):
		try:			
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/fastequip_window.py")
		except:
			import exception
			exception.Abort("CostumeWindow.LoadWindow.LoadObject")
		
		self.wndEquip = self.GetChild("equipslot")
		self.bottone_change = self.GetChild("change_button")
		self.clear_button = self.GetChild("clear_button")
		self.TitleBar = self.GetChild("TitleBar")
		self.amountInput = self.GetChild("AmountInput")
		self.pag1_button = self.GetChild("page1_button")
		self.pag2_button = self.GetChild("page2_button")
		self.pag3_button = self.GetChild("page3_button")
		self.pag4_button = self.GetChild("page4_button")
		self.pag5_button = self.GetChild("page5_button")
		self.pag6_button = self.GetChild("page6_button")
		
		self.TitleBar.SetCloseEvent(ui.__mem_func__(self.Close))
		self.wndEquip.SetSelectEmptySlotEvent(ui.__mem_func__(self.SelectItemSlot))
		self.wndEquip.SetSelectItemSlotEvent(ui.__mem_func__(self.SelectItemSlot))
		self.wndEquip.SetOverInItemEvent(ui.__mem_func__(self.OverInItem))
		self.wndEquip.SetOverOutItemEvent(ui.__mem_func__(self.OnOverOutItem))
		self.bottone_change.SetEvent(ui.__mem_func__(self.__change_button))
		self.clear_button.SetEvent(ui.__mem_func__(self.__clear_button))
		self.pag1_button.SAFE_SetEvent(self.__pag1_button)
		self.pag2_button.SetEvent(ui.__mem_func__(self.__pag2_button))
		self.pag3_button.SetEvent(ui.__mem_func__(self.__pag3_button))
		self.pag4_button.SetEvent(ui.__mem_func__(self.__pag4_button))
		self.pag5_button.SetEvent(ui.__mem_func__(self.__pag5_button))
		self.pag6_button.SetEvent(ui.__mem_func__(self.__pag6_button))
		
		if os.path.exists(self.saveName):
			self.tokens = open(self.saveName, "r").read().split()
		else:
			open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")
		
		

	def __clear_button(self):
		if selfs.FAST_PAGE == 1:
			self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage1.pyc"
			if os.path.exists(self.saveName):
				os.remove(self.saveName)
				open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")
		elif selfs.FAST_PAGE == 2:
			self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage2.pyc"
			if os.path.exists(self.saveName):
				os.remove(self.saveName)
				open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")
		elif selfs.FAST_PAGE == 3:
			self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage3.pyc"
			if os.path.exists(self.saveName):
				os.remove(self.saveName)
				open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")
		elif selfs.FAST_PAGE == 4:
			self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage4.pyc"
			if os.path.exists(self.saveName):
				os.remove(self.saveName)
				open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")
		elif selfs.FAST_PAGE == 5:
			self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage5.pyc"
			if os.path.exists(self.saveName):
				os.remove(self.saveName)
				open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")
		elif selfs.FAST_PAGE == 6:
			self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage6.pyc"
			if os.path.exists(self.saveName):
				os.remove(self.saveName)
				open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")

	def __pag1_button(self):
		if selfs.FAST_PAGE == 5:
			self.__pag4_button()
			selfs.FAST_PAGE = 4
		elif selfs.FAST_PAGE == 4:
			self.__pag33_button()
			selfs.FAST_PAGE = 3
		elif selfs.FAST_PAGE == 3:
			self.__pag2_button()
			selfs.FAST_PAGE = 2
		elif selfs.FAST_PAGE == 2:
			self.__pag11_button()
			selfs.FAST_PAGE = 1
		elif selfs.FAST_PAGE == 1:
			return
		else:
			pass
			
	def __pag11_button(self):
	
		self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage1.pyc"
		if os.path.exists(self.saveName):
			open(self.saveName, "r").read().split()
		else:
			open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")

	def __pag2_button(self):

		
		self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage2.pyc"
		if os.path.exists(self.saveName):
			open(self.saveName, "r").read().split()
		else:
			open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")

	def __pag3_button(self):
		if selfs.FAST_PAGE == 1:
			self.__pag2_button()
			selfs.FAST_PAGE = 2
		elif selfs.FAST_PAGE == 2:
			self.__pag33_button()
			selfs.FAST_PAGE = 3
		elif selfs.FAST_PAGE == 3:
			self.__pag4_button()
			selfs.FAST_PAGE = 4
		elif selfs.FAST_PAGE == 4:
			self.__pag5_button()
			selfs.FAST_PAGE = 5
		else:
			pass
			
	def __pag33_button(self):

		
		self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage3.pyc"
		if os.path.exists(self.saveName):
			open(self.saveName, "r").read().split()
		else:
			open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")

	def __pag4_button(self):

		
		self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage4.pyc"
		if os.path.exists(self.saveName):
			open(self.saveName, "r").read().split()
		else:
			open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")
			
	def __pag5_button(self):

		
		self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage5.pyc"
		if os.path.exists(self.saveName):
			open(self.saveName, "r").read().split()
		else:
			open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")
			
	def __pag6_button(self):

		
		self.saveName = "lib/"+str(player.GetName())+"_SpeedEquipPage6.pyc"
		if os.path.exists(self.saveName):
			open(self.saveName, "r").read().split()
		else:
			open(self.saveName, "w").write("@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@\t@")

	def __change_button(self):
		for i in range(1,13):
			if self.tokens[i-1] != "@":
				net.SendItemUsePacket(int(self.tokens[i-1]))	

	def OverInItem(self, slotNumber):
		if self.tooltipItem:
			self.tooltipItem.SetInventoryItem(int(self.tokens[slotNumber-1]))

	def OnOverOutItem(self):
		if self.tooltipItem:
			self.tooltipItem.HideToolTip()

	def SelectItemSlot(self, itemSlotIndex):
		isAttached = mouseModule.mouseController.isAttached()
		if isAttached:
			attachedSlotType = mouseModule.mouseController.GetAttachedType()
			attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
			itemIndex = player.GetItemIndex(attachedSlotPos)
			itemCount = player.GetItemCount(attachedSlotPos)
			item.SelectItem(itemIndex)
			itemType = item.GetItemType()
			itemSubType = item.GetItemSubType()
			itemIndex2 = player.GetItemIndex(attachedSlotPos)
			
			if item.IsWearableFlag(item.WEARABLE_BODY):
				self.tokens[1-1] = attachedSlotPos
				snd.PlaySound("sound/ui/equip_metal_armor.wav")
			elif item.IsWearableFlag(item.WEARABLE_HEAD):
				self.tokens[2-1] = attachedSlotPos
				snd.PlaySound("sound/ui/drop.wav")
			elif item.IsWearableFlag(item.WEARABLE_FOOTS):
				self.tokens[3-1] = attachedSlotPos
				snd.PlaySound("sound/ui/drop.wav")
			elif item.IsWearableFlag(item.WEARABLE_WRIST):
				self.tokens[4-1] = attachedSlotPos
				snd.PlaySound("sound/ui/drop.wav")
			elif item.IsWearableFlag(item.WEARABLE_WEAPON):
				self.tokens[5-1] = attachedSlotPos
				if itemSubType == 2:
					snd.PlaySound("sound/ui/equip_bow.wav")
				else:
					snd.PlaySound("sound/ui/equip_metal_weapon.wav")
			elif item.IsWearableFlag(item.WEARABLE_NECK):
				self.tokens[6-1] = attachedSlotPos
				snd.PlaySound("sound/ui/equip_ring_amulet.wav")
			elif item.IsWearableFlag(item.WEARABLE_EAR):
				self.tokens[7-1] = attachedSlotPos
				snd.PlaySound("sound/ui/equip_ring_amulet.wav")
			elif item.IsWearableFlag(item.WEARABLE_SHIELD):
				self.tokens[8-1] = attachedSlotPos
				snd.PlaySound("sound/ui/drop.wav")
			elif item.IsWearableFlag(item.WEARABLE_ARROW):
				self.tokens[9-1] = attachedSlotPos
				snd.PlaySound("sound/ui/drop.wav")
			elif itemIndex >=18000 and itemIndex <= 18089:
				#self.tokens[10-1] = attachedSlotPos
				#snd.PlaySound("sound/ui/drop.wav")
				return
			elif itemIndex >=41001 and itemIndex <= 41513:
				self.tokens[11-1] = attachedSlotPos
				snd.PlaySound("sound/ui/drop.wav")
			elif itemIndex >=45001 and itemIndex <= 45206 or itemIndex >= 73001 and itemIndex <= 75620:
				self.tokens[12-1] = attachedSlotPos
				snd.PlaySound("sound/ui/drop.wav")
			else:
				return
			
			open(self.saveName, "w").write("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s" % tuple(self.tokens))	
			mouseModule.mouseController.DeattachObject()

	def OnUpdate(self):	
		if self.wndCostumeEquipment:
			self.wndCostumeEquipment.AdjustPositionAndSize()
			
		if selfs.FAST_EQUIP == 2:
			self.Close()
		else:
			pass
			
		if selfs.FAST_PAGE == 1:
			self.amountInput.SetText("I")
		elif selfs.FAST_PAGE == 2:
			self.amountInput.SetText("II")
		elif selfs.FAST_PAGE == 3:
			self.amountInput.SetText("III")
		elif selfs.FAST_PAGE == 4:
			self.amountInput.SetText("IV")
		elif selfs.FAST_PAGE == 5:
			self.amountInput.SetText("V")
		else:
			pass
	
		
			
		tokens = open(self.saveName, "r").read().split()		
		self.tokens = tokens
		for i in range(1,13):	
			if tokens[i-1] == "@":
				self.wndEquip.SetItemSlot(i, 0, 0)
				self.wndCostumeEquipment.wndCostumeEquipmentSlot.SetItemSlot(i, 0, 0)
			else:
				itemIndex = player.GetItemIndex(int(tokens[i-1]))
				if itemIndex != 0:
					item.SelectItem(itemIndex)
					if i == 1 and item.IsWearableFlag(item.WEARABLE_BODY):
						self.wndEquip.SetItemSlot(i, itemIndex, 0)
					elif i == 2 and item.IsWearableFlag(item.WEARABLE_HEAD):
						self.wndEquip.SetItemSlot(i, itemIndex, 0)
					elif i == 3 and item.IsWearableFlag(item.WEARABLE_FOOTS):
						self.wndEquip.SetItemSlot(i, itemIndex, 0)
					elif i == 4 and item.IsWearableFlag(item.WEARABLE_WRIST):
						self.wndEquip.SetItemSlot(i, itemIndex, 0)
					elif i == 5 and item.IsWearableFlag(item.WEARABLE_WEAPON):
						self.wndEquip.SetItemSlot(i, itemIndex, 0)
					elif i == 6 and item.IsWearableFlag(item.WEARABLE_NECK):
						self.wndEquip.SetItemSlot(i, itemIndex, 0)
					elif i == 7 and item.IsWearableFlag(item.WEARABLE_EAR):
						self.wndEquip.SetItemSlot(i, itemIndex, 0)
					elif i == 8 and item.IsWearableFlag(item.WEARABLE_SHIELD):
						self.wndEquip.SetItemSlot(i, itemIndex, 0)
					elif i == 9 and item.IsWearableFlag(item.WEARABLE_ARROW):
						self.wndEquip.SetItemSlot(i, itemIndex, 0)
					elif i == 10 and itemIndex >=18000 and itemIndex <= 18089:
						self.wndEquip.SetItemSlot(i, itemIndex, 0)
					elif i == 11 and itemIndex >=41001 and itemIndex <= 41512:
						self.wndCostumeEquipment.wndCostumeEquipmentSlot.SetItemSlot(i, itemIndex, 0)
					elif i == 12 and itemIndex >=45001 and itemIndex <= 45206 or itemIndex >= 73001 and itemIndex <= 75620:
						self.wndCostumeEquipment.wndCostumeEquipmentSlot.SetItemSlot(i, itemIndex, 0)
					else:
						self.wndEquip.SetItemSlot(i, 0, 0)
						self.wndCostumeEquipment.wndCostumeEquipmentSlot.SetItemSlot(i, 0, 0)
						self.tokens[i-1] = "@"
						open(self.saveName, "w").write("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s" % tuple(self.tokens))
						continue
				else:
					self.wndEquip.SetItemSlot(i, 0, 0)		
					self.wndCostumeEquipment.wndCostumeEquipmentSlot.SetItemSlot(i, 0, 0)	


	def OnMoveWindow(self, x, y):
		if self.wndCostumeEquipment:
			self.wndCostumeEquipment.AdjustPositionAndSize()
	
	def OnPressEscapeKey(self):
		self.Close()
		#snd.PlaySound("sound/ui/click.wav")
		return TRUE

		
