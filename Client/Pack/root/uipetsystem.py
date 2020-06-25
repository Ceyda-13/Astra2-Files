import ui
import net
import app
import item
import grp
import localeInfo
import constInfo
import wndMgr
import petskill
import uipetfeed
import uiToolTip
import uiScriptLocale
import mouseModule
import player
import __builtin__ as selfs

SKILL_SLOT_ENABLE = "d:/ymir work/ui/pet/skill_button/skill_enable_button.sub"
SKILL_SLOT_MAX = 3

AFFECT_DICT = {
		item.APPLY_MAX_HP : localeInfo.TOOLTIP_MAX_HP,
		item.APPLY_MAX_SP : localeInfo.TOOLTIP_MAX_SP,
		item.APPLY_CON : localeInfo.TOOLTIP_CON,
		item.APPLY_INT : localeInfo.TOOLTIP_INT,
		item.APPLY_STR : localeInfo.TOOLTIP_STR,
		item.APPLY_DEX : localeInfo.TOOLTIP_DEX,
		item.APPLY_ATT_SPEED : localeInfo.TOOLTIP_ATT_SPEED,
		item.APPLY_MOV_SPEED : localeInfo.TOOLTIP_MOV_SPEED,
		item.APPLY_CAST_SPEED : localeInfo.TOOLTIP_CAST_SPEED,
		item.APPLY_HP_REGEN : localeInfo.TOOLTIP_HP_REGEN,
		item.APPLY_SP_REGEN : localeInfo.TOOLTIP_SP_REGEN,
		item.APPLY_POISON_PCT : localeInfo.TOOLTIP_APPLY_POISON_PCT,
		item.APPLY_STUN_PCT : localeInfo.TOOLTIP_APPLY_STUN_PCT,
		item.APPLY_SLOW_PCT : localeInfo.TOOLTIP_APPLY_SLOW_PCT,
		item.APPLY_CRITICAL_PCT : localeInfo.TOOLTIP_APPLY_CRITICAL_PCT,
		item.APPLY_PENETRATE_PCT : localeInfo.TOOLTIP_APPLY_PENETRATE_PCT,

		item.APPLY_ATTBONUS_WARRIOR : localeInfo.TOOLTIP_APPLY_ATTBONUS_WARRIOR,
		item.APPLY_ATTBONUS_ASSASSIN : localeInfo.TOOLTIP_APPLY_ATTBONUS_ASSASSIN,
		item.APPLY_ATTBONUS_SURA : localeInfo.TOOLTIP_APPLY_ATTBONUS_SURA,
		item.APPLY_ATTBONUS_SHAMAN : localeInfo.TOOLTIP_APPLY_ATTBONUS_SHAMAN,
		item.APPLY_ATTBONUS_MONSTER : localeInfo.TOOLTIP_APPLY_ATTBONUS_MONSTER,

		item.APPLY_ATTBONUS_HUMAN : localeInfo.TOOLTIP_APPLY_ATTBONUS_HUMAN,
		item.APPLY_ATTBONUS_ANIMAL : localeInfo.TOOLTIP_APPLY_ATTBONUS_ANIMAL,
		item.APPLY_ATTBONUS_ORC : localeInfo.TOOLTIP_APPLY_ATTBONUS_ORC,
		item.APPLY_ATTBONUS_MILGYO : localeInfo.TOOLTIP_APPLY_ATTBONUS_MILGYO,
		item.APPLY_ATTBONUS_UNDEAD : localeInfo.TOOLTIP_APPLY_ATTBONUS_UNDEAD,
		item.APPLY_ATTBONUS_DEVIL : localeInfo.TOOLTIP_APPLY_ATTBONUS_DEVIL,
		item.APPLY_STEAL_HP : localeInfo.TOOLTIP_APPLY_STEAL_HP,
		item.APPLY_STEAL_SP : localeInfo.TOOLTIP_APPLY_STEAL_SP,
		item.APPLY_MANA_BURN_PCT : localeInfo.TOOLTIP_APPLY_MANA_BURN_PCT,
		item.APPLY_DAMAGE_SP_RECOVER : localeInfo.TOOLTIP_APPLY_DAMAGE_SP_RECOVER,
		item.APPLY_BLOCK : localeInfo.TOOLTIP_APPLY_BLOCK,
		item.APPLY_DODGE : localeInfo.TOOLTIP_APPLY_DODGE,
		item.APPLY_RESIST_SWORD : localeInfo.TOOLTIP_APPLY_RESIST_SWORD,
		item.APPLY_RESIST_TWOHAND : localeInfo.TOOLTIP_APPLY_RESIST_TWOHAND,
		item.APPLY_RESIST_DAGGER : localeInfo.TOOLTIP_APPLY_RESIST_DAGGER,
		item.APPLY_RESIST_BELL : localeInfo.TOOLTIP_APPLY_RESIST_BELL,
		item.APPLY_RESIST_FAN : localeInfo.TOOLTIP_APPLY_RESIST_FAN,
		item.APPLY_RESIST_BOW : localeInfo.TOOLTIP_RESIST_BOW,
		item.APPLY_RESIST_FIRE : localeInfo.TOOLTIP_RESIST_FIRE,
		item.APPLY_RESIST_ELEC : localeInfo.TOOLTIP_RESIST_ELEC,
		item.APPLY_RESIST_MAGIC : localeInfo.TOOLTIP_RESIST_MAGIC,
		item.APPLY_RESIST_WIND : localeInfo.TOOLTIP_APPLY_RESIST_WIND,
		item.APPLY_REFLECT_MELEE : localeInfo.TOOLTIP_APPLY_REFLECT_MELEE,
		item.APPLY_REFLECT_CURSE : localeInfo.TOOLTIP_APPLY_REFLECT_CURSE,
		item.APPLY_POISON_REDUCE : localeInfo.TOOLTIP_APPLY_POISON_REDUCE,
		item.APPLY_KILL_SP_RECOVER : localeInfo.TOOLTIP_APPLY_KILL_SP_RECOVER,
		item.APPLY_EXP_DOUBLE_BONUS : localeInfo.TOOLTIP_APPLY_EXP_DOUBLE_BONUS,
		item.APPLY_GOLD_DOUBLE_BONUS : localeInfo.TOOLTIP_APPLY_GOLD_DOUBLE_BONUS,
		item.APPLY_ITEM_DROP_BONUS : localeInfo.TOOLTIP_APPLY_ITEM_DROP_BONUS,
		item.APPLY_POTION_BONUS : localeInfo.TOOLTIP_APPLY_POTION_BONUS,
		item.APPLY_KILL_HP_RECOVER : localeInfo.TOOLTIP_APPLY_KILL_HP_RECOVER,
		item.APPLY_IMMUNE_STUN : localeInfo.TOOLTIP_APPLY_IMMUNE_STUN,
		item.APPLY_IMMUNE_SLOW : localeInfo.TOOLTIP_APPLY_IMMUNE_SLOW,
		item.APPLY_IMMUNE_FALL : localeInfo.TOOLTIP_APPLY_IMMUNE_FALL,
		item.APPLY_BOW_DISTANCE : localeInfo.TOOLTIP_BOW_DISTANCE,
		item.APPLY_DEF_GRADE_BONUS : localeInfo.TOOLTIP_DEF_GRADE,
		item.APPLY_ATT_GRADE_BONUS : localeInfo.TOOLTIP_ATT_GRADE,
		item.APPLY_MAGIC_ATT_GRADE : localeInfo.TOOLTIP_MAGIC_ATT_GRADE,
		item.APPLY_MAGIC_DEF_GRADE : localeInfo.TOOLTIP_MAGIC_DEF_GRADE,
		item.APPLY_MAX_STAMINA : localeInfo.TOOLTIP_MAX_STAMINA,
		item.APPLY_MALL_ATTBONUS : localeInfo.TOOLTIP_MALL_ATTBONUS,
		item.APPLY_MALL_DEFBONUS : localeInfo.TOOLTIP_MALL_DEFBONUS,
		item.APPLY_MALL_EXPBONUS : localeInfo.TOOLTIP_MALL_EXPBONUS,
		item.APPLY_MALL_ITEMBONUS : localeInfo.TOOLTIP_MALL_ITEMBONUS,
		item.APPLY_MALL_GOLDBONUS : localeInfo.TOOLTIP_MALL_GOLDBONUS,
		item.APPLY_SKILL_DAMAGE_BONUS : localeInfo.TOOLTIP_SKILL_DAMAGE_BONUS,
		item.APPLY_NORMAL_HIT_DAMAGE_BONUS : localeInfo.TOOLTIP_NORMAL_HIT_DAMAGE_BONUS,
		item.APPLY_SKILL_DEFEND_BONUS : localeInfo.TOOLTIP_SKILL_DEFEND_BONUS,
		item.APPLY_NORMAL_HIT_DEFEND_BONUS : localeInfo.TOOLTIP_NORMAL_HIT_DEFEND_BONUS,
		item.APPLY_PC_BANG_EXP_BONUS : localeInfo.TOOLTIP_MALL_EXPBONUS_P_STATIC,
		item.APPLY_PC_BANG_DROP_BONUS : localeInfo.TOOLTIP_MALL_ITEMBONUS_P_STATIC,
		item.APPLY_RESIST_WARRIOR : localeInfo.TOOLTIP_APPLY_RESIST_WARRIOR,
		item.APPLY_RESIST_ASSASSIN : localeInfo.TOOLTIP_APPLY_RESIST_ASSASSIN,
		item.APPLY_RESIST_SURA : localeInfo.TOOLTIP_APPLY_RESIST_SURA,
		item.APPLY_RESIST_SHAMAN : localeInfo.TOOLTIP_APPLY_RESIST_SHAMAN,
		item.APPLY_MAX_HP_PCT : localeInfo.TOOLTIP_APPLY_MAX_HP_PCT,
		item.APPLY_MAX_SP_PCT : localeInfo.TOOLTIP_APPLY_MAX_SP_PCT,
		item.APPLY_ENERGY : localeInfo.TOOLTIP_ENERGY,
		item.APPLY_COSTUME_ATTR_BONUS : localeInfo.TOOLTIP_COSTUME_ATTR_BONUS,
		
		item.APPLY_MAGIC_ATTBONUS_PER : localeInfo.TOOLTIP_MAGIC_ATTBONUS_PER,
		item.APPLY_MELEE_MAGIC_ATTBONUS_PER : localeInfo.TOOLTIP_MELEE_MAGIC_ATTBONUS_PER,
		item.APPLY_RESIST_ICE : localeInfo.TOOLTIP_RESIST_ICE,
		item.APPLY_RESIST_EARTH : localeInfo.TOOLTIP_RESIST_EARTH,
		item.APPLY_RESIST_DARK : localeInfo.TOOLTIP_RESIST_DARK,
		item.APPLY_ANTI_CRITICAL_PCT : localeInfo.TOOLTIP_ANTI_CRITICAL_PCT,
		item.APPLY_ANTI_PENETRATE_PCT : localeInfo.TOOLTIP_ANTI_PENETRATE_PCT,
	}

