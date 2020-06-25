import uiScriptLocale
import item
import player

EQUIPMENT_START_INDEX = player.EQUIPMENT_SLOT_START
window = {
	"name" : "InventoryWindow",

	## 600 - (width + 오른쪽으로 부터 띄우기 24 px)
	"x" : SCREEN_WIDTH - 176,
	"y" : SCREEN_HEIGHT - 37 - 567,

	"style" : ["movable", "float",],

	"width" : 176,
	"height" : 545,

	"children" :
	[
		## Inventory, Equipment Slots
		{
			"name" : "board",
			"type" : "board",
			"style" : ["attach",],

			"x" : 0,
			"y" : 0,

			"width" : 176,
			"height" : 545,

			"children" :
			[
				## Title
				{
					"name" : "TitleBar",
					"type" : "titlebar",
					"style" : ["attach",],

					"x" : 8 + 40,
					"y" : 8,

					"width" : 161 - 40,
					"color" : "yellow",

					"children" :
					[
						{ "name":"TitleName", "type":"text", "x":77 - 20, "y":4, "text":uiScriptLocale.INVENTORY_TITLE, "text_horizontal_align":"center" },
					],
				},

				## Equipment Slot
				{
					"name" : "Equipment_Base",
					"type" : "image",

					"x" : 10,
					"y" : 33,

					"image" : "d:/ymir work/ui/equipment_bg_without_ring.tga",

					"children" :
					[
						{
							"name" : "EquipmentSlot",
							"type" : "slot",

							"x" : 3,
							"y" : 3,

							"width" : 150,
							"height" : 182,

							"slot" : [
										{"index":EQUIPMENT_START_INDEX+0, "x":39, "y":37, "width":32, "height":64},
										{"index":EQUIPMENT_START_INDEX+1, "x":39, "y":2, "width":32, "height":32},
										{"index":EQUIPMENT_START_INDEX+2, "x":39, "y":145, "width":32, "height":32},
										{"index":EQUIPMENT_START_INDEX+3, "x":75, "y":67, "width":32, "height":32},
										{"index":EQUIPMENT_START_INDEX+4, "x":3, "y":3, "width":32, "height":96},
										{"index":EQUIPMENT_START_INDEX+5, "x":114, "y":67, "width":32, "height":32},
										{"index":EQUIPMENT_START_INDEX+6, "x":114, "y":35, "width":32, "height":32},
										{"index":EQUIPMENT_START_INDEX+7, "x":2, "y":145, "width":32, "height":32},
										{"index":EQUIPMENT_START_INDEX+8, "x":75, "y":145, "width":32, "height":32},
										{"index":EQUIPMENT_START_INDEX+9, "x":114, "y":2, "width":32, "height":32},
										{"index":EQUIPMENT_START_INDEX+10, "x":75, "y":35, "width":32, "height":32},
										## 새 반지1
										##{"index":item.EQUIPMENT_RING1, "x":2, "y":106, "width":32, "height":32},
										## 새 반지2
										##{"index":item.EQUIPMENT_RING2, "x":75, "y":106, "width":32, "height":32},
										## 새 벨트
										{"index":item.EQUIPMENT_BELT, "x":39, "y":106, "width":32, "height":32},
									],
						},
						## Dragon Soul Button
						{
							"name" : "DSSButton",
							"type" : "button",

							"x" : 114,
							"y" : 107,

							"tooltip_text" : uiScriptLocale.TASKBAR_DRAGON_SOUL,

							"default_image" : "d:/ymir work/ui/dragonsoul/dss_inventory_button_01.tga",
							"over_image" : "d:/ymir work/ui/dragonsoul/dss_inventory_button_02.tga",
							"down_image" : "d:/ymir work/ui/dragonsoul/dss_inventory_button_03.tga",
						},
						## MallButton
						{
							"name" : "MallButton",
							"type" : "button",

							"x" : 118,
							"y" : 148,

							"tooltip_text" : uiScriptLocale.MALL_TITLE,

							"default_image" : "d:/ymir work/ui/game/TaskBar/Mall_Button_01.tga",
							"over_image" : "d:/ymir work/ui/game/TaskBar/Mall_Button_02.tga",
							"down_image" : "d:/ymir work/ui/game/TaskBar/Mall_Button_03.tga",
						},
						## CostumeButton
						{
							"name" : "CostumeButton",
							"type" : "button",

							"x" : 78,
							"y" : 5,

							"tooltip_text" : uiScriptLocale.COSTUME_TITLE,

							"default_image" : "d:/ymir work/ui/game/taskbar/costume_Button_01.tga",
							"over_image" : "d:/ymir work/ui/game/taskbar/costume_Button_02.tga",
							"down_image" : "d:/ymir work/ui/game/taskbar/costume_Button_03.tga",
						},
						{
							"name" : "Equipment_Tab_01",
							"type" : "radio_button",

							"x" : 86,
							"y" : 161,

							"default_image" : "d:/ymir work/ui/game/windows/tab_button_small_01.sub",
							"over_image" : "d:/ymir work/ui/game/windows/tab_button_small_02.sub",
							"down_image" : "d:/ymir work/ui/game/windows/tab_button_small_03.sub",

							"children" :
							[
								{
									"name" : "Equipment_Tab_01_Print",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",

									"text" : "I",
								},
							],
						},
						{
							"name" : "Equipment_Tab_02",
							"type" : "radio_button",

							"x" : 86 + 32,
							"y" : 161,

							"default_image" : "d:/ymir work/ui/game/windows/tab_button_small_01.sub",
							"over_image" : "d:/ymir work/ui/game/windows/tab_button_small_02.sub",
							"down_image" : "d:/ymir work/ui/game/windows/tab_button_small_03.sub",

							"children" :
							[
								{
									"name" : "Equipment_Tab_02_Print",
									"type" : "text",

									"x" : 0,
									"y" : 0,

									"all_align" : "center",

									"text" : "II",
								},
							],
						},
					],
				},
				## Item Slot
				{
					"name" : "ItemSlot",
					"type" : "grid_table",

					"x" : 8,
					"y" : 246,

					"start_index" : 0,
					"x_count" : 5,
					"y_count" : 9,
					"x_step" : 32,
					"y_step" : 32,

					"image" : "d:/ymir work/ui/public/Slot_Base.sub"
				},
			],
		},
	],
}

