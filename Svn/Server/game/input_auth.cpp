#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "input.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "protocol.h"
#include "matrix_card.h"
#include "passpod.h"
#include "locale_service.h"
#include "auth_brazil.h"
#include "db.h"

#include "utils.h"

bool FN_IS_VALID_LOGIN_STRING(const char *str)
{
	const char*	tmp;

	if (!str || !*str)
		return false;

	if (strlen(str) < 2)
		return false;

	for (tmp = str; *tmp; ++tmp)
	{
		if (isdigit(*tmp) || isalpha(*tmp))
			continue;

#ifdef ENABLE_ACCOUNT_W_SPECIALCHARS // @correction180
		switch (*tmp)
		{
			case ' ':
			case '_':
			case '-':
			case '.':
			case '!':
			case '@':
			case '#':
			case '$':
			case '%':
			case '^':
			case '&':
			case '*':
			case '(':
			case ')':
				continue;
		}
#endif

		return false;
	}

	return true;
}

bool Login_IsInChannelService(const char* c_login)
{
	if (c_login[0] == '[')
		return true;
	return false;
}

CInputAuth::CInputAuth()
{
}

void CInputAuth::Login(LPDESC d, const char * c_pData)
{
	TPacketCGLogin3 * pinfo = (TPacketCGLogin3 *) c_pData;

	if (!g_bAuthServer)
	{
		sys_err ("CInputAuth class is not for game server. IP %s might be a hacker.",
			inet_ntoa(d->GetAddr().sin_addr));
		d->DelayedDisconnect(5);
		return;
	}

	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	char passwd[PASSWD_MAX_LEN + 1];
	strlcpy(passwd, pinfo->passwd, sizeof(passwd));

	sys_log(0, "InputAuth::Login : %s(%d) desc %p",
			login, strlen(login), get_pointer(d));

	if (false == FN_IS_VALID_LOGIN_STRING(login))
	{
		sys_log(0, "InputAuth::Login : IS_NOT_VALID_LOGIN_STRING(%s) desc %p",
				login, get_pointer(d));
		LoginFailure(d, "NOID");
		return;
	}

	if (g_bNoMoreClient)
	{
		TPacketGCLoginFailure failurePacket;

		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));

		d->Packet(&failurePacket, sizeof(failurePacket));
		return;
	}

	if (DESC_MANAGER::instance().FindByLoginName(login))
	{
		LoginFailure(d, "ALREADY");
		return;
	}

#ifdef __HARDWARE_INFOS__
	{
		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT mac_addr FROM account.block_info WHERE mac_addr = '%s'", pinfo->mac_addr));
		if (pMsg->Get()->uiNumRows)
		{
			LoginFailure(d, "BLOCK");
			return;
		}
	}
	{
		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT machine_guid FROM account.block_info WHERE machine_guid = '%s'", pinfo->machine_guid));
		if (pMsg->Get()->uiNumRows)
		{
			LoginFailure(d, "BLOCK");
			return;
		}
	}
	{
		std::unique_ptr<SQLMsg> pMsg(DBManager::instance().DirectQuery("SELECT hdd_serial FROM account.block_info WHERE hdd_serial = '%s'", pinfo->hdd_serial));
		if (pMsg->Get()->uiNumRows)
		{
			LoginFailure(d, "BLOCK");
			return;
		}
	}
#endif

	uint32_t dwKey = DESC_MANAGER::instance().CreateLoginKey(d);
	uint32_t dwPanamaKey = dwKey ^ pinfo->adwClientKey[0] ^ pinfo->adwClientKey[1] ^ pinfo->adwClientKey[2] ^ pinfo->adwClientKey[3];
	d->SetPanamaKey(dwPanamaKey);

	sys_log(0, "InputAuth::Login : key %u:0x%x login %s", dwKey, dwPanamaKey, login);

