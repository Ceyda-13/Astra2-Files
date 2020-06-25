#include "stdafx.h"

static struct timeval null_time = { 0, 0 };

#define ishprint(x)     ((((x) & 0xE0) > 0x90) || isprint(x))

void printdata(const uint8_t *data, int32_t bytes)
{
    int32_t                 i, j, k;
    const uint8_t       *p;

    fprintf(stderr, "------------------------------------------------------------------\n");
    j = bytes;
    while (1)
    {
	k       = j >= 16 ? 16 : j;

	p       = data;
	for (i = 0; i < 16; i++)
	{
	    if (i >= k)
		fprintf(stderr, "   ");
	    else
		fprintf(stderr, "%02x ", *p);
	    p++;
	}

	fprintf(stderr, "| ");

	p       = data;
	for (i = 0; i < k; i++)
	{
	    if (i >= k)
		fprintf(stderr, " ");
	    else
	    {
		fprintf(stderr, "%c", ishprint(*p) && ishprint(*(p+1)) ? *p : '.');
	    }
	    p++;
	}

	fprintf(stderr, "\n");

	j       -= 16;
	data    += 16;

	if (j <= 0)
	    break;
    }

    fprintf(stderr, "------------------------------------------------------------------\n");
}


struct timeval * timediff(const struct timeval *a, const struct timeval *b)
{
    static struct timeval rslt;

    if (a->tv_sec < b->tv_sec)
	return &null_time;
    else if (a->tv_sec == b->tv_sec)
    {
	if (a->tv_usec < b->tv_usec)
	    return &null_time;
	else
	{
	    rslt.tv_sec         = 0;
	    rslt.tv_usec        = a->tv_usec - b->tv_usec;
	    return &rslt;
	}
    }
    else
    {
	rslt.tv_sec = a->tv_sec - b->tv_sec;

	if (a->tv_usec < b->tv_usec)
	{
	    rslt.tv_usec = a->tv_usec + 1000000 - b->tv_usec;
	    rslt.tv_sec--;
	} else
	    rslt.tv_usec = a->tv_usec - b->tv_usec;

	return &rslt;
    }
}

struct timeval * timeadd(struct timeval *a, struct timeval *b)
{
    static struct timeval rslt;

    rslt.tv_sec         = a->tv_sec + b->tv_sec;
    rslt.tv_usec        = a->tv_usec + b->tv_usec;

    while (rslt.tv_usec >= 1000000)
    {
	rslt.tv_usec -= 1000000;
	rslt.tv_sec++;
    }

    return &rslt;
}


char *time_str(time_t ct)
{
    static char * time_s;

    time_s = asctime(localtime(&ct));

    time_s[strlen(time_s) - 6] = '\0';
    time_s += 4;

    return (time_s);
}

void trim_and_lower(const char * src, char * dest, size_t dest_size)
{
    const char * tmp = src;
    size_t len = 0;

    if (!dest || dest_size == 0)
	return;

    if (!src)
    {
	*dest = '\0';
	return;
    }

    while (*tmp)
    {
	if (!isnhspace(*tmp))
	    break;

	tmp++;
    }

    --dest_size;

    while (*tmp && len < dest_size)
    {
	*(dest++) = LOWER(*tmp);
	++tmp;
	++len;
    }

    *dest = '\0';

    if (len > 0)
    {
	--dest;

	while (*dest && isnhspace(*dest) && len--)
	    *(dest--) = '\0';
    }
}

void lower_string(const char *src, char *dest, size_t dest_size)
{
    const char* tmp = src;
    size_t len = 0;

    if (!dest || dest_size == 0)
	return;

    if (!src)
    {
	*dest = '\0';
	return;
    }

    --dest_size;

    while (*tmp && len < dest_size)
    {
	*(dest++) = LOWER(*tmp);
	++tmp;
	++len;
    }

    *dest = '\0';
}

char *str_dup(const char *source)
{
    char *new_line;

    CREATE(new_line, char, strlen(source) + 1);
    return (strcpy(new_line, source));
}

int32_t is_abbrev(char *arg1, char *arg2)
{
    if (!*arg1)
	return 0;

    for (; *arg1 && *arg2; arg1++, arg2++)
	if (LOWER(*arg1) != LOWER(*arg2))
	    return 0;

    if (!*arg1)
	return 1;
    else
	return 0;
}


int32_t filesize(FILE *fp)
{
    int32_t pos;
    int32_t size;

    pos = ftell(fp);

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, pos, SEEK_SET);

    return (size);
}


void parse_token(char *src, char *token, char *value)
{
    char *tmp;

    for (tmp = src; *tmp && *tmp != ':'; tmp++)
    {
	if (isnhspace(*tmp))
	    continue;

	*(token++) = LOWER(*tmp);
    }

    *token = '\0';

    for (tmp += 2; *tmp; tmp++)
    {
	if (*tmp == '\n' || *tmp == '\r')
	    continue;

	*(value++) = *tmp;
    }   

    *value = '\0';
}   


