#include "stdafx.h"
#include <sstream>
#ifndef _WIN32
#include <ifaddrs.h>
#endif

#include "constants.h"
#include "utils.h"
#include "log.h"
#include "desc.h"
#include "desc_manager.h"
#include "item_manager.h"
#include "p2p.h"
#include "char.h"
#include "ip_ban.h"
#include "war_map.h"
#include "locale_service.h"
#include "config.h"
#include "dev_log.h"
#include "db.h"
#include "skill_power.h"

using std::string;

uint8_t	g_bChannel = 0;
uint16_t	mother_port = 50080;
int32_t		passes_per_sec = 25;
uint16_t	db_port = 0;
uint16_t	p2p_port = 50900;
char	db_addr[ADDRESS_MAX_LEN + 1];
int32_t		save_event_second_cycle = passes_per_sec * 120;
int32_t		ping_event_second_cycle = passes_per_sec * 60;
bool	g_bNoMoreClient = false;
bool	g_bNoRegen = false;
bool	g_bNoPasspod = false;

bool		g_bTrafficProfileOn = false;
uint32_t		g_dwTrafficProfileFlushCycle = 3600;

int32_t			test_server = 0;
int32_t			speed_server = 0;
#ifdef __AUCTION__
int32_t			auction_server = 0;
#endif
bool		distribution_test_server = false;
bool		china_event_server = false;
bool		guild_mark_server = true;
uint8_t		guild_mark_min_level = 3;
bool		no_wander = false;
int32_t			g_iUserLimit = 32768;

char		g_szPublicIP[16] = "0";
char		g_szInternalIP[16] = "0";
bool		g_bSkillDisable = false;
int32_t			g_iFullUserCount = 500;
int32_t			g_iBusyUserCount = 100;
bool		g_bEmpireWhisper = true;
uint8_t		g_bAuthServer = false;

bool		g_bCheckClientVersion = true;
string	g_stClientVersion = "1215955205";

uint8_t		g_bBilling = false;

string	g_stAuthMasterIP;
uint16_t		g_wAuthMasterPort = 0;

std::set<uint32_t> s_set_dwFileCRC = {};
std::set<uint32_t> s_set_dwProcessCRC = {};

string g_stHostname = "";
string g_table_postfix = "";

string g_stQuestDir = "./quest";
string g_stDefaultQuestObjectDir = "./quest/object";
std::set<string> g_setQuestObjectDir;
std::string g_stServiceName;
std::string g_stServiceBasePath;
std::string g_stServiceMapPath;
std::string g_stLocaleFilename;

std::vector<std::string>	g_stAdminPageIP;
std::string	g_stAdminPagePassword = "SHOWMETHEMONEY";

string g_stBlockDate = "30000705";

std::string g_stLocale;

char	teen_addr[ADDRESS_MAX_LEN + 1] = {0};
uint16_t	teen_port	= 0;

int32_t SPEEDHACK_LIMIT_COUNT   = 50;
int32_t SPEEDHACK_LIMIT_BONUS   = 80;
int32_t g_iSyncHackLimitCount = 20;

int32_t VIEW_RANGE = 5000;
int32_t VIEW_BONUS_RANGE = 500;

string g_strWebMallURL = "www.metin2.de";

uint32_t g_uiSpamBlockDuration = 60 * 15;
uint32_t g_uiSpamBlockScore = 100;
uint32_t g_uiSpamReloadCycle = 60 * 10;

bool	g_bCheckMultiHack = true;

int32_t		g_iSpamBlockMaxLevel = 10;

void	LoadStateUserCount();
void	LoadValidCRCList();
bool	LoadClientVersion();
bool	g_protectNormalPlayer   = false;
bool	g_noticeBattleZone      = false;

bool	isHackShieldEnable = false;
int32_t		HackShield_FirstCheckWaitTime = passes_per_sec * 30;
int32_t		HackShield_CheckCycleTime = passes_per_sec * 180;

bool		bXTrapEnabled = false;

int32_t		gPlayerMaxLevel = 99;
bool	g_BlockCharCreation = false;

int32_t		openid_server = 0;
char	openid_host[256];
char	openid_uri[256];

uint8_t PK_PROTECT_LEVEL = 15;

bool	g_bEmpireShopPriceTripleDisable = false; // @correction206
bool	g_bGlobalShoutEnable = false; // @correction207
bool	g_bDisablePrismNeed = false; // @correction208
bool	g_bDisableEmotionMask = false; // @correction209
uint16_t	g_wItemCountLimit = 200; // @correction002
uint32_t	g_dwItemBonusChangeTime = 60; // @correction210
bool	g_bEnableBootaryCheck = false; // @correction211
bool	g_bGMHostCheck = false; // @correction212
bool	g_bGuildInviteLimit = false; // @correction213
bool	g_bGuildInfiniteMembers = false; // @correction214
bool	g_bChinaIntoxicationCheck = false; // @correction215
bool	g_bEnableSpeedHackCrash = false; // @correction216
int32_t		g_iStatusPointGetLevelLimit = 90; // @correction217
int32_t		g_iStatusPointSetMaxValue = 90; // @correction218
int32_t		g_iShoutLimitLevel = 15; // @correction219
uint32_t	g_dwSkillBookNextReadMin = 28800; // @correction220
uint32_t	g_dwSkillBookNextReadMax = 43200; // @correction220
int32_t		g_iShoutLimitTime = 15; // @correction221
int32_t		g_iDbLogLevel = LOG_LEVEL_MAX; // @correction222
int32_t		g_iSysLogLevel = LOG_LEVEL_MAX; // @correction223
int32_t		g_aiItemDestroyTime[ITEM_DESTROY_TIME_MAX] = {300, 150, 300}; // @correction224
uint32_t	g_ItemDropTimeLimitValue = 0; // @correction225
uint32_t	g_BoxUseTimeLimitValue = 0; // @correction226
uint32_t	g_BuySellTimeLimitValue = 0; // @correction227
bool	g_NoDropMetinStone = false; // @correction228
bool	g_NoMountAtGuildWar = false; // @correction229
bool	g_NoPotionsOnPVP = false; // @correction230
bool	gHackCheckEnable = false; // @correction120
std::string	g_stProxyIP = ""; // @correction123
// @correction138 BEGIN
int32_t		gShutdownAge = 0;
int32_t		gShutdownEnable = 0;
// @correction138 END
uint64_t GOLD_MAX = 0xffffffffffffffffULL;
uint32_t g_dwMaxDrop = 1000;
uint32_t g_dwStartLevel = 1;

