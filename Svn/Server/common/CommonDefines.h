#pragma once

// ### General Features ###
#define _IMPROVED_PACKET_ENCRYPTION_
#define __UDP_BLOCK__
#define ENABLE_DEFAULT_PRIV // @correction110
//#define ENABLE_ITEMAWARD_REFRESH // @correction113
#define ENABLE_NEWGUILDMAKE // @correction145
#define ENABLE_QUEST_DIE_EVENT // @correction146
#define ENABLE_CMD_PLAYER // @correction147
#define ENABLE_EXPTABLE_FROMDB // @correction148
#define ENABLE_COMMON_FILES // @correction149
#define ENABLE_COMMON_CONFIGS // @correction150
#define ENABLE_AUTODETECT_INTERNAL_IP // @correction151
// #define ENABLE_IMMUNE_PERC // @correction152
#define ENABLE_IMMUNE_FIX // @correction153
//#define ENABLE_SPAMDB_REFRESH // @correction154
#define ENABLE_TRANSLATE_LUA // @correction155
// #define ENABLE_LOCALECHECK_CHANGENAME // @correction156
// #define ENABLE_PC_OPENSHOP // @correction157
// #define ENABLE_BLOCK_CMD_SHORTCUT // @correction158
#define ENABLE_SET_STATE_WITH_TARGET // @correction159
#define ENABLE_CMD_IPURGE_EX // @correction160
#define ENABLE_FORCE2MASTERSKILL // @correction161
// #define ENABLE_MOUNT_SKILL_ATTACK_CHECK // @correction162
// #define ENABLE_NULLIFYAFFECT_LIMIT // @correction163
// #define ENABLE_MASTER_SKILLBOOK_NO_STEPS // @correction164
#define ENABLE_FIREWORK_STUN // @correction165
#define ENABLE_ADDSTONE_FAILURE // @correction166
#define ENABLE_EFFECT_EXTRAPOT // @correction167
#define ENABLE_NEWEXP_CALCULATION // @correction168
#define ENABLE_EFFECT_PENETRATE // @correction169
#define NEW_ICEDAMAGE_SYSTEM // @correction118
#define ENABLE_ANTI_CMD_FLOOD // @correction170
#define ENABLE_OPEN_SHOP_WITH_ARMOR // @correction171
// #define ENABLE_SHOWNPCLEVEL // @correction172
#define ENABLE_GOHOME_IF_MAP_NOT_ALLOWED // @correction173
#define ENABLE_GM_FLAG_IF_TEST_SERVER // @correction174
#define ENABLE_GM_FLAG_FOR_LOW_WIZARD // @correction175
#define ENABLE_GOHOME_IF_MAP_NOT_EXIST // @correction176
#define ENABLE_PICKAXE_RENEWAL // @correction177
#define ENABLE_FISHINGROD_RENEWAL // @correction178
#define ENABLE_PARTYKILL // @correction179
// #define ENABLE_ACCOUNT_W_SPECIALCHARS // @correction180
#define ENABLE_CHAT_COLOR_SYSTEM // @correction181
// #define ENABLE_CHAT_LOGGING // @correction182
// #define ENABLE_CHAT_SPAMLIMIT // @correction183
#define ENABLE_WHISPER_CHAT_SPAMLIMIT // @correction184
#define ENABLE_CHECK_GHOSTMODE // @correction185
//#define ENABLE_QUEST_CATEGORY // @correction186
#define ENABLE_D_NJGUILD // @correction187
#define ENABLE_FULL_NOTICE // @correction188
#define ENABLE_NEWSTUFF //@correction232
#define ENABLE_PORT_SECURITY // @correction115
#define ENABLE_BELT_INVENTORY_EX // @correction189
#define UNLIMITED_ACCESSORY_SOCKET_TIME // @correction205
// ### General Features ###