#ifdef ENABLE_BRAZIL_AUTH_FEATURE // @correction005
	if (!test_server)
	{
		int32_t result = auth_brazil(login, passwd);

		switch (result)
		{
			case AUTH_BRAZIL_SERVER_ERR:
			case AUTH_BRAZIL_NOID:
				LoginFailure(d, "NOID");
				return;
			case AUTH_BRAZIL_WRONGPWD:
				LoginFailure(d, "WRONGPWD");
				return;
			case AUTH_BRAZIL_FLASHUSER:
				LoginFailure(d, "FLASH");
				return;
		}
	}
#endif

	TPacketCGLogin3 * p = M2_NEW TPacketCGLogin3;
	memcpy(p, pinfo, sizeof(TPacketCGLogin3));

	char szPasswd[PASSWD_MAX_LEN * 2 + 1];
	DBManager::instance().EscapeString(szPasswd, sizeof(szPasswd), passwd, strlen(passwd));
	// sys_err("%s %s %s", passwd, szPasswd, mysql_hash_password(szPasswd).c_str());

	char szLogin[LOGIN_MAX_LEN * 2 + 1];
	DBManager::instance().EscapeString(szLogin, sizeof(szLogin), login, strlen(login));

	if (Login_IsInChannelService(szLogin))
	{
		sys_log(0, "ChannelServiceLogin [%s]", szLogin);

		DBManager::instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p,
				"SELECT '%s',password,securitycode,social_id,id,status,availDt - NOW() > 0,"
				"UNIX_TIMESTAMP(silver_expire),"
				"UNIX_TIMESTAMP(gold_expire),"
				"UNIX_TIMESTAMP(safebox_expire),"
				"UNIX_TIMESTAMP(autoloot_expire),"
				"UNIX_TIMESTAMP(fish_mind_expire),"
				"UNIX_TIMESTAMP(marriage_fast_expire),"
				"UNIX_TIMESTAMP(money_drop_rate_expire),"
				"UNIX_TIMESTAMP(create_time)"
				" FROM account WHERE login='%s'",

				szPasswd, szLogin);
	}
	else
	{
#ifdef _WIN32
		DBManager::instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p,
				"SELECT PASSWORD('%s'),password,securitycode,social_id,id,status,availDt - NOW() > 0,"
				"UNIX_TIMESTAMP(silver_expire),"
				"UNIX_TIMESTAMP(gold_expire),"
				"UNIX_TIMESTAMP(safebox_expire),"
				"UNIX_TIMESTAMP(autoloot_expire),"
				"UNIX_TIMESTAMP(fish_mind_expire),"
				"UNIX_TIMESTAMP(marriage_fast_expire),"
				"UNIX_TIMESTAMP(money_drop_rate_expire),"
				"UNIX_TIMESTAMP(create_time)"
				" FROM account WHERE login='%s'", szPasswd, szLogin);
#else
		// @correction058 1. PASSWORD('%s') -> %s 2. szPasswd wrapped inside mysql_hash_password(%s).c_str()
		DBManager::instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p,
				"SELECT '%s',password,securitycode,social_id,id,status,availDt - NOW() > 0,"
				"UNIX_TIMESTAMP(silver_expire),"
				"UNIX_TIMESTAMP(gold_expire),"
				"UNIX_TIMESTAMP(safebox_expire),"
				"UNIX_TIMESTAMP(autoloot_expire),"
				"UNIX_TIMESTAMP(fish_mind_expire),"
				"UNIX_TIMESTAMP(marriage_fast_expire),"
				"UNIX_TIMESTAMP(money_drop_rate_expire),"
				"UNIX_TIMESTAMP(create_time)"
				" FROM account WHERE login='%s'",
				mysql_hash_password(szPasswd).c_str(), szLogin);
#endif
	}
}