bool is_string_true(const char * string)
{
	bool	result = 0;
	if (isnhdigit(*string))
	{
		str_to_number(result, string);
		return result > 0 ? true : false;
	}
	else if (LOWER(*string) == 't')
		return true;
	else
		return false;
}

std::set<int32_t> s_set_map_allows = {};

bool map_allow_find(int32_t index)
{
	if (g_bAuthServer)
		return false;

	if (s_set_map_allows.find(index) == s_set_map_allows.end())
		return false;

	return true;
}

void map_allow_log()
{
	std::set<int32_t>::iterator i;

	for (i = s_set_map_allows.begin(); i != s_set_map_allows.end(); ++i)
		sys_log(0, "MAP_ALLOW: %d", *i);
}

void map_allow_add(int32_t index)
{
	if (map_allow_find(index) == true)
	{
		fprintf(stderr, "!!! FATAL ERROR !!! multiple MAP_ALLOW setting!!\n");
		exit(1);
	}

	fprintf(stderr, "MAP ALLOW %d\n", index);
	s_set_map_allows.insert(index);
}

void map_allow_copy(int32_t * pl, int32_t size)
{
	int32_t iCount = 0;
	std::set<int32_t>::iterator it = s_set_map_allows.begin();

	while (it != s_set_map_allows.end())
	{
		int32_t i = *(it++);
		*(pl++) = i;

		if (++iCount > size)
			break;
	}
}

void FN_add_adminpageIP(char *line)
{
	char *last;
	const char *delim = " \t\r\n";
	char *v = strtok_r(line, delim, &last);

	while (v)
	{
		g_stAdminPageIP.push_back(v);
		v = strtok_r(nullptr, delim, &last);
	}
}

void FN_log_adminpage()
{
	itertype(g_stAdminPageIP) iter = g_stAdminPageIP.begin();

	while (iter != g_stAdminPageIP.end())
	{
		dev_log(LOG_DEB0, "ADMIN_PAGE_IP = %s", (*iter).c_str());
		++iter;
	}

	dev_log(LOG_DEB0, "ADMIN_PAGE_PASSWORD = %s", g_stAdminPagePassword.c_str());
}

