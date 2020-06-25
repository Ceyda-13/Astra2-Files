#pragma once

enum
{
	ADDRESS_MAX_LEN = 15
};

void config_init(const std::string& st_localeServiceName);

extern char sql_addr[256];

extern uint16_t mother_port;
extern uint16_t p2p_port;

extern char db_addr[ADDRESS_MAX_LEN + 1];
extern uint16_t db_port;

extern char teen_addr[ADDRESS_MAX_LEN + 1];
extern uint16_t teen_port;

extern char passpod_addr[ADDRESS_MAX_LEN + 1];
extern uint16_t passpod_port;

extern int32_t passes_per_sec;
extern int32_t save_event_second_cycle;
extern int32_t ping_event_second_cycle;
extern int32_t test_server;
extern bool	guild_mark_server;
extern uint8_t guild_mark_min_level;
extern bool	distribution_test_server;
extern bool	china_event_server;

extern bool	g_bNoMoreClient;
extern bool	g_bNoRegen;

extern bool	g_bTrafficProfileOn;

extern uint8_t	g_bChannel;

extern bool	map_allow_find(int32_t index);
extern void	map_allow_copy(int32_t * pl, int32_t size);
extern bool	no_wander;

extern int32_t		g_iUserLimit;
extern time_t	g_global_time;

const char *	get_table_postfix();

extern std::string	g_stHostname;
extern std::string	g_stLocale;
extern std::string	g_stLocaleFilename;

extern char		g_szPublicIP[16];
extern char		g_szInternalIP[16];

extern int32_t (*is_twobyte) (const char * str);
extern int32_t (*check_name) (const char * str);

extern bool		g_bSkillDisable;

extern int32_t		g_iFullUserCount;
extern int32_t		g_iBusyUserCount;
extern void		LoadStateUserCount();

extern bool	g_bEmpireWhisper;

extern uint8_t	g_bAuthServer;
extern uint8_t	g_bBilling;

extern uint8_t	PK_PROTECT_LEVEL;

extern void	LoadValidCRCList();
extern bool	IsValidProcessCRC(uint32_t dwCRC);
extern bool	IsValidFileCRC(uint32_t dwCRC);

extern std::string	g_stAuthMasterIP;
extern uint16_t		g_wAuthMasterPort;

extern std::string	g_stClientVersion;
extern bool		g_bCheckClientVersion;
extern void		CheckClientVersion();

extern std::string	g_stQuestDir;
extern std::set<std::string> g_setQuestObjectDir;
extern std::string g_stServiceBasePath;
extern std::string g_stServiceMapPath;
extern std::string g_stServiceName;

extern std::vector<std::string>	g_stAdminPageIP;
extern std::string	g_stAdminPagePassword;

extern int32_t	SPEEDHACK_LIMIT_COUNT;
extern int32_t 	SPEEDHACK_LIMIT_BONUS;

extern int32_t g_iSyncHackLimitCount;

extern std::string g_strWebMallURL;

extern int32_t VIEW_RANGE;
extern int32_t VIEW_BONUS_RANGE;

extern bool g_bCheckMultiHack;
extern bool g_protectNormalPlayer;
extern bool g_noticeBattleZone;

extern uint32_t g_GoldDropTimeLimitValue;

extern bool isHackShieldEnable;
extern int32_t  HackShield_FirstCheckWaitTime;
extern int32_t  HackShield_CheckCycleTime;
extern bool bXTrapEnabled;

extern int32_t gPlayerMaxLevel;

extern bool g_BlockCharCreation;

#ifdef __AUCTION__
extern int32_t auction_server;
#endif
extern int32_t speed_server;
extern int32_t openid_server;
extern bool	g_bNoPasspod;
extern std::string g_stBlockDate;

extern int32_t g_iSpamBlockMaxLevel;
extern uint32_t g_uiSpamBlockScore;
extern uint32_t g_uiSpamBlockDuration;
extern uint32_t g_uiSpamReloadCycle;

extern void map_allow_log();

extern bool	g_bEmpireShopPriceTripleDisable; // @correction206
extern bool g_bGlobalShoutEnable; // @correction207
extern bool g_bDisablePrismNeed; // @correction208
extern bool g_bDisableEmotionMask; // @correction209
extern uint16_t g_wItemCountLimit; // @correction002
extern uint32_t g_dwItemBonusChangeTime; // @correction210
extern bool	g_bEnableBootaryCheck; // @correction211
extern bool	g_bGMHostCheck; // @correction212
extern bool	g_bGuildInviteLimit; // @correction213
extern bool	g_bGuildInfiniteMembers; // @correction214
extern bool	g_bChinaIntoxicationCheck; // @correction215
extern bool	g_bEnableSpeedHackCrash; // @correction216
extern int32_t g_iStatusPointGetLevelLimit; // @correction217
extern int32_t g_iStatusPointSetMaxValue; // @correction218
extern int32_t g_iShoutLimitLevel; // @correction219
extern uint32_t g_dwSkillBookNextReadMin; // @correction220
extern uint32_t g_dwSkillBookNextReadMax; // @correction220
extern int32_t g_iShoutLimitTime; // @correction221
extern int32_t g_iDbLogLevel; // @correction222
extern int32_t g_iSysLogLevel; // @correction223
// @correction224 BEGIN
enum ItemDestroyTime {ITEM_DESTROY_TIME_AUTOGIVE, ITEM_DESTROY_TIME_DROPGOLD, ITEM_DESTROY_TIME_DROPITEM, ITEM_DESTROY_TIME_MAX};
extern int32_t g_aiItemDestroyTime[ITEM_DESTROY_TIME_MAX];
// @correction224 END
extern uint32_t g_ItemDropTimeLimitValue; // @correction225
extern uint32_t g_BoxUseTimeLimitValue; // @correction226
extern uint32_t g_BuySellTimeLimitValue; // @correction227
extern bool g_NoDropMetinStone; // @correction228
extern bool g_NoMountAtGuildWar; // @correction229
extern bool g_NoPotionsOnPVP; // @correction230
extern bool gHackCheckEnable; // @correction120
extern std::string g_stProxyIP; // @correction123
// @correction138 BEGIN
extern int32_t gShutdownAge;
extern int32_t gShutdownEnable;
// @correction138 END
extern uint64_t GOLD_MAX;
extern uint32_t g_dwMaxDrop;
extern uint32_t g_dwStartLevel;