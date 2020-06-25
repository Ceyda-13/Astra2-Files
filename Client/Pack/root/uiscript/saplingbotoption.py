import uiScriptLocale

ROOT_PATH = "d:/ymir work/ui/public/"

TEMPORARY_X = +13
TEXT_TEMPORARY_X = -10
BUTTON_TEMPORARY_X = 5
PVP_X = -10

window = {
	"name" : "SystemOptionDialog",
	"style" : ("movable", "float",),

	"x" : 0,
	"y" : 0,

	"width" : 230,
	"height" : 120,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board",

			"x" : 0,
			"y" : 0,

			"width" : 230,
			"height" : 120,

			"children" :
			(
				## Title
				{
					"name" : "titlebar",
					"type" : "titlebar",
					"style" : ("attach",),

					"x" : 8,
					"y" : 8,

					"width" : 210,
					"color" : "gray",

					"children" :
					(
						{ 
						"name":"titlename", "type":"text", "x":0, "y":3, 
						"horizontal_align":"center", "text_horizontal_align":"center",
						"text" : "Çoklu Efsun Botu Sistemi",
						 },
					),
				},
				{
					"name" : "bgm_button",
					"type" : "button",

					"x" : 20,
					"y" : 45,

					"text" : "1 - Efsun Botu",
					"text_color" : 0xffF8BF24,

					"default_image" : ROOT_PATH + "Large_Button_01.sub",
					"over_image" : ROOT_PATH + "Large_Button_02.sub",
					"down_image" : ROOT_PATH + "Large_Button_03.sub",
				},
				{
					"name" : "bgm_button2",
					"type" : "button",

					"x" : 120,
					"y" : 45,

					"text" : "2 - Efsun Botu",
					"text_color" : 0xffF8BF24,

					"default_image" : ROOT_PATH + "Large_Button_01.sub",
					"over_image" : ROOT_PATH + "Large_Button_02.sub",
					"down_image" : ROOT_PATH + "Large_Button_03.sub",
				},
				{
					"name" : "bgm_button3",
					"type" : "button",

					"x" : 20,
					"y" : 75,

					"text" : "3 - Efsun Botu",
					"text_color" : 0xffF8BF24,

					"default_image" : ROOT_PATH + "Large_Button_01.sub",
					"over_image" : ROOT_PATH + "Large_Button_02.sub",
					"down_image" : ROOT_PATH + "Large_Button_03.sub",
				},
				{
					"name" : "bgm_button4",
					"type" : "button",

					"x" : 120,
					"y" : 75,

					"text" : "4 - Efsun Botu",
					"text_color" : 0xffF8BF24,

					"default_image" : ROOT_PATH + "Large_Button_01.sub",
					"over_image" : ROOT_PATH + "Large_Button_02.sub",
					"down_image" : ROOT_PATH + "Large_Button_03.sub",
				},
			),
		},
	),
}