bool GetIPInfo()
{
#ifndef _WIN32
	struct ifaddrs* ifaddrp = nullptr;

	if (0 != getifaddrs(&ifaddrp))
		return false;

	for( struct ifaddrs* ifap=ifaddrp ; nullptr != ifap ; ifap = ifap->ifa_next )
	{
		struct sockaddr_in * sai = (struct sockaddr_in *) ifap->ifa_addr;

		if (!ifap->ifa_netmask ||
				sai->sin_addr.s_addr == 0 ||
				sai->sin_addr.s_addr == 16777343)
			continue;
#else
	WSADATA wsa_data;
	char host_name[100];
	HOSTENT* host_ent;
	int32_t n = 0;

	if (WSAStartup(0x0101, &wsa_data)) {
		return false;
	}

	gethostname(host_name, sizeof(host_name));
	host_ent = gethostbyname(host_name);
	if (host_ent == nullptr) {
		return false;
	}
	for ( ; host_ent->h_addr_list[n] != nullptr; ++n) {
		struct sockaddr_in addr;
		struct sockaddr_in* sai = &addr;
		memcpy(&sai->sin_addr.s_addr, host_ent->h_addr_list[n], host_ent->h_length);
#endif

		char * netip = inet_ntoa(sai->sin_addr);

		if (!strncmp(netip, "192.168", 7))
		{
			strlcpy(g_szInternalIP, netip, sizeof(g_szInternalIP));
#ifndef _WIN32
			fprintf(stderr, "INTERNAL_IP: %s interface %s\n", netip, ifap->ifa_name);
#else
			fprintf(stderr, "INTERNAL_IP: %s\n", netip);
#endif
		}
		else if (!strncmp(netip, "10.", 3))
		{
			strlcpy(g_szInternalIP, netip, sizeof(g_szInternalIP));
#ifndef _WIN32
			fprintf(stderr, "INTERNAL_IP: %s interface %s\n", netip, ifap->ifa_name);
#else
			fprintf(stderr, "INTERNAL_IP: %s\n", netip);
#endif
		}
		else if (g_szPublicIP[0] == '0')
		{
			strlcpy(g_szPublicIP, netip, sizeof(g_szPublicIP));
#ifndef _WIN32
			fprintf(stderr, "PUBLIC_IP: %s interface %s\n", netip, ifap->ifa_name);
#else
			fprintf(stderr, "PUBLIC_IP: %s\n", netip);
#endif
		}
	}

#ifndef _WIN32
	freeifaddrs( ifaddrp );
#else
	WSACleanup();
#endif

	if (g_szPublicIP[0] != '0')
		return true;
	else
	{
#ifdef ENABLE_AUTODETECT_INTERNAL_IP // @correction151
		if (g_szInternalIP[0] == '0')
			return false;
		else
		{
			strlcpy(g_szPublicIP, g_szInternalIP, sizeof(g_szPublicIP));
			fprintf(stderr, "INTERNAL_IP -> PUBLIC_IP: %s\n", g_szPublicIP);
			return true;
		}
#else
		return false;
#endif
	}
}

bool __LoadConnectConfigFile(const char* configName)
{
	char buf[256] = {};
	char token_string[256] = {};
	char value_string[256] = {};

	char db_host[2][64] = {}, db_user[2][64] = {}, db_pwd[2][64] = {}, db_db[2][64] = {};
	int32_t mysql_db_port[2] = {};

	char log_host[64] = {}, log_user[64] = {}, log_pwd[64] = {}, log_db[64] = {};
	int32_t log_port = 0;

	bool isCommonSQL = false;
	bool isPlayerSQL = false;

	FILE* fpOnlyForDB;

	if (!(fpOnlyForDB = fopen(configName, "r")))
	{
		fprintf(stderr, "Can not open [%s]\n", configName);
		exit(1);
	}

	while (fgets(buf, 256, fpOnlyForDB))
	{
		parse_token(buf, token_string, value_string);

		TOKEN("player_sql")
		{
			const char * line = two_arguments(value_string, db_host[0], sizeof(db_host[0]), db_user[0], sizeof(db_user[0]));
			line = two_arguments(line, db_pwd[0], sizeof(db_pwd[0]), db_db[0], sizeof(db_db[0]));

			if ('\0' != line[0])
			{
				char buf[256];
				one_argument(line, buf, sizeof(buf));
				str_to_number(mysql_db_port[0], buf);
			}

			if (!*db_host[0] || !*db_user[0] || !*db_pwd[0] || !*db_db[0])
			{
				fprintf(stderr, "PLAYER_SQL syntax: logsql <host user password db>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "PLAYER_SQL: %s %s %s %s %d", db_host[0], db_user[0], db_pwd[0], db_db[0], mysql_db_port[0]);
			isPlayerSQL = true;
			continue;
		}

		TOKEN("common_sql")
		{
			const char * line = two_arguments(value_string, db_host[1], sizeof(db_host[1]), db_user[1], sizeof(db_user[1]));
			line = two_arguments(line, db_pwd[1], sizeof(db_pwd[1]), db_db[1], sizeof(db_db[1]));

			if ('\0' != line[0])
			{
				char buf[256];
				one_argument(line, buf, sizeof(buf));
				str_to_number(mysql_db_port[1], buf);
			}

			if (!*db_host[1] || !*db_user[1] || !*db_pwd[1] || !*db_db[1])
			{
				fprintf(stderr, "COMMON_SQL syntax: logsql <host user password db>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "COMMON_SQL: %s %s %s %s %d", db_host[1], db_user[1], db_pwd[1], db_db[1], mysql_db_port[1]);
			isCommonSQL = true;
			continue;
		}

		TOKEN("log_sql")
		{
			const char * line = two_arguments(value_string, log_host, sizeof(log_host), log_user, sizeof(log_user));
			line = two_arguments(line, log_pwd, sizeof(log_pwd), log_db, sizeof(log_db));

			if ('\0' != line[0])
			{
				char buf[256];
				one_argument(line, buf, sizeof(buf));
				str_to_number(log_port, buf);
			}

			if (!*log_host || !*log_user || !*log_pwd || !*log_db)
			{
				fprintf(stderr, "LOG_SQL syntax: logsql <host user password db>\n");
				exit(1);
			}

			char buf[1024];
			snprintf(buf, sizeof(buf), "LOG_SQL: %s %s %s %s %d", log_host, log_user, log_pwd, log_db, log_port);
			continue;
		}
	}

	fclose(fpOnlyForDB);

	if (!isCommonSQL)
	{
		puts("LOAD_COMMON_SQL_INFO_FAILURE:");
		puts("");
		puts("CONFIG:");
		puts("------------------------------------------------");
		puts("COMMON_SQL: HOST USER PASSWORD DATABASE");
		puts("");
		exit(1);
	}

	if (!isPlayerSQL)
	{
		puts("LOAD_PLAYER_SQL_INFO_FAILURE:");
		puts("");
		puts("CONFIG:");
		puts("------------------------------------------------");
		puts("PLAYER_SQL: HOST USER PASSWORD DATABASE");
		puts("");
		exit(1);
	}

	AccountDB::instance().Connect(db_host[1], mysql_db_port[1], db_user[1], db_pwd[1], db_db[1]);

	if (false == AccountDB::instance().IsConnected())
	{
		fprintf(stderr, "cannot start server while no common sql connected\n");
		exit(1);
	}

	fprintf(stderr, "CommonSQL connected\n");

	if (!LocaleService_Init())
	{
		fprintf(stderr, "invalid locale %s\n", g_stServiceName.c_str());
		exit(1);
	}

	fprintf(stderr, "Setting DB to locale %s\n", g_stLocale.c_str());

	AccountDB::instance().SetLocale(g_stLocale);

	AccountDB::instance().ConnectAsync(db_host[1], mysql_db_port[1], db_user[1], db_pwd[1], db_db[1], g_stLocale.c_str());

#ifdef ENABLE_COMMON_CONFIGS // @correction150
	if (g_bAuthServer)
		memcpy(db_db[0], "account", sizeof("account"));
#endif
	DBManager::instance().Connect(db_host[0], mysql_db_port[0], db_user[0], db_pwd[0], db_db[0]);

	if (!DBManager::instance().IsConnected())
	{
		fprintf(stderr, "PlayerSQL.ConnectError\n");
		exit(1);
	}

	fprintf(stderr, "PlayerSQL connected\n");

	if (false == g_bAuthServer)
	{
		LogManager::instance().Connect(log_host, log_port, log_user, log_pwd, log_db);

		if (!LogManager::instance().IsConnected())
		{
			fprintf(stderr, "LogSQL.ConnectError\n");
			exit(1);
		}

		fprintf(stderr, "LogSQL connected\n");

		LogManager::instance().BootLog(g_stHostname.c_str(), g_bChannel);
	}

	{
		char szQuery[256];
		snprintf(szQuery, sizeof(szQuery), "SELECT mValue FROM locale WHERE mKey='SKILL_POWER_BY_LEVEL'");
		std::unique_ptr<SQLMsg> pMsg(AccountDB::instance().DirectQuery(szQuery));

		if (pMsg->Get()->uiNumRows == 0)
		{
			fprintf(stderr, "[SKILL_PERCENT] Query failed: %s", szQuery);
			exit(1);
		}

		MYSQL_ROW row;

		row = mysql_fetch_row(pMsg->Get()->pSQLResult);

		const char * p = row[0];
		int32_t cnt = 0;
		char num[128];
		int32_t aiBaseSkillPowerByLevelTable[SKILL_MAX_LEVEL+1];

		fprintf(stderr, "SKILL_POWER_BY_LEVEL %s\n", p);
		while (*p != '\0' && cnt < (SKILL_MAX_LEVEL + 1))
		{
			p = one_argument(p, num, sizeof(num));
			aiBaseSkillPowerByLevelTable[cnt++] = atoi(num);

			if (*p == '\0')
			{
				if (cnt != (SKILL_MAX_LEVEL + 1))
				{
					fprintf(stderr, "[SKILL_PERCENT] locale table has not enough skill information! (count: %d query: %s)", cnt, szQuery);
					exit(1);
				}

				fprintf(stderr, "SKILL_POWER_BY_LEVEL: Done! (count %d)\n", cnt);
				break;
			}
		}

		for (int32_t job = 0; job < JOB_MAX_NUM * 2; ++job)
		{
			snprintf(szQuery, sizeof(szQuery), "SELECT mValue from locale where mKey='SKILL_POWER_BY_LEVEL_TYPE%d' ORDER BY CAST(mValue AS unsigned)", job);
			std::unique_ptr<SQLMsg> pMsg(AccountDB::instance().DirectQuery(szQuery));

			if (pMsg->Get()->uiNumRows == 0)
			{
				CTableBySkill::instance().SetSkillPowerByLevelFromType(job, aiBaseSkillPowerByLevelTable);
				continue;
			}

			row = mysql_fetch_row(pMsg->Get()->pSQLResult);
			cnt = 0;
			p = row[0];
			int32_t aiSkillTable[SKILL_MAX_LEVEL + 1];

			fprintf(stderr, "SKILL_POWER_BY_JOB %d %s\n", job, p);
			while (*p != '\0' && cnt < (SKILL_MAX_LEVEL + 1))
			{
				p = one_argument(p, num, sizeof(num));
				aiSkillTable[cnt++] = atoi(num);

				if (*p == '\0')
				{
					if (cnt != (SKILL_MAX_LEVEL + 1))
					{
						fprintf(stderr, "[SKILL_PERCENT] locale table has not enough skill information! (count: %d query: %s)", cnt, szQuery);
						exit(1);
					}

					fprintf(stderr, "SKILL_POWER_BY_JOB: Done! (job: %d count: %d)\n", job, cnt);
					break;
				}
			}

			CTableBySkill::instance().SetSkillPowerByLevelFromType(job, aiSkillTable);
		}
	}

	log_set_expiration_days(2);
	return true;
}

bool __LoadDefaultConfigFile(const char* configName)
{
	FILE *fp;

	char buf[256] = {};
	char token_string[256] = {};
	char value_string[256] = {};

	if (!(fp = fopen(configName, "r")))
		return false;

	while (fgets(buf, 256, fp))
	{
		parse_token(buf, token_string, value_string);

		TOKEN("hostname")
		{
			g_stHostname = value_string;
			fprintf(stderr, "HOSTNAME: %s\n", g_stHostname.c_str());
			continue;
		}

		TOKEN("channel")
		{
			str_to_number(g_bChannel, value_string);
			continue;
		}

		TOKEN("port")
		{
			str_to_number(mother_port, value_string);
			continue;
		}

		TOKEN("p2p_port")
		{
			str_to_number(p2p_port, value_string);
			continue;
		}

		TOKEN("map_allow")
		{
			char * p = value_string;
			string stNum;

			for (; *p; p++)
			{
				if (isnhspace(*p))
				{
					if (stNum.length())
					{
						int32_t	index = 0;
						str_to_number(index, stNum.c_str());
						map_allow_add(index);
						stNum.clear();
					}
				}
				else
					stNum += *p;
			}

			if (stNum.length())
		{
				int32_t	index = 0;
				str_to_number(index, stNum.c_str());
				map_allow_add(index);
			}

			continue;
		}

		TOKEN("auth_server")
		{
			char szIP[32];
			char szPort[32];

			two_arguments(value_string, szIP, sizeof(szIP), szPort, sizeof(szPort));

			if (!*szIP || (!*szPort && strcasecmp(szIP, "master")))
			{
				fprintf(stderr, "AUTH_SERVER: syntax error: <ip|master> <port>\n");
				exit(1);
			}

			g_bAuthServer = true;

			LoadBanIP("BANIP");

			if (!strcasecmp(szIP, "master"))
				fprintf(stderr, "AUTH_SERVER: I am the master\n");
			else
		{
				g_stAuthMasterIP = szIP;
				str_to_number(g_wAuthMasterPort, szPort);

				fprintf(stderr, "AUTH_SERVER: master %s %u\n", g_stAuthMasterIP.c_str(), g_wAuthMasterPort);
			}
			continue;
		}

		TOKEN("teen_addr")
		{
			strlcpy(teen_addr, value_string, sizeof(teen_addr));

			for (int32_t n =0; n < ADDRESS_MAX_LEN; ++n)
		{
				if (teen_addr[n] == ' ')
					teen_addr[n] = '\0';
			}

			continue;
		}

		TOKEN("teen_port")
		{
			str_to_number(teen_port, value_string);
		}

	}

	fclose(fp);
	return true;
}

bool __LoadGeneralConfigFile(const char* configName)
{
	FILE *fp;

	char buf[256] = {};
	char token_string[256] = {};
	char value_string[256] = {};

	if (!(fp = fopen(configName, "r")))
		return false;

	while (fgets(buf, 256, fp))
	{
		parse_token(buf, token_string, value_string);

		TOKEN("WEB_AUTH")
		{
			two_arguments(value_string, openid_host, sizeof(openid_host), openid_uri, sizeof(openid_uri));

			if (!*openid_host || !*openid_uri)
			{
				fprintf(stderr, "WEB_AUTH syntax error (ex: WEB_AUTH <host(metin2.co.kr) uri(/kyw/gameauth.php)>\n");
				exit(1);
			}

			char buf[1024];
			openid_server = 1;
			snprintf(buf, sizeof(buf), "WEB_AUTH: %s %s", openid_host, openid_uri);
			continue;
		}

		TOKEN("BLOCK_LOGIN")
		{
			g_stBlockDate = value_string;
		}

		TOKEN("adminpage_ip")
		{
			FN_add_adminpageIP(value_string);
		}

		TOKEN("adminpage_ip1")
		{
			FN_add_adminpageIP(value_string);
		}

		TOKEN("adminpage_ip2")
		{
			FN_add_adminpageIP(value_string);
		}

		TOKEN("adminpage_ip3")
		{
			FN_add_adminpageIP(value_string);
		}

		TOKEN("adminpage_password")
		{
			g_stAdminPagePassword = value_string;
		}

		TOKEN("db_port")
		{
			str_to_number(db_port, value_string);
			continue;
		}

		TOKEN("db_addr")
		{
			strlcpy(db_addr, value_string, sizeof(db_addr));

			for (int32_t n =0; n < ADDRESS_MAX_LEN; ++n)
			{
				if (db_addr[n] == ' ')
					db_addr[n] = '\0';
			}

			continue;
		}

		TOKEN("empire_whisper")
		{
			bool b_value = 0;
			str_to_number(b_value, value_string);
			g_bEmpireWhisper = !!b_value;
			continue;
		}

		TOKEN("mark_server")
		{
			guild_mark_server = is_string_true(value_string);
			continue;
		}

		TOKEN("mark_min_level")
		{
			str_to_number(guild_mark_min_level, value_string);
			guild_mark_min_level = MINMAX(0, guild_mark_min_level, GUILD_MAX_LEVEL);
			continue;
		}

		TOKEN("log_keep_days")
		{
			int32_t i = 0;
			str_to_number(i, value_string);
			log_set_expiration_days(MINMAX(1, i, 90));
			continue;
		}

		TOKEN("passes_per_sec")
		{
			str_to_number(passes_per_sec, value_string);
			continue;
		}

		TOKEN("save_event_second_cycle")
		{
			int32_t	cycle = 0;
			str_to_number(cycle, value_string);
			save_event_second_cycle = cycle * passes_per_sec;
			continue;
		}

		TOKEN("ping_event_second_cycle")
		{
			int32_t	cycle = 0;
			str_to_number(cycle, value_string);
			ping_event_second_cycle = cycle * passes_per_sec;
			continue;
		}

		TOKEN("table_postfix")
		{
			g_table_postfix = value_string;
			continue;
		}

		TOKEN("test_server")
		{
			str_to_number(test_server, value_string);
			fprintf(stderr, "-----------------------------------------------\n");
			fprintf(stderr, "TEST_SERVER %d\n", test_server);
			fprintf(stderr, "-----------------------------------------------\n");
			continue;
		}

		TOKEN("speed_server")
		{
			fprintf(stderr, "-----------------------------------------------\n");
			fprintf(stderr, "SPEED_SERVER\n");
			fprintf(stderr, "-----------------------------------------------\n");
			str_to_number(speed_server, value_string);
			continue;
		}
#ifdef __AUCTION__
		TOKEN("auction_server")
		{
			fprintf(stderr, "-----------------------------------------------\n");
			fprintf(stderr, "AUCTION_SERVER\n");
			fprintf(stderr, "-----------------------------------------------\n");
			str_to_number(auction_server, value_string);
			continue;
		}
#endif
		TOKEN("distribution_test_server")
		{
			str_to_number(distribution_test_server, value_string);
			continue;
		}

		TOKEN("china_event_server")
		{
			str_to_number(china_event_server, value_string);
			continue;
		}

		TOKEN("shutdowned")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bNoMoreClient = !!flag;
			continue;
		}

		TOKEN("no_regen")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bNoRegen = !!flag;
			continue;
		}

		TOKEN("traffic_profile")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bTrafficProfileOn = !!flag;
			continue;
		}

		TOKEN("no_wander")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			no_wander = !!flag;
			continue;
		}

		TOKEN("user_limit")
		{
			str_to_number(g_iUserLimit, value_string);
			continue;
		}

		TOKEN("skill_disable")
		{
			str_to_number(g_bSkillDisable, value_string);
			continue;
		}

		TOKEN("billing")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bBilling = !!flag;
		}

		TOKEN("quest_dir")
		{
			sys_log(0, "QUEST_DIR SETTING : %s", value_string);
			g_stQuestDir = value_string;
		}

		TOKEN("quest_object_dir")
		{
			std::istringstream is(value_string);
			sys_log(0, "QUEST_OBJECT_DIR SETTING : %s", value_string);
			string dir;
			while (!is.eof())
			{
				is >> dir;
				if (is.fail())
					break;
				g_setQuestObjectDir.insert(dir);
				sys_log(0, "QUEST_OBJECT_DIR INSERT : %s", dir .c_str());
			}
		}

		TOKEN("service_base_dir")
		{
			sys_log(0, "SERVICE_BASE_DIR SETTING : %s", value_string);
			g_stServiceBasePath = value_string;
		}

		TOKEN("service_map_dir")
		{
			sys_log(0, "SERVICE_MAP_DIR SETTING : %s", value_string);
			g_stServiceMapPath = value_string;
		}

		TOKEN("locale_file_dir")
		{
			sys_log(0, "LOCALE_FILE_DIR SETTING : %s", value_string);
			g_stLocaleFilename = value_string;
		}

		TOKEN("locale_service")
		{
			sys_log(0, "LOCALE_SERVICE SETTING : %s", value_string);
			g_stServiceName = value_string;
		}

		TOKEN("locale")
		{
			sys_log(0, "LOCALE SETTING : %s", value_string);
			g_stLocale = value_string;
		}

		TOKEN("synchack_limit_count")
		{
			str_to_number(g_iSyncHackLimitCount, value_string);
		}

		TOKEN("speedhack_limit_count")
		{
			str_to_number(SPEEDHACK_LIMIT_COUNT, value_string);
		}

		TOKEN("speedhack_limit_bonus")
		{
			str_to_number(SPEEDHACK_LIMIT_BONUS, value_string);
		}

		TOKEN("mall_url")
		{
			g_strWebMallURL = value_string;
		}

		TOKEN("bind_ip")
		{
			strlcpy(g_szPublicIP, value_string, sizeof(g_szPublicIP));
		}

		TOKEN("internal_ip")
		{
			strlcpy(g_szInternalIP, value_string, sizeof(g_szInternalIP));
		}

		TOKEN("view_range")
		{
			str_to_number(VIEW_RANGE, value_string);
		}

		TOKEN("spam_block_duration")
		{
			str_to_number(g_uiSpamBlockDuration, value_string);
		}

		TOKEN("spam_block_score")
		{
			str_to_number(g_uiSpamBlockScore, value_string);
			g_uiSpamBlockScore = MAX(1, g_uiSpamBlockScore);
		}

		TOKEN("spam_block_reload_cycle")
		{
			str_to_number(g_uiSpamReloadCycle, value_string);
			g_uiSpamReloadCycle = MAX(60, g_uiSpamReloadCycle);
		}

		TOKEN("check_multihack")
		{
			str_to_number(g_bCheckMultiHack, value_string);
		}

		TOKEN("spam_block_max_level")
		{
			str_to_number(g_iSpamBlockMaxLevel, value_string);
		}

		TOKEN("protect_normal_player")
		{
			str_to_number(g_protectNormalPlayer, value_string);
		}

		TOKEN("notice_battle_zone")
		{
			str_to_number(g_noticeBattleZone, value_string);
		}

		TOKEN("hackshield_enable")
		{
			int32_t flag = 0;

			str_to_number(flag, value_string);

			if (1 == flag)
			{
				isHackShieldEnable = true;
			}
		}

		TOKEN("hackshield_first_check_time")
		{
			int32_t secs = 30;
			str_to_number(secs, value_string);

			HackShield_FirstCheckWaitTime = passes_per_sec * secs;
		}

		TOKEN("hackshield_check_cycle_time")
		{
			int32_t secs = 180;
			str_to_number(secs, value_string);

			HackShield_CheckCycleTime = passes_per_sec * secs;
		}

		TOKEN("xtrap_enable")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			bXTrapEnabled = !!flag;
		}

		TOKEN("pk_protect_level")
		{
		    str_to_number(PK_PROTECT_LEVEL, value_string);
		    fprintf(stderr, "PK_PROTECT_LEVEL: %d\n", PK_PROTECT_LEVEL);
		}

		TOKEN("max_level")
		{
			str_to_number(gPlayerMaxLevel, value_string);

			gPlayerMaxLevel = MINMAX(1, gPlayerMaxLevel, PLAYER_MAX_LEVEL_CONST);

			fprintf(stderr, "PLAYER_MAX_LEVEL: %d\n", gPlayerMaxLevel);
		}

		TOKEN("block_char_creation")
		{
			int32_t tmp = 0;

			str_to_number(tmp, value_string);

			if (0 == tmp)
				g_BlockCharCreation = false;
			else
				g_BlockCharCreation = true;

			continue;
		}

		TOKEN("check_version_server")
		{
			int32_t flag = 0;

			str_to_number(flag, value_string);
			g_bCheckClientVersion = !!flag;
			fprintf(stderr, "CHECK_VERSION_SERVER: %d\n", g_bCheckClientVersion);
			continue;
		}

		TOKEN("check_version_value")
		{
			g_stClientVersion = value_string;
			fprintf(stderr, "CHECK_VERSION_VALUE: %s\n", g_stClientVersion.c_str());
			continue;
		}

// @correction002 BEGIN
		TOKEN("item_count_limit")
		{
			str_to_number(g_wItemCountLimit, value_string);
			fprintf(stderr, "ITEM_COUNT_LIMIT: %d\n", g_wItemCountLimit);
			continue;
		}
// @correction002 END

// @correction206 BEGIN
		TOKEN("shop_price_3x_tax")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bEmpireShopPriceTripleDisable = !flag;
			fprintf(stderr, "SHOP_PRICE_3X_TAX: %s\n", (!g_bEmpireShopPriceTripleDisable)?"ENABLED":"DISABLED");
			continue;
		}
