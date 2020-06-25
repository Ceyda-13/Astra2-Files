import uiScriptLocale
import item
import app

window = {
	"name" : "BioWindow",

	"x" : SCREEN_WIDTH - 176 - 148,
	"y" : SCREEN_HEIGHT - 37 - 565 + 209 + 32+100,
#	"x" : -148,
#	"y" : 241,
	"width" : 148,
	"height" : 139+25,

	"type" : "image",
	"image" : "d:/ymir work/ui/game/belt_inventory/bg.tga",
	

	"children" :
	(
		## Expand Buttons
		{
			"name" : "ExpandBtn",
			"type" : "button",

			"x" : 2,
			"y" : 25,

			"default_image" : "d:/ymir work/ui/game/belt_inventory/btn_expand_normal.tga",
			"over_image" : "d:/ymir work/ui/game/belt_inventory/btn_expand_over.tga",
			"down_image" : "d:/ymir work/ui/game/belt_inventory/btn_expand_down.tga",
			"disable_image" : "d:/ymir work/ui/game/belt_inventory/btn_expand_disabled.tga",
		},

		
		## Belt Inventory Layer (include minimize button)
		{
			"name" : "BeltInventoryLayer",
#			"type" : "board",
#			"style" : ("attach", "float"),

			"x" : 5,
			"y" : 0,

			"width" : 148,
			"height" : 139+25,

			"children" :
			(
				## Minimize Button
				{
					"name" : "MinimizeBtn",
					"type" : "button",

					"x" : 2,
					"y" : 25,

					"width" : 10,

					"default_image" : "d:/ymir work/ui/game/belt_inventory/btn_minimize_normal.tga",
					"over_image" : "d:/ymir work/ui/game/belt_inventory/btn_minimize_over.tga",
					"down_image" : "d:/ymir work/ui/game/belt_inventory/btn_minimize_down.tga",
					"disable_image" : "d:/ymir work/ui/game/belt_inventory/btn_minimize_disabled.tga",
				},
				
				

				## Real Belt Inventory Board
				{
					"name" : "BeltInventoryBoard",
					"type" : "board",
					"style" : ("attach", "float"),

					"x" : 10,
					"y" : 0,

					"width" : 138,
					"height" : 139+25,

					"children" :
					(
						## Belt Inventory Slots
						{
							"name" : "baslik",
							"type" : "text",

							"x" : 40,
							"y" : 15,

							"text" : "Biyolog Ekraný",

						},
						
						
						{
							"name" : "BeltInventorySlot",
							"type" : "grid_table",

							"x" : 5+50,
							"y" : 5+35,

							"start_index" : 999,
							"x_count" : 1,
							"y_count" : 1,
							"x_step" : 32,
							"y_step" : 32,

							"image" : "d:/ymir work/ui/public/Slot_Base.sub"
						},
						
						{
							"name" : "countslot",
							"type" : "image",
							"x" : 40,
							"y" :80,
							"image" : "d:/ymir work/ui/public/Parameter_Slot_02.sub",

							"children" :
							(
								{
									"name" : "count",
									"type":"text",
									"text":"0/0",
									"x":0,
									"y":0,
									"r":1.0,
									"g":1.0,
									"b":1.0,
									"a":1.0,
									"all_align" : "center",
								},
							),
						},
						
						{
							"name" : "timeslot",
							"type" : "image",
							"x" : 40,
							"y" :80+20,
							"image" : "d:/ymir work/ui/public/Parameter_Slot_02.sub",

							"children" :
							(
								{
									"name" : "time",
									"type":"text",
									"text":"59:45",
									"x":0,
									"y":0,
									"r":1.0,
									"g":1.0,
									"b":1.0,
									"a":1.0,
									"all_align" : "center",
								},
							),
						},
						
						
						{
							"name" : "biobutton",
							"type" : "button",

							"x" : 42,
							"y" : 105+20,

							"default_image" : "d:/ymir work/ui/public/acceptbutton00.tga",
							"over_image" : "d:/ymir work/ui/public/acceptbutton01.tga",
							"down_image" : "d:/ymir work/ui/public/acceptbutton02.tga",
						},
						

					),
				},
			)
		},

	),
}