struct tm * tm_calc(const struct tm * curr_tm, int32_t days)
{
    char                yoon = 0;
    static struct tm    new_tm;
    int32_t                 monthdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (!curr_tm)
    {       
	time_t time_s = time(0);
	new_tm = *localtime(&time_s);
    }
    else    
	memcpy(&new_tm, curr_tm, sizeof(struct tm));

    if (new_tm.tm_mon == 1)
    {
	if (!((new_tm.tm_year + 1900) % 4))
	{
	    if (!((new_tm.tm_year + 1900) % 100))
	    {
		if (!((new_tm.tm_year + 1900) % 400))
		    yoon = true;
	    }   
	    else
			yoon = true;
	}

	if (yoon)
	    new_tm.tm_mday += 1;
    }

    if (yoon)
		monthdays[1] = 29;
    else
		monthdays[1] = 28;

    new_tm.tm_mday += days;

    if (new_tm.tm_mday <= 0)
    {
	new_tm.tm_mon--;

	if (new_tm.tm_mon < 0)
	{
	    new_tm.tm_year--;
	    new_tm.tm_mon = 11;
	}

	new_tm.tm_mday = monthdays[new_tm.tm_mon] + new_tm.tm_mday;
    }
    else if (new_tm.tm_mday > monthdays[new_tm.tm_mon])
    {
	new_tm.tm_mon++;

	if (new_tm.tm_mon > 11)
	{
	    new_tm.tm_year++;
	    new_tm.tm_mon = 0;
	}

	new_tm.tm_mday = monthdays[new_tm.tm_mon] - new_tm.tm_mday;
    }

    return (&new_tm);
}

int32_t MIN(int32_t a, int32_t b)
{
    return a < b ? a : b;
}

int32_t MAX(int32_t a, int32_t b)
{
    return a > b ? a : b;
}

int32_t MINMAX(int32_t min, int32_t value, int32_t max)
{
    int32_t tv;

    tv = (min > value ? min : value);
    return (max < tv) ? max : tv;
}

uint32_t thecore_random()
{
#ifdef __WIN32__
    return rand();
#else
    return random();
#endif
}

int32_t number_ex(int32_t from, int32_t to, const char *file, int32_t line)
{
    if (from > to)
    {
	int32_t tmp = from;

	sys_err("number(): first argument is bigger than second argument %d -> %d, %s %d", from, to, file, line);

	from = to;
	to = tmp;
    }

	int32_t returnValue = 0;

	if ((to - from + 1) != 0)
		returnValue = ((thecore_random() % (to - from + 1)) + from);
	else
		sys_err("number(): devided by 0");

    return returnValue;
}

float fnumber(float from, float to)
{
	return (((float)thecore_random() / (float)RAND_MAX) * (to - from)) + from;
}

#ifndef __WIN32__
void thecore_sleep(struct timeval* timeout)
{
    if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, timeout) < 0)
    {
	if (errno != EINTR)
	{
	    sys_err("select sleep %s", strerror(errno));
	    return;
	}
    }
}

void thecore_msleep(uint32_t dwMillisecond)
{
    static struct timeval tv_sleep;
    tv_sleep.tv_sec = dwMillisecond / 1000;
    tv_sleep.tv_usec = dwMillisecond * 1000;
    thecore_sleep(&tv_sleep);
}

void core_dump_unix(const char *who, uint16_t line)
{   
    sys_err("*** Dumping Core %s:%d ***", who, line);

    fflush(stdout);
    fflush(stderr); 

    if (fork() == 0)
        abort();
}
#else

void thecore_sleep(struct timeval* timeout)
{
    Sleep(timeout->tv_sec * 1000 + timeout->tv_usec / 1000);
}

void thecore_msleep(uint32_t dwMillisecond)
{
    Sleep(dwMillisecond);
}

void gettimeofday(struct timeval* t, struct timezone* dummy)
{
    uint32_t millisec = GetTickCount();

    t->tv_sec = (millisec / 1000);
    t->tv_usec = (millisec % 1000) * 1000;
}

void core_dump_unix(const char *who, uint16_t line)
{   
}

#endif

float get_float_time()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    tv.tv_sec -= 1057699978;
    return ((float) tv.tv_sec + ((float) tv.tv_usec / 1000000.0f));
}

static uint32_t get_boot_sec()
{
	static struct timeval tv_boot = {0L, 0L};

	if (tv_boot.tv_sec == 0)
		gettimeofday(&tv_boot, nullptr);

	return tv_boot.tv_sec;
}

uint32_t get_dword_time()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    tv.tv_sec -= get_boot_sec();
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}