// @correction206 END

// @correction207 BEGIN
		TOKEN("global_shout")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bGlobalShoutEnable = !!flag;
			fprintf(stderr, "GLOBAL_SHOUT: %s\n", (g_bGlobalShoutEnable) ? "ENABLED" : "DISABLED");
			continue;
		}
// @correction207 END

// @correction208 BEGIN
		TOKEN("prism_item_require")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bDisablePrismNeed = !flag;
			fprintf(stderr, "PRISM_ITEM_REQUIRE: %s\n", (!g_bDisablePrismNeed) ? "ENABLED" : "DISABLED");
			continue;
		}
// @correction208 END

// @correction209 BEGIN
		TOKEN("emotion_mask_require")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bDisableEmotionMask = !flag;
			fprintf(stderr, "EMOTION_MASK_REQUIRE: %s\n", (g_bDisableEmotionMask) ? "ENABLED" : "DISABLED");
			continue;
		}
// @correction209 END

// @correction210 BEGIN
		TOKEN("change_attr_time_limit")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);
			g_dwItemBonusChangeTime = flag;
			fprintf(stderr, "CHANGE_ATTR_TIME_LIMIT: %u\n", g_dwItemBonusChangeTime);
			continue;
		}
// @correction210 END

// @correction211 BEGIN
		TOKEN("bootary_check")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bEnableBootaryCheck = !!flag;
			fprintf(stderr, "BOOTARY_CHECK: %s\n", (g_bEnableBootaryCheck) ? "ENABLED" : "DISABLED");
			continue;
		}
