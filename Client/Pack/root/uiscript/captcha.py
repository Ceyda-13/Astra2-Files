import uiScriptLocale

window = {
	"name" : "QuestionDialog",
	"style" : ("movable", "float",),

	"x" : SCREEN_WIDTH/2 - 125,
	"y" : SCREEN_HEIGHT/2 - 52,

	"width" : 280,
	"height" : 110+125,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board",

			"x" : 0,
			"y" : 0,

			"width" : 280,
			"height" : 110+125,

			"children" :
			(				
				{
					"name" : "captchaimage",
					"type" : "image",

					"x" : 35,
					"y" : 33,

					"image" : "captcha/background.tga",

					"children" :
					(
						{
							"name" : "cp1",
							"type" : "image",
							
							"image" : "captcha/1.tga",

							"x" : 5,
							"y" : 15,

						},
						
						{
							"name" : "cp2",
							"type" : "image",
							
							"image" : "captcha/1.tga",

							"x" : 15+15+10,
							"y" : 15,

						},
						
						{
							"name" : "cp3",
							"type" : "image",
							
							"image" : "captcha/9.tga",

							"x" : 15+15+10+40,
							"y" : 15,

						},
						
						{
							"name" : "cp4",
							"type" : "image",
							
							"image" : "captcha/3.tga",

							"x" : 15+15+10+40+40,
							"y" : 15,

						},
						
						{
							"name" : "cp5",
							"type" : "image",
							
							"image" : "captcha/8.tga",

							"x" : 15+15+10+40+40+40,
							"y" : 15,

						},
					),
				},
				
				{
					"name" : "message1",
					"type" : "text",

					"x" : 5,
					# "y" : 25+100,
					"y" : 18,

					"text" : uiScriptLocale.MESSAGE,

					"horizontal_align" : "center",
					"text_horizontal_align" : "center",
					"text_vertical_align" : "center",
				},
				{
					"name" : "message3",
					"type" : "text",

					"x" : 0,
					# "y" : 40+100,
					"y" : 25+100,

					"text" : uiScriptLocale.MESSAGE,

					"horizontal_align" : "center",
					"text_horizontal_align" : "center",
					"text_vertical_align" : "center",
				},
				{
					"name" : "message2",
					"type" : "text",

					"x" : 0,
					# "y" : 50+5+100,
					"y" : 35+5+100,

					"text" : uiScriptLocale.MESSAGE,

					"horizontal_align" : "center",
					"text_horizontal_align" : "center",
					"text_vertical_align" : "center",
				},
				
				{
					"name" : "b0",
					"type" : "button",

					"x" : -80,
					"y" : 68+5+100-15,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : "",

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},
				
				{
					"name" : "b1",
					"type" : "button",

					"x" : 0,
					"y" : 68+5+100-15,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : "",

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},
				{
					"name" : "b2",
					"type" : "button",

					"x" : 80,
					"y" : 68+5+100-15,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : "",

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},
				
				{
					"name" : "b3",
					"type" : "button",

					"x" : -80,
					"y" : 68+5+100+25,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : "",

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},
				
				{
					"name" : "b4",
					"type" : "button",

					"x" : 0,
					"y" : 68+5+100+25,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : "",

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},
				{
					"name" : "b5",
					"type" : "button",

					"x" : 80,
					"y" : 68+5+100+25,

					"width" : 61,
					"height" : 21,

					"horizontal_align" : "center",
					"text" : "",

					"default_image" : "d:/ymir work/ui/public/middle_button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_button_03.sub",
				},
			),
		},
	),
}