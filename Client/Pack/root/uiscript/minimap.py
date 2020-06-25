import uiScriptLocale
import localeInfo

ROOT = "d:/ymir work/ui/minimap/"

WINDOW_WIDTH = 186
WINDOW_HEIGHT = 200

window = {
	"name" : "MiniMap",

	"x" : SCREEN_WIDTH - 150,
	"y" : 0,

	"width" : WINDOW_WIDTH,
	"height" : WINDOW_HEIGHT,

	"children" :
	[
		## OpenWindow
		{
			"name" : "OpenWindow",
			"type" : "window",

			"x" : 0,
			"y" : 0,

			"width" : WINDOW_WIDTH,
			"height" : WINDOW_HEIGHT,

			"children" :
			[
				{
					"name" : "OpenWindowBGI",
					"type" : "image",
					"x" : 14,
					"y" : 0,
					"image" : ROOT + "minimap.sub",
				},
				## MiniMapWindow
				{
					"name" : "MiniMapWindow",
					"type" : "window",

					"x" : 18,
					"y" : 5,

					"width" : 128,
					"height" : 128,
				},
				## ScaleUpButton
				{
					"name" : "ScaleUpButton",
					"type" : "button",

					"x" : 115,
					"y" : 116,

					"default_image" : ROOT + "minimap_scaleup_default.sub",
					"over_image" : ROOT + "minimap_scaleup_over.sub",
					"down_image" : ROOT + "minimap_scaleup_down.sub",
				},
				## ScaleDownButton
				{
					"name" : "ScaleDownButton",
					"type" : "button",

					"x" : 129,
					"y" : 103,

					"default_image" : ROOT + "minimap_scaledown_default.sub",
					"over_image" : ROOT + "minimap_scaledown_over.sub",
					"down_image" : ROOT + "minimap_scaledown_down.sub",
				},
				## MiniMapHideButton
				{
					"name" : "MiniMapHideButton",
					"type" : "button",

					"x" : 125,
					"y" : 6,

					"default_image" : ROOT + "minimap_close_default.sub",
					"over_image" : ROOT + "minimap_close_over.sub",
					"down_image" : ROOT + "minimap_close_down.sub",
				},
				## AtlasShowButton
				{
					"name" : "AtlasShowButton",
					"type" : "button",

					"x" : 21,
					"y" : 7,

					"default_image" : ROOT + "atlas_open_default.sub",
					"over_image" : ROOT + "atlas_open_over.sub",
					"down_image" : ROOT + "atlas_open_down.sub",
				},
				{
					"name" : "OpenWindowBGI2",
					"type" : "image",
					"x" : 14,
					"y" : 0,
					"image" : ROOT + "minimap.tga",
				},
				## ScaleUpButton2
				{
					"name" : "ScaleUpButton2",
					"type" : "button",

					"x" : 106 + 52, #115
					"y" : 60,

					"default_image" : ROOT + "plus1.tga",
					"over_image" : ROOT + "plus1.tga",
					"down_image" : ROOT + "plus2.tga",
				},
				## ScaleDownButton2
				{
					"name" : "ScaleDownButton2",
					"type" : "button",

					"x" : 100 + 52, #129
					"y" : 105,
					
					"default_image" : ROOT + "moin1.tga",
					"over_image" : ROOT + "moin1.tga",
					"down_image" : ROOT + "moin2.tga",
				},
				## MiniMapHideButton2
				{
					"name" : "MiniMapHideButton2",
					"type" : "button",

					"x" : 94 + 52, #125
					"y" : 34,

					"default_image" : ROOT + "fermer1.tga",
					"over_image" : ROOT + "fermer1.tga",
					"down_image" : ROOT + "fermer2.tga",
				},
				## AtlasShowButton2
				{
					"name" : "AtlasShowButton2",
					"type" : "button",

					"x" : 86 + 52, #21
					"y" : 130,

					"default_image" : ROOT + "m1.tga",
					"over_image" : ROOT + "m1.tga",
					"down_image" : ROOT + "m2.tga",
				},
				## ServerInfo
				{
					"name" : "ServerInfo",
					"type" : "text",
					
					"text_horizontal_align" : "center",

					"outline" : 1,

					"x" : 84,
					"y" : 140,

					"text" : "",
				},
				## PositionInfo
				{
					"name" : "PositionInfo",
					"type" : "text",
					
					"text_horizontal_align" : "center",

					"outline" : 1,

					"x" : 84,
					"y" : 160,

					"text" : "",
				},
				## ObserverCount
				{
					"name" : "ObserverCount",
					"type" : "text",
					
					"text_horizontal_align" : "center",

					"outline" : 1,

					"x" : 84,
					"y" : 180,

					"text" : "",
				},
			],
		},
		{
			"name" : "CloseWindow",
			"type" : "window",

			"x" : 0,
			"y" : 0,

			"width" : 132,
			"height" : 48,

			"children" :
			[
				## ShowButton
				{
					"name" : "MiniMapShowButton",
					"type" : "button",

					"x" : 100+14,
					"y" : 4,

					"default_image" : ROOT + "minimap_open_default.sub",
					"over_image" : ROOT + "minimap_open_default.sub",
					"down_image" : ROOT + "minimap_open_default.sub",
				},
			],
		},
	],
}

