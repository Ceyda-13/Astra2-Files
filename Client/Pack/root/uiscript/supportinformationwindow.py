import uiScriptLocale
import localeInfo

ROOT_PATH = "d:/ymir work/ui/game/windows/"
ROOT = "d:/ymir work/ui/game/"
SUPPORT_DIRECTORY = "d:/ymir work/ui/support/"
SMALL_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_00.sub"
MIDDLE_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_01.sub"
LARGE_VALUE_FILE = "d:/ymir work/ui/public/Parameter_Slot_03.sub"
XLARGE_BUTTON_FILE = "d:/ymir work/ui/public/xlarge_button_03.sub"
BASE_SLOT_FILE = "d:/ymir work/ui/public/Slot_Base.sub"

SUPPORT_BG_WIDTH		= 352
SUPPORT_BG_HEIGHT	= 270

LONG_LABEL_WIDTH	= 266
LONG_LABEL_HEIGHT	= 19

SHORT_LABLE_WIDTH	= 90
SHORT_LABLE_HEIGHT	= 20

MIDDLE_LABLE_WIDTH	= 168
MIDDLE_LABLE_HEIGHT	= 20

EXP_GAGUE_INTERVAL	= 2
EXP_IMG_WIDTH		= 16
EXP_IMG_HEIGHT		= 16

GOLD_COLOR	= 0xFFFEE3AE
WHITE_COLOR = 0xFFFFFFFF
ORANGE_COLOR = 0xFFF2A505
	
