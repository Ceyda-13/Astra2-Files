import uiScriptLocale
PUBLIC_PATH =  "d:/ymir work/ui/public/"

WINDOW_WIDTH = 352
WINDOW_HEIGHT = 467

window = {
	"name" : "PrivateShopEditor",

	"x" : 0,
	"y" : 0,

	"style" : ("movable",),

	"width" : WINDOW_WIDTH,
	"height" : WINDOW_HEIGHT,

	"children" :
	(
		{
			"name" : "board",
			"type" : "board",
			"style" : ("attach",),

			"x" : 0,
			"y" : 0,

			"width" : WINDOW_WIDTH,
			"height" : WINDOW_HEIGHT,

			"children" :
			(
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
					(
						{ "name":"TitleName", "type":"text", "x":WINDOW_WIDTH / 2, "y":4, "text":"Düzenleme Ekraný", "text_horizontal_align":"center" },
					),
				},

				## Item Slot
				{
					"name" : "ItemSlot",
					"type" : "grid_table",

					"x" : 15,
					"y" : 123,

					"start_index" : 0,
					"x_count" : 10,
					"y_count" : 9,
					"x_step" : 32,
					"y_step" : 32,

					"image" : "d:/ymir work/ui/public/Slot_Base.sub",
				},
				## Item Slot
				# {
					# "name" : "ItemSlot2",
					# "type" : "grid_table",

					# "x" : 175,
					# "y" : 123,

					# "start_index" : 45,
					# "x_count" : 5,
					# "y_count" : 9,
					# "x_step" : 32,
					# "y_step" : 32,

					# "image" : "d:/ymir work/ui/public/Slot_Base.sub",
				# },
				{
					"name" : "OfflineWrapper",
					"type" : "window",
					
					"x" : 13,
					"y" : 34,

					"width" : 322,
					"height" : 70,

					"children":
					(
					
						# Back Button
						{
							"name" : "BackButton",
							"type" : "button",

							"x" : 2,
							"y" : 0,

							"default_image" : "d:/ymir work/ui/game/shopsearchp2p/btn_back_normal.dds",
							"over_image" : "d:/ymir work/ui/game/shopsearchp2p/btn_back_over.dds",
							"down_image" : "d:/ymir work/ui/game/shopsearchp2p/btn_back_down.dds",
						},
						
						# Page 1 button
						{
							"name" : "Page1Button",
							"type" : "radio_button",

							"x" : 2,
							"y" : 0,
							
							"tooltip_text" : "1. Sayfa",

							"default_image" : "d:/ymir work/ui/game/shopsearchp2p/pageone.dds",
							"over_image" : "d:/ymir work/ui/game/shopsearchp2p/pageone_hover.dds",
							"down_image" : "d:/ymir work/ui/game/shopsearchp2p/pageone_active.dds",
						},
						
						# Page 2 button
						{
							"name" : "Page2Button",
							"type" : "radio_button",

							"x" : 27,
							"y" : 0,
							
							"tooltip_text" : "2. Sayfa",

							"default_image" : "d:/ymir work/ui/game/shopsearchp2p/pagetwo.dds",
							"over_image" : "d:/ymir work/ui/game/shopsearchp2p/pagetwo_hover.dds",
							"down_image" : "d:/ymir work/ui/game/shopsearchp2p/pagetwo_active.dds",
						},

						## Name
						{
							"name" : "NameSlot",
							"type" : "slotbar",
							"x" : 2,
							"y" : 30,
							"width" : 297,
							"height" : 18,
							
							# "color_left" : 0xff2A2522,
							# "color_right" : 0xff433B38,
							# "color_center" : 0xff000000,

							"children" :
							(
								{
									"name" : "NameLine",
									"type" : "editline",
									"x" : 3,
									"y" : 3,
									"width" : 291,
									"height" : 15,
									"input_limit" : 25,
									"text" : "1234567890123456789012345",
								},
							),
						},
						
						# Rename Button OK
						{
							"name" : "RenameButton",
							"type" : "button",

							"x" : 304,
							"y" : 30,

							"default_image" : "d:/ymir work/ui/game/shopsearchp2p/checkbutton.dds",
							"over_image" : "d:/ymir work/ui/game/shopsearchp2p/checkbutton_over.dds",
							"down_image" : "d:/ymir work/ui/game/shopsearchp2p/checkbutton_down.dds",

							"text": "",
						},

						{
							"name" : "Position",
							"type" : "text",

							"x" : 2,
							"y" : 54,

							"width" : 184 - 20,
							"height" : 20,

							"text": "",
						},
						
						{
							"name" : "ItemsNetWorthText",
							"type" : "text",

							"x" : 2,
							"y" : 69,

							"width" : 184 - 20,
							"height" : 20,

							"text": "Toplam Tutar: 2.000.000.000",
						},
						
						### Right buttons
						
						# FAQ Button
						{
							"name" : "ShopFAQButton",
							"type" : "button",

							"horizontal_align" : "right", 
							"x" : 20,
							"y" : 0,

							"default_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_faq_normal.dds",
							"over_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_faq_hover.dds",
							"down_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_faq_pressed.dds",

							"tooltip_text": "Yardým",
							"tooltip_x" : 0,
							"tooltip_y" : 25,
						},
						
						# Close Button
						{
							"name" : "CloseButton",
							"type" : "button",

							"horizontal_align" : "right", 
							"x" : 68,
							"y" : 0,
							
							"tooltip_text" : "Pazarý Kapat",
							"tooltip_x" : 0,
							"tooltip_y" : 25,
							
							"default_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_close_normal.dds",
							"over_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_close_hover.dds",
							"down_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_close_pressed.dds",
						},

						# Open New Shop button
						{
							"name" : "OpenShopButton",
							"type" : "button",

							"horizontal_align" : "right", 
							"x" : 68,
							"y" : 0,
							
							"tooltip_text" : "Yeni Pazar Aç",
							"tooltip_x" : 0,
							"tooltip_y" : 25,
							
							"default_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_close_normal.dds",
							"over_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_close_hover.dds",
							"down_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_close_pressed.dds",
						},
						
						# Shop Search opener button
						{
							"name" : "ShopSearchButton",
							"type" : "button",

							"horizontal_align" : "right", 
							"x" : 116,
							"y" : 0,
							
							"tooltip_text" : "Item Arama",
							"tooltip_x" : 0,
							"tooltip_y" : 25,
							
							"default_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_search_normal.dds",
							"over_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_search_hover.dds",
							"down_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_search_pressed.dds",
						},
						
						# Log with sold items button
						{
							"name" : "ShopLogButton",
							"type" : "radio_button",

							"horizontal_align" : "right", 
							"x" : 164,
							"y" : 0,
							
							"tooltip_text" : "Pazar Geçmiþi",
							"tooltip_x" : 0,
							"tooltip_y" : 25,
							
							"default_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_log_normal.dds",
							"over_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_log_hover.dds",
							"down_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_log_active.dds",
						},
						
						# Shop deco choose button
						{
							"name" : "ShopDecoButton",
							"type" : "button",

							"horizontal_align" : "right", 
							"x" : 212,
							"y" : 0,
							
							"tooltip_text" : "Dekorasyon Deðiþtir",
							"tooltip_x" : 0,
							"tooltip_y" : 25,
							
							"default_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_skin_normal.dds",
							"over_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_skin_hover.dds",
							"down_image" : "d:/ymir work/ui/game/shopsearchp2p/shop_btn_skin_pressed.dds",
						},
						
						# Teleport button
						{
							"name" : "TeleportBtn",
							"type" : "button",

							"horizontal_align" : "right", 
							"x" : 88,
							"y" : 54,
							
							"text" : "",
							
							"default_image" : "d:/ymir work/ui/small_btn.dds",
							"over_image" : "d:/ymir work/ui/small_btn_over.dds",
							"down_image" : "d:/ymir work/ui/small_btn_down.dds",
						},
					),
				},
				
				## Log Manager ##
				
				{
					"name" : "Log_Page",
					"type" : "window",
					
					"x" : 10, "y" : 60, 
					
					"width" : 331, "height" : 380,
					
					"children" : 
					(
					
						# Header Item Name
						{ "name" : "Log_Header_Item_Name", "type" : "text", "x" : 15, "y" : 0, "text" : "Satilan Nesne", },
						{ "name" : "Log_Header_Buy_Time", "type" : "text", "x" : 144, "y" : 0, "text" : "Zaman", },
						{ "name" : "Log_Header_Item_Price", "type" : "text", "x" : 15, "y" : 0, "text" : "Fiyat", "horizontal_align" : "right", "text_horizontal_align" : "right",  },
						
						# Log Container
						{
							"name" : "Log_Container",
							"type" : "brownboard2",
							"x" : 0, "y" : 16, "width" : 331, "height" : 380-16,
							
							"clip_master" : 1,
						},
						
						# Main Board Scroll
						{
							"name" : "LogScrollBar",
							"type" : "slimscrollbar",

							"x" : 326,
							"y" : 16,
							"size" : 380-16,
						},
					),
				},
				
				## END OF LOG ##
				
				{
					"name":"Shop_Time_Gauge_Slot",
					"type":"image",
					"x" : 17,
					"y" : 51,
					"vertical_align" : "bottom", 
					"image" : "d:/ymir work/ui/game/shopsearchp2p/timegauge.dds",
					
					"children" : 
					(
						{
							"name" : "Shop_Time_Gauge",
							"type" : "ani_image",

							"x" : 4,
							"y" : 0,

							"delay" : 6,

							"images" :
							(
								"D:/Ymir Work/UI/Pattern/SPGauge/01.tga",
								"D:/Ymir Work/UI/Pattern/SPGauge/02.tga",
								"D:/Ymir Work/UI/Pattern/SPGauge/03.tga",
								"D:/Ymir Work/UI/Pattern/SPGauge/04.tga",
								"D:/Ymir Work/UI/Pattern/SPGauge/05.tga",
								"D:/Ymir Work/UI/Pattern/SPGauge/06.tga",
								"D:/Ymir Work/UI/Pattern/SPGauge/07.tga",
							),
						},
					),
				},
				
				{
					"name" : "Shop_Time_Text",
					"type" : "text",

					"x" : 18,
					"y" : 30,
					
					"vertical_align" : "bottom",

					"text": "Kalan Zaman: 02:50:22 / 3Days",
				},
				

				{
					"name" : "GoldStashWrapper",
					"type" : "window",

					"x" : 183,
					"y" : 32,
					
					"vertical_align" : "bottom", 
					"horizontal_align" : "right",  

					"width" : 184,
					"height" : 32,

					"children" :
					(
						{
							"name" : "GoldStash_SlotBar",
							"type" : "slotbar",
							"x" : 8,
							"y" : 0,
							"width" : 130,
							"height" : 18,
							"horizontal_align":"center",
							
							# "color_left" : 0xff2A2522,
							# "color_right" : 0xff433B38,
							# "color_center" : 0xff000000,

							"children" :
							(
								{
									"name":"GoldStash_Slot",
									"type":"button",

									"x":0,
									"y":0,

									"horizontal_align":"center",
									#"vertical_align":"bottom",

									#"default_image" : "d:/ymir work/ui/public/parameter_slot_05.sub",
									#"over_image" : "d:/ymir work/ui/public/parameter_slot_05.sub",
									#"down_image" : "d:/ymir work/ui/public/parameter_slot_05.sub",
									
									"width" : 130,
									"height" : 18,

									"children" :
									(
										{
											"name":"WithdrawStash",
											"type":"image",

											"x":-18,
											"y":2,

											"image":"d:/ymir work/ui/game/windows/money_icon.sub",
										},
										{
											"name" : "GoldStashValue",
											"type" : "text",

											"x" : 3,
											"y" : 3,

											"horizontal_align" : "right",
											"text_horizontal_align" : "right",

											"text" : "123456789",
										},
									),
								},
							),
						},
					),					
				},

				{
					"name" : "time_window",
					"type" : "window",
					
					"x" : 80,
					"y" : 75,
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
				},
			),
		},
	),
}