void CInputAuth::LoginOpenID(LPDESC d, const char * c_pData)
{
	TPacketCGLogin5 *tempInfo1 = (TPacketCGLogin5 *)c_pData;

	char* authKey = tempInfo1->authKey;
	char returnID[LOGIN_MAX_LEN + 1] = {0};

	int32_t test_url_get_protocol = auth_OpenID(authKey, inet_ntoa(d->GetAddr().sin_addr), returnID);

	if (0!=test_url_get_protocol)
	{
		LoginFailure(d, "OpenID Fail");
		return;
	}

	TPacketCGLogin3 tempInfo2;
	strncpy(tempInfo2.login, returnID, LOGIN_MAX_LEN + 1);
	strncpy(tempInfo2.passwd, "0000", PASSWD_MAX_LEN);
	for(int32_t i=0;i<4;i++)
		tempInfo2.adwClientKey[i] = tempInfo1->adwClientKey[i];
	TPacketCGLogin3 * pinfo = &tempInfo2;

	if (!g_bAuthServer)
	{
		sys_err ("CInputAuth class is not for game server. IP %s might be a hacker.",
			inet_ntoa(d->GetAddr().sin_addr));
		d->DelayedDisconnect(5);
		return;
	}

	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	char passwd[PASSWD_MAX_LEN + 1];
	strlcpy(passwd, pinfo->passwd, sizeof(passwd));

	sys_log(0, "InputAuth::Login : %s(%d) desc %p",
			login, strlen(login), get_pointer(d));

	if (false == FN_IS_VALID_LOGIN_STRING(login))
	{
		sys_log(0, "InputAuth::Login : IS_NOT_VALID_LOGIN_STRING(%s) desc %p",
				login, get_pointer(d));
		LoginFailure(d, "NOID");
		return;
	}

	if (g_bNoMoreClient)
	{
		TPacketGCLoginFailure failurePacket;

		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));

		d->Packet(&failurePacket, sizeof(failurePacket));
		return;
	}

	if (DESC_MANAGER::instance().FindByLoginName(login))
	{
		LoginFailure(d, "ALREADY");
		return;
	}

	uint32_t dwKey = DESC_MANAGER::instance().CreateLoginKey(d);
	uint32_t dwPanamaKey = dwKey ^ pinfo->adwClientKey[0] ^ pinfo->adwClientKey[1] ^ pinfo->adwClientKey[2] ^ pinfo->adwClientKey[3];
	d->SetPanamaKey(dwPanamaKey);

	sys_log(0, "InputAuth::Login : key %u:0x%x login %s", dwKey, dwPanamaKey, login);

#ifdef ENABLE_BRAZIL_AUTH_FEATURE // @correction005
	if (!test_server)
	{
		int32_t result = auth_brazil(login, passwd);

		switch (result)
		{
			case AUTH_BRAZIL_SERVER_ERR:
			case AUTH_BRAZIL_NOID:
				LoginFailure(d, "NOID");
				return;
			case AUTH_BRAZIL_WRONGPWD:
				LoginFailure(d, "WRONGPWD");
				return;
			case AUTH_BRAZIL_FLASHUSER:
				LoginFailure(d, "FLASH");
				return;
		}
	}
