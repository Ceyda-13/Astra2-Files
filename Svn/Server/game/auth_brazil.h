#pragma once

#define AUTH_BRAZIL_SERVER_ERR		0
#define AUTH_BRAZIL_SUCC		1
#define AUTH_BRAZIL_NOID		2
#define AUTH_BRAZIL_WRONGPWD		3
#define AUTH_BRAZIL_FLASHUSER		4

int32_t auth_brazil(const char *login, const char *pwd);


int32_t auth_brazil_inc_query_count();
void auth_brazil_log();