# @@@correction052 BEGIN
import player
for i in xrange(player.INVENTORY_PAGE_COUNT):
	tooltip_text = uiScriptLocale.INVENTORY_PAGE_BUTTON_TOOLTIP_1
	if i == 1:
		tooltip_text = uiScriptLocale.INVENTORY_PAGE_BUTTON_TOOLTIP_2
	if i == 2:
		tooltip_text = uiScriptLocale.INVENTORY_PAGE_BUTTON_TOOLTIP_3
	if i == 3:
		tooltip_text = uiScriptLocale.INVENTORY_PAGE_BUTTON_TOOLTIP_4
	window["children"][0]["children"] += [
		{
			"name" : "Inventory_Tab_0%d" % (i + 1),
			"type" : "radio_button",

			"x" : 10 + i * 39,
			"y" : 33 + 191,

			"default_image" : "d:/ymir work/ui/game/windows/tab_button_large_half_01.sub",
			"over_image" : "d:/ymir work/ui/game/windows/tab_button_large_half_02.sub",
			"down_image" : "d:/ymir work/ui/game/windows/tab_button_large_half_03.sub",
			"tooltip_text" : tooltip_text,

			"children" :
			[
				{
					"name" : "Inventory_Tab_0%d_Print" % (i + 1),
					"type" : "text",

					"x" : 0,
					"y" : 0,

					"all_align" : "center",

					"text" : RomaNumers[i],
				},
			],
		},]
# @@@correction052 END

if ENABLE_PENDANT: 
	window["children"][0]["children"][1]["children"][0]["slot"] += [
										{"index":item.EQUIPMENT_PENDANT, "x":3, "y":106, "width":32, "height":32},]
if ENABLE_COSTUME_PET:
	window["children"][0]["children"][1]["image"] = "d:/ymir work/ui/equipment_bg_without_ring_with_pet.tga"
	window["children"][0]["children"][1]["children"][0]["slot"] += [
										{"index":item.EQUIPMENT_PET, "x":75, "y":106, "width":32, "height":32},]

if ENABLE_EXTEND_INVEN_SYSTEM:
	window["height"] += 70 + 5
	window["y"] -= 70 + 5
	window["children"][0]["height"] += 70 + 5
	window["children"][0]["children"] += [
		{
			"name" : "Category_Bg",
			"type" : "image",

			"image" : "storage/btn_bg.dds",

			"x" : 8,
			"y" : 538,
		},]

	for i in xrange(6):
		window["children"][0]["children"] += [
			{
				"name" : "Category_Tab_%d" % i,
				"type" : "radio_button",

				"x" : 10 + ((i % 3) * 61),
				"y" : 538 + (37 * (i / 3)),

				"default_image" : "storage/%d_0.dds" % i,
				"over_image" : "storage/%d_1.dds" % i,
				"down_image" : "storage/%d_2.dds" % i,

			},]


if ENABLE_SORT_ITEMS:
	window["children"][0]["children"] += [
						{
							"name" : "YenileButton",
							"type" : "button",

							"x" : 7,
							"y" : 7.6,

							"tooltip_text" : " Envanteri duzenle ",

							"default_image" : "d:/ymir work/ui/game/windows/refresh_titlebar_up.sub",
							"over_image" : "d:/ymir work/ui/game/windows/refresh_titlebar_over.sub",
							"down_image" : "d:/ymir work/ui/game/windows/refresh_titlebar_down.sub",
						},]

if ENABLE_NEW_BUTTONS:
	for i in xrange(3):
		window["children"][0]["children"] += [
			{
				"name" : "New_Button_%d" % i,
				"type" : "button",

				"x" : -16.5,
				"y" : 33 + 22 * i,

				"default_image" : "inventory/%d_0.png" % i,
				"over_image" : "inventory/%d_1.png" % i,
				"down_image" : "inventory/%d_2.png" % i,
				
				"always_pick" : 1,
				
				# "tooltip_text" : "",
				# "tooltip_x" : -56,
				# "tooltip_y" : 4,
			},]