if ENABLE_WOLFMAN_CHARACTER:
	AFFECT_DICT[item.APPLY_RESIST_WOLFMAN] = localeInfo.TOOLTIP_APPLY_RESIST_WOLFMAN

if ENABLE_NEW_BONUSES:
	AFFECT_DICT[item.APPLY_ATTBONUS_STONE] = localeInfo.TOOLTIP_ATTBONUS_STONE_PCT
	AFFECT_DICT[item.APPLY_CAST_SPEED] = localeInfo.TOOLTIP_ATTBONUS_BOSS
	AFFECT_DICT[item.APPLY_ATTBONUS_BOSS] = localeInfo.TOOLTIP_ATTBONUS_BOSS

def checkdiv(n):
	x = str(n/10.0)
	if len(x) > 3:
		return str(x)[0:3]
	return str(x)

def pointop(n):
	t = int(n)
	if t / 10 < 1:
		return "0."+n
	else:		
		return n[0:len(n)-1]+"."+n[len(n)-1:]
		
def GetAffectString(affectType, affectValue):
	if 0 == affectType:
		return None

	if 0 == affectValue:
		return None

	try:
		return AFFECT_DICT[affectType](affectValue)
	except TypeError:
		return "UNKNOWN_VALUE[%s] %s" % (affectType, affectValue)
	except KeyError:
		return "UNKNOWN_TYPE[%s] %s" % (affectType, affectValue)
		