#endif

	TPacketCGLogin3 * p = M2_NEW TPacketCGLogin3;
	memcpy(p, pinfo, sizeof(TPacketCGLogin3));

	char szPasswd[PASSWD_MAX_LEN * 2 + 1];
	DBManager::instance().EscapeString(szPasswd, sizeof(szPasswd), passwd, strlen(passwd));

	char szLogin[LOGIN_MAX_LEN * 2 + 1];
	DBManager::instance().EscapeString(szLogin, sizeof(szLogin), login, strlen(login));

	if (Login_IsInChannelService(szLogin))
	{
		sys_log(0, "ChannelServiceLogin [%s]", szLogin);

		DBManager::instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p,
				"SELECT '%s',password,securitycode,social_id,id,status,availDt - NOW() > 0,"
				"UNIX_TIMESTAMP(silver_expire),"
				"UNIX_TIMESTAMP(gold_expire),"
				"UNIX_TIMESTAMP(safebox_expire),"
				"UNIX_TIMESTAMP(autoloot_expire),"
				"UNIX_TIMESTAMP(fish_mind_expire),"
				"UNIX_TIMESTAMP(marriage_fast_expire),"
				"UNIX_TIMESTAMP(money_drop_rate_expire),"
				"UNIX_TIMESTAMP(create_time)"
				" FROM account WHERE login='%s'",

				szPasswd, szLogin);
	}
	else
	{
#ifdef _WIN32
		DBManager::instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p,
				"SELECT PASSWORD('%s'),password,securitycode,social_id,id,status,availDt - NOW() > 0,"
				"UNIX_TIMESTAMP(silver_expire),"
				"UNIX_TIMESTAMP(gold_expire),"
				"UNIX_TIMESTAMP(safebox_expire),"
				"UNIX_TIMESTAMP(autoloot_expire),"
				"UNIX_TIMESTAMP(fish_mind_expire),"
				"UNIX_TIMESTAMP(marriage_fast_expire),"
				"UNIX_TIMESTAMP(money_drop_rate_expire),"
				"UNIX_TIMESTAMP(create_time)"
				" FROM account WHERE login='%s'", szPasswd, szLogin);
#else
		// @correction058 1. PASSWORD('%s') -> %s 2. szPasswd wrapped inside mysql_hash_password(%s).c_str()
		DBManager::instance().ReturnQuery(QID_AUTH_LOGIN, dwKey, p,
				"SELECT '%s',password,securitycode,social_id,id,status,availDt - NOW() > 0,"
				"UNIX_TIMESTAMP(silver_expire),"
				"UNIX_TIMESTAMP(gold_expire),"
				"UNIX_TIMESTAMP(safebox_expire),"
				"UNIX_TIMESTAMP(autoloot_expire),"
				"UNIX_TIMESTAMP(fish_mind_expire),"
				"UNIX_TIMESTAMP(marriage_fast_expire),"
				"UNIX_TIMESTAMP(money_drop_rate_expire),"
				"UNIX_TIMESTAMP(create_time)"
				" FROM account WHERE login='%s'",
				mysql_hash_password(szPasswd).c_str(), szLogin);
#endif
	}
}

extern void socket_timeout(int32_t s, int32_t sec, int32_t usec);

int32_t CInputAuth::auth_OpenID(const char *authKey, const char *ipAddr, char *rID)
{

	extern char openid_host[256];
	extern char openid_uri[256];

    int32_t		port	= 80;

    int32_t fd = socket_connect(openid_host, port);
    if (fd < 0)
    {
		sys_err("[auth_OpenID] : could not connect to OpenID server(%s)", openid_host);
		return 1;
    }

    socket_block(fd);
    socket_timeout(fd, 3, 0);

    {
		char request[512];
		int32_t len = snprintf(request, sizeof(request),
						"GET %s?auth_key=%s&ip=%s HTTP/1.1\r\n"
						"Host: %s\r\n"
						"Connection: Close\r\n\r\n",
						openid_uri, authKey, ipAddr, openid_host);

		if (socket_write(fd, request, len) < 0)
		{
			sys_err("[auth_OpenID] : could not send auth-request (%s)", authKey);
			socket_close(fd);
			return 2;
		}
    }

    {
	char reply[1024] = {0};
	int32_t len;
	len = socket_read(fd, reply, sizeof(reply));
	socket_close(fd);

	if (len <= 0)
	{
	    sys_err("[auth_OpenID] : could not recv auth-reply (%s)", authKey);
	    return 3;
	}

	char buffer[1024];
	strcpy(buffer, reply);

	const char *delim = "\r\n";
	char *v = strtok(buffer, delim);
	char *result = 0;

	while (v)
	{
		result = v;
		v = strtok(nullptr, delim);
	}


	char *id = strtok(result, "%");
	char *success = strtok(nullptr, "%");

	if (!*id || !*success)
	{
	    sys_err("[auth_OpenID] : OpenID AuthServer Reply Error (%s)", reply);
		return 4;
	}

	if (0 != strcmp("OK", success))
	{
		int32_t returnNumber = 0;
		str_to_number(returnNumber, id);
		switch (returnNumber)
		{
		case 1:
			sys_err("[auth_OpenID] : AuthKey incorrect");
			break;
		case 2:
			sys_err("[auth_OpenID] : ip incorrect");
			break;
		case 3:
			sys_err("[auth_OpenID] : used AuthKey");
			break;
		case 4:
			sys_err("[auth_OpenID] : AuthKey not delivered");
			break;
		case 5:
			sys_err("[auth_OpenID] : ip not delivered");
			break;
		case 6:
			sys_err("[auth_OpenID] : AuthKey time over");
			break;
		default:
			break;
		}

		return 5;
	}

	strcpy(rID, id);

	return 0;
    }
}


