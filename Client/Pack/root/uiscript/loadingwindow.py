import uiScriptLocale
import localeInfo

window = {

	"x" : 0,
	"y" : 0,

	"width" : SCREEN_WIDTH,
	"height" : SCREEN_HEIGHT,

	"children" :
	[
		## Board
		{
			"name" : "BackGround",
			"type" : "expanded_image",

			"x" : 0,
			"y" : 0,

			"image" : "d:/ymir work/ui/intro/pattern/Line_Pattern.tga",

			"x_scale" : float(SCREEN_WIDTH) / 800.0,
			"y_scale" : float(SCREEN_HEIGHT) / 600.0,
		},
		{ 
			"name":"ErrorMessage", 
			"type":"text", "x":10, "y":10, 
			"text": uiScriptLocale.LOAD_ERROR, 
		},
		
		{
			"name" : "GageBoard",
			"type" : "window",
			"style" : ("ltr",),
			"x" : float(SCREEN_WIDTH) * 400 / 800.0 - 200 - 27 - 27,
			"y" : float(SCREEN_HEIGHT) * 500 / 600.0 ,
			"width" : 400, 
			"height": 80,

			"children" :
			[
			
				{
					"name" : "BackGage",
					"type" : "expanded_image",

					"x" : 40,
					"y" : 25,

					"image" : uiScriptLocale.LOCALE_UISCRIPT_PATH + "loading/gauge_empty.dds",
				},
				{
					"name" : "FullGage",
					"type" : "expanded_image",

					"x" : 40,
					"y" : 25,

					"image" : uiScriptLocale.LOCALE_UISCRIPT_PATH + "loading/gauge_full.dds",
				},
			],
		},
	],
}

if localeInfo.IsARABIC():
	window["children"][0]["image"] = "d:/ymir work/uiloading/background_loading_warrior.sub"

