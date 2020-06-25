#define __LIBTHECORE__
#include "stdafx.h"
#include "memory.h"

LPHEART		thecore_heart = nullptr;

volatile int32_t	shutdowned = false;
volatile int32_t	tics = 0;
uint32_t	thecore_profiler[NUM_PF];

int32_t	bCheckpointCheck = 1;

static int32_t pid_init(void)
{
#ifdef _WIN32
	return true;
#else
	FILE*	fp;
	if ((fp = fopen("pid", "w")))
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
		sys_log(0, "\nStart of pid: %d\n", getpid()); // @warme012
	}
	else
	{
		printf("pid_init(): could not open file for writing. (filename: ./pid)");
		sys_err("\nError writing pid file\n");
		return false;
	}
	return true;
#endif
}

static void pid_deinit(void)
{
#ifdef _WIN32
    return;
#else
    remove("./pid");
	sys_log(0, "\nEnd of pid\n"); // @warme012
#endif
}

int32_t thecore_init(int32_t fps, HEARTFUNC heartbeat_func)
{
#ifdef _WIN32
    srand(time(0));
#else
    srandom(time(0) + getpid() + getuid());
    srandomdev();
#endif
    signal_setup();

	if (!log_init() || !pid_init())
		return false;

	thecore_heart = heart_new(1000000 / fps, heartbeat_func);
	return true;
}

void thecore_shutdown()
{
    shutdowned = true;
}

int32_t thecore_idle(void)
{
    thecore_tick();

    if (shutdowned)
		return 0;

	int32_t pulses;
	uint32_t t = get_dword_time();

	if (!(pulses = heart_idle(thecore_heart)))
	{
		thecore_profiler[PF_IDLE] += (get_dword_time() - t);
		return 0;
	}

    thecore_profiler[PF_IDLE] += (get_dword_time() - t);
    return pulses;
}

void thecore_destroy(void)
{
	pid_deinit();
	log_destroy();
}

int32_t thecore_pulse(void)
{
	return (thecore_heart->pulse);
}

float thecore_pulse_per_second(void)
{
	return ((float) thecore_heart->passes_per_sec);
}

float thecore_time(void)
{
	return ((float) thecore_heart->pulse / (float) thecore_heart->passes_per_sec);
}

int32_t thecore_is_shutdowned(void)
{
	return shutdowned;
}

void thecore_tick(void)
{
	++tics;
}