class PetSystemMain(ui.ScriptWindow):
	def __init__(self, vnum = 0):
		ui.ScriptWindow.__init__(self)
		self.vnum = vnum
		self.__LoadWindow()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def Show(self):
		ui.ScriptWindow.Show(self)

	def Close(self):
		self.Hide()
		self.PetPos = -1
		self.feedwind.Close()
	
	def __LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/PetInformationWindow.py")
		except:
			import exception
			exception.Abort("PetInformationWindow.LoadWindow.LoadObject")
			
		try:
			if localeInfo.IsARABIC():
				self.GetChild("PetInfoUIBG").LeftRightReverse()

			self.feedwind = uipetfeed.PetFeedWindow()
			self.board = self.GetChild("board")
			self.boardclose = self.GetChild("ClosePetInfoWndButton")
			
			self.wndUpBringingPetSlot = self.GetChild("UpBringing_Pet_Slot")
			self.wndUpBringingPetSlot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)
			if localeInfo.IsARABIC():
				self.wndUpBringingPetSlot.SetPosition(295,55)

			self.evolname = self.GetChild("EvolName")
			self.petname = self.GetChild("PetName")
			
			self.expwind = self.GetChild("UpBringing_Pet_EXP_Gauge_Board")
			self.tooltipEXP = TextToolTip()
			self.tooltipEXP.Hide()

			self.lifeTimeGauge = self.GetChild("LifeGauge")
			self.lifeTimeGauge.SetScale(1.61, 1.0)
			self.lifeTimeGauge.SetWindowHorizontalAlignLeft()
			
			if localeInfo.IsARABIC():
				self.lifeTimeGauge.SetPosition(26,0)

			self.petlevel = self.GetChild("LevelValue")
			self.petexpa = self.GetChild("UpBringing_Pet_EXPGauge_01")
			self.petexpb = self.GetChild("UpBringing_Pet_EXPGauge_02")
			self.petexpc = self.GetChild("UpBringing_Pet_EXPGauge_03")
			self.petexpd = self.GetChild("UpBringing_Pet_EXPGauge_04")
			self.petexpe = self.GetChild("UpBringing_Pet_EXPGauge_05")
			self.petexppages = []			
			self.petexppages.append(self.petexpa)
			self.petexppages.append(self.petexpb)
			self.petexppages.append(self.petexpc)
			self.petexppages.append(self.petexpd)
			self.petexppages.append(self.petexpe)
			
			for exp in self.petexppages:
				exp.SetSize(0, 0)
			
			self.petages = self.GetChild("AgeValue")
			
			self.petdur = self.GetChild("LifeTextValue")

			self.nutribtn = self.GetChild("FeedLifeTimeButton")
			self.sviluppobtn = self.GetChild("FeedEvolButton")
			self.itemexp = self.GetChild("FeedExpButton")
			
			self.pethp = self.GetChild("HpValue")
			self.petdef = self.GetChild("DefValue")
			self.petsp = self.GetChild("SpValue")

			self.Tab_Button_01 = self.GetChild("Tab_Button_01")
			self.Tab_Button_01.SetEvent(ui.__mem_func__(self.RefreshState), 0)
			self.Tab_01 = self.GetChild("Tab_01")
			self.Tab_Button_02 = self.GetChild("Tab_Button_02")
			self.Tab_Button_02.SetEvent(ui.__mem_func__(self.RefreshState), 1)
			self.Tab_02 = self.GetChild("Tab_02")
			self.DetermineButton = self.GetChild("DetermineButton")
			self.DetermineButton.SetEvent(ui.__mem_func__(self.Determine))
			self.Pet_Change_Button = self.GetChild("Pet_Change_Button")
			self.Pet_Change_Button.SetEvent(ui.__mem_func__(self.ChangeAttr))
			self.Pet_OK_Button = self.GetChild("Pet_OK_Button")
			self.Pet_OK_Button.SetEvent(ui.__mem_func__(self.RefreshState), 0)
			self.PetInfo_Page = self.GetChild("PetInfo_Page")
			self.PetAttrChange_Page = self.GetChild("PetAttrChange_Page")
			self.PetDetermineInfoText1 = self.GetChild("PetDetermineInfoText1")
			self.PetDetermineInfoText2 = self.GetChild("PetDetermineInfoText2")
			self.PetDetermineInfoText3 = self.GetChild("PetDetermineInfoText3")
			self.PetDetermineInfoText4 = self.GetChild("PetDetermineInfoText4")
			self.Change_Pet_Slot = self.GetChild("Change_Pet_Slot")
			self.ClosePetChangeWndButton = self.GetChild("ClosePetChangeWndButton")
			self.PetType = self.GetChild("PetType")
			self.Change_Pet_Slot.SetSelectItemSlotEvent(ui.__mem_func__(self.SelectItemSlot))
			self.Change_Pet_Slot.SetSelectEmptySlotEvent(ui.__mem_func__(self.SelectEmptySlot))
			self.Change_Pet_Slot.SetOverInItemEvent(ui.__mem_func__(self.OverInItem))
			self.Change_Pet_Slot.SetOverOutItemEvent(ui.__mem_func__(self.OverOutItem))
			self.ClosePetChangeWndButton.SetEvent(ui.__mem_func__(self.Close))
			self.PetPos = -1
			self.RefreshState(0)

			arabic_start_pos_x = 36
			self.skillSlot = []
			for value in range(SKILL_SLOT_MAX):
				self.skillSlot.append(self.GetChild("PetSkillSlot"+str(value)))
				self.skillSlot[value].ClearSlot(value)
				# self.skillSlot[value].SetSlot(0, 2, 32, 32, petskill.GetEmptySkill())
				self.skillSlot[value].SetCoverButton(value, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE, False, False)
				self.skillSlot[value].SetAlwaysRenderCoverButton(value)

				self.skillSlot[value].AppendSlotButton("d:/ymir work/ui/game/windows/btn_plus_up.sub",\
											"d:/ymir work/ui/game/windows/btn_plus_over.sub",\
											"d:/ymir work/ui/game/windows/btn_plus_down.sub")
				if localeInfo.IsARABIC():
					## 36, 100, 164
					self.skillSlot[value].SetPosition(arabic_start_pos_x, 365)
					arabic_start_pos_x = arabic_start_pos_x + 64

				self.skillSlot[value].SetSelectItemSlotEvent(ui.__mem_func__(self.RevertPet))
				self.skillSlot[value].SetUseSlotEvent(ui.__mem_func__(self.RevertPet))
				self.skillSlot[value].SetOverInItemEvent(ui.__mem_func__(self.PetSkillTooltipShow))
				self.skillSlot[value].SetOverOutItemEvent(ui.__mem_func__(self.PetSkillTooltipHide))	
			
			self.SetDefaultInfo()
			
			self.arrytooltip = [ [-1,-1], [-1,-1], [-1,-1]]
			PET_FILE_NAME = "%s/pet_skill.txt" % app.GetLocalePath()
			PET_FILE_SKILL = "%s/pet_skill_bonus.txt" % app.GetLocalePath()
			self.linespet = pack_open(PET_FILE_NAME, "r").readlines()
			self.linespetskill = pack_open(PET_FILE_SKILL, "r").readlines()
			self.SkillTooltip = uiToolTip.ToolTip(180)

			self.boardclose.SetEvent(ui.__mem_func__(self.Close,))
			self.nutribtn.SetToggleDownEvent(lambda arg=0,arg1=1: self.OpenFeedBox(arg,arg1))
			self.nutribtn.SetToggleUpEvent(lambda arg=1,arg1=0: self.OpenFeedBox(arg,arg1))
			self.itemexp.SetToggleDownEvent(lambda arg=0,arg1=3: self.OpenFeedBox(arg,arg1))
			self.itemexp.SetToggleUpEvent(lambda arg=1,arg1=0: self.OpenFeedBox(arg,arg1))
			self.sviluppobtn.SetEvent(ui.__mem_func__(self.evolution))
		except:
			import exception
			exception.Abort("PetInformationWindow.LoadWindow.BindObject")

	def OverOutItem(self):
		if selfs.wndToolTipItem:
			selfs.wndToolTipItem.HideToolTip()

	def OverInItem(self, overSlotPos):
		if overSlotPos == 0 or overSlotPos == 2:
			if selfs.wndToolTipItem:
				selfs.wndToolTipItem.SetInventoryItem(self.PetPos)
		else:
			if selfs.wndToolTipItem:
				selfs.wndToolTipItem.ClearToolTip()
				selfs.wndToolTipItem.AddItemData(55033)

	def SelectItemSlot(self, index):
		if index == 0:
			self.Change_Pet_Slot.SetItemSlot(0, 0, 0)
			self.Change_Pet_Slot.SetItemSlot(1, 0, 0)
			self.Change_Pet_Slot.SetItemSlot(2, 0, 0)

	def SelectEmptySlot(self, index):
		if constInfo.GET_ITEM_QUESTION_DIALOG_STATUS() == 1:
			return

		if mouseModule.mouseController.isAttached():
			if index == 0:
				attachedSlotType = mouseModule.mouseController.GetAttachedType()
				attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
				attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()
				attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()

				if player.SLOT_TYPE_INVENTORY == attachedSlotType:
					item.SelectItem(attachedItemIndex)
					if item.ITEM_TYPE_PET == item.GetItemType() and item.PET_UPBRINGING == item.GetItemSubType():
						self.PetPos = attachedSlotPos
						self.Change_Pet_Slot.SetItemSlot(0, attachedItemIndex, 0)
						self.Change_Pet_Slot.SetItemSlot(1, 55033, 0)
						self.Change_Pet_Slot.SetItemSlot(2, 0, 0)

						self.PetDetermineInfoText1.SetText("")
						self.PetDetermineInfoText2.SetText("")
						self.PetDetermineInfoText3.SetText("")
						self.PetDetermineInfoText4.SetText("")

			mouseModule.mouseController.DeattachObject()

	def ResultAttr(self, a, b, c, type, pos):
		self.PetPos = pos
		self.Change_Pet_Slot.SetItemSlot(0, 0, 0)
		self.Change_Pet_Slot.SetItemSlot(1, 0, 0)
		self.Change_Pet_Slot.SetItemSlot(2, player.GetItemIndex(pos), 0)
		self.PetDetermineInfoText1.SetText("Yeni Pet Tipi: " + constInfo.GetPetType(type) + "(%d)" % type)
		self.PetDetermineInfoText2.SetText("Yeni HP: +" + pointop(str(a)) + "%")
		self.PetDetermineInfoText3.SetText("Yeni Defans: +" + pointop(str(b)) + "%")
		self.PetDetermineInfoText4.SetText("Yeni STR: +" + pointop(str(c)) + "%")

	def PetTypee(self, typee):
		self.PetType.SetText("Pet Tipi: " + constInfo.GetPetType(int(typee)) + "(%s)" % typee)

	def ChangeAttr(self):
		if self.PetPos != -1:
			net.SendChatPacket("/petattr %d" % self.PetPos)

	def Determine(self):
		net.SendChatPacket("/determine")

	def RefreshState(self, i):	
		if i == 0:
			self.Tab_Button_02.SetUp()
			self.PetAttrChange_Page.Hide()
			self.PetInfo_Page.Show()
			self.Tab_01.Show()
			self.Tab_02.Hide()
		elif i == 1:
			self.Tab_Button_01.SetUp()
			self.PetAttrChange_Page.Show()
			self.PetInfo_Page.Hide()
			self.Tab_02.Show()
			self.Tab_01.Hide()

	def PetSkillTooltipShow(self, slot):
		if self.arrytooltip[slot][0] > 0:
			tokens = self.linespet[self.arrytooltip[slot][0]-1][:-1].split("\t")
			tokens2 = self.linespetskill[self.arrytooltip[slot][0]-1][:-1].split("\t")

			self.SkillTooltip.ClearToolTip()
			self.SkillTooltip.AutoAppendTextLine(tokens[1], grp.GenerateColor(0.9490, 0.9058, 0.7568, 1.0))
			self.SkillTooltip.AppendDescription(tokens[4], 26)
			self.SkillTooltip.AppendSpace(5)

			if self.arrytooltip[slot][0] != 10 and self.arrytooltip[slot][0] != 17 and self.arrytooltip[slot][0] != 18:
				self.SkillTooltip.AutoAppendTextLine(GetAffectString(int(tokens2[1]), int(tokens2[self.arrytooltip[slot][1]+1])))
			elif self.arrytooltip[slot][0] == 10:
				self.SkillTooltip.AutoAppendTextLine(localeInfo.PET_SYSTEM_HP_RESTORED + str(tokens2[self.arrytooltip[slot][1]+1]))
			elif self.arrytooltip[slot][0] == 17:
				self.SkillTooltip.AutoAppendTextLine(localeInfo.PET_SYSTEM_IMMORTAL_TIME + checkdiv(int(tokens2[self.arrytooltip[slot][1]+1])) + "saniye")

			self.SkillTooltip.AutoAppendTextLine(localeInfo.PET_SYSTEM_SKILL_COOLDOWN + tokens[5] + "saniye", grp.GenerateColor(1.0, 0.7843, 0.0, 1.0))
			self.SkillTooltip.AlignHorizonalCenter()
			self.SkillTooltip.ShowToolTip()

	def PetSkillTooltipHide(self):
		self.SkillTooltip.HideToolTip()

	def evolution(self):
		net.SendChatPacket("/petvoincrease")
		
	def SetDefaultInfo(self):
		self.evolname.SetText("")
		self.petname.SetText("")
		self.PetType.SetText("")
		
		self.petlevel.SetText("")
		self.petages.SetText("")
		self.petdur.SetText("")
		
		self.pethp.SetText("")
		self.petdef.SetText("")
		self.petsp.SetText("")
		
		self.SetDuration("0", "0", "0")
		
		self.wndUpBringingPetSlot.ClearSlot(0)

		self.SetExperience(0,0,0)
		
		self.arrytooltip = [ [-1,-1], [-1,-1], [-1,-1]]
		
		self.nutribtn.Disable()
		self.sviluppobtn.Disable()
		self.itemexp.Disable()
	
	def OpenFeedBox(self, mode, btn):
		if constInfo.FEEDWIND == btn or constInfo.FEEDWIND == 0:
			if mode == 0:
				self.feedwind.Show()
				constInfo.FEEDWIND = btn
			else:
				self.feedwind.Close()
				constInfo.FEEDWIND = 0
		else:
			self.nutribtn.Enable()
			self.sviluppobtn.Enable()
			self.itemexp.Enable()
			self.feedwind.Close()
			constInfo.FEEDWIND = 0			
	
	def SetImageSlot(self, vnum):
		self.wndUpBringingPetSlot.SetItemSlot(0, int(vnum), 0)
		self.wndUpBringingPetSlot.SetAlwaysRenderCoverButton(0, True)

	def SetEvolveName(self, evol_level):
		self.evolname.SetText(constInfo.GetEvolName(evol_level))
	
	def SetName(self, name):
		if name != "":
			self.nutribtn.Enable()
			self.sviluppobtn.Enable()
			self.itemexp.Enable()
		else:
			self.nutribtn.Disable()
			self.sviluppobtn.Disable()
			self.itemexp.Disable()
			
		self.petname.SetText(name)
	
	def SetLevel(self, level):
		if int(level) == 40 or int(level) == 60 or int(level) == 80:
			constInfo.EVOLUTION = int(level)
		else:
			constInfo.EVOLUTION = 0
		self.petlevel.SetText(level)
	
	def SetAges(self, ages):
		self.petages.SetText(ages)
	
	def SetDuration(self, dur, durt, dura):
		dur1 = int(dur) / 60
		durt1 = int(durt) / 60
		tmpage = int((int(durt) / 60 - int(dur) / 60) / 24)		
		if int(dur) > 0:
			self.lifeTimeGauge.SetPercentage(int(dur), int(durt))
			self.lifeTimeGauge.Show()
		else:
			self.lifeTimeGauge.Hide()
		self.GetChild("LifeTextValue").SetText(localeInfo.SecondToH(int(dur)) + " / " + localeInfo.SecondToH(int(durt)) + " " + uiScriptLocale.PET_INFORMATION_LIFE_TIME)
		birthSec = max(0, app.GetGlobalTimeStamp() - int(dura))
		self.SetAges(localeInfo.SecondToDay(birthSec))
		
	def SetHp(self, hp):
		self.pethp.SetText("+" + pointop(hp) + "%")
		
	def SetDef(self, deff):
		self.petdef.SetText("+" + pointop(deff) + "%")
		
	def SetSp(self, sp):
		self.petsp.SetText("+" + pointop(sp) + "%")
	
	def SetSkill(self, slot, idx, lv):
		if int(idx) != -1:
			self.skillSlot[int(slot)].ClearSlot(int(slot))
			self.skillSlot[int(slot)].SetPetSkillSlot(int(slot), int(idx), int(lv))
			self.skillSlot[int(slot)].SetCoverButton(int(slot))
			self.skillSlot[int(slot)].SetAlwaysRenderCoverButton(int(slot), True)
			self.arrytooltip[int(slot)][0] = int(idx)
			self.arrytooltip[int(slot)][1] = int(lv)
		else:
			self.skillSlot[int(slot)].ClearSlot(int(slot))
			self.skillSlot[int(slot)].SetCoverButton(int(slot), SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE, SKILL_SLOT_ENABLE, False, False)
			self.skillSlot[int(slot)].SetAlwaysRenderCoverButton(int(slot), True)

	def SetExperience(self, expm, expi, exptot):
		expm = int(expm)
		expi = int(expi)
		exptot = int(exptot)

		if exptot > 0:	
			totalexp = exptot
			totexpm = int( float(totalexp) / 100 * 90 )
			totexpi = totalexp - totexpm
			expi = min(expi, totexpi)
			expmp =  float(expm) / totexpm * 100
			expip =  float(expi) / totexpi * 100
		else:
			totalexp = 0
			totexpm = 0
			totexpi = 0
			expmp =  0
			expip =  0

		curPoint = int(min(expm, totexpm))
		curPoint = int(max(expm, 0))
		maxPoint = int(max(totexpm, 0))
		
		curPointi = int(min(expi, totexpi))
		curPointi = int(max(expi, 0))
		maxPointi = int(max(totexpi, 0))

		quarterPoint = maxPoint / 4
		quarterPointi = maxPointi 
		FullCount = 0
		FullCounti = 0

		if 0 != quarterPoint:
			FullCount = min(4, curPoint / quarterPoint)
			
		if 0 != quarterPointi:
			FullCounti = min(1, curPointi / quarterPointi)

		for i in xrange(4):
			self.petexppages[i].Hide()
			
		self.petexppages[4].Hide()

		for i in xrange(FullCount):
			self.petexppages[i].SetRenderingRect(0.0, 0.0, 0.0, 0.0)
			self.petexppages[i].Show()
			
		for i in xrange(FullCounti):
			self.petexppages[4].SetRenderingRect(0.0, 0.0, 0.0, 0.0)
			self.petexppages[4].Show()

		if 0 != quarterPoint:
			if FullCount < 4:
				Percentage = float(curPoint % quarterPoint) / quarterPoint - 1.0
				self.petexppages[FullCount].SetRenderingRect(0.0, Percentage, 0.0, 0.0)
				self.petexppages[FullCount].Show()
				
		if 0 != quarterPointi:
			if FullCounti < 1:
				Percentage = float(curPointi % quarterPointi) / quarterPointi - 1.0
				self.petexppages[4].SetRenderingRect(0.0, Percentage, 0.0, 0.0)
				self.petexppages[4].Show()

		if localeInfo.IsARABIC():
			tooltip_text = '%.2f%%' % expmp				+ ' :'	+ str(localeInfo.PET_INFO_EXP)			+ '\\n'	\
						 + str(totexpm)					+ ' :'	+ str(localeInfo.PET_INFO_NEXT_EXP)		+ '\\n'	\
						 + '%.2f%%' % expip				+ ' :'	+ str(localeInfo.PET_INFO_ITEM_EXP)		+ '\\n'	\
						 + str(totexpi)					+ ' :'	+ str(localeInfo.PET_INFO_NEXT_ITEM_EXP)	
			self.tooltipEXP.SetText(tooltip_text)
		else:
			tooltip_text = str(localeInfo.PET_INFO_EXP) 			+ ': ' + '%.2f%%' % expmp + '\\n'	\
						 + str(localeInfo.PET_INFO_NEXT_EXP) 		+ ': ' + str(totexpm) + '\\n'	\
						 + str(localeInfo.PET_INFO_ITEM_EXP) 		+ ': ' + '%.2f%%' % expip + '\\n'	\
						 + str(localeInfo.PET_INFO_NEXT_ITEM_EXP)	+ ': ' + str(totexpi)
						 
			self.tooltipEXP.SetText(tooltip_text)

	def RevertPet(self, slot):
		if mouseModule.mouseController.isAttached():
			attachedSlotType = mouseModule.mouseController.GetAttachedType()
			attachedSlotPos = mouseModule.mouseController.GetAttachedSlotNumber()
			attachedItemCount = mouseModule.mouseController.GetAttachedItemCount()
			attachedItemIndex = mouseModule.mouseController.GetAttachedItemIndex()

			if player.SLOT_TYPE_INVENTORY == attachedSlotType:
				if attachedItemIndex == 55028:
					net.SendChatPacket("/petrevert "+str(slot))

			mouseModule.mouseController.DeattachObject()

	def OnUpdate(self):
		if constInfo.FEEDWIND == 0:
			self.nutribtn.Enable()
			self.itemexp.Enable()
		if True == self.expwind.IsIn():
			self.tooltipEXP.Show()
		else:
			self.tooltipEXP.Hide()