window = {
	"name" : "SupportInformationWindow",
	"style" : ("movable", "float",),
	
	"x" : SCREEN_WIDTH - 176 -200 -146 -145,
	"y" : SCREEN_HEIGHT - 37 - 565,

	"width" : SUPPORT_BG_WIDTH,
	"height" : SUPPORT_BG_HEIGHT,

	"children" :
	(
		{
			"name" : "board",
			"type" : "window",

			"x" : 0,
			"y" : 0,

			"width" : SUPPORT_BG_WIDTH,
			"height" : SUPPORT_BG_HEIGHT,
			
			"children" :
			(
				## Support UI BG
				{ "name" : "PetUIBG", "type" : "expanded_image", "style" : ("attach",), "x" : 0, "y" : 0, "image" : "d:/ymir work/ui/support/support_bg.tga" },
				{
					"name" : "SlotSupportImage",
					"type" : "slot",
					"x" : 25,
					"y" : 226,
					"width" : 32,
					"height" : 32,
					
					"slot" : ({"index":0, "x":0, "y":0, "width":32, "height":32},),
					
				},
				{
					"name" : "SlotSupportArmor",
					"type" : "slot",
					"x" : 121,
					"y" : 92,
					"width" : 0,
					"height" : 0,
			
					"slot" :({"index":1, "x":0, "y":0, "width":0, "height":0},),
				},
				{
					"name" : "SlotSupportWeapon",
					"type" : "slot",
					"x" : 80,
					"y" : 70,
					"width" : 0,
					"height" : 0,
			
					"slot" :({"index":3, "x":0, "y":0, "width":0, "height":0},),
				},
				{
					"name" : "SlotSupportHair",
					"type" : "slot",
					"x" : 122,
					"y" : 64,
					"width" : 0,
					"height" : 0,
			
					"slot" :({"index":2, "x":0, "y":0, "width":0, "height":0},),
				},
				##Slot Support
				{
					"name" : "CombSlot",
					"type" : "slot",
			
					"x" : 0,
					"y" : 0,
			
					"width" : 300,
					"height" : 300,
					
					"slot" :
					(
						{"index":1, "x":121, "y":92, "width":31, "height":64},
						{"index":2, "x":122, "y":64, "width":32, "height":32},
						{"index":3, "x":80, "y":70, "width":31, "height":96},
					),
					"children" :
					(
					),
				},
				## Support Information Title
				{ 
					"name" : "TitleWindow", "type" : "window", "x" : 10, "y" : 10, "width" : SUPPORT_BG_WIDTH-10-15, "height" : 15, "style" : ("attach",),
					"children" :
					(
						{"name":"TitleName", "type":"text", "x":0, "y":0, "text":uiScriptLocale.SUPPORTWND_TITLE, "all_align" : "center"},
					),	
				},
				
				## Close Button
				{ 
					"name" : "CloseButton", 
					"type" : "button", 
					"x" : SUPPORT_BG_WIDTH -10-15, 
					"y" : 10, 
					"tooltip_text" : "Pencereyi Kapat", 
					"default_image" : "d:/ymir work/ui/public/close_button_01.sub",	
					"over_image" : "d:/ymir work/ui/public/close_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/close_button_03.sub",
				},
				{ 
					"name" : "SupportsNameWindow", "type" : "window", "x" : 227, "y" : 30, "width" : SHORT_LABLE_WIDTH, "height" : SHORT_LABLE_HEIGHT, "style" : ("attach",),
					"children" :
					(
						{"name":"SupportsNameWindows", "type":"text", "x":0, "y":0, "text": uiScriptLocale.BATTLE_FILED_RANK_NAME, "color":GOLD_COLOR, "all_align" : "center"},
					),	
				},
				## Support Name
				{ 
					"name" : "NameWindow", "type" : "window", "x" : 136, "y" : 26+SHORT_LABLE_HEIGHT+3, "width" : LONG_LABEL_WIDTH, "height" : LONG_LABEL_HEIGHT, "style" : ("attach",),
					"children" :
					(
						{"name":"SupportsName", "type":"text", "x":0, "y":0, "text": "", "r":1.0, "g":1.0, "b":1.0, "a":1.0, "all_align" : "center"},
					),	
				},
				
				## Level Title
				{ 
					"name" : "LevelWindow", "type" : "window", "x" : 228, "y" : 72, "width" : SHORT_LABLE_WIDTH, "height" : SHORT_LABLE_HEIGHT, "style" : ("attach",),
					"children" :
					(
						{"name":"LevelTitle", "type":"text", "x":0, "y":0, "text": uiScriptLocale.GUILD_MEMBER_LEVEL, "color":GOLD_COLOR, "all_align" : "center"},
					),	
				},
				## Level Value
				{ 
					"name" : "LevelValueWindow", "type" : "window", "x" : 228, "y" : 69+SHORT_LABLE_HEIGHT+3, "width" : SHORT_LABLE_WIDTH, "height" : SHORT_LABLE_HEIGHT, "style" : ("attach",),
					"children" :
					(
						{"name":"LevelValue", "type":"text", "x":0, "y":0, "text": "", "color":WHITE_COLOR, "all_align" : "center"},
					),	
				},
				## Specialita Title
				{ 
					"name" : "SpecialityWindow", "type" : "window", "x" : 228, "y" : 115, "width" : SHORT_LABLE_WIDTH, "height" : SHORT_LABLE_HEIGHT, "style" : ("attach",),
					"children" :
					(
						{"name":"SpecialityTitle", "type":"text", "x":0, "y":0, "text": localeInfo.TASKBAR_SKILL, "color":GOLD_COLOR, "all_align" : "center"},
					),	
				},
				## Speciality Value
				{ 
					"name" : "SpecialityValueWindow", "type" : "window", "x" : 228, "y" : 111+SHORT_LABLE_HEIGHT+3, "width" : SHORT_LABLE_WIDTH, "height" : SHORT_LABLE_HEIGHT, "style" : ("attach",),
					"children" :
					(
						{"name":"SpecialityValue", "type":"text", "x":0, "y":0, "text": "", "color":WHITE_COLOR, "all_align" : "center"},
					),	
				},			
				## EXP Title
				{ 
					"name" : "ExpWindow", "type" : "window", "x" : 228, "y" : 158, "width" : SHORT_LABLE_WIDTH, "height" : SHORT_LABLE_HEIGHT, "style" : ("attach",),
					"children" :
					(
						{"name":"ExpTitle", "type":"text", "x":0, "y":0, "text": uiScriptLocale.HELP_EXP, "color":GOLD_COLOR, "all_align" : "center"},
					),	
				},
				## EXP Gauge
				{
					"name" : "UpBringing_Support_EXP_Gauge_Board",
					"type" : "window",
					"style": ("ltr",),

					"x" : 232,
					"y" : 180,
				
					"width"		: EXP_IMG_WIDTH * 5 + EXP_GAGUE_INTERVAL * 4,
					"height"	: EXP_IMG_HEIGHT,

					"children" :
					(
						{
							"name" : "UpBringing_Support_EXPGauge_01",
							"type" : "expanded_image",

							"x" : 0,
							"y" : 0,

							"image" : SUPPORT_DIRECTORY + "exp_gauge/exp_on.sub",
						},
						{
							"name" : "UpBringing_Support_EXPGauge_02",
							"type" : "expanded_image",

							"x" : EXP_IMG_WIDTH + EXP_GAGUE_INTERVAL + 1,
							"y" : 0,

							"image" : SUPPORT_DIRECTORY + "exp_gauge/exp_on.sub",
						},
						{
							"name" : "UpBringing_Support_EXPGauge_03",
							"type" : "expanded_image",

							"x" : EXP_IMG_WIDTH * 2 + EXP_GAGUE_INTERVAL * 2 + 3,
							"y" : 0,

							"image" : SUPPORT_DIRECTORY + "exp_gauge/exp_on.sub",
						},
						{
							"name" : "UpBringing_Support_EXPGauge_04",
							"type" : "expanded_image",

							"x" : EXP_IMG_WIDTH * 3 + EXP_GAGUE_INTERVAL * 3 + 4,
							"y" : 0,

							"image" : SUPPORT_DIRECTORY + "exp_gauge/exp_on.sub",
						},
					),
				}, 
				#End of EXP
				{ 
					"name" : "AbilitiesWindow", "type" : "window", "x" : 43, "y" : 202, "width" : LONG_LABEL_WIDTH, "height" : LONG_LABEL_HEIGHT, "style" : ("attach",),
					"children" :
					(
						{"name":"AbilitiesName", "type":"text", "x":0, "y":0, "text": localeInfo.DETAILS_CATE_6, "color":ORANGE_COLOR, "all_align" : "center"},
					),	
				},
				##Equip
				{ 
					"name" : "EquipWindow", "type" : "window", "x" : -10, "y" : 40, "width" : LONG_LABEL_WIDTH, "height" : LONG_LABEL_HEIGHT, "style" : ("attach",),
					"children" :
					(
						{"name":"EquipName", "type":"text", "x":0, "y":0, "text": localeInfo.PRIVATESHOPSEARCH_ARMOR, "color":GOLD_COLOR, "all_align" : "center"},
					),	
				},		
				## Int Title
				{ 
					"name" : "IntWindow", "type" : "window", "x" : 25, "y" : 232, "width" : MIDDLE_LABLE_WIDTH, "height" : MIDDLE_LABLE_HEIGHT, "style" : ("attach",),
					"children" :
					(
						{"name":"IntTitle", "type":"text", "x":0, "y":0, "text": uiScriptLocale.SUPPORTWND_BECINT, "color":GOLD_COLOR, "all_align" : "center"},
					),	
				},
				## Int Value
				{ 
					"name" : "IntValueWindow", "type" : "window", "x" : -10 + MIDDLE_LABLE_WIDTH, "y" : 232, "width" : MIDDLE_LABLE_WIDTH, "height" : MIDDLE_LABLE_HEIGHT, "style" : ("attach",),
					"children" :
					(
						{"name":"SupportIntValue", "type":"text", "x":0, "y":0, "text": "", "color":WHITE_COLOR, "all_align" : "center"},
					),	
				},
			),			
		},
	),
}