// @correction211 END

// @correction212 BEGIN
		TOKEN("gm_host_check")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);

			g_bGMHostCheck = !!flag;
			fprintf(stderr, "GM_HOST_CHECK: %d\n", g_bGMHostCheck);
			continue;
		}
// @correction212 END

// @correction213 BEGIN
		TOKEN("guild_invite_limit")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);

			g_bGuildInviteLimit = !!flag;
			fprintf(stderr, "GUILD_INVITE_LIMIT: %d\n", g_bGuildInviteLimit);
			continue;
		}
// @correction213 END

// @correction214 END
		TOKEN("guild_infinite_members")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);

			g_bGuildInfiniteMembers = !!flag;
			fprintf(stderr, "GUILD_INFINITE_MEMBERS: %d\n", g_bGuildInfiniteMembers);
			continue;
		}
// @correction214 END

// @correction215 BEGIN
		TOKEN("china_intoxication_check")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bChinaIntoxicationCheck = !!flag;
			fprintf(stderr, "CHINA_INTOXICATION_CHECK: %d\n", g_bChinaIntoxicationCheck);
			continue;
		}
// @correction215 END

// @correction216 BEGIN
		TOKEN("speed_hack_crash")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_bEnableSpeedHackCrash = !!flag;
			fprintf(stderr, "SPEED_HACK_CRASH: %d\n", g_bEnableSpeedHackCrash);
			continue;
		}
