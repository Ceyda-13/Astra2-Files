import uiScriptLocale

WINDOW_WIDTH = 184
WINDOW_HEIGHT = 328 + 26
if ENABLE_OFFLINE_PRIVATE_SHOP:
	WINDOW_WIDTH += 161
	WINDOW_HEIGHT += 58

window = {
	"name" : "PrivateShopBuilder",

	"x" : 0,
	"y" : 0,

	"style" : ["movable", "float",],

	"width" : WINDOW_WIDTH,
	"height" : WINDOW_HEIGHT,

	"children" :
	[
		{
			"name" : "board",
			"type" : "board",
			"style" : ["attach",],

			"x" : 0,
			"y" : 0,

			"width" : WINDOW_WIDTH,
			"height" : WINDOW_HEIGHT,

			"children" :
			[
				## Title
				{
					"name" : "TitleBar",
					"type" : "titlebar",
					"style" : ["attach",],

					"x" : 8,
					"y" : 8,

					"width" : WINDOW_WIDTH - 15,
					"color" : "gray",

					"children" :
					[
						{ "name":"TitleName", "type":"text", "x":WINDOW_WIDTH / 2, "y":4, "text":uiScriptLocale.PRIVATE_SHOP_TITLE, "text_horizontal_align":"center" },
					],
				},

				{
					"name" : "NameSlot",
					"type" : "slotbar",
					"x" : 13,
					"y" : 35,
					"width" : 90 + 67,
					"height" : 18,

					"children" :
					[
						{
							"name" : "NameLine",
							"type" : "text",
							"x" : 3,
							"y" : 3,
							"width" : 157,
							"height" : 15,
							"input_limit" : 25,
							"text" : "1234567890123456789012345",
						},
					],
				},

				## Item Slot
				{
					"name" : "ItemSlot",
					"type" : "grid_table",

					"x" : 12,
					"y" : 34 + 26,

					"start_index" : 0,
					"x_count" : 5,
					"y_count" : 8,
					"x_step" : 32,
					"y_step" : 32,

					"image" : "d:/ymir work/ui/public/Slot_Base.sub",
				},

				## Ok
				{
					"name" : "OkButton",
					"type" : "button",

					"x" : 21,
					"y" : 295 + 26,

					"width" : 61,
					"height" : 21,

					"text" : uiScriptLocale.OK,

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},

				## Close
				{
					"name" : "CloseButton",
					"type" : "button",

					"x" : 104,
					"y" : 295 + 26,

					"width" : 61,
					"height" : 21,

					"text" : uiScriptLocale.CLOSE,

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},
			],
		},
	],
}


if ENABLE_OFFLINE_PRIVATE_SHOP:
	window["children"][0]["children"][1]["width"] += 160
	window["children"][0]["children"][1]["children"][0]["width"] += 155
	window["children"][0]["children"][2]["y"] += 26
	window["children"][0]["children"][2]["x_count"] = 10
	window["children"][0]["children"][2]["y_count"] = 9
	window["children"][0]["children"][3]["x"] += 43
	window["children"][0]["children"][3]["y"] -= 288
	window["children"][0]["children"][3]["vertical_align"] = "bottom"
	window["children"][0]["children"][4]["x"] += 121
	window["children"][0]["children"][4]["y"] -= 288
	window["children"][0]["children"][4]["vertical_align"] = "bottom"
	window["style"].pop(1)
	window["children"][0]["children"][1]["children"][0]["type"] = "editline"
	window["children"][0]["children"][1]["children"][0]["text"] = ""

	window["height"] += 35
	window["children"][0]["height"] += 35
	window["children"][0]["children"] += [
		{
			"name" : "Page1Button",
			"type" : "radio_button",

			"x" : 82 - 25,
			"y" : 295 + 35 + 26 + 26,

			"default_image" : "d:/ymir work/ui/game/shopsearchp2p/pageone.dds",
			"over_image" : "d:/ymir work/ui/game/shopsearchp2p/pageone_hover.dds",
			"down_image" : "d:/ymir work/ui/game/shopsearchp2p/pageone_active.dds",
		},
		{
			"name" : "Page2Button",
			"type" : "radio_button",

			"x" : 242 + 25,
			"y" : 295 + 35 + 26 + 26,

			"default_image" : "d:/ymir work/ui/game/shopsearchp2p/pagetwo.dds",
			"over_image" : "d:/ymir work/ui/game/shopsearchp2p/pagetwo_hover.dds",
			"down_image" : "d:/ymir work/ui/game/shopsearchp2p/pagetwo_active.dds",
		},
		{
			"name" : "Position",
			"type" : "text",
			"x" : window["width"] / 2,
			"y" : 34 + 29,
			"text" : "",
			"text_horizontal_align" : "center",
		},
		{
			"name" : "time_window",
			"type" : "window",
			
			"x" : 0,
			"y" : 295 + 35 + 35,
			"width" : 184,
			"height" : 22 + 20,
			"horizontal_align" : "center",
			"children" :
			(
				{
					"name" : "time_select_img",
					"type" : "expanded_image",
					"x" : 13,
					"y" : 20,
					"x_scale" : 0.759,
					"y_scale" : 1.0,
					"image" : "d:/ymir work/ui/quest_re/button_one.sub",
				},
				{
					"name" : "cur_time_text_window",
					"type" : "window",
					
					"x" : 13,
					"y" : 20,
					"width" : 175-16,
					"height" : 16,
					
					"children" :
					(
						{"name":"cur_time_text", "type":"text", "x":0, "y":0, "text": "-", "all_align" : "center"},
					),
				},
				{
					"name" : "time_select_button",
					"type" : "button",

					"x" : 13 + 143,
					"y" : 20,

					"default_image" : "d:/ymir work/ui/game/party_match/arrow_default.sub",
					"over_image" : "d:/ymir work/ui/game/party_match/arrow_over.sub",
					"down_image" : "d:/ymir work/ui/game/party_match/arrow_down.sub",
				},
				{
					"name" : "time_select_pivot_window",
					"type" : "window",

					"x" : 13,
					"y" : 16+20,
					"width" : 175,
					"height" : 0,
				},
			),
		},]