// ### CommonDefines Systems ###
//#define __AUCTION__
#define __PET_SYSTEM__
// @correction190 BEGIN
#define ENABLE_WOLFMAN_CHARACTER
#ifdef ENABLE_WOLFMAN_CHARACTER
#define USE_MOB_BLEEDING_AS_POISON
#define USE_MOB_CLAW_AS_DAGGER
#define USE_ITEM_BLEEDING_AS_POISON
#define USE_ITEM_CLAW_AS_DAGGER
// #define USE_LYCAN_CREATE_POSITION
#define USE_WOLFMAN_STONES
#define USE_WOLFMAN_BOOKS
#endif
// @correction190 END
#define ENABLE_PLAYER_PER_ACCOUNT5 // @correction112
#define ENABLE_PARTY_FLAG // @correction116
#define __ACCE_SYSTEM__ // @correction191
#define __CHANGE_LOOK__ // @correction192
#define __CHANGE_LOOK_ITEM__ // @correction192
#define WJ_ELDER_ATTRIBUTE_SYSTEM // @correction193
#define ENABLE_FAST_NPC // @correction194
#define ENABLE_FAST_STAT // @correction195
#define ENABLE_SHOP_TAB // @correction196
#define ENABLE_MULTISHOP // @correction197
#define __VIEW_TARGET_PLAYER_HP__ // @correction198
#define __VIEW_TARGET_DECIMAL_HP__ // @correction199
#define ENABLE_GUILDWAR_KILL_COUNTER // @correction200
#define ENABLE_LIBL
#define ENABLE_DISCORD_RPC // @correction201
#define __SOULBIND__ // @correction202
#define __QUIVER__ // @correction203
#define __COSTUME_WEAPON__ // @correction204
#define __COSTUME_EFFECT__
#define __SEND_TARGET_INFO__
#define __OFFLINE_PRIVATE_SHOP__
#define __UNLIMITED_POTIONS__
#define __ELEMENT_ADD__
#define __PENDANT__
#define __MOVE_CHANNEL__
#define __MOUNT__
#define __COSTUME_MOUNT__
#define __COSTUME_PET__
#define __NEW_BONUSES__
#define __ALIGN_RENEWAL__
#define __GUILD_DRAGONLAIR__
#ifdef __GUILD_DRAGONLAIR__
	// #define __DESTROY_INFINITE_STATUES_GM__
	#define __LASER_EFFECT_ON_75HP__
	#define __LASER_EFFECT_ON_50HP__
#endif
#define __TEMPLE_OCHAO__
#define __678TH_SKILL__
#define __SUPPORT__
#define __GROWTH_PET__
#define __BOSS_TRACKING_SYSTEM__
#define __DEFENSE_WAVE__
#define __SHOW_CHEST_DROP__
#define __EXTEND_INVEN__
#define __SORT_INVENTORY__
#define __SPILT_ITEMS__
#define __PICK_ALL_ITEMS__
#define __NO_CLEAR_BUFF_WHEN_MONSTER_KILL__
#define __MINI_GAME__
#ifdef __MINI_GAME__
	#define __MINI_GAME_RUMI__
	#define __MINI_GAME_ATTENDANCE__
	#define __MINI_GAME_MONSTERBACK__
	#ifdef __MINI_GAME_MONSTERBACK__
		#define __ACCUMULATE_DAMAGE_DISPLAY__
	#endif
	#define __MINI_GAME_FISH__
	#define __MINI_GAME_CATCH_KING__
#endif
#define __ANTI_RESIST_MAGIC_REDUCTION__
#define __MOVE_COSTUME_ATTR__
#define __SPECIAL_GACHA__
#define __12ZI__
#define __RITUEL_STONE__
#define __GUILD_GENERAL_AND_LEADER__
#define __MULTI_LANGUAGE__
#define __DS_GRADE_MYTH__
#define __DS_CHANGE_ATTRIBUTE__
#define __CUBE_COPY_SOCKET_AND_APPLY__
#define __SOCKET_UPGRADE__
#define __SWITCHBOT__
#define __FAST_RUHBK__
#define __AFAR_BIOLOG__
#define __DS_REFINE_MASTER__
#define __SHOW_MAP_BOSS_AND_STONE__
#define __FAST_USE_GIFTBOX__
#define __KILL_NOTICE__
#define __AUTO_PICKUP__
#define __BLOCK_EXP__
#define __SOUL__
#define __ANTICHEAT__
#ifdef __ANTICHEAT__
	#define __HARDWARE_INFOS__
	#define WJ_SECURITY_SYSTEM
#endif
#define __SKILLS_LEVEL_OVER_P__
#define __ELEMENT_ALL__
#define __ADDON_ATTR__
// ### CommonDefines Systems ###