int32_t CInputAuth::Analyze(LPDESC d, uint8_t bHeader, const char * c_pData)
{

	if (!g_bAuthServer)
	{
		sys_err ("CInputAuth class is not for game server. IP %s might be a hacker.",
			inet_ntoa(d->GetAddr().sin_addr));
		d->DelayedDisconnect(5);
		return 0;
	}

	int32_t iExtraLen = 0;

	if (test_server)
		sys_log(0, " InputAuth Analyze Header[%d] ", bHeader);

	switch (bHeader)
	{
		case HEADER_CG_PONG:
			Pong(d);
			break;

		case HEADER_CG_LOGIN3:
			Login(d, c_pData);
			break;

		case HEADER_CG_LOGIN5_OPENID:
			if (openid_server)
				LoginOpenID(d, c_pData);
			else
				sys_err("HEADER_CG_LOGIN5_OPENID : wrong client access");
			break;

		case HEADER_CG_PASSPOD_ANSWER:
			PasspodAnswer(d, c_pData);
			break;

		case HEADER_CG_HANDSHAKE:
			break;

		default:
			sys_err("This phase does not handle this header %d (0x%x)(phase: AUTH)", bHeader, bHeader);
			break;
	}

	return iExtraLen;
}

void CInputAuth::PasspodAnswer(LPDESC d, const char * c_pData)
{

	if (!g_bAuthServer)
	{
		sys_err ("CInputAuth class is not for game server. IP %s might be a hacker.",
			inet_ntoa(d->GetAddr().sin_addr));
		d->DelayedDisconnect(5);
		return;
	}

	TPacketCGPasspod * packet = (TPacketCGPasspod*)c_pData;

	RequestConfirmPasspod Confirm;

	memcpy(Confirm.passpod, packet->szAnswer, MAX_PASSPOD + 1);
	memcpy(Confirm.login, d->GetAccountTable().login, LOGIN_MAX_LEN + 1);


	if (!d->GetAccountTable().id)
	{
		sys_err("HEADER_CG_PASSPOD_ANSWER received to desc with no account table binded");
		return;
	}

	int32_t ret_code = 1;
	sys_log(0, "Passpod start %s %s", d->GetAccountTable().login, packet->szAnswer);
	ret_code = CPasspod::instance().ConfirmPasspod(d->GetAccountTable().login, packet->szAnswer);

	if (ret_code != 0)
	{
		sys_log(0, "PASSPOD: wrong answer: %s ret_code %d", d->GetAccountTable().login, ret_code);

		LoginFailure(d, ERR_MESSAGE[ret_code]);

		if (!d->CheckMatrixTryCount())
		{
			LoginFailure(d, "QUIT");
			d->SetPhase(PHASE_CLOSE);
		}
	}
	else
	{
		sys_log(0, "PASSPOD: success: %s", d->GetAccountTable().login);
		DBManager::instance().SendAuthLogin(d);
	}

}
