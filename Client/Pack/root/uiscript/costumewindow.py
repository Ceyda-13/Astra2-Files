import uiScriptLocale
import item
import app

COSTUME_START_INDEX = item.COSTUME_SLOT_START

window = {
	"name" : "CostumeWindow",
	"x" : SCREEN_WIDTH - 175 - 140,
	"y" : SCREEN_HEIGHT - 37 - 565,
	"style" : ("movable", "float",),
	"width" : 140,
	"height" : 180,

	"children" :
	[
		{
			"name" : "board",
			"type" : "board",
			"style" : ("attach",),
			"x" : 0,
			"y" : 0,
			"width" : 140,
			"height" : 180,
			
			"children" :
			[
				{
					"name" : "TitleBar",
					"type" : "titlebar",
					"style" : ("attach",),
					"x" : 6,
					"y" : 6,
					"width" : 130,
					"color" : "yellow",
					"children" :
					[
						{ "name":"TitleName", "type":"text", "x":60, "y":3, "text":uiScriptLocale.COSTUME_WINDOW_TITLE, "text_horizontal_align":"center" },
					],
				},

				{
					"name" : "Costume_Base",
					"type" : "image",
					"x" : 13,
					"y" : 38,
					
					"image" : uiScriptLocale.LOCALE_UISCRIPT_PATH + "costume/costume_bg.jpg",					

					"children" :
					[
						{
							"name" : "CostumeSlot",
							"type" : "slot",
							"x" : 3,
							"y" : 3,
							"width" : 127,
							"height" : 145,
							"slot" : [
										{"index":item.COSTUME_SLOT_BODY, "x":62, "y":45, "width":32, "height":64},#몸
										{"index":item.COSTUME_SLOT_HAIR, "x":62, "y": 9, "width":32, "height":32},#머리
										{"index":item.EQUIPMENT_RING1, "x":13, "y":178, "width":32, "height":32},
										{"index":item.EQUIPMENT_RING2, "x":62, "y":178, "width":32, "height":32},
									],
						},
					],
				},
			],
		},
	],
}

if ENABLE_COSTUME_MOUNT:
	window["children"][0]["children"][1]["children"][0]["slot"] += [
										{"index":item.COSTUME_SLOT_MOUNT, "x":13, "y":126, "width":32, "height":32},]

if ENABLE_ACCE_SYSTEM: # @@@correction024
	window["height"] += 50
	window["children"][0]["height"] += 50
	window["children"][0]["children"][1]["image"] = uiScriptLocale.LOCALE_UISCRIPT_PATH + "costume/new_costume_bg.jpg"
	window["children"][0]["children"][1]["children"][0]["slot"] += [
										{"index":item.COSTUME_SLOT_ACCE, "x":62, "y":126, "width":32, "height":32},]

if ENABLE_WEAPON_COSTUME_SYSTEM: # @@@correction038
	window["children"][0]["children"][1]["children"][0]["slot"] += [
										{"index":item.COSTUME_SLOT_WEAPON, "x":13, "y":13, "width":32, "height":96},]

# if ENABLE_COSTUME_EFFECT: # @@@correction039
	window["height"] += 47
	window["children"][0]["height"] += 47
	window["children"][0]["children"][1]["children"][0]["height"] += 130
	window["children"][0]["children"][1]["image"] = uiScriptLocale.LOCALE_UISCRIPT_PATH + "costume/new_costume_bg_with_ring.jpg"
	# window["children"][0]["children"][1]["children"][0]["slot"] += [
										# {"index":item.COSTUME_SLOT_EFFECT_WEAPON, "x":13, "y":178, "width":32, "height":32},
										# {"index":item.COSTUME_SLOT_EFFECT_BODY, "x":62, "y":178, "width":32, "height":32},]
