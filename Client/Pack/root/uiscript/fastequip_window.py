import item
import uiScriptLocale

window = {
	"name" : "FastEquipWindow",
	
	"x" : SCREEN_WIDTH - 354,
	"y" : SCREEN_HEIGHT - 37 - 565,
	
	"style" : ("movable", "float",),
	
	"width" : 180,
	"height" : 280,
	
	"children" :
	(
		{
			"name" : "board",
			"type" : "board",
			"style" : ("attach",),
			
			"x" : 0,
			"y" : 0,
			
			"width" : 180,
			"height" : 280,
			
			"children" :
			(
				{
					"name" : "TitleBar",
					"type" : "titlebar",
					"style" : ("attach",),
					
					"x" : 6,
					"y" : 6,
					
					"width" : 170-2,
					"color" : "yellow",
					
					"children" :
					(
						{ "name":"TitleName", "type":"text", "x":(170-12)/2, "y":3, "text":"Hýzlý Ekipman", "text_horizontal_align":"center" },
					),
				},
				
				{
					"name" : "equip_Base",
					"type" : "image",
					
					"x" : 13,
					"y" : 35,
					
					"image" : "fast_equip/equipchange.tga",
					
					"children" :
					(

						{
							"name" : "equipslot",
							"type" : "slot",
							"x" : 3,
							"y" : 3,
							
							"width" : 145,
							"height" : 172,
							
							"slot" : (
										{"index":1, "x":39, "y":37, "width":32, "height":64},
										{"index":2, "x":39, "y":2, "width":32, "height":32},
										{"index":3, "x":39, "y":145, "width":32, "height":32},
										{"index":4, "x":75, "y":67, "width":32, "height":32},
										{"index":5, "x":3, "y":3, "width":32, "height":96},
										{"index":6, "x":114, "y":67, "width":32, "height":32},
										{"index":7, "x":114, "y":35, "width":32, "height":32},
										{"index":8, "x":75, "y":35, "width":32, "height":32},
										{"index":9, "x":114, "y":2, "width":32, "height":32},
										{"index":10, "x":39, "y":106, "width":32, "height":32},
									),
						},
					),
				},
				
				{
					"name" : "change_button",
					"type" : "button",
					
					"x" : 15+3-10+3,
					"y" : 218+5,
					
					"text" : "Deðiþtir",
					
					"default_image" : "d:/ymir work/ui/public/middle_Button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_Button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_Button_03.sub",
				},
				
				{
					"name" : "clear_button",
					"type" : "button",
					
					"x" : 95+13,
					"y" : 218+5,
					
					"text" : "Temizle",
					
					"default_image" : "d:/ymir work/ui/public/middle_Button_01.sub",
					"over_image" : "d:/ymir work/ui/public/middle_Button_02.sub",
					"down_image" : "d:/ymir work/ui/public/middle_Button_03.sub",
				},
				
				{
					"name" : "page1_button",
					"type" : "button",
					
					"x" : 15+3-10+3,
					"y" : 245,
					
					"text" : "Geri",
					
					"default_image" : "d:/ymir work/ui/public/Small_Button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Small_Button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Small_Button_03.sub",
				},
				
				{
					"name" : "page2_button",
					"type" : "radio_button",
					
					"x" : (170/2)-19+3,
					"y" : 245,
					
					#"text" : "II",
					
					"default_image" : "d:/ymir work/ui/public/Small_Button_011.sub",
					"over_image" : "d:/ymir work/ui/public/Small_Button_021.sub",
					"down_image" : "d:/ymir work/ui/public/Small_Button_031.sub",
				},
				
				{
					"name" : "page3_button",
					"type" : "button",
					
					"x" : 115+3+3+5,
					"y" : 245,
					
					"text" : "Ýleri",
					
					"default_image" : "d:/ymir work/ui/public/Small_Button_01.sub",
					"over_image" : "d:/ymir work/ui/public/Small_Button_02.sub",
					"down_image" : "d:/ymir work/ui/public/Small_Button_03.sub",
				},
				
				{
					"name" : "page4_button",
					"type" : "radio_button",
					
					"x" : 15+3,
					"y" : 265+3,
					
					#"text" : "IV",
					
					"default_image" : "d:/ymir work/ui/public/Small_Button_011.sub",
					"over_image" : "d:/ymir work/ui/public/Small_Button_021.sub",
					"down_image" : "d:/ymir work/ui/public/Small_Button_031.sub",
				},
				{
					"name" : "page5_button",
					"type" : "radio_button",
					
					"x" : (170/2)-19+3,
					"y" : 265+3,
					
					#"text" : "V",
					
					"default_image" : "d:/ymir work/ui/public/Small_Button_011.sub",
					"over_image" : "d:/ymir work/ui/public/Small_Button_021.sub",
					"down_image" : "d:/ymir work/ui/public/Small_Button_031.sub",
				},
				{
					"name" : "page6_button",
					"type" : "radio_button",
					
					"x" : 115+3,
					"y" : 265+3,
					
					#"text" : "VI",
					
					"default_image" : "d:/ymir work/ui/public/Small_Button_011.sub",
					"over_image" : "d:/ymir work/ui/public/Small_Button_021.sub",
					"down_image" : "d:/ymir work/ui/public/Small_Button_031.sub",
				},
				{
					"name" : "BetragInputImage",
					"type" : "image",
					"horizontal_align" : "Center",
					"x" : 56+2,
					"y" : 246,
	
					"image" : "d:/ymir work/ui/public/Parameter_Slot_02.sub",
				},
				{
					"name" : "imas",
					"type" : "image",
					"horizontal_align" : "Center",
					"x" : (170/2)-19+3+12,
					"y" : 218+7,
	
					"image" : "d:/ymir work/ui/game/windows/money_icon.sub",
				},
				{
					"name" : "AmountInput",
					"type" : "text",
					"text" : "I",
					"x" : (170/2)-19+3+16,
					"y" : 248,
				},
			),
		},
	),
}