// @correction216 END

// @correction217 BEGIN
		TOKEN("status_point_get_level_limit")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iStatusPointGetLevelLimit = MINMAX(0, flag, PLAYER_MAX_LEVEL_CONST);
			fprintf(stderr, "STATUS_POINT_GET_LEVEL_LIMIT: %d\n", g_iStatusPointGetLevelLimit);
			continue;
		}
// @correction217 END

// @correction218 BEGIN
		TOKEN("status_point_set_max_value")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iStatusPointSetMaxValue = flag;
			fprintf(stderr, "STATUS_POINT_SET_MAX_VALUE: %d\n", g_iStatusPointSetMaxValue);
			continue;
		}
// @correction218 END

// @correction219 BEGIN
		TOKEN("shout_limit_level")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iShoutLimitLevel = flag;
			fprintf(stderr, "SHOUT_LIMIT_LEVEL: %d\n", g_iShoutLimitLevel);
			continue;
		}
// @correction219 END

// @correction220 BEGIN
		TOKEN("skillbook_nextread_min")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);
			g_dwSkillBookNextReadMin = flag;
			fprintf(stderr, "SKILLBOOK_NEXTREAD_MIN: %u\n", g_dwSkillBookNextReadMin);
			continue;
		}

		TOKEN("skillbook_nextread_max")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);
			g_dwSkillBookNextReadMax = flag;
			fprintf(stderr, "SKILLBOOK_NEXTREAD_MAX: %u\n", g_dwSkillBookNextReadMax);
			continue;
		}
// @correction220 END

// @correction221 BEGIN
		TOKEN("shout_limit_time")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			if (flag <= 0) continue;

			g_iShoutLimitTime = flag;
			fprintf(stderr, "SHOUT_LIMIT_TIME: %d\n", g_iShoutLimitTime);
			continue;
		}
// @correction221 END

// @correction222 BEGIN
		TOKEN("db_log_level")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);

			g_iDbLogLevel = flag;
			fprintf(stderr, "DB_LOG_LEVEL: %d\n", g_iDbLogLevel);
			continue;
		}
// @correction222 END

// @correction223 BEGIN
		TOKEN("sys_log_level")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);

			g_iSysLogLevel = flag;
			fprintf(stderr, "SYS_LOG_LEVEL: %d\n", g_iSysLogLevel);
			continue;
		}
// @correction223 END