class PetSystemMini(ui.ScriptWindow):
	def __init__(self, vnum = 0):
		ui.ScriptWindow.__init__(self)
		self.vnum = vnum
		self.__LoadWindow()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def Show(self):
		ui.ScriptWindow.Show(self)

	def Close(self):
		self.Hide()
	
	def __LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/PetMiniInformationWindow.py")
		except:
			import exception
			exception.Abort("PetMiniInformationWindow.LoadWindow.LoadObject")
		try:
			self.expwind = self.GetChild("pet_mini_info_exp_gauge_board")

			if localeInfo.IsARABIC():
				self.GetChild("main_bg").LeftRightReverse()

			self.mainicon = self.GetChild("main_slot_img")

			self.petSlot = self.GetChild("pet_icon_slot")
			self.petSlot.SetSlotStyle(wndMgr.SLOT_STYLE_NONE)			
			# self.petSlot.SAFE_SetButtonEvent("LEFT", "EXIST", self.SelectItemSlot)
			if localeInfo.IsARABIC():
				self.petSlot.SetPosition(0,6)

			self.tooltipEXP = TextToolTip()
			self.tooltipEXP.Hide()

			self.petSlotAniImg = self.GetChild("pet_icon_slot_ani_img")
			# self.petSlotAniImg.Hide()
			if localeInfo.IsARABIC():
				self.petSlotAniImg.SetPosition(34, 3)

			self.pet_mini_exp_01 = self.GetChild("pet_mini_EXPGauge_01")
			self.pet_mini_exp_02 = self.GetChild("pet_mini_EXPGauge_02")
			self.pet_mini_exp_03 = self.GetChild("pet_mini_EXPGauge_03")
			self.pet_mini_exp_04 = self.GetChild("pet_mini_EXPGauge_04")
			self.pet_mini_exp_05 = self.GetChild("pet_mini_EXPGauge_05")
			self.petmini_exp = []
			self.petmini_exp.append(self.pet_mini_exp_01)
			self.petmini_exp.append(self.pet_mini_exp_02)
			self.petmini_exp.append(self.pet_mini_exp_03)
			self.petmini_exp.append(self.pet_mini_exp_04)
			self.petmini_exp.append(self.pet_mini_exp_05)

			self.skillSlot = []
			for value in range(SKILL_SLOT_MAX):
				self.skillSlot.append(self.GetChild("mini_skill_slot"+str(value)))
				
				if localeInfo.IsARABIC():
					arabic_start_pos_x = -23
					self.skillSlot[value].SetPosition(arabic_start_pos_x, 0)

				self.skillSlot[value].ClearSlot(0)
				self.skillSlot[value].SetSlotScale(0, 2, 16, 16, petskill.GetEmptySkill(), 0.5, 0.5)
				# self.skillSlot[value].SetSelectItemSlotEvent(ui.__mem_func__(self.UseSkill))
				# self.skillSlot[value].SetUseSlotEvent(ui.__mem_func__(self.UseSkill))

			self.lifeTimeGauge = self.GetChild("LifeGauge")
			self.lifeTimeGauge.SetWindowHorizontalAlignLeft()
			if localeInfo.IsARABIC():
				self.GetChild("gauge_left").LeftRightReverse()
				self.GetChild("gauge_right").LeftRightReverse()

			self.SetDefaultInfo()
		except:
			import exception
			exception.Abort("PetMiniInformationWindow.LoadWindow.BindObject")

	def SetDefaultInfo(self):		
		self.SetDuration("0", "0", "0")
		self.petSlot.ClearSlot(0)
		self.SetExperience(0, 0, 0)

	def SetImageSlot(self, vnum):
		self.petSlot.SetItemSlot(0, int(vnum), 0)
		self.petSlot.SetAlwaysRenderCoverButton(0, True)

	def SetDuration(self, dur, durt, dura):
		tmpage = int((int(durt)/60 -int(dur) /60)/24)		
		if int(dur) > 0:
			self.lifeTimeGauge.SetPercentage(int(dur), int(durt))
			self.lifeTimeGauge.Show()
		else:
			self.lifeTimeGauge.Hide()

	def SetSkill(self, slot, idx, lv):
		if int(idx) != -1:
			self.skillSlot[int(slot)].ClearSlot(0)
			self.skillSlot[int(slot)].SetPetSkillSlot(0, int(idx), int(lv), 0.5, 0.5)
			self.skillSlot[int(slot)].SetCoverButton(0, "d:/ymir work/ui/pet/mini_window/pet_slot_corvermini.sub", "d:/ymir work/ui/pet/mini_window/pet_slot_corvermini.sub", "d:/ymir work/ui/pet/mini_window/pet_slot_corvermini.sub" , "d:/ymir work/ui/pet/mini_window/pet_slot_corvermini.sub")
			self.skillSlot[int(slot)].SetAlwaysRenderCoverButton(0, True)
		else:
			self.skillSlot[int(slot)].ClearSlot(0)
			self.skillSlot[int(slot)].SetSlotScale(0, 2, 16, 16, petskill.GetEmptySkill(), 0.5, 0.5)
			self.skillSlot[int(slot)].SetAlwaysRenderCoverButton(0, True)


	def SetExperience(self, expm, expi, exptot):
		expm = int(expm)
		expi = int(expi)
		exptot = int(exptot)
		
		if exptot > 0:	
			totalexp = exptot
			totexpm = int(float(totalexp) / 100 * 90)
			totexpi = totalexp - totexpm
			expi = min(expi, totexpi)
			expmp =  float(expm) / totexpm * 100
			expip =  float(expi) / totexpi * 100
		else:
			totalexp = 0
			totexpm = 0
			totexpi = 0
			expmp =  0
			expip =  0

		curPoint = int(min(expm, totexpm))
		curPoint = int(max(expm, 0))
		maxPoint = int(max(totexpm, 0))
		
		curPointi = int(min(expi, totexpi))
		curPointi = int(max(expi, 0))
		maxPointi = int(max(totexpi, 0))

		quarterPoint = maxPoint / 4
		quarterPointi = maxPointi 
		FullCount = 0
		FullCounti = 0

		if 0 != quarterPoint:
			FullCount = min(4, curPoint / quarterPoint)
			
		if 0 != quarterPointi:
			FullCounti = min(1, curPointi / quarterPointi)

		for i in xrange(4):
			self.petmini_exp[i].Hide()
			
		self.petmini_exp[4].Hide()

		for i in xrange(FullCount):
			self.petmini_exp[i].SetRenderingRect(0.0, 0.0, 0.0, 0.0)
			self.petmini_exp[i].Show()
			
		for i in xrange(FullCounti):
			self.petmini_exp[4].SetRenderingRect(0.0, 0.0, 0.0, 0.0)
			self.petmini_exp[4].Show()

		if 0 != quarterPoint:
			if FullCount < 4:
				Percentage = float(curPoint % quarterPoint) / quarterPoint - 1.0
				self.petmini_exp[FullCount].SetRenderingRect(0.0, Percentage, 0.0, 0.0)
				self.petmini_exp[FullCount].Show()
				
		if 0 != quarterPointi:
			if FullCounti < 1:
				Percentage = float(curPointi % quarterPointi) / quarterPointi - 1.0
				self.petmini_exp[4].SetRenderingRect(0.0, Percentage, 0.0, 0.0)
				self.petmini_exp[4].Show()

		if localeInfo.IsARABIC():
			tooltip_text = '%.2f%%' % expmp				+ ' :'	+ str(localeInfo.PET_INFO_EXP)			+ '\\n'	\
						 + str(expm)					+ ' :'	+ str(localeInfo.PET_INFO_NEXT_EXP)		+ '\\n'	\
						 + '%.2f%%' % expip				+ ' :'	+ str(localeInfo.PET_INFO_ITEM_EXP)		+ '\\n'	\
						 + str(expi)					+ ' :'	+ str(localeInfo.PET_INFO_NEXT_ITEM_EXP)	
			self.tooltipEXP.SetText(tooltip_text)
		else:
			tooltip_text = str(localeInfo.PET_INFO_EXP) 			+ ': ' + '%.2f%%' % expmp + '\\n'	\
						 + str(localeInfo.PET_INFO_NEXT_EXP) 		+ ': ' + str(expm) + '\\n'	\
						 + str(localeInfo.PET_INFO_ITEM_EXP) 		+ ': ' + '%.2f%%' % expip + '\\n'	\
						 + str(localeInfo.PET_INFO_NEXT_ITEM_EXP)	+ ': ' + str(expi)
						 
			self.tooltipEXP.SetText(tooltip_text)

	def OnUpdate(self):
		if self.expwind.IsIn():
			self.tooltipEXP.Show()
		else:
			self.tooltipEXP.Hide()

class TextToolTip(ui.Window):
	def __init__(self):
		ui.Window.__init__(self, "TOP_MOST")
		self.TextList = []

	def __del__(self):
		ui.Window.__del__(self)
		del self.TextList		

	def SetText(self, text):
		count = text.count("\\n")
		length = len(self.TextList)
		del self.TextList
		self.TextList = []
		for i in xrange(count + 1):
			self.i = ui.TextLine()
			self.i.SetParent(self)
			self.i.SetHorizontalAlignCenter()
			self.i.SetOutline()
			self.i.SetText(text.split("\\n")[i])
			self.i.Show()
			self.TextList.append(self.i)

	def OnRender(self):
		(mouseX, mouseY) = wndMgr.GetMousePosition()
		
		if localeInfo.IsARABIC():
			mouseX = mouseX + 100
			mouseY = mouseY - 70
		else:
			mouseY = mouseY - 50

		mouseY = mouseY - len(self.TextList) * 5
		for textline in self.TextList:
			textline.SetPosition(mouseX, mouseY)
			mouseY = mouseY + 15

