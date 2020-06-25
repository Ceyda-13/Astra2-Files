#pragma once

typedef struct heart	HEART;
typedef struct heart *	LPHEART;

typedef void (*HEARTFUNC) (LPHEART heart, int32_t pulse);

struct heart
{
    HEARTFUNC		func;

    struct timeval	before_sleep;
    struct timeval	opt_time;
    struct timeval	last_time;

    int32_t			passes_per_sec;
    int32_t			pulse;
};

extern LPHEART	heart_new(int32_t opt_usec, HEARTFUNC func);
extern void	heart_delete(LPHEART ht);
extern int32_t	heart_idle(LPHEART ht);
extern void	heart_beat(LPHEART ht, int32_t pulses);