// @correction224 BEGIN
		TOKEN("item_destroy_time_autogive")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);

			g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE] = flag;
			fprintf(stderr, "ITEM_DESTROY_TIME_AUTOGIVE: %d\n", g_aiItemDestroyTime[ITEM_DESTROY_TIME_AUTOGIVE]);
			continue;
		}

		TOKEN("item_destroy_time_dropgold")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);

			g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPGOLD] = flag;
			fprintf(stderr, "ITEM_DESTROY_TIME_DROPGOLD: %d\n", g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPGOLD]);
			continue;
		}

		TOKEN("item_destroy_time_dropitem")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);

			g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPITEM] = flag;
			fprintf(stderr, "ITEM_DESTROY_TIME_DROPITEM: %d\n", g_aiItemDestroyTime[ITEM_DESTROY_TIME_DROPITEM]);
			continue;
		}
// @correction224 END

// @correction225 BEGIN
		TOKEN("item_drop_limit_time")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);
			g_ItemDropTimeLimitValue = flag * 1000;
			fprintf(stderr, "ITEM_DROP_LIMIT_TIME: %u\n", g_ItemDropTimeLimitValue);
			continue;
		}
// @correction225 END

// @correction226 BEGIN
		TOKEN("box_use_limit_time")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);
			g_BoxUseTimeLimitValue = flag * 1000;
			fprintf(stderr, "BOX_USE_LIMIT_TIME: %u\n", g_BoxUseTimeLimitValue);
			continue;
		}
// @correction226 END

// @correction227 BEGIN
		TOKEN("buysell_limit_time")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);
			g_BuySellTimeLimitValue = flag * 1000;
			fprintf(stderr, "BUYSELL_LIMIT_TIME: %u\n", g_BuySellTimeLimitValue);
			continue;
		}
// @correction227 END

// @correction228 BEGIN
		TOKEN("no_drop_metin_stone")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_NoDropMetinStone = !!flag;
			fprintf(stderr, "NO_DROP_METIN_STONE: %d\n", g_NoDropMetinStone);
			continue;
		}
// @correction228 END

// @correction229 BEGIN
		TOKEN("no_mount_at_guild_war")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_NoMountAtGuildWar = !!flag;
			fprintf(stderr, "NO_MOUNT_AT_GUILD_WAR: %d\n", g_NoMountAtGuildWar);
			continue;
		}
// @correction229 END

// @correction230 BEGIN
		TOKEN("no_potions_on_pvp")
		{
			int32_t flag = 0;
			str_to_number(flag, value_string);
			g_NoPotionsOnPVP = !!flag;
			fprintf(stderr, "NO_POTIONS_ON_PVP: %d\n", g_NoPotionsOnPVP);
			continue;
		}
// @correction230 END

// @correction120 BEGIN
		TOKEN("enable_hack_check")
		{
			uint32_t flag = 0;
			str_to_number(flag, value_string);

			gHackCheckEnable = !!flag;
			fprintf(stderr, "ENABLE_HACK_CHECK: %d\n", gHackCheckEnable);
			continue;
		}
// @correction120 END

// @correction123 BEGIN
		TOKEN("proxy_ip")
		{
			g_stProxyIP = value_string;
		}
// @correction123 END

// @correction138 BEGIN
		TOKEN("shutdown_age")
		{
			str_to_number(gShutdownAge, value_string);
			fprintf(stderr, "SHUTDOWN_AGE: %d\n", gShutdownAge);

		}
		TOKEN("shutdown_enable")
		{
			str_to_number(gShutdownEnable, value_string);
			fprintf(stderr, "SHUTDOWN_ENABLE: %d\n", gShutdownEnable);
		}
// @correction138 END

		TOKEN("MAX_GOLD")
		{
			str_to_number(GOLD_MAX, value_string);
			fprintf(stderr, "GOLD_MAX: %llu\n", GOLD_MAX);
		}

		TOKEN("MAX_DROP")
		{
			str_to_number(g_dwMaxDrop, value_string);
			fprintf(stderr, "MAX_DROP: %u\n", g_dwMaxDrop);
		}

		TOKEN("START_LEVEL")
		{
			str_to_number(g_dwStartLevel, value_string);
			fprintf(stderr, "START_LEVEL: %u\n", g_dwStartLevel);
		}
	}
	fclose(fp);
	return true;
}

bool __LoadDefaultCMDFile(const char* cmdName)
{
	FILE *fp;
	char buf[256] = {};

	if ((fp = fopen(cmdName, "r")))
	{
		while (fgets(buf, 256, fp))
		{
			char cmd[32], levelname[32];
			int32_t level;

			two_arguments(buf, cmd, sizeof(cmd), levelname, sizeof(levelname));

			if (!*cmd || !*levelname)
			{
#ifdef ENABLE_CMD_PLAYER // @correction147
				fprintf(stderr, "CMD syntax error: <cmd> <PLAYER | LOW_WIZARD | WIZARD | HIGH_WIZARD | GOD | IMPLEMENTOR | DISABLE>\n");
#else
				fprintf(stderr, "CMD syntax error: <cmd> <LOW_WIZARD | WIZARD | HIGH_WIZARD | GOD | IMPLEMENTOR | DISABLE>\n");
#endif
				exit(1);
			}

			if (!strcasecmp(levelname, "LOW_WIZARD"))
				level = GM_LOW_WIZARD;
			else if (!strcasecmp(levelname, "WIZARD"))
				level = GM_WIZARD;
			else if (!strcasecmp(levelname, "HIGH_WIZARD"))
				level = GM_HIGH_WIZARD;
			else if (!strcasecmp(levelname, "GOD"))
				level = GM_GOD;
			else if (!strcasecmp(levelname, "IMPLEMENTOR"))
				level = GM_IMPLEMENTOR;
#ifdef ENABLE_CMD_PLAYER // @correction147
			else if (!strcasecmp(levelname, "PLAYER"))
				level = GM_PLAYER;
#endif
			else if (!strcasecmp(levelname, "DISABLE"))
				level = GM_DISABLE; // @correction101
			else
			{
#ifdef ENABLE_CMD_PLAYER // @correction147
				fprintf(stderr, "CMD syntax error: <cmd> <PLAYER | LOW_WIZARD | WIZARD | HIGH_WIZARD | GOD | IMPLEMENTOR | DISABLE>\n");
#else
				fprintf(stderr, "CMD syntax error: <cmd> <LOW_WIZARD | WIZARD | HIGH_WIZARD | GOD | IMPLEMENTOR | DISABLE>\n");
#endif
				exit(1);
			}

			if (test_server)
				fprintf(stderr, "CMD_REWRITE: [%s] [%s:%d]\n", cmd, levelname, level);
			interpreter_set_privilege(cmd, level);
		}

		fclose(fp);
		return true;
	}
	return false;
}

