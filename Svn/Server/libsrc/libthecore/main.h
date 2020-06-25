#ifndef __INC_LIBTHECORE_MAIN_H__
#define __INC_LIBTHECORE_MAIN_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __LIBTHECORE__
    extern volatile int32_t	tics;
    extern volatile int32_t shutdowned;
#endif
    extern int32_t	bCheckpointCheck;
#include "heart.h"

    extern LPHEART	thecore_heart;

    enum ENUM_PROFILER
    {
	PF_IDLE,
	PF_HEARTBEAT,
	NUM_PF
    };

    extern uint32_t		thecore_profiler[NUM_PF];

    extern int32_t			thecore_init(int32_t fps, HEARTFUNC heartbeat_func);
    extern int32_t			thecore_idle(void);
    extern void			thecore_shutdown(void);
    extern void			thecore_destroy(void);
    extern int32_t			thecore_pulse(void);
    extern float		thecore_time(void);
    extern float		thecore_pulse_per_second(void);
	extern int32_t			thecore_is_shutdowned(void);

	extern void			thecore_tick(void);

#ifdef __cplusplus
}
#endif
#endif
