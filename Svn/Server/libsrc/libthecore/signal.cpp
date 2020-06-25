#define __LIBTHECORE__
#include "stdafx.h"

#ifdef _WIN32
void signal_setup() {}
void signal_timer_disable() {}
void signal_timer_enable(int32_t timeout_seconds) {}
#elif __FreeBSD__
#define RETSIGTYPE void

RETSIGTYPE reap(int32_t sig)
{
    while (waitpid(-1, nullptr, WNOHANG) > 0);
    signal(SIGCHLD, reap);
}


RETSIGTYPE checkpointing(int32_t sig)
{
    if (!tics)
    {
        sys_err("CHECKPOINT shutdown: tics did not updated.");
        if (bCheckpointCheck)
            abort();
    }
    else
		tics = 0;
}


RETSIGTYPE hupsig(int32_t sig)
{
    shutdowned = true;
    sys_err("SIGHUP, SIGINT, SIGTERM signal has been received. shutting down.");
}

RETSIGTYPE usrsig(int32_t sig)
{
    core_dump();
}

void signal_timer_disable(void)
{
    struct itimerval itime;
    struct timeval interval;

    interval.tv_sec	= 0;
    interval.tv_usec	= 0;

    itime.it_interval = interval;
    itime.it_value = interval;

    setitimer(ITIMER_VIRTUAL, &itime, nullptr);
}

void signal_timer_enable(int32_t sec)
{
    struct itimerval itime;
    struct timeval interval;

    interval.tv_sec	= sec;
    interval.tv_usec	= 0;

    itime.it_interval = interval;
    itime.it_value = interval;

    setitimer(ITIMER_VIRTUAL, &itime, nullptr);
}

void signal_setup(void)
{
    signal_timer_enable(30);

    signal(SIGVTALRM, checkpointing);

    signal(SIGHUP, hupsig);
    signal(SIGCHLD, reap);
    signal(SIGINT, hupsig);
    signal(SIGTERM, hupsig);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGUSR1, usrsig);
}

#endif
