import uiScriptLocale

WINDOW_WIDTH = 345
WINDOW_HEIGHT = 360

window = {
	"name" : "ShopDialog",

	"x" : SCREEN_WIDTH - WINDOW_WIDTH - 216,
	"y" : 10,

	"style" : ("movable", "float",),

	"width" : WINDOW_WIDTH,
	"height" : WINDOW_HEIGHT,

	"children" :
	[
		{
			"name" : "board",
			"type" : "board",
			"style" : ("attach",),

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
					"style" : ("attach",),

					"x" : 8,
					"y" : 8,

					"width" : WINDOW_WIDTH - 15,
					"color" : "gray",

					"children" :
					[
						{ "name":"TitleName", "type":"text", "x":84, "y":4, "text":uiScriptLocale.SHOP_TITLE, "text_horizontal_align":"center" },
					],
				},

				## Item Slot
				{
					"name" : "ItemSlot",
					"type" : "grid_table",

					"x" : 12,
					"y" : 34,

					"start_index" : 0,
					"x_count" : 5,
					"y_count" : 8,
					"x_step" : 32,
					"y_step" : 32,

					"image" : "d:/ymir work/ui/public/Slot_Base.sub",
				},

				## Buy
				{
					"name" : "BuyButton",
					"type" : "toggle_button",

					"x" : 21,
					"y" : 295,

					"width" : 61,
					"height" : 21,

					"text" : uiScriptLocale.SHOP_BUY,

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},

				## Sell
				{
					"name" : "SellButton",
					"type" : "toggle_button",

					"x" : 104,
					"y" : 295,

					"width" : 61,
					"height" : 21,

					"text" : uiScriptLocale.SHOP_SELL,

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},

				## Close
				{
					"name" : "CloseButton",
					"type" : "button",

					"x" : 0,
					"y" : 295,

					"horizontal_align" : "center",

					"text" : uiScriptLocale.PRIVATE_SHOP_CLOSE_BUTTON,

					"default_image" : "d:/ymir work/ui/public/large_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/large_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/large_button_03.sub",
				},

				## MiddleTab1
				{
					"name" : "MiddleTab1",
					"type" : "radio_button",

					"x" : 21,
					"y" : 295,

					"width" : 61,
					"height" : 21,

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},

				## MiddleTab2
				{
					"name" : "MiddleTab2",
					"type" : "radio_button",

					"x" : 104,
					"y" : 295,

					"width" : 61,
					"height" : 21,

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},

				## SmallTab1
				{
					"name" : "SmallTab1",
					"type" : "radio_button",

					"x" : 21,
					"y" : 295,

					"width" : 43,
					"height" : 21,

					"default_image" : "d:/ymir work/ui/public/small_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/small_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/small_button_03.sub",
				},

				## SmallTab2
				{
					"name" : "SmallTab2",
					"type" : "radio_button",

					"x" : 71,
					"y" : 295,

					"width" : 43,
					"height" : 21,

					"default_image" : "d:/ymir work/ui/public/small_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/small_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/small_button_03.sub",
				},

				## SmallTab3
				{
					"name" : "SmallTab3",
					"type" : "radio_button",

					"x" : 120,
					"y" : 295,

					"width" : 43,
					"height" : 21,

					"default_image" : "d:/ymir work/ui/public/small_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/small_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/small_button_03.sub",
				},
			],
		},
	],
}

if ENABLE_SHOP_TAB: # @@@correction030
	import shop
	window["children"][0]["children"][0]["children"][0]["x"] = 0
	window["children"][0]["children"][0]["children"][0]["horizontal_align"] = "center"
	window["children"][0]["children"][1]["x_count"] = shop.SHOP_PAGE_COLUMN
	window["children"][0]["children"][1]["y_count"] = shop.SHOP_PAGE_ROW
	for i in xrange(len(window["children"][0]["children"]) - 2):
		window["children"][0]["children"].pop(2)

	window["children"][0]["children"] += [
		{
			"name" : "Page1Button",
			"type" : "radio_button",

			"x" : 82,
			"y" : 33,
			
			"vertical_align" : "bottom",

			"default_image" : "d:/ymir work/ui/game/shopsearchp2p/pageone.dds",
			"over_image" : "d:/ymir work/ui/game/shopsearchp2p/pageone_hover.dds",
			"down_image" : "d:/ymir work/ui/game/shopsearchp2p/pageone_active.dds",
		},
		{
			"name" : "Page2Button",
			"type" : "radio_button",

			"x" : 242,
			"y" : 33,
			
			"vertical_align" : "bottom",

			"default_image" : "d:/ymir work/ui/game/shopsearchp2p/pagetwo.dds",
			"over_image" : "d:/ymir work/ui/game/shopsearchp2p/pagetwo_hover.dds",
			"down_image" : "d:/ymir work/ui/game/shopsearchp2p/pagetwo_active.dds",
		},]

