import uiScriptLocale

window = {
	"name" : "LoginWindow",
	"style" : ("movable",),
	
	"x" : 0,
	"y" : 0,
	
	"width" : SCREEN_WIDTH,
	"height" : SCREEN_HEIGHT,
	
	"children" :
	[
		# Main Background
		{
			"name" : "BGImg",
			"type" : "expanded_image", "x" : 0, "y" : 0,
			"x_scale" : float(SCREEN_WIDTH) / 1920.0,
			"y_scale" : float(SCREEN_HEIGHT) / 1080.0,
			#"horizontal_align" : "center",
			#"vertical_align" : "center", 
			"image" : "d:/ymir work/ui/login_new/bg_1920x1080.jpg",
		},
		{
			"name" : "LoginDialog",
			"type" : "window",
			
			"x" : 0,
			"y" : 0,
			
			"horizontal_align" : "center",
			"vertical_align" : "center",
			
			"width" : 800,
			"height" : 470,
			
			"children" : 
			[
				{
					"name" : "LoginWrapper",
					"type" : "image",
					
					"x" : 0,
					"y" : 87,
					
					"image" : "d:/ymir work/ui/login_new/dialogbg.dds",
					
					"children" : 
					[
						{
							"name" : "UserInputImg",
							"type" : "image",
							
							"x" : 16,
							"y" : 65,
							
							"image" : "d:/ymir work/ui/login_new/user_input.dds",
							
							"children" : 
							[
								{
									"name" : "UserEditLine",
									"type" : "editline",

									"x" : 50,
									"y" : 8,

									"width" : 194,
									"height" : 22,

									"input_limit" : 30,

									"fontsize" : "LARGE",
									
									"color" : 0xff8d5e51,
								},
							],
						},
						
						{
							"name" : "PwInputImg",
							"type" : "image",
							
							"x" : 16,
							"y" : 105,
							
							"image" : "d:/ymir work/ui/login_new/pw_input.dds",
							
							"children" : 
							[
								{
									"name" : "PwEditLine",
									"type" : "editline",

									"x" : 50,
									"y" : 8,

									"width" : 194,
									"height" : 22,

									"input_limit" : 30,
									"secret_flag" : 1,

									"fontsize" : "LARGE",
									
									"color" : 0xff8d5e51,
								},
							],
						},
						
						{
							"name" : "LoginBtn",
							"type" : "button",
							
							"x" : 17,
							"y" : 147,
							
							"default_image" : "d:/ymir work/ui/login_new/login_btn_default.dds",
							"over_image" : "d:/ymir work/ui/login_new/login_btn_over.dds",
							"down_image" : "d:/ymir work/ui/login_new/login_btn_down.dds",
						},
						{
							"name" : "ForgotPwBtn",
							"type" : "button",
							
							"x" : 100,
							"y" : 190,
							"width" : 93,
							"height" : 19,
							
							#"text_horizontal_align" : "center",
							#"text_vertical_align" : "center",
							
							#"text" : "Forgot password?",
							"children" : 
							[
								{ "name" : "ForgotPwText", "type" : "text", "x" : 0, "y" : 0, "text" : "Forgot password ?", "color" : 0xff905f53, "all_align" : "center", },
							],
						},
						{
							"name" : "SvButton1",
							"type" : "radio_button",
							
							"x" : 64,
							"y" : 220,
							
							"default_image" : "d:/ymir work/ui/login_new/sv1_btn_default.dds",
							"over_image" : "d:/ymir work/ui/login_new/sv1_btn_over.dds",
							"down_image" : "d:/ymir work/ui/login_new/sv1_btn_down.dds",
						},
						{
							"name" : "SvButton2",
							"type" : "radio_button",
							
							"x" : 146,
							"y" : 220,
							
							"default_image" : "d:/ymir work/ui/login_new/sv2_btn_default.dds",
							"over_image" : "d:/ymir work/ui/login_new/sv2_btn_over.dds",
							"down_image" : "d:/ymir work/ui/login_new/sv2_btn_down.dds",
						},
						{
							"name" : "ChannelBtn1",
							"type" : "radio_button",
							
							"x" : 271,
							"y" : 65,
							
							"default_image" : "d:/ymir work/ui/login_new/ch_btn_default.dds",
							"over_image" : "d:/ymir work/ui/login_new/ch_btn_over.dds",
							"down_image" : "d:/ymir work/ui/login_new/ch_btn_down.dds",
							
							"children" : 
							[
								{
									"name" : "ChannelText1",
									"type" : "text",
									
									"x" : 0,
									"y" : 4,
									
									"text_horizontal_align" : "center",
									"horizontal_align" : "center",
									
									"text" : "CH1",
									
									"fontsize" : "LARGE",
									"color" : 0xffa8602c,
								},
								
								{
									"name" : "ChannelState1",
									"type" : "text",
									
									"x" : 0,
									"y" : 23,
									
									"text_horizontal_align" : "center",
									"horizontal_align" : "center",
									
									"text" : "OFF",
									"color" : 0xffb77643,
								},
							],
						},
						
						{
							"name" : "ChannelBtn2",
							"type" : "radio_button",
							
							"x" : 386,
							"y" : 65,
							
							"default_image" : "d:/ymir work/ui/login_new/ch_btn_default.dds",
							"over_image" : "d:/ymir work/ui/login_new/ch_btn_over.dds",
							"down_image" : "d:/ymir work/ui/login_new/ch_btn_down.dds",
							
							"children" : 
							[
								{
									"name" : "ChannelText2",
									"type" : "text",
									
									"x" : 0,
									"y" : 4,
									
									"text_horizontal_align" : "center",
									"horizontal_align" : "center",
									
									"text" : "CH2",
									
									"fontsize" : "LARGE",
									"color" : 0xffa8602c,
								},
								
								{
									"name" : "ChannelState2",
									"type" : "text",
									
									"x" : 0,
									"y" : 23,
									
									"text_horizontal_align" : "center",
									"horizontal_align" : "center",
									
									"text" : "OFF",
									"color" : 0xffb77643,
								},
							],
						},
						
						{
							"name" : "ChannelBtn3",
							"type" : "radio_button",
							
							"x" : 271,
							"y" : 116,
							
							"default_image" : "d:/ymir work/ui/login_new/ch_btn_default.dds",
							"over_image" : "d:/ymir work/ui/login_new/ch_btn_over.dds",
							"down_image" : "d:/ymir work/ui/login_new/ch_btn_down.dds",
							
							"children" : 
							[
								{
									"name" : "ChannelText3",
									"type" : "text",
									
									"x" : 0,
									"y" : 4,
									
									"text_horizontal_align" : "center",
									"horizontal_align" : "center",
									
									"text" : "CH3",
									
									"fontsize" : "LARGE",
									"color" : 0xffa8602c,
								},
								
								{
									"name" : "ChannelState3",
									"type" : "text",
									
									"x" : 0,
									"y" : 23,
									
									"text_horizontal_align" : "center",
									"horizontal_align" : "center",
									
									"text" : "OFF",
									"color" : 0xffb77643,
								},
							],
						},
						{
							"name" : "ChannelBtn4",
							"type" : "radio_button",
							
							"x" : 386,
							"y" : 116,
							
							"default_image" : "d:/ymir work/ui/login_new/ch_btn_default.dds",
							"over_image" : "d:/ymir work/ui/login_new/ch_btn_over.dds",
							"down_image" : "d:/ymir work/ui/login_new/ch_btn_down.dds",
							
							"children" : 
							[
								{
									"name" : "ChannelText4",
									"type" : "text",
									
									"x" : 0,
									"y" : 4,
									
									"text_horizontal_align" : "center",
									"horizontal_align" : "center",
									
									"text" : "CH4",
									
									"fontsize" : "LARGE",
									"color" : 0xffa8602c,
								},
								
								{
									"name" : "ChannelState4",
									"type" : "text",
									
									"x" : 0,
									"y" : 23,
									
									"text_horizontal_align" : "center",
									"horizontal_align" : "center",
									
									"text" : "OFF",
									"color" : 0xffb77643,
								},
							],
						},
						{
							"name" : "ChannelBtn5",
							"type" : "radio_button",
							
							"x" : 271,
							"y" : 168,
							
							"default_image" : "d:/ymir work/ui/login_new/ch_btn_default.dds",
							"over_image" : "d:/ymir work/ui/login_new/ch_btn_over.dds",
							"down_image" : "d:/ymir work/ui/login_new/ch_btn_down.dds",
							
							"children" : 
							[
								{
									"name" : "ChannelText5",
									"type" : "text",
									
									"x" : 0,
									"y" : 4,
									
									"text_horizontal_align" : "center",
									"horizontal_align" : "center",
									
									"text" : "CH5",
									
									"fontsize" : "LARGE",
									"color" : 0xffa8602c,
								},
								
								{
									"name" : "ChannelState5",
									"type" : "text",
									
									"x" : 0,
									"y" : 23,
									
									"text_horizontal_align" : "center",
									"horizontal_align" : "center",
									
									"text" : "OFF",
									"color" : 0xffb77643,
								},
							],
						},
						{
							"name" : "ChannelBtn6",
							"type" : "radio_button",
							
							"x" : 386,
							"y" : 168,
							
							"default_image" : "d:/ymir work/ui/login_new/ch_btn_default.dds",
							"over_image" : "d:/ymir work/ui/login_new/ch_btn_over.dds",
							"down_image" : "d:/ymir work/ui/login_new/ch_btn_down.dds",
							
							"children" : 
							[
								{
									"name" : "ChannelText6",
									"type" : "text",
									
									"x" : 0,
									"y" : 4,
									
									"text_horizontal_align" : "center",
									"horizontal_align" : "center",
									
									"text" : "CH6",
									
									"fontsize" : "LARGE",
									"color" : 0xffa8602c,
								},
								
								{
									"name" : "ChannelState6",
									"type" : "text",
									
									"x" : 0,
									"y" : 23,
									
									"text_horizontal_align" : "center",
									"horizontal_align" : "center",
									
									"text" : "OFF",
									"color" : 0xffb77643,
								},
							],
						},
					],
					
				},
				{
					"name" : "RubinumLogo",
					"type" : "image", 
					
					"x" : 44,
					"y" : 10, # 18
					
					"image" : "d:/ymir work/ui/login_new/logo.tga",
				},				
				{
					"name" : "WebBtn",
					"type" : "button",
					
					"x" : -90 - 25 + 45,
					"y" : 54,
					
					"horizontal_align" : "center",
					"vertical_align" : "bottom",
					
					"default_image" : "d:/ymir work/ui/login_new/web_btn_default.dds",
					"over_image" : "d:/ymir work/ui/login_new/web_btn_over.dds",
					"down_image" : "d:/ymir work/ui/login_new/web_btn_down.dds",
				},
				{
					"name" : "TwitchBtn",
					"type" : "button",
					
					"x" : -45 - 25 + 45,
					"y" : 54,
					
					"horizontal_align" : "center",
					"vertical_align" : "bottom",
					
					"default_image" : "d:/ymir work/ui/login_new/twitch_btn_default.dds",
					"over_image" : "d:/ymir work/ui/login_new/twitch_btn_over.dds",
					"down_image" : "d:/ymir work/ui/login_new/twitch_btn_down.dds",
				},
				{
					"name" : "DiscordBtn",
					"type" : "button",
					
					"x" : 0 - 25 + 45,
					"y" : 54,
					
					"horizontal_align" : "center",
					"vertical_align" : "bottom",
					
					"default_image" : "d:/ymir work/ui/login_new/discord_btn_default.dds",
					"over_image" : "d:/ymir work/ui/login_new/discord_btn_over.dds",
					"down_image" : "d:/ymir work/ui/login_new/discord_btn_down.dds",
				},
				# {
					# "name" : "WikiBtn",
					# "type" : "button",
					
					# "x" : 45 - 25,
					# "y" : 54,
					
					# "horizontal_align" : "center",
					# "vertical_align" : "bottom",
					
					# "default_image" : "d:/ymir work/ui/login_new/wiki_btn_default.dds",
					# "over_image" : "d:/ymir work/ui/login_new/wiki_btn_over.dds",
					# "down_image" : "d:/ymir work/ui/login_new/wiki_btn_down.dds",
				# },
				# {
					# "name" : "BoardBtn",
					# "type" : "button",
					
					# "x" : 90 - 25,
					# "y" : 54,
					
					# "horizontal_align" : "center",
					# "vertical_align" : "bottom",
					
					# "default_image" : "d:/ymir work/ui/login_new/board_btn_default.dds",
					# "over_image" : "d:/ymir work/ui/login_new/board_btn_over.dds",
					# "down_image" : "d:/ymir work/ui/login_new/board_btn_down.dds",
				# },
				{
					"name" : "ExitBtn",
					"type" : "button",
					
					"x" : 135 - 25 - 45,
					"y" : 54,
					
					"horizontal_align" : "center",
					"vertical_align" : "bottom",
					
					"default_image" : "d:/ymir work/ui/login_new/exit_btn_default.dds",
					"over_image" : "d:/ymir work/ui/login_new/exit_btn_over.dds",
					"down_image" : "d:/ymir work/ui/login_new/exit_btn_down.dds",
				},
			],
		},
	],
}

window["children"][1]["children"] += [
	{
		"name" : "FooterWrapper",
		"type" : "image", 
		
		"x" : 0,
		"y" : 43 + 70,
		
		"vertical_align" : "bottom",
		
		"image" : "d:/ymir work/ui/login_new/footer_bg.dds",
		
		"children" : 
		[

		],
	},
]

a = len(window["children"][1]["children"]) - 1
b = float((795.0 - 44.0 - 33.0) / 15.0)
for i in xrange(16):
	window["children"][1]["children"][a]["children"] += [
		{
			"name" : "Lang%d" % i,
			"type" : "radio_button",

			"x" : 22 + b * i,
			"y" : 11,

			"width" : 33,
			"height" : 20,

			"default_image" : "d:/ymir work/ui/login_new/flags/%d_norm.tga" % i,
			"over_image" : "d:/ymir work/ui/login_new/flags/%d_over.tga" % i,
			"down_image" : "d:/ymir work/ui/login_new/flags/%d_down.tga" % i,
		},]