#ifdef ENABLE_EXPTABLE_FROMDB // @correction148
bool __LoadExpTableFromDB(void)
{
	std::unique_ptr<SQLMsg> pMsg(AccountDB::instance().DirectQuery("SELECT level, exp FROM exp_table"));
	if (pMsg->Get()->uiNumRows == 0)
		return false;

	uint32_t new_exp_table[PLAYER_MAX_LEVEL_CONST+1] = {};
	if (exp_table != nullptr)
		memcpy(new_exp_table, exp_table, (PLAYER_MAX_LEVEL_CONST+1)*sizeof(uint32_t));

	MYSQL_ROW row = nullptr;
	while ((row = mysql_fetch_row(pMsg->Get()->pSQLResult)))
	{
		uint32_t level = 0;
		uint32_t exp = 0;
		str_to_number(level, row[0]);
		str_to_number(exp, row[1]);
		if (level > PLAYER_MAX_LEVEL_CONST)
			continue;
		new_exp_table[level] = exp;
	}
	exp_table = new_exp_table;
	return true;
}
#endif

void config_init(const string& st_localeServiceName)
{
	if (!GetIPInfo())
	{
	//	fprintf(stderr, "Can not get public ip address\n");
	//	exit(1);
	}

#ifdef ENABLE_COMMON_CONFIGS // @correction150
	if (!__LoadDefaultConfigFile("CONFIG"))
	{
		fprintf(stderr, "Can not open CONFIG\n");
		exit(1);
	}

	if (!__LoadGeneralConfigFile("/usr/game/share/conf/CONFIG") || !__LoadConnectConfigFile("/usr/game/share/conf/CONFIG"))
	{
		fprintf(stderr, "Can not open /usr/game/share/conf/CONFIG\n");
		exit(1);
	}
	else
		fprintf(stderr, "COMMON CONFIG LOAD OK /usr/game/share/conf/CONFIG\n");
#else
	if (!__LoadConnectConfigFile("CONFIG") || !__LoadDefaultConfigFile("CONFIG") || !__LoadGeneralConfigFile("CONFIG"))
	{
		fprintf(stderr, "Can not open CONFIG\n");
		exit(1);
	}
#endif

	if (g_setQuestObjectDir.empty())
		g_setQuestObjectDir.insert(g_stDefaultQuestObjectDir);

	if (0 == db_port)
	{
		fprintf(stderr, "DB_PORT not configured\n");
		exit(1);
	}

	if (0 == g_bChannel)
	{
		fprintf(stderr, "CHANNEL not configured\n");
		exit(1);
	}

	if (g_stHostname.empty())
	{
		fprintf(stderr, "HOSTNAME must be configured.\n");
		exit(1);
	}

	LocaleService_LoadLocaleStringFile();
	LocaleService_TransferDefaultSetting();
	LocaleService_LoadEmpireTextConvertTables();

#ifdef ENABLE_EXPTABLE_FROMDB // @correction148
	// if (!__LoadExpTableFromDB())
	// {
		// fprintf(stderr, "Failed to Load ExpTable from DB so exit\n");
		// exit(1);
	// }
#endif

#ifdef ENABLE_COMMON_FILES // @correction149
	std::string st_cmdFileName("/usr/game/share/conf/CMD");
#else
	std::string st_cmdFileName("CMD");
#endif
	__LoadDefaultCMDFile(st_cmdFileName.c_str());

	// if (!gHackCheckEnable) // @correction120
		// assert(test_server);

	LoadValidCRCList();
	LoadStateUserCount();

	CWarMapManager::instance().LoadWarMapInfo(nullptr);

	FN_log_adminpage();
	if (g_szPublicIP[0] == '0')
	{
		fprintf(stderr, "Can not get public ip address\n");
		exit(1);
	}
}

const char* get_table_postfix()
{
	return g_table_postfix.c_str();
}

void LoadValidCRCList()
{
	s_set_dwProcessCRC.clear();
	s_set_dwFileCRC.clear();

	FILE * fp;
	char buf[256];

#ifdef ENABLE_COMMON_FILES // @correction149
	if ((fp = fopen("/usr/game/share/conf/CRC", "r")))
#else
	if ((fp = fopen("CRC", "r")))
#endif
	{
		while (fgets(buf, 256, fp))
		{
			if (!*buf)
				continue;

			uint32_t dwValidClientProcessCRC;
			uint32_t dwValidClientFileCRC;

			sscanf(buf, " %u %u ", &dwValidClientProcessCRC, &dwValidClientFileCRC);

			s_set_dwProcessCRC.insert(dwValidClientProcessCRC);
			s_set_dwFileCRC.insert(dwValidClientFileCRC);

			fprintf(stderr, "CLIENT_CRC: %u %u\n", dwValidClientProcessCRC, dwValidClientFileCRC);
		}

		fclose(fp);
	}
}

bool LoadClientVersion()
{
#ifdef ENABLE_COMMON_FILES // @correction149
	FILE * fp = fopen("/usr/game/share/conf/VERSION", "r");
#else
	FILE * fp = fopen("VERSION", "r");
#endif
	if (!fp)
		return false;

	char buf[256];
	fgets(buf, 256, fp);

	char * p = strchr(buf, '\n');
	if (p) *p = '\0';

	fprintf(stderr, "VERSION: \"%s\"\n", buf);

	g_stClientVersion = buf;
	fclose(fp);
	return true;
}

void CheckClientVersion()
{
	// @correction005
	const DESC_MANAGER::DESC_SET & set = DESC_MANAGER::instance().GetClientSet();
	DESC_MANAGER::DESC_SET::const_iterator it = set.begin();

	while (it != set.end())
	{
		LPDESC d = *(it++);

		if (!d->GetCharacter())
			continue;

		if (0 != g_stClientVersion.compare(d->GetClientVersion())) // @correction030 (version > date)
		{
			d->GetCharacter()->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("클라이언트 버전이 틀려 로그아웃 됩니다. 정상적으로 패치 후 접속하세요."));
			d->DelayedDisconnect(10);
		}
	}
}

void LoadStateUserCount()
{
	FILE * fp = fopen("state_user_count", "r");

	if (!fp)
		return;
	// @correction005
	fscanf(fp, " %d %d ", &g_iFullUserCount, &g_iBusyUserCount);

	fclose(fp);
}

bool IsValidProcessCRC(uint32_t dwCRC)
{
	return s_set_dwProcessCRC.find(dwCRC) != s_set_dwProcessCRC.end();
}

bool IsValidFileCRC(uint32_t dwCRC)
{
	return s_set_dwFileCRC.find(dwCRC) != s_set_dwFileCRC.end();
}