if ENABLE_OFFLINE_PRIVATE_SHOP:
	window["children"][0]["children"] += [
		{
			"name" : "ShopSearchButton",
			"type" : "button",
			
			"x" : 16,
			"y" : 104,
			
			"tooltip_text" : "Çevrim Dýþý Pazar [J]",
			# "tooltip_text" : uiScriptLocale.SHOP_SEARCH_TITLE + " [J]",
			"tooltip_x" : -56,
			"tooltip_y" : 10,
			
			"default_image" : ROOT + "minimap_shop_normal.dds",
			"over_image" : ROOT + "minimap_shop_hover.dds",
			"down_image" : ROOT + "minimap_shop_pressed.dds",
		},]


if localeInfo.IsARABIC():
	window["children"][0]["children"][5]["x"] = 0
	window["children"][0]["children"][5]["y"] = 57
	window["children"][0]["children"][0]["image"] = "locale/ae/ui/minimap/minimap.sub"

if ENABLE_SOME_INFOS:
	window["children"][0]["children"] += [
				## FPS
				{
					"name" : "FPS",
					"type" : "text",
					
					"text_horizontal_align" : "center",

					"outline" : 1,

					"fontsize" : "SMALL",

					"x" : -70 + 50,
					"y" : 8,

					"text" : "FPS: ",
				},
				## Ping
				# {
					# "name" : "Ping",
					# "type" : "text",
					
					# "text_horizontal_align" : "center",

					# "outline" : 1,

					# "fontsize" : "SMALL",

					# "x" : -20,
					# "y" : 8,

					# "text" : "Ping: ",
				# },
				## Day
				{
					"name" : "Day",
					"type" : "text",
					
					"text_horizontal_align" : "center",

					"outline" : 1,

					"fontsize" : "LARGE",

					"x" : 84,
					"y" : 175,

					"text" : "",
				},
				## Clock
				{
					"name" : "Clock",
					"type" : "text",
					
					"text_horizontal_align" : "center",

					"outline" : 1,

					"fontsize" : "LARGE",

					"x" : 84,
					"y" : 187,

					"text" : "",
				},]

if ENABLE_BOSS_TRACKING:
	window["children"][0]["children"] += [		
		{
			"name" : "BossTrackingButton",
			"type" : "button",

			"x" : 39,
			"y" : 119,
			
			"tooltip_text" : localeInfo.BOSS_TRACKIN_SYSTEM_BUTTON_TOOLTIP,
			
			"default_image" : "d:/ymir work/ui/boss_follow/boss_open_default.tga",
			"over_image" : "d:/ymir work/ui/boss_follow/boss_open_over.tga",
			"down_image" : "d:/ymir work/ui/boss_follow/boss_open_down.tga",
		},		
	]

if ENABLE_DEFENSE_WAVE:
	window["children"][0]["children"] += [
				{
					"name" : "MastWindow",
					"type" : "thinboard",

					"x" : 35,
					"y" : 160+60,

					"width" : 105,
					"height" : 37,
					"children" :
					[
						{
							"name" : "MastText",
							"type" : "text",

							"text_horizontal_align" : "center",

							"x" : 35,
							"y" : 8,

							"text" : uiScriptLocale.DEFANCE_WAWE_MAST_TEXT,
						},
						{
							"name" : "MastHp",
							"type" : "gauge",

							"x" : 10,
							"y" : 23,

							"width" : 85,
							"color" : "red",
							
							"tooltip_text" : uiScriptLocale.DEFANCE_WAWE_GAUGE_TOOLTIP,
						},
					],
				},]

if ENABLE_MINI_GAME:
	window["children"][0]["children"] += [
		{
			"name" : "minigame_button",
			"type" : "button",

			"x" : 6,
			"y" : 28,

			"default_image" : "d:/ymir work/ui/minimap/E_open_default.tga",
			"over_image" : "d:/ymir work/ui/minimap/E_open_over.tga",
			"down_image" : "d:/ymir work/ui/minimap/E_open_down.tga",
		},]

if ENABLE_12ZI and not localeInfo.IsARABIC():
	window["children"][0]["children"][5]["x"] = 0
	window["children"][0]["children"][5]["y"] = 57

if ENABLE_DUNGEON_INFO_SYSTEM:
	window["children"][0]["children"] += [
	## Wiki Btn
	# {
		# "name" : "WikiBtn",
		# "type" : "button",
		
		# "x" : 39,
		# "y" : 119,
		
		# "tooltip_text" : "Wiki",
		# "tooltip_x" : -46,
		# "tooltip_y" : 10,
		
		# "default_image" : ROOT + "wikibtn_norm.dds",
		# "over_image" : ROOT + "wikibtn_hover.dds",
		# "down_image" : ROOT + "wikibtn_down.dds",
	# },
	## Dungeon Info Button
	{
		"name" : "DungeonInfoButton",
		"type" : "button",
		
		"x" : 1 + 4,
		"y" : 59 + 22,
		
		"tooltip_text" : "Zindan Bilgi [F7]",
		"tooltip_x" : -51,
		"tooltip_y" : 10,
		
		"default_image" : ROOT + "dragoneye.dds",
		"over_image" : ROOT + "dragoneye.dds",
		"down_image" : ROOT + "dragoneye.dds",
	},]